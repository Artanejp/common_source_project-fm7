/*
	HITACH BASIC Master Jr Emulator 'eBASICMasterJr'

	Author : Takeda.Toshiya
	Date   : 2015.08.28-

	[ virtual machine ]
*/

#include "bmjr.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../mc6800.h"
#include "../mc6820.h"
#include "../noise.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "./memory.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------
using BMJR::MEMORY;

VM::VM(EMU_TEMPLATE* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	//first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
	cpu = new MC6800(this, emu);
	pia = new MC6820(this, emu);
	
	memory = new MEMORY(this, emu);
	
	// Set names
#if defined(_USE_QT)
	dummy->set_device_name(_T("1st Dummy"));
#endif
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(drec);
	event->set_context_sound(memory);
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	drec->set_context_ear(memory, SIG_MEMORY_DATAREC_EAR, 1);
	
	memory->set_context_drec(drec);
	memory->set_context_cpu(cpu);
	memory->set_context_pia(pia);
	
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
	release_devices();
}

// ----------------------------------------------------------------------------
// drive virtual machine
// ----------------------------------------------------------------------------


void VM::special_reset(int num)
{
	// reset all devices	
	VM_TEMPLATE::reset(); // OK?
}


double VM::get_frame_rate()
{
	if(event != nullptr) {
		return event->get_frame_rate();
	}
	return VM_TEMPLATE::get_frame_rate();
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
	return nullptr;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	if(memory != nullptr) {
		memory->draw_screen();
	}
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	if(event != nullptr) {
		event->initialize_sound(rate, samples);
	}
}

uint16_t* VM::create_sound(int* extra_frames)
{
	if(event != nullptr) {
		return event->create_sound(extra_frames);
	}
	return VM_TEMPLATE::create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
	if(event != nullptr) {
		return event->get_sound_buffer_ptr();
	}
	return VM_TEMPLATE::get_sound_buffer_ptr();
}

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch == 0) {
		if(memory != nullptr) {
			memory->set_volume(0, decibel_l, decibel_r);
		}
	} else if(ch == 1) {
		if(drec != nullptr) {
			drec->set_volume(0, decibel_l, decibel_r);
		}
	} else if(ch == 2) {
		if(drec != nullptr) {
			// ToDo: Fixme
			drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
			drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
			drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
		}
 	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	if((!repeat) && (memory != nullptr)) {
		memory->key_down(code);
	}
}

void VM::key_up(int code)
{
	if(memory != nullptr) {
		memory->key_up(code);
	}
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	if(drec == nullptr) return;
	
	bool remote = drec->get_remote();
	
	if(drec->play_tape(file_path) && remote) {
		// if machine already sets remote on, start playing now
		push_play(drv);
	}
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	if(drec == nullptr) return;
	bool remote = drec->get_remote();
	
	if(drec->rec_tape(file_path) && remote) {
		// if machine already sets remote on, start recording now
		push_play(drv);
	}
}

void VM::close_tape(int drv)
{
	if(drec == nullptr) return;
	
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
	drec->set_remote(false);
}

bool VM::is_tape_inserted(int drv)
{
	if(drec != nullptr) {
		return drec->is_tape_inserted();
	}
	return VM_TEMPLATE::is_tape_inserted(drv);
}

bool VM::is_tape_playing(int drv)
{
	if(drec != nullptr) {
		return drec->is_tape_playing();
	}
	return false;
}

bool VM::is_tape_recording(int drv)
{
	if(drec != nullptr) {
		return drec->is_tape_recording();
	}
	return VM_TEMPLATE::is_tape_recording(drv);
}

int VM::get_tape_position(int drv)
{
	if(drec != nullptr) {
		return drec->get_tape_position();
	}
	return 0;
}

const _TCHAR* VM::get_tape_message(int drv)
{
	if(drec != nullptr) {
		return drec->get_message();
	}
	return _T("");
}

void VM::push_play(int drv)
{
	if(drec == nullptr) return;
	drec->set_remote(false);
	drec->set_ff_rew(0);
	drec->set_remote(true);
}

void VM::push_stop(int drv)
{
	if(drec == nullptr) return;
	drec->set_remote(false);
}

void VM::push_fast_forward(int drv)
{
	if(drec == nullptr) return;
	drec->set_remote(false);
	drec->set_ff_rew(1);
	drec->set_remote(true);
}

void VM::push_fast_rewind(int drv)
{
	if(drec == nullptr) return;
	drec->set_remote(false);
	drec->set_ff_rew(-1);
	drec->set_remote(true);
}

bool VM::is_frame_skippable()
{
	if(event != nullptr) {
		return event->is_frame_skippable();
	}
	return false;
}

double VM::get_current_usec()
{
	if(event != nullptr) {
		return event->get_current_usec();
	}
	return VM_TEMPLATE::get_current_usec();
}

uint64_t VM::get_current_clock_uint64()
{
	if(event != nullptr) {
		return event->get_current_clock_uint64();
	}
	return VM_TEMPLATE::get_current_clock_uint64();
}

#define STATE_VERSION	3

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!(VM_TEMPLATE::process_state_core(state_fio, loading, STATE_VERSION))) {
		return false;
	}
	// Machine specified.
	if(loading) {
		update_config();
	}
 	return true;
}
