/*
	National JR-100 Emulator 'eJR-100'

	Author : Takeda.Toshiya
	Date   : 2015.08.27-

	[ virtual machine ]
*/

#include "jr100.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../mc6800.h"
#include "../not.h"
#include "../pcm1bit.h"
#include "../sy6522.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "memory.h"

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
	cpu = new MC6800(this, emu);	// MB8861N
	not_mic = new NOT(this, emu);
	not_ear = new NOT(this, emu);
	pcm = new PCM1BIT(this, emu);
	via = new SY6522(this, emu);
#if defined(_USE_QT)
	cpu->set_device_name(_T("CPU(MC6800)"));
	not_mic->set_device_name(_T("NOT GATE(MIC)"));
	not_ear->set_device_name(_T("NOT GATE(EAR PHONE)"));
	pcm->set_device_name(_T("SOUND DEVICE"));
	via->set_device_name(_T("SY6522 VIA"));
#endif	
	
	memory = new MEMORY(this, emu);
#if defined(_USE_QT)
	memory->set_device_name(_T("MEMORY"));
#endif	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	event->set_context_sound(drec);
#if defined(USE_SOUND_FILES)
	drec->load_sound_data(DATAREC_SNDFILE_RELAY_ON, _T("RELAY_ON.WAV"));
	drec->load_sound_data(DATAREC_SNDFILE_RELAY_OFF, _T("RELAYOFF.WAV"));
#endif
	via->set_context_port_a(memory, SIG_MEMORY_VIA_PORT_A, 0xff, 0);
	via->set_context_port_b(memory, SIG_MEMORY_VIA_PORT_B, 0xff, 0);
	via->set_context_port_b(pcm, SIG_PCM1BIT_SIGNAL, 0x80, 0);	// PB7 -> Speaker
	via->set_context_port_b(via, SIG_MEMORY_VIA_PORT_B, 0x80, -1);	// PB7 -> PB6
	via->set_context_cb2(not_mic, SIG_NOT_INPUT, 1);		// CB2 -> NOT -> MIC
	via->set_constant_clock(CPU_CLOCKS >> 2);
	not_mic->set_context_out(drec, SIG_DATAREC_MIC, 1);
	drec->set_context_ear(not_ear, SIG_NOT_INPUT, 1);		// EAR -> NOT -> CA1,CB1
	not_ear->set_context_out(via, SIG_SY6522_PORT_CA1, 1);
	not_ear->set_context_out(via, SIG_SY6522_PORT_CB1, 1);
	// Sound:: Force realtime rendering. This is temporally fix. 20161024 K.O
	pcm->set_realtime_render(true);
	
	memory->set_context_via(via);
	
	// cpu bus
	cpu->set_context_mem(memory);
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

void VM::special_reset()
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
		drec->set_volume(2 + DATAREC_SNDFILE_RELAY_ON,  decibel_l, decibel_r);
		drec->set_volume(2 + DATAREC_SNDFILE_RELAY_OFF, decibel_l, decibel_r);
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
	push_play();
}

void VM::rec_tape(const _TCHAR* file_path)
{
	drec->rec_tape(file_path);
	push_play();
}

void VM::close_tape()
{
	push_stop();
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

