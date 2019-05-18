/*
	Hino Electronics CEFUCOM-21 Emulator 'eCEFUCOM-21'

	Author : Takeda.Toshiya
	Date   : 2019.03.28-

	[ virtual machine ]
*/

#include "cefucom21.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../ay_3_891x.h"
#include "../datarec.h"
#include "../i8255.h"
#include "../io.h"
#include "../mc6847.h"
#include "../noise.h"
#include "../memory.h"
#include "../not.h"
#include "../rp5c01.h"
#include "../z80.h"
#include "../z80ctc.h"
#include "../z80pio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "mcu.h"
#include "pcu.h"

//#define BOOT_BASIC

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	mcu_psg = new AY_3_891X(this, emu);
#ifdef USE_DEBUGGER
	mcu_psg->set_context_debugger(new DEBUGGER(this, emu));
#endif
	mcu_drec = new DATAREC(this, emu);
	mcu_drec->set_context_noise_play(new NOISE(this, emu));
	mcu_drec->set_context_noise_stop(new NOISE(this, emu));
	mcu_drec->set_context_noise_fast(new NOISE(this, emu));
	mcu_io = new IO(this, emu);
	mcu_vdp = new MC6847(this, emu);
	mcu_mem = new MEMORY(this, emu);
	mcu_not = new NOT(this, emu);
	mcu_cpu = new Z80(this, emu);
	mcu_pio = new Z80PIO(this, emu);
	mcu = new MCU(this, emu);
	
	pcu_pio1 = new I8255(this, emu);
	pcu_pio2 = new I8255(this, emu);
	pcu_pio3 = new I8255(this, emu);
	pcu_io = new IO(this, emu);
	pcu_mem = new MEMORY(this, emu);
	pcu_rtc = new RP5C01(this, emu);
	pcu_cpu = new Z80(this, emu);
	pcu_ctc1 = new Z80CTC(this, emu);
	pcu_ctc2 = new Z80CTC(this, emu);
	pcu_pio = new Z80PIO(this, emu);
	pcu = new PCU(this, emu);
	
	// set contexts
	event->set_context_cpu(mcu_cpu);
	event->set_context_cpu(pcu_cpu);
	event->set_context_sound(mcu_psg);
	event->set_context_sound(mcu_drec);
	event->set_context_sound(mcu_drec->get_context_noise_play());
	event->set_context_sound(mcu_drec->get_context_noise_stop());
	event->set_context_sound(mcu_drec->get_context_noise_fast());
	
	mcu_vdp->load_font_image(create_local_path(_T("FONT.ROM")));
	mcu_vdp->set_vram_ptr(vram, 0x1800);
#ifdef BOOT_BASIC
	mcu_vdp->set_context_vsync(mcu_not, SIG_NOT_INPUT, 1);
	mcu_not->set_context_out(mcu_cpu, SIG_CPU_IRQ, 1);
#endif
	
	mcu_vdp->set_context_vsync(mcu, SIG_MCU_SYSPORT, 0x10);
	mcu_drec->set_context_ear(mcu, SIG_MCU_SYSPORT, 0x20);
	// bit6: printer busy
	mcu_vdp->set_context_hsync(mcu, SIG_MCU_SYSPORT, 0x80);
	
