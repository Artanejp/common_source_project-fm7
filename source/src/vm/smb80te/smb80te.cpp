/*
	SHARP SM-B-80TE Emulator 'eSM-B-80TE'

	Author : Takeda.Toshiya
	Date   : 2016.12.29-

	[ virtual machine ]
*/

#include "smb80te.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../io.h"
#include "../noise.h"
#include "../not.h"
#include "../z80.h"
#include "../z80pio.h"

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
	io = new IO(this, emu);
	not_ear = new NOT(this, emu);
	cpu = new Z80(this, emu);
	pio1 = new Z80PIO(this, emu);
	pio1->set_device_name(_T("8255 PIO (LEDs/Keyboard/CMT)"));
	pio2 = new Z80PIO(this, emu);
	pio2->set_device_name(_T("8255 PIO (User)"));
	
	memory = new MEMORY(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(drec);
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	// PIO1 PA0-7	-> 7seg-LED data
	pio1->set_context_port_a(memory, SIG_MEMORY_PIO1_PA, 0xff, 0);
	// PIO1 PB0-2	<- Keyboard data
	// PIO1 PB3-5	-> 7seg-LED/Keyboard column
	pio1->set_context_port_b(memory, SIG_MEMORY_PIO1_PB, 0x38, 0);
	// PIO1 PB6	-> MIC
	pio1->set_context_port_b(drec, SIG_DATAREC_MIC, 0x40, 0);
	// PIO1 PB7	<- NOT <- EAR
	drec->set_context_ear(not_ear, SIG_NOT_INPUT, 1);
	not_ear->set_context_out(pio1, SIG_Z80PIO_PORT_B, 0x80);
	
	memory->set_context_cpu(cpu);
	memory->set_context_drec(drec);
	memory->set_context_pio1(pio1);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pio1);
	
	// z80 family daisy chain
	pio1->set_context_intr(cpu, 0);
	pio1->set_context_child(pio2);
	pio2->set_context_intr(cpu, 1);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_range_rw(0xd0, 0xd3, pio1);
	io->set_iomap_range_rw(0xd4, 0xd7, pio2);
	io->set_iomap_range_rw(0xd8, 0xdb, memory);
	
	// initialize all devices
#if defined(__GIT_REPO_VERSION)
	strncpy(_git_revision, __GIT_REPO_VERSION, sizeof(_git_revision) - 1);
#endif
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	decl_state();
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
		drec->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
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
	if(code == 0x97 && !repeat) {
		reset();
	}
}

void VM::key_up(int code)
{
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	drec->play_tape(file_path);
//	drec->set_remote(true);
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	drec->rec_tape(file_path);
//	drec->set_remote(true);
}

void VM::close_tape(int drv)
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
//	drec->set_remote(false);
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
	drec->set_ff_rew(0);
	drec->set_remote(true);
}

void VM::push_stop(int drv)
{
	drec->set_remote(false);
}

void VM::push_fast_forward(int drv)
{
	drec->set_ff_rew(1);
	drec->set_remote(true);
}

void VM::push_fast_rewind(int drv)
{
	drec->set_ff_rew(-1);
	drec->set_remote(true);
}

void VM::load_binary(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		memory->load_ram(file_path);
	}
}

void VM::save_binary(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		memory->save_ram(file_path);
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

#define STATE_VERSION	3

#include "../../statesub.h"
#include "../../qt/gui/csp_logger.h"
extern CSP_Logger DLL_PREFIX_I *csp_logger;

void VM::decl_state(void)
{
	state_entry = new csp_state_utils(STATE_VERSION, 0, (_TCHAR *)(_T("CSP::SM_B_80TE_HEAD")), csp_logger);
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->decl_state();
	}
}

void VM::save_state(FILEIO* state_fio)
{
	//state_fio->FputUint32(STATE_VERSION);
	
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
}

bool VM::load_state(FILEIO* state_fio)
{
	//if(state_fio->FgetUint32() != STATE_VERSION) {
	//	return false;
	//}
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		emu->out_debug_log("INFO: HEADER DATA ERROR");
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		if(!device->load_state(state_fio)) {
			return false;
		}
	}
	return true;
}

