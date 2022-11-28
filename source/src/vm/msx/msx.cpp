/*
	ASCII MSX1 Emulator 'yaMSX1'
	ASCII MSX2 Emulator 'yaMSX2'
	Pioneer PX-7 Emulator 'ePX-7'

	Author : tanam
	Date   : 2013.06.29-

	modified by Takeda.Toshiya
	modified by umaiboux

	[ virtual machine ]
*/

#include "msx.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../i8255.h"
#include "../io.h"
#if defined(_PX7)
#include "../ld700.h"
#endif
#include "../noise.h"
#include "../not.h"
//#include "../ym2203.h"
#include "../ay_3_891x.h"
#include "../pcm1bit.h"
#if defined(_MSX2)
#include "../rp5c01.h"
#include "../v99x8.h"
#else
#include "../tms9918a.h"
#endif
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "joystick.h"
#include "keyboard.h"
#include "memory.h"
#if defined(_MSX2)
#include "rtcif.h"
#endif

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
	pio = new I8255(this, emu);
	io = new IO(this, emu);
#if defined(_PX7)
	ldp = new LD700(this, emu);
#endif
	not_remote = new NOT(this, emu);
	psg = new AY_3_891X(this, emu);
#ifdef USE_DEBUGGER
	psg->set_context_debugger(new DEBUGGER(this, emu));
#endif
	pcm = new PCM1BIT(this, emu);
#if defined(_MSX2)
	rtc = new RP5C01(this, emu);
	vdp = new V99X8(this, emu);
#else
	vdp = new TMS9918A(this, emu);
#ifdef USE_DEBUGGER
	vdp->set_context_debugger(new DEBUGGER(this, emu));
#endif
#endif
	cpu = new Z80(this, emu);
	
	joystick = new JOYSTICK(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
#if defined(_MSX2)
	rtcif = new RTCIF(this, emu);
#endif
	slot0 = new SLOT0(this, emu);	// #0: main memory
	slot1 = new SLOT1(this, emu);	// #1: rom-cartridge or msx-dos
	slot2 = new SLOT2(this, emu);	// #2: fdd-cartridge or p-basic
	slot3 = new SLOT3(this, emu);	// #3: rom-cartridge or ram-cartridge
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	event->set_context_sound(pcm);
	event->set_context_sound(drec);
#if defined(_PX7)
	event->set_context_sound(ldp);
#endif
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	drec->set_context_ear(psg, SIG_AY_3_891X_PORT_A, 0x80);
	pio->set_context_port_a(memory, SIG_MEMORY_SEL, 0xff, 0);
	pio->set_context_port_c(keyboard, SIG_KEYBOARD_COLUMN, 0x0f, 0);
	pio->set_context_port_c(not_remote, SIG_NOT_INPUT, 0x10, 0);
	not_remote->set_context_out(drec, SIG_DATAREC_REMOTE, 1);
	pio->set_context_port_c(drec, SIG_DATAREC_MIC, 0x20, 0);
	pio->set_context_port_c(pcm, SIG_PCM1BIT_SIGNAL, 0x80, 0);
	psg->set_context_port_b(joystick, SIG_JOYSTICK_SEL, 0x40, 0);
	vdp->set_context_irq(cpu, SIG_CPU_IRQ, 1);
#if defined(_PX7)
	pio->set_context_port_c(slot2, SIG_SLOT2_MUTE, 0x10, 0);
	ldp->set_context_exv(slot2, SIG_SLOT2_EXV, 1);
	ldp->set_context_ack(slot2, SIG_SLOT2_ACK, 1);
	ldp->set_context_sound(psg, SIG_AY_3_891X_PORT_A, 0x80);
#endif
	
	joystick->set_context_psg(psg);
//	keyboard->set_context_cpu(cpu);
	keyboard->set_context_pio(pio);
	memory->set_context_slot(0, slot0);
	memory->set_context_slot(1, slot1);
	memory->set_context_slot(2, slot2);
	memory->set_context_slot(3, slot3);
#if defined(_MSX2)
	rtcif->set_context_rtc(rtc);
#endif
#if defined(_PX7)
	slot2->set_context_cpu(cpu);
	slot2->set_context_ldp(ldp);
	slot2->set_context_vdp(vdp);
#endif
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(dummy);
#if !defined(_PX7)
	cpu->set_context_bios(memory);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
#ifdef _MSX2
	io->set_iomap_range_rw(0xb4, 0xb5, rtcif);
	io->set_iomap_range_rw(0x98, 0x9b, vdp);
#else
	io->set_iomap_range_rw(0x98, 0x99, vdp);
#endif
	io->set_iomap_range_rw(0xa8, 0xab, pio);
	io->set_iomap_alias_w(0xa0, psg, 0);	// PSG ch
	io->set_iomap_alias_w(0xa1, psg, 1);	// PSG data
	io->set_iomap_alias_r(0xa2, psg, 1);	// PSG data
	io->set_iomap_range_rw(0xfc, 0xff, memory);
	
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
	vdp->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	psg->initialize_sound(rate, 3579545, samples, 0, 0);
	pcm->initialize_sound(rate, 8000);
#if defined(_PX7)
	ldp->initialize_sound(rate, samples);
#endif
}

