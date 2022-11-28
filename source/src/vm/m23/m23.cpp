/*
	SORD M23 Emulator 'Emu23'
	SORD M68 Emulator 'Emu68'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ virtual machine ]
*/

#include "m23.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../am9511.h"
#include "../disk.h"
#include "../hd46505.h"
#include "../io.h"
#include "../mb8877.h"
#include "../memory.h"
#include "../noise.h"
#include "../not.h"
#include "../pcm1bit.h"
#include "../z80.h"
#include "../z80ctc.h"
#include "../z80pio.h"
#include "../z80sio.h"
#include "../z80dma.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "apu.h"
#include "beep.h"
#include "display.h"
#include "floppy.h"
#include "keyboard.h"
#include "membus.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
#ifdef _M68
	config.drive_type = 2;
#else
	if(!(config.drive_type >= 0 && config.drive_type < 2)) {
		config.drive_type = 0;
	}
#endif
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
//	apu = new AM9511(this, emu);
	crtc = new HD46505(this, emu);
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	not_drq = new NOT(this, emu);
	cpu = new Z80(this, emu);
	ctc = new Z80CTC(this, emu);
#ifdef _M68
	ctc2 = new Z80CTC(this, emu);
#endif
	dma = new Z80DMA(this, emu);
#ifdef USE_DEBUGGER
	dma->set_context_debugger(new DEBUGGER(this, emu));
#endif
	pio = new Z80PIO(this, emu);
	sio = new Z80SIO(this, emu);
	
	apu = new APU(this, emu);
	apu->set_context_apu(new AM9511(this, emu));
	beep = new BEEP(this, emu);
	display = new DISPLAY(this, emu);
	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMBUS(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(beep);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	
	/* Z80PIO:
		Port.A	bit0-7: Output
		Port.B	bit0: Input
			bit1: Input
			bit2: Output
			bit3: Output
			bit4: Output
			bit5: Input
			bit6: Input
			bit7: Input <- FDC IRQ
	*/
	fdc->set_context_irq(pio, SIG_Z80PIO_PORT_B, 0x80);
	fdc->set_context_drq(not_drq, SIG_NOT_INPUT, 1);
	not_drq->set_context_out(dma, SIG_Z80DMA_READY, 1);
	/* Z80CTC:
		Ch.0	SIO Ch.A Baud Rate
		Ch.1	SIO Ch.B Baud Rate
		Ch.2	Keyboard Timer
		Ch.3	Software Timer
	*/
	ctc->set_context_zc0(sio, SIG_Z80SIO_TX_CLK_CH0, 1);
	ctc->set_context_zc0(sio, SIG_Z80SIO_RX_CLK_CH0, 1);
	ctc->set_context_zc1(sio, SIG_Z80SIO_TX_CLK_CH1, 1);
	ctc->set_context_zc1(sio, SIG_Z80SIO_RX_CLK_CH1, 1);
	ctc->set_constant_clock(0, CPU_CLOCKS / 13);	// ŽÀ‘ª 307.6KHz
	ctc->set_constant_clock(1, CPU_CLOCKS / 13);	// (4MHz/13•ªŽü) / CTC:2•ªŽü / SIO:16•ªŽü = 9600 baud
	ctc->set_constant_clock(2, 12500);		// ŽÀ‘ª 12.49KHz
	ctc->set_constant_clock(3, 2500);		// ŽÀ‘ª 2.499KHz
#ifdef _M68
	ctc2->set_constant_clock(0, 50);		// ŽÀ‘ª 50Hz
	ctc2->set_constant_clock(1, 50);
	ctc2->set_constant_clock(2, 50);
	ctc2->set_constant_clock(3, 50);
#endif
	dma->set_context_memory(memory);
	dma->set_context_io(io);
	
	display->set_vram_ptr(memory->get_vram());
	display->set_regs_ptr(crtc->get_regs());
	floppy->set_context_fdc(fdc);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_dma(dma);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// z80 family daisy chain
	DEVICE* parent_dev = NULL;
	int level = 0;
	
	#define Z80_DAISY_CHAIN(dev) { \
		if(parent_dev == NULL) { \
			cpu->set_context_intr(dev); \
		} else { \
			parent_dev->set_context_child(dev); \
		} \
		dev->set_context_intr(cpu, level++); \
		parent_dev = dev; \
	}
	// NOTE: IEI and IEO seems not to be connected to other Z80 family chips on M68 :-(
	Z80_DAISY_CHAIN(dma);
	Z80_DAISY_CHAIN(ctc);
