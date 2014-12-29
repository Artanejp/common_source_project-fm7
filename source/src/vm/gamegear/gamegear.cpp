/*
	SEGA GAME GEAR Emulator 'yaGAME GEAR'

	Author : tanam
	Date   : 2013.08.24-

	[ virtual machine ]
*/

#include "gamegear.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../disk.h"
#include "../i8251.h"
#include "../i8255.h"
#include "../io.h"
#include "../sn76489an.h"
#include "../315-5124.h"
#include "../upd765a.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

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
	
	drec = new DATAREC(this, emu);
	sio = new I8251(this, emu);
	pio_k = new I8255(this, emu);
	pio_f = new I8255(this, emu);
	io = new IO(this, emu);
	psg = new SN76489AN(this, emu);
	vdp = new _315_5124(this, emu);
	fdc = new UPD765A(this, emu);
	cpu = new Z80(this, emu);
	
	key = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	system = new SYSTEM(this, emu);

	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	
	drec->set_context_out(pio_k, SIG_I8255_PORT_B, 0x80);
	pio_k->set_context_port_c(key, SIG_KEYBOARD_COLUMN, 0x07, 0);
	pio_k->set_context_port_c(drec, SIG_DATAREC_REMOTE, 0x08, 0);
	pio_k->set_context_port_c(drec, SIG_DATAREC_OUT, 0x10, 0);
	pio_f->set_context_port_c(fdc, SIG_UPD765A_MOTOR_NEG, 2, 0);
	pio_f->set_context_port_c(fdc, SIG_UPD765A_TC, 4, 0);
	pio_f->set_context_port_c(fdc, SIG_UPD765A_RESET, 8, 0);
	pio_f->set_context_port_c(memory, SIG_MEMORY_SEL, 0x40, 0);
	fdc->set_context_irq(pio_f, SIG_I8255_PORT_A, 1);
	fdc->set_context_index(pio_f, SIG_I8255_PORT_A, 4);
	
	key->set_context_cpu(cpu);
	key->set_context_pio(pio_k);
	system->set_context_key(key);
	vdp->set_context_psg(psg);
	vdp->set_context_key(key);
///	vdp->set_context_cpu(cpu);

	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(system);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif

	// i/o bus
	io->set_iomap_range_rw(0x00, 0x06, system);	// GG  START
	io->set_iomap_single_w(0x80, system);		// COL TENKEY
	io->set_iomap_single_w(0xc0, system);		// COL JOYPAD
	io->set_iomap_range_rw(0xfc, 0xfe, system);	// COL JOYPAD
	io->set_iomap_range_rw(0xff, 0xff, psg);	// COL PSG
	io->set_iomap_range_rw(0x7e, 0x7f, vdp);	// SG  VDP
	io->set_iomap_range_rw(0xbe, 0xbf, vdp);	// SG  VDP
	io->set_iomap_range_rw(0xdc, 0xdf, pio_k);	// SG  KEY
	io->set_iomap_range_rw(0xe0, 0xe3, fdc);	// SG  FDD
	io->set_iomap_range_rw(0xe4, 0xe7, pio_f);	// SG  FDD
	io->set_iomap_range_rw(0xe8, 0xe9, sio);	// SG  SERIAL

	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}

	// BIOS
	memory->bios();

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
	psg->init(rate, 3579545, 4000);
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
// user interface
// ----------------------------------------------------------------------------

void VM::open_cart(int drv, _TCHAR* file_path)
{
	if(drv == 0) {
		memory->open_cart(file_path);
		if (strstr(file_path, ".col") || 
			strstr(file_path, ".COL")) {
				vdp->set_console(0x00);
				vdp->set_context_irq(cpu, SIG_CPU_NMI, 1);
				memory->bios();
		} else {
			vdp->set_context_irq(cpu, SIG_CPU_IRQ, 1);
			if (strstr(file_path, ".gg") || 
				strstr(file_path, ".GG")) vdp->set_console(0x40);
			else
				vdp->set_console(0x20);
		}
		reset();
	}
}

void VM::close_cart(int drv)
{
	if(drv == 0) {
		memory->close_cart();
		reset();
	}
}

bool VM::cart_inserted(int drv)
{
	if(drv == 0) {
		return memory->cart_inserted();
	} else {
		return false;
	}
}

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

