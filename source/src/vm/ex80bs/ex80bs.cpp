/*
	TOSHIBA EX-80 Emulator 'eEX-80'

	Author : Takeda.Toshiya
	Date   : 2015.12.10-

	[ virtual machine ]
*/

#include "ex80bs.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../i8080.h"
#include "../i8251.h"
#include "../i8255.h"
#include "../io.h"
#include "../pcm1bit.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "./cmt.h"
#include "./display.h"
#include "./memory.h"

using EX80BS::CMT;
using EX80BS::DISPLAY;
using EX80BS::MEMORY;

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU_TEMPLATE* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device

	sio = new I8251(this, emu);
	pio = new I8255(this, emu);
	io = new IO(this, emu);
	io->space = 0x100;

	pcm = new PCM1BIT(this, emu);
#ifdef USE_DEBUGGER
//	pcm->set_context_debugger(new DEBUGGER(this, emu));
#endif
	cpu = new I8080(this, emu);

	cmt = new CMT(this, emu);
	display = new DISPLAY(this, emu);
	memory = new MEMORY(this, emu);
	// Set names
#if defined(_USE_QT)
	dummy->set_device_name(_T("1st Dummy"));

	pio->set_device_name(_T("i8255(SOUND/KEY/DISPLAY)"));
	sio->set_device_name(_T("i8251(CMT)"));
	pcm->set_device_name(_T("SOUND OUT"));
#endif

	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);

	sio->set_context_out(cmt, SIG_CMT_OUT);
	pio->set_context_port_c(pcm, SIG_PCM1BIT_SIGNAL, 0x08, 0);
	pio->set_context_port_c(memory, SIG_KEYBOARD_COLUMN, 0x70, 0);
	pio->set_context_port_c(display, SIG_DISPLAY_PC, 0x87, 0);

	// Sound:: Force realtime rendering. This is temporally fix. 20161024 K.O
	//pcm->set_realtime_render(true);

	cmt->set_context_sio(sio);
	display->set_context_cpu(cpu);
	display->set_ram_ptr(memory->get_ram());
	display->set_vram_ptr(memory->get_vram());
	memory->set_context_cpu(cpu);
	memory->set_context_pio(pio);

	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(memory);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif

	// io bus
	io->set_iomap_range_rw(0xdc, 0xdd, sio);
	io->set_iomap_single_w(0xaf, memory);
	io->set_iomap_single_r(0xef, memory);
	io->set_iomap_range_rw(0xf8, 0xfb, pio);

	// initialize all devices
#if defined(__GIT_REPO_VERSION)
	set_git_repo_version(__GIT_REPO_VERSION);
#endif
	initialize_devices();
}

VM::~VM()
{
	// delete all devices
	release_devices();
//	for(DEVICE* device = first_device; device;) {
//		DEVICE *next_device = device->next_device;
//		device->release();
//		delete device;
//		device = next_device;
//	}
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
	__UNLIKELY_IF(display == nullptr) return;
	display->draw_screen();
}

int VM::max_draw_ranges()
{
	return (config.monitor_type == 0) ? 9 : 8;
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
		pcm->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	if(!repeat) {
		memory->key_down(code);
	}
}

void VM::key_up(int code)
{
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::load_binary(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		memory->load_binary(file_path);
	}
}

void VM::save_binary(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		memory->save_binary(file_path);
	}
}

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	cmt->play_tape(file_path);
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	cmt->rec_tape(file_path);
}

void VM::close_tape(int drv)
{
	cmt->close_tape();
}

bool VM::is_tape_inserted(int drv)
{
	return cmt->is_tape_inserted();
}

bool VM::is_frame_skippable()
{
	__UNLIKELY_IF(event == nullptr) return false;
	return event->is_frame_skippable();
}

double VM::get_current_usec()
{
	__UNLIKELY_IF(event == nullptr) return 0.0;
	return event->get_current_usec();
}

uint64_t VM::get_current_clock_uint64()
{
	__UNLIKELY_IF(event == nullptr) return (uint64_t)0;
	return event->get_current_clock_uint64();
}

#define STATE_VERSION	2

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!(VM_TEMPLATE::process_state_core(state_fio, loading, STATE_VERSION))) {
		return false;
	}
	// Machine specified.
 	return true;
}
