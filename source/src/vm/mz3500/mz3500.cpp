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
#include "../ls244.h"
#include "../mz1p17.h"
#include "../not.h"
#include "../pcm1bit.h"
#include "../prnfile.h"
#include "../upd1990a.h"
#include "../upd7220.h"
#include "../upd765a.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "main.h"
#include "sub.h"
#include "keyboard.h"

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
	if(config.printer_device_type == 0) {
		printer = new PRNFILE(this, emu);
	} else if(config.printer_device_type == 1) {
		printer = new MZ1P17(this, emu);
	} else {
		printer = dummy;
	}
	sio = new I8251(this, emu);
	pit = new I8253(this, emu);
	pio = new I8255(this, emu);
	subio = new IO(this, emu);
	ls244 = new LS244(this, emu);
	not_data1 = new NOT(this, emu);
	not_data2 = new NOT(this, emu);
	not_data3 = new NOT(this, emu);
	not_data4 = new NOT(this, emu);
	not_data5 = new NOT(this, emu);
	not_data6 = new NOT(this, emu);
	not_data7 = new NOT(this, emu);
	not_data8 = new NOT(this, emu);
	not_busy = new NOT(this, emu);
	pcm = new PCM1BIT(this, emu);
	rtc = new UPD1990A(this, emu);
	gdc_chr = new UPD7220(this, emu);
	gdc_gfx = new UPD7220(this, emu);
	subcpu = new Z80(this, emu);
	sub = new SUB(this, emu);
	kbd = new KEYBOARD(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu, CPU_CLOCKS);
	event->set_context_cpu(subcpu, CPU_CLOCKS);
	event->set_context_sound(pcm);
	
	// mz3500sm p.59
	fdc->set_context_irq(main, SIG_MAIN_INTFD, 1);
	fdc->set_context_drq(main, SIG_MAIN_DRQ, 1);
	fdc->set_context_index(main, SIG_MAIN_INDEX, 1);
	
	// mz3500sm p.78
	if(config.printer_device_type == 0) {
		PRNFILE *prnfile = (PRNFILE *)printer;
		prnfile->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
		prnfile->set_context_ack(pio, SIG_I8255_PORT_C, 0x40);
	} else if(config.printer_device_type == 1) {
		MZ1P17 *mz1p17 = (MZ1P17 *)printer;
		mz1p17->mode = MZ1P17_MODE_MZ1;
		mz1p17->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
		mz1p17->set_context_ack(pio, SIG_I8255_PORT_C, 0x40);
		
		// sw1=on, sw2=off, select MZ-1P02 (mz3500sm p.85)
		config.dipswitch &= ~0x03;
		config.dipswitch |=  0x01;
	}
	
	// mz3500sm p.72,77
	sio->set_context_rxrdy(subcpu, SIG_CPU_NMI, 1);
	
	// mz3500sm p.77,83
	// i8253 ch.0 -> i8251 txc,rxc
	pit->set_context_ch1(pcm, SIG_PCM1BIT_SIGNAL, 1);
	pit->set_context_ch2(pit, SIG_I8253_GATE_1, 1);
	pit->set_constant_clock(0, 2450760);
	pit->set_constant_clock(1, 2450760);
	pit->set_constant_clock(2, 2450760);
	
	// mz3500sm p.78,80,81
	pio->set_context_port_a(not_data1, SIG_NOT_INPUT, 0x01, 0);
	pio->set_context_port_a(not_data2, SIG_NOT_INPUT, 0x02, 0);
	pio->set_context_port_a(not_data3, SIG_NOT_INPUT, 0x04, 0);
	pio->set_context_port_a(not_data4, SIG_NOT_INPUT, 0x08, 0);
	pio->set_context_port_a(not_data5, SIG_NOT_INPUT, 0x10, 0);
	pio->set_context_port_a(not_data6, SIG_NOT_INPUT, 0x20, 0);
	pio->set_context_port_a(not_data7, SIG_NOT_INPUT, 0x40, 0);
	pio->set_context_port_a(not_data8, SIG_NOT_INPUT, 0x80, 0);
	pio->set_context_port_b(rtc, SIG_UPD1990A_STB, 0x01, 0);
	pio->set_context_port_b(rtc, SIG_UPD1990A_C0,  0x02, 0);
	pio->set_context_port_b(rtc, SIG_UPD1990A_C1,  0x04, 0);
	pio->set_context_port_b(rtc, SIG_UPD1990A_C2,  0x08, 0);
	pio->set_context_port_b(rtc, SIG_UPD1990A_DIN, 0x10, 0);
	pio->set_context_port_b(rtc, SIG_UPD1990A_CLK, 0x20, 0);
	pio->set_context_port_b(main, SIG_MAIN_SRDY, 0x40, 0);
