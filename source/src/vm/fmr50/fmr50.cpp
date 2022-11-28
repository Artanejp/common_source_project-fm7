/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.04.28 -

	[ virtual machine ]
*/

#include "fmr50.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../harddisk.h"
#include "../hd46505.h"
#ifdef _FMR60
#include "../hd63484.h"
#endif
#include "../i8251.h"
#include "../i8253.h"
#include "../i8259.h"
#if defined(HAS_I286)
//#include "../i286_np21.h"
#include "../i286.h"
#else
#include "../i386_np21.h"
//#include "../i386.h"
#endif
#include "../io.h"
#include "../mb8877.h"
#include "../msm58321.h"
#include "../noise.h"
#include "../pcm1bit.h"
#include "../scsi_hdd.h"
#include "../scsi_host.h"
#include "../upd71071.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "bios.h"
#include "cmos.h"
#include "floppy.h"
#include "keyboard.h"
#include "memory.h"
#include "scsi.h"
//#include "serial.h"
#include "timer.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
/*
	Machine ID & CPU ID

	FMR-50FD/HD/LT	0xF8
	FMR-50FX/HX	0xE0
	FMR-50SFX/SHX	0xE8
	FMR-50LT	0xF8
	FMR-50NBX	0x28
	FMR-50NB	0x60
	FMR-50NE/T	0x08
	FMR-CARD	0x70

	80286		0x00
	80386		0x01
	80386SX		0x03
	80486		0x02
*/
	static const int cpu_clock[] = {
#if defined(HAS_I286)
		 8000000, 12000000
#elif defined(HAS_I386)
		16000000, 20000000
#elif defined(HAS_I486)
		20000000, 25000000
#endif
	};
	
#if defined(_FMR60) && (defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM))
	uint8_t machine_id = 0xf0;	// FMR-70/80
#else
	uint8_t machine_id = 0xf8;	// FMR-50/60
#endif
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("MACHINE.ID")), FILEIO_READ_BINARY)) {
		machine_id = fio->Fgetc();
		fio->Fclose();
	}
	delete fio;
	
	machine_id &= ~7;
#if defined(HAS_I286)
	machine_id |= 0;	// 286
#elif defined(HAS_I386)
//	machine_id |= 1;	// 386DX
	machine_id |= 3;	// 386SX
#elif defined(HAS_I486)
	machine_id |= 2;	// 486SX/DX
#endif
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
#if defined(HAS_I286)
	cpu = new I286(this, emu);
//	cpu->device_model = INTEL_80286;
#elif defined(HAS_I386)
	cpu = new I386(this, emu);
	cpu->device_model = INTEL_80386;
#elif defined(HAS_I486)
	cpu = new I386(this, emu);
#if defined(HAS_I486DX)
	cpu->device_model = INTEL_I486DX;
#else
	cpu->device_model = INTEL_I486SX;
#endif
#endif
	crtc = new HD46505(this, emu);
#ifdef _FMR60
	acrtc = new HD63484(this, emu);
#endif
	sio = new I8251(this, emu);
	pit0 = new I8253(this, emu);
	pit0->set_device_name(_T("8253 PIT #0"));
	pit1 = new I8253(this, emu);
	pit1->set_device_name(_T("8253 PIT #1"));
	pic = new I8259(this, emu);
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	rtc = new MSM58321(this, emu);
	pcm = new PCM1BIT(this, emu);
	scsi_host = new SCSI_HOST(this, emu);
	for(int i = 0; i < USE_HARD_DISK; i++) {
		scsi_hdd[i] = new SCSI_HDD(this, emu);
		scsi_hdd[i]->set_device_name(_T("SCSI Hard Disk Drive #%d"), i + 1);
		scsi_hdd[i]->scsi_id = i;
		scsi_hdd[i]->set_disk_handler(0, new HARDDISK(emu));
		scsi_hdd[i]->set_context_interface(scsi_host);
		scsi_host->set_context_target(scsi_hdd[i]);
	}
	dma = new UPD71071(this, emu);
#ifdef USE_DEBUGGER
	dma->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	if(FILEIO::IsFileExisting(create_local_path(_T("IPL.ROM")))) {
		bios = NULL;
	} else {
		bios = new BIOS(this, emu);
	}
	cmos = new CMOS(this, emu);
	floppy = new FLOPPY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	scsi = new SCSI(this, emu);
