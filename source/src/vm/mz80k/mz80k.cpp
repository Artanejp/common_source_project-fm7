/*
	SHARP MZ-80K/C Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	SHARP MZ-80A Emulator 'EmuZ-80A'
	Modify : Hideki Suga
	Date   : 2014.12.10 -

	[ virtual machine ]
*/

#include "mz80k.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#if defined(_MZ1200) || defined(_MZ80A)
#include "../and.h"
#endif
#include "../datarec.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../ls393.h"
#include "../mz1p17.h"
#include "../noise.h"
#include "../pcm1bit.h"
#include "../prnfile.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "keyboard.h"
#include "memory.h"
#include "printer.h"

#if defined(SUPPORT_MZ80AIF)
#include "../io.h"
#include "../disk.h"
#include "../mb8877.h"
#include "mz80aif.h"
#elif defined(SUPPORT_MZ80FIO)
#include "../io.h"
#include "../disk.h"
#include "../t3444a.h"
#include "mz80fio.h"
#endif

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
#if defined(_MZ1200) || defined(_MZ80A)
	and_int = new AND(this, emu);
#endif
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
	ctc = new I8253(this, emu);
	pio = new I8255(this, emu);
	counter = new LS393(this, emu);
	pcm = new PCM1BIT(this, emu);
	cpu = new Z80(this, emu);
	
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	printer = new PRINTER(this, emu);
	
#if defined(SUPPORT_MZ80AIF)
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);	// mb8866
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	mz80aif = new MZ80AIF(this, emu);
#elif defined(SUPPORT_MZ80FIO)
	io = new IO(this, emu);
	fdc = new T3444A(this, emu);	// t3444m
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	mz80fio = new MZ80FIO(this, emu);
#endif
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	event->set_context_sound(drec);
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
#endif
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
#if defined(_MZ1200) || defined(_MZ80A)
	and_int->set_context_out(cpu, SIG_CPU_IRQ, 1);
	and_int->set_mask(SIG_AND_BIT_0 | SIG_AND_BIT_1);
#endif
	drec->set_context_ear(pio, SIG_I8255_PORT_C, 0x20);
	drec->set_context_remote(pio, SIG_I8255_PORT_C, 0x10);
	ctc->set_context_ch0(counter, SIG_LS393_CLK, 1);
	ctc->set_context_ch1(ctc, SIG_I8253_CLOCK_2, 1);
#if defined(_MZ1200) || defined(_MZ80A)
	ctc->set_context_ch2(and_int, SIG_AND_BIT_0, 1);
#else
	ctc->set_context_ch2(cpu, SIG_CPU_IRQ, 1);
#endif
	ctc->set_constant_clock(0, 2000000);
	ctc->set_constant_clock(1, 31250);
	pio->set_context_port_a(keyboard, SIG_KEYBOARD_COLUMN, 0x0f, 0);
	pio->set_context_port_c(memory, SIG_MEMORY_VGATE, 1, 0);
	pio->set_context_port_c(drec, SIG_DATAREC_MIC, 2, 0);
#if defined(_MZ1200) || defined(_MZ80A)
	pio->set_context_port_c(and_int, SIG_AND_BIT_1, 4, 0);
#endif
	pio->set_context_port_c(drec, SIG_DATAREC_TRIG, 8, 0);
	counter->set_context_1qa(pcm, SIG_PCM1BIT_SIGNAL, 1);
	
	keyboard->set_context_pio(pio);
	memory->set_context_ctc(ctc);
	memory->set_context_pio(pio);
	
#if defined(SUPPORT_MZ80AIF)
	fdc->set_context_irq(memory, SIG_MEMORY_FDC_IRQ, 1);
	fdc->set_context_drq(memory, SIG_MEMORY_FDC_DRQ, 1);
	mz80aif->set_context_fdc(fdc);
#elif defined(SUPPORT_MZ80FIO)
	mz80fio->set_context_fdc(fdc);
