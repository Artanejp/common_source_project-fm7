/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.07.15-

	[ virtual machine ]
*/

#include "pc6001.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../i8255.h"
#include "../io.h"
#ifdef _PC6001
#include "../mc6847.h"
#include "display.h"
#else
#include "../upd7752.h"
#endif
#include "../pc6031.h"
#include "../pc80s31k.h"
#include "../upd765a.h"
#include "../ym2203.h"
#include "../z80.h"

#include "../datarec.h"
#include "../mcs48.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#if defined(_PC6601) || defined(_PC6601SR)
#include "floppy.h"
#endif
#include "joystick.h"
#include "memory.h"
#include "printer.h"
#include "psub.h"
#include "sub.h"
#include "timer.h"

#include "../../fileio.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	FILEIO* fio = new FILEIO();
	support_pc80s31k = fio->IsFileExists(emu->bios_path(_T("DISK.ROM")));
#ifdef _PC6601SR
	support_sub_cpu = false;
#else
	support_sub_cpu = fio->IsFileExists(emu->bios_path(_T(SUB_CPU_ROM_FILE_NAME)));
#endif
	delete fio;
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	pio_sub = new I8255(this, emu);
	io = new IO(this, emu);
	psg = new YM2203(this, emu);
	cpu = new Z80(this, emu);
	
#if defined(_PC6601) || defined(_PC6601SR)
	floppy = new FLOPPY(this, emu);
#endif
	joystick = new JOYSTICK(this, emu);
	memory = new MEMORY(this, emu);
