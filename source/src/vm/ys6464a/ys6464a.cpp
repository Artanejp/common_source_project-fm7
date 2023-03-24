/*
	SHINKO SANGYO YS-6464A Emulator 'eYS-6464A'

	Author : Takeda.Toshiya
	Date   : 2009.12.30 -

	[ virtual machine ]
*/

#include "ys6464a.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../io.h"
#include "../i8255.h"
#include "../memory.h"
#include "../pcm1bit.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
#include "keyboard.h"

using YS6464A::DISPLAY;
using YS6464A::KEYBOARD;

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU_TEMPLATE* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	dummy->set_device_name(_T("1st Dummy"));
	io = new IO(this, emu);
	io->space = 0x100;
	pio = new I8255(this, emu);
	memory = new MEMORY(this, emu);
	memory->space = 0x10000;
	memory->bank_size = 0x2000;

//	pcm = new PCM1BIT(this, emu);
	cpu = new Z80(this, emu);

	display = new DISPLAY(this, emu);
	keyboard = new KEYBOARD(this, emu);

	// set contexts
	event->set_context_cpu(cpu);
//	event->set_context_sound(pcm);

//	pio->set_context_port_b(pcm, SIG_PCM1BIT_SIGNAL, 0x01, 0);
	pio->set_context_port_b(display, SIG_DISPLAY_PORT_B, 0xf0, 0);
	pio->set_context_port_c(display, SIG_DISPLAY_PORT_C, 0xf0, 0);
	pio->set_context_port_c(keyboard, SIG_KEYBOARD_PORT_C, 0xf0, 0);
	keyboard->set_context_pio(pio);

	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif

	// memory bus
	memset(ram, 0, sizeof(ram));
	memset(rom, 0xff, sizeof(rom));

	memory->read_bios(_T("MON.ROM"), rom, sizeof(rom));

	memory->set_memory_r(0x0000, 0x1fff, rom);
	memory->set_memory_r(0x2000, 0x3fff, rom);
	memory->set_memory_r(0x4000, 0x5fff, rom);
	memory->set_memory_r(0x6000, 0x7fff, rom);
	memory->set_memory_rw(0x8000, 0x9fff, ram);
	memory->set_memory_rw(0xa000, 0xbfff, ram);
	memory->set_memory_rw(0xc000, 0xdfff, ram);
	memory->set_memory_rw(0xe000, 0xffff, ram);

	// i/o bus
	io->set_iomap_range_w(0xf8, 0xfb, pio);
	io->set_iomap_range_r(0xf8, 0xfb, pio);

	// initialize all devices
#if defined(__GIT_REPO_VERSION)
	set_git_repo_version(__GIT_REPO_VERSION);
#endif
	initialize_devices();
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
	display->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);

	// init sound gen
//	pcm->initialize_sound(rate, 8000);
}

uint16_t* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
	return event->get_sound_buffer_ptr();
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::load_binary(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		memory->read_image(file_path, ram, sizeof(ram));
	}
}

void VM::save_binary(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		memory->write_image(file_path, ram, sizeof(ram));
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

double VM::get_current_usec()
{
	if(event == NULL) return 0.0;
	return event->get_current_usec();
}

uint64_t VM::get_current_clock_uint64()
{
		if(event == NULL) return (uint64_t)0;
		return event->get_current_clock_uint64();
}

#define STATE_VERSION	2

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!(VM_TEMPLATE::process_state_core(state_fio, loading, STATE_VERSION))) {
		return false;
	}
	// Machine specified.
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}
