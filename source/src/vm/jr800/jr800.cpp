/*
	National JR-800 Emulator 'eJR-800'

	Author : Takeda.Toshiya
	Date   : 2017.03.13-

	[ virtual machine ]
*/

#include "jr800.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../hd44102.h"
//#include "../mc6800.h"
#include "../hd6301.h"
#include "../memory.h"
#include "../noise.h"
#include "../pcm1bit.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "io.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
	for(int i = 0; i < 8; i++) {
		lcd[i] = new HD44102(this, emu);
	}
//	cpu = new MC6800(this, emu);
	cpu = new HD6301(this, emu);
	memory = new MEMORY(this, emu);
	pcm = new PCM1BIT(this, emu);
	
	io = new IO(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(drec);
	event->set_context_sound(pcm);
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
//	cpu->set_context_port1(drec, SIG_DATAREC_MIC, 0x01, 0);
	cpu->set_context_port1(pcm, SIG_PCM1BIT_ON, 0x08, 0);
	cpu->set_context_port1(pcm, SIG_PCM1BIT_SIGNAL, 0x10, 0);
	
	for(int i = 0; i < 8; i++) {
		io->set_context_lcd(i, lcd[i]);
	}
	
	// cpu bus
	cpu->set_context_mem(memory);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// memory bus
	memset(ram, 0x00, sizeof(ram));
	memset(rom, 0xff, sizeof(rom));
	
	memory->read_bios(_T("BASIC.ROM"), rom, sizeof(rom));
	
	memory->set_memory_rw(0x2000, 0x7fff, ram);
	memory->set_memory_r(0x8000, 0xffff, rom);
	memory->set_memory_mapped_io_rw(0x0a00, 0x0bff, io);
	memory->set_memory_mapped_io_rw(0x0d00, 0x0fff, io);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
}

VM::~VM()
{
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
	cpu->write_signal(SIG_MC6801_PORT_1, 0x00, 0xff);
	cpu->write_signal(SIG_MC6801_PORT_2, 0x00, 0xff);
	cpu->write_signal(SIG_MC6801_PORT_3, 0x00, 0xff);
	cpu->write_signal(SIG_MC6801_PORT_4, 0x00, 0xff);
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
	for(int y = 0; y < 2; y++) {
		for(int x = 0; x < 4; x++) {
			lcd[x + 4 * y]->screen_update(50 * x - 4, 32 * y, y == 0);
		}
	}
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	pcm->initialize_sound(rate, 8000);
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
		pcm->set_volume(0, decibel_l, decibel_r);
	} if(ch == 1) {
		drec->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 2) {
		drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	drec->play_tape(file_path);
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	drec->rec_tape(file_path);
}

void VM::close_tape(int drv)
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
}

bool VM::is_tape_inserted(int drv)
{
	return drec->is_tape_inserted();
}

bool VM::is_tape_playing(int drv)
{
	return drec->is_tape_playing();
}

bool VM::is_tape_recording(int drv)
{
	return drec->is_tape_recording();
}

int VM::get_tape_position(int drv)
{
	return drec->get_tape_position();
}

const _TCHAR* VM::get_tape_message(int drv)
{
	return drec->get_message();
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

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
	state_fio->Fwrite(ram, sizeof(ram), 1);
}

bool VM::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(!device->load_state(state_fio)) {
			return false;
		}
	}
	state_fio->Fread(ram, sizeof(ram), 1);
	return true;
}

