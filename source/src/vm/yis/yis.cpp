/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2017.04.13-

	[ virtual machine ]
*/

#include "yis.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../m6502.h"
#include "../io.h"
#include "../memory.h"
#include "../am9511.h"
#include "../beep.h"
#include "../disk.h"
#include "../mb8877.h"
#include "../mc6820.h"
#include "../mc6844.h"
#include "../mc6850.h"
#include "../msm58321.h"
#include "../noise.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "calendar.h"
#include "display.h"
#include "floppy.h"
#include "keyboard.h"
#include "mapper.h"
#include "membus.h"
#include "sound.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	cpu = new M6502(this, emu);	// YM-2002
	io = new IO(this, emu);
	io->space = 0x10000;
	apu = new AM9511(this, emu);
	beep = new BEEP(this, emu);
	fdc = new MB8877(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	pia = new MC6820(this, emu);	// MB8874
	dma = new MC6844(this, emu);	// HD46504
	acia1 = new MC6850(this, emu);	// MB8863
	acia2 = new MC6850(this, emu);
	rtc = new MSM58321(this, emu);	// MSM5832RS
	
	calendar = new CALENDAR(this, emu);
	display = new DISPLAY(this, emu);
	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	mapper = new MAPPER(this, emu);
	memory = new MEMBUS(this, emu);
	memory->space = 0x10000;
	memory->bank_size = 0x100;
	sound = new SOUND(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(beep);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	
	fdc->set_context_drq(dma, SIG_MC6844_TX_RQ_0, 1);
	dma->set_context_memory(memory);
	dma->set_context_ch0(fdc);
	
	calendar->set_context_rtc(rtc);
	floppy->set_context_fdc(fdc);
	keyboard->set_context_cpu(cpu);
	mapper->set_context_memory(memory);
	sound->set_context_beep(beep);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// memory bus
	memory->read_bios(_T("BIOS.ROM"), rom, sizeof(rom));
	// $0000-$efff will be mapped in mapper class
	memory->set_memory_mapped_io_rw(0xf000, 0xf0ff, io);
	memory->set_memory_r(0xf100, 0xffff, rom + 0x100);
	
	// io bus
	io->set_iomap_range_rw(0xf000, 0xf016, dma);
	io->set_iomap_range_rw(0xf020, 0xf023, fdc);
	io->set_iomap_single_w(0xf024, floppy);
	io->set_iomap_single_rw(0xf030, keyboard);
	io->set_iomap_single_r(0xf031, keyboard);
	io->set_iomap_range_w(0xf031, 0xf032, sound);
	io->set_iomap_range_rw(0xf034, 0xf035, apu);
	io->set_iomap_range_rw(0xf036, 0xf037, calendar);
	io->set_iomap_range_rw(0xf038, 0xf03b, pia);
	io->set_iomap_range_rw(0xf03c, 0xf03d, acia1);
	io->set_iomap_range_rw(0xf03e, 0xf03f, acia2);
	io->set_iomap_range_rw(0xf040, 0xf04b, display);
	io->set_iomap_range_rw(0xf04f, 0xf05f, mapper);
//	io->set_iomap_range_rw(0xf060, 0xf06f, keyboard);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2DD); // 1DD
		fdc->set_track_size(i, 6238);
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
	display->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	beep->initialize_sound(rate, 49152 / 0x80, 8000);
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
		fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	if(!repeat) {
		keyboard->key_down(code);
	}
}

void VM::key_up(int code)
{
}

bool VM::get_caps_locked()
{
	return keyboard->get_caps_locked();
}

bool VM::get_kana_locked()
{
	return keyboard->get_kana_locked();
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	fdc->open_disk(drv, file_path, bank);
}

void VM::close_floppy_disk(int drv)
{
	fdc->close_disk(drv);
}

bool VM::is_floppy_disk_inserted(int drv)
{
	return fdc->is_disk_inserted(drv);
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
	fdc->is_disk_protected(drv, value);
}

bool VM::is_floppy_disk_protected(int drv)
{
	return fdc->is_disk_protected(drv);
}

uint32_t VM::is_floppy_disk_accessed()
{
	return fdc->read_signal(0);
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

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		const _TCHAR *name = char_to_tchar(typeid(*device).name() + 6); // skip "class "
		int len = (int)_tcslen(name);
		
		if(!state_fio->StateCheckInt32(len)) {
			return false;
		}
		if(!state_fio->StateCheckBuffer(name, len, 1)) {
			return false;
		}
		if(!device->process_state(state_fio, loading)) {
			return false;
		}
	}
	return true;
}

