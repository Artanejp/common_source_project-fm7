/*
	SHARP MZ-80K/C Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	SHARP MZ-80A Emulator 'EmuZ-80A'
	Modify : Hideki Suga
	Date   : 2014.12.10 -

	[ virtual machine ]
*/

#include "mz80k.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#if defined(_MZ1200) || defined(_MZ80A)
#include "../and.h"
#endif
#include "../datarec.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../ls393.h"
#include "../mz1p17.h"
#include "../pcm1bit.h"
#include "../prnfile.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "keyboard.h"
#include "memory.h"
#include "printer.h"

#if defined(SUPPORT_MZ80AIF)
#include "../io.h"
#include "../disk.h"
#include "../mb8877.h"
#include "mz80aif.h"
#elif defined(SUPPORT_MZ80FIO)
#include "../io.h"
#include "../disk.h"
#include "../t3444a.h"
#include "mz80fio.h"
#endif

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
#if defined(_USE_QT)
	dummy->set_device_name(_T("1st Dummy"));
	event->set_device_name(_T("EVENT"));
#endif	
	
#if defined(_MZ1200) || defined(_MZ80A)
	and_int = new AND(this, emu);
#if defined(_USE_QT)
	and_int->set_device_name(_T("AND GATE(INTERRUPTS)"));
#endif	
#endif
	drec = new DATAREC(this, emu);
	ctc = new I8253(this, emu);
	pio = new I8255(this, emu);
	counter = new LS393(this, emu);
	pcm = new PCM1BIT(this, emu);
	cpu = new Z80(this, emu);
#if defined(_USE_QT)
	counter->set_device_name(_T("74LS393 COUNTER(CTC/SOUND)"));
#endif	
	
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	printer = new PRINTER(this, emu);
#if defined(_USE_QT)
	keyboard->set_device_name(_T("KEYBOARD I/F"));
	memory->set_device_name(_T("MEMORY"));
	printer->set_device_name(_T("PRINTER I/F"));
#endif	
	
#if defined(SUPPORT_MZ80AIF)
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);	// mb8866
	mz80aif = new MZ80AIF(this, emu);
#if defined(_USE_QT)
	io->set_device_name(_T("I/O BUS"));
	mz80aif->set_device_name(_T("MZ-80 AIF"));
#endif
#elif defined(SUPPORT_MZ80FIO)
	io = new IO(this, emu);
	fdc = new T3444A(this, emu);	// t3444m
	mz80fio = new MZ80FIO(this, emu);
#if defined(_USE_QT)
	io->set_device_name(_T("I/O BUS"));
	mz80fio->set_device_name(_T("MZ-80 FIO"));
#endif
#endif
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	event->set_context_sound(drec);
#if defined(USE_SOUND_FILES)

#if defined(SUPPORT_MZ80AIF)
	if(fdc->load_sound_data(MB8877_SND_TYPE_SEEK, _T("FDDSEEK.WAV"))) {
		event->set_context_sound(fdc);
	}
#elif defined(SUPPORT_MZ80FIO)
	if(fdc->load_sound_data(T3444A_SND_TYPE_SEEK, _T("FDDSEEK.WAV"))) {
		event->set_context_sound(fdc);
	}
#endif
	
	drec->load_sound_data(DATAREC_SNDFILE_EJECT, _T("CMTEJECT.WAV"));
	//drec->load_sound_data(DATAREC_SNDFILE_PLAY, _T("CMTPLAY.WAV"));
	//drec->load_sound_data(DATAREC_SNDFILE_STOP, _T("CMTSTOP.WAV"));
	drec->load_sound_data(DATAREC_SNDFILE_RELAY_ON, _T("CMTPLAY.WAV"));
	drec->load_sound_data(DATAREC_SNDFILE_RELAY_OFF, _T("CMTSTOP.WAV"));
#endif
	
#if defined(_MZ1200) || defined(_MZ80A)
	and_int->set_context_out(cpu, SIG_CPU_IRQ, 1);
	and_int->set_mask(SIG_AND_BIT_0 | SIG_AND_BIT_1);
#endif
	drec->set_context_ear(pio, SIG_I8255_PORT_C, 0x20);
	drec->set_context_remote(pio, SIG_I8255_PORT_C, 0x10);
	ctc->set_context_ch0(counter, SIG_LS393_CLK, 1);
	ctc->set_context_ch1(ctc, SIG_I8253_CLOCK_2, 1);
#if defined(_MZ1200) || defined(_MZ80A)
	ctc->set_context_ch2(and_int, SIG_AND_BIT_0, 1);
#else
	ctc->set_context_ch2(cpu, SIG_CPU_IRQ, 1);
#endif
	ctc->set_constant_clock(0, 2000000);
	ctc->set_constant_clock(1, 31250);
	pio->set_context_port_a(keyboard, SIG_KEYBOARD_COLUMN, 0x0f, 0);
	pio->set_context_port_c(memory, SIG_MEMORY_VGATE, 1, 0);
	pio->set_context_port_c(drec, SIG_DATAREC_MIC, 2, 0);
