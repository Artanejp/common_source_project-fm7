/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

	[ virtual machine ]
*/

#include "mz2800.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../i8253.h"
#include "../i8255.h"
#include "../i8259.h"
#include "../i286.h"
#include "../io.h"
#include "../mb8877.h"
#include "../mz1p17.h"
#include "../not.h"
#include "../pcm1bit.h"
//#include "../pcpr201.h"
#include "../prnfile.h"
#include "../rp5c01.h"
//#include "../sasi.h"
#include "../upd71071.h"
#include "../ym2203.h"
#include "../z80pio.h"
#include "../z80sio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "crtc.h"
#include "floppy.h"
#include "joystick.h"
#include "keyboard.h"
#include "memory.h"
#include "mouse.h"
#include "printer.h"
#include "reset.h"
#include "serial.h"
#include "sysport.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
#if defined(_USE_QT)
	dummy->set_device_name(_T("1st Dummy"));
	event->set_device_name(_T("EVENT"));
#endif	
	cpu = new I286(this, emu);
#if defined(_USE_QT)
	cpu->set_device_name(_T("CPU(i286)"));
#endif	
	pit = new I8253(this, emu);
	pio0 = new I8255(this, emu);
	pic = new I8259(this, emu);
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);
	not_busy = new NOT(this, emu);
#if defined(_USE_QT)
	not_busy->set_device_name(_T("NOT GATE(PRINTER BUSY)"));
#endif
	pcm = new PCM1BIT(this, emu);
	rtc = new RP5C01(this, emu);	// RP-5C15
