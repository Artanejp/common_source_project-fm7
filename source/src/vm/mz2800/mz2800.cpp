/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

	[ virtual machine ]
*/

#include "mz2800.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../disk.h"
#include "../harddisk.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../i8259.h"
#include "../i286.h"
#include "../io.h"
#include "../mb8877.h"
#include "../mz1p17.h"
#include "../noise.h"
#include "../not.h"
#include "../pcm1bit.h"
//#include "../pcpr201.h"
#include "../prnfile.h"
#include "../rp5c01.h"
#include "../scsi_hdd.h"
#include "../scsi_host.h"
#include "../upd71071.h"
#include "../ym2203.h"
#include "../z80pio.h"
#include "../z80sio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "crtc.h"
#include "floppy.h"
#include "joystick.h"
#include "keyboard.h"
#include "./memory.h"
#include "mouse.h"
#include "printer.h"
#include "reset.h"
#include "sasi.h"
#include "serial.h"
#include "sysport.h"

using MZ2800::CRTC;
using MZ2800::FLOPPY;
using MZ2800::JOYSTICK;
using MZ2800::KEYBOARD;
using MZ2800::MEMORY;
using MZ2800::MOUSE;
using MZ2800::PRINTER;
using MZ2800::RESET;
using MZ2800::SASI;
using MZ2800::SERIAL;
using MZ2800::SYSPORT;

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU_TEMPLATE* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	dummy->set_device_name(_T("1st Dummy"));

	cpu = new I286(this, emu);
	pit = new I8253(this, emu);
	pio0 = new I8255(this, emu);
	pic = new I8259(this, emu);
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
#ifdef USE_DEBUGGER
//	fdc->set_context_debugger(new DEBUGGER(this, emu));
#endif
	not_busy = new NOT(this, emu);
	not_busy->set_device_name(_T("NOT GATE(PRINTER BUSY)"));

	pcm = new PCM1BIT(this, emu);
#ifdef USE_DEBUGGER
//	pcm->set_context_debugger(new DEBUGGER(this, emu));
#endif
	rtc = new RP5C01(this, emu);	// RP-5C15
	sasi_host = new SCSI_HOST(this, emu);
	for(int i = 0; i < array_length(sasi_hdd); i++) {
		sasi_hdd[i] = new SASI_HDD(this, emu);
		sasi_hdd[i]->set_device_name(_T("SASI Hard Disk Drive #%d"), i + 1);
		sasi_hdd[i]->scsi_id = i;
		sasi_hdd[i]->bytes_per_sec = 32 * 1024; // 32KB/s
		sasi_hdd[i]->set_context_interface(sasi_host);
		sasi_host->set_context_target(sasi_hdd[i]);
	}
	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
		sasi_hdd[drv >> 1]->set_disk_handler(drv & 1, new HARDDISK(emu));
	}
	dma = new UPD71071(this, emu);
#ifdef USE_DEBUGGER
	dma->set_context_debugger(new DEBUGGER(this, emu));
#endif
	opn = new YM2203(this, emu);
#ifdef USE_DEBUGGER
	opn->set_context_debugger(new DEBUGGER(this, emu));
