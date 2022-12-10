/*
	MICOM MAHJONG Emulator 'eMuCom Mahjong'

	Author : Hiroaki GOTO as GORRY
	Date   : 2020.07.20 -

	[ virtual machine ]
*/

#include "micom_mahjong.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../pcm1bit.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "./keyboard.h"
#include "./memory.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------
using MICOM_MAHJONG::KEYBOARD;
using MICOM_MAHJONG::MEMORY;

VM::VM(EMU_TEMPLATE* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device

	dummy->set_device_name(_T("1st Dummy"));

	cpu = new Z80(this, emu);
	
	memory = new MEMORY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	pcm = new PCM1BIT(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	
	memory->set_context_keyboard(keyboard);
	memory->set_context_pcm(pcm);
	
	keyboard->set_context_memory(memory);
	
	// cpu bus
	cpu->set_context_mem(memory);
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
	memory->draw_screen();
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
	}
}
#endif

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

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!(VM_TEMPLATE::process_state_core(state_fio, loading, STATE_VERSION))) {
		return false;
	}
	// Machine specified.
	return true;
}
