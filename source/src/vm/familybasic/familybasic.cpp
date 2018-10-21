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
#include "../noise.h"
#include "../ym2413.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "./memory.h"
#include "./apu.h"
#include "./ppu.h"

using FAMILYBASIC::MEMORY;
using FAMILYBASIC::APU;
using FAMILYBASIC::PPU;


// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// check configs
//	boot_mode = config.boot_mode;
	boot_mode = -1;
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
//	cpu = new M6502(this, emu);
	opll = new YM2413(this, emu);
	
	memory = new MEMORY(this, emu);
	apu = new APU(this, emu);
	ppu = new PPU(this, emu);
	cpu = new N2A03(this, emu); // cpu shoud be reset after other device
	
	dummy->set_device_name(_T("1st Dummy"));
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(apu);
	event->set_context_sound(drec);
	event->set_context_sound(opll);
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	memory->set_context_cpu(cpu);
	memory->set_context_apu(apu);
	memory->set_context_ppu(ppu);
	memory->set_context_drec(drec);
	memory->set_context_opll(opll);
	memory->set_spr_ram_ptr(ppu->get_spr_ram());
	apu->set_context_cpu(cpu);
	apu->set_context_memory(memory);
	ppu->set_context_cpu(cpu);
	ppu->set_context_memory(memory);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_intr(dummy);
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
		} else if(config.boot_mode == 2) {
			memory->load_rom_image(_T("PLAYBOX_BASIC.NES"));
			ppu->load_rom_image(_T("PLAYBOX_BASIC.NES"));
		} else if(config.boot_mode == 3) {
			memory->load_rom_image(_T("VRC7_BASIC_V2.NES"));
			ppu->load_rom_image(_T("VRC7_BASIC_V2.NES"));
		} else if(config.boot_mode == 4) {
			memory->load_rom_image(_T("VRC7_BASIC_V3.NES"));
			ppu->load_rom_image(_T("VRC7_BASIC_V3.NES"));
		} else if(config.boot_mode == 5) {
			memory->load_rom_image(_T("MMC5_BASIC_V3.NES"));
			ppu->load_rom_image(_T("MMC5_BASIC_V3.NES"));
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
	opll->initialize_sound(rate, 3579545, samples);
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
		apu->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		opll->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 2) {
		drec->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 3) {
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

#define STATE_VERSION	6

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
 	for(DEVICE* device = first_device; device; device = device->next_device) {
		// Note: typeid(foo).name is fixed by recent ABI.Not dec 6.
 		// const char *name = typeid(*device).name();
		//       But, using get_device_name() instead of typeid(foo).name() 20181008 K.O
		const char *name = device->get_device_name();
		int len = strlen(name);
		
		if(!state_fio->StateCheckInt32(len)) {
			if(loading) {
				printf("Class name len Error: DEVID=%d EXPECT=%s\n", device->this_device_id, name);
			}
			return false;
		}
		if(!state_fio->StateCheckBuffer(name, len, 1)) {
			if(loading) {
				printf("Class name Error: DEVID=%d EXPECT=%s\n", device->this_device_id, name);
			}
 			return false;
 		}
		if(!device->process_state(state_fio, loading)) {
			if(loading) {
				printf("Data loading Error: DEVID=%d\n", device->this_device_id);
			}
 			return false;
 		}
 	}
	// Machine specified.
	state_fio->StateInt32(boot_mode);
	if(loading) {
		update_config();
	}
 	return true;
}

