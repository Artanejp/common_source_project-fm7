/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.29 -

	[ virtual machine ]
*/

#include "fmr30.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../i8237.h"
#include "../i8251.h"
#include "../i8253.h"
#include "../i8259.h"
#include "../i286.h"
#include "../io.h"
#include "../mb8877.h"
#include "../scsi_hdd.h"
#include "../scsi_host.h"
#include "../sn76489an.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "../fmr50/bios.h"
#include "cmos.h"
#include "floppy.h"
#include "keyboard.h"
#include "memory.h"
#include "rtc.h"
#include "scsi.h"
#include "serial.h"
#include "system.h"
#include "timer.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	dma = new I8237(this, emu);
	sio_kb = new I8251(this, emu);	// keyboard
	sio_sub = new I8251(this, emu);	// sub display
	sio_ch1 = new I8251(this, emu);	// RS-232C ch.1
	sio_ch2 = new I8251(this, emu);	// RS-232C ch.2
	pit = new I8253(this, emu);
	pic = new I8259(this, emu);
	cpu = new I286(this, emu);
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);
#if defined(_USE_QT)
	dummy->set_device_name(_T("1st Dummy"));
	event->set_device_name(_T("EVENT"));

	dma->set_device_name(_T("i8237 DMAC"));
	sio_kb->set_device_name(_T("i8251 SIO(KEYBOARD)"));
	sio_sub->set_device_name(_T("i8251 SIO(SUB SYSTEM)"));
	sio_ch1->set_device_name(_T("i8251 SIO(RS-232C #1)"));
	sio_ch2->set_device_name(_T("i8251 SIO(RS-232C #2)"));
	pit->set_device_name(_T("i8253 PIT"));
	pic->set_device_name(_T("i8259 PIC"));
	cpu->set_device_name(_T("CPU(i286)"));
#endif
	
	scsi_host = new SCSI_HOST(this, emu);
#if defined(_USE_QT)
	scsi_host->set_device_name(_T("SCSI HOST"));
#endif	
	for(int i = 0; i < 7; i++) {
		if(FILEIO::IsFileExisting(create_local_path(_T("SCSI%d.DAT"), i))) {
			SCSI_HDD* scsi_hdd = new SCSI_HDD(this, emu);
#if defined(_USE_QT)
			char d_name[64] = {0};
			snprintf(d_name, 64, "SCSI DISK #%d", i + 1);
			scsi_hdd->set_device_name(d_name);
#endif			
			scsi_hdd->scsi_id = i;
			scsi_hdd->set_context_interface(scsi_host);
			scsi_host->set_context_target(scsi_hdd);
		}
	}
	psg = new SN76489AN(this, emu);
#if defined(_USE_QT)
	psg->set_device_name(_T("SN76489 PSG"));
#endif	
	if(FILEIO::IsFileExisting(create_local_path(_T("IPL.ROM")))) {
		bios = NULL;
	} else {
		bios = new BIOS(this, emu);
#if defined(_USE_QT)
		bios->set_device_name(_T("PSEUDO BIOS"));
#endif	
	}
	cmos = new CMOS(this, emu);
	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	rtc = new RTC(this, emu);
	scsi = new SCSI(this, emu);
	serial = new SERIAL(this, emu);
	system = new SYSTEM(this, emu);
	timer = new TIMER(this, emu);
#if defined(_USE_QT)
	cmos->set_device_name(_T("CMOS RAM"));
	floppy->set_device_name(_T("FLOPPY I/F"));
	keyboard->set_device_name(_T("KEYBOARD"));
	memory->set_device_name(_T("MEMORY"));
	rtc->set_device_name(_T("RTC"));
	scsi->set_device_name(_T("SCSI I/F"));
	serial->set_device_name(_T("SERIAL I/F"));
	system->set_device_name(_T("SYSTEM I/O"));
	timer->set_device_name(_T("TIMER I/F"));
