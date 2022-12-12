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

#include "../disk.h"
#include "../i8255.h"
#include "../io.h"
#ifdef _PC6001
#include "../mc6847.h"
#include "display.h"
#else
#include "../upd7752.h"
#endif
#include "../noise.h"
#include "../pc6031.h"
#include "../pc80s31k.h"
#include "../prnfile.h"
#include "../upd765a.h"
#if defined(_PC6001MK2SR) || defined(_PC6601SR)
#include "../ym2203.h"
#else
#include "../ay_3_891x.h"
#endif
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
//#include "psub.h"
#include "sub.h"
#include "timer.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	support_pc80s31k = FILEIO::IsFileExisting(create_local_path(_T("DISK.ROM")));
#ifdef _PC6601SR
	support_sub_cpu = false;
#else
	support_sub_cpu = FILEIO::IsFileExisting(create_local_path(_T(SUB_CPU_ROM_FILE_NAME)));
#endif
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	pio_sub = new I8255(this, emu);
	io = new IO(this, emu);
	io->space = 0x100;
	noise_seek = new NOISE(this, emu);
	noise_head_down = new NOISE(this, emu);
	noise_head_up = new NOISE(this, emu);
#if defined(_PC6001MK2SR) || defined(_PC6601SR)
	psg = new YM2203(this, emu);
#else
	psg = new AY_3_891X(this, emu);
#endif
#ifdef USE_DEBUGGER
	psg->set_context_debugger(new DEBUGGER(this, emu));
#endif
	cpu = new Z80(this, emu);
	
	if(config.printer_type == 0) {
		printer = new PRNFILE(this, emu);
	} else {
		printer = dummy;
	}
	
#if defined(_PC6601) || defined(_PC6601SR)
	floppy = new FLOPPY(this, emu);
	floppy->set_context_noise_seek(noise_seek);
//	floppy->set_context_noise_head_down(noise_head_down);
//	floppy->set_context_noise_head_up(noise_head_up);
#endif
	joystick = new JOYSTICK(this, emu);
	memory = new MEMORY(this, emu);
	timer = new TIMER(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	event->set_context_sound(noise_seek);
	event->set_context_sound(noise_head_down);
	event->set_context_sound(noise_head_up);
	
	pio_sub->set_context_port_b(printer, SIG_PRINTER_DATA, 0xff, 0);
	pio_sub->set_context_port_c(printer, SIG_PRINTER_STROBE, 0x01, 0);
	pio_sub->set_context_port_c(memory, SIG_MEMORY_PIO_PORT_C, 0x06, 0);	// CRTKILL,CGSWN
	
#ifdef _PC6001
	display = new DISPLAY(this, emu);
	vdp = new MC6847(this, emu);
	display->set_context_vdp(vdp);
	display->set_vram_ptr(memory->get_vram());
	display->set_context_timer(timer);
	vdp->load_font_image(create_local_path(_T("CGROM60.60")));
//	vdp->set_context_cpu(cpu);
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
		cpu_sub->set_device_name(_T("MCS48 MCU (Sub)"));
		sub = new SUB(this, emu);
		drec = new DATAREC(this, emu);
		drec->set_device_name(_T("Data Recorder (Sub)"));
		drec->set_context_noise_play(new NOISE(this, emu));
		drec->set_context_noise_stop(new NOISE(this, emu));
		drec->set_context_noise_fast(new NOISE(this, emu));
		event->set_context_cpu(cpu_sub, 8000000);
		event->set_context_sound(drec);
		event->set_context_sound(drec->get_context_noise_play());
		event->set_context_sound(drec->get_context_noise_stop());
		event->set_context_sound(drec->get_context_noise_fast());
		cpu_sub->set_context_mem(new MCS48MEM(this, emu));
		cpu_sub->set_context_io(sub);
#ifdef USE_DEBUGGER
		cpu_sub->set_context_debugger(new DEBUGGER(this, emu));
#endif
		sub->set_context_pio(pio_sub);
		sub->set_context_drec(drec);
		sub->set_context_timer(timer);
		pio_sub->set_context_port_c(cpu_sub, SIG_CPU_IRQ, 0x80, 0);
		drec->set_context_ear(sub, SIG_SUB_DATAREC, 1);
		timer->set_context_sub(sub);
//	} else {
//		psub = new PSUB(this, emu);
//		psub->set_context_pio(pio_sub);
//		psub->set_context_timer(timer);
//		timer->set_context_sub(psub);
//		cpu_sub = NULL;
	}
	if(support_pc80s31k) {
		pio_fdd = new I8255(this, emu);
		pio_fdd->set_device_name(_T("8255 PIO (FDD I/F)"));
		pio_pc80s31k = new I8255(this, emu);
		pio_pc80s31k->set_device_name(_T("8255 PIO (320KB FDD)"));
		pc80s31k = new PC80S31K(this, emu);
		pc80s31k->set_device_name(_T("PC-80S31K (320KB FDD)"));
		fdc_pc80s31k = new UPD765A(this, emu);
		fdc_pc80s31k->set_device_name(_T("uPD765A FDC (320KB FDD)"));
		cpu_pc80s31k = new Z80(this, emu);
		cpu_pc80s31k->set_device_name(_T("Z80 CPU (320KB FDD)"));
		
		event->set_context_cpu(cpu_pc80s31k, 4000000);
		
		pc80s31k->set_context_cpu(cpu_pc80s31k);
		pc80s31k->set_context_fdc(fdc_pc80s31k);
		pc80s31k->set_context_pio(pio_pc80s31k);
		pio_fdd->set_context_port_a(pio_pc80s31k, SIG_I8255_PORT_B, 0xff, 0);
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
		fdc_pc80s31k->set_context_noise_seek(noise_seek);
		fdc_pc80s31k->set_context_noise_head_down(noise_head_down);
		fdc_pc80s31k->set_context_noise_head_up(noise_head_up);
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
		pc6031->set_context_noise_seek(noise_seek);
//		pc6031->set_context_noise_head_down(noise_head_down);
//		pc6031->set_context_noise_head_up(noise_head_up);
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
//	} else {
//		io->set_iomap_range_rw(0x90, 0x93, psub);
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
		cpu_sub->load_rom_image(create_local_path(_T(SUB_CPU_ROM_FILE_NAME)));
#endif
	}
	int drive_num = 0;