//	pio->set_context_port_b(sub, SIG_SUB_PIO_PM, 0x80, 0);	// P/M: CG Selection
	pio->set_context_port_c(kbd, SIG_KEYBOARD_DC, 0x01, 0);
	pio->set_context_port_c(kbd, SIG_KEYBOARD_STC, 0x02, 0);
	pio->set_context_port_c(kbd, SIG_KEYBOARD_ACKC, 0x04, 0);
	// i8255 pc3 <- intr (not use ???)
	pio->set_context_port_c(pcm, SIG_PCM1BIT_MUTE, 0x10, 0);
	pio->set_context_port_c(printer, SIG_PRINTER_STROBE, 0x20, 0);
	// i8255 pc6 <- printer ack
	pio->set_context_port_c(ls244, SIG_LS244_INPUT, 0x80, -6);
	
	// mz3500sm p.78
	not_data1->set_context_out(printer, SIG_PRINTER_DATA, 0x01);
	not_data2->set_context_out(printer, SIG_PRINTER_DATA, 0x02);
	not_data3->set_context_out(printer, SIG_PRINTER_DATA, 0x04);
	not_data4->set_context_out(printer, SIG_PRINTER_DATA, 0x08);
	not_data5->set_context_out(printer, SIG_PRINTER_DATA, 0x10);
	not_data6->set_context_out(printer, SIG_PRINTER_DATA, 0x20);
	not_data7->set_context_out(printer, SIG_PRINTER_DATA, 0x40);
	not_data8->set_context_out(printer, SIG_PRINTER_DATA, 0x80);
	not_busy->set_context_out(ls244, SIG_LS244_INPUT, 0x04);
	
	// mz3500sm p.80,81
	rtc->set_context_dout(ls244, SIG_LS244_INPUT, 0x01);
	
	gdc_chr->set_vram_ptr(sub->get_vram_chr(), 0x2000, 0xfff);
	sub->set_sync_ptr_chr(gdc_chr->get_sync());
	sub->set_ra_ptr_chr(gdc_chr->get_ra());
	sub->set_cs_ptr_chr(gdc_chr->get_cs());
	sub->set_ead_ptr_chr(gdc_chr->get_ead());
	
	gdc_gfx->set_vram_ptr(sub->get_vram_gfx(), 0x18000);
	sub->set_sync_ptr_gfx(gdc_gfx->get_sync());
	sub->set_ra_ptr_gfx(gdc_gfx->get_ra());
	sub->set_cs_ptr_gfx(gdc_gfx->get_cs());
	sub->set_ead_ptr_gfx(gdc_gfx->get_ead());
	
	kbd->set_context_subcpu(subcpu);
	kbd->set_context_ls244(ls244);
	
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
	subio->set_iomap_range_w(0x00, 0x0f, sub);	// int0 to main (set flipflop)
	subio->set_iomap_range_rw(0x10, 0x1f, sio);
	subio->set_iomap_range_rw(0x20, 0x2f, pit);
	subio->set_iomap_range_rw(0x30, 0x3f, pio);
	subio->set_iomap_range_r(0x40, 0x4f, ls244);	// input port
	subio->set_iomap_range_rw(0x50, 0x5f, sub);	// crt control i/o
	subio->set_iomap_range_rw(0x60, 0x6f, gdc_gfx);
	subio->set_iomap_range_rw(0x70, 0x7f, gdc_chr);
#ifdef _IO_DEBUG_LOG
	subio->cpu_index = 1;
#endif
	
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
	// GDC clock (mz3500sm p.33,34)
	if(config.monitor_type == 0 || config.monitor_type == 1) {
		gdc_chr->set_horiz_freq(20920);
		gdc_gfx->set_horiz_freq(20920);
	} else {
		gdc_chr->set_horiz_freq(15870);
		gdc_gfx->set_horiz_freq(15870);
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
	
	// halt key is not pressed (mz3500sm p.80)
	halt = 0;
	ls244->write_signal(SIG_LS244_INPUT, 0x80, 0xff);
	
	// set printer signal (mz3500sm p.78)
	not_busy->write_signal(SIG_NOT_INPUT, 0, 0);		// busy = low
	ls244->write_signal(SIG_LS244_INPUT, 0x1c, 0x1c);	// busy = ~(low), pe = ~(low), pdtr = high
	pio->write_signal(SIG_I8255_PORT_C, 0x40, 0x40);	// ack = high
}

void VM::special_reset()
{
	// halt key is pressed (mz3500sm p.80)
	halt = 8;
	ls244->write_signal(SIG_LS244_INPUT, 0x00, 0x80);
}

void VM::run()
{
	// halt key is released (mz3500sm p.80)
	if(halt != 0 && --halt == 0) {
		ls244->write_signal(SIG_LS244_INPUT, 0x80, 0x80);
	}
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

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch == 0) {
		pcm->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	kbd->key_down(code);
}

void VM::key_up(int code)
{
	kbd->key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_disk(int drv, const _TCHAR* file_path, int bank)
{
	fdc->open_disk(drv, file_path, bank);
}

void VM::close_disk(int drv)
{
	fdc->close_disk(drv);
}

bool VM::disk_inserted(int drv)
{
	return fdc->disk_inserted(drv);
}

void VM::set_disk_protected(int drv, bool value)
{
	fdc->set_disk_protected(drv, value);
}

bool VM::get_disk_protected(int drv)
{
	return fdc->get_disk_protected(drv);
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

#define STATE_VERSION	3

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
	state_fio->FputUint8(halt);
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
	halt = state_fio->FgetUint8();
	return true;
}

