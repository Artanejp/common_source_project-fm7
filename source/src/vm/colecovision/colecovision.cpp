/*
	COLECO ColecoVision Emulator 'yaCOLECOVISION'

	Author : tanam
	Date   : 2016.08.14-

	[ virtual machine ]
*/

#include "colecovision.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../io.h"
#include "../sn76489an.h"
#include "../tms9918a.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "./keyboard.h"
#include "./memory.h"

using COLECOVISION::KEYBOARD;
using COLECOVISION::MEMORY;
// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU_TEMPLATE* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device

	io = new IO(this, emu);
	io->space = 0x100;
	psg = new SN76489AN(this, emu);
#ifdef USE_DEBUGGER
//	psg->set_context_debugger(new DEBUGGER(this, emu));
#endif
	vdp = new TMS9918A(this, emu);
#ifdef USE_DEBUGGER
	vdp->set_context_debugger(new DEBUGGER(this, emu));
#endif
	cpu = new Z80(this, emu);

	key = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);

	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);

	vdp->set_context_irq(cpu, SIG_CPU_NMI, 1);
	key->set_context_cpu(cpu);

	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif

	// i/o bus
	io->set_iomap_range_w(0x80, 0x9f, key);
	io->set_iomap_range_rw(0xbe, 0xbf, vdp);
	io->set_iomap_range_w(0xc0, 0xdf, key);
	io->set_iomap_range_r(0xfc, 0xff, key);
	io->set_iomap_range_w(0xff, 0xff, psg);

	// initialize all devices
#if defined(__GIT_REPO_VERSION)
	set_git_repo_version(__GIT_REPO_VERSION);
#endif
	initialize_devices();
}

VM::~VM()
{
	// delete all devices
	release_devices();
//	for(DEVICE* device = first_device; device;) {
//		DEVICE *next_device = device->next_device;
//		device->release();
//		delete device;
//		device = next_device;
//	}
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
// drive virtual machine
// ----------------------------------------------------------------------------

void VM::reset()
{
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
}

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	vdp->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);

	// init sound gen
	psg->initialize_sound(rate, 3579545, 8000);
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
		psg->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_cart(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		memory->open_cart(file_path);
		reset();
	}
}

void VM::close_cart(int drv)
{
	if(drv == 0) {
		memory->close_cart();
		reset();
	}
}

bool VM::is_cart_inserted(int drv)
{
	if(drv == 0) {
		return memory->is_cart_inserted();
	} else {
		return false;
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

#define STATE_VERSION	3

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!(VM_TEMPLATE::process_state_core(state_fio, loading, STATE_VERSION))) {
		return false;
	}
	// Machine specified.
//	if(loading) {
//		update_config();
//	}
 	return true;
}
