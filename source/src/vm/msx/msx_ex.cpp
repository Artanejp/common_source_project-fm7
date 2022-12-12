/*
	Common Source Code Project
	MSX Series (experimental)

	Origin : src/vm/msx/msx.cpp

	modified by umaiboux
	Date   : 2016.03.xx-

	[ virtual machine ]
*/

//#define USE_PORT_F4

#include "msx_ex.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../i8255.h"
#include "../io.h"
#if defined(LDC_SLOT)
#include "../ld700.h"
#endif
#include "../noise.h"
#include "../not.h"
//#include "../ym2203.h"
#include "../ay_3_891x.h"
#include "../pcm1bit.h"
#if !defined(_MSX1_VARIANTS)
#include "../rp5c01.h"
#if defined(_MSX_VDP_MESS)
#include "../v9938.h"
#else
#include "../v99x8.h"
#endif
#else
#include "../tms9918a.h"
#endif
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "joystick.h"
#include "keyboard.h"
#include "memory_ex.h"
#if !defined(_MSX1_VARIANTS)
#include "rtcif.h"
#endif
#include "kanjirom.h"
#include "sound_cart.h"
#ifdef USE_PRINTER
#include "printer.h"
//#include "../pcpr201.h"
#include "../prnfile.h"
#if defined(_MZP)
// test
#include "../mz1p17.h"
#endif
#endif
#include "../ym2413.h"

#if defined(MSX_PSG_STEREO)
#include "psg_stereo.h"
#endif

#ifdef USE_PORT_F4
class PORT_F4 : public DEVICE
{
private:
	uint8_t port;
	
public:
	PORT_F4(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		port = 0;
	}
	~PORT_F4() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data) {
		port = data & 0xFF;
	}
	uint32_t read_io8(uint32_t addr) {
		return port&0xFF;
	}
	void reset() {
		port = 0;
	}
};
#endif

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
	pio = new I8255(this, emu);
	io = new IO(this, emu);
	io->space = 0x100;
#if defined(LDC_SLOT)
	ldp = new LD700(this, emu);
#endif
	not_remote = new NOT(this, emu);
	psg = new AY_3_891X(this, emu);
#ifdef USE_DEBUGGER
	psg->set_context_debugger(new DEBUGGER(this, emu));
#endif
	pcm = new PCM1BIT(this, emu);
#if !defined(_MSX1_VARIANTS)
	rtc = new RP5C01(this, emu);
	vdp = new V99X8(this, emu);
#else
	vdp = new TMS9918A(this, emu);
#ifdef USE_DEBUGGER
	vdp->set_context_debugger(new DEBUGGER(this, emu));
