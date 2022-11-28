/*
	NEC PC-100 Emulator 'ePC-100'

	Author : Takeda.Toshiya
	Date   : 2008.07.12 -

	[ virtual machine ]
*/

#include "pc100.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../and.h"
#include "../beep.h"
#include "../disk.h"
#include "../i8251.h"
#include "../i8255.h"
#include "../i8259.h"
#include "../i86.h"
#include "../io.h"
#include "../memory.h"
#include "../msm58321.h"
#include "../noise.h"
#include "../pcm1bit.h"
#include "../upd765a.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "crtc.h"
#include "ioctrl.h"
#include "kanji.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	and_drq = new AND(this, emu);
	beep = new BEEP(this, emu);
	sio = new I8251(this, emu);
	pio0 = new I8255(this, emu);
	pio0->set_device_name(_T("8255 PIO (RTC)"));
	pio1 = new I8255(this, emu);
	pio1->set_device_name(_T("8255 PIO (CRTC)"));
	pic = new I8259(this, emu);
	cpu = new I86(this, emu);
	cpu->device_model = INTEL_8086;
	io = new IO(this, emu);
	memory = new MEMORY(this, emu);
	rtc = new MSM58321(this, emu);
	pcm = new PCM1BIT(this, emu);
	fdc = new UPD765A(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	
	crtc = new CRTC(this, emu);
	ioctrl = new IOCTRL(this, emu);
	kanji = new KANJI(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(beep);
	event->set_context_sound(pcm);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	
	and_drq->set_context_out(cpu, SIG_CPU_NMI, 1);
	and_drq->set_mask(SIG_AND_BIT_0 | SIG_AND_BIT_1);
	sio->set_context_rxrdy(pic, SIG_I8259_IR1, 1);
	pio0->set_context_port_a(rtc, SIG_MSM58321_READ, 1, 0);
	pio0->set_context_port_a(rtc, SIG_MSM58321_WRITE, 2, 0);
	pio0->set_context_port_a(rtc, SIG_MSM58321_ADDR_WRITE, 4, 0);
	pio0->set_context_port_c(rtc, SIG_MSM58321_DATA, 0x0f, 0);
	pio1->set_context_port_a(crtc, SIG_CRTC_BITMASK_LOW, 0xff, 0);
	pio1->set_context_port_b(crtc, SIG_CRTC_BITMASK_HIGH, 0xff, 0);
	pio1->set_context_port_c(crtc, SIG_CRTC_VRAM_PLANE, 0x3f, 0);
	pio1->set_context_port_c(and_drq, SIG_AND_BIT_0, 0x80, 0);
	pio1->set_context_port_c(ioctrl, SIG_IOCTRL_RESET, 0x40, 0);
	pic->set_context_cpu(cpu);
	rtc->set_context_data(pio0, SIG_I8255_PORT_C, 0x0f, 0);
	rtc->set_context_busy(pio0, SIG_I8255_PORT_C, 0x10);
	fdc->set_context_irq(cpu, SIG_CPU_NMI, 1);
	fdc->set_context_drq(and_drq, SIG_AND_BIT_1, 1);
	
	crtc->set_context_pic(pic);
	ioctrl->set_context_pic(pic);
	ioctrl->set_context_fdc(fdc);
	ioctrl->set_context_beep(beep);
	ioctrl->set_context_pcm(pcm);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// memory bus
	memset(ram, 0, sizeof(ram));
	memset(ipl, 0xff, sizeof(ipl));
	
	memory->read_bios(_T("IPL.ROM"), ipl, sizeof(ipl));
	
	memory->set_memory_rw(0x00000, 0xbffff, ram);
	memory->set_memory_mapped_io_rw(0xc0000, 0xdffff, crtc);
	memory->set_memory_r(0xf8000, 0xfffff, ipl);
	
	// i/o bus
	io->set_iomap_alias_rw(0x00, pic, 0);
	io->set_iomap_alias_rw(0x02, pic, 1);
//	io->set_iomap_alias_rw(0x04, dma, 0);
//	io->set_iomap_alias_rw(0x06, dma, 1);
	io->set_iomap_alias_r(0x08, fdc, 0);
	io->set_iomap_alias_rw(0x0a, fdc, 1);
	io->set_iomap_alias_rw(0x10, pio0, 0);
	io->set_iomap_alias_rw(0x12, pio0, 1);
	io->set_iomap_alias_rw(0x14, pio0, 2);
	io->set_iomap_alias_rw(0x16, pio0, 3);
	io->set_iomap_alias_rw(0x18, pio1, 0);
	io->set_iomap_alias_rw(0x1a, pio1, 1);
	io->set_iomap_alias_rw(0x1c, pio1, 2);
	io->set_iomap_alias_rw(0x1e, pio1, 3);
	io->set_iomap_single_r(0x20, ioctrl);
	io->set_iomap_single_rw(0x22, ioctrl);
	io->set_iomap_single_w(0x24, ioctrl);
	io->set_iomap_alias_rw(0x28, sio, 0);
	io->set_iomap_alias_rw(0x2a, sio, 1);
	io->set_iomap_single_rw(0x30, crtc);
	io->set_iomap_single_rw(0x38, crtc);
	io->set_iomap_single_rw(0x3a, crtc);
	io->set_iomap_single_rw(0x3c, crtc);
	io->set_iomap_single_rw(0x3e, crtc);
	for(int i = 0x40; i < 0x62; i++) {
		io->set_iomap_single_rw(i, crtc);
	}
	io->set_iomap_single_rw(0x80, kanji);
	io->set_iomap_single_rw(0x81, kanji);
	io->set_iomap_single_w(0x84, kanji);
	io->set_iomap_single_w(0x86, kanji);
	
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
	for(int i = 0; i < 4; i++) {
		if(config.drive_type) {
			fdc->set_drive_type(i, DRIVE_TYPE_2DD);
		} else {
			fdc->set_drive_type(i, DRIVE_TYPE_2D);
		}
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
	crtc->draw_screen();
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
		beep->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		pcm->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 2) {
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
	ioctrl->key_down(code);
}

void VM::key_up(int code)
{
	ioctrl->key_up(code);
}

bool VM::get_caps_locked()
{
	return ioctrl->get_caps_locked();
}

bool VM::get_kana_locked()
{
	return ioctrl->get_kana_locked();
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

#define STATE_VERSION	4

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
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}

