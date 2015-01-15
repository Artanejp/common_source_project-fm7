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
#if defined(_MZ800)
#include "../not.h"
#endif
#include "../sn76489an.h"
#include "../z80pio.h"
#include "../z80sio.h"
#include "floppy.h"
#if defined(_MZ1500)
#include "psg.h"
#endif
#include "quickdisk.h"
#endif

#include "../../fileio.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
#if defined(_MZ800)
	boot_mode = config.boot_mode;
#endif
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	and_int = new AND(this, emu);
	drec = new DATAREC(this, emu);
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
	fdc = new MB8877(this, emu);	// mb8876
#if defined(_MZ800)
	not_pit = new NOT(this, emu);
	psg = new SN76489AN(this, emu);
#elif defined(_MZ1500)
	psg_l = new SN76489AN(this, emu);
	psg_r = new SN76489AN(this, emu);
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
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
#if defined(_MZ800)
	event->set_context_sound(psg);
#elif defined(_MZ1500)
	event->set_context_sound(psg_l);
	event->set_context_sound(psg_r);
#endif
	
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
	pio->set_context_port_c(drec, SIG_DATAREC_OUT, 0x02, 0);
	// 8255:PC2 -> (N)AND -> Z80:INT
	// 8255:PC3 -> DATA RECORDER:MOTOR ON/OFF
	pio->set_context_port_c(drec, SIG_DATAREC_TRIG, 0x08, 0);
	// 8255:PC4 <- DATA RECORDER:MOTOR REMOTE
	drec->set_context_remote(pio, SIG_I8255_PORT_C, 0x10);
	// 8255:PC5 <- DATA RECORDER:READ DATA
	drec->set_context_out(pio, SIG_I8255_PORT_C, 0x20);
	// 8255:PC6 <- MEMORY:556 OUT (1.5KHz)
	// 8255:PC7 <- MEMORY:VBLANK
	
#if defined(_MZ800) || defined(_MZ1500)
	// Z80PIO:PA0 <- PRINTER:RDA
	// Z80PIO:PA1 <- PRINTER:STA
	// Z80PIO:PA2 <- GND
	// Z80PIO:PA3 <- GND
#if defined(_MZ800)
	// Z80PIO:PA4 <- NOT <- 8253:OUT#0
	// Z80PIO:PA5 <- HBLANK
	memory->set_context_pio_int(pio_int);
#elif defined(_MZ1500)
	// Z80PIO:PA4 <- 8253:OUT#0
	// Z80PIO:PA5 <- 8253:OUT#2
#endif
	// Z80PIO:PA6 -> PRINTER:IRT
	// Z80PIO:PA7 -> PRINTER:RDP
#endif
	
#if defined(_MZ800) || defined(_MZ1500)
	// Z80SIO:RTSA -> QD:WRGA
	sio_qd->set_context_rts0(qd, QUICKDISK_SIO_RTSA, 1);
	// Z80SIO:DTRB -> QD:MTON
	sio_qd->set_context_dtr1(qd, QUICKDISK_SIO_DTRB, 1);
	// Z80SIO:SENDA -> QD:RECV
	sio_qd->set_context_sync0(qd, QUICKDISK_SIO_SYNC, 1);
	sio_qd->set_context_rxdone0(qd, QUICKDISK_SIO_RXDONE, 1);
	sio_qd->set_context_send0(qd, QUICKDISK_SIO_DATA);
	sio_qd->set_context_break0(qd, QUICKDISK_SIO_BREAK, 1);
	// Z80SIO:CTSA <- QD:PROTECT
	// Z80SIO:DCDA <- QD:INSERT
	// Z80SIO:DCDB <- QD:HOE
	qd->set_context_sio(sio_qd);
	
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
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2DD);
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

#if defined(_MZ800) || defined(_MZ1500)
int VM::access_lamp()
{
	uint32 status = fdc->read_signal(0) | qd->read_signal(0);
	return (status & (1 | 4)) ? 1 : (status & (2 | 8)) ? 2 : 0;
}
#endif

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	pcm->init(rate, 8000);
#if defined(_MZ800)
	psg->init(rate, 3579545, 8000);
#elif defined(_MZ1500)
	psg_l->init(rate, 3579545, 8000);
	psg_r->init(rate, 3579545, 8000);
#endif
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

void VM::play_tape(_TCHAR* file_path)
{
	drec->play_tape(file_path);
	drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
}

void VM::rec_tape(_TCHAR* file_path)
{
	drec->rec_tape(file_path);
	drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
}

void VM::close_tape()
{
	drec->close_tape();
	drec->write_signal(SIG_DATAREC_REMOTE, 0, 0);
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

#if defined(_MZ800) || defined(_MZ1500)
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

#endif

bool VM::now_skip()
{
	return event->now_skip();
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

#define STATE_VERSION	1

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
#if defined(_MZ800)
	state_fio->FputInt32(boot_mode);
#endif
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
#if defined(_MZ800)
	boot_mode = state_fio->FgetInt32();
#endif
	return true;
}