//	mcu_vdp->set_context_vsync(pcu_ctc1, SIG_Z80CTC_TRIG_1, 1);
//	mcu_vdp->set_context_vsync(pcu_ctc2, SIG_Z80CTC_TRIG_1, 1);
	mcu_vdp->set_context_vsync(mcu_pio, SIG_Z80PIO_PORT_A, 0x40);
	
	mcu_pio->set_context_port_a(pcu_pio, SIG_Z80PIO_PORT_A, 0x38, -3);
	mcu_pio->set_context_port_b(pcu_pio, SIG_Z80PIO_PORT_B, 0xff,  0);
	mcu_pio->set_context_ready_a(pcu_pio, SIG_Z80PIO_STROBE_A, 1);
	mcu_pio->set_context_ready_b(pcu_pio, SIG_Z80PIO_STROBE_B, 1);
	mcu_pio->set_hand_shake(0, true);
	mcu_pio->set_hand_shake(1, true);
	
	mcu->set_context_drec(mcu_drec);
	mcu->set_context_psg(mcu_psg);
	mcu->set_context_vdp(mcu_vdp);
	
	pcu_pio->set_context_port_a(mcu_pio, SIG_Z80PIO_PORT_A, 0x38, -3);
	pcu_pio->set_context_port_b(mcu_pio, SIG_Z80PIO_PORT_B, 0xff,  0);
	pcu_pio->set_context_ready_a(mcu_pio, SIG_Z80PIO_STROBE_A, 1);
	pcu_pio->set_context_ready_b(mcu_pio, SIG_Z80PIO_STROBE_B, 1);
	pcu_pio->set_hand_shake(0, true);
	pcu_pio->set_hand_shake(1, true);
	
	// cpu bus
	mcu_cpu->set_context_mem(mcu_mem);
	mcu_cpu->set_context_io(mcu_io);
	mcu_cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	mcu_cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	pcu_cpu->set_context_mem(pcu_mem);
	pcu_cpu->set_context_io(pcu_io);
	pcu_cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	pcu_cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// z80 family daisy chain
	DEVICE* parent_dev = NULL;
	int level = 0;
	
	#define Z80_DAISY_CHAIN(cpu, dev) { \
		if(parent_dev == NULL) { \
			cpu->set_context_intr(dev); \
		} else { \
			parent_dev->set_context_child(dev); \
		} \
		dev->set_context_intr(cpu, level++); \
		parent_dev = dev; \
	}
	Z80_DAISY_CHAIN(pcu_cpu, pcu_ctc1);
	Z80_DAISY_CHAIN(pcu_cpu, pcu_ctc2);
	Z80_DAISY_CHAIN(pcu_cpu, pcu_pio );
#ifndef BOOT_BASIC
	parent_dev = NULL;
	Z80_DAISY_CHAIN(mcu_cpu, mcu_pio );
#endif
	
	// memory bus
	memset(mcu_rom, 0xff, sizeof(mcu_rom));
	memset(mcu_ram, 0x00, sizeof(mcu_ram));
	memset(pcu_rom, 0xff, sizeof(pcu_rom));
	memset(pcu_ram, 0x00, sizeof(pcu_ram));
	
	memset(vram, 0x00, sizeof(vram));
	memset(cram, 0x00, sizeof(cram));
	
	mcu_mem->read_bios(_T("BASIC.ROM"), mcu_rom, sizeof(mcu_rom));
#ifdef BOOT_BASIC
	memset(mcu_rom + 0x7800, 0, 0x800);
#else
//	memset(mcu_rom, 0, 0x8000);
#endif
	mcu_mem->set_memory_r (0x0000, 0x6fff, mcu_rom);
	mcu_mem->set_memory_rw(0x6000, 0x77ff, vram);
	mcu_mem->set_memory_r (0x7800, 0x7fff, mcu_rom + 0x7800);
//	mcu_mem->set_memory_rw(0x8000, 0xffff, mcu_ram);
	mcu_mem->set_memory_rw(0x8000, 0xefff, mcu_ram);
	mcu_mem->set_memory_rw(0xf000, 0xffff, cram);
	
	pcu_mem->read_bios(_T("MENU.ROM"), pcu_rom, sizeof(pcu_rom));
#ifdef BOOT_BASIC
	memset(pcu_rom, 0, 0x8000);
#endif
	pcu_mem->set_memory_r (0x0000, 0x7fff, pcu_rom);
	pcu_mem->set_memory_rw(0x8000, 0xffff, pcu_ram);
	pcu_mem->set_memory_rw(0xc000, 0xcfff, cram);
	
	// i/o bus
	mcu_io->set_iomap_single_rw(0x40, mcu);
	mcu_io->set_iomap_range_r  (0x80, 0x89, mcu);
	mcu_io->set_iomap_alias_w  (0xc0, mcu_psg, 1);	// PSG data
	mcu_io->set_iomap_alias_w  (0xc1, mcu_psg, 0);	// PSG ch
