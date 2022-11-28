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

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	io = new IO(this, emu);
	flipflop = new LS393(this, emu);
	fdc = new MB8877(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
//	sio_rs = new MC6850(this, emu);
	sio_cmt = new MC6850(this, emu);
//	sio_key = new MC6850(this, emu);
	pcm = new PCM1BIT(this, emu);
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
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2HD); // 8inch 2D
	}
	fdc->write_signal(SIG_MB8877_MOTOR, 1, 1);
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
	ctc->write_io8(0, 0x07); // default frequency for beep
	ctc->write_io8(0, 0xef);
	pcm->write_signal(SIG_PCM1BIT_ON, 0, 0); // beep off
}

void VM::run()
{
	event->drive();
}

double VM::get_frame_rate()
{
	return event->get_frame_rate();
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
	if(ch == 0) {
		pcm->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

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

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	cmt->play_tape(file_path);
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	cmt->rec_tape(file_path);
}

void VM::close_tape(int drv)
{
	cmt->close_tape();
}

bool VM::is_tape_inserted(int drv)
{
	return cmt->is_tape_inserted();
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
	return event->is_frame_skippable();
}

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

#define STATE_VERSION	1

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