#endif
#endif
	cpu = new Z80(this, emu);
	
	joystick = new JOYSTICK(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY_EX(this, emu);
#if !defined(_MSX1_VARIANTS)
	rtcif = new RTCIF(this, emu);
#endif
//	slot0 = new SLOT0(this, emu);	// #0: main memory
//	slot1 = new SLOT1(this, emu);	// #1: rom-cartridge or msx-dos
//	slot2 = new SLOT2(this, emu);	// #2: fdd-cartridge or p-basic
//	slot3 = new SLOT3(this, emu);	// #3: rom-cartridge or ram-cartridge
	slot_mainrom = new SLOT_MAINROM(this, emu);
	slot_cart[0] = new SLOT_CART(this, emu);
	slot_cart[1] = new SLOT_CART(this, emu);
	sound_cart[0] = new SOUND_CART(this, emu);
	sound_cart[1] = new SOUND_CART(this, emu);
	kanjirom = new KANJIROM(this, emu);
#ifdef USE_PRINTER
	printer = new PRINTER(this, emu);
#endif
#if defined(LDC_SLOT)
	slot_ldc = new SLOT_LDC(this, emu);
#endif
#if defined(MAPPERRAM_SLOT)
	slot_ram = new SLOT_MAPPERRAM(this, emu);
#endif
#if defined(RAM64K_SLOT)
	slot_ram = new SLOT_RAM64K(this, emu);
#endif
#if defined(SUBROM_SLOT)
	slot_subrom = new SLOT_SUBROM(this, emu);
#endif
#if defined(FIRMWARE32K1_SLOT)
	slot_firmware32k1 = new SLOT_FIRMWARE32K(this, emu);
#endif
#if defined(FIRMWARE32K2_SLOT)
	slot_firmware32k2 = new SLOT_FIRMWARE32K(this, emu);
#endif
#if defined(FDD_PATCH_SLOT)
	slot_fdd_patch = new SLOT_FDD_PATCH(this, emu);
#endif
#if defined(MSXDOS2_SLOT)
	slot_msxdos2 = new SLOT_MSXDOS2(this, emu);
#endif
	ym2413 = new YM2413(this, emu);
#if defined(MSXMUSIC_SLOT)
	slot_msxmusic = new SLOT_MSXMUSIC(this, emu);
#endif
#if defined(MSX_PSG_STEREO)
	psg_stereo = new PSG_STEREO(this, emu);
#endif
	
	// set contexts
	event->set_context_cpu(cpu);
#if defined(MSX_PSG_STEREO)
	event->set_context_sound(psg_stereo);
#else
	event->set_context_sound(psg);
#endif
	event->set_context_sound(pcm);
	event->set_context_sound(drec);
#if defined(LDC_SLOT)
	event->set_context_sound(ldp);
#endif
	event->set_context_sound(ym2413);
	event->set_context_sound(sound_cart[0]);
	event->set_context_sound(sound_cart[1]);
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	slot_cart[0]->set_context_sound(sound_cart[0]);
	slot_cart[1]->set_context_sound(sound_cart[1]);
	
	drec->set_context_ear(psg, SIG_AY_3_891X_PORT_A, 0x80);
	pio->set_context_port_a(memory, SIG_MEMORY_SEL, 0xff, 0);
	pio->set_context_port_c(keyboard, SIG_KEYBOARD_COLUMN, 0x0f, 0);
	pio->set_context_port_c(not_remote, SIG_NOT_INPUT, 0x10, 0);
	not_remote->set_context_out(drec, SIG_DATAREC_REMOTE, 1);
	pio->set_context_port_c(drec, SIG_DATAREC_MIC, 0x20, 0);
	pio->set_context_port_c(pcm, SIG_PCM1BIT_SIGNAL, 0x80, 0);
	psg->set_context_port_b(joystick, SIG_JOYSTICK_SEL, 0x40, 0);
	vdp->set_context_irq(cpu, SIG_CPU_IRQ, 1);
#if defined(LDC_SLOT)
	pio->set_context_port_c(slot_ldc, SIG_LDC_MUTE, 0x10, 0);
	ldp->set_context_exv(slot_ldc, SIG_LDC_EXV, 1);
	ldp->set_context_ack(slot_ldc, SIG_LDC_ACK, 1);
	ldp->set_context_sound(psg, SIG_AY_3_891X_PORT_A, 0x80);
#endif
	
	joystick->set_context_psg(psg);
//	keyboard->set_context_cpu(cpu);
	keyboard->set_context_pio(pio);
//	memory->set_context_slot(0, slot0);
//	memory->set_context_slot(1, slot1);
//	memory->set_context_slot(2, slot2);
//	memory->set_context_slot(3, slot3);
	memory->set_context_slot_dummy(dummy);
	memory->set_context_slot(MAINROM_SLOT, slot_mainrom);
	memory->set_context_slot(CART1_SLOT, slot_cart[0]);
#if defined(CART2_SLOT)
	memory->set_context_slot(CART2_SLOT, slot_cart[1]);
#endif
#if defined(LDC_SLOT)
	memory->set_context_slot(LDC_SLOT, slot_ldc);
#endif
#if defined(MAPPERRAM_SLOT)
	memory->set_context_slot(MAPPERRAM_SLOT, slot_ram);
	memory->set_context_mapper(slot_ram);
#endif
#if defined(RAM64K_SLOT)
	memory->set_context_slot(RAM64K_SLOT, slot_ram);
#endif
#if defined(SUBROM_SLOT)
	memory->set_context_slot(SUBROM_SLOT, slot_subrom);
#endif
#if defined(FIRMWARE32K1_SLOT)
	slot_firmware32k1->set_context_filename(FIRMWARE32K1_FILENAME);
	memory->set_context_slot(FIRMWARE32K1_SLOT, slot_firmware32k1);
#endif
#if defined(FIRMWARE32K2_SLOT)
	slot_firmware32k2->set_context_filename(FIRMWARE32K2_FILENAME);
	memory->set_context_slot(FIRMWARE32K2_SLOT, slot_firmware32k2);
#endif
#if defined(FDD_PATCH_SLOT)
	memory->set_context_slot(FDD_PATCH_SLOT, slot_fdd_patch);
	memory->set_context_fdd_patch(slot_fdd_patch);
#endif
#if defined(MSXDOS2_SLOT)
	memory->set_context_slot(MSXDOS2_SLOT, slot_msxdos2);
#endif
#if defined(MSXMUSIC_SLOT)
	memory->set_context_slot(MSXMUSIC_SLOT, slot_msxmusic);
#endif
#if defined(_MSX_VDP_MESS) && defined(FDD_PATCH_SLOT)
	memory->set_context_vdp(vdp);
#endif

#if !defined(_MSX1_VARIANTS)
	rtcif->set_context_rtc(rtc);
#endif
#if defined(LDC_SLOT)
	slot_ldc->set_context_cpu(cpu);
	slot_ldc->set_context_ldp(ldp);
	slot_ldc->set_context_vdp(vdp);
#endif
	
#ifdef USE_PRINTER
	if(config.printer_type == 0) {  
		printer->set_context_prn(new PRNFILE(this, emu));
#if defined(_MZP)
	} else if(config.printer_type == 1) {
		MZ1P17 *mz1p17 = new MZ1P17(this, emu);
		mz1p17->mode = MZ1P17_MODE_X1;
		printer->set_context_prn(mz1p17);
	} else if(config.printer_type == 2) {
		MZ1P17 *mz1p17 = new MZ1P17(this, emu);
		mz1p17->mode = MZ1P17_MODE_MZ80P4;
		printer->set_context_prn(mz1p17);
#else
//	} else if(config.printer_type == 1) {
//		HXP560 *hxp560 = new HXP560(this, emu);
//		printer->set_context_prn(hxp560);
//	} else if(config.printer_type == 2) {
//		printer->set_context_prn(new PCPR201(this, emu));
#endif
	} else {
//		printer->set_context_prn(dummy);
		printer->set_context_prn(printer);
	}
#endif

	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(dummy);
#if defined(FDD_PATCH_SLOT)
	cpu->set_context_bios(memory);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
#if !defined(_MSX1_VARIANTS)
	io->set_iomap_range_rw(0xb4, 0xb5, rtcif);
	io->set_iomap_range_rw(0x98, 0x9b, vdp);
#else
	io->set_iomap_range_rw(0x98, 0x99, vdp);
#endif
	io->set_iomap_range_rw(0xa8, 0xab, pio);
#if defined(MSX_PSG_STEREO)
	psg_stereo->set_context_psg(psg);
	io->set_iomap_alias_w(0xa0, psg_stereo, 0);	// PSG ch
	io->set_iomap_alias_w(0xa1, psg_stereo, 1);	// PSG data
	io->set_iomap_alias_r(0xa2, psg_stereo, 1);	// PSG data
#else
	io->set_iomap_alias_w(0xa0, psg, 0);	// PSG ch
	io->set_iomap_alias_w(0xa1, psg, 1);	// PSG data
	io->set_iomap_alias_r(0xa2, psg, 1);	// PSG data
#endif
	io->set_iomap_range_rw(0xfc, 0xff, memory);
	io->set_iomap_range_rw(0xd8, 0xdb, kanjirom);
#ifdef USE_PRINTER
	io->set_iomap_range_rw(0x90, 0x91, printer);
#endif
	io->set_iomap_range_rw(0x7c, 0x7d, ym2413);
	
#ifdef USE_PORT_F4
	static PORT_F4 *port_f4;
	port_f4 = new PORT_F4(this, emu);
	io->set_iomap_range_rw(0xf4, 0xf4, port_f4);
#endif

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
	vdp->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
#if defined(MSX_PSG_STEREO)
	psg_stereo->initialize_sound(rate, 3579545, samples, 0, 0);
#else
	psg->initialize_sound(rate, 3579545, samples, 0, 0);
#endif
	pcm->initialize_sound(rate, 8000);
#if defined(LDC_SLOT)
	ldp->initialize_sound(rate, samples);
#endif
	ym2413->initialize_sound(rate, 3579545, samples);
	sound_cart[0]->initialize_sound(rate, 3579545, samples);
	sound_cart[1]->initialize_sound(rate, 3579545, samples);
}

