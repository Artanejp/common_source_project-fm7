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
#include "../noise.h"
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

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
	cpu = new I8080(this, emu);
	pio = new I8155(this, emu);
	io = new IO(this, emu);
	pcm = new PCM1BIT(this, emu);
	rtc = new UPD1990A(this, emu);
	
	cmt = new CMT(this, emu);
	keyboard = new KEYBOARD(this, emu);
	lcd = new LCD(this, emu);
	memory = new MEMORY(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	event->set_context_sound(drec);
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	drec->set_context_ear(cpu, SIG_I8085_SID, 1);
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
	pio->set_context_port_b(pcm, SIG_PCM1BIT_MUTE, 0x20, 0);
	pio->set_context_timer(pcm, SIG_PCM1BIT_SIGNAL, 1);
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
		drec->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 2) {
		drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
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

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	cmt->close_tape();
	
	bool remote = drec->get_remote();
	
	if(drec->play_tape(file_path) && remote) {
		// if machine already sets remote on, start playing now
		push_play(drv);
	}
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
	drec->set_remote(false);
	
	cmt->rec_tape(file_path);
}

void VM::close_tape(int drv)
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
	drec->set_remote(false);
	
	cmt->close_tape();
}

bool VM::is_tape_inserted(int drv)
{
	return drec->is_tape_inserted() || cmt->is_tape_inserted();
}

bool VM::is_tape_playing(int drv)
{
	if(drec->is_tape_inserted()) {
		return drec->is_tape_playing();
	} else {
		return cmt->is_tape_playing();
	}
}

bool VM::is_tape_recording(int drv)
{
	if(drec->is_tape_inserted()) {
		return drec->is_tape_recording();
	} else {
		return cmt->is_tape_recording();
	}
}

int VM::get_tape_position(int drv)
{
	if(drec->is_tape_inserted()) {
		return drec->get_tape_position();
	} else {
		return cmt->get_tape_position();
	}
}

const _TCHAR* VM::get_tape_message(int drv)
{
	if(drec->is_tape_inserted()) {
		return drec->get_message();
	} else {
		return NULL;
	}
}

void VM::push_play(int drv)
{
	if(drec->is_tape_inserted()) {
		drec->set_remote(false);
		drec->set_ff_rew(0);
		drec->set_remote(true);
	}
}

void VM::push_stop(int drv)
{
	if(drec->is_tape_inserted()) {
		drec->set_remote(false);
	}
}

void VM::push_fast_forward(int drv)
{
	if(drec->is_tape_inserted()) {
		drec->set_remote(false);
		drec->set_ff_rew(1);
		drec->set_remote(true);
	}
}

void VM::push_fast_rewind(int drv)
{
	if(drec->is_tape_inserted()) {
		drec->set_remote(false);
		drec->set_ff_rew(-1);
		drec->set_remote(true);
	}
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
		const char *name = typeid(*device).name() + 6; // skip "class "
		int len = (int)strlen(name);
		
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

