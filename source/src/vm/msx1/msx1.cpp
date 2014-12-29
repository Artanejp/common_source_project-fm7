/*
	ASCII MSX1 Emulator 'yaMSX1'
	Pioneer PX-7 Emulator 'ePX-7'

	Author : tanam
	Date   : 2013.06.29-

	modified by Takeda.Toshiya, umaiboux

	[ virtual machine ]
*/

#include "msx1.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../i8255.h"
#include "../io.h"
#include "../ld700.h"
#include "../not.h"
#include "../ym2203.h"
#include "../pcm1bit.h"
#include "../tms9918a.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "joystick.h"
#include "keyboard.h"
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
	pio = new I8255(this, emu);
	io = new IO(this, emu);
	ldp = new LD700(this, emu);
	not = new NOT(this, emu);
	psg = new YM2203(this, emu);
	pcm = new PCM1BIT(this, emu);
	vdp = new TMS9918A(this, emu);
	cpu = new Z80(this, emu);
	
	joystick = new JOYSTICK(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	slot0 = new SLOT_MAIN(this, emu);
	slot1 = new SLOT_CART(this, emu);
	slot2 = new SLOT_SUB(this, emu);
	slot3 = new SLOT_CART(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	event->set_context_sound(pcm);
	event->set_context_sound(ldp);
	
	drec->set_context_out(psg, SIG_YM2203_PORT_A, 0x80);
	pio->set_context_port_a(memory, SIG_MEMORY_SEL, 0xff, 0);
	pio->set_context_port_c(keyboard, SIG_KEYBOARD_COLUMN, 0x0f, 0);
	pio->set_context_port_c(slot2, SIG_SLOT2_MUTE, 0x10, 0);
	pio->set_context_port_c(not, SIG_NOT_INPUT, 0x10, 0);
	not->set_context_out(drec, SIG_DATAREC_REMOTE, 1);
	ldp->set_context_exv(slot2, SIG_SLOT2_EXV, 1);
	ldp->set_context_ack(slot2, SIG_SLOT2_ACK, 1);
	ldp->set_context_sound(psg, SIG_YM2203_PORT_A, 0x80);
	pio->set_context_port_c(drec, SIG_DATAREC_OUT, 0x20, 0);
	pio->set_context_port_c(pcm, SIG_PCM1BIT_SIGNAL, 0x80, 0);
	psg->set_context_port_b(joystick, SIG_JOYSTICK_SEL, 0x40, 0);
	vdp->set_context_irq(cpu, SIG_CPU_IRQ, 1);
	
	joystick->set_context_psg(psg);
	keyboard->set_context_cpu(cpu);
	keyboard->set_context_pio(pio);
	memory->set_context_slot(0, slot0);
	memory->set_context_slot(1, slot1);
	memory->set_context_slot(2, slot2);
	memory->set_context_slot(3, slot3);
	slot2->set_context_cpu(cpu);
	slot2->set_context_ldp(ldp);
	slot2->set_context_vdp(vdp);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_range_rw(0x98, 0x99, vdp);
	io->set_iomap_range_rw(0xa8, 0xab, pio);
	
	io->set_iomap_alias_w(0xa0, psg, 0);	// PSG ch
	io->set_iomap_alias_w(0xa1, psg, 1);	// PSG data
	io->set_iomap_alias_r(0xa2, psg, 1);	// PSG data
	
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
	psg->init(rate, 3579545, samples, 0, 0);
	pcm->init(rate, 8000);
	ldp->initialize_sound(rate, samples);
}

uint16* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

int VM::sound_buffer_ptr()
{
	return event->sound_buffer_ptr();
}

void VM::movie_sound_callback(uint8 *buffer, long size)
{
	ldp->movie_sound_callback(buffer, size);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_cart(int drv, _TCHAR* file_path)
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

bool VM::cart_inserted(int drv)
{
	if(drv == 0) {
		return slot1->cart_inserted();
	} else {
		return slot3->cart_inserted();
	}
}

void VM::play_tape(_TCHAR* file_path)
{
	drec->play_tape(file_path);
}

void VM::rec_tape(_TCHAR* file_path)
{
	drec->rec_tape(file_path);
}

void VM::close_tape()
{
	drec->close_tape();
}

bool VM::tape_inserted()
{
	return drec->tape_inserted();
}

void VM::open_laser_disc(_TCHAR* file_path)
{
	ldp->open_disc(file_path);
}

void VM::close_laser_disc()
{
	ldp->close_disc();
}

bool VM::laser_disc_inserted()
{
	return ldp->disc_inserted();
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

