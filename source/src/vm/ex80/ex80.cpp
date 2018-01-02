/*
	TOSHIBA EX-80 Emulator 'eEX-80'

	Author : Takeda.Toshiya
	Date   : 2015.12.10-

	[ virtual machine ]
*/

#include "ex80.h"
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

#include "cmt.h"
#include "display.h"
#include "keyboard.h"
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
	
	sio = new I8251(this, emu);
	pio = new I8255(this, emu);
	io = new IO(this, emu);
	pcm = new PCM1BIT(this, emu);
	cpu = new I8080(this, emu);
	
	cmt = new CMT(this, emu);
	display = new DISPLAY(this, emu);
	keyboard = new KEYBOARD(this, emu);
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
	pio->set_context_port_c(keyboard, SIG_KEYBOARD_COLUMN, 0x70, 0);
	pio->set_context_port_c(display, SIG_DISPLAY_DMA, 0x80, 0);
	// Sound:: Force realtime rendering. This is temporally fix. 20161024 K.O
	//pcm->set_realtime_render(true);
	
	cmt->set_context_sio(sio);
	display->set_context_cpu(cpu);
	display->set_ram_ptr(memory->get_ram());
	keyboard->set_context_pio(pio);
	memory->set_context_cpu(cpu);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// io bus
	io->set_iomap_range_rw(0xdc, 0xdd, sio);
	io->set_iomap_range_rw(0xf8, 0xfb, pio);
	
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
	}
}
#endif

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
		const char *name = typeid(*device).name() + 6; // skip "class "
		
		state_fio->FputInt32(strlen(name));
		state_fio->Fwrite(name, strlen(name), 1);
		device->save_state(state_fio);
	}
}

bool VM::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		const char *name = typeid(*device).name() + 6; // skip "class "
		
		if(!(state_fio->FgetInt32() == strlen(name) && state_fio->Fcompare(name, strlen(name)))) {
			return false;
		}
		if(!device->load_state(state_fio)) {
			return false;
		}
	}
	return true;
}

