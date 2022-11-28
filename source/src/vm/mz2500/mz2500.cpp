/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.11.24 -

	[ virtual machine ]
*/

#include "mz2500.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../disk.h"
#include "../harddisk.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../io.h"
#include "../mb8877.h"
#include "../mz1p17.h"
#include "../noise.h"
#include "../pcm1bit.h"
//#include "../pcpr201.h"
#include "../prnfile.h"
#include "../rp5c01.h"
#include "../scsi_hdd.h"
#include "../scsi_host.h"
#include "../w3100a.h"
#include "../ym2203.h"
#include "../z80.h"
#include "../z80pio.h"
#include "../z80sio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "calendar.h"
#include "cmt.h"
#include "crtc.h"
#include "floppy.h"
#include "interrupt.h"
#include "joystick.h"
#include "keyboard.h"
#include "memory.h"
#include "mouse.h"
#include "mz1e26.h"
#include "mz1e30.h"
#include "mz1r13.h"
#include "mz1r37.h"
#include "printer.h"
#include "serial.h"
#include "timer.h"

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
	pit = new I8253(this, emu);
	pio_i = new I8255(this, emu);
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	pcm = new PCM1BIT(this, emu);
	rtc = new RP5C01(this, emu);	// RP-5C15
	sasi_host = new SCSI_HOST(this, emu);
	sasi_hdd = new SASI_HDD(this, emu);
	sasi_hdd->set_device_name(_T("SASI Hard Disk Drive"));
	sasi_hdd->scsi_id = 0;
//	sasi_hdd->bytes_per_sec = 32 * 1024; // 32KB/s
	sasi_hdd->bytes_per_sec = 3600 / 60 * 256 * 33; // 3600rpm, 256bytes x 33sectors in track (thanks Mr.Sato)
	for(int i = 0; i < USE_HARD_DISK; i++) {
		sasi_hdd->set_disk_handler(i, new HARDDISK(emu));
	}
	sasi_hdd->set_context_interface(sasi_host);
	sasi_host->set_context_target(sasi_hdd);
	w3100a = new W3100A(this, emu);
	opn = new YM2203(this, emu);
#ifdef USE_DEBUGGER
	opn->set_context_debugger(new DEBUGGER(this, emu));
