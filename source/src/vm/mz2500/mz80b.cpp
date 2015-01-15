/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2013.03.14-

	[ virtual machine ]
*/

#include "mz80b.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../disk.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../io.h"
#include "../mb8877.h"
#include "../pcm1bit.h"
#include "../z80.h"
#include "../z80pio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "cmt.h"
#include "floppy.h"
#include "keyboard.h"
#include "memory80b.h"
#include "mz1r12.h"
#include "mz1r13.h"
#include "timer.h"

#ifdef SUPPORT_QUICK_DISK
#include "../z80sio.h"
#include "../mz700/quickdisk.h"
#endif

#ifdef SUPPORT_16BIT_BOARD
#include "../i286.h"
#include "../i8259.h"
#include "mz1m01.h"
#endif

#include "../../fileio.h"

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
	pit = new I8253(this, emu);
	pio_i = new I8255(this, emu);
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);
	pcm = new PCM1BIT(this, emu);
	cpu = new Z80(this, emu);
	pio = new Z80PIO(this, emu);
	
	cmt = new CMT(this, emu);
	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	mz1r12 = new MZ1R12(this, emu);
	mz1r13 = new MZ1R13(this, emu);
	timer = new TIMER(this, emu);
	
#ifdef SUPPORT_QUICK_DISK
	sio = new Z80SIO(this, emu);
	qd = new QUICKDISK(this, emu);
#endif
	
#ifdef SUPPORT_16BIT_BOARD
	pio_to16 = new Z80PIO(this, emu);
	cpu_16 = new I286(this, emu);	// 8088
	pic_16 = new I8259(this, emu);
	mz1m01 = new MZ1M01(this, emu);
#endif
	
	// set contexts
	event->set_context_cpu(cpu);
#ifdef SUPPORT_16BIT_BOARD
	event->set_context_cpu(cpu_16, 5000000);
#endif
	event->set_context_sound(pcm);
	
	drec->set_context_out(cmt, SIG_CMT_OUT, 1);
	drec->set_context_remote(cmt, SIG_CMT_REMOTE, 1);
	drec->set_context_end(cmt, SIG_CMT_END, 1);
	drec->set_context_top(cmt, SIG_CMT_TOP, 1);
	
	pit->set_context_ch0(pit, SIG_I8253_CLOCK_1, 1);
	pit->set_context_ch1(pit, SIG_I8253_CLOCK_2, 1);
	pit->set_constant_clock(0, 31250);
	pio_i->set_context_port_a(cmt, SIG_CMT_PIO_PA, 0xff, 0);
	pio_i->set_context_port_a(memory, SIG_CRTC_REVERSE, 0x10, 0);
	pio_i->set_context_port_c(cmt, SIG_CMT_PIO_PC, 0xff, 0);
	pio_i->set_context_port_c(pcm, SIG_PCM1BIT_SIGNAL, 0x04, 0);
	pio->set_context_port_a(memory, SIG_MEMORY_VRAM_SEL, 0xc0, 0);
	pio->set_context_port_a(memory, SIG_CRTC_WIDTH80, 0x20, 0);
	pio->set_context_port_a(keyboard, SIG_KEYBOARD_COLUMN, 0x1f, 0);
	
	cmt->set_context_pio(pio_i);
	cmt->set_context_drec(drec);
	floppy->set_context_fdc(fdc);
	keyboard->set_context_pio_i(pio_i);
	keyboard->set_context_pio(pio);
	memory->set_context_cpu(cpu);
	memory->set_context_pio(pio_i);
	timer->set_context_pit(pit);
	
#ifdef SUPPORT_QUICK_DISK
	// Z80SIO:RTSA -> QD:WRGA
	sio->set_context_rts0(qd, QUICKDISK_SIO_RTSA, 1);
	// Z80SIO:DTRB -> QD:MTON
	sio->set_context_dtr1(qd, QUICKDISK_SIO_DTRB, 1);
	// Z80SIO:SENDA -> QD:RECV
	sio->set_context_sync0(qd, QUICKDISK_SIO_SYNC, 1);
	sio->set_context_rxdone0(qd, QUICKDISK_SIO_RXDONE, 1);
	sio->set_context_send0(qd, QUICKDISK_SIO_DATA);
	sio->set_context_break0(qd, QUICKDISK_SIO_BREAK, 1);
	// Z80SIO:CTSA <- QD:PROTECT
	// Z80SIO:DCDA <- QD:INSERT
	// Z80SIO:DCDB <- QD:HOE
	qd->set_context_sio(sio);
#endif
	
#ifdef SUPPORT_16BIT_BOARD
	pio_to16->set_context_port_a(mz1m01, SIG_MZ1M01_PORT_A, 0xff, 0);
	pio_to16->set_context_port_b(mz1m01, SIG_MZ1M01_PORT_B, 0x80, 0);
	pio_to16->set_context_ready_a(pic_16, SIG_I8259_IR0, 1);
	pio_to16->set_context_ready_b(pic_16, SIG_I8259_IR1, 1);
	pio_to16->set_context_port_b(pic_16, SIG_I8259_IR2, 0x80, 0);
	pio_to16->set_hand_shake(0, true);
	pio_to16->set_hand_shake(1, true);
	pic_16->set_context_cpu(cpu_16);
	cpu_16->set_context_mem(mz1m01);
	cpu_16->set_context_io(mz1m01);
	cpu_16->set_context_intr(pic_16);