//	printer = new PRINTER(this, emu);
	timer = new TIMER(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	
//	pio_sub->set_context_port_b(printer, SIG_PRINTER_OUT, 0xff, 0);
//	pio_sub->set_context_port_c(printer, SIG_PRINTER_STB, 0x01, 0);
	pio_sub->set_context_port_c(memory, SIG_MEMORY_PIO_PORT_C, 0x06, 0);	// CRTKILL,CGSWN
	
#ifdef _PC6001
	display = new DISPLAY(this, emu);
	vdp = new MC6847(this, emu);
	display->set_context_vdp(vdp);
	display->set_vram_ptr(memory->get_vram());
	display->set_context_timer(timer);
	vdp->load_font_image(emu->bios_path(_T("CGROM60.60")));
	vdp->set_context_cpu(cpu);
	pio_sub->set_context_port_c(vdp, SIG_MC6847_ENABLE, 0x02, 0);	// CRTKILL
#else
	voice = new UPD7752(this, emu);
	event->set_context_sound(voice);
	memory->set_context_timer(timer);
#endif
	memory->set_context_cpu(cpu);
	joystick->set_context_psg(psg);
	
	timer->set_context_cpu(cpu);
#ifndef _PC6001
	timer->set_context_memory(memory);
#endif
	if(support_sub_cpu) {
		cpu_sub = new MCS48(this, emu);
		sub = new SUB(this, emu);
		drec = new DATAREC(this, emu);
		event->set_context_cpu(cpu_sub, 8000000);
		cpu_sub->set_context_mem(new MCS48MEM(this, emu));
		cpu_sub->set_context_io(sub);
#ifdef USE_DEBUGGER
		cpu_sub->set_context_debugger(new DEBUGGER(this, emu));
#endif
		sub->set_context_pio(pio_sub);
		sub->set_context_drec(drec);
		sub->set_context_timer(timer);
		pio_sub->set_context_port_c(cpu_sub, SIG_CPU_IRQ, 0x80, 0);
		drec->set_context_out(sub, SIG_SUB_DATAREC, 1);
		timer->set_context_sub(sub);
	} else {
		psub = new PSUB(this, emu);
		psub->set_context_pio(pio_sub);
		psub->set_context_timer(timer);
		timer->set_context_sub(psub);
		cpu_sub = NULL;
	}
	if(support_pc80s31k) {
		pio_fdd = new I8255(this, emu);
		pio_pc80s31k = new I8255(this, emu);
		pc80s31k = new PC80S31K(this, emu);
		fdc_pc80s31k = new UPD765A(this, emu);
		cpu_pc80s31k = new Z80(this, emu);
		
		event->set_context_cpu(cpu_pc80s31k, 4000000);
		pc80s31k->set_context_cpu(cpu_pc80s31k);
		pc80s31k->set_context_fdc(fdc_pc80s31k);
		pc80s31k->set_context_pio(pio_pc80s31k);
		pio_fdd->set_context_port_a(pio_pc80s31k, SIG_I8255_PORT_A, 0xff, 0);
		pio_fdd->set_context_port_b(pio_pc80s31k, SIG_I8255_PORT_A, 0xff, 0);
		pio_fdd->set_context_port_c(pio_pc80s31k, SIG_I8255_PORT_C, 0x0f, 4);
		pio_fdd->set_context_port_c(pio_pc80s31k, SIG_I8255_PORT_C, 0xf0, -4);
		pio_fdd->clear_ports_by_cmdreg = true;
		pio_pc80s31k->set_context_port_a(pio_fdd, SIG_I8255_PORT_B, 0xff, 0);
		pio_pc80s31k->set_context_port_b(pio_fdd, SIG_I8255_PORT_A, 0xff, 0);
		pio_pc80s31k->set_context_port_c(pio_fdd, SIG_I8255_PORT_C, 0x0f, 4);
		pio_pc80s31k->set_context_port_c(pio_fdd, SIG_I8255_PORT_C, 0xf0, -4);
		pio_pc80s31k->clear_ports_by_cmdreg = true;
		fdc_pc80s31k->set_context_irq(cpu_pc80s31k, SIG_CPU_IRQ, 1);
		cpu_pc80s31k->set_context_mem(pc80s31k);
		cpu_pc80s31k->set_context_io(pc80s31k);
		cpu_pc80s31k->set_context_intr(pc80s31k);
#ifdef USE_DEBUGGER
		cpu_pc80s31k->set_context_debugger(new DEBUGGER(this, emu));
#endif
#if defined(_PC6601) || defined(_PC6601SR)
		floppy->set_context_ext(pio_fdd);
#endif
	} else {
		pc6031 = new PC6031(this, emu);
#if defined(_PC6601) || defined(_PC6601SR)
		floppy->set_context_ext(pc6031);
#endif
		cpu_pc80s31k = NULL;
	}
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(timer);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	if(support_sub_cpu) {
		io->set_iomap_range_rw(0x90, 0x93, sub);
	} else {
		io->set_iomap_range_rw(0x90, 0x93, psub);
	}
	io->set_iomap_alias_w(0xa0, psg, 0);			// PSG ch
	io->set_iomap_alias_w(0xa1, psg, 1);			// PSG data
	io->set_iomap_alias_r(0xa2, psg, 1);			// PSG data
#if defined(_PC6601SR) || defined(_PC6001MK2SR)
	io->set_iomap_alias_r(0xa3, psg, 0);			// FM status
	io->set_iomap_range_rw(0x40, 0x6f, memory);		// VRAM addr
#endif
#ifdef _PC6001
	io->set_iomap_single_w(0xb0, display);			// VRAM addr
	io->set_iomap_single_w(0x00, memory);			// MEMORY MAP
#else
	io->set_iomap_single_w(0xb0, memory);			// VRAM addr
	io->set_iomap_range_rw(0xc0, 0xcf, memory);		// VRAM addr
	io->set_iomap_range_rw(0xe0, 0xe3, voice);		// VOICE
#if defined(_PC6601) || defined(_PC6601SR)
	io->set_iomap_range_rw(0xb8, 0xbf, timer);		// IRQ
	io->set_iomap_range_rw(0xfa, 0xfb, timer);		// IRQ
#endif
	io->set_iomap_range_rw(0xf3, 0xf7, timer);		// IRQ/Timer
#endif
	
#if defined(_PC6601) || defined(_PC6601SR)
	io->set_iomap_range_rw(0xb1, 0xb3, floppy);		// DISK DRIVE
	io->set_iomap_range_rw(0xb5, 0xb7, floppy);		// DISK DRIVE (Mirror)
	io->set_iomap_range_rw(0xd0, 0xde, floppy);		// DISK DRIVE
#else
	io->set_iovalue_single_r(0xb1, 0x01);
#if defined(_PC6601SR)
	io->set_iovalue_single_r(0xb2, 0x02);
#elif defined(_PC6001MK2SR)
	io->set_iovalue_single_r(0xb2, 0x00);
#endif
	
	if(support_pc80s31k) {
		io->set_iomap_range_rw(0xd0, 0xd2, pio_fdd);
		io->set_iomap_single_w(0xd3, pio_fdd);
	} else {
		io->set_iomap_range_rw(0xd0, 0xd3, pc6031);
	}
#endif
	io->set_iomap_range_rw(0xf0, 0xf2, memory);		// MEMORY MAP
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	if(support_sub_cpu) {
		// load rom images after cpustate is allocated
#ifdef _PC6601SR
#else
		cpu_sub->load_rom_image(emu->bios_path(SUB_CPU_ROM_FILE_NAME));
#endif
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
	if(support_pc80s31k) {
		pio_fdd->write_signal(SIG_I8255_PORT_C, 0, 0xff);
		pio_pc80s31k->write_signal(SIG_I8255_PORT_C, 0, 0xff);
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
	} else if(index == 1) {
		return cpu_sub;
	} else if(index == 2) {
		return cpu_pc80s31k;
	}
	return NULL;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
#ifdef _PC6001
	display->draw_screen();
#else
	memory->draw_screen();
#endif
}
// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	psg->init(rate, 4000000, samples, 0, 0);
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

int VM::access_lamp()
{
	uint32 status = 0; /// fdc->read_signal(0);
#if defined(_PC6601) || defined(_PC6601SR)
	status = floppy->read_signal(0);
#endif
	if(support_pc80s31k) {
		status |= fdc_pc80s31k->read_signal(0);
	} else {
		status |= pc6031->read_signal(0);
	}
	return status;
}

void VM::open_disk(int drv, _TCHAR* file_path, int offset)
{
#if defined(_PC6601) || defined(_PC6601SR)
	if(drv < 2) {
		floppy->open_disk(drv, file_path, offset);
		return;
	} else {
		drv -= 2;
	}
#endif
	if(support_pc80s31k) {
		fdc_pc80s31k->open_disk(drv, file_path, offset);
	} else {
		pc6031->open_disk(drv, file_path, offset);
	}
}

void VM::close_disk(int drv)
{
#if defined(_PC6601) || defined(_PC6601SR)
	if(drv < 2) {
		floppy->close_disk(drv);
		return;
	} else {
		drv -= 2;
	}
#endif
	if(support_pc80s31k) {
		fdc_pc80s31k->close_disk(drv);
	} else {
		pc6031->close_disk(drv);
	}
}

bool VM::disk_inserted(int drv)
{
#if defined(_PC6601) || defined(_PC6601SR)
	if(drv < 2) {
		return floppy->disk_inserted(drv);
	} else {
		drv -= 2;
	}
#endif
	if(support_pc80s31k) {
		return fdc_pc80s31k->disk_inserted(drv);
	} else {
		return pc6031->disk_inserted(drv);
	}
}

void VM::play_tape(_TCHAR* file_path)
{
	if(support_sub_cpu) {
		drec->play_tape(file_path);
		drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
	} else {
		psub->play_tape(file_path);
	}
}

void VM::rec_tape(_TCHAR* file_path)
{
	if(support_sub_cpu) {
		sub->rec_tape(file_path);
//		drec->rec_tape(file_path);
//		drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
	} else {
		psub->rec_tape(file_path);
	}
}

void VM::close_tape()
{
	if(support_sub_cpu) {
		if(sub->tape_inserted()) {
			sub->close_tape();	// temporary
		} else {
			drec->close_tape();
			drec->write_signal(SIG_DATAREC_REMOTE, 0, 0);
		}
	} else {
		psub->close_tape();
	}
}

bool VM::tape_inserted()
{
	if(support_sub_cpu) {
		return drec->tape_inserted() || sub->tape_inserted();
	} else {
		return psub->tape_inserted();
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
