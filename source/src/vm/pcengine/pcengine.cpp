/*
	NEC-HE PC Engine Emulator 'ePCEngine'

	Author : Takeda.Toshiya
	Date   : 2012.10.31-

	[ virtual machine ]
*/

#include "pcengine.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../huc6280.h"
#include "pce.h"

#include "../../fileio.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	
	pceevent = new EVENT(this, emu);
//	pceevent->set_frames_per_sec(FRAMES_PER_SEC);
//	pceevent->set_lines_per_frame(LINES_PER_FRAME);
	
	pcecpu = new HUC6280(this, emu);
//	pcecpu->set_context_event_manager(pceevent);
	pce = new PCE(this, emu);
//	pce->set_context_event_manager(pceevent);
	
	pceevent->set_context_cpu(pcecpu, CPU_CLOCKS);
	pceevent->set_context_sound(pce);
	
	pcecpu->set_context_mem(pce);
	pcecpu->set_context_io(pce);
	pce->set_context_cpu(pcecpu);
	
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
	pceevent->drive();
}

double VM::frame_rate()
{
	return pceevent->frame_rate();
}

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	pce->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	pceevent->initialize_sound(rate, samples);
	
	// init sound gen
	pce->initialize_sound(rate);
}

uint16* VM::create_sound(int* extra_frames)
{
	return pceevent->create_sound(extra_frames);
}

int VM::sound_buffer_ptr()
{
	return pceevent->sound_buffer_ptr();
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_cart(int drv, _TCHAR* file_path)
{
	if(drv == 0) {
		pce->open_cart(file_path);
		pce->reset();
		pcecpu->reset();
	}
}

void VM::close_cart(int drv)
{
	if(drv == 0) {
		pce->close_cart();
		pce->reset();
		pcecpu->reset();
	}
}

bool VM::cart_inserted(int drv)
{
	if(drv == 0) {
		return pce->cart_inserted();
	} else {
		return false;
	}
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

