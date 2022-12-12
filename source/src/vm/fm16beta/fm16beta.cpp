/*
	FUJITSU FM16beta Emulator 'eFM16beta'

	Author : Takeda.Toshiya
	Date   : 2017.12.28-

	[ virtual machine ]
*/

#include "fm16beta.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../disk.h"
#include "../hd46505.h"
#include "../i8237.h"
#include "../i8251.h"
#include "../i8259.h"
#if defined(HAS_I186)
#include "../i86.h"
#elif defined(HAS_I286)
#include "../i286.h"
#endif
#include "../io.h"
#include "../mb8877.h"
#include "../mc6809.h"
#include "../mc6840.h"
#include "../msm58321.h"
#include "../noise.h"
#include "../pcm1bit.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "cmos.h"
#include "keyboard.h"
#include "main.h"
#include "sub.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	crtc = new HD46505(this, emu);
#if defined(HAS_I186)
	cpu = new I86(this, emu);
	cpu->device_model = INTEL_80186;
#elif defined(HAS_I286)
	cpu = new I286(this, emu);
#endif
	io = new IO(this, emu);
	io->space = 0x10000;
	io->bus_width = 16;
	dma = new I8237(this, emu);
#ifdef USE_DEBUGGER
	dma->set_context_debugger(new DEBUGGER(this, emu));
#endif
	sio = new I8251(this, emu);
	pic = new I8259(this, emu);
	pic->num_chips = 2;
	fdc_2hd = new MB8877(this, emu);
	fdc_2hd->set_context_noise_seek(new NOISE(this, emu));
	fdc_2hd->set_context_noise_head_down(new NOISE(this, emu));
	fdc_2hd->set_context_noise_head_up(new NOISE(this, emu));
	fdc_2d = new MB8877(this, emu);
	fdc_2d->set_context_noise_seek(new NOISE(this, emu));
	fdc_2d->set_context_noise_head_down(new NOISE(this, emu));
	fdc_2d->set_context_noise_head_up(new NOISE(this, emu));
	subcpu = new MC6809(this, emu);
	ptm = new MC6840(this, emu);
	rtc = new MSM58321(this, emu);
	pcm = new PCM1BIT(this, emu);

	cmos = new CMOS(this, emu);
	keyboard = new KEYBOARD(this, emu);
	mainbus = new MAIN(this, emu);
#if defined(HAS_I186)
	mainbus->space = 0x0100000; // 1MB
#elif defined(HAS_I286)
	mainbus->space = 0x1000000; // 16MB
#endif
	mainbus->bank_size = 0x4000;
	mainbus->bus_width = 16;
	
	subbus = new SUB(this, emu);
	subbus->space = 0x10000;
	subbus->bank_size = 0x80;
	subbus->bus_width = 8;
	
	// set contexts
	event->set_context_cpu(cpu, 8000000);
	event->set_context_cpu(subcpu, 2000000);
	event->set_context_sound(pcm);
	event->set_context_sound(fdc_2hd->get_context_noise_seek());
	event->set_context_sound(fdc_2hd->get_context_noise_head_down());
	event->set_context_sound(fdc_2hd->get_context_noise_head_up());
	event->set_context_sound(fdc_2d->get_context_noise_seek());
	event->set_context_sound(fdc_2d->get_context_noise_head_down());
	event->set_context_sound(fdc_2d->get_context_noise_head_up());
	
	keyboard->set_context_main(mainbus);
#if defined(HAS_I286)
	mainbus->set_context_cpu(cpu);
#endif
	mainbus->set_context_dma(dma);
	mainbus->set_context_fdc_2hd(fdc_2hd);
	mainbus->set_context_fdc_2d(fdc_2d);
	mainbus->set_context_pic(pic);
	mainbus->set_context_pcm(pcm);
	mainbus->set_context_rtc(rtc);
	mainbus->set_context_sub(subbus);
	mainbus->set_context_keyboard(keyboard);

	dma->set_context_cpu(cpu);
	dma->set_context_memory(mainbus);
	dma->set_context_ch0(fdc_2d);
	dma->set_context_ch1(fdc_2hd);

	sio->set_context_txrdy(mainbus, SIG_MAIN_IRQ0_TX, 1);
	sio->set_context_rxrdy(mainbus, SIG_MAIN_IRQ0_RX, 1);
	sio->set_context_syndet(mainbus, SIG_MAIN_IRQ0_SYN, 1);

	fdc_2hd->set_context_irq(mainbus, SIG_MAIN_IRQ5, 1);
	fdc_2hd->set_context_drq(mainbus, SIG_MAIN_DRQ_2HD, 1);
	fdc_2hd->set_context_drq(dma, SIG_I8237_CH1, 1);

	fdc_2d->set_context_irq(mainbus, SIG_MAIN_IRQ4, 1);
	fdc_2d->set_context_drq(mainbus, SIG_MAIN_DRQ_2D, 1);
	fdc_2d->set_context_drq(dma, SIG_I8237_CH0, 1);

	ptm->set_context_ch0(pcm, SIG_PCM1BIT_SIGNAL, 1);
	ptm->set_context_irq(mainbus, SIG_MAIN_IRQ8, 1);
	ptm->set_internal_clock(19200); // temporary
	ptm->set_external_clock(0, 19200);
	ptm->set_external_clock(1, 19200);
	ptm->set_external_clock(2, 19200);

	rtc->set_context_data(mainbus, SIG_MAIN_RTC_DATA, 0x0f, 0);
	rtc->set_context_busy(mainbus, SIG_MAIN_RTC_BUSY, 0x80);

	crtc->set_context_disp(subbus, SIG_SUB_DISP, 1);
	crtc->set_context_vsync(subbus, SIG_SUB_VSYNC, 1);
	
	subbus->set_context_crtc(crtc);
	subbus->set_chregs_ptr(crtc->get_regs());
	subbus->set_context_pcm(pcm);
	subbus->set_context_main(mainbus);
	subbus->set_context_subcpu(subcpu);
	subbus->set_context_keyboard(keyboard);

	// cpu bus
	cpu->set_context_mem(mainbus);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
