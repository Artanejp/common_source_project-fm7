/*
	EPSON QC-10 Emulator 'eQC-10'

	Author : Takeda.Toshiya
	Date   : 2008.02.13 -

	[ virtual machine ]
*/

#include "qc10.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../disk.h"
#include "../hd146818p.h"
#include "../i8237.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../i8259.h"
#include "../io.h"
#include "../pcm1bit.h"
#include "../upd7220.h"
#include "../upd765a.h"
#include "../z80.h"
#include "../z80sio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
#include "floppy.h"
#include "keyboard.h"
#include "memory.h"
#include "mfont.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	rtc = new HD146818P(this, emu);
	dma0 = new I8237(this, emu);
	dma1 = new I8237(this, emu);
	pit0 = new I8253(this, emu);
	pit1 = new I8253(this, emu);
	pio = new I8255(this, emu);
	pic = new I8259(this, emu);
	io = new IO(this, emu);
	pcm = new PCM1BIT(this, emu);
	gdc = new UPD7220(this, emu);
	fdc = new UPD765A(this, emu);
	cpu = new Z80(this, emu);
	sio = new Z80SIO(this, emu);	// uPD7201
	
	display = new DISPLAY(this, emu);
	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	mfont = new MFONT(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	
	rtc->set_context_intr(pic, SIG_I8259_IR2 | SIG_I8259_CHIP1, 1);
	dma0->set_context_memory(memory);
	dma0->set_context_ch0(fdc);
	dma0->set_context_ch1(gdc);
#ifdef SINGLE_MODE_DMA
	dma0->set_context_child_dma(dma1);
#endif
	dma1->set_context_memory(memory);
	pit0->set_context_ch0(memory, SIG_MEMORY_PCM, 1);
	pit0->set_context_ch1(pic, SIG_I8259_IR5 | SIG_I8259_CHIP1, 1);
	pit0->set_context_ch2(pic, SIG_I8259_IR1 | SIG_I8259_CHIP0, 1);
	pit0->set_constant_clock(2, CPU_CLOCKS >> 1);	// 1.9968MHz
	pit1->set_context_ch0(pcm, SIG_PCM1BIT_SIGNAL, 1);
	pit1->set_context_ch1(pit0, SIG_I8253_CLOCK_0, 1);
	pit1->set_context_ch1(pit0, SIG_I8253_CLOCK_1, 1);
	pit1->set_constant_clock(0, CPU_CLOCKS >> 1);	// 1.9968MHz
	pit1->set_constant_clock(1, CPU_CLOCKS >> 1);	// 1.9968MHz
	pit1->set_constant_clock(2, CPU_CLOCKS >> 1);	// 1.9968MHz
	pio->set_context_port_c(pic, SIG_I8259_IR0 | SIG_I8259_CHIP1, 8, 0);
	pic->set_context_cpu(cpu);
	gdc->set_context_drq(dma0, SIG_I8237_CH1, 1);
	gdc->set_vram_ptr(display->get_vram(), VRAM_SIZE);
	// IR5 of I8259 #0 is from light pen
	fdc->set_context_irq(pic, SIG_I8259_IR6 | SIG_I8259_CHIP0, 1);
	fdc->set_context_irq(memory, SIG_MEMORY_FDC_IRQ, 1);
	fdc->set_context_drq(dma0, SIG_I8237_CH0, 1);
	sio->set_context_intr(pic, SIG_I8259_IR4 | SIG_I8259_CHIP0);
	sio->set_context_send0(keyboard, SIG_KEYBOARD_RECV);
	
	display->set_context_gdc(gdc);
	display->set_sync_ptr(gdc->get_sync());
	display->set_zoom_ptr(gdc->get_zoom());
	display->set_ra_ptr(gdc->get_ra());
	display->set_cs_ptr(gdc->get_cs());
	display->set_ead_ptr(gdc->get_ead());
	floppy->set_context_fdc(fdc);
	floppy->set_context_mem(memory);
	keyboard->set_context_sio(sio);
	memory->set_context_pit(pit0);
	memory->set_context_pcm(pcm);
	memory->set_context_fdc(fdc);
	mfont->set_context_pic(pic);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
#ifdef SINGLE_MODE_DMA
	cpu->set_context_dma(dma0);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_range_rw(0x00, 0x03, pit0);
	io->set_iomap_range_rw(0x04, 0x07, pit1);
	io->set_iomap_alias_rw(0x08, pic, I8259_ADDR_CHIP0 | 0);
	io->set_iomap_alias_rw(0x09, pic, I8259_ADDR_CHIP0 | 1);
	io->set_iomap_alias_rw(0x0c, pic, I8259_ADDR_CHIP1 | 0);
	io->set_iomap_alias_rw(0x0d, pic, I8259_ADDR_CHIP1 | 1);
	io->set_iomap_alias_rw(0x10, sio, 0);
	io->set_iomap_alias_rw(0x11, sio, 2);
	io->set_iomap_alias_rw(0x12, sio, 1);
	io->set_iomap_alias_rw(0x13, sio, 3);
	io->set_iomap_range_rw(0x14, 0x17, pio);
	io->set_iomap_range_rw(0x18, 0x1b, memory);
	io->set_iomap_range_w(0x1c, 0x23, memory);
	io->set_iomap_range_rw(0x2c, 0x2d, display);
	io->set_iomap_range_r(0x30, 0x33, memory);
	io->set_iomap_range_w(0x30, 0x33, floppy);
	io->set_iomap_range_rw(0x34, 0x35, fdc);
	io->set_iomap_range_rw(0x38, 0x3b, gdc);
	io->set_iomap_range_rw(0x3c, 0x3d, rtc);
	io->set_iomap_range_rw(0x40, 0x4f, dma0);
	io->set_iomap_range_rw(0x50, 0x5f, dma1);
	io->set_iomap_range_rw(0xfc, 0xfd, mfont);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int i = 0; i < 4; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2D);
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
	pcm->init(rate, 4000);
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