#if defined(_PC6601) || defined(_PC6601SR)
	floppy->get_disk_handler(0)->drive_num = drive_num++;
	floppy->get_disk_handler(1)->drive_num = drive_num++;
#endif
	if(support_pc80s31k) {
		fdc_pc80s31k->get_disk_handler(0)->drive_num = drive_num++;
		fdc_pc80s31k->get_disk_handler(1)->drive_num = drive_num++;
	} else {
		pc6031->get_disk_handler(0)->drive_num = drive_num++;
		pc6031->get_disk_handler(1)->drive_num = drive_num++;
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
	psg->initialize_sound(rate, 4000000, samples, 0, 0);
#ifndef _PC6001
	voice->initialize_sound(rate);
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

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch-- == 0) {
		psg->set_volume(1, decibel_l, decibel_r);
#if !defined(_PC6001)
	} else if(ch-- == 0) {
		voice->set_volume(0, decibel_l, decibel_r);
#endif
	} else if(ch-- == 0) {
		if(support_sub_cpu) {
			drec->set_volume(0, decibel_l, decibel_r);
		}
	} else if(ch-- == 0) {
		noise_seek->set_volume(0, decibel_l, decibel_r);
		noise_head_down->set_volume(0, decibel_l, decibel_r);
		noise_head_up->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		if(support_sub_cpu) {
			drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
			drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
			drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
		}
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
//	if(!support_sub_cpu) {
//		psub->key_down(code);
//	}
}

void VM::key_up(int code)
{
//	if(!support_sub_cpu) {
//		psub->key_up(code);
//	}
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_cart(int drv, const _TCHAR* file_path)
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

bool VM::is_cart_inserted(int drv)
{
	if(drv == 0) {
		return memory->is_cart_inserted();
	} else {
		return false;
	}
}

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
#if defined(_PC6601) || defined(_PC6601SR)
	if(drv < 2) {
		floppy->open_disk(drv, file_path, bank);
		return;
	} else {
		drv -= 2;
	}
#endif
	if(support_pc80s31k) {
		fdc_pc80s31k->open_disk(drv, file_path, bank);
	} else {
		pc6031->open_disk(drv, file_path, bank);
	}
}

void VM::close_floppy_disk(int drv)
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

