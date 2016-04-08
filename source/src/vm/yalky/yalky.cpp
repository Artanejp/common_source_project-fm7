/*
	Yuasa Kyouiku System YALKY Emulator 'eYALKY'

	Author : Takeda.Toshiya
	Date   : 2016.03.28-

	[ virtual machine ]
*/

#include "yalky.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../i8080.h"
#include "../i8155.h"
#include "../memory.h"
#include "../not.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "io.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	config.tape_sound = false;
	config.wave_shaper = false;
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	cpu = new I8080(this, emu);	// 8085
	pio = new I8155(this, emu);	// 8156
	memory = new MEMORY(this, emu);
	not_ear = new NOT(this, emu);
	
	io = new IO(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(drec);
	
	drec->set_context_rotate(io, SIG_IO_DREC_ROT, 1);
	drec->set_context_ear(cpu, SIG_I8085_RST5, 1);
	drec->set_context_ear(not_ear, SIG_NOT_INPUT, 1);
	not_ear->set_context_out(cpu, SIG_I8085_SID, 1);
	cpu->set_context_sod(drec, SIG_DATAREC_MIC, 1);
	pio->set_context_port_b(io, SIG_IO_PORT_B, 0xff, 0);
	pio->set_context_port_c(io, SIG_IO_PORT_C, 0xff, 0);
	pio->set_context_timer(cpu, SIG_I8085_RST7, 1);
	pio->set_constant_clock(CPU_CLOCKS);
	
	io->set_context_drec(drec);
	io->set_context_pio(pio);
	io->set_vram_ptr(vram);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(io);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// memory bus
	memset(rom, 0xff, sizeof(rom));
	
	memory->read_bios(_T("BIOS.ROM"), rom, sizeof(rom));
	
	memory->set_memory_r(0x0000, 0x1fff, rom);
	memory->set_memory_rw(0x4000, 0x43ff, vram);
	memory->set_memory_rw(0x6000, 0x60ff, ram);
	
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
//	pio->write_signal(SIG_I8155_PORT_A, 0x80, 0xa0); // PA5=0,PA7=1
	pio->write_signal(SIG_I8155_PORT_A, 0xa0, 0xa0); // PA5=1,PA7=1
	memset(ram, 0, sizeof(ram));
	memset(vram, 0, sizeof(vram));
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
	drec->set_volume(0, decibel_l, decibel_r);
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::play_tape(const _TCHAR* file_path)
{
	drec->play_tape(file_path);
	drec->set_remote(true);
}

void VM::rec_tape(const _TCHAR* file_path)
{
	drec->rec_tape(file_path);
	drec->set_remote(true);
}

void VM::close_tape()
{
	drec->close_tape();
	drec->set_remote(false);
}

bool VM::is_tape_inserted()
{
	return drec->is_tape_inserted();
}

bool VM::is_tape_playing()
{
	return drec->is_tape_playing();
}

bool VM::is_tape_recording()
{
	return drec->is_tape_recording();
}

int VM::get_tape_position()
{
	return drec->get_tape_position();
}

void VM::push_play()
{
	drec->set_ff_rew(0);
	drec->set_remote(true);
}

void VM::push_stop()
{
	drec->set_remote(false);
}

void VM::push_fast_forward()
{
	drec->set_ff_rew(1);
	drec->set_remote(true);
}

void VM::push_fast_rewind()
{
	drec->set_ff_rew(-1);
	drec->set_remote(true);
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

#define STATE_VERSION	1

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->Fwrite(vram, sizeof(vram), 1);
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
	state_fio->Fread(vram, sizeof(vram), 1);
	return true;
}

