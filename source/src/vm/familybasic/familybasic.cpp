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

int VM::sound_buffer_ptr()
{
	return event->sound_buffer_ptr();
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::play_tape(_TCHAR* file_path)
{
	drec->play_tape(file_path);
	drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
}

void VM::rec_tape(_TCHAR* file_path)
{
	drec->rec_tape(file_path);
	drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
}

void VM::close_tape()
{
	drec->close_tape();
	drec->write_signal(SIG_DATAREC_REMOTE, 0, 1);
}

bool VM::tape_inserted()
{
	return drec->tape_inserted();
}

bool VM::now_skip()
{
	return event->now_skip();
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

