/*
	Common Source Code Project
	SVI-3x8

	Origin : src/vm/msx/msx_ex.cpp

	modified by tanam
	Date   : 2018.12.09-

	[ virtual machine ]
*/

#include "msx_ex.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

//#include "../datarec.h"
#include "../i8255.h"
#include "../io.h"
#include "../noise.h"
#include "../not.h"
#include "../ay_3_891x.h"
#include "../pcm1bit.h"
#include "../tms9918a.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "joystick.h"
#include "keyboard.h"
#include "memory_ex.h"
#ifdef USE_PRINTER
#include "printer.h"
#include "../prnfile.h"
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
	
//	drec = new DATAREC(this, emu);
//	drec->set_context_noise_play(new NOISE(this, emu));
//	drec->set_context_noise_stop(new NOISE(this, emu));
//	drec->set_context_noise_fast(new NOISE(this, emu));
	pio = new I8255(this, emu);
	io = new IO(this, emu);
	not_remote = new NOT(this, emu);
	psg = new AY_3_891X(this, emu);
	pcm = new PCM1BIT(this, emu);
	vdp = new TMS9918A(this, emu);
	cpu = new Z80(this, emu);
	
	joystick = new JOYSTICK(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY_EX(this, emu);
#ifdef USE_PRINTER
	printer = new PRINTER(this, emu);
#endif
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	event->set_context_sound(pcm);
//	event->set_context_sound(drec);
//	event->set_context_sound(drec->get_context_noise_play());
//	event->set_context_sound(drec->get_context_noise_stop());
//	event->set_context_sound(drec->get_context_noise_fast());	
//	drec->set_context_ear(psg, SIG_AY_3_891X_PORT_A, 0x80);
	pio->set_context_port_c(keyboard, SIG_KEYBOARD_COLUMN, 0x0f, 0);
	pio->set_context_port_c(not_remote, SIG_NOT_INPUT, 0x10, 0);
//	not_remote->set_context_out(drec, SIG_DATAREC_REMOTE, 1);
//	pio->set_context_port_c(drec, SIG_DATAREC_MIC, 0x20, 0);
	pio->set_context_port_c(pcm, SIG_PCM1BIT_SIGNAL, 0x80, 0);
	psg->set_context_port_b(memory, SIG_MEMORY_SEL, 0x0f, 0);
	vdp->set_context_irq(cpu, SIG_CPU_IRQ, 1);
	
	joystick->set_context_psg(psg);
	joystick->set_context_memory(memory);
	keyboard->set_context_pio(pio);
	
#ifdef USE_PRINTER
	if(config.printer_type == 0) {  
		printer->set_context_prn(new PRNFILE(this, emu));
	} else {
		printer->set_context_prn(printer);
	}
#endif

	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_range_rw(0x30, 0x38, memory);
	io->set_iomap_range_w(0x80, 0x81, vdp);
	io->set_iomap_range_r(0x84, 0x85, vdp);
	io->set_iomap_range_w(0x94, 0x97, pio);
	io->set_iomap_range_r(0x99, 0x9a, pio);
	io->set_iomap_alias_w(0x88, psg, 0);	// PSG ch
	io->set_iomap_alias_w(0x8c, psg, 1);	// PSG data
	io->set_iomap_alias_r(0x90, psg, 1);	// STICK
	io->set_iomap_alias_r(0x98, memory, 1);	// STRIG
#ifdef USE_PRINTER
	io->set_iomap_range_rw(0x10, 0x11, printer);
#endif
	
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
	psg->set_volume(1, decibel_l, decibel_r);
	} else if(ch == 1) {
		pcm->set_volume(0, decibel_l, decibel_r);
//	} else if(ch == 2) {
//		drec->set_volume(0, decibel_l, decibel_r);
//	} else if(ch == 6) {
//		drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
//		drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
//		drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_cart(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
//		slot_cart[drv]->open_cart(file_path);
//		memory->set_context_slot(MAINROM_SLOT, slot_cart[0]);
		memory->open_cart(file_path);
	}

	reset();
}

void VM::close_cart(int drv)
{
	if(drv == 0) {
//		slot_cart[drv]->close_cart();
//		memory->set_context_slot(MAINROM_SLOT, slot_mainrom);
		memory->close_cart();
	}
	reset();
}

bool VM::is_cart_inserted(int drv)
{
	if(drv == 0) {
//		return slot_cart[drv]->is_cart_inserted();
		return memory->is_cart_inserted();
	} else {
		return false;
	}
}

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	memory->play_tape(file_path);
//	bool remote = drec->get_remote();
	
//	if(drec->play_tape(file_path) && remote) {
//		// if machine already sets remote on, start playing now
//		push_play(drv);
//	}
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
//	bool remote = drec->get_remote();
	
//	if(drec->rec_tape(file_path) && remote) {
//		// if machine already sets remote on, start recording now
//		push_play(drv);
//	}
}

void VM::close_tape(int drv)
{
	emu->lock_vm();
	memory->close_tape();
	emu->unlock_vm();
//	drec->set_remote(false);
}

bool VM::is_tape_inserted(int drv)
{
	return memory->is_tape_inserted();
}

bool VM::is_tape_playing(int drv)
{
	return false;
//	return drec->is_tape_playing();
}

bool VM::is_tape_recording(int drv)
{
	return false;
//	return drec->is_tape_recording();
}

int VM::get_tape_position(int drv)
{
	return 0;
//	return drec->get_tape_position();
}

const _TCHAR* VM::get_tape_message(int drv)
{
	return memory->get_message();
}

void VM::push_play(int drv)
{
//	drec->set_remote(false);
//	drec->set_ff_rew(0);
//	drec->set_remote(true);
}

void VM::push_stop(int drv)
{
//	drec->set_remote(false);
}

void VM::push_fast_forward(int drv)
{
//	drec->set_remote(false);
//	drec->set_ff_rew(1);
//	drec->set_remote(true);
}

void VM::push_fast_rewind(int drv)
{
//	drec->set_remote(false);
//	drec->set_ff_rew(-1);
//	drec->set_remote(true);
}

#if defined(FDD_PATCH_SLOT)
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
	return true;
}
