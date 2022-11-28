/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ virtual machine ]
*/

#include "mz5500.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../disk.h"
#include "../i8237.h"
#include "../i8255.h"
#include "../i8259.h"
#if defined(_MZ6550)
#include "../i286.h"
#else
#include "../i86.h"
#endif
#include "../io.h"
#include "../ls393.h"
#include "../mz1p17.h"
#include "../noise.h"
#include "../not.h"
#include "../prnfile.h"
#include "../rp5c01.h"
#include "../upd7220.h"
#include "../upd765a.h"
//#include "../ym2203.h"
#include "../ay_3_891x.h"
#include "../z80ctc.h"
#include "../z80sio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
#include "keyboard.h"
#include "memory.h"
#include "sysport.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	if(config.printer_type == 0) {
		printer = new PRNFILE(this, emu);
	} else if(config.printer_type == 1) {
		printer = new MZ1P17(this, emu);
	} else {
		printer = dummy;
	}
	dma = new I8237(this, emu);
#ifdef USE_DEBUGGER
	dma->set_context_debugger(new DEBUGGER(this, emu));
#endif
	pio = new I8255(this, emu);
	pic = new I8259(this, emu);
#if defined(_MZ6550)
	cpu = new I286(this, emu);
#else
	cpu = new I86(this, emu);
	cpu->device_model = INTEL_8086;
