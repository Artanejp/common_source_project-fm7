/*
	FUJITSU FM16pi Emulator 'eFM16pi'

	Author : Takeda.Toshiya
	Date   : 2010.12.25-

	[ virtual machine ]
*/

#include "fm16pi.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../disk.h"
#include "../i8251.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../i8259.h"
#include "../i86.h"
#include "../io.h"
#include "../mb8877.h"
#include "../memory.h"
#include "../msm58321.h"
#include "../noise.h"
#include "../not.h"
#include "../pcm1bit.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

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
	
	sio = new I8251(this, emu);	// for rs232c
	pit = new I8253(this, emu);
	pio = new I8255(this, emu);	// for system port
	pic = new I8259(this, emu);
	cpu = new I86(this, emu);
	cpu->device_model = INTEL_8086;
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	memory = new MEMORY(this, emu);
	rtc = new MSM58321(this, emu);
	not_pit = new NOT(this, emu);
	pcm = new PCM1BIT(this, emu);
	
	sub = new SUB(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	
/*
	IRQ 0	PIT CH.0
	IRQ 1	SIO RXRDY
	IRQ 2	SIO SYNDET
	IRQ 3	SIO TXRDY
	IRQ 4	KEY IN
	IRQ 5	EXT IRQ
	IRQ 6	PRN ACK
	IRQ 7	FDC IRQ
*/
	sio->set_context_rxrdy(pic, SIG_I8259_IR1, 1);
	sio->set_context_syndet(pic, SIG_I8259_IR2, 1);
	sio->set_context_txrdy(pic, SIG_I8259_IR3, 1);
	
/*
	TIMER 	Ch.0	IRQ0
		Ch.1	RS-232C sclock
		Ch.2	Speaker
*/
	pit->set_context_ch0(pic, SIG_I8259_IR0, 1);
	pit->set_context_ch0(not_pit, SIG_NOT_INPUT, 1);
	not_pit->set_context_out(pio, SIG_I8255_PORT_B, 0x20);
	pit->set_context_ch2(pcm, SIG_PCM1BIT_SIGNAL, 1);
	pit->set_constant_clock(0, 2457600);
	pit->set_constant_clock(1, 2457600);
	pit->set_constant_clock(2, 2457600);
	
	pic->set_context_cpu(cpu);
	
	rtc->set_context_data(sub, SIG_SUB_RTC, 0x0f, 0);
	rtc->set_context_busy(sub, SIG_SUB_RTC, 0x10);
	
	fdc->set_context_irq(pic, SIG_I8259_IR7, 1);
	fdc->set_context_irq(pio, SIG_I8255_PORT_B, 0x40);
	fdc->set_context_drq(pio, SIG_I8255_PORT_B, 0x80);
	
	sub->set_context_cpu(cpu);
	sub->set_context_fdc(fdc);
	sub->set_context_pcm(pcm);
	sub->set_context_pic(pic);
	sub->set_context_pio(pio);
	sub->set_context_rtc(rtc);
	sub->set_vram_ptr(ram + 0x78000);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// memory bus
	memset(ram, 0, sizeof(ram));
	memset(kanji, 0xff, sizeof(kanji));
	memset(cart, 0xff, sizeof(cart));
	
	memory->read_bios(_T("BACKUP.BIN"), ram, sizeof(ram));
	memory->read_bios(_T("KANJI.ROM"), kanji, sizeof(kanji));
	memory->read_bios(_T("CART.ROM"), cart, sizeof(cart));
	
	memory->set_memory_rw(0x00000, 0x6ffff, ram);
	memory->set_memory_rw(0x70000, 0x73fff, ram + 0x78000);
	memory->set_memory_rw(0x74000, 0x77fff, ram + 0x78000);
	memory->set_memory_rw(0x78000, 0x7bfff, ram + 0x78000);
	memory->set_memory_rw(0x7c000, 0x7ffff, ram + 0x78000);
	memory->set_memory_r(0x80000, 0xbffff, kanji);
	memory->set_memory_r(0xc0000, 0xfffff, cart);
	
	// i/o bus
	io->set_iomap_alias_rw(0x00, pic, 0);
	io->set_iomap_alias_rw(0x02, pic, 1);
/*
	20H	bit0-7	w	printer data
	22H	bit0	r	printer busy (1=busy)
		bit1	r	printer error (0=error)
		bit2	r	printer acknlg (0=active)
		bit3	r	printer pe (1=empty)
		bit4	r	printer slct (1=online)
		bit5	r	timer irq (0=active)
		bit6	r	fdc irq (1=active)
		bit7	r	fdc drq (1=active)
	24H	bit0	r	datarec write protect (1=protected)
		bit1	r	datarec counter pulse (4hz)
		bit2	r	datarec input
		bit3	r	key irq (0=active)
		bit4-6	w	datarec control
				0,0,0	stop
				0,0,1	ff
				0,1,0	rec
				1,0,0	pause
				1,0,1	rew
				1,1,0	play
		bit7	e	datarec output
*/
	io->set_iomap_alias_rw(0x20, pio, 0);
	io->set_iomap_alias_rw(0x22, pio, 1);
	io->set_iomap_alias_rw(0x24, pio, 2);
	io->set_iomap_alias_w(0x26, pio, 3);
	
/*
	40H	bit0-3	r	rtc data
		bit4	r	rtc busy
		bit5	r	bcr data
		bit7	r	nmi mask (0=masked)
		bit0-3	w	rtc data/addr
		bit4	w	rtc cs
		bit5	w	rtc addr write
		bit6	w	rtc read
		bit7	w	rtc write
	60H	bit0-6	r	kbd data
		bit7	r	kbd make/break (0=make)
*/
	io->set_iomap_single_rw(0x40, sub);
	io->set_iomap_single_r(0x60, sub);
	
	io->set_iomap_alias_rw(0x80, sio, 0);
	io->set_iomap_alias_rw(0x82, sio, 1);
	
/*
	A0H	bit0-1	w	rs-232c clock select
		bit2	w	printer strobe (1=on)
		bit3	w	printer irq reset (0=reset)
		bit4	w	timer irq reset (0=reset)
		bit5	w	power off (1=power off)
		bit6	w	speaker on/off (1=on)
		bit7	w	nmi mask (0=masked)
*/
	io->set_iomap_single_w(0xa0, sub);
	
/*
	C0H	bit0-7	r	fdc status register
		bit0-7	w	fdc command register
	C2H	bit0-7	rw	fdc track register
	C4H	bit0-7	rw	fdc sector register
	C6H	bit0-7	rw	fdc data register
	C8H	bit0	rw	floppy side register
	CAH	bit0-1	rw	floppy drive number
		bit6	rw	floppy drive disable (1=disable)
		bit7	rw	floppy motor on/off (1=on)
*/
	io->set_iomap_alias_rw(0xc0, fdc, 0);
	io->set_iomap_alias_rw(0xc2, fdc, 1);
	io->set_iomap_alias_rw(0xc4, fdc, 2);
	io->set_iomap_alias_rw(0xc6, fdc, 3);
	io->set_iomap_single_rw(0xc8, sub);
	io->set_iomap_single_rw(0xca, sub);
	
	io->set_iomap_alias_rw(0xe0, pit, 0);
	io->set_iomap_alias_rw(0xe2, pit, 1);
	io->set_iomap_alias_rw(0xe4, pit, 2);
	io->set_iomap_alias_w(0xe6, pit, 3);
	
/*
	400H	bit0	w	memory write protect 20000H-47FFFH
		bit1	w	memory write protect 48000H-6FFFFH
*/
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2D);
	}
}

VM::~VM()
{
	// save memory
	memory->write_bios(_T("BACKUP.BIN"), ram, sizeof(ram));
	
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
	
	// initial device settings
	pio->write_signal(SIG_I8255_PORT_B, 0x3f, 0xff);	// printer disconnected
	pio->write_signal(SIG_I8255_PORT_C, 0x0c, 0x0f);
}

void VM::notify_power_off()
{
//	this->out_debug_log(_T("--- POWER OFF ---\n"));
	sub->notify_power_off();
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
	sub->key_down(code);
}

void VM::key_up(int code)
{
	sub->key_up(code);
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
		const char *name = typeid(*device).name() + 6; // skip "class "
		int len = (int)strlen(name);
		
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

