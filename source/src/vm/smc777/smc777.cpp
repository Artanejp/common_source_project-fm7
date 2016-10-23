/*
	SONY SMC-70 Emulator 'eSMC-70'
	SONY SMC-777 Emulator 'eSMC-777'

	Author : Takeda.Toshiya
	Date   : 2015.08.13-

	[ virtual machine ]
*/

#include "smc777.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../disk.h"
#include "../hd46505.h"
#include "../mb8877.h"
#if defined(_SMC70)
#include "../msm58321.h"
#endif
#include "../pcm1bit.h"
#if defined(_SMC777)
#include "../sn76489an.h"
#endif
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "io.h"

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
#endif	
	
	drec = new DATAREC(this, emu);
	crtc = new HD46505(this, emu);
	fdc = new MB8877(this, emu);
#if defined(_SMC70)
	rtc = new MSM58321(this, emu);
#endif
	pcm = new PCM1BIT(this, emu);
#if defined(_SMC777)
	psg = new SN76489AN(this, emu);
#endif
	cpu = new Z80(this, emu);
#if defined(_USE_QT)
	cpu->set_device_name(_T("CPU(Z80)"));
#endif	
	
	io = new IO(this, emu);
#if defined(_USE_QT)
	io->set_device_name(_T("I/O BUS"));
#endif	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
#if defined(_SMC777)
	event->set_context_sound(psg);
#endif
	event->set_context_sound(drec);
#if defined(USE_SOUND_FILES)
	if(fdc->load_sound_data(MB8877_SND_TYPE_SEEK, _T("FDDSEEK.WAV"))) {
		event->set_context_sound(fdc);
	}
	drec->load_sound_data(DATAREC_SNDFILE_RELAY_ON, _T("RELAY_ON.WAV"));
	drec->load_sound_data(DATAREC_SNDFILE_RELAY_OFF, _T("RELAYOFF.WAV"));
#endif
	// Sound:: Force realtime rendering. This is temporally fix. 20161024 K.O
	pcm->set_realtime_render(true);

	drec->set_context_ear(io, SIG_IO_DATAREC_IN, 1);
	crtc->set_context_disp(io, SIG_IO_CRTC_DISP, 1);
	crtc->set_context_vsync(io, SIG_IO_CRTC_VSYNC, 1);
	fdc->set_context_drq(io, SIG_IO_FDC_DRQ, 1);
	fdc->set_context_irq(io, SIG_IO_FDC_IRQ, 1);
#if defined(_SMC70)
	rtc->set_context_data(io, SIG_IO_RTC_DATA, 0x0f, 0);
	rtc->set_context_busy(io, SIG_IO_RTC_BUSY, 1);
#endif
	
	io->set_context_cpu(cpu);
	io->set_context_crtc(crtc, crtc->get_regs());
	io->set_context_drec(drec);
	io->set_context_fdc(fdc);
	io->set_context_pcm(pcm);
#if defined(_SMC70)
	io->set_context_rtc(rtc);
#elif defined(_SMC777)
	io->set_context_psg(psg);
#endif
	
	// cpu bus
	cpu->set_context_mem(io);
	cpu->set_context_io(io);
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2DD); // 1DD
		fdc->set_drive_rpm(i, 600);
	}
	fdc->write_signal(SIG_MB8877_MOTOR, 1, 1);
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
	io->warm_start = false;
}

void VM::special_reset()
{
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
	io->warm_start = true;
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
	io->draw_screen();
}

uint32_t VM::get_access_lamp_status()
{
	uint32_t status = fdc->read_signal(0);
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
	pcm->initialize_sound(rate, 5000);
#if defined(_SMC777)
	psg->initialize_sound(rate, CPU_CLOCKS, 5000);
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
#if defined(_SMC777)
	if(ch-- == 0) {
		psg->set_volume(0, decibel_l, decibel_r);
	} else
#endif
	if(ch-- == 0) {
		pcm->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		drec->set_volume(0, decibel_l, decibel_r);
	}
#if defined(USE_SOUND_FILES)
	else if(ch-- == 0) {
		fdc->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		drec->set_volume(2 + DATAREC_SNDFILE_RELAY_ON , decibel_l, decibel_r);
		drec->set_volume(2 + DATAREC_SNDFILE_RELAY_OFF, decibel_l, decibel_r);
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
		io->key_down_up(code, true);
	}
}

void VM::key_up(int code)
{
	io->key_down_up(code, false);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

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

void VM::play_tape(const _TCHAR* file_path)
{
	drec->play_tape(file_path);
}

void VM::rec_tape(const _TCHAR* file_path)
{
	drec->rec_tape(file_path);
}

void VM::close_tape()
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
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

#define STATE_VERSION	2

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

