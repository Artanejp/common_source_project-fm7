/*
	Computer Research CRC-80 Emulator 'eCRC-80'

	Author : Takeda.Toshiya
	Date   : 2022.06.05-

	[ virtual machine ]
*/

#include "crc80.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../io.h"
#include "../memory.h"
#include "../noise.h"
#include "../not.h"
#include "../z80.h"
#include "../z80pio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
#include "membus.h"

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
	io = new IO(this, emu);
	not = new NOT(this, emu);
	cpu = new Z80(this, emu);
	pio = new Z80PIO(this, emu);
	
	display = new DISPLAY(this, emu);
	memory = new MEMBUS(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(drec);
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	drec->set_context_ear(not, SIG_NOT_INPUT, 1);
	not->set_context_out(pio, SIG_Z80PIO_PORT_B, 0x80);
	pio->set_context_port_b(drec, SIG_DATAREC_MIC, 0x40, 0);
	pio->set_context_port_a(display, SIG_DISPLAY_PA, 0xff, 0);
	pio->set_context_port_b(display, SIG_DISPLAY_PB, 0x0f, 0);
	
	display->set_context_pio(pio);
	memory->set_context_cpu(cpu);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pio);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// memory bus
	memset(mon, 0xff, sizeof(mon));
	memset(tty, 0xff, sizeof(tty));
	memset(ext, 0xff, sizeof(ext));
	memset(ram, 0x00, sizeof(ram));
	
	memory->read_bios(_T("MON.ROM"), mon, sizeof(mon));
	memory->read_bios(_T("TTY.ROM"), tty, sizeof(tty));
	memory->read_bios(_T("EXT.ROM"), ext, sizeof(ext));
	
	for(int i = 0; i < 0x8000; i += 0x1000) {
		memory->set_memory_r (i + 0x0000, i + 0x03ff, mon);
		memory->set_memory_r (i + 0x0400, i + 0x07ff, tty);
		memory->set_memory_r (i + 0x0800, i + 0x0bff, ext);
		memory->set_memory_rw(i + 0x8000, i + 0x8fff, ram);
	}
	
	// i/o bus
	for(int i = 0; i < 0x100; i += 4) {
		io->set_iomap_alias_rw(i + 0, pio, 0);
		io->set_iomap_alias_rw(i + 1, pio, 2);
		io->set_iomap_alias_rw(i + 2, pio, 1);
		io->set_iomap_alias_rw(i + 3, pio, 3);
	}
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	pio->write_signal(SIG_Z80PIO_PORT_B, (config.dipswitch & 2) ? 0 : 0xff, 0x10);
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
	pio->write_signal(SIG_Z80PIO_PORT_A, 0xff, 0xff);
	pio->write_signal(SIG_Z80PIO_PORT_B, (config.dipswitch & 2) ? 0 : 0xff, 0x10);
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
		drec->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
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
}

void VM::key_up(int code)
{
}

bool VM::get_caps_locked()
{
	return false;
}

bool VM::get_kana_locked()
{
	return false;
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::load_binary(int drv, const _TCHAR* file_path)
{
	memory->read_image(file_path, ram, sizeof(ram));
}

void VM::save_binary(int drv, const _TCHAR* file_path)
{
	memory->write_image(file_path, ram, sizeof(ram));
}

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	if(drec->play_tape(file_path)) {
		push_play(drv);
	}
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	if(drec->rec_tape(file_path)) {
		push_play(drv);
	}
}

void VM::close_tape(int drv)
{
	emu->lock_vm();
	push_stop(drv);
	drec->close_tape();
	emu->unlock_vm();
}

bool VM::is_tape_inserted(int drv)
{
	return drec->is_tape_inserted();
}

bool VM::is_tape_playing(int drv)
{
	return drec->is_tape_playing();
}

bool VM::is_tape_recording(int drv)
{
	return drec->is_tape_recording();
}

int VM::get_tape_position(int drv)
{
	return drec->get_tape_position();
}

const _TCHAR* VM::get_tape_message(int drv)
{
	return drec->get_message();
}

void VM::push_play(int drv)
{
	drec->set_remote(false);
	drec->set_ff_rew(0);
	drec->set_remote(true);
}

void VM::push_stop(int drv)
{
	drec->set_remote(false);
}

void VM::push_fast_forward(int drv)
{
	drec->set_remote(false);
	drec->set_ff_rew(1);
	drec->set_remote(true);
}

void VM::push_fast_rewind(int drv)
{
	drec->set_remote(false);
	drec->set_ff_rew(-1);
	drec->set_remote(true);
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

#define STATE_VERSION	1

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
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}

