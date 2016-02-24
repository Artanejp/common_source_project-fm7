/*
	Nintendo Family BASIC Emulator 'eFamilyBASIC'

	Origin : nester
	Author : Takeda.Toshiya
	Date   : 2010.08.11-

	[ virtual machine ]
*/

#include "familybasic.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../m6502.h"

#include "memory.h"
#include "apu.h"
#include "ppu.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// check configs
//	boot_mode = config.boot_mode;
	boot_mode = -1;
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	cpu = new M6502(this, emu);
	
	memory = new MEMORY(this, emu);
	apu = new APU(this, emu);
	ppu = new PPU(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(apu);
	event->set_context_sound(drec);
	
	memory->set_context_cpu(cpu);
	memory->set_context_apu(apu);
	memory->set_context_ppu(ppu);
	memory->set_context_drec(drec);
	memory->set_spr_ram_ptr(ppu->get_spr_ram());
	apu->set_context_cpu(cpu);
	apu->set_context_memory(memory);
	ppu->set_context_cpu(cpu);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_intr(dummy);
	
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
	// load basic rom
	if(boot_mode != config.boot_mode) {
		if(boot_mode != -1) {
			memory->save_backup();
		}
		if(config.boot_mode == 0) {
			memory->load_rom_image(_T("BASIC_V2.NES"));
			ppu->load_rom_image(_T("BASIC_V2.NES"));
		} else if(config.boot_mode == 1) {
			memory->load_rom_image(_T("BASIC_V3.NES"));
			ppu->load_rom_image(_T("BASIC_V3.NES"));
		} else {
			memory->load_rom_image(_T("PLAYBOX_BASIC.NES"));
			ppu->load_rom_image(_T("PLAYBOX_BASIC.NES"));
		}
		boot_mode = config.boot_mode;
	}
	
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
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	ppu->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	apu->initialize_sound(rate, samples);
}

uint16* VM::create_sound(int* extra_frames)
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
		apu->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		drec->set_volume(0, decibel_l, decibel_r);
	}
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
	drec->close_tape();
	drec->write_signal(SIG_DATAREC_REMOTE, 0, 1);
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
	if(boot_mode != config.boot_mode) {
		// boot mode is changed !!!
//		boot_mode = config.boot_mode;
		reset();
	} else {
		for(DEVICE* device = first_device; device; device = device->next_device) {
			device->update_config();
		}
	}
}

#define STATE_VERSION	1

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
	state_fio->FputInt32(boot_mode);
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
	boot_mode = state_fio->FgetInt32();
	return true;
}

