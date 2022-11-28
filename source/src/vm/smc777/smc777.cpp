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
#include "../noise.h"
#include "../pcm1bit.h"
#if defined(_SMC777)
#include "../sn76489an.h"
#endif
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "memory.h"

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
	crtc = new HD46505(this, emu);
	fdc = new MB8877(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
#if defined(_SMC70)
	rtc = new MSM58321(this, emu);
#endif
	pcm = new PCM1BIT(this, emu);
#if defined(_SMC777)
	psg = new SN76489AN(this, emu);
#endif
	cpu = new Z80(this, emu);
	
	memory = new MEMORY(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
#if defined(_SMC777)
	event->set_context_sound(psg);
#endif
	event->set_context_sound(drec);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	drec->set_context_ear(memory, SIG_MEMORY_DATAREC_IN, 1);
	crtc->set_context_disp(memory, SIG_MEMORY_CRTC_DISP, 1);
	crtc->set_context_vsync(memory, SIG_MEMORY_CRTC_VSYNC, 1);
	fdc->set_context_drq(memory, SIG_MEMORY_FDC_DRQ, 1);
	fdc->set_context_irq(memory, SIG_MEMORY_FDC_IRQ, 1);
#if defined(_SMC70)
	rtc->set_context_data(memory, SIG_MEMORY_RTC_DATA, 0x0f, 0);
	rtc->set_context_busy(memory, SIG_MEMORY_RTC_BUSY, 1);
#endif
	
	memory->set_context_cpu(cpu);
	memory->set_context_crtc(crtc, crtc->get_regs());
	memory->set_context_drec(drec);
	memory->set_context_fdc(fdc);
	memory->set_context_pcm(pcm);
#if defined(_SMC70)
	memory->set_context_rtc(rtc);
#elif defined(_SMC777)
	memory->set_context_psg(psg);
#endif
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(memory);
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
	memory->warm_start = false;
}

void VM::special_reset()
{
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
	memory->warm_start = true;
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
	} else if(ch-- == 0) {
		fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	if(!repeat) {
		memory->key_down_up(code, true);
	}
}

void VM::key_up(int code)
{
	memory->key_down_up(code, false);
}

bool VM::get_caps_locked()
{
	return memory->get_caps_locked();
}

bool VM::get_kana_locked()
{
	return memory->get_kana_locked();
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

uint32_t VM::is_floppy_disk_accessed()
{
	return fdc->read_signal(0);
}

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

#define STATE_VERSION	4

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
	return true;
}