#ifdef SINGLE_MODE_DMA
	cpu->set_context_dma(dma);
#endif
	subcpu->set_context_mem(subbus);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
	subcpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	
	// i/o bus
	io->set_iomap_range_rw(0x0000, 0x0001, pic);
	io->set_iomap_range_rw(0x0010, 0x001f, dma);
	io->set_iomap_range_w(0x0020, 0x0023, mainbus);	// dma bank regs
#if defined(HAS_I286)
	io->set_iomap_single_rw(0x0060, mainbus);		// reset
#endif

	io->set_iomap_range_rw(0xf000, 0xf7ff, cmos);
	io->set_iomap_range_rw(0xfc80, 0xfcff, subbus);	// shared ram

	io->set_iomap_range_r(0xfd00, 0xfd01, keyboard);
	io->set_iomap_range_rw(0xfd02, 0xfd05, mainbus);

	io->set_iomap_range_rw(0xfd06, 0xfd07, sio);

	io->set_iomap_single_rw(0xfd0f, mainbus);

	io->set_iomap_range_rw(0xfd10, 0xfd11, mainbus);

	io->set_iomap_range_rw(0xfd18, 0xfd1b, fdc_2d);
	io->set_iomap_range_rw(0xfd1c, 0xfd1f, mainbus);

	io->set_iomap_range_r(0xfd20, 0xfd22, subbus);	// attention

	io->set_iomap_single_rw(0xfd2c, mainbus);

	io->set_iomap_range_rw(0xfd30, 0xfd33, fdc_2hd);
	io->set_iomap_range_rw(0xfd34, 0xfd37, mainbus);

	io->set_iomap_range_rw(0xfd38, 0xfd3f, ptm);
	io->set_iomap_range_rw(0xfd98, 0xfd9f, subbus);
	io->set_iomap_single_w(0xfda0, subbus);
	io->set_iomap_single_r(0xfda0, mainbus);

	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int i = 0; i < 4; i++) {
		fdc_2hd->set_drive_type(i, DRIVE_TYPE_2HD);
		fdc_2d->set_drive_type(i, DRIVE_TYPE_2D);
	}
	fdc_2hd->get_disk_handler(0)->drive_num = 0;
	fdc_2hd->get_disk_handler(1)->drive_num = 1;
	fdc_2d->get_disk_handler(0)->drive_num = 2;
	fdc_2d->get_disk_handler(1)->drive_num = 3;
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

	emu->out_debug_log(_T("----- RESET -----\n"));

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
	} else if(index == 1) {
		return subcpu;
	}
	return NULL;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	subbus->draw_screen();
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
		fdc_2hd->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc_2hd->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc_2hd->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
		fdc_2d->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc_2d->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc_2d->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	keyboard->key_down(code);
}

void VM::key_up(int code)
{
	keyboard->key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < 2) {
		fdc_2hd->open_disk(drv, file_path, bank);
	} else if(drv < 4) {
		fdc_2d->open_disk(drv - 2, file_path, bank);
	}
}

void VM::close_floppy_disk(int drv)
{
	if(drv < 2) {
		fdc_2hd->close_disk(drv);
	} else if(drv < 4) {
		fdc_2d->close_disk(drv - 2);
	}
}

bool VM::is_floppy_disk_inserted(int drv)
{
	if(drv < 2) {
		return fdc_2hd->is_disk_inserted(drv);
	} else if(drv < 4) {
		return fdc_2d->is_disk_inserted(drv - 2);
	}
	return false;
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
	if(drv < 2) {
		fdc_2hd->is_disk_protected(drv, value);
	} else if(drv < 4) {
		fdc_2d->is_disk_protected(drv - 2, value);
	}
}

bool VM::is_floppy_disk_protected(int drv)
{
	if(drv < 2) {
		return fdc_2hd->is_disk_protected(drv);
	} else if(drv < 4) {
		return fdc_2d->is_disk_protected(drv - 2);
	}
	return false;
}

uint32_t VM::is_floppy_disk_accessed()
{
	return (fdc_2hd->read_signal(0) & 3) | ((fdc_2d->read_signal(0) & 3) << 2);
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