#endif
	cpu = new Z80(this, emu);
	pio = new Z80PIO(this, emu);
	sio = new Z80SIO(this, emu);
	
	calendar = new CALENDAR(this, emu);
	cmt = new CMT(this, emu);
	crtc = new CRTC(this, emu);
	floppy = new FLOPPY(this, emu);
	interrupt = new INTERRUPT(this, emu);
	joystick = new JOYSTICK(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	mouse = new MOUSE(this, emu);
	mz1e26 = new MZ1E26(this, emu);
	mz1e30 = new MZ1E30(this, emu);
	mz1r13 = new MZ1R13(this, emu);
	mz1r37 = new MZ1R37(this, emu);
	printer = new PRINTER(this, emu);
	serial = new SERIAL(this, emu);
	timer = new TIMER(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(opn);
	event->set_context_sound(pcm);
	event->set_context_sound(drec);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	drec->set_context_ear(cmt, SIG_CMT_OUT, 1);
	drec->set_context_remote(cmt, SIG_CMT_REMOTE, 1);
	drec->set_context_end(cmt, SIG_CMT_END, 1);
	drec->set_context_top(cmt, SIG_CMT_TOP, 1);
	
	pit->set_context_ch0(interrupt, SIG_INTERRUPT_I8253, 1);
	pit->set_context_ch0(pit, SIG_I8253_CLOCK_1, 1);
	pit->set_context_ch1(pit, SIG_I8253_CLOCK_2, 1);
	pit->set_constant_clock(0, 31250);
	pio_i->set_context_port_a(cmt, SIG_CMT_PIO_PA, 0xff, 0);
	pio_i->set_context_port_c(cmt, SIG_CMT_PIO_PC, 0xff, 0);
	pio_i->set_context_port_c(crtc, SIG_CRTC_MASK, 0x01, 0);
	pio_i->set_context_port_c(pcm, SIG_PCM1BIT_SIGNAL, 0x04, 0);
	rtc->set_context_alarm(interrupt, SIG_INTERRUPT_RP5C15, 1);
	rtc->set_context_pulse(opn, SIG_YM2203_PORT_B, 8);
	sasi_host->set_context_irq(mz1e30, SIG_MZ1E30_IRQ, 1);
	sasi_host->set_context_drq(mz1e30, SIG_MZ1E30_DRQ, 1);
	opn->set_context_port_a(floppy, SIG_FLOPPY_REVERSE, 0x02, 0);
	opn->set_context_port_a(crtc, SIG_CRTC_PALLETE, 0x04, 0);
	opn->set_context_port_a(mouse, SIG_MOUSE_SEL, 0x08, 0);
	pio->set_context_port_a(crtc, SIG_CRTC_COLUMN_SIZE, 0x20, 0);
	pio->set_context_port_a(keyboard, SIG_KEYBOARD_COLUMN, 0x1f, 0);
	sio->set_context_dtr(1, mouse, SIG_MOUSE_DTR, 1);
	
	calendar->set_context_rtc(rtc);
	cmt->set_context_pio(pio_i);
	cmt->set_context_drec(drec);
	crtc->set_context_mem(memory);
	crtc->set_context_int(interrupt);
	crtc->set_context_pio(pio_i);
	crtc->set_vram_ptr(memory->get_vram());
	crtc->set_tvram_ptr(memory->get_tvram());
	crtc->set_kanji_ptr(memory->get_kanji());
	crtc->set_pcg_ptr(memory->get_pcg());
	floppy->set_context_fdc(fdc);
	keyboard->set_context_pio_i(pio_i);
	keyboard->set_context_pio(pio);
	memory->set_context_cpu(cpu);
	memory->set_context_crtc(crtc);
	mouse->set_context_sio(sio);
	mz1e30->set_context_host(sasi_host);
	if(config.printer_type == 0) {  
		printer->set_context_prn(new PRNFILE(this, emu));
	} else if(config.printer_type == 1) {
		MZ1P17 *mz1p17 = new MZ1P17(this, emu);
		mz1p17->mode = MZ1P17_MODE_MZ1;
		printer->set_context_prn(mz1p17);
//	} else if(config.printer_type == 2) {
//		printer->set_context_prn(new PCPR201(this, emu));
	} else {
		printer->set_context_prn(dummy);
	}
	serial->set_context_sio(sio);
	timer->set_context_pit(pit);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pio);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// z80 family daisy chain
	pio->set_context_intr(cpu, 0);
	pio->set_context_child(sio);
	sio->set_context_intr(cpu, 1);
	sio->set_context_child(interrupt);
	interrupt->set_context_intr(cpu, 2);
	
	// i/o bus
	io->set_iomap_range_rw(0x60, 0x63, w3100a);
	io->set_iomap_range_rw(0xa0, 0xa3, serial);
	io->set_iomap_range_rw(0xa4, 0xa5, mz1e30);
	io->set_iomap_range_rw(0xa8, 0xa9, mz1e30);
	io->set_iomap_range_rw(0xac, 0xad, mz1r37);
	io->set_iomap_single_w(0xae, crtc);
	io->set_iomap_range_rw(0xb0, 0xb3, serial);
	io->set_iomap_range_rw(0xb4, 0xb5, memory);
	io->set_iomap_range_rw(0xb8, 0xb9, mz1r13);
	io->set_iomap_range_rw(0xbc, 0xbf, crtc);
	io->set_iomap_range_w(0xc6, 0xc7, interrupt);
	io->set_iomap_range_rw(0xc8, 0xc9, opn);
	io->set_iomap_single_rw(0xca, mz1e26);
	io->set_iomap_single_rw(0xcc, calendar);
	io->set_iomap_single_w(0xcd, serial);
	io->set_iomap_range_w(0xce, 0xcf, memory);
	io->set_iomap_range_rw(0xd8, 0xdb, fdc);
	io->set_iomap_range_w(0xdc, 0xdd, floppy);
	io->set_iomap_range_rw(0xe0, 0xe3, pio_i);
	io->set_iomap_range_rw(0xe4, 0xe7, pit);
	io->set_iomap_range_rw(0xe8, 0xeb, pio);
	io->set_iomap_single_rw(0xef, joystick);
	io->set_iomap_range_w(0xf0, 0xf3, timer);
	io->set_iomap_range_rw(0xf4, 0xf7, crtc);
	io->set_iomap_range_rw(0xfe, 0xff, printer);
	
	io->set_iowait_range_rw(0xc8, 0xc9, 1);
	io->set_iowait_single_rw(0xcc, 3);
	io->set_iowait_range_rw(0xd8, 0xdf, 1);
	io->set_iowait_range_rw(0xe8, 0xeb, 1);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int drv = 0; drv < MAX_DRIVE; drv++) {
//		if(config.drive_type) {
//			fdc->set_drive_type(drv, DRIVE_TYPE_2D);
//		} else {
			fdc->set_drive_type(drv, DRIVE_TYPE_2DD);
//		}
	}
	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
		if(!(config.last_hard_disk_path[drv][0] != _T('\0') && FILEIO::IsFileExisting(config.last_hard_disk_path[drv]))) {
			create_local_path(config.last_hard_disk_path[drv], _MAX_PATH, _T("SASI%d.DAT"), drv);
		}
	}
	monitor_type = config.monitor_type;
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
	
	// set initial port status
	opn->write_signal(SIG_YM2203_PORT_B, (monitor_type & 2) ? 0x77 : 0x37, 0xff);
}