#endif
	io = new IO(this, emu);
	div = new LS393(this, emu);
	not_data0 = new NOT(this, emu);
	not_data0->set_device_name(_T("NOT Gate (Printer Bit0)"));
	not_data1 = new NOT(this, emu);
	not_data1->set_device_name(_T("NOT Gate (Printer Bit1)"));
	not_data2 = new NOT(this, emu);
	not_data2->set_device_name(_T("NOT Gate (Printer Bit2)"));
	not_data3 = new NOT(this, emu);
	not_data3->set_device_name(_T("NOT Gate (Printer Bit3)"));
	not_data4 = new NOT(this, emu);
	not_data4->set_device_name(_T("NOT Gate (Printer Bit4)"));
	not_data5 = new NOT(this, emu);
	not_data5->set_device_name(_T("NOT Gate (Printer Bit5)"));
	not_data6 = new NOT(this, emu);
	not_data6->set_device_name(_T("NOT Gate (Printer Bit6)"));
	not_data7 = new NOT(this, emu);
	not_data7->set_device_name(_T("NOT Gate (Printer Bit7)"));
	not_busy = new NOT(this, emu);
	not_busy->set_device_name(_T("NOT Gate (Printer Busy)"));
	rtc = new RP5C01(this, emu);
	gdc = new UPD7220(this, emu);
	fdc = new UPD765A(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	psg = new AY_3_891X(this, emu);	// AY-3-8912
#ifdef USE_DEBUGGER
	psg->set_context_debugger(new DEBUGGER(this, emu));
#endif
	ctc0 = new Z80CTC(this, emu);
#if defined(_MZ6500) || defined(_MZ6550)
	ctc1 = new Z80CTC(this, emu);
#endif
	sio = new Z80SIO(this, emu);
	
	display = new DISPLAY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	sysport = new SYSPORT(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	
	if(config.printer_type == 0) {
		PRNFILE *prnfile = (PRNFILE *)printer;
		prnfile->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
		prnfile->set_context_ack(pio, SIG_I8255_PORT_C, 0x40);
	} else if(config.printer_type == 1) {
		MZ1P17 *mz1p17 = (MZ1P17 *)printer;
		mz1p17->mode = MZ1P17_MODE_MZ1;
		mz1p17->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
		mz1p17->set_context_ack(pio, SIG_I8255_PORT_C, 0x40);
	}
	dma->set_context_memory(memory);
	dma->set_context_ch1(fdc);
	pio->set_context_port_a(not_data0, SIG_NOT_INPUT, 0x01, 0);
	pio->set_context_port_a(not_data1, SIG_NOT_INPUT, 0x02, 0);
	pio->set_context_port_a(not_data2, SIG_NOT_INPUT, 0x04, 0);
	pio->set_context_port_a(not_data3, SIG_NOT_INPUT, 0x08, 0);
	pio->set_context_port_a(not_data4, SIG_NOT_INPUT, 0x10, 0);
	pio->set_context_port_a(not_data5, SIG_NOT_INPUT, 0x20, 0);
	pio->set_context_port_a(not_data6, SIG_NOT_INPUT, 0x40, 0);
	pio->set_context_port_a(not_data7, SIG_NOT_INPUT, 0x80, 0);
	pio->set_context_port_c(keyboard, SIG_KEYBOARD_INPUT, 0x03, 0);
	pio->set_context_port_c(pic, SIG_I8259_IR2 | SIG_I8259_CHIP0, 0x08, 0);
	pio->set_context_port_c(printer, SIG_PRINTER_STROBE, 0x20, 0);
	pic->set_context_cpu(cpu);
	div->set_context_2qb(ctc0, SIG_Z80CTC_TRIG_3, 1);
#if defined(_MZ6500) || defined(_MZ6550)
	div->set_context_1qb(ctc1, SIG_Z80CTC_TRIG_0, 1);
	div->set_context_2qb(ctc1, SIG_Z80CTC_TRIG_1, 1);
	div->set_context_2qb(ctc1, SIG_Z80CTC_TRIG_2, 1);
	div->set_context_2qd(ctc1, SIG_Z80CTC_TRIG_3, 1);
#endif
	rtc->set_context_alarm(pic, SIG_I8259_IR0 | SIG_I8259_CHIP1, 1);
	gdc->set_vram_ptr(memory->get_vram(), 0x80000);
	gdc->set_context_vsync(pic, SIG_I8259_IR0 | SIG_I8259_CHIP0, 1);
	fdc->set_context_irq(pic, SIG_I8259_IR1 | SIG_I8259_CHIP1, 1);
	fdc->set_context_drq(dma, SIG_I8237_CH1, 1);
	psg->set_context_port_a(pic, SIG_I8259_IR7 | SIG_I8259_CHIP0, 0x20, 0);
	psg->set_context_port_a(pic, SIG_I8259_IR7 | SIG_I8259_CHIP1, 0x40, 0);
	psg->set_context_port_a(memory, SIG_MEMORY_BANK, 0xe0, 0);
	ctc0->set_context_intr(pic, SIG_I8259_IR5 | SIG_I8259_CHIP0);
	ctc0->set_context_zc0(div, SIG_LS393_CLK, 1);
	ctc0->set_context_zc1(sio, SIG_Z80SIO_TX_CLK_CH0, 1);
	ctc0->set_context_zc1(sio, SIG_Z80SIO_RX_CLK_CH0, 1);
	ctc0->set_context_zc2(sio, SIG_Z80SIO_TX_CLK_CH1, 1);
	ctc0->set_context_zc2(sio, SIG_Z80SIO_RX_CLK_CH1, 1);
#if defined(_MZ6500) || defined(_MZ6550)
	ctc0->set_context_child(ctc1);
	ctc1->set_context_intr(pic, SIG_I8259_IR5 | SIG_I8259_CHIP0);
#endif
	sio->set_context_intr(pic, SIG_I8259_IR1 | SIG_I8259_CHIP0);
	
	not_data0->set_context_out(printer, SIG_PRINTER_DATA, 0x01);
	not_data1->set_context_out(printer, SIG_PRINTER_DATA, 0x02);
	not_data2->set_context_out(printer, SIG_PRINTER_DATA, 0x04);
	not_data3->set_context_out(printer, SIG_PRINTER_DATA, 0x08);
	not_data4->set_context_out(printer, SIG_PRINTER_DATA, 0x10);
	not_data5->set_context_out(printer, SIG_PRINTER_DATA, 0x20);
	not_data6->set_context_out(printer, SIG_PRINTER_DATA, 0x40);
	not_data7->set_context_out(printer, SIG_PRINTER_DATA, 0x80);
	not_busy->set_context_out(pio, SIG_I8255_PORT_B, 0x01);
	display->set_vram_ptr(memory->get_vram());
	display->set_sync_ptr(gdc->get_sync());
	display->set_ra_ptr(gdc->get_ra());
	display->set_cs_ptr(gdc->get_cs());
	display->set_ead_ptr(gdc->get_ead());
	keyboard->set_context_pio(pio);
	keyboard->set_context_pic(pic);
	memory->set_context_cpu(cpu);
	sysport->set_context_fdc(fdc);
	sysport->set_context_ctc(ctc0);
	sysport->set_context_sio(sio);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
#ifdef SINGLE_MODE_DMA
	cpu->set_context_dma(dma);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_range_rw(0x00, 0x0f, dma);
	io->set_iomap_range_rw(0x10, 0x1f, pio);
	io->set_iomap_range_rw(0x20, 0x2f, fdc);
	for(int i = 0x30; i < 0x40; i += 2) {
		io->set_iomap_alias_rw(i, pic, I8259_ADDR_CHIP0 | ((i >> 1) & 1));
	}
	for(int i = 0x40; i < 0x50; i += 2) {
		io->set_iomap_alias_rw(i, pic, I8259_ADDR_CHIP1 | ((i >> 1) & 1));
	}
	io->set_iomap_range_w(0x50, 0x5f, memory);
	io->set_iomap_range_r(0x60, 0x6f, sysport);
	io->set_iomap_range_w(0x70, 0x7f, sysport);
#if defined(_MZ6500) || defined(_MZ6550)
	io->set_iomap_single_rw(0xcd, memory);
#endif
	for(int i = 0x100; i < 0x110; i += 2) {
		io->set_iomap_alias_rw(i, gdc, (i >> 1) & 1);
	}
	io->set_iomap_range_rw(0x110, 0x17f, display);
	io->set_iomap_range_rw(0x200, 0x20f, sio);
	io->set_iomap_range_rw(0x210, 0x21f, ctc0);
	io->set_iomap_range_rw(0x220, 0x22f, rtc);
	for(int i = 0x230; i < 0x240; i++) {
		io->set_iomap_alias_rw(i, psg, ~i & 1);
	}
	io->set_iomap_range_r(0x240, 0x25f, sysport);
	io->set_iomap_range_w(0x260, 0x26f, sysport);
	io->set_iomap_range_r(0x270, 0x27f, sysport);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int i = 0; i < 4; i++) {
#if defined(_MZ6500) || defined(_MZ6550)
		fdc->set_drive_type(i, DRIVE_TYPE_2HD);
#else
		fdc->set_drive_type(i, DRIVE_TYPE_2D);
#endif
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
	not_busy->write_signal(SIG_NOT_INPUT, 0, 0);		// busy = low
	pio->write_signal(SIG_I8255_PORT_B, 0x03, 0x07);	// busy = ~(low), pe = ~(low), pdtr = ~(high)
	pio->write_signal(SIG_I8255_PORT_C, 0x40, 0x40);	// ack = high
}

void VM::special_reset()
{
	// nmi
	cpu->write_signal(SIG_CPU_NMI, 1, 1);
	sysport->nmi_reset();
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
	psg->initialize_sound(rate, 4000000, samples, 0, 0);
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
		psg->set_volume(1, decibel_l, decibel_r);
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

#define STATE_VERSION	8

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

