/*
	TOSHIBA T-200/250 Emulator 'eT-250'

	Author : Takeda.Toshiya
	Date   : 2023.12.29-

	[ virtual machine ]
*/

#include "t250.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../beep.h"
#include "../disk.h"
#include "../hd46505.h"
#include "../i8080.h"
#ifdef SUPPORT_CCM
#include "../i8251.h"
#include "../i8253.h"
#endif
#include "../i8257.h"
#include "../i8279.h"
#include "../io.h"
#include "../memory.h"
#include "../noise.h"
#include "../prnfile.h"
#include "../upd765a.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#ifdef SUPPORT_CCM
#include "ccm.h"
#endif
#include "membus.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	beep = new BEEP(this, emu);
	crtc = new HD46505(this, emu);
	cpu = new I8080(this, emu);	// 8085
#ifdef SUPPORT_CCM
	sio = new I8251(this, emu);
	pit = new I8253(this, emu);
#endif
	dma = new I8257(this, emu);
	kbc = new I8279(this, emu);
	io = new IO(this, emu);
	io->space = 0x100;
	fdc = new UPD765A(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	
#ifdef SUPPORT_CCM
	ccm = new CMM(this, emu);
#endif
	memory = new MEMBUS(this, emu);
	memory->space = 0x10000;
	memory->bank_size = 0x800;
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(beep);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	
#ifdef SUPPORT_CCM
	sio->set_context_txrdy(memory, SIG_MEMBUS_SIO_TXR, 1);
	sio->set_context_rxrdy(memory, SIG_MEMBUS_SIO_RXR, 1);
#endif
	dma->set_context_cpu(cpu);
	dma->set_context_memory(memory);
	dma->set_context_ch2(fdc);
	dma->set_context_tc(fdc, SIG_UPD765A_TC, 1);
#ifdef USE_DEBUGGER
	dma->set_context_debugger(new DEBUGGER(this, emu));
#endif
	kbc->set_context_sl(memory, SIG_MEMBUS_KEY_SEL, 0xff);
	kbc->set_context_irq(memory, SIG_MEMBUS_KEY_IRQ, 1);
	fdc->set_context_irq(cpu, SIG_I8085_RST5, 1);
	fdc->set_context_drq(dma, SIG_I8257_CH2, 1);
	
	memory->set_context_beep(beep);
	memory->set_context_cpu(cpu);
	memory->set_context_fdc(fdc);
	memory->set_context_kbc(kbc);
	if(config.printer_type == 0) {  
		PRNFILE *prn = new PRNFILE(this, emu);
		prn->set_context_ack(memory, SIG_MEMBUS_PRN_ACK, 1);
		prn->set_context_busy(memory, SIG_MEMBUS_PRN_BUSY, 1);
		memory->set_context_prn(prn);
	} else {
		memory->set_context_prn(dummy);
	}
	memory->set_regs_ptr(crtc->get_regs());
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(io);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_range_rw (0x80, 0x88, dma);
	io->set_iomap_range_rw (0x90, 0x91, kbc);
#ifdef SUPPORT_CCM
	io->set_iomap_range_rw (0xa0, 0xa3, pit);
	io->set_iomap_range_rw (0xa4, 0xa5, sio);
	io->set_iomap_range_rw (0xa6, 0xa7, ccm);
#endif
	io->set_iomap_range_w  (0xc0, 0xc3, memory);
	io->set_iomap_range_r  (0xd0, 0xd4, memory);
	io->set_iomap_range_rw (0xe0, 0xe1, crtc);
	io->set_iomap_single_r (0xf0, fdc);
	io->set_iomap_single_w (0xf0, memory);
	io->set_iomap_single_rw(0xf1, fdc);
	
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
	beep->initialize_sound(rate, 2400, 8000);
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
		beep->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
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
	memory->key_up(code);
}

bool VM::get_caps_locked()
{
	return false;//keyboard->get_caps_locked();
}

bool VM::get_kana_locked()
{
	return false;//keyboard->get_kana_locked();
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