void VM::special_reset()
{
	// reset all devices
//	for(DEVICE* device = first_device; device; device = device->next_device) {
//		device->special_reset();
//	}
	memory->special_reset();
	cpu->reset();
}

void VM::run()
{
	event->drive();
}

double VM::get_frame_rate()
{
	return event->get_frame_rate();
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
	crtc->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	opn->initialize_sound(rate, 2000000, samples, 0, -8);
	pcm->initialize_sound(rate, 4096);
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
		opn->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		opn->set_volume(1, decibel_l, decibel_r);
	} else if(ch == 2) {
		pcm->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 3) {
		drec->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 4) {
		drec->set_volume(1, decibel_l, decibel_r);
	} else if(ch == 5) {
		fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 6) {
		drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// socket
// ----------------------------------------------------------------------------

void VM::notify_socket_connected(int ch)
{
	w3100a->notify_connected(ch);
}

void VM::notify_socket_disconnected(int ch)
{
	w3100a->notify_disconnected(ch);
}

uint8_t* VM::get_socket_send_buffer(int ch, int* size)
{
	return w3100a->get_send_buffer(ch, size);
}

void VM::inc_socket_send_buffer_ptr(int ch, int size)
{
	w3100a->inc_send_buffer_ptr(ch, size);
}

uint8_t* VM::get_socket_recv_buffer0(int ch, int* size0, int* size1)
{
	return w3100a->get_recv_buffer0(ch, size0, size1);
}

uint8_t* VM::get_socket_recv_buffer1(int ch)
{
	return w3100a->get_recv_buffer1(ch);
}

void VM::inc_socket_recv_buffer_ptr(int ch, int size)
{
	w3100a->inc_recv_buffer_ptr(ch, size);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

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

void VM::open_hard_disk(int drv, const _TCHAR* file_path)
{
	if(drv < USE_HARD_DISK) {
		sasi_hdd->open(drv, file_path, 256);
	}
}

void VM::close_hard_disk(int drv)
{
	if(drv < USE_HARD_DISK) {
		sasi_hdd->close(drv);
	}
}

bool VM::is_hard_disk_inserted(int drv)
{
	if(drv < USE_HARD_DISK) {
		return sasi_hdd->mounted(drv);
	}
	return false;
}

uint32_t VM::is_hard_disk_accessed()
{
	uint32_t status = 0;
	
	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
		if(sasi_hdd->accessed(drv)) {
			status |= 1 << drv;
		}
	}
	return status;
}

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	bool remote = drec->get_remote();
	bool opened = drec->play_tape(file_path);
	
	if(opened && remote) {
		// if machine already sets remote on, start playing now
		push_play(drv);
	}
	cmt->close_tape();
	cmt->play_tape(opened);
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	bool remote = drec->get_remote();
	bool opened = drec->rec_tape(file_path);
	
	if(opened && remote) {
		// if machine already sets remote on, start recording now
		push_play(drv);
	}
	cmt->close_tape();
	cmt->rec_tape(opened);
}

void VM::close_tape(int drv)
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
	drec->set_remote(false);
	cmt->close_tape();
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

#define STATE_VERSION	8

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
	state_fio->StateValue(monitor_type);
	return true;
}