//	mcu_io->set_iomap_alias_r  (0xc0, mcu_psg, 1);
	mcu_io->set_iomap_alias_r  (0xc1, mcu_psg, 1);	// PSG data
	mcu_io->set_iomap_range_rw (0xe0, 0xe3, mcu_pio);
	
	pcu_io->cpu_index = 1;
	pcu_io->set_iomap_range_rw (0x00, 0x03, pcu_ctc1);
	pcu_io->set_iomap_range_rw (0x08, 0x0b, pcu_ctc2);
	pcu_io->set_iomap_alias_rw (0x10, pcu_pio, 0);
	pcu_io->set_iomap_alias_rw (0x11, pcu_pio, 2);
	pcu_io->set_iomap_alias_rw (0x12, pcu_pio, 1);
	pcu_io->set_iomap_alias_rw (0x13, pcu_pio, 3);
	pcu_io->set_iomap_range_rw (0x20, 0x2f, pcu_rtc);
	pcu_io->set_iomap_range_rw (0x38, 0x39, pcu);
	pcu_io->set_iomap_range_rw (0x60, 0x63, pcu_pio1);
	pcu_io->set_iomap_range_rw (0x64, 0x67, pcu_pio2);
	pcu_io->set_iomap_range_rw (0x68, 0x6b, pcu_pio3);
	
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
	mcu_vdp->write_signal(SIG_MC6847_GM,  6, 7);
	mcu_vdp->write_signal(SIG_MC6847_CSS, 0, 0);
	mcu_vdp->write_signal(SIG_MC6847_AG,  0, 0);

//	pcu_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
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
		return mcu_cpu;
	} else if(index == 1) {
		return pcu_cpu;
	}
	return NULL;
}
#endif

// ----------------------------------------------------------------------------
// draw screen
// ----------------------------------------------------------------------------

void VM::draw_screen()
{
	mcu_vdp->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	mcu_psg->initialize_sound(rate, 1996750, samples, 0, 0);
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
		mcu_psg->set_volume(1, decibel_l, decibel_r);
	} else if(ch == 1) {
		mcu_drec->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 2) {
		mcu_drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
		mcu_drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
		mcu_drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	bool remote = mcu_drec->get_remote();
	
	if(mcu_drec->play_tape(file_path) && remote) {
		// if machine already sets remote on, start playing now
		push_play(drv);
	}
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	bool remote = mcu_drec->get_remote();
	
	if(mcu_drec->rec_tape(file_path) && remote) {
		// if machine already sets remote on, start recording now
		push_play(drv);
	}
}

void VM::close_tape(int drv)
{
	emu->lock_vm();
	mcu_drec->close_tape();
	emu->unlock_vm();
	mcu_drec->set_remote(false);
}

bool VM::is_tape_inserted(int drv)
{
	return mcu_drec->is_tape_inserted();
}

bool VM::is_tape_playing(int drv)
{
	return mcu_drec->is_tape_playing();
}

bool VM::is_tape_recording(int drv)
{
	return mcu_drec->is_tape_recording();
}

int VM::get_tape_position(int drv)
{
	return mcu_drec->get_tape_position();
}

const _TCHAR* VM::get_tape_message(int drv)
{
	return mcu_drec->get_message();
}

void VM::push_play(int drv)
{
	mcu_drec->set_remote(false);
	mcu_drec->set_ff_rew(0);
	mcu_drec->set_remote(true);
}

void VM::push_stop(int drv)
{
	mcu_drec->set_remote(false);
}

void VM::push_fast_forward(int drv)
{
	mcu_drec->set_remote(false);
	mcu_drec->set_ff_rew(1);
	mcu_drec->set_remote(true);
}

void VM::push_fast_rewind(int drv)
{
	mcu_drec->set_remote(false);
	mcu_drec->set_ff_rew(-1);
	mcu_drec->set_remote(true);
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

#define STATE_VERSION	5

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
		const char *name = typeid(*device).name() + 6; // skip "class "
		int len = strlen(name);
		
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