//	sasi = new SASI(this, emu);
	dma = new UPD71071(this, emu);
	opn = new YM2203(this, emu);
	pio1 = new Z80PIO(this, emu);
	sio = new Z80SIO(this, emu);
	
	crtc = new CRTC(this, emu);
	floppy = new FLOPPY(this, emu);
	joystick = new JOYSTICK(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	mouse = new MOUSE(this, emu);
	printer = new PRINTER(this, emu);
	rst = new RESET(this, emu);
	serial = new SERIAL(this, emu);
	sysport = new SYSPORT(this, emu);
#if defined(_USE_QT)
	crtc->set_device_name(_T("CRTC"));
	floppy->set_device_name(_T("FLOPPY I/F"));
	joystick->set_device_name(_T("JOYSTICK I/F"));
	keyboard->set_device_name(_T("KEYBOARD I/F"));
	memory->set_device_name(_T("MEMORY"));
	mouse->set_device_name(_T("MOUSE I/F"));
	printer->set_device_name(_T("PRINTER I/F"));
	rst->set_device_name(_T("RESET I/F"));
	serial->set_device_name(_T("SERIAL I/F"));
	sysport->set_device_name(_T("SYSTEM PORT"));
#endif	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(opn);
	event->set_context_sound(pcm);
	
	pit->set_constant_clock(0, 31250);
	pit->set_constant_clock(2, 31250);
	pit->set_context_ch0(pit, SIG_I8253_CLOCK_1, 1);
	pit->set_context_ch0(pic, SIG_I8259_CHIP0 | SIG_I8259_IR0, 1);
	pit->set_context_ch1(pic, SIG_I8259_CHIP0 | SIG_I8259_IR6, 1);
	pit->set_context_ch2(pic, SIG_I8259_CHIP1 | SIG_I8259_IR0, 1);
	pio0->set_context_port_c(rst, SIG_RESET_CONTROL, 0xff, 0);
	pio0->set_context_port_c(crtc, SIG_CRTC_MASK, 0x01, 0);
	pio0->set_context_port_c(pcm, SIG_PCM1BIT_SIGNAL, 0x04, 0);
	pic->set_context_cpu(cpu);
	fdc->set_context_drq(dma, SIG_UPD71071_CH1, 1);
	fdc->set_context_irq(pic, SIG_I8259_CHIP0 | SIG_I8259_IR5, 1);
	not_busy->set_context_out(pic, SIG_I8259_CHIP1 | SIG_I8259_IR1, 1);
	rtc->set_context_alarm(pic, SIG_I8259_CHIP1 | SIG_I8259_IR2, 1);
	rtc->set_context_pulse(opn, SIG_YM2203_PORT_B, 8);
//	sasi->set_context_drq(dma, SIG_UPD71071_CH0, 1);
//	sasi->set_context_irq(pic, SIG_I8259_CHIP0 | SIG_I8259_IR4, 1);
	dma->set_context_memory(memory);
//	dma->set_context_ch0(sasi);
	dma->set_context_ch1(fdc);
	dma->set_context_tc(pic, SIG_I8259_CHIP0 | SIG_I8259_IR3, 1);
	opn->set_context_irq(pic, SIG_I8259_CHIP1 | SIG_I8259_IR7, 1);
	opn->set_context_port_a(crtc, SIG_CRTC_PALLETE, 0x04, 0);
	opn->set_context_port_a(mouse, SIG_MOUSE_SEL, 0x08, 0);
	pio1->set_context_port_a(crtc, SIG_CRTC_COLUMN_SIZE, 0x20, 0);
	pio1->set_context_port_a(keyboard, SIG_KEYBOARD_COLUMN, 0xff, 0);
	sio->set_context_intr(pic, SIG_I8259_CHIP0 | SIG_I8259_IR2);
	sio->set_context_dtr(1, mouse, SIG_MOUSE_DTR, 1);
	
	crtc->set_context_pic(pic);
	crtc->set_context_pio(pio0);
	crtc->set_vram_ptr(memory->get_vram());
	crtc->set_tvram_ptr(memory->get_tvram());
	crtc->set_kanji_ptr(memory->get_kanji());
	crtc->set_pcg_ptr(memory->get_pcg());
	floppy->set_context_fdc(fdc);
	keyboard->set_context_pio0(pio0);
	keyboard->set_context_pio1(pio1);
	memory->set_context_crtc(crtc);
	mouse->set_context_sio(sio);
	if(config.printer_device_type == 0) {  
		PRNFILE *prnfile = new PRNFILE(this, emu);
		prnfile->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
		printer->set_context_prn(prnfile);
	} else if(config.printer_device_type == 1) {
		MZ1P17 *mz1p17 = new MZ1P17(this, emu);
		mz1p17->mode = MZ1P17_MODE_MZ1;
		mz1p17->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
		printer->set_context_prn(mz1p17);
//	} else if(config.printer_device_type == 2) {
//		PCPR201 *pcpr201 = new PCPR201(this, emu);
//		pcpr201->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
//		printer->set_context_prn(pcpr201);
	} else {
		printer->set_context_prn(dummy);
	}
	serial->set_context_sio(sio);
	sysport->set_context_pit(pit);
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
	io->set_iomap_range_rw(0x70, 0x7f, dma);
	io->set_iomap_alias_rw(0x80, pic, I8259_ADDR_CHIP0 | 0);
	io->set_iomap_alias_rw(0x81, pic, I8259_ADDR_CHIP0 | 1);
	io->set_iomap_alias_rw(0x82, pic, I8259_ADDR_CHIP1 | 0);
	io->set_iomap_alias_rw(0x83, pic, I8259_ADDR_CHIP1 | 1);
	io->set_iomap_range_rw(0x8c, 0x8d, memory);
	io->set_iovalue_single_r(0x8e, 0xff);	// dipswitch
	io->set_flipflop_single_rw(0x8f, 0x00);	// shut
	io->set_iomap_range_rw(0xa0, 0xa3, serial);
	for(uint32_t p = 0xae; p <= 0x1fae; p += 0x100) {
		io->set_iomap_single_w(p, crtc);
	}
//	io->set_iomap_single_rw(0xaf, sasi);
	io->set_iomap_range_rw(0xb0, 0xb3, serial);
	io->set_iomap_single_r(0xbe, sysport);
	io->set_iomap_range_rw(0xc8, 0xc9, opn);
	io->set_iovalue_single_r(0xca, 0x7f);	// voice communication ???
	for(uint32_t p = 0xcc; p <= 0xfcc; p += 0x100) {
		io->set_iomap_alias_rw(p, rtc, p >> 8);
	}
	io->set_iomap_single_w(0xcd, serial);
	io->set_iomap_single_rw(0xce, memory);
	io->set_iomap_range_rw(0xd8, 0xdb, fdc);
	io->set_iomap_range_w(0xdc, 0xdf, floppy);
	io->set_iomap_range_rw(0xe0, 0xe3, pio0);
	io->set_iomap_range_rw(0xe4, 0xe7, pit);
	io->set_iomap_range_rw(0xe8, 0xeb, pio1);
	io->set_iomap_single_rw(0xef, joystick);
//	io->set_iomap_range_w(0xf0, 0xf3, sysport);
	io->set_iomap_single_w(0x170, crtc);
	io->set_iomap_single_w(0x172, crtc);
	io->set_iomap_single_w(0x174, crtc);
	io->set_iomap_single_w(0x176, crtc);
	io->set_iomap_range_w(0x178, 0x17b, crtc);
	io->set_iomap_range_rw(0x1fe, 0x1ff, printer);
	io->set_iomap_single_w(0x270, crtc);
	io->set_iomap_single_w(0x272, crtc);
	io->set_iomap_single_rw(0x274, memory);
	
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
	// temporary fix...
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
	
	// set initial port status
	pio0->write_signal(SIG_I8255_PORT_B, 0x7c, 0xff);
	opn->write_signal(SIG_YM2203_PORT_B, 0x37, 0xff);
}

void VM::cpu_reset()
{
	cpu->reset();
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
	crtc->draw_screen();
}

uint32_t VM::get_access_lamp_status()
{
	uint32_t status = fdc->read_signal(0);
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
	opn->initialize_sound(rate, 2000000, samples, 0, -8);
	pcm->initialize_sound(rate, 4096);
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
		opn->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		opn->set_volume(1, decibel_l, decibel_r);
	} else if(ch == 2) {
		pcm->set_volume(0, decibel_l, decibel_r);
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

#define STATE_VERSION	2

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
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
	return true;
}

