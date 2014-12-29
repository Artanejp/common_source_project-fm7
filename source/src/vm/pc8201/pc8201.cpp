/*
	NEC PC-8201 Emulator 'ePC-8201'

	Author : Takeda.Toshiya
	Date   : 2009.03.31-

	[ virtual machine ]
*/

#include "pc8201.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../i8080.h"
#include "../i8155.h"
#include "../io.h"
#include "../pcm1bit.h"
#include "../upd1990a.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "cmt.h"
#include "keyboard.h"
#include "lcd.h"
#include "memory.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	cpu = new I8080(this, emu);
	pio = new I8155(this, emu);
	io = new IO(this, emu);
	buzzer = new PCM1BIT(this, emu);
	rtc = new UPD1990A(this, emu);
	
	cmt = new CMT(this, emu);
	keyboard = new KEYBOARD(this, emu);
	lcd = new LCD(this, emu);
	memory = new MEMORY(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(buzzer);
	
	drec->set_context_out(cpu, SIG_I8085_SID, 1);
	cpu->set_context_sod(cmt, SIG_CMT_SOD, 1);
	pio->set_context_port_a(rtc, SIG_UPD1990A_C0, 1, 0);
	pio->set_context_port_a(rtc, SIG_UPD1990A_C1, 2, 0);
	pio->set_context_port_a(rtc, SIG_UPD1990A_C2, 4, 0);
	pio->set_context_port_a(rtc, SIG_UPD1990A_CLK, 8, 0);
	pio->set_context_port_a(rtc, SIG_UPD1990A_DIN, 0x10, 0);
	pio->set_context_port_a(keyboard, SIG_KEYBOARD_COLUMN_L, 0xff, 0);
	pio->set_context_port_a(lcd, SIG_LCD_CHIPSEL_L, 0xff, 0);
	pio->set_context_port_b(keyboard, SIG_KEYBOARD_COLUMN_H, 1, 0);
	pio->set_context_port_b(lcd, SIG_LCD_CHIPSEL_H, 3, 0);
	pio->set_context_port_b(buzzer, SIG_PCM1BIT_MUTE, 0x20, 0);
	pio->set_context_timer(buzzer, SIG_PCM1BIT_SIGNAL, 1);
	pio->set_constant_clock(CPU_CLOCKS);
	rtc->set_context_dout(pio, SIG_I8155_PORT_C, 1);
	rtc->set_context_tp(cpu, SIG_I8085_RST7, 1);
	
	memory->set_context_cmt(cmt);
	memory->set_context_drec(drec);
	memory->set_context_rtc(rtc);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(io);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_range_w(0x90, 0x9f, memory);
	io->set_iomap_range_rw(0xa0, 0xaf, memory);
	io->set_iomap_range_rw(0xb0, 0xbf, pio);
	io->set_iomap_range_r(0xe0, 0xef, keyboard);
	io->set_iomap_range_rw(0xf0, 0xff, lcd);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	rtc->write_signal(SIG_UPD1990A_STB, 0, 0);
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
	lcd->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	buzzer->init(rate, 8000);
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
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::play_tape(_TCHAR* file_path)
{
	cmt->close_tape();
	drec->play_tape(file_path);
}

void VM::rec_tape(_TCHAR* file_path)
{
	drec->close_tape();
	cmt->rec_tape(file_path);
}

void VM::close_tape()
{
	drec->close_tape();
	cmt->close_tape();
}

bool VM::tape_inserted()
{
	return drec->tape_inserted() || cmt->tape_inserted();
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

