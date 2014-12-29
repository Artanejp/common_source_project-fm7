/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'

	Author : Takeda.Toshiya
	Date   : 2008.08.26 -

	[ virtual machine ]
*/

#include "tk80bs.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../i8080.h"
#include "../i8251.h"
#include "../i8255.h"
#include "../io.h"
#include "../memory.h"
#include "../pcm1bit.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "cmt.h"
#include "display.h"
#include "keyboard.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// check configs
//	boot_mode = config.boot_mode;
	boot_mode = -1;
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	sio_b = new I8251(this, emu);	// on TK-80BS
	pio_b = new I8255(this, emu);
	pio_t = new I8255(this, emu);	// on TK-80
	memio = new IO(this, emu);
	memory = new MEMORY(this, emu);
	pcm0 = new PCM1BIT(this, emu);
	pcm1 = new PCM1BIT(this, emu);
	cpu = new I8080(this, emu);
	
	cmt = new CMT(this, emu);
	display = new DISPLAY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm0);
	event->set_context_sound(pcm1);
	
/*	8255 on TK-80
	
	PA	key matrix
	PB0	serial in
	PC0	serial out
	PC1	sound #1
	PC2	sound #2
	PC4-6	key column
	PC7	dma disable
*/
	sio_b->set_context_out(cmt, SIG_CMT_OUT);
	pio_b->set_context_port_c(display, SIG_DISPLAY_MODE, 3, 0);
	pio_t->set_context_port_c(pcm0, SIG_PCM1BIT_SIGNAL, 2, 0);
	pio_t->set_context_port_c(pcm1, SIG_PCM1BIT_SIGNAL, 4, 0);
	pio_t->set_context_port_c(keyboard, SIG_KEYBOARD_COLUMN, 0x70, 0);
	pio_t->set_context_port_c(display, SIG_DISPLAY_DMA, 0x80, 0);
	
	cmt->set_context_sio(sio_b);
	display->set_context_key(keyboard);
	display->set_vram_ptr(vram);
	display->set_led_ptr(ram + 0x3f8);
	keyboard->set_context_pio_b(pio_b);
	keyboard->set_context_pio_t(pio_t);
	keyboard->set_context_cpu(cpu);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(pio_t);
	cpu->set_context_intr(keyboard);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// memory bus
	memset(mon, 0xff, sizeof(mon));
	memset(bsmon, 0xff, sizeof(bsmon));
	memset(ext, 0xff, sizeof(ext));
	
	static const uint8 top[3] = {0xc3, 0x00, 0xf0};
	static const uint8 rst[3] = {0xc3, 0xdd, 0x83};
	
	if(!memory->read_bios(_T("TK80.ROM"), mon, sizeof(mon))) {
		// default
		memcpy(mon, top, sizeof(top));
		memcpy(mon + 0x38, rst, sizeof(rst));
	}
	if(memory->read_bios(_T("BSMON.ROM"), bsmon, sizeof(bsmon))) {
		// patch
		memcpy(mon + 0x38, rst, sizeof(rst));
	}
	memory->read_bios(_T("EXT.ROM"), ext, sizeof(ext));
	
	memory->set_memory_r(0x0000, 0x07ff, mon);
	memory->set_memory_r(0x0c00, 0x7bff, ext);
	memory->set_memory_mapped_io_rw(0x7c00, 0x7dff, memio);
	memory->set_memory_rw(0x7e00, 0x7fff, vram);
	memory->set_memory_rw(0x8000, 0xcfff, ram);
	memory->set_memory_r(0xd000, 0xefff, basic);
	memory->set_memory_r(0xf000, 0xffff, bsmon);
	
	// memory mapped i/o
	memio->set_iomap_alias_rw(0x7df8, sio_b, 0);
	memio->set_iomap_alias_rw(0x7df9, sio_b, 1);
	memio->set_iomap_alias_rw(0x7dfc, pio_b, 0);
	memio->set_iomap_alias_rw(0x7dfd, pio_b, 1);
	memio->set_iomap_alias_rw(0x7dfe, pio_b, 2);
	memio->set_iomap_alias_w(0x7dff, pio_b, 3);
	
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
	// load basic rom
	if(boot_mode != config.boot_mode) {
		memset(basic, 0xff, sizeof(basic));
		if(config.boot_mode == 0) {
			memory->read_bios(_T("LV1BASIC.ROM"), basic + 0x1000, 0x1000);
		} else {
			memory->read_bios(_T("LV2BASIC.ROM"), basic, sizeof(basic));
		}
		boot_mode = config.boot_mode;
		
		memset(ram, 0, sizeof(ram));
		memset(vram, 0x20, sizeof(vram));
	}
	
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
	
	// init 8255 on TK-80
	pio_t->write_io8(0xfb, 0x92);
	pio_t->write_signal(SIG_I8255_PORT_A, 0xff, 0xff);
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
	pcm0->init(rate, 8000);
	pcm1->init(rate, 8000);
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

void VM::load_binary(int drv, _TCHAR* file_path)
{
	if(drv == 0) {
		memory->read_image(file_path, ram, sizeof(ram));
	}
}

void VM::save_binary(int drv, _TCHAR* file_path)
{
	if(drv == 0) {
		memory->write_image(file_path, ram, sizeof(ram));
	}
}

void VM::play_tape(_TCHAR* file_path)
{
	cmt->play_tape(file_path);
}

void VM::rec_tape(_TCHAR* file_path)
{
	cmt->rec_tape(file_path);
}

void VM::close_tape()
{
	cmt->close_tape();
}

bool VM::tape_inserted()
{
	return cmt->tape_inserted();
}

bool VM::now_skip()
{
	return event->now_skip();
}

void VM::update_config()
{
	if(boot_mode != config.boot_mode) {
		// boot mode is changed !!!
//		boot_mode = config.boot_mode;
		reset();
	} else {
		for(DEVICE* device = first_device; device; device = device->next_device) {
			device->update_config();
		}
	}
}

