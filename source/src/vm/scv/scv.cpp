/*
	EPOCH Super Cassette Vision Emulator 'eSCV'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ virtual machine ]
*/

#include "scv.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../upd7801.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "io.h"
#include "memory.h"
#include "sound.h"
#include "vdp.h"

using SCV::IO;
using SCV::MEMORY;
using SCV::SOUND;
using SCV::VDP;

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
	cpu = new UPD7801(this, emu);
	
	io = new IO(this, emu);
	memory = new MEMORY(this, emu);
	sound = new SOUND(this, emu);
	vdp = new VDP(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(sound);
	
	io->set_context_mem(memory);
	io->set_context_sound(sound);
	memory->set_context_sound(sound);
	sound->set_context_cpu(cpu);
	vdp->set_context_cpu(cpu);
	vdp->set_font_ptr(memory->get_font());
	vdp->set_vram_ptr(memory->get_vram());
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
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
	sound->initialize_sound(rate);
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
	sound->set_volume(ch, decibel_l, decibel_r);
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

#define STATE_VERSION	2

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!(VM_TEMPLATE::process_state_core(state_fio, loading, STATE_VERSION))) {
		return false;
	}
	// Machine specified.
	return true;
}
