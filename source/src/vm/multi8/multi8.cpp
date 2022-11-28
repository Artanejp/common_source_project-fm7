/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.15 -

	[ virtual machine ]
*/

#include "multi8.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../beep.h"
#include "../disk.h"
#include "../hd46505.h"
#include "../i8251.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../i8259.h"
#include "../io.h"
#include "../noise.h"
#include "../upd765a.h"
//#include "../ym2203.h"
#include "../ay_3_891x.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "cmt.h"
#include "display.h"
#include "floppy.h"
#include "kanji.h"
#include "keyboard.h"
#include "memory.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	beep = new BEEP(this, emu);
	crtc = new HD46505(this, emu);
	sio = new I8251(this, emu);
	pit = new I8253(this, emu);
	pio = new I8255(this, emu);
	pic = new I8259(this, emu);
	io = new IO(this, emu);
	fdc = new UPD765A(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	psg = new AY_3_891X(this, emu);
#ifdef USE_DEBUGGER
	psg->set_context_debugger(new DEBUGGER(this, emu));
#endif
	cpu = new Z80(this, emu);
	
	cmt = new CMT(this, emu);
	display = new DISPLAY(this, emu);
	floppy = new FLOPPY(this, emu);
	kanji = new KANJI(this, emu);
	key = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(beep);
	event->set_context_sound(psg);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	
	crtc->set_context_vsync(pio, SIG_I8255_PORT_A, 0x20);
	sio->set_context_out(cmt, SIG_CMT_OUT);
	pit->set_context_ch1(pit, SIG_I8253_CLOCK_2, 1);
	pit->set_context_ch1(pic, SIG_I8259_CHIP0 | SIG_I8259_IR5, 1);
	pit->set_context_ch2(pic, SIG_I8259_CHIP0 | SIG_I8259_IR6, 1);
	pit->set_constant_clock(0, CPU_CLOCKS >> 1);
	pit->set_constant_clock(1, CPU_CLOCKS >> 1);
	pio->set_context_port_b(display, SIG_DISPLAY_I8255_B, 0xff, 0);
	pio->set_context_port_c(memory, SIG_MEMORY_I8255_C, 0xff, 0);
	pic->set_context_cpu(cpu);
	fdc->set_context_irq(pic, SIG_I8259_CHIP0 | SIG_I8259_IR0, 1);
	fdc->set_context_drq(floppy, SIG_FLOPPY_DRQ, 1);
	psg->set_context_port_a(cmt, SIG_CMT_REMOTE, 2, 0);
	psg->set_context_port_a(pio, SIG_I8255_PORT_A, 2, 1);
	psg->set_context_port_a(beep, SIG_BEEP_ON, 8, 1);
	
	cmt->set_context_sio(sio);
	display->set_vram_ptr(memory->get_vram());
	display->set_regs_ptr(crtc->get_regs());
	floppy->set_context_fdc(fdc);
	kanji->set_context_pio(pio);
	memory->set_context_pio(pio);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(pic);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_range_rw(0x00, 0x01, key);
	io->set_iomap_range_rw(0x18, 0x19, psg);
	io->set_iomap_range_rw(0x1c, 0x1d, crtc);
	io->set_iomap_range_rw(0x20, 0x21, sio);
	io->set_iomap_range_rw(0x24, 0x27, pit);
	io->set_iomap_range_rw(0x28, 0x2b, pio);
	io->set_iomap_alias_rw(0x2c, pic, I8259_ADDR_CHIP0 | 0);
	io->set_iomap_alias_rw(0x2d, pic, I8259_ADDR_CHIP0 | 1);
	io->set_iomap_range_rw(0x30, 0x37, display);
	io->set_iomap_range_rw(0x40, 0x41, kanji);
	io->set_iomap_range_rw(0x70, 0x71, fdc);
	io->set_iomap_range_rw(0x72, 0x74, floppy);
	io->set_iomap_single_w(0x78, memory);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(int i = 0; i < 4; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2D);
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
	beep->initialize_sound(rate, 2400, 8000);
	psg->initialize_sound(rate, 3579545, samples, 0, 0);
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
		psg->set_volume(1, decibel_l, decibel_r);
	} else if(ch == 1) {
		beep->set_volume(0, decibel_l, decibel_r);
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

bool VM::get_caps_locked()
{
	return key->get_caps_locked();
}

bool VM::get_kana_locked()
{
	return key->get_kana_locked();
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

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	cmt->play_tape(file_path);
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	cmt->rec_tape(file_path);
}

void VM::close_tape(int drv)
{
	cmt->close_tape();
}

bool VM::is_tape_inserted(int drv)
{
	return cmt->is_tape_inserted();
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