bool VM::is_floppy_disk_inserted(int drv)
{
#if defined(_PC6601) || defined(_PC6601SR)
	if(drv < 2) {
		return floppy->is_disk_inserted(drv);
	} else {
		drv -= 2;
	}
#endif
	if(support_pc80s31k) {
		return fdc_pc80s31k->is_disk_inserted(drv);
	} else {
		return pc6031->is_disk_inserted(drv);
	}
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
#if defined(_PC6601) || defined(_PC6601SR)
	if(drv < 2) {
		floppy->is_disk_protected(drv, value);
		return;
	} else {
		drv -= 2;
	}
#endif
	if(support_pc80s31k) {
		fdc_pc80s31k->is_disk_protected(drv, value);
	} else {
		pc6031->is_disk_protected(drv, value);
	}
}

bool VM::is_floppy_disk_protected(int drv)
{
#if defined(_PC6601) || defined(_PC6601SR)
	if(drv < 2) {
		return floppy->is_disk_protected(drv);
	} else {
		drv -= 2;
	}
#endif
	if(support_pc80s31k) {
		return fdc_pc80s31k->is_disk_protected(drv);
	} else {
		return pc6031->is_disk_protected(drv);
	}
}

uint32_t VM::is_floppy_disk_accessed()
{
	uint32_t status = 0; /// fdc->read_signal(0);
	if(support_pc80s31k) {
		status |= fdc_pc80s31k->read_signal(0);
	} else {
		status |= pc6031->read_signal(0);
	}
#if defined(_PC6601) || defined(_PC6601SR)
	status <<= 2;
	status |= floppy->read_signal(0);
#endif
	return status;
}

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	if(support_sub_cpu) {
		// support both p6/p6t and wav
#if 1
		bool remote = drec->get_remote();
		
		if(drec->play_tape(file_path) && remote) {
			// if machine already sets remote on, start playing now
			push_play(drv);
		}
#else
		sub->play_tape(file_path);	// temporary
#endif
//	} else {
//		// support only p6/p6t
//		psub->play_tape(file_path);
	}
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	if(support_sub_cpu) {
		// support both p6/p6t and wav
#if 0
		bool remote = drec->get_remote();
		
		if(drec->rec_tape(file_path) && remote) {
			// if machine already sets remote on, start recording now
			push_play(drv);
		}
#else
		sub->rec_tape(file_path);	// temporary
#endif
//	} else {
//		// support both p6/p6t and wav
//		psub->rec_tape(file_path);
	}
}

void VM::close_tape(int drv)
{
	if(support_sub_cpu) {
		if(sub->is_tape_inserted()) {
			sub->close_tape();	// temporary
		} else {
			emu->lock_vm();
			drec->close_tape();
			emu->unlock_vm();
			drec->set_remote(false);
		}
//	} else {
//		psub->close_tape();
	}
}

bool VM::is_tape_inserted(int drv)
{
	if(support_sub_cpu) {
		return drec->is_tape_inserted() || sub->is_tape_inserted();
//	} else {
//		return psub->is_tape_inserted();
	}
}

bool VM::is_tape_playing(int drv)
{
	if(support_sub_cpu) {
		return drec->is_tape_playing();
	} else {
		return false;
	}
}

bool VM::is_tape_recording(int drv)
{
	if(support_sub_cpu) {
		return drec->is_tape_recording();
	} else {
		return false;
	}
}

int VM::get_tape_position(int drv)
{
	if(support_sub_cpu) {
		return drec->get_tape_position();
	} else {
		return 0;
	}
}

const _TCHAR* VM::get_tape_message(int drv)
{
	if(support_sub_cpu) {
		return drec->get_message();
	} else {
		return NULL;
	}
}

void VM::push_play(int drv)
{
	if(support_sub_cpu) {
		drec->set_remote(false);
		drec->set_ff_rew(0);
		drec->set_remote(true);
	}
}

void VM::push_stop(int drv)
{
	if(support_sub_cpu) {
		drec->set_remote(false);
	}
}

void VM::push_fast_forward(int drv)
{
	if(support_sub_cpu) {
		drec->set_remote(false);
		drec->set_ff_rew(1);
		drec->set_remote(true);
	}
}

void VM::push_fast_rewind(int drv)
{
	if(support_sub_cpu) {
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

#define STATE_VERSION	8

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
	state_fio->StateValue(sr_mode);
	return true;
}