#if defined(_MZ1200) || defined(_MZ80A)
	pio->set_context_port_c(and_int, SIG_AND_BIT_1, 4, 0);
#endif
	pio->set_context_port_c(drec, SIG_DATAREC_TRIG, 8, 0);
	counter->set_context_1qa(pcm, SIG_PCM1BIT_SIGNAL, 1);
	
	keyboard->set_context_pio(pio);
	memory->set_context_ctc(ctc);
	memory->set_context_pio(pio);
	
#if defined(SUPPORT_MZ80AIF)
	fdc->set_context_irq(memory, SIG_MEMORY_FDC_IRQ, 1);
	fdc->set_context_drq(memory, SIG_MEMORY_FDC_DRQ, 1);
	mz80aif->set_context_fdc(fdc);
#elif defined(SUPPORT_MZ80FIO)
	mz80fio->set_context_fdc(fdc);
#endif
	
	if(config.printer_device_type == 0) {  
		printer->set_context_prn(new PRNFILE(this, emu));
	} else if(config.printer_device_type == 1) {
		MZ1P17 *mz1p17 = new MZ1P17(this, emu);
		mz1p17->mode = MZ1P17_MODE_MZ80P4;
		printer->set_context_prn(mz1p17);
	} else {
		printer->set_context_prn(dummy);
	}
	
	// cpu bus
	cpu->set_context_mem(memory);
#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
	cpu->set_context_io(io);
#else
	cpu->set_context_io(dummy);
#endif
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
#if defined(SUPPORT_MZ80AIF)
	io->set_iomap_range_rw(0xd8, 0xdb, fdc);
	io->set_iomap_range_w(0xdc, 0xdd, mz80aif);
#elif defined(SUPPORT_MZ80FIO)
	io->set_iomap_range_rw(0xf8, 0xfb, mz80fio);
#endif
	io->set_iomap_range_rw(0xfe, 0xff, printer);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
#if defined(SUPPORT_MZ80AIF)
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2DD);
	}
#elif defined(SUPPORT_MZ80FIO)
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2D);
//		fdc->set_drive_mfm(i, false);
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
#if defined(_MZ1200) || defined(_MZ80A)
	and_int->write_signal(SIG_AND_BIT_0, 0, 1);	// CLOCK = L
	and_int->write_signal(SIG_AND_BIT_1, 1, 1);	// INTMASK = H
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

#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
uint32_t VM::get_access_lamp_status()
{
	uint32_t status = fdc->read_signal(0);
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
		pcm->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		drec->set_volume(0, decibel_l, decibel_r);
	}
#if defined(USE_SOUND_FILES)
	else if(ch == 2) {
#if defined(SUPPORT_MZ80AIF)
		fdc->set_volume(MB8877_SND_TYPE_SEEK, decibel_l, decibel_r);
#elif defined(SUPPORT_MZ80FIO)
		fdc->set_volume(T3444A_SND_TYPE_SEEK, decibel_l, decibel_r);
#endif
	} else if(ch == 3) {
		drec->set_volume(2 + DATAREC_SNDFILE_RELAY_ON, decibel_l, decibel_r);
		drec->set_volume(2 + DATAREC_SNDFILE_RELAY_OFF, decibel_l, decibel_r);
		drec->set_volume(2 + DATAREC_SNDFILE_EJECT, decibel_l, decibel_r);
	}		
#endif
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	if(!repeat) {
		keyboard->key_down(code);
	}
}

void VM::key_up(int code)
{
//	keyboard->key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

#if defined(SUPPORT_MZ80AIF) || defined(SUPPORT_MZ80FIO)
void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	fdc->open_disk(drv, file_path, bank);
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
#endif

void VM::play_tape(const _TCHAR* file_path)
{
	drec->play_tape(file_path);
	drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
}

void VM::rec_tape(const _TCHAR* file_path)
{
	drec->rec_tape(file_path);
	drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
}

void VM::close_tape()
{
#if defined(USE_SOUND_FILES)
	drec->write_signal(SIG_SOUNDER_ADD + DATAREC_SNDFILE_EJECT, 1, 1);
#endif
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();

	drec->write_signal(SIG_DATAREC_REMOTE, 0, 0);
}

bool VM::is_tape_inserted()
{
	return drec->is_tape_inserted();
}

bool VM::is_tape_playing()
{
	return drec->is_tape_playing();
}

bool VM::is_tape_recording()
{
	return drec->is_tape_recording();
}

int VM::get_tape_position()
{
	return drec->get_tape_position();
}

void VM::push_play()
{
	drec->set_ff_rew(0);
	drec->set_remote(true);
}

void VM::push_stop()
{
	drec->set_remote(false);
}

void VM::push_fast_forward()
{
	drec->set_ff_rew(1);
	drec->set_remote(true);
}

void VM::push_fast_rewind()
{
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

#define STATE_VERSION	5

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

