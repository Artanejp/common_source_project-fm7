/*
	Systems Formulate BUBCOM80 Emulator 'eBUBCOM80'

	Author : Takeda.Toshiya
	Date   : 2018.05.08-

	[ virtual machine ]
*/

#include "bubcom80.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../disk.h"
#include "../io.h"
#include "../ls393.h"
#include "../mb8877.h"
#include "../mc6850.h"
#include "../noise.h"
#include "../pcm1bit.h"
#include "../prnfile.h"
#include "../z80.h"
#include "../z80ctc.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "bubblecasette.h"
#include "cmt.h"
#include "display.h"
#include "floppy.h"
#include "keyboard.h"
#include "membus.h"
#include "rtc.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

using BUBCOM80::BUBBLECASETTE;
using BUBCOM80::CMT;
using BUBCOM80::DISPLAY;
using BUBCOM80::FLOPPY;
using BUBCOM80::KEYBOARD;
using BUBCOM80::MEMBUS;
using BUBCOM80::RTC;

VM::VM(EMU_TEMPLATE* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	//first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device

	io = new IO(this, emu);
	io->space = 0x10000;
	flipflop = new LS393(this, emu);
	fdc = new MB8877(this, emu);
#ifdef USE_DEBUGGER
	//fdc->set_context_debugger(new DEBUGGER(this, emu));
#endif
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
//	sio_rs = new MC6850(this, emu);
	sio_cmt = new MC6850(this, emu);
//	sio_key = new MC6850(this, emu);
	pcm = new PCM1BIT(this, emu);
#ifdef USE_DEBUGGER
//	pcm->set_context_debugger(new DEBUGGER(this, emu));
#endif
	cpu = new Z80(this, emu);
	ctc = new Z80CTC(this, emu);

	bubblecasette[0] = new BUBBLECASETTE(this, emu);
	bubblecasette[1] = new BUBBLECASETTE(this, emu);
	cmt = new CMT(this, emu);
	if(config.printer_type == 0) {
		printer = new PRNFILE(this, emu);
//	} else if(config.printer_type == 1) {
//		printer = new BC861(this, emu);
	} else {
		printer = dummy;
	}
	display = new DISPLAY(this, emu);
	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	membus = new MEMBUS(this, emu);
	membus->space = 0x10000;
	membus->bank_size = 0x800;

	rtc = new RTC(this, emu);

	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());

	flipflop->set_context_1qa(pcm, SIG_PCM1BIT_SIGNAL, 1);
	fdc->set_context_drq(display, SIG_DISPLAY_DMAC_CH0, 1);
	sio_cmt->set_context_out(cmt, SIG_CMT_OUT);
	ctc->set_context_zc0(flipflop, SIG_LS393_CLK, 1);
	ctc->set_constant_clock(0, CPU_CLOCKS);
	ctc->set_constant_clock(1, CPU_CLOCKS);
	ctc->set_constant_clock(2, CPU_CLOCKS);

	cmt->set_context_sio(sio_cmt);
	display->set_context_cpu(cpu);
	display->set_context_cmt(cmt);
	display->set_context_pcm(pcm);
	display->set_context_prn(printer);
	display->set_context_dmac_mem(membus);
	display->set_context_dmac_ch0(fdc);
	display->set_context_dmac_ch2(display); // crtc
	floppy->set_context_fdc(floppy);

	// cpu bus
	cpu->set_context_mem(membus);
	cpu->set_context_io(io);
	cpu->set_context_intr(ctc);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif

	// z80 family daisy chain
	ctc->set_context_intr(cpu, 0);

	// i/o bus
	io->set_iomap_range_rw(0x0000, 0x0008, bubblecasette[0]);
	io->set_iomap_range_rw(0x000c, 0x000d, membus);
//	io->set_iomap_range_rw(0x0010, 0x0011, display);
	io->set_iomap_range_rw(0x0010, 0x001f, display); // not full-decoded ?
	io->set_iomap_single_rw(0x0020, display);
	io->set_iomap_range_rw(0x0030, 0x0031, sio_cmt);
	io->set_iomap_range_rw(0x0040, 0x0043, ctc);
	io->set_iomap_single_rw(0x0050, display);
