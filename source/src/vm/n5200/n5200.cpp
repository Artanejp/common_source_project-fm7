/*
	NEC N5200 Emulator 'eN5200'

	Author : Takeda.Toshiya
	Date   : 2009.06.03-

	[ virtual machine ]
*/

#include "n5200.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../beep.h"
#include "../i386.h"
#include "../i8237.h"
#include "../i8251.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../i8259.h"
#include "../io.h"
#include "../upd1990a.h"
#include "../upd7220.h"
#include "../upd765a.h"

#include "display.h"
#include "floppy.h"
#include "keyboard.h"
#include "memory.h"
#include "system.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	beep = new BEEP(this, emu);
	cpu = new I386(this, emu);
	dma = new I8237(this, emu);
	sio_r = new I8251(this, emu);	// for rs232c
	sio_k = new I8251(this, emu);	// for keyboard
	pit = new I8253(this, emu);
	pio_s = new I8255(this, emu);	// for system port
	pio_p = new I8255(this, emu);	// for printer
	pic = new I8259(this, emu);
	io = new IO(this, emu);
	rtc = new UPD1990A(this, emu);
	gdc_c = new UPD7220(this, emu);
	gdc_g = new UPD7220(this, emu);
	fdc = new UPD765A(this, emu);
	
	display = new DISPLAY(this, emu);
	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	system = new SYSTEM(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(beep);
	
//???	sio_r->set_context_rxrdy(pic, SIG_I8259_CHIP0 | SIG_I8259_IR4, 1);
	sio_k->set_context_rxrdy(pic, SIG_I8259_CHIP0 | SIG_I8259_IR1, 1);
	sio_k->set_context_rst(keyboard, SIG_KEYBOARD_RST, 1);
	sio_k->set_context_out(keyboard, SIG_KEYBOARD_RECV);
	pit->set_context_ch0(pic, SIG_I8259_CHIP0 | SIG_I8259_IR0, 1);
	pit->set_constant_clock(0, 1996800);
	pit->set_constant_clock(1, 300);	// ???
	pit->set_constant_clock(2, 1996800);
	pio_s->set_context_port_c(beep, SIG_BEEP_MUTE, 8, 0);
	pic->set_context_cpu(cpu);
	rtc->set_context_dout(pio_s, SIG_I8255_PORT_B, 1);
	dma->set_context_memory(memory);
	dma->set_context_ch2(fdc);	// 1MB
	dma->set_context_ch3(fdc);	// 640KB
	gdc_g->set_vram_ptr(memory->get_vram(), 0x20000);
	fdc->set_context_irq(pic, SIG_I8259_IR6, 1);
	fdc->set_context_drq(floppy, SIG_FLOPPY_DRQ, 1);
	
	display->set_context_pic(pic);
	display->set_vram_ptr(memory->get_vram());
	display->set_tvram_ptr(memory->get_tvram());
	floppy->set_context_fdc(fdc);
	floppy->set_context_dma(dma);
	keyboard->set_context_sio(sio_k);
	system->set_context_dma(dma);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
	
	// i/o bus
	io->set_iomap_alias_w(0x00, pic, 0);
	io->set_iomap_alias_w(0x02, pic, 1);
	io->set_iomap_alias_w(0x08, pic, 2);
	io->set_iomap_alias_w(0x0a, pic, 3);
	io->set_iomap_alias_w(0x01, dma, 0);
	io->set_iomap_alias_w(0x03, dma, 1);
	io->set_iomap_alias_w(0x05, dma, 2);
	io->set_iomap_alias_w(0x07, dma, 3);
	io->set_iomap_alias_w(0x09, dma, 4);
	io->set_iomap_alias_w(0x0b, dma, 5);
	io->set_iomap_alias_w(0x0d, dma, 6);
	io->set_iomap_alias_w(0x0f, dma, 7);
	io->set_iomap_alias_w(0x11, dma, 8);
	io->set_iomap_alias_w(0x13, dma, 9);
	io->set_iomap_alias_w(0x15, dma, 0x0a);
	io->set_iomap_alias_w(0x17, dma, 0x0b);
	io->set_iomap_alias_w(0x19, dma, 0x0c);
	io->set_iomap_alias_w(0x1b, dma, 0x0d);
	io->set_iomap_alias_w(0x1d, dma, 0x0e);
	io->set_iomap_alias_w(0x1f, dma, 0x0f);
	io->set_iomap_single_w(0x20, rtc);
	io->set_iomap_single_w(0x21, system);
	io->set_iomap_single_w(0x23, system);
	io->set_iomap_single_w(0x25, system);
	io->set_iomap_single_w(0x27, system);
	io->set_iomap_single_w(0x29, system);
	io->set_iomap_alias_w(0x30, sio_r, 0);
	io->set_iomap_alias_w(0x32, sio_r, 1);
	io->set_iomap_alias_w(0x31, pio_s, 0);
	io->set_iomap_alias_w(0x33, pio_s, 1);
	io->set_iomap_alias_w(0x35, pio_s, 2);
	io->set_iomap_alias_w(0x37, pio_s, 3);
	io->set_iomap_single_w(0x3b, system);
	io->set_iomap_alias_w(0x40, pio_p, 0);
	io->set_iomap_alias_w(0x42, pio_p, 1);
	io->set_iomap_alias_w(0x44, pio_p, 2);
	io->set_iomap_alias_w(0x46, pio_p, 3);
	io->set_iomap_alias_w(0x41, sio_k, 0);
	io->set_iomap_alias_w(0x43, sio_k, 1);
	io->set_iomap_single_w(0x50, system);
	io->set_iomap_single_w(0x52, system);
	io->set_iomap_alias_w(0x60, gdc_c, 0);
	io->set_iomap_alias_w(0x62, gdc_c, 1);
	io->set_iomap_single_w(0x64, display);
	io->set_iomap_single_w(0x68, display);
	io->set_iomap_single_w(0x6a, display);
	io->set_iomap_single_w(0x6c, display);
	io->set_iomap_single_w(0x70, display);
	io->set_iomap_single_w(0x72, display);
	io->set_iomap_single_w(0x74, memory);
	io->set_iomap_single_w(0x76, display);
	io->set_iomap_single_w(0x78, display);
	io->set_iomap_single_w(0x7a, display);
	io->set_iomap_alias_w(0x71, pit, 0);
	io->set_iomap_alias_w(0x73, pit, 1);
	io->set_iomap_alias_w(0x75, pit, 2);
	io->set_iomap_alias_w(0x77, pit, 3);
	io->set_iomap_single_w(0x92, floppy);
	io->set_iomap_single_w(0x94, floppy);
	io->set_iomap_alias_w(0xa0, gdc_g, 0);
	io->set_iomap_alias_w(0xa2, gdc_g, 1);
	io->set_iomap_single_w(0xa4, display);
	io->set_iomap_single_w(0xa6, display);
	io->set_iomap_single_w(0xa8, display);
	io->set_iomap_single_w(0xaa, display);
	io->set_iomap_single_w(0xac, display);
	io->set_iomap_single_w(0xae, display);
	io->set_iomap_single_w(0xca, floppy);
	io->set_iomap_single_w(0xcc, floppy);
	io->set_iomap_single_w(0xbe, floppy);
	
	io->set_iomap_alias_r(0x00, pic, 0);
	io->set_iomap_alias_r(0x02, pic, 1);
	io->set_iomap_alias_r(0x08, pic, 2);
	io->set_iomap_alias_r(0x0a, pic, 3);
	io->set_iomap_alias_r(0x01, dma, 0);
	io->set_iomap_alias_r(0x03, dma, 1);
	io->set_iomap_alias_r(0x05, dma, 2);
	io->set_iomap_alias_r(0x07, dma, 3);
	io->set_iomap_alias_r(0x09, dma, 4);
	io->set_iomap_alias_r(0x0b, dma, 5);
	io->set_iomap_alias_r(0x0d, dma, 6);
	io->set_iomap_alias_r(0x0f, dma, 7);
	io->set_iomap_alias_r(0x11, dma, 8);
	io->set_iomap_alias_r(0x13, dma, 9);
	io->set_iomap_alias_r(0x15, dma, 0x0a);
	io->set_iomap_alias_r(0x17, dma, 0x0b);
	io->set_iomap_alias_r(0x19, dma, 0x0c);
	io->set_iomap_alias_r(0x1b, dma, 0x0d);
	io->set_iomap_alias_r(0x1d, dma, 0x0e);
	io->set_iomap_alias_r(0x1f, dma, 0x0f);
	io->set_iomap_alias_r(0x30, sio_r, 0);
	io->set_iomap_alias_r(0x32, sio_r, 1);
	io->set_iomap_alias_r(0x31, pio_s, 0);
	io->set_iomap_alias_r(0x33, pio_s, 1);
	io->set_iomap_alias_r(0x35, pio_s, 2);
	io->set_iomap_alias_r(0x37, pio_s, 3);
	io->set_iomap_single_r(0x39, system);
	io->set_iomap_alias_r(0x40, pio_p, 0);
	io->set_iomap_alias_r(0x42, pio_p, 1);
	io->set_iomap_alias_r(0x44, pio_p, 2);
	io->set_iomap_alias_r(0x46, pio_p, 3);
	io->set_iomap_alias_r(0x41, sio_k, 0);
	io->set_iomap_alias_r(0x43, sio_k, 1);
	io->set_iomap_alias_r(0x60, gdc_c, 0);
	io->set_iomap_alias_r(0x62, gdc_c, 1);
	io->set_iomap_alias_r(0x71, pit, 0);
	io->set_iomap_alias_r(0x73, pit, 1);
	io->set_iomap_alias_r(0x75, pit, 2);
	io->set_iomap_single_r(0x90, floppy);
	io->set_iomap_single_r(0x92, floppy);
	io->set_iomap_single_r(0x94, floppy);
	io->set_iomap_alias_r(0xa0, gdc_g, 0);
	io->set_iomap_alias_r(0xa2, gdc_g, 1);
	io->set_iomap_single_r(0xc8, floppy);
	io->set_iomap_single_r(0xca, floppy);
	io->set_iomap_single_r(0xcc, floppy);
	io->set_iomap_single_r(0xbe, floppy);
	
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
	
	// initial device settings
	sio_k->write_signal(SIG_I8251_DSR, 1, 1);		// DSR = 1
	pio_s->write_io8(3, 0x92);
	pio_s->write_signal(SIG_I8255_PORT_A, 0x04, 0xff);	// SW = FDD
	pio_s->write_signal(SIG_I8255_PORT_B, 0xe1, 0xff);
	pio_p->write_io8(3, 0x82);
	pio_p->write_signal(SIG_I8255_PORT_B, 0x51, 0xff);
	beep->write_signal(SIG_BEEP_ON, 1, 1);
	beep->write_signal(SIG_BEEP_MUTE, 1, 1);
}

void VM::run()
{
	event->drive();
}

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	display->draw_screen();
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
	beep->init(rate, 2400, 8000);
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
	keyboard->key_down(code);
}

void VM::key_up(int code)
{
	keyboard->key_up(code);
//	emu->out_debug_log("-----\n");
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

