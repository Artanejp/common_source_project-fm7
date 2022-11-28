/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2008.06.05 -

	[ virtual machine ]
*/

#include "mz700.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../and.h"
#include "../datarec.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../io.h"
#include "../noise.h"
#include "../pcm1bit.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

//#include "cmos.h"
#include "emm.h"
#include "kanji.h"
#include "keyboard.h"
#include "memory.h"
#include "ramfile.h"

#if defined(_MZ800) || defined(_MZ1500)
#include "../disk.h"
#include "../mb8877.h"
#include "../not.h"
#include "../sn76489an.h"
#include "../z80pio.h"
#include "../z80sio.h"
#include "floppy.h"
#if defined(_MZ1500)
#include "../mz1p17.h"
#include "../prnfile.h"
#include "psg.h"
#endif
#include "quickdisk.h"
#endif
#if defined(_MZ700) || defined(_MZ1500)
#include "joystick.h"
#endif

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
#if defined(_MZ800)
	boot_mode = config.boot_mode;
#endif
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	and_int = new AND(this, emu);
	and_int->set_device_name(_T("AND Gate (IRQ)"));
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
	pit = new I8253(this, emu);
	pio = new I8255(this, emu);
	io = new IO(this, emu);
	pcm = new PCM1BIT(this, emu);
	cpu = new Z80(this, emu);
	
