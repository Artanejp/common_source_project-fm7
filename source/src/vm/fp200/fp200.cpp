/*
	CASIO FP-200 Emulator 'eFP-200'

	Author : Takeda.Toshiya
	Date   : 2013.03.21-

	[ virtual machine ]
*/

#include "fp200.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../i8080.h"
#include "../memory.h"
#include "../noise.h"
#include "../rp5c01.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "io.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
	cpu = new I8080(this, emu);	// i8085
	memory = new MEMORY(this, emu);
	rtc = new RP5C01(this, emu);
	
	io = new IO(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(drec);
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	drec->set_context_ear(io, SIG_IO_CMT, 1);
	cpu->set_context_sod(io, SIG_IO_SOD, 1);
	
	io->set_context_cpu(cpu);
	io->set_context_drec(drec);
	io->set_context_rtc(rtc);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(io);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// memory bus
	memset(rom, 0xff, sizeof(rom));
	memset(ram, 0, sizeof(ram));
	
	memory->read_bios(_T("BIOS.ROM"), rom, sizeof(rom));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("RAM.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
	
	memory->set_memory_r(0x0000, 0x7fff, rom);
	memory->set_memory_rw(0x8000, 0xffff, ram);
	memory->set_wait_rw(0x0000, 0xffff, 1);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
}

VM::~VM()
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("RAM.BIN")), FILEIO_WRITE_BINARY)) {
		fio->Fwrite(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
	
	// delete all devices
	for(DEVICE* device = first_device; device;) {
		DEVICE *next_device = device->next_device;
		device->release();
		delete device;
		device = next_device;
	}
}

DEVICE* VM::get_device(int id)
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(device->this_device_id == id) {
			return device;
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// drive virtual machine
// ----------------------------------------------------------------------------

void VM::reset()
{
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
}

void VM::run()
{
	event->drive();
}

// ----------------------------------------------------------------------------
// debugger
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
DEVICE *VM::get_cpu(int index)
{
	if(index == 0) {
		return cpu;
	}
	return NULL;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	io->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
}

uint16_t* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
	return event->get_sound_buffer_ptr();
}

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch == 0) {
		drec->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	if(!repeat) {
		io->key_down(code);
	}
}

void VM::key_up(int code)
{
	io->key_up();
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	io->close_tape();
	
	bool remote = drec->get_remote();
	
	if(drec->play_tape(file_path) && remote) {
		// if machine already sets remote on, start playing now
		push_play(drv);
	}
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
	drec->set_remote(false);
	
	io->rec_tape(file_path);
}

void VM::close_tape(int drv)
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
	drec->set_remote(false);
	
	io->close_tape();
}

bool VM::is_tape_inserted(int drv)
{
	return drec->is_tape_inserted() || io->is_tape_inserted();
}

bool VM::is_tape_playing(int drv)
{
	if(drec->is_tape_inserted()) {
		return drec->is_tape_playing();
	} else {
		return io->is_tape_playing();
	}
}

bool VM::is_tape_recording(int drv)
{
	if(drec->is_tape_inserted()) {
		return drec->is_tape_recording();
	} else {
		return io->is_tape_recording();
	}
}

int VM::get_tape_position(int drv)
{
	if(drec->is_tape_inserted()) {
		return drec->get_tape_position();
	} else {
		return io->get_tape_position();
	}
}

const _TCHAR* VM::get_tape_message(int drv)
{
	if(drec->is_tape_inserted()) {
		return drec->get_message();
	} else {
		return NULL;
	}
}

void VM::push_play(int drv)
{
	if(drec->is_tape_inserted()) {
		drec->set_remote(false);
		drec->set_ff_rew(0);
		drec->set_remote(true);
	}
}

void VM::push_stop(int drv)
{
	if(drec->is_tape_inserted()) {
		drec->set_remote(false);
	}
}

void VM::push_fast_forward(int drv)
{
	if(drec->is_tape_inserted()) {
		drec->set_remote(false);
		drec->set_ff_rew(1);
		drec->set_remote(true);
	}
}

void VM::push_fast_rewind(int drv)
{
	if(drec->is_tape_inserted()) {
		drec->set_remote(false);
		drec->set_ff_rew(-1);
		drec->set_remote(true);
	}
}

bool VM::is_frame_skippable()
{
	return event->is_frame_skippable();
}

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

#define STATE_VERSION	3

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		const char *name = typeid(*device).name() + 6; // skip "class "
		int len = (int)strlen(name);
		
		if(!state_fio->StateCheckInt32(len)) {
			return false;
		}
		if(!state_fio->StateCheckBuffer(name, len, 1)) {
			return false;
		}
		if(!device->process_state(state_fio, loading)) {
			return false;
		}
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}