uint16_t* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
	return event->get_sound_buffer_ptr();
}

#if defined(_PX7)
void VM::movie_sound_callback(uint8_t *buffer, long size)
{
	ldp->movie_sound_callback(buffer, size);
}
#endif

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch-- == 0) {
		psg->set_volume(1, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		pcm->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		drec->set_volume(0, decibel_l, decibel_r);
#if defined(_PX7)
	} else if(ch-- == 0) {
		ldp->set_volume(0, decibel_l, decibel_r);
#endif
	} else if(ch-- == 0) {
		drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_cart(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		slot1->open_cart(file_path);
	} else {
		slot3->open_cart(file_path);
	}
	reset();
}

void VM::close_cart(int drv)
{
	if(drv == 0) {
		slot1->close_cart();
	} else {
		slot3->close_cart();
	}
	reset();
}

bool VM::is_cart_inserted(int drv)
{
	if(drv == 0) {
		return slot1->is_cart_inserted();
	} else {
		return slot3->is_cart_inserted();
	}
}

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	bool remote = drec->get_remote();
	
	if(drec->play_tape(file_path) && remote) {
		// if machine already sets remote on, start playing now
		push_play(drv);
	}
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	bool remote = drec->get_remote();
	
	if(drec->rec_tape(file_path) && remote) {
		// if machine already sets remote on, start recording now
		push_play(drv);
	}
}

void VM::close_tape(int drv)
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
	drec->set_remote(false);
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

void VM::load_binary(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		pac2->open_rampac2(file_path);
	}
}

#if defined(_PX7)
void VM::open_laser_disc(int drv, const _TCHAR* file_path)
{
	ldp->open_disc(file_path);
}

void VM::close_laser_disc(int drv)
{
	ldp->close_disc();
}

bool VM::is_laser_disc_inserted(int drv)
{
	return ldp->is_disc_inserted();
}

uint32_t VM::is_laser_disc_accessed()
{
	return lpd->read_signal(0);
}
#else
void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	memory->open_disk(drv, file_path, bank);
}

void VM::close_floppy_disk(int drv)
{
	memory->close_disk(drv);
}

bool VM::is_floppy_disk_inserted(int drv)
{
	return memory->is_disk_inserted(drv);
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
	memory->is_disk_protected(drv, value);
}

bool VM::is_floppy_disk_protected(int drv)
{
	return memory->is_disk_protected(drv);
}

uint32_t VM::is_floppy_disk_accessed()
{
	return memory->read_signal(0);
}
#endif

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

#define STATE_VERSION	5

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

