/*
	SHARP MZ-3500 Emulator 'EmuZ-3500'

	Author : Takeda.Toshiya
	Date   : 2010.08.31-

	[ virtual machine ]
*/

#include "mz3500.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../disk.h"
#include "../i8251.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../io.h"
#include "../pcm1bit.h"
#include "../upd1990a.h"
#include "../upd7220.h"
#include "../upd765a.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "main.h"
#include "sub.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	// for main cpu
	io = new IO(this, emu);
	fdc = new UPD765A(this, emu);
	cpu = new Z80(this, emu);
	main = new MAIN(this, emu);
	
	// for sub cpu
	sio = new I8251(this, emu);
	pit = new I8253(this, emu);
	pio = new I8255(this, emu);
	subio = new IO(this, emu);
	pcm = new PCM1BIT(this, emu);
	rtc = new UPD1990A(this, emu);
	gdc_chr = new UPD7220(this, emu);
	gdc_gfx = new UPD7220(this, emu);
	subcpu = new Z80(this, emu);
	sub = new SUB(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_cpu(subcpu);
	event->set_context_sound(pcm);
	
	// mz3500sm p.59
	fdc->set_context_irq(main, SIG_MAIN_INTFD, 1);
	fdc->set_context_drq(main, SIG_MAIN_DRQ, 1);
	fdc->set_context_index(main, SIG_MAIN_INDEX, 1);
	
	// mz3500sm p.72,77
	sio->set_context_rxrdy(subcpu, SIG_CPU_NMI, 1);
	
	// mz3500sm p.77
	// i8253 ch.0 -> i8251 txc,rxc
	pit->set_context_ch1(pcm, SIG_PCM1BIT_SIGNAL, 1);
	pit->set_context_ch2(pit, SIG_I8253_GATE_1, 1);
	pit->set_constant_clock(0, 2450000);
	pit->set_constant_clock(1, 2450000);
	pit->set_constant_clock(2, 2450000);
	
	// mz3500sm p.78,80,81
	// i8255 pa0-pa7 -> printer data
	pio->set_context_port_b(rtc, SIG_UPD1990A_STB, 0x01, 0);
	pio->set_context_port_b(rtc, SIG_UPD1990A_C0,  0x02, 0);
	pio->set_context_port_b(rtc, SIG_UPD1990A_C1,  0x04, 0);
	pio->set_context_port_b(rtc, SIG_UPD1990A_C2,  0x08, 0);
	pio->set_context_port_b(rtc, SIG_UPD1990A_DIN, 0x10, 0);
	pio->set_context_port_b(rtc, SIG_UPD1990A_CLK, 0x20, 0);
	pio->set_context_port_b(main, SIG_MAIN_SRDY, 0x40, 0);
	pio->set_context_port_b(sub, SIG_SUB_PIO_PM, 0x80, 0);	// P/M: CG Selection
	pio->set_context_port_c(sub, SIG_SUB_KEYBOARD_DC, 0x01, 0);
	pio->set_context_port_c(sub, SIG_SUB_KEYBOARD_STC, 0x02, 0);
	pio->set_context_port_c(sub, SIG_SUB_KEYBOARD_ACKC, 0x04, 0);
	// i8255 pc3 <- intr (not use ???)
	pio->set_context_port_c(pcm, SIG_PCM1BIT_MUTE, 0x10, 0);
	// i8255 pc5 -> printer strobe
	// i8255 pc6 <- printer ack
	pio->set_context_port_c(sub, SIG_SUB_PIO_OBF, 0x80, 0);
	
	// mz3500sm p.80,81
	rtc->set_context_dout(sub, SIG_SUB_RTC_DOUT, 1);
	
	gdc_chr->set_vram_ptr(sub->get_vram_chr(), 0x1000);
	sub->set_sync_ptr_chr(gdc_chr->get_sync());
	sub->set_ra_ptr_chr(gdc_chr->get_ra());
	sub->set_cs_ptr_chr(gdc_chr->get_cs());
	sub->set_ead_ptr_chr(gdc_chr->get_ead());
	
	gdc_gfx->set_vram_ptr(sub->get_vram_gfx(), 0x20000);
	sub->set_sync_ptr_gfx(gdc_gfx->get_sync());
	sub->set_ra_ptr_gfx(gdc_gfx->get_ra());
	sub->set_cs_ptr_gfx(gdc_gfx->get_cs());
	sub->set_ead_ptr_gfx(gdc_gfx->get_ead());
	
	// mz3500sm p.23
	subcpu->set_context_busack(main, SIG_MAIN_SACK, 1);
	
	main->set_context_cpu(cpu);
	main->set_context_subcpu(subcpu);
	main->set_context_fdc(fdc);
	
	sub->set_context_main(main);
	sub->set_ipl(main->get_ipl());
	sub->set_common(main->get_common());
	
	// mz3500sm p.17
	io->set_iomap_range_rw(0xec, 0xef, main);	// reset int0
	io->set_iomap_range_rw(0xf4, 0xf7, fdc);	// fdc: f4h,f6h = status, f5h,f7h = data
	io->set_iomap_range_rw(0xf8, 0xfb, main);	// mfd interface
	io->set_iomap_range_rw(0xfc, 0xff, main);	// memory mpaper
	
	// mz3500sm p.18
	subio->set_iomap_range_rw(0x00, 0x0f, sub);	// int0 to main (set flipflop)
	subio->set_iomap_range_rw(0x10, 0x1f, sio);
	subio->set_iomap_range_rw(0x20, 0x2f, pit);
	subio->set_iomap_range_rw(0x30, 0x3f, pio);
	subio->set_iomap_range_r(0x40, 0x4f, sub);	// input port
	subio->set_iomap_range_w(0x50, 0x5f, sub);	// crt control i/o
	subio->set_iomap_range_rw(0x60, 0x6f, gdc_gfx);
	subio->set_iomap_range_rw(0x70, 0x7f, gdc_chr);
	
	// cpu bus
	cpu->set_context_mem(main);
	cpu->set_context_io(io);
	cpu->set_context_intr(main);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	subcpu->set_context_mem(sub);
	subcpu->set_context_io(subio);
	subcpu->set_context_intr(sub);
#ifdef USE_DEBUGGER
	subcpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int i = 0; i < 4; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2DD);
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
	
	// set busreq of sub cpu
	subcpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
}

void VM::run()
{
	event->drive();
}

double VM::frame_rate()
{
	return event->frame_rate();
}

// ----------------------------------------------------------------------------
// debugger
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
DEVICE *VM::get_cpu(int index)
{
	if(index == 0) {
		return cpu;
	} else if(index == 1) {
		return subcpu;
	}
	return NULL;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	sub->draw_screen();
}

int VM::access_lamp()
{
	uint32 status = fdc->read_signal(0);
	return (status & (1 | 4)) ? 1 : (status & (2 | 8)) ? 2 : 0;
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	pcm->init(rate, 8000);
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
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	sub->key_down(code);
}

void VM::key_up(int code)
{
	sub->key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_disk(int drv, _TCHAR* file_path, int offset)
{
	fdc->open_disk(drv, file_path, offset);
}

void VM::close_disk(int drv)
{
	fdc->close_disk(drv);
}

bool VM::disk_inserted(int drv)
{
	return fdc->disk_inserted(drv);
}

bool VM::now_skip()
{
	return event->now_skip();
}

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