uint16_t* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

int VM::get_sound_buffer_ptr()
{
	return event->get_sound_buffer_ptr();
}

#if defined(LDC_SLOT)
void VM::movie_sound_callback(uint8_t *buffer, long size)
{
	ldp->movie_sound_callback(buffer, size);
}
#endif

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch == 0) {
#if defined(MSX_PSG_STEREO)
	psg_stereo->set_volume(1, decibel_l, decibel_r);
#else
	psg->set_volume(1, decibel_l, decibel_r);
#endif
	} else if(ch == 1) {
		pcm->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 2) {
		drec->set_volume(0, decibel_l, decibel_r);
#if defined(_PX7)
	} else if(ch-- == 3) {
		ldp->set_volume(0, decibel_l, decibel_r);
#endif
	} else if(ch == 3) {
		sound_cart[0]->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 4) {
		sound_cart[1]->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 5) {
		ym2413->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 6) {
		drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_cart(int drv, const _TCHAR* file_path)
{
	if(drv < 2) {
		slot_cart[drv]->open_cart(file_path);
	}
	reset();
}

void VM::close_cart(int drv)
{
	if(drv < 2) {
		slot_cart[drv]->close_cart();
	}
	reset();
}

bool VM::is_cart_inserted(int drv)
{
	if(drv < 2) {
		return slot_cart[drv]->is_cart_inserted();
	} else {
		return false;
	}
}

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	bool remote = drec->get_remote();
	
	if(drec->play_tape(file_path) && remote) {
		// if machine already sets remote on, start playing now
		push_play(drv);
	}
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	bool remote = drec->get_remote();
	
	if(drec->rec_tape(file_path) && remote) {
		// if machine already sets remote on, start recording now
		push_play(drv);
	}
}

