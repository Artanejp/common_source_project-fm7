/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

	[ virtual machine ]
*/

#include "j3100.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../hd46505.h"
#include "../i8237.h"
//#include "../i8250.h"
#include "../i8253.h"
#include "../i8259.h"
#include "../io.h"
#include "../noise.h"
#include "../pcm1bit.h"
#include "../upd765a.h"
#ifdef TYPE_SL
#include "../i86.h"
#include "../rp5c01.h"
#else
#include "../i286.h"
#include "../hd146818p.h"
#endif

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
#include "dmareg.h"
#include "floppy.h"
#include "sasi.h"
#ifdef TYPE_SL
#include "slkeyboard.h"
#include "slmemory.h"
#include "slsystem.h"
#else
#include "keyboard.h"
#include "memory.h"
#include "system.h"
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
	
	crtc = new HD46505(this, emu);
	dma = new I8237(this, emu);
#ifdef USE_DEBUGGER
	dma->set_context_debugger(new DEBUGGER(this, emu));
#endif
#ifndef TYPE_SL
	dma2 = new I8237(this, emu);
#ifdef USE_DEBUGGER
	dma2->set_context_debugger(new DEBUGGER(this, emu));
#endif
#endif
//	sio = new I8250(this, emu);
	pit = new I8253(this, emu);
#if !(defined(_J3100SS) || defined(_J3100SE))
	pit->device_model = INTEL_8254;
	pit->set_device_name(_T("8254 PIT"));
#endif
	pic = new I8259(this, emu);
#ifndef TYPE_SL
	pic->num_chips = 2;
#endif
#ifdef TYPE_SL
	cpu = new I86(this, emu);
	cpu->device_model = INTEL_8086;
#else
	cpu = new I286(this, emu);
