/*
	CANON X-07 Emulator 'eX-07'

	Author : Takeda.Toshiya
	Date   : 2007.12.26 -

	[ virtual machine ]
*/

#include "x07.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../beep.h"
#include "../memory.h"
#include "../z80.h"

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
	
	beep = new BEEP(this, emu);
	memory = new MEMORY(this, emu);
	cpu = new Z80(this, emu);
	
	io = new IO(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(beep);
	
	io->set_context_beep(beep);
	io->set_context_cpu(cpu);
	io->set_context_mem(memory, ram);
	
	// memory bus
	memset(ram, 0, sizeof(ram));
	memset(app, 0xff, sizeof(app));
	memset(tv, 0xff, sizeof(tv));
	memset(bas, 0xff, sizeof(bas));
	
	memory->read_bios(_T("APP.ROM"), app, sizeof(app));
	memory->read_bios(_T("TV.ROM"), tv, sizeof(tv));
	memory->read_bios(_T("BASIC.ROM"), bas, sizeof(bas));
	
	memory->set_memory_rw(0x0000, 0x5fff, ram);
	memory->set_memory_r(0x6000, 0x7fff, app);
	memory->set_memory_rw(0x8000, 0x97ff, vram);
	memory->set_memory_r(0xa000, 0xafff, tv);
	memory->set_memory_r(0xb000, 0xffff, bas);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(io);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
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
	
	// init sound gen
	beep->init(rate, 1000, 8000);
}

uint16* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

int VM::sound_buffer_ptr()
{
	return event->sound_buffer_ptr();
}

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	io->key_down(code);
}

void VM::key_up(int code)
{
	io->key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::play_tape(_TCHAR* file_path)
{
	io->play_tape(file_path);
}

void VM::rec_tape(_TCHAR* file_path)
{
	io->rec_tape(file_path);
}

void VM::close_tape()
{
	io->close_tape();
}

bool VM::tape_inserted()
{
	return io->tape_inserted();
}

bool VM::now_skip()
{
	return event->now_skip();
}

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