//	serial = new SERIAL(this, emu);
	timer = new TIMER(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu, cpu_clock[config.cpu_type & 1]);
	event->set_context_sound(pcm);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	
/*	pic	0	timer
		1	keyboard
		2	rs-232c
		3	ex rs-232c
		4	(option)
		5	(option)
		6	floppy drive or dma ???
		7	(slave)
		8	scsi
		9	(option)
		10	(option)
		11	(option)
		12	printer
		13	(option)
		14	(option)
		15	(reserve)

	dma	0	floppy drive
		1	hard drive
		2	(option)
		3	(reserve)
*/
	crtc->set_context_disp(memory, SIG_MEMORY_DISP, 1);
	crtc->set_context_vsync(memory, SIG_MEMORY_VSYNC, 1);
#ifdef _FMR60
	acrtc->set_vram_ptr((uint16_t*)memory->get_vram(), 0x80000);
#endif
	pit0->set_context_ch0(timer, SIG_TIMER_CH0, 1);
	pit0->set_context_ch1(timer, SIG_TIMER_CH1, 1);
	pit0->set_context_ch2(pcm, SIG_PCM1BIT_SIGNAL, 1);
	pit0->set_constant_clock(0, 307200);
	pit0->set_constant_clock(1, 307200);
	pit0->set_constant_clock(2, 307200);
	pit1->set_constant_clock(1, 1228800);
	pic->set_context_cpu(cpu);
	fdc->set_context_drq(dma, SIG_UPD71071_CH0, 1);
	fdc->set_context_irq(floppy, SIG_FLOPPY_IRQ, 1);
	rtc->set_context_data(timer, SIG_TIMER_RTC, 0x0f, 0);
	rtc->set_context_busy(timer, SIG_TIMER_RTC, 0x80);
	scsi_host->set_context_irq(scsi, SIG_SCSI_IRQ, 1);
	scsi_host->set_context_drq(scsi, SIG_SCSI_DRQ, 1);
	dma->set_context_memory(memory);
	dma->set_context_ch0(fdc);
	dma->set_context_ch1(scsi_host);
	
	floppy->set_context_fdc(fdc);
	floppy->set_context_pic(pic);
	keyboard->set_context_pic(pic);
	memory->set_context_cpu(cpu);
	memory->set_machine_id(machine_id);
	memory->set_context_crtc(crtc);
	memory->set_chregs_ptr(crtc->get_regs());
	scsi->set_context_dma(dma);
	scsi->set_context_pic(pic);
	scsi->set_context_host(scsi_host);
	timer->set_context_pcm(pcm);
	timer->set_context_pic(pic);
	timer->set_context_rtc(rtc);
	
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
#ifdef _FMR60
		bios->set_avram_ptr(memory->get_avram());
#else
		bios->set_kvram_ptr(memory->get_kvram());
#endif
		cpu->set_context_bios(bios);
	}
#ifdef SINGLE_MODE_DMA
	cpu->set_context_dma(dma);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_alias_rw(0x00, pic, I8259_ADDR_CHIP0 | 0);
	io->set_iomap_alias_rw(0x02, pic, I8259_ADDR_CHIP0 | 1);
	io->set_iomap_alias_rw(0x10, pic, I8259_ADDR_CHIP1 | 0);
	io->set_iomap_alias_rw(0x12, pic, I8259_ADDR_CHIP1 | 1);
	io->set_iomap_single_rw(0x20, memory);	// reset
	io->set_iomap_single_r(0x21, memory);	// cpu misc
	io->set_iomap_single_w(0x22, memory);	// dma
	io->set_iomap_single_rw(0x24, memory);	// dma
	io->set_iomap_single_r(0x26, timer);
	io->set_iomap_single_r(0x27, timer);
	io->set_iomap_single_r(0x30, memory);	// cpu id
	io->set_iomap_alias_rw(0x40, pit0, 0);
	io->set_iomap_alias_rw(0x42, pit0, 1);
	io->set_iomap_alias_rw(0x44, pit0, 2);
	io->set_iomap_alias_rw(0x46, pit0, 3);
	io->set_iomap_alias_rw(0x50, pit1, 0);
	io->set_iomap_alias_rw(0x52, pit1, 1);
	io->set_iomap_alias_rw(0x54, pit1, 2);
	io->set_iomap_alias_rw(0x56, pit1, 3);
	io->set_iomap_single_rw(0x60, timer);
	io->set_iomap_single_rw(0x70, timer);
	io->set_iomap_single_w(0x80, timer);
