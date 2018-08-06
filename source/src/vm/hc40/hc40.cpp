/*
	EPSON HC-40 Emulator 'eHC-40'

	Author : Takeda.Toshiya
	Date   : 2008.02.23 -

	[ virtual machine ]
*/

#include "hc40.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../beep.h"
#include "../datarec.h"
#include "../noise.h"
#include "../ptf20.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "io.h"
#include "memory.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	dummy->set_device_name(_T("1st Dummy"));
	
	beep = new BEEP(this, emu);
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
	tf20 = new PTF20(this, emu);
	cpu = new Z80(this, emu);
	
	io = new IO(this, emu);
	memory = new MEMORY(this, emu);
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(beep);
	event->set_context_sound(drec);
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());

	drec->set_context_ear(io, SIG_IO_DREC, 1);
	tf20->set_context_sio(io, SIG_IO_ART);
	
	io->set_context_cpu(cpu);
	io->set_context_mem(memory, memory->get_ram());
	io->set_context_tf20(tf20);
	io->set_context_beep(beep);
	io->set_context_drec(drec);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(io);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// initialize all devices
#if defined(__GIT_REPO_VERSION)
	strncpy(_git_revision, __GIT_REPO_VERSION, sizeof(_git_revision) - 1);
#endif
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	decl_state();
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

void VM::special_reset()
{
	// system reset
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
	io->sysreset();
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
	beep->initialize_sound(rate, 1000, 8000);
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
		beep->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		drec->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 2) {
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
	io->key_down(code);
}

void VM::key_up(int code)
{
	io->key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	tf20->open_disk(drv, file_path, bank);
}

void VM::close_floppy_disk(int drv)
{
	tf20->close_disk(drv);
}

bool VM::is_floppy_disk_inserted(int drv)
{
	return tf20->is_disk_inserted(drv);
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
	tf20->is_disk_protected(drv, value);
}

bool VM::is_floppy_disk_protected(int drv)
{
	return tf20->is_disk_protected(drv);
}

uint32_t VM::is_floppy_disk_accessed()
{
	return tf20->read_signal(0);
}

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	drec->play_tape(file_path);
//	drec->set_remote(true);
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	drec->rec_tape(file_path);
//	drec->set_remote(true);
}

void VM::close_tape(int drv)
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
//	drec->set_remote(false);
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

void VM::push_play(int drv)
{
	drec->set_ff_rew(0);
	drec->set_remote(true);
}

void VM::push_stop(int drv)
{
	drec->set_remote(false);
}

void VM::push_fast_forward(int drv)
{
	drec->set_ff_rew(1);
	drec->set_remote(true);
}

void VM::push_fast_rewind(int drv)
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

#define STATE_VERSION	3

#include "../../statesub.h"
#include "../../qt/gui/csp_logger.h"
extern CSP_Logger DLL_PREFIX_I *csp_logger;

void VM::decl_state(void)
{
	state_entry = new csp_state_utils(STATE_VERSION, 0, (_TCHAR *)(_T("CSP::HC_40_HEAD")), csp_logger);
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->decl_state();
	}
}

void VM::save_state(FILEIO* state_fio)
{
	//state_fio->FputUint32(STATE_VERSION);
	
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
}

bool VM::load_state(FILEIO* state_fio)
{
	//if(state_fio->FgetUint32() != STATE_VERSION) {
	//	return false;
	//}
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		emu->out_debug_log("INFO: HEADER DATA ERROR");
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(!device->load_state(state_fio)) {
			return false;
		}
	}
	return true;
}