#endif
	pio1 = new Z80PIO(this, emu);
	sio = new Z80SIO(this, emu);
	
	crtc = new CRTC(this, emu);
	floppy = new FLOPPY(this, emu);
	joystick = new JOYSTICK(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	mouse = new MOUSE(this, emu);
	printer = new PRINTER(this, emu);
	rst = new RESET(this, emu);
	sasi = new SASI(this, emu);
	serial = new SERIAL(this, emu);
	sysport = new SYSPORT(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(opn);
	event->set_context_sound(pcm);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	
	pit->set_constant_clock(0, 31250);
	pit->set_constant_clock(2, 31250);
	pit->set_context_ch0(pit, SIG_I8253_CLOCK_1, 1);
	pit->set_context_ch0(pic, SIG_I8259_CHIP0 | SIG_I8259_IR0, 1);
	pit->set_context_ch1(pic, SIG_I8259_CHIP0 | SIG_I8259_IR6, 1);
	pit->set_context_ch2(pic, SIG_I8259_CHIP1 | SIG_I8259_IR0, 1);
	pio0->set_context_port_c(rst, SIG_RESET_CONTROL, 0xff, 0);
	pio0->set_context_port_c(crtc, SIG_CRTC_MASK, 0x01, 0);
	pio0->set_context_port_c(pcm, SIG_PCM1BIT_SIGNAL, 0x04, 0);
	// Sound:: Force realtime rendering. This is temporally fix. 20161024 K.O
	//pcm->set_realtime_render(true);

	
	pic->set_context_cpu(cpu);
	fdc->set_context_drq(dma, SIG_UPD71071_CH1, 1);
	fdc->set_context_irq(pic, SIG_I8259_CHIP0 | SIG_I8259_IR5, 1);
	not_busy->set_context_out(pic, SIG_I8259_CHIP1 | SIG_I8259_IR1, 1);
	rtc->set_context_alarm(pic, SIG_I8259_CHIP1 | SIG_I8259_IR2, 1);
	rtc->set_context_pulse(opn, SIG_YM2203_PORT_B, 8);
	sasi_host->set_context_bsy(sasi, SIG_SASI_BSY, 1);
	sasi_host->set_context_cd (sasi, SIG_SASI_CXD, 1);
	sasi_host->set_context_io (sasi, SIG_SASI_IXO, 1);
	sasi_host->set_context_msg(sasi, SIG_SASI_MSG, 1);
	sasi_host->set_context_req(sasi, SIG_SASI_REQ, 1);
	sasi_host->set_context_ack(sasi, SIG_SASI_ACK, 1);
	dma->set_context_memory(memory);
	dma->set_context_ch0(sasi);
	dma->set_context_ch1(fdc);
	dma->set_context_tc0(pic, SIG_I8259_CHIP0 | SIG_I8259_IR3, 3); // OK?
	dma->set_context_tc1(pic, SIG_I8259_CHIP0 | SIG_I8259_IR3, 3); // OK?
	dma->set_context_tc0(sasi, SIG_SASI_TC, 1);
	opn->set_context_irq(pic, SIG_I8259_CHIP1 | SIG_I8259_IR7, 1);
	opn->set_context_port_a(crtc, SIG_CRTC_PALLETE, 0x04, 0);
	opn->set_context_port_a(mouse, SIG_MOUSE_SEL, 0x08, 0);
	pio1->set_context_port_a(crtc, SIG_CRTC_COLUMN_SIZE, 0x20, 0);
	pio1->set_context_port_a(keyboard, SIG_KEYBOARD_COLUMN, 0xff, 0);
	sio->set_context_intr(pic, SIG_I8259_CHIP0 | SIG_I8259_IR2);
	sio->set_context_dtr(1, mouse, SIG_MOUSE_DTR, 1);
	
	crtc->set_context_pic(pic);
	crtc->set_context_pio(pio0);
	crtc->set_vram_ptr(memory->get_vram());
	crtc->set_tvram_ptr(memory->get_tvram());
	crtc->set_kanji_ptr(memory->get_kanji());
	crtc->set_pcg_ptr(memory->get_pcg());
	floppy->set_context_fdc(fdc);
	keyboard->set_context_pio0(pio0);
	keyboard->set_context_pio1(pio1);
	memory->set_context_crtc(crtc);
	mouse->set_context_sio(sio);
	if(config.printer_type == 0) {
		PRNFILE *prnfile = new PRNFILE(this, emu);
		prnfile->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
		printer->set_context_prn(prnfile);
	} else if(config.printer_type == 1) {
		MZ1P17 *mz1p17 = new MZ1P17(this, emu);
		mz1p17->mode = MZ1P17_MODE_MZ1;
		mz1p17->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
		printer->set_context_prn(mz1p17);
//	} else if(config.printer_type == 2) {
//		PCPR201 *pcpr201 = new PCPR201(this, emu);
//		pcpr201->set_context_busy(not_busy, SIG_NOT_INPUT, 1);
//		printer->set_context_prn(pcpr201);
	} else {
		printer->set_context_prn(dummy);
	}
	sasi->set_context_host(sasi_host);
	sasi->set_context_dma(dma);
	sasi->set_context_pic(pic);
	serial->set_context_sio(sio);
	sysport->set_context_pit(pit);
	sysport->set_context_sio(sio);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
#ifdef SINGLE_MODE_DMA
	cpu->set_context_dma(dma);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_range_rw(0x70, 0x7f, dma);
	io->set_iomap_alias_rw(0x80, pic, I8259_ADDR_CHIP0 | 0);
	io->set_iomap_alias_rw(0x81, pic, I8259_ADDR_CHIP0 | 1);
	io->set_iomap_alias_rw(0x82, pic, I8259_ADDR_CHIP1 | 0);
	io->set_iomap_alias_rw(0x83, pic, I8259_ADDR_CHIP1 | 1);
	io->set_iomap_range_rw(0x8c, 0x8d, memory);
	io->set_iovalue_single_r(0x8e, 0xff);	// dipswitch
	io->set_flipflop_single_rw(0x8f, 0x00);	// shut
	io->set_iomap_range_rw(0xa0, 0xa3, serial);
	for(uint32_t p = 0xae; p <= 0x1fae; p += 0x100) {
		io->set_iomap_single_w(p, crtc);
	}
	io->set_iomap_range_rw(0xa4, 0xa5, sasi);
	io->set_iomap_range_rw(0xb0, 0xb3, serial);
	io->set_iomap_single_r(0xbe, sysport);
	io->set_iomap_range_rw(0xc8, 0xc9, opn);
	io->set_iovalue_single_r(0xca, 0x7f);	// voice communication ???
	for(uint32_t p = 0xcc; p <= 0xfcc; p += 0x100) {
		io->set_iomap_alias_rw(p, rtc, p >> 8);
	}
	io->set_iomap_single_w(0xcd, serial);
	io->set_iomap_single_rw(0xce, memory);
	io->set_iomap_range_rw(0xd8, 0xdb, fdc);
	io->set_iomap_range_w(0xdc, 0xdf, floppy);
	io->set_iomap_range_rw(0xe0, 0xe3, pio0);
	io->set_iomap_range_rw(0xe4, 0xe7, pit);
	io->set_iomap_range_rw(0xe8, 0xeb, pio1);
	io->set_iomap_single_rw(0xef, joystick);
//	io->set_iomap_range_w(0xf0, 0xf3, sysport);
	io->set_iomap_single_w(0x170, crtc);
	io->set_iomap_single_w(0x172, crtc);
	io->set_iomap_single_w(0x174, crtc);
	io->set_iomap_single_w(0x176, crtc);
	io->set_iomap_range_w(0x178, 0x17b, crtc);
	io->set_iomap_range_rw(0x1fe, 0x1ff, printer);
	io->set_iomap_single_w(0x270, crtc);
	io->set_iomap_single_w(0x272, crtc);
	io->set_iomap_single_rw(0x274, memory);
	
	// initialize all devices
#if defined(__GIT_REPO_VERSION)
	strncpy(_git_revision, __GIT_REPO_VERSION, sizeof(_git_revision) - 1);
#endif
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
		if(!(config.last_hard_disk_path[drv][0] != _T('\0') && FILEIO::IsFileExisting(config.last_hard_disk_path[drv]))) {
			create_local_path(config.last_hard_disk_path[drv], _MAX_PATH, _T("SASI%d.DAT"), drv);
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
	
	// set initial port status
	pio0->write_signal(SIG_I8255_PORT_B, 0x7c, 0xff);
	opn->write_signal(SIG_YM2203_PORT_B, 0x37, 0xff);
}

void VM::cpu_reset()
{
	cpu->reset();
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
	opn->initialize_sound(rate, 2000000, samples, 0, -8);
	pcm->initialize_sound(rate, 4096);
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
		opn->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		opn->set_volume(1, decibel_l, decibel_r);
	} else if(ch == 2) {
		pcm->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 3) {
		fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	fdc->open_disk(drv, file_path, bank);
	
	if(fdc->get_media_type(drv) == MEDIA_TYPE_2DD) {
		if(fdc->get_drive_type(drv) == DRIVE_TYPE_2D) {
			fdc->set_drive_type(drv, DRIVE_TYPE_2DD);
		}
	} else if(fdc->get_media_type(drv) == MEDIA_TYPE_2D) {
		if(fdc->get_drive_type(drv) == DRIVE_TYPE_2DD) {
			fdc->set_drive_type(drv, DRIVE_TYPE_2D);
		}
	}
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

void VM::open_hard_disk(int drv, const _TCHAR* file_path)
{
	if(drv < USE_HARD_DISK) {
		sasi_hdd[drv >> 1]->open(drv & 1, file_path, 1024);
	}
}

void VM::close_hard_disk(int drv)
{
	if(drv < USE_HARD_DISK) {
		sasi_hdd[drv >> 1]->close(drv & 1);
	}
}

bool VM::is_hard_disk_inserted(int drv)
{
	if(drv < USE_HARD_DISK) {
		return sasi_hdd[drv >> 1]->mounted(drv & 1);
	}
	return false;
}

uint32_t VM::is_hard_disk_accessed()
{
	uint32_t status = 0;
	
	for(int drv = 0; drv < USE_HARD_DISK; drv++) {
		if(sasi_hdd[drv >> 1]->accessed(drv & 1)) {
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

double VM::get_current_usec()
{
	if(event == NULL) return 0.0;
	return event->get_current_usec();
}

uint64_t VM::get_current_clock_uint64()
{
		if(event == NULL) return (uint64_t)0;
		return event->get_current_clock_uint64();
}

#define STATE_VERSION	8

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
 	for(DEVICE* device = first_device; device; device = device->next_device) {
		// Note: typeid(foo).name is fixed by recent ABI.Not dec 6.
 		// const char *name = typeid(*device).name();
		//       But, using get_device_name() instead of typeid(foo).name() 20181008 K.O
		const char *name = device->get_device_name();
		int len = strlen(name);
		
		if(!state_fio->StateCheckInt32(len)) {
			if(loading) {
				printf("Class name len Error: DEVID=%d EXPECT=%s\n", device->this_device_id, name);
			}
			return false;
		}
		if(!state_fio->StateCheckBuffer(name, len, 1)) {
			if(loading) {
				printf("Class name Error: DEVID=%d EXPECT=%s\n", device->this_device_id, name);
			}
 			return false;
 		}
		if(!device->process_state(state_fio, loading)) {
			if(loading) {
				printf("Data loading Error: DEVID=%d\n", device->this_device_id);
			}
 			return false;
 		}
 	}
	// Machine specified.
	return true;
}