#endif
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	
	dma->set_context_memory(memory);
	dma->set_context_ch0(fdc);
	dma->set_context_ch1(scsi_host);
	dma->set_context_tc1(scsi, SIG_SCSI_TC, 1);
	sio_kb->set_context_rxrdy(serial, SIG_SERIAL_RXRDY_KB, 1);
	sio_kb->set_context_txrdy(serial, SIG_SERIAL_TXRDY_KB, 1);
	sio_sub->set_context_rxrdy(serial, SIG_SERIAL_RXRDY_SUB, 1);
	sio_sub->set_context_txrdy(serial, SIG_SERIAL_TXRDY_SUB, 1);
	sio_ch1->set_context_rxrdy(serial, SIG_SERIAL_RXRDY_CH1, 1);
	sio_ch1->set_context_txrdy(serial, SIG_SERIAL_TXRDY_CH1, 1);
	sio_ch2->set_context_rxrdy(serial, SIG_SERIAL_RXRDY_CH2, 1);
	sio_ch2->set_context_txrdy(serial, SIG_SERIAL_TXRDY_CH2, 1);
	pit->set_context_ch0(timer, SIG_TIMER_CH0, 1);
	pit->set_context_ch1(timer, SIG_TIMER_CH1, 1);
	pit->set_constant_clock(0, 1000000);
	pit->set_constant_clock(1, 1000000);
	pic->set_context_cpu(cpu);
	fdc->set_context_drq(dma, SIG_I8237_CH0, 1);
	fdc->set_context_irq(floppy, SIG_FLOPPY_IRQ, 1);
	scsi_host->set_context_irq(scsi, SIG_SCSI_IRQ, 1);
	scsi_host->set_context_drq(scsi, SIG_SCSI_DRQ, 1);
	
	floppy->set_context_fdc(fdc);
	floppy->set_context_pic(pic);
	keyboard->set_context_sio(sio_kb);
	memory->set_context_cpu(cpu);
	memory->set_context_dma(dma);
	rtc->set_context_pic(pic);
	scsi->set_context_dma(dma);
	scsi->set_context_pic(pic);
	scsi->set_context_host(scsi_host);
	serial->set_context_pic(pic);
	serial->set_context_sio(sio_kb, sio_sub, sio_ch1, sio_ch2);
	timer->set_context_pic(pic);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
	if(bios) {
		bios->set_context_mem(memory);
		bios->set_context_io(io);
		bios->set_cmos_ptr(cmos->get_cmos());
		bios->set_vram_ptr(memory->get_vram());
		bios->set_cvram_ptr(memory->get_cvram());
		bios->set_kvram_ptr(memory->get_kvram());
		cpu->set_context_bios(bios);
	}
#ifdef SINGLE_MODE_DMA
	cpu->set_context_dma(dma);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_range_rw(0x00, 0x07, rtc);
	io->set_iomap_range_rw(0x08, 0x09, sio_kb);
	io->set_iomap_range_rw(0x0a, 0x0b, serial);
	io->set_iomap_range_rw(0x10, 0x11, sio_sub);
	io->set_iomap_range_rw(0x12, 0x13, serial);
	io->set_iomap_single_r(0x18, system);
	io->set_iomap_range_rw(0x1d, 0x1e, memory);
	io->set_iomap_range_r(0x20, 0x21, system);
	io->set_iomap_single_rw(0x26, memory);
	io->set_iomap_range_rw(0x30, 0x33, fdc);
	io->set_iomap_range_rw(0x34, 0x36, floppy);
	io->set_iomap_single_w(0x40, psg);
	io->set_iomap_range_rw(0x42, 0x43, timer);
	io->set_iomap_range_rw(0x46, 0x47, system);
	io->set_iomap_range_rw(0x60, 0x61, sio_ch1);
	io->set_iomap_range_rw(0x62, 0x66, serial);
	io->set_iomap_range_rw(0x70, 0x71, sio_ch2);
	io->set_iomap_range_rw(0x72, 0x76, serial);
	io->set_iomap_alias_rw(0x100, pic, I8259_ADDR_CHIP0 | 0);
	io->set_iomap_alias_rw(0x101, pic, I8259_ADDR_CHIP0 | 1);
	io->set_iomap_alias_rw(0x108, pic, I8259_ADDR_CHIP1 | 0);
	io->set_iomap_alias_rw(0x10a, pic, I8259_ADDR_CHIP1 | 1);
	io->set_iomap_range_rw(0x110, 0x11f, dma);
	io->set_iomap_range_w(0x120, 0x123, memory);
	io->set_iomap_range_rw(0x130, 0x133, pit);
	io->set_iomap_range_rw(0x2f0, 0x2f3, scsi);
	io->set_iomap_range_rw(0x300, 0x30f, memory);
	io->set_iomap_range_rw(0xc000, 0xdfff, cmos);
	io->set_iomap_single_rw(0xff00, system);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	if(bios) {
		for(int i = 0; i < MAX_DRIVE; i++) {
			bios->set_disk_handler(i, fdc->get_disk_handler(i));
		}
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
	// temporary fix...
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
	
	// set devices
	sio_kb->write_signal(SIG_I8251_DSR, 1, 1);
	sio_sub->write_signal(SIG_I8251_DSR, 0, 0);
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

uint32_t VM::get_access_lamp_status()
{
	uint32_t status_fdd = fdc->read_signal(0);
	uint32_t status_hdd = scsi_host->read_signal(0);
	if(bios) {
		uint32_t status = bios->read_signal(0);
		status_fdd |= status & 0x0f;
		status_hdd |= status >> 4;
	}
	return (status_hdd) ? 4 : (status_fdd & (1 | 4)) ? 1 : (status_fdd & (2 | 8)) ? 2 : 0;
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	psg->initialize_sound(rate, 125000, 10000);
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
		psg->set_volume(0, decibel_l, decibel_r);
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
	fdc->open_disk(drv, file_path, bank);
	floppy->change_disk(drv);
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