#endif
	io = new IO(this, emu);
	io->space = 0x10000;
	io->bus_width = 16;
	pcm = new PCM1BIT(this, emu);
	fdc = new UPD765A(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
#ifdef TYPE_SL
	rtc = new RP5C01(this, emu);
#else
	rtc = new HD146818P(this, emu);
#endif
	
	display = new DISPLAY(this, emu);
	dmareg = new DMAREG(this, emu);
	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	sasi = new SASI(this, emu);
	system = new SYSTEM(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// dmac
	io->set_iomap_range_rw(0x00, 0x0f, dma);
	io->set_iomap_range_w(0x80, 0x8f, dmareg);
	dma->set_context_cpu(cpu);
	dma->set_context_memory(memory);
	dma->set_context_ch2(fdc);
	dmareg->set_context_dma(dma);
#ifndef TYPE_SL
	for(int i = 0xc0; i <= 0xde; i += 2) {
		io->set_iomap_alias_rw(i, dma2, i >> 1);
	}
	dma2->set_context_memory(memory);
	dma2->set_mode_word(true);
	dmareg->set_context_dma2(dma2);
#endif
	
	// pic
	io->set_iomap_alias_rw(0x20, pic, I8259_ADDR_CHIP0 | 0);
	io->set_iomap_alias_rw(0x21, pic, I8259_ADDR_CHIP0 | 1);
#ifndef TYPE_SL
	io->set_iomap_alias_rw(0xa0, pic, I8259_ADDR_CHIP1 | 0);
	io->set_iomap_alias_rw(0xa1, pic, I8259_ADDR_CHIP1 | 1);
#endif
	pic->set_context_cpu(cpu);
	
	// pit
	io->set_iomap_range_rw(0x40, 0x43, pit);
	pit->set_constant_clock(0, 1190000);
	pit->set_constant_clock(1, 1190000);
	pit->set_constant_clock(2, 1190000);
	pit->set_context_ch0(pic, SIG_I8259_IR0 | SIG_I8259_CHIP0, 1);	// to PIC#0 IR0
	pit->set_context_ch2(pcm, SIG_PCM1BIT_SIGNAL, 1);
#ifndef TYPE_SL
	pit->set_context_ch2(system, SIG_SYSTEM_TC2O, 1);
#endif
	
	// system status/command
#ifndef TYPE_SL
	io->set_iomap_single_rw(0x61, system);
	system->set_context_pcm(pcm);
	system->set_context_pit(pit);
#endif
	
	// rtc
#ifdef TYPE_SL
	io->set_iomap_range_rw(0x2c0, 0x2cf, rtc);
#else
	io->set_iomap_alias_w(0x70, rtc, 1);	// bit7 = nmi mask
	io->set_iomap_alias_rw(0x71, rtc, 0);
	rtc->set_context_intr(pic, SIG_I8259_IR0 | SIG_I8259_CHIP1, 1);	// to PIC#1 IR0 (IR8)
#endif
	
	// nmi mask register
#ifdef TYPE_SL
	io->set_iomap_single_w(0xa0, system);
#else
	// 0x70 bit7 (not implemented)
#endif
	
	// crtc
	io->set_iomap_range_rw(0x3d0, 0x3d1, crtc);
	io->set_iomap_range_rw(0x3d4, 0x3d5, crtc);
	io->set_iomap_range_w(0x3d8, 0x3d9, display);
	io->set_iomap_single_r(0x3da, display);
	crtc->set_context_disp(display, SIG_DISPLAY_ENABLE, 1);
	crtc->set_context_vblank(display, SIG_DISPLAY_VBLANK, 1);
	display->set_regs_ptr(crtc->get_regs());
	display->set_vram_ptr(memory->get_vram());
	
	// fdc
	io->set_iomap_single_w(0x3f2, floppy);
	io->set_iomap_range_rw(0x3f4, 0x3f5, fdc);
	io->set_iomap_single_rw(0x3f7, floppy);
	fdc->set_context_irq(pic, SIG_I8259_IR6 | SIG_I8259_CHIP0, 1);	// to PIC#0 IR6
	fdc->set_context_drq(dma, SIG_I8237_CH2, 1);			// to DMA Ch.2
	floppy->set_context_fdc(fdc);
	
	// printer
	io->set_flipflop_single_rw(0x378, 0x00);
	io->set_iovalue_single_r(0x379, 0x78);
	io->set_flipflop_single_rw(0x37a, 0x00);
	
	// keyboard
#ifdef TYPE_SL
	io->set_iomap_range_rw(0x60, 0x61, keyboard);
#else
#endif
	keyboard->set_context_pic(pic);
	
	// serial port
	
	// sasi
#if defined(_J3100GT) || defined(TYPE_SL)
	io->set_iomap_range_rw(0x1f0, 0x1f3, sasi);
	sasi->set_context_pic(pic);
#endif
	
	// ems
	static const int ems_addr[] = {
		0x208, 0x4208, 0x8208, 0xc208,
		0x218, 0x4218, 0x8218, 0xc218,
		0x258, 0x4258, 0x8258, 0xc258,
		0x268, 0x4268, 0x8268, 0xc268,
#ifdef TYPE_SL
		0x2a8, 0x42a8, 0x82a8, 0xc2a8,
		0x2b8, 0x42b8, 0x82b8, 0xc2b8,
		0x2e8, 0x42e8, 0x82e8, 0xc2e8,
#endif
	};
	for(int i = 0; i < array_length(ems_addr); i++) {
		io->set_iomap_single_rw(ems_addr[i], memory);
#ifdef TYPE_SL
		io->set_iomap_single_w(ems_addr[i] + 1, memory);
#endif
	}
#ifdef TYPE_SL
	io->set_iomap_range_rw(0xee, 0xef, memory);
#endif
	
	// special registers for J-3100SL/SS/SE
#ifdef TYPE_SL
//	62	bit0
//		bit1	1=8087‚ ‚è
	io->set_iovalue_single_r(0x62, 0x26);	// unknown
//	io->set_flipflop_single_rw(0x63, 0x00);	// unknown
	static const int iovalues[0x20] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x14, 0xff, 0xff, 0x00, 0xff, 0xe0, 0xff, 0x00, 0xff, 0xff,
		0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	};
	for(int i = 0xe0; i <= 0xff; i++) {
		if(i == 0xee && i == 0xef) {
			continue;
		}
		io->set_flipflop_single_rw(i, iovalues[i & 0x1f]);
	}
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
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	keyboard->key_down(code);
}

void VM::key_up(int code)
{
	keyboard->key_up(code);
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

