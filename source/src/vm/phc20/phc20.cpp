/*
	SANYO PHC-20 Emulator 'ePHC-20'

	Author : Takeda.Toshiya
	Date   : 2010.09.03-

	[ virtual machine ]
*/

#include "phc20.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../mc6847.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "memory.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
#if defined(_USE_QT)
	dummy->set_device_name(_T("1st Dummy"));
#endif	
	drec = new DATAREC(this, emu);
	vdp = new MC6847(this, emu);
	cpu = new Z80(this, emu);
#if defined(_USE_QT)
	cpu->set_device_name(_T("CPU(Z80)"));
#endif	
	memory = new MEMORY(this, emu);
#if defined(_USE_QT)
	memory->set_device_name(_T("MEMORY"));
#endif
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(drec);
#if defined(USE_SOUND_FILES)
	drec->load_sound_data(DATAREC_SNDFILE_RELAY_ON,  _T("RELAY_ON.WAV"));
	drec->load_sound_data(DATAREC_SNDFILE_RELAY_OFF, _T("RELAYOFF.WAV"));
#endif
	
	drec->set_context_ear(memory, SIG_MEMORY_SYSPORT, 1);
	vdp->set_context_vsync(memory, SIG_MEMORY_SYSPORT, 2);	// vsync / hsync?
	
	vdp->load_font_image(create_local_path(_T("FONT.ROM")));
	vdp->set_vram_ptr(memory->get_vram(), 0x400);
	vdp->set_context_cpu(cpu);
	vdp->write_signal(SIG_MC6847_AG, 0, 0);		// force character mode
	vdp->write_signal(SIG_MC6847_AS, 0, 0);
	vdp->write_signal(SIG_MC6847_INTEXT, 0, 0);	// force internal character ???
	vdp->write_signal(SIG_MC6847_CSS, 0, 0);
	
	memory->set_context_drec(drec);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(dummy);
	cpu->set_context_intr(dummy);
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
	vdp->draw_screen();
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
	}
#if defined(USE_SOUND_FILES)
	else if(ch == 1) {
		 for(int i = 0; i < DATAREC_SNDFILE_END; i++) {
			 drec->set_volume(i + 2, decibel_l, decibel_r);
		 }
	 }
#endif
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::play_tape(const _TCHAR* file_path)
{
	drec->play_tape(file_path);
	drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
}

void VM::rec_tape(const _TCHAR* file_path)
{
	drec->rec_tape(file_path);
	drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
}

void VM::close_tape()
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
	drec->write_signal(SIG_DATAREC_REMOTE, 0, 0);
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
	return true;
}

