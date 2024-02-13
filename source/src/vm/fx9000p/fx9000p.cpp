/*
	CASIO FX-9000P Emulator 'eFX-9000P'

	Author : Takeda.Toshiya
	Date   : 2022.03.25-

	[ virtual machine ]
*/

#include "fx9000p.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../hd46505.h"
#include "../memory.h"
#include "../noise.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "./io.h"

using FX9000P::IO;
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
	memory = new MEMORY(this, emu);
	memory->space = 0x10000;
	memory->bank_size = 0x1000;
	cpu = new Z80(this, emu);

	io = new IO(this, emu);

	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(drec);
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());

	drec->set_context_ear(io, SIG_IO_EAR, 1);
	crtc->set_context_disp(io, SIG_IO_DISP, 1);

	io->set_context_cpu(cpu);
	io->set_context_crtc(crtc);
	io->set_context_drec(drec);

	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif

	// memory bus
	memset(basic, 0xff, sizeof(basic));
	memset(op1, 0xff, sizeof(op1));
	memset(vram, 0x00, sizeof(vram));
#if 0
	memset(dram, 0x00, sizeof(dram));
	memset(cmos, 0x00, sizeof(cmos));
#else
	// initial ram contents ???
	for(int i = 0; i < sizeof(dram); i++) {
		dram[i] = (0x2e + i) & 0xff;
	}
	for(int i = 0; i < sizeof(cmos); i++) {
		cmos[i] = (0x2e + i) & 0xff;
	}
#endif

	memory->read_bios(_T("BASIC.ROM"), basic, sizeof(basic));
	memory->read_bios(_T("OP1.ROM"), op1, sizeof(op1));
	memory->read_bios(_T("RAM.BIN"), cmos, sizeof(cmos));

	memory->set_memory_r(0x0000, 0x2fff, basic);
	memory->set_memory_r(0x5000, 0x5fff, op1);
	memory->set_memory_mapped_io_rw(0x6000, 0x6fff, io);
	memory->set_memory_rw(0x7000, 0x7fff, vram);
	memory->set_memory_rw(0x8000, 0xbfff, dram);
	memory->set_memory_rw(0xc000, 0xefff, cmos);

	// initialize all devices
#if defined(__GIT_REPO_VERSION)
	set_git_repo_version(__GIT_REPO_VERSION);
#endif
	initialize_devices();
}

VM::~VM()
{
	memory->write_bios(_T("RAM.BIN"), cmos, sizeof(cmos));

	// delete all devices
	for(DEVICE* device = first_device; device;) {
		DEVICE *next_device = device->next_device;
		device->release();
		delete device;
		device = next_device;
	}
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
	__UNLIKELY_IF(io == nullptr) return;
	io->draw_screen(vram);
}


// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	__UNLIKELY_IF(io == nullptr) return;
	if(!repeat) {
		io->key_down(code);
	}
}

void VM::key_up(int code)
{
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	__UNLIKELY_IF(event == nullptr) return;
	event->initialize_sound(rate, samples);
}

uint16_t* VM::create_sound(int* extra_frames)
{
	__UNLIKELY_IF(event == nullptr) return nullptr;
	return event->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
	__UNLIKELY_IF(event == nullptr) return 0;
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

bool VM::is_frame_skippable()
{
	__UNLIKELY_IF(event == nullptr) return false;
	return event->is_frame_skippable();
}

#define STATE_VERSION	1

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!(VM_TEMPLATE::process_state_core(state_fio, loading, STATE_VERSION))) {
		return false;
	}
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateArray(dram, sizeof(dram), 1);
	state_fio->StateArray(cmos, sizeof(cmos), 1);
	return true;
}