//	io->set_iomap_range_rw(0x0060, 0x0068, display);
	io->set_iomap_range_rw(0x0060, 0x006f, display); // not full-decoded ?
	io->set_iomap_single_w(0x0080, membus);
	io->set_iomap_range_r(0x0400, 0x047f, keyboard);
	io->set_iomap_range_rw(0x0800, 0x0fff, display);
	io->set_iomap_range_rw(0x3fd0, 0x3fd3, fdc);
	io->set_iomap_single_w(0x3fd8, floppy);
	io->set_iomap_range_rw(0x3fe0, 0x3fed, rtc);
	io->set_iomap_single_rw(0x3ff0, display);
	io->set_iomap_range_rw(0x4000, 0xffff, display);

	// initialize all devices
#if defined(__GIT_REPO_VERSION)
	set_git_repo_version(__GIT_REPO_VERSION);
#endif
	initialize_devices();

	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2HD); // 8inch 2D
	}
	fdc->write_signal(SIG_MB8877_MOTOR, 1, 1);
}

VM::~VM()
{
	// delete all devices
	release_devices();
}


// ----------------------------------------------------------------------------
// drive virtual machine
// ----------------------------------------------------------------------------

void VM::reset()
{
	// reset all devices
	VM_TEMPLATE::reset();

	ctc->write_io8(0, 0x07); // default frequency for beep
	ctc->write_io8(0, 0xef);
	pcm->write_signal(SIG_PCM1BIT_ON, 0, 0); // beep off
}

void VM::run()
{
	if(event != nullptr) {
		event->drive();
	}
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
	return NULL;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	if(display != nullptr) {
		display->draw_screen();
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

	// init sound gen
	if(pcm != nullptr) {
		pcm->initialize_sound(rate, 8000);
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
		if(pcm != nullptr) {
			pcm->set_volume(0, decibel_l, decibel_r);
		}
	} else if(ch == 1) {
		if(fdc != nullptr) {
			fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
			fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
			fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
		}
	}
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(fdc != nullptr) {
		fdc->open_disk(drv, file_path, bank);
	}
}

void VM::close_floppy_disk(int drv)
{
	if(fdc != nullptr) {
		fdc->close_disk(drv);
	}
}

bool VM::is_floppy_disk_inserted(int drv)
{
	if(fdc != nullptr) {
		return fdc->is_disk_inserted(drv);
	}
	return false;
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
	if(fdc != nullptr) {
		fdc->is_disk_protected(drv, value);
	}
}

bool VM::is_floppy_disk_protected(int drv)
{
	if(fdc != nullptr) {
		return fdc->is_disk_protected(drv);
	}
	return false;
}

uint32_t VM::is_floppy_disk_accessed()
{
	if(fdc != nullptr) {
		return fdc->read_signal(0);
	}
	return 0;
}

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	if(cmt == nullptr) return;
	cmt->play_tape(file_path);
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	if(cmt == nullptr) return;
	cmt->rec_tape(file_path);
}

void VM::close_tape(int drv)
{
	if(cmt == nullptr) return;
	cmt->close_tape();
}

bool VM::is_tape_inserted(int drv)
{
	if(cmt != nullptr) {
		return cmt->is_tape_inserted();
	}
	return false;
}

void VM::open_bubble_casette(int drv, const _TCHAR *path, int bank)
{
	if(drv < 2 && bubblecasette[drv] != NULL) {
		bubblecasette[drv]->open((_TCHAR *)path, bank);
	}
}

void VM::close_bubble_casette(int drv)
{
	if(drv < 2 && bubblecasette[drv] != NULL) {
		bubblecasette[drv]->close();
	}
}

bool VM::is_bubble_casette_inserted(int drv)
{
	if(drv < 2 && bubblecasette[drv] != NULL) {
		return bubblecasette[drv]->is_bubble_inserted();
	}
	return false;
}

bool VM::is_bubble_casette_protected(int drv)
{
	if(drv < 2 && bubblecasette[drv] != NULL) {
		return bubblecasette[drv]->is_bubble_protected();
	}
	return false;
}

void VM::is_bubble_casette_protected(int drv, bool flag)
{
	if(drv < 2 && bubblecasette[drv] != NULL) {
		bubblecasette[drv]->set_bubble_protect(flag);
	}
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

#define STATE_VERSION	1

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!(VM_TEMPLATE::process_state_core(state_fio, loading, STATE_VERSION))) {
		return false;
	}
	// Machine specified.
	if(loading) {
		update_config();
	}
	//mainio->restore_opn();
 	return true;
}
