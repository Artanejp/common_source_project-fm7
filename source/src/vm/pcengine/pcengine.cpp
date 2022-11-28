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
#include "../msm5205.h"
#include "../scsi_cdrom.h"
#include "../scsi_host.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "pce.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	
	pceevent = new EVENT(this, emu);
//	pceevent->set_frames_per_sec(FRAMES_PER_SEC);
//	pceevent->set_lines_per_frame(LINES_PER_FRAME);
	
	pcecpu = new HUC6280(this, emu);
//	pcecpu->set_context_event_manager(pceevent);
	adpcm = new MSM5205(this, emu);
//	adpcm->set_context_event_manager(pceevent);
	scsi_host = new SCSI_HOST(this, emu);
//	scsi_host->set_context_event_manager(pceevent);
	scsi_cdrom = new SCSI_CDROM(this, emu);
//	scsi_cdrom->set_context_event_manager(pceevent);
	
	pce = new PCE(this, emu);
//	pce->set_context_event_manager(pceevent);
	
	pceevent->set_context_cpu(pcecpu, CPU_CLOCKS);
	pceevent->set_context_sound(pce);
	// NOTE: adpcm::mix() and scsi_cdrom::mix() will be called in pce::mix()
//	pceevent->set_context_sound(adpcm);
//	pceevent->set_context_sound(scsi_cdrom);
	
	pcecpu->set_context_mem(pce);
	pcecpu->set_context_io(pce);
#ifdef USE_DEBUGGER
	pcecpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	scsi_cdrom->scsi_id = 0;
	scsi_cdrom->set_context_interface(scsi_host);
	scsi_host->set_context_target(scsi_cdrom);
	
	scsi_host->set_context_irq(pce, SIG_PCE_SCSI_IRQ, 1);
	scsi_host->set_context_drq(pce, SIG_PCE_SCSI_DRQ, 1);
	scsi_host->set_context_bsy(pce, SIG_PCE_SCSI_BSY, 1);
	scsi_cdrom->set_context_done(pce, SIG_PCE_CDDA_DONE, 1);
	adpcm->set_context_vclk(pce, SIG_PCE_ADPCM_VCLK, 1);
	
	pce->set_context_cpu(pcecpu);
	pce->set_context_adpcm(adpcm);
	pce->set_context_scsi_host(scsi_host);
	pce->set_context_scsi_cdrom(scsi_cdrom);
	
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

double VM::get_frame_rate()
{
	return pceevent->get_frame_rate();
}

// ----------------------------------------------------------------------------
// debugger
// ----------------------------------------------------------------------------

#ifdef USE_DEBUGGER
DEVICE *VM::get_cpu(int index)
{
	if(index == 0) {
		return pcecpu;
	}
	return NULL;
}
#endif

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
	adpcm->initialize_sound(ADPCM_CLOCK / 6, MSM5205_S48_4B);
}

uint16_t* VM::create_sound(int* extra_frames)
{
	return pceevent->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
	return pceevent->get_sound_buffer_ptr();
}

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch == 0) {
		pce->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		scsi_cdrom->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 2) {
		adpcm->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_cart(int drv, const _TCHAR* file_path)
{
	pce->open_cart(file_path);
	pce->reset();
	pcecpu->reset();
}

void VM::close_cart(int drv)
{
	pce->close_cart();
	pce->reset();
	pcecpu->reset();
}

bool VM::is_cart_inserted(int drv)
{
	return pce->is_cart_inserted();
}

void VM::open_compact_disc(int drv, const _TCHAR* file_path)
{
	scsi_cdrom->open(file_path);
}

void VM::close_compact_disc(int drv)
{
	scsi_cdrom->close();
}

bool VM::is_compact_disc_inserted(int drv)
{
	return scsi_cdrom->mounted();
}

uint32_t VM::is_compact_disc_accessed()
{
	return scsi_cdrom->accessed();
}

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

#define STATE_VERSION	2

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		const _TCHAR *name = char_to_tchar(typeid(*device).name() + 6); // skip "class "
		int len = (int)_tcslen(name);
		
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