void VM::close_tape(int drv)
{
	emu->lock_vm();
	drec->close_tape();
	emu->unlock_vm();
	drec->set_remote(false);
}

bool VM::is_tape_inserted(int drv)
{
	return drec->is_tape_inserted();
}

bool VM::is_tape_playing(int drv)
{
	return drec->is_tape_playing();
}

bool VM::is_tape_recording(int drv)
{
	return drec->is_tape_recording();
}

int VM::get_tape_position(int drv)
{
	return drec->get_tape_position();
}

const _TCHAR* VM::get_tape_message(int drv)
{
	return drec->get_message();
}

void VM::push_play(int drv)
{
	drec->set_remote(false);
	drec->set_ff_rew(0);
	drec->set_remote(true);
}

void VM::push_stop(int drv)
{
	drec->set_remote(false);
}

void VM::push_fast_forward(int drv)
{
	drec->set_remote(false);
	drec->set_ff_rew(1);
	drec->set_remote(true);
}

void VM::push_fast_rewind(int drv)
{
	drec->set_remote(false);
	drec->set_ff_rew(-1);
	drec->set_remote(true);
}

#if defined(LDC_SLOT)
void VM::open_laser_disc(int drv, const _TCHAR* file_path)
{
	ldp->open_disc(file_path);
}

void VM::close_laser_disc(int drv)
{
	ldp->close_disc();
}

bool VM::is_laser_disc_inserted(int drv)
{
	return ldp->is_disc_inserted();
}

uint32_t VM::is_laser_disc_accessed()
{
	return ldp->read_signal(0);
}
#endif

#if defined(FDD_PATCH_SLOT)
void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	memory->open_disk(drv, file_path, bank);
}

void VM::close_floppy_disk(int drv)
{
	memory->close_disk(drv);
}

bool VM::is_floppy_disk_inserted(int drv)
{
	return memory->is_disk_inserted(drv);
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
	memory->is_disk_protected(drv, value);
}

bool VM::is_floppy_disk_protected(int drv)
{
	return memory->is_disk_protected(drv);
}

uint32_t VM::is_floppy_disk_accessed()
{
	return memory->read_signal(0);
}
#endif

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

#define STATE_VERSION	6

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