#ifdef _M68
	Z80_DAISY_CHAIN(pio);
	Z80_DAISY_CHAIN(sio);
	Z80_DAISY_CHAIN(ctc2);
#else
	Z80_DAISY_CHAIN(sio);
	Z80_DAISY_CHAIN(pio);
#endif
	
	// i/o bus
	if(config.drive_type == 0) {
		// M2-FDI-I
		io->set_iomap_range_rw(0xb0, 0xb3, fdc);
		io->set_iomap_single_rw(0xb4, dma);
		io->set_iomap_single_w(0xb5, floppy);
		io->set_iomap_single_w(0xb6, memory);
	} else if(config.drive_type == 1) {
		// M2-FDI
		io->set_iomap_range_rw(0xc0, 0xc3, fdc);
		io->set_iomap_single_rw(0xc4, dma);
		io->set_iomap_single_w(0xc5, floppy);
		io->set_iomap_single_w(0xc6, memory);
	} else {
		// M2-FDI-V
		io->set_iomap_range_rw(0xc8, 0xcb, fdc);
		io->set_iomap_single_rw(0xcc, dma);
		io->set_iomap_single_w(0xcd, floppy);
		io->set_iomap_single_w(0xce, memory);
	}
	io->set_iomap_single_w(0xcf, memory);
	io->set_iomap_range_rw(0xd0, 0xd1, memory);
	io->set_iomap_single_w(0xd2, memory);
	io->set_iomap_single_rw(0xd3, beep);
	io->set_iomap_single_w(0xd4, apu);
	io->set_iomap_single_w(0xd7, display);
#ifdef _M68
	io->set_iomap_range_rw(0xd8, 0xdb, ctc2);
#endif
	io->set_iomap_range_rw(0xdc, 0xdd, apu);
	io->set_iomap_range_rw(0xe0, 0xef, keyboard);
	io->set_iomap_range_rw(0xf0, 0xf3, crtc);
	io->set_iomap_range_rw(0xf4, 0xf7, pio);
	io->set_iomap_range_rw(0xf8, 0xfb, sio);
	io->set_iomap_range_rw(0xfc, 0xff, ctc);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int drv = 0; drv < MAX_DRIVE; drv++) {
		static const struct {
			uint8_t type;
			int rpm;
			bool mfm;
		} drive_type[] = {
			{DRIVE_TYPE_2DD, 600, true}, // 3.5inch-1DD
			{DRIVE_TYPE_2DD, 300, true}, // 5.25inch-1DD
			{DRIVE_TYPE_2HD, 360, true}, // 8inch-2D
		};
		fdc->set_drive_type(drv, drive_type[config.drive_type].type);
		fdc->set_drive_rpm (drv, drive_type[config.drive_type].rpm );
		fdc->set_drive_mfm (drv, drive_type[config.drive_type].mfm );
	}
	fdc->write_signal(SIG_MB8877_MOTOR, 1, 1);
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
	display->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	beep->initialize_sound(rate, 8000);
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
		keyboard->key_down(code);
	}
}

void VM::key_up(int code)
{
}

bool VM::get_caps_locked()
{
	return keyboard->get_caps_locked();
}

bool VM::get_kana_locked()
{
	return keyboard->get_kana_locked();
}

uint32_t VM::get_led_status()
{
	return keyboard->get_led_status();
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	fdc->open_disk(drv, file_path, bank);
	
	if(fdc->is_disk_inserted(drv) && fdc->is_disk_changed(drv)) {
		pio->write_signal(SIG_Z80PIO_PORT_B, 0x80, 0x80);
	}
}

void VM::close_floppy_disk(int drv)
{
	if(fdc->is_disk_inserted(drv)) {
		fdc->close_disk(drv);
		pio->write_signal(SIG_Z80PIO_PORT_B, 0x80, 0x80);
	}
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

#define STATE_VERSION	0

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