#ifdef USE_DEBUGGER
	cpu_16->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	mz1m01->set_context_cpu(cpu_16);
	mz1m01->set_context_pic(pic_16);
	mz1m01->set_context_pio(pio_to16);
#endif
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	
	// z80 family daisy chain
#ifdef SUPPORT_16BIT_BOARD
	// FIXME: Z80PIO on MZ-1M01 is not daisy-chained to other Z80 family !!!
	cpu->set_context_intr(pio_to16);
	pio_to16->set_context_intr(cpu, 0);
	pio_to16->set_context_child(pio);
#else
	cpu->set_context_intr(pio);
#endif
	pio->set_context_intr(cpu, 1);
#ifdef SUPPORT_QUICK_DISK
	pio->set_context_child(sio);
	sio->set_context_intr(cpu, 2);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_range_rw(0xb8, 0xbb, mz1r13);
#ifdef SUPPORT_QUICK_DISK
	io->set_iomap_alias_rw(0xd0, sio, 0);
	io->set_iomap_alias_rw(0xd1, sio, 2);
	io->set_iomap_alias_rw(0xd2, sio, 1);
	io->set_iomap_alias_rw(0xd3, sio, 3);
#endif
#ifdef SUPPORT_16BIT_BOARD
	io->set_iomap_range_rw(0xd4, 0xd7, pio_to16);
#endif
	io->set_iomap_range_rw(0xd8, 0xdb, fdc);
	io->set_iomap_range_w(0xdc, 0xdd, floppy);
	io->set_iomap_range_rw(0xe0, 0xe3, pio_i);
	io->set_iomap_range_rw(0xe4, 0xe7, pit);
	io->set_iomap_range_rw(0xe8, 0xeb, pio);
	io->set_iomap_range_w(0xf0, 0xf3, timer);
	io->set_iomap_range_w(0xf4, 0xf7, memory);
	io->set_iomap_range_rw(0xf8, 0xfa, mz1r12);
	
	io->set_iowait_range_rw(0xd8, 0xdf, 1);
	io->set_iowait_range_rw(0xe8, 0xeb, 1);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int i = 0; i < MAX_DRIVE; i++) {
//		fdc->set_drive_type(i, DRIVE_TYPE_2DD);
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

void VM::special_reset()
{
	// reset all devices
//	for(DEVICE* device = first_device; device; device = device->next_device) {
//		device->special_reset();
//	}
	memory->special_reset();
	cpu->reset();
#ifdef SUPPORT_16BIT_BOARD
	pio_to16->reset();
	cpu_16->reset();
	pic_16->reset();
	mz1m01->reset();
#endif
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
#ifdef SUPPORT_16BIT_BOARD
	} else if(index == 1) {
		return cpu_16;
#endif
	}
	return NULL;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	memory->draw_screen();
}

int VM::access_lamp()
{
#ifdef SUPPORT_QUICK_DISK
	uint32 status = fdc->read_signal(0) | qd->read_signal(0);
#else
	uint32 status = fdc->read_signal(0);
#endif
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
	pcm->init(rate, 8000);
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

#ifdef SUPPORT_QUICK_DISK
void VM::open_quickdisk(int drv, _TCHAR* file_path)
{
	if(drv == 0) {
		qd->open_disk(file_path);
	}
}

void VM::close_quickdisk(int drv)
{
	if(drv == 0) {
		qd->close_disk();
	}
}

bool VM::quickdisk_inserted(int drv)
{
	if(drv == 0) {
		return qd->disk_inserted();
	} else {
		return false;
	}
}
#endif

void VM::play_tape(_TCHAR* file_path)
{
	if(check_file_extension(file_path, _T(".dat"))) {
		memory->load_dat_image(file_path);
		return;
	} else if(check_file_extension(file_path, _T(".mzt"))) {
		if(config.direct_load_mzt && memory->load_mzt_image(file_path)) {
			return;
		}
	} else if(check_file_extension(file_path, _T(".mtw"))) {
		memory->load_mzt_image(file_path);
	}
	bool value = drec->play_tape(file_path);
	cmt->close_tape();
	cmt->play_tape(value);
}

void VM::rec_tape(_TCHAR* file_path)
{
	bool value = drec->rec_tape(file_path);
	cmt->close_tape();
	cmt->rec_tape(value);
}

void VM::close_tape()
{
	drec->close_tape();
	cmt->close_tape();
}

bool VM::tape_inserted()
{
	return drec->tape_inserted();
}

void VM::push_play()
{
	drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
}

void VM::push_stop()
{
	drec->write_signal(SIG_DATAREC_REMOTE, 0, 0);
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

#define STATE_VERSION	1

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
}

bool VM::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(!device->load_state(state_fio)) {
			return false;
		}
	}
	return true;
}