#ifdef _FMRCARD
	io->set_iomap_single_w(0x90, cmos);
#endif
	io->set_iomap_range_rw(0xa0, 0xaf, dma);
	io->set_iomap_alias_rw(0x200, fdc, 0);
	io->set_iomap_alias_rw(0x202, fdc, 1);
	io->set_iomap_alias_rw(0x204, fdc, 2);
	io->set_iomap_alias_rw(0x206, fdc, 3);
	io->set_iomap_single_rw(0x208, floppy);
	io->set_iomap_single_rw(0x20c, floppy);
	io->set_iomap_single_rw(0x400, memory);	// crtc
	io->set_iomap_single_rw(0x402, memory);	// crtc
	io->set_iomap_single_rw(0x404, memory);	// crtc
	io->set_iomap_single_w(0x408, memory);	// crtc
	io->set_iomap_single_rw(0x40a, memory);	// crtc
	io->set_iomap_single_rw(0x40c, memory);	// crtc
	io->set_iomap_single_rw(0x40e, memory);	// crtc
	io->set_iomap_alias_rw(0x500, crtc, 0);
	io->set_iomap_alias_rw(0x502, crtc, 1);
#ifdef _FMR60
	io->set_iomap_range_rw(0x520, 0x523, acrtc);
#endif
	io->set_iomap_single_rw(0x600, keyboard);
	io->set_iomap_single_rw(0x602, keyboard);
	io->set_iomap_single_rw(0x604, keyboard);
	io->set_iomap_alias_rw(0xa00, sio, 0);
	io->set_iomap_alias_rw(0xa02, sio, 1);
//	io->set_iomap_single_r(0xa04, serial);
//	io->set_iomap_single_r(0xa06, serial);
//	io->set_iomap_single_w(0xa08, serial);
	io->set_iomap_single_rw(0xc30, scsi);
	io->set_iomap_single_rw(0xc32, scsi);
	io->set_iomap_range_rw(0x3000, 0x3fff, cmos);
	io->set_iomap_range_rw(0xfd98, 0xfd9f, memory);	// crtc
	io->set_iomap_single_rw(0xfda0, memory);	// crtc
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
		if(!(config.last_hard_disk_path[drv][0] != _T('\0') && FILEIO::IsFileExisting(config.last_hard_disk_path[drv]))) {
			create_local_path(config.last_hard_disk_path[drv], _MAX_PATH, _T("SCSI%d.DAT"), drv);
		}
	}
	if(bios) {
		for(int drv = 0; drv < MAX_DRIVE; drv++) {
			bios->set_floppy_disk_handler(drv, fdc->get_disk_handler(drv));
		}
		for(int drv = 0; drv < USE_HARD_DISK; drv++) {
			bios->set_hard_disk_handler(drv, scsi_hdd[drv]->get_disk_handler(0));
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

uint32_t VM::is_floppy_disk_accessed()
{
	uint32_t status = fdc->read_signal(0);
	if(bios) {
		status |= bios->read_signal(0);
	}
	return status;
}

void VM::open_hard_disk(int drv, const _TCHAR* file_path)
{
	if(drv < USE_HARD_DISK) {
		scsi_hdd[drv]->open(0, file_path, 512);
	}
}

void VM::close_hard_disk(int drv)
{
	if(drv < USE_HARD_DISK) {
		scsi_hdd[drv]->close(0);
	}
}

bool VM::is_hard_disk_inserted(int drv)
{
	if(drv < USE_HARD_DISK) {
		return scsi_hdd[drv]->mounted(0);
	}
	return false;
}

uint32_t VM::is_hard_disk_accessed()
{
	uint32_t status = 0;
	
	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
		if(scsi_hdd[drv]->accessed(0)) {
			status |= 1 << drv;
		}
	}
	return status;
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

#define STATE_VERSION	7

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
	return true;
}

