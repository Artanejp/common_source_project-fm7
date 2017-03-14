/*
	NEC PC-2001 Emulator 'ePC-2001'

	Origin : PockEmul
	Author : Takeda.Toshiya
	Date   : 2016.03.18-

	[ virtual machine ]
*/

#include "pc2001.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../memory.h"
#include "../noise.h"
#include "../pcm1bit.h"
#include "../upd16434.h"
#include "../upd1990a.h"
#include "../upd7810.h"

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
	event->set_device_name(_T("EVENT"));
#endif	
	
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
	memory = new MEMORY(this, emu);
	pcm = new PCM1BIT(this, emu);
	lcd[0] = new UPD16434(this, emu);
	lcd[0]->set_device_name(_T("uPD16434 LCD Controller #0"));
	lcd[1] = new UPD16434(this, emu);
	lcd[1]->set_device_name(_T("uPD16434 LCD Controller #1"));
	lcd[2] = new UPD16434(this, emu);
	lcd[2]->set_device_name(_T("uPD16434 LCD Controller #2"));
	lcd[3] = new UPD16434(this, emu);
	lcd[3]->set_device_name(_T("uPD16434 LCD Controller #3"));
	rtc = new UPD1990A(this, emu);
	cpu = new UPD7810(this, emu);
#if defined(_USE_QT)
	lcd[0]->set_device_name(_T("uPD16434 LCD CONTROLLER #1"));
	lcd[1]->set_device_name(_T("uPD16434 LCD CONTROLLER #2"));
	lcd[2]->set_device_name(_T("uPD16434 LCD CONTROLLER #3"));
	lcd[3]->set_device_name(_T("uPD16434 LCD CONTROLLER #4"));
	cpu->set_device_name(_T("CPU(uPD7810)"));
#endif	
	
	io = new IO(this, emu);
#if defined(_USE_QT)
	io->set_device_name(_T("I/O BUS"));
#endif	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	event->set_context_sound(drec);
#if defined(USE_SOUND_FILES)
	drec->load_sound_data(DATAREC_SNDFILE_RELAY_ON,  _T("CMTPLAY.WAV"));
	drec->load_sound_data(DATAREC_SNDFILE_RELAY_OFF, _T("CMTSTOP.WAV"));
	drec->load_sound_data(DATAREC_SNDFILE_EJECT,     _T("CMTEJECT.WAV"));
#endif
	drec->set_context_ear(io, SIG_IO_DREC_IN, 1);
	rtc->set_context_dout(io, SIG_IO_RTC_IN, 1);
	cpu->set_context_to(pcm, SIG_PCM1BIT_SIGNAL, 1);
	
	io->set_context_lcd(0, lcd[0]);
	io->set_context_lcd(1, lcd[1]);
	io->set_context_lcd(2, lcd[2]);
	io->set_context_lcd(3, lcd[3]);
	io->set_context_drec(drec);
	io->set_context_rtc(rtc);
	io->set_context_cpu(cpu);
	
	// memory bus
	memset(ram, 0xff, sizeof(ram));
//	memset(ram, 0, sizeof(ram));
	memset(rom1, 0xff, sizeof(rom1));
	memset(rom2, 0xff, sizeof(rom2));
	
	memory->read_bios(_T("0000-0FFF.ROM"), rom1, sizeof(rom1));
	memory->read_bios(_T("2000-5FFF.ROM"), rom2, sizeof(rom2));
	
	memory->set_memory_r(0x0000, 0x0fff, rom1);
	memory->set_memory_r(0x2000, 0x5fff, rom2);
	memory->set_memory_rw(0xb000, 0xffff, ram);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
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
	for(int i = 0; i < 4; i++) {
		lcd[i]->draw(60 * i);
	}
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
	} else if(ch == 2) {
		drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
	}
#if defined(USE_SOUND_FILES)
	 else if(ch == 2) {
		 for(int i = 0; i < DATAREC_SNDFILE_END; i++) {
			 drec->set_volume(i + 2, decibel_l, decibel_r);
		 }
	 }
#endif
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

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
	state_fio->Fwrite(ram, sizeof(ram), 1);
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
	state_fio->Fread(ram, sizeof(ram), 1);
	return true;
}

