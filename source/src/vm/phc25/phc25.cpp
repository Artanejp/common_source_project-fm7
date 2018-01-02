/*
	SANYO PHC-25 Emulator 'ePHC-25'
	SEIKO MAP-1010 Emulator 'eMAP-1010'

	Author : Takeda.Toshiya
	Date   : 2010.08.03-

	[ virtual machine ]
*/

#include "phc25.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../io.h"
#include "../mc6847.h"
#include "../noise.h"
#include "../not.h"
//#include "../ym2203.h"
#include "../ay_3_891x.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "joystick.h"
#include "keyboard.h"
#include "memory.h"
#include "system.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	dummy->set_device_name(_T("1st Dummy"));
	
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
	io = new IO(this, emu);
	vdp = new MC6847(this, emu);
	not_vsync = new NOT(this, emu);
//	psg = new YM2203(this, emu);
	psg = new AY_3_891X(this, emu);
	cpu = new Z80(this, emu);
	not_vsync->set_device_name(_T("NOT GATE(VSYNC)"));
	
	joystick = new JOYSTICK(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	system = new SYSTEM(this, emu);
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	event->set_context_sound(drec);
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	vdp->load_font_image(create_local_path(_T("FONT.ROM")));
	vdp->set_vram_ptr(memory->get_vram(), 0x1800);
//	vdp->set_context_cpu(cpu);
	vdp->set_context_vsync(not_vsync, SIG_NOT_INPUT, 1);
	not_vsync->set_context_out(cpu, SIG_CPU_IRQ, 1);
	
	vdp->set_context_vsync(system, SIG_SYSTEM_PORT, 0x10);
	drec->set_context_ear(system, SIG_SYSTEM_PORT, 0x20);
	// bit6: printer busy
	vdp->set_context_hsync(system, SIG_SYSTEM_PORT, 0x80);
	
	joystick->set_context_psg(psg);
#ifdef _MAP1010
	memory->set_context_keyboard(keyboard);
#endif
	system->set_context_drec(drec);
	system->set_context_vdp(vdp);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_single_rw(0x40, system);
#ifndef _MAP1010
	io->set_iomap_range_r(0x80, 0x88, keyboard);
#endif
	io->set_iomap_alias_w(0xc0, psg, 1);	// PSG data
	io->set_iomap_alias_w(0xc1, psg, 0);	// PSG ch
//	io->set_iomap_alias_r(0xc0, psg, 1);
	io->set_iomap_alias_r(0xc1, psg, 1);	// PSG data
	
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
	
	// init sound gen
	psg->initialize_sound(rate, 1996750, samples, 0, 0);
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
		psg->set_volume(1, decibel_l, decibel_r);
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

#define STATE_VERSION	4

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		const char *name = typeid(*device).name() + 6; // skip "class "
		
		state_fio->FputInt32(strlen(name));
		state_fio->Fwrite(name, strlen(name), 1);
		device->save_state(state_fio);
	}
}

bool VM::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		const char *name = typeid(*device).name() + 6; // skip "class "
		
		if(!(state_fio->FgetInt32() == strlen(name) && state_fio->Fcompare(name, strlen(name)))) {
			return false;
		}
		if(!device->load_state(state_fio)) {
			return false;
		}
	}
	return true;
}

