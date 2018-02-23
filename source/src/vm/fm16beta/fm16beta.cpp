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
#include "../i286.h"
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

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	crtc = new HD46505(this, emu);
	cpu = new I286(this, emu);
	io = new IO(this, emu);
	dma = new I8237(this, emu);
	sio = new I8251(this, emu);
	pic = new I8259(this, emu);
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
	main = new MAIN(this, emu);
	

	
	
	
	sub = new SUB(this, emu);
	
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
	
	keyboard->set_context_main(main);
#ifdef HAS_I286
	main->set_context_cpu(cpu);
#endif
	main->set_context_dma(dma);
	main->set_context_fdc_2hd(fdc_2hd);
	main->set_context_fdc_2d(fdc_2d);
	main->set_context_pic(pic);
	main->set_context_pcm(pcm);
	main->set_context_rtc(rtc);
	main->set_context_sub(sub);
	main->set_context_keyboard(keyboard);

	dma->set_context_memory(main);
	dma->set_context_ch0(fdc_2d);
	dma->set_context_ch1(fdc_2hd);

	sio->set_context_txrdy(main, SIG_MAIN_IRQ0_TX, 1);
	sio->set_context_rxrdy(main, SIG_MAIN_IRQ0_RX, 1);
	sio->set_context_syndet(main, SIG_MAIN_IRQ0_SYN, 1);

	fdc_2hd->set_context_irq(main, SIG_MAIN_IRQ5, 1);
	fdc_2hd->set_context_drq(main, SIG_MAIN_DRQ_2HD, 1);
	fdc_2hd->set_context_drq(dma, SIG_I8237_CH1, 1);

	fdc_2d->set_context_irq(main, SIG_MAIN_IRQ4, 1);
	fdc_2d->set_context_drq(main, SIG_MAIN_DRQ_2D, 1);
	fdc_2d->set_context_drq(dma, SIG_I8237_CH0, 1);

	ptm->set_context_ch0(pcm, SIG_PCM1BIT_SIGNAL, 1);
	ptm->set_context_irq(main, SIG_MAIN_IRQ8, 1);
	ptm->set_internal_clock(19200); // temporary
	ptm->set_external_clock(0, 19200);
	ptm->set_external_clock(1, 19200);
	ptm->set_external_clock(2, 19200);

	rtc->set_context_data(main, SIG_MAIN_RTC_DATA, 0x0f, 0);
	rtc->set_context_busy(main, SIG_MAIN_RTC_BUSY, 0x80);

	crtc->set_context_disp(sub, SIG_SUB_DISP, 1);
	crtc->set_context_vsync(sub, SIG_SUB_VSYNC, 1);
	
	sub->addr_max = 0x10000;
	sub->bank_size = 0x80;
	sub->set_context_crtc(crtc);
	sub->set_chregs_ptr(crtc->get_regs());
	sub->set_context_pcm(pcm);
	sub->set_context_main(main);
	sub->set_context_subcpu(subcpu);
	sub->set_context_keyboard(keyboard);

	// cpu bus
	cpu->set_context_mem(main);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
#ifdef SINGLE_MODE_DMA
	cpu->set_context_dma(dma);
#endif
	subcpu->set_context_mem(sub);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
	subcpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	
	// i/o bus
	io->set_iomap_range_rw(0x0000, 0x0001, pic);
	io->set_iomap_range_rw(0x0010, 0x001f, dma);
	io->set_iomap_range_w(0x0020, 0x0023, main);	// dma bank regs
#ifdef HAS_I286
	io->set_iomap_single_rw(0x0060, main);		// reset
#endif

	io->set_iomap_range_rw(0xf000, 0xf7ff, cmos);
	io->set_iomap_range_rw(0xfc80, 0xfcff, sub);	// shared ram

	io->set_iomap_range_r(0xfd00, 0xfd01, keyboard);
	io->set_iomap_range_rw(0xfd02, 0xfd05, main);

	io->set_iomap_range_rw(0xfd06, 0xfd07, sio);

	io->set_iomap_single_rw(0xfd0f, main);

	io->set_iomap_range_rw(0xfd10, 0xfd11, main);

	io->set_iomap_range_rw(0xfd18, 0xfd1b, fdc_2d);
	io->set_iomap_range_rw(0xfd1c, 0xfd1f, main);

	io->set_iomap_range_r(0xfd20, 0xfd22, sub);	// attention

	io->set_iomap_single_rw(0xfd2c, main);

	io->set_iomap_range_rw(0xfd30, 0xfd33, fdc_2hd);
	io->set_iomap_range_rw(0xfd34, 0xfd37, main);

	io->set_iomap_range_rw(0xfd38, 0xfd3f, ptm);
	io->set_iomap_range_rw(0xfd98, 0xfd9f, sub);
	io->set_iomap_single_w(0xfda0, sub);
	io->set_iomap_single_r(0xfda0, main);

	
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
	sub->draw_screen();
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

#define STATE_VERSION	1

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

