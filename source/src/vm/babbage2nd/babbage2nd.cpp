/*
	Gijutsu-Hyoron-Sha Babbage-2nd Emulator 'eBabbage-2nd'

	Author : Takeda.Toshiya
	Date   : 2009.12.26 -

	[ virtual machine ]
*/

#include "babbage2nd.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../io.h"
#include "../memory.h"
#include "../z80.h"
#include "../z80ctc.h"
#include "../z80pio.h"

#include "display.h"
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
	
	io = new IO(this, emu);
	memory = new MEMORY(this, emu);
	cpu = new Z80(this, emu);
	ctc = new Z80CTC(this, emu);
	pio1 = new Z80PIO(this, emu);
	pio2 = new Z80PIO(this, emu);
	
	display = new DISPLAY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	
	pio2->set_context_port_b(display, SIG_DISPLAY_7SEG_LED, 0xff, 0);
	keyboard->set_context_pio(pio2);
	// p.145, fig.3-4
	ctc->set_context_zc2(ctc, SIG_Z80CTC_TRIG_1, 1);
	ctc->set_context_zc1(ctc, SIG_Z80CTC_TRIG_0, 1);
	// p.114, fig.2-52
	pio1->set_context_port_b(display, SIG_DISPLAY_8BIT_LED, 0xff, 0);
	//pio1->set_context_port_b(pio1, SIG_Z80PIO_PORT_A, 0xff, 0);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(ctc);
	
	// z80 family daisy chain
	ctc->set_context_intr(cpu, 0);
	ctc->set_context_child(pio1);
	pio1->set_context_intr(cpu, 1);
	pio1->set_context_child(pio2);
	pio2->set_context_intr(cpu, 2);
	
	// memory bus
	memset(ram, 0, sizeof(ram));
	memset(rom, 0xff, sizeof(rom));
	
	memory->read_bios(_T("MON.ROM"), rom, sizeof(rom));
	
	memory->set_memory_r(0x0000, 0x07ff, rom);
	memory->set_memory_rw(0x1000, 0x17ff, ram);
	
	// i/o bus
	io->set_iomap_range_rw(0x00, 0x03, ctc);
	io->set_iomap_range_rw(0x10, 0x13, pio1);
	io->set_iomap_range_rw(0x20, 0x23, pio2);
	
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
	//keyboard->key_up(code);
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