#endif
	
	if(config.printer_type == 0) {  
		printer->set_context_prn(new PRNFILE(this, emu));
	} else if(config.printer_type == 1) {
		MZ1P17 *mz1p17 = new MZ1P17(this, emu);
		mz1p17->mode = MZ1P17_MODE_MZ80P4;
		printer->set_context_prn(mz1p17);
	} else {
		printer->set_context_prn(dummy);
	}
	
	// cpu bus
	cpu->set_context_mem(memory);
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
	cpu->set_context_io(io);
#else
	cpu->set_context_io(dummy);
#endif
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
#if defined(SUPPORT_MZ80AIF)
	io->set_iomap_range_rw(0xd8, 0xdb, fdc);
	io->set_iomap_range_w(0xdc, 0xdd, mz80aif);
#elif defined(SUPPORT_MZ80FIO)
	io->set_iomap_range_rw(0xf8, 0xfb, mz80fio);
#endif
	io->set_iomap_range_rw(0xfe, 0xff, printer);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
#if defined(SUPPORT_MZ80AIF)
	for(int drv = 0; drv < MAX_DRIVE; drv++) {
//		if(config.drive_type) {
			fdc->set_drive_type(drv, DRIVE_TYPE_2DD);
//		} else {
//			fdc->set_drive_type(drv, DRIVE_TYPE_2D);
//		}
	}
#elif defined(SUPPORT_MZ80FIO)
	for(int drv = 0; drv < MAX_DRIVE; drv++) {
		fdc->set_drive_type(drv, DRIVE_TYPE_2D);
//		fdc->set_drive_mfm(drv, false);
	}
#endif
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
#if defined(_MZ1200) || defined(_MZ80A)
	and_int->write_signal(SIG_AND_BIT_0, 0, 1);	// CLOCK = L
	and_int->write_signal(SIG_AND_BIT_1, 1, 1);	// INTMASK = H
#endif
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
	if(ch-- == 0) {
		pcm->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		drec->set_volume(0, decibel_l, decibel_r);
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
	} else if(ch-- == 0) {
		fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
#endif
	} else if(ch-- == 0) {
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
	if(!repeat) {
		keyboard->key_down(code);
	}
}

void VM::key_up(int code)
{
//	keyboard->key_up(code);
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

#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	fdc->open_disk(drv, file_path, bank);
	
#if defined(SUPPORT_MZ80AIF)
	if(fdc->get_media_type(drv) == MEDIA_TYPE_2DD) {
		if(fdc->get_drive_type(drv) == DRIVE_TYPE_2D) {
			fdc->set_drive_type(drv, DRIVE_TYPE_2DD);
		}
	} else if(fdc->get_media_type(drv) == MEDIA_TYPE_2D) {
		if(fdc->get_drive_type(drv) == DRIVE_TYPE_2DD) {
			fdc->set_drive_type(drv, DRIVE_TYPE_2D);
		}
	}
#endif
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
#endif


void VM::play_tape(int drv, const _TCHAR* file_path)
{
	bool remote = drec->get_remote();
	
	if(drec->play_tape(file_path) && remote) {
		// if machine already sets remote on, start playing now
		push_play(drv);
	}
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	bool remote = drec->get_remote();
	
	if(drec->rec_tape(file_path) && remote) {
		// if machine already sets remote on, start recording now
		push_play(drv);
	}
}

void VM::close_tape(int drv)
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
	drec->set_remote(false);
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
	drec->set_remote(false);
	drec->set_ff_rew(0);
	drec->set_remote(true);
}

void VM::push_stop(int drv)
{
	drec->set_remote(false);
}

void VM::push_fast_forward(int drv)
{
	drec->set_remote(false);
	drec->set_ff_rew(1);
	drec->set_remote(true);
}

void VM::push_fast_rewind(int drv)
{
	drec->set_remote(false);
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

#define STATE_VERSION	7

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		const char *name = typeid(*device).name() + 6; // skip "class "
		int len = (int)strlen(name);
		
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