//	cmos = new CMOS(this, emu);
	emm = new EMM(this, emu);
	kanji = new KANJI(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	ramfile = new RAMFILE(this, emu);
	
#if defined(_MZ800) || defined(_MZ1500)
	and_snd = new AND(this, emu);
	and_snd->set_device_name(_T("AND Gate (Sound)"));
	fdc = new MB8877(this, emu);	// mb8876
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
#if defined(_MZ800)
	not_pit = new NOT(this, emu);
	not_pit->set_device_name(_T("NOT Gate (PIT)"));
	psg = new SN76489AN(this, emu);
#elif defined(_MZ1500)
	if(config.printer_type == 0) {
		printer = new PRNFILE(this, emu);
	} else if(config.printer_type == 1) {
		printer = new MZ1P17(this, emu);
	} else {
		printer = dummy;
	}
	not_reset = new NOT(this, emu);
	not_reset->set_device_name(_T("NOT Gate (Reset)"));
	not_strobe = new NOT(this, emu);
	not_strobe->set_device_name(_T("NOT Gate (Strobe)"));
	psg_l = new SN76489AN(this, emu);
	psg_l->set_device_name(_T("SN76489AN PSG (Left)"));
	psg_r = new SN76489AN(this, emu);
	psg_r->set_device_name(_T("SN76489AN PSG (Right)"));
#endif
	pio_int = new Z80PIO(this, emu);
	sio_rs = new Z80SIO(this, emu);
	sio_qd = new Z80SIO(this, emu);
	
	floppy = new FLOPPY(this, emu);
#if defined(_MZ1500)
	psg = new PSG(this, emu);
#endif
	qd = new QUICKDISK(this, emu);
#endif
#if defined(_MZ700) || defined(_MZ1500)
	joystick = new JOYSTICK(this, emu);
#endif
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
#if defined(_MZ800)
	event->set_context_sound(psg);
#elif defined(_MZ1500)
	event->set_context_sound(psg_l);
	event->set_context_sound(psg_r);
#endif
	event->set_context_sound(drec);
#if defined(_MZ800) || defined(_MZ1500)
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
#endif
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	// VRAM/PCG wait
	memory->set_context_cpu(cpu);
	
	// memory mapped I/O
	memory->set_context_pio(pio);
	memory->set_context_pit(pit);
	
#if defined(_MZ1500)
	// psg mixer
	psg->set_context_psg_l(psg_l);
	psg->set_context_psg_r(psg_r);
#endif
	
#if defined(_MZ800)
	// 8253:CLK#0 <- 1.10MHz
	pit->set_constant_clock(0, 1100000);
#else
	// 8253:CLK#0 <- 895KHz
	pit->set_constant_clock(0, CPU_CLOCKS / 4);
	memory->set_context_joystick(joystick);
#endif
	
#if defined(_MZ800) || defined(_MZ1500)
	// 8253:OUT#0 AND 8255:PC0 -> SPEAKER
	pit->set_context_ch0(and_snd, SIG_AND_BIT_0, 1);
	pio->set_context_port_c(and_snd, SIG_AND_BIT_1, 1, 0);
	and_snd->set_context_out(pcm, SIG_PCM1BIT_SIGNAL, 1);
	and_snd->set_mask(SIG_AND_BIT_0 | SIG_AND_BIT_1);
#else
	// 8253:OUT#0 -> SPEAKER
	pit->set_context_ch0(pcm, SIG_PCM1BIT_SIGNAL, 1);
#endif
#if defined(_MZ800)
	// 8253:OUT#0 -> NOT -> Z80PIO:PA4
	pit->set_context_ch0(not_pit, SIG_NOT_INPUT, 1);
	not_pit->set_context_out(pio_int, SIG_Z80PIO_PORT_A, 0x10);
#elif defined(_MZ1500)
	// 8253:OUT#0 -> Z80PIO:PA4
	pit->set_context_ch0(pio_int, SIG_Z80PIO_PORT_A, 0x10);
#endif
	
	// 8253:CLK#1 <- 15.7KHz
	pit->set_constant_clock(1, CPU_CLOCKS / 228);
	
	// 8253:OUT#1 -> 8253:CLK#2
	pit->set_context_ch1(pit, SIG_I8253_CLOCK_2, 1);
	
	// 8253:OUT#2 (N)AND 8255:PC2 -> Z80:INT
	pit->set_context_ch2(and_int, SIG_AND_BIT_0, 1);
	pio->set_context_port_c(and_int, SIG_AND_BIT_1, 4, 0);
	and_int->set_context_out(cpu, SIG_CPU_IRQ, 1);
	and_int->set_mask(SIG_AND_BIT_0 | SIG_AND_BIT_1);
	
#if defined(_MZ1500)
	// 8253:OUT#2 -> Z80PIO:PA5
	pit->set_context_ch2(pio_int, SIG_Z80PIO_PORT_A, 0x20);
#endif
	
	// 8255:PA0-3 -> KEYBOARD:STROBE
	pio->set_context_port_a(keyboard, SIG_KEYBOARD_COLUMN, 0x0f, 0);
#if defined(_MZ800)
	// 8255:PA4 -> JOYSTICK #1
	// 8255:PA5 -> JOYSTICK #2
#endif
	// 8255:PA7 -> 556 RESET
	
	// 8255:PB0-7 <- KEYBOARD:DATA
	keyboard->set_context_pio(pio);
	
	// 8255:PC0 -> AND -> SPEAKER
	// 8255:PC1 -> DATA RECORDER:WRITE DATA
	pio->set_context_port_c(drec, SIG_DATAREC_MIC, 0x02, 0);
	// 8255:PC2 -> (N)AND -> Z80:INT
	// 8255:PC3 -> DATA RECORDER:MOTOR ON/OFF
	pio->set_context_port_c(drec, SIG_DATAREC_TRIG, 0x08, 0);
	// 8255:PC4 <- DATA RECORDER:MOTOR REMOTE
	drec->set_context_remote(pio, SIG_I8255_PORT_C, 0x10);
	// 8255:PC5 <- DATA RECORDER:READ DATA
	drec->set_context_ear(pio, SIG_I8255_PORT_C, 0x20);
	// 8255:PC6 <- MEMORY:556 OUT (1.5KHz)
	// 8255:PC7 <- MEMORY:VBLANK
	
#if defined(_MZ800) || defined(_MZ1500)
	// Z80PIO:PA2 <- GND
	// Z80PIO:PA3 <- GND
#if defined(_MZ800)
	// Z80PIO:PA4 <- NOT <- 8253:OUT#0
	// Z80PIO:PA5 <- HBLANK
	memory->set_context_pio_int(pio_int);
#elif defined(_MZ1500)
	// Z80PIO:PA0 <- PRINTER:RDA (BUSY)
	// Z80PIO:PA1 <- PRINTER:STA (PE)
	if(config.printer_type == 0) {
		PRNFILE *prnfile = (PRNFILE *)printer;
		prnfile->set_context_busy(pio_int, SIG_Z80PIO_PORT_A, 0x01);
	} else if(config.printer_type == 1) {
		MZ1P17 *mz1p17 = (MZ1P17 *)printer;
		mz1p17->mode = MZ1P17_MODE_MZ2;
		mz1p17->set_context_busy(pio_int, SIG_Z80PIO_PORT_A, 0x01);
	}
	// Z80PIO:PA4 <- 8253:OUT#0
	// Z80PIO:PA5 <- 8253:OUT#2
	// Z80PIO:PA6 -> NOT -> PRINTER:IRT (RESET)
	// Z80PIO:PA7 -> NOT -> PRINTER:RDP (STROBE)
	// Z80PIO:PB  -> PRINTER:DATA
	pio_int->set_context_port_a(not_reset, SIG_NOT_INPUT, 0x40, 0);
	not_reset->set_context_out(printer, SIG_PRINTER_RESET, 0x01);
	pio_int->set_context_port_a(not_strobe, SIG_NOT_INPUT, 0x80, 0);
	not_strobe->set_context_out(printer, SIG_PRINTER_STROBE, 0x01);
	pio_int->set_context_port_b(printer, SIG_PRINTER_DATA, 0xff, 0);
#endif
#endif
	
#if defined(_MZ800) || defined(_MZ1500)
	// Z80SIO:RTSA -> QD:WRGA
	sio_qd->set_context_rts(0, qd, QUICKDISK_SIO_RTSA, 1);
	// Z80SIO:DTRB -> QD:MTON
	sio_qd->set_context_dtr(1, qd, QUICKDISK_SIO_DTRB, 1);
	// Z80SIO:SENDA -> QD:RECV
	sio_qd->set_context_sync(0, qd, QUICKDISK_SIO_SYNC, 1);
	sio_qd->set_context_rxdone(0, qd, QUICKDISK_SIO_RXDONE, 1);
	sio_qd->set_context_send(0, qd, QUICKDISK_SIO_DATA);
	sio_qd->set_context_break(0, qd, QUICKDISK_SIO_BREAK, 1);
	// Z80SIO:CTSA <- QD:PROTECT
	// Z80SIO:DCDA <- QD:INSERT
	// Z80SIO:DCDB <- QD:HOE
	qd->set_context_sio(sio_qd);
	
	sio_rs->set_tx_clock(0, 1200 * 16);	// 1200 baud
	sio_rs->set_rx_clock(0, 1200 * 16);	// baud-rate can be changed by jumper pin
	sio_rs->set_tx_clock(1, 1200 * 16);
	sio_rs->set_rx_clock(1, 1200 * 16);
	
	sio_qd->set_tx_clock(0, 101562.5);
	sio_qd->set_rx_clock(0, 101562.5);
	sio_qd->set_tx_clock(1, 101562.5);
	sio_qd->set_rx_clock(1, 101562.5);
	
	// floppy drives
	floppy->set_context_cpu(cpu);
	floppy->set_context_fdc(fdc);
	fdc->set_context_drq(floppy, SIG_FLOPPY_DRQ, 1);
#endif
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
#if defined(_MZ800) || defined(_MZ1500)
	cpu->set_context_intr(pio_int);
	// z80 family daisy chain
	// 0=8253:OUT2
	pio_int->set_context_intr(cpu, 1);
	pio_int->set_context_child(sio_rs);
	sio_rs->set_context_intr(cpu, 2);
	sio_rs->set_context_child(sio_qd);
	sio_qd->set_context_intr(cpu, 3);
#else
	cpu->set_context_intr(dummy);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// emm
	io->set_iomap_range_rw(0x00, 0x03, emm);
	// kanji
	io->set_iomap_range_rw(0xb8, 0xb9, kanji);
	// ramfile
	io->set_iomap_range_rw(0xea, 0xeb, ramfile);
	// cmos
//	io->set_iomap_range_rw(0xf8, 0xfa, cmos);
	
#if defined(_MZ800)
	// 8255/8253
	io->set_iomap_range_rw(0xd0, 0xd3, pio);
	io->set_iomap_range_rw(0xd4, 0xd7, pit);
#endif
	
#if defined(_MZ800) || defined(_MZ1500)
	// floppy drives
	io->set_iomap_range_rw(0xd8, 0xdb, fdc);
	io->set_iomap_range_w(0xdc, 0xdf, floppy);
#endif
	
	// memory mapper
#if defined(_MZ800)
	io->set_iomap_range_r(0xe0, 0xe1, memory);
	io->set_iomap_range_w(0xe0, 0xe6, memory);
#elif defined(_MZ1500)
	io->set_iomap_range_w(0xe0, 0xe6, memory);
	io->set_iovalue_single_r(0xe8, 0xef); // bit4=0: voice board is missing
#else
	io->set_iomap_range_w(0xe0, 0xe4, memory);
#endif
	
#if defined(_MZ800)
	// crtc
	io->set_iomap_range_w(0xcc, 0xcf, memory);
	io->set_iomap_single_r(0xce, memory);
	// palette
	io->set_iomap_single_w(0xf0, memory);
#elif defined(_MZ1500)
	// palette
	io->set_iomap_range_w(0xf0, 0xf1, memory);
#endif
	
#if defined(_MZ800)
	// joystick
//	io->set_iomap_range_r(0xf0, 0xf1, joystick);
#endif
	
	// psg
#if defined(_MZ800)
	io->set_iomap_single_w(0xf2, psg);
#elif defined(_MZ1500)
	io->set_iomap_single_w(0xe9, psg);
	io->set_iomap_single_w(0xf2, psg_l);
	io->set_iomap_single_w(0xf3, psg_r);
#endif
	
#if defined(_MZ800) || defined(_MZ1500)
	// z80pio/sio
	// z80pio and z80sio*2
	static const int z80_sio_addr[4] = {0, 2, 1, 3};
	static const int z80_pio_addr[4] = {1, 3, 0, 2};
	for(int i = 0; i < 4; i++) {
		io->set_iomap_alias_rw(0xb0 + i, sio_rs, z80_sio_addr[i]);
		io->set_iomap_alias_rw(0xf4 + i, sio_qd, z80_sio_addr[i]);
		io->set_iomap_alias_rw(0xfc + i, pio_int, z80_pio_addr[i]);
	}
#else
	// printer
	io->set_iovalue_single_r(0xfe, 0xc0);
#endif
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
#if defined(_MZ800) || defined(_MZ1500)
	for(int drv = 0; drv < MAX_DRIVE; drv++) {
//		if(config.drive_type) {
			fdc->set_drive_type(drv, DRIVE_TYPE_2DD);
//		} else {
//			fdc->set_drive_type(drv, DRIVE_TYPE_2D);
//		}
	}
#endif
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
	and_int->write_signal(SIG_AND_BIT_0, 0, 1);	// CLOCK = L
	and_int->write_signal(SIG_AND_BIT_1, 1, 1);	// INTMASK = H
#if defined(_MZ800) || defined(_MZ1500)
	and_snd->write_signal(SIG_AND_BIT_1, 1, 1);	// SNDMASK = H
#endif
#if defined(_MZ1500)
	pio_int->write_signal(SIG_Z80PIO_PORT_A, 0x02, 0x03);	// BUSY = L, PE = H
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

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	pcm->initialize_sound(rate, 8000);
#if defined(_MZ800)
	psg->initialize_sound(rate, 3579545, 8000);
#elif defined(_MZ1500)
	psg_l->initialize_sound(rate, 3579545, 8000);
	psg_r->initialize_sound(rate, 3579545, 8000);
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
#if defined(_MZ800)
	if(ch-- == 0) {
		psg->set_volume(0, decibel_l, decibel_r);
	} else
#elif defined(_MZ1500)
	if(ch-- == 0) {
		psg_l->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		psg_r->set_volume(0, decibel_l, decibel_r);
	} else
#endif
	if(ch-- == 0) {
		pcm->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		drec->set_volume(0, decibel_l, decibel_r);
#if defined(_MZ800) || defined(_MZ1500)
	} else if(ch-- == 0) {
		fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
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

#if defined(_MZ800) || defined(_MZ1500)
void VM::open_quick_disk(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		qd->open_disk(file_path);
	}
}

void VM::close_quick_disk(int drv)
{
	if(drv == 0) {
		qd->close_disk();
	}
}

bool VM::is_quick_disk_inserted(int drv)
{
	if(drv == 0) {
		return qd->is_disk_inserted();
	} else {
		return false;
	}
}

uint32_t VM::is_quick_disk_accessed()
{
	return qd->read_signal(0);
}

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	fdc->open_disk(drv, file_path, bank);
	
	if(fdc->get_media_type(drv) == MEDIA_TYPE_2DD) {
		if(fdc->get_drive_type(drv) == DRIVE_TYPE_2D) {
			fdc->set_drive_type(drv, DRIVE_TYPE_2DD);
		}
	} else if(fdc->get_media_type(drv) == MEDIA_TYPE_2D) {
		if(fdc->get_drive_type(drv) == DRIVE_TYPE_2DD) {
			fdc->set_drive_type(drv, DRIVE_TYPE_2D);
		}
	}
}

void VM::close_floppy_disk(int drv)
{
	fdc->close_disk(drv);
}

bool VM::is_floppy_disk_inserted(int drv)
{
	return fdc->is_disk_inserted(drv);
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
	fdc->is_disk_protected(drv, value);
}

bool VM::is_floppy_disk_protected(int drv)
{
	return fdc->is_disk_protected(drv);
}

uint32_t VM::is_floppy_disk_accessed()
{
	return fdc->read_signal(0);
}
#endif

bool VM::is_frame_skippable()
{
	return event->is_frame_skippable();
}

void VM::update_config()
{
#if defined(_MZ800)
	if(boot_mode != config.boot_mode) {
		// boot mode is changed !!!
		boot_mode = config.boot_mode;
		reset();
	} else {
#endif
		for(DEVICE* device = first_device; device; device = device->next_device) {
			device->update_config();
		}
#if defined(_MZ800)
	}
#endif
}

#define STATE_VERSION	3

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
#if defined(_MZ800)
	state_fio->StateValue(boot_mode);
#endif
	return true;
}

