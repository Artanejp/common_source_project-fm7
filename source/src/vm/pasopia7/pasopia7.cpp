/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ virtual machine ]
*/

#include "pasopia7.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../disk.h"
#include "../hd46505.h"
#include "../i8255.h"
#include "../io.h"
#include "../ls393.h"
#include "../noise.h"
#include "../not.h"
#include "../pcm1bit.h"
#include "../sn76489an.h"
#include "../upd765a.h"
#include "../z80.h"
#include "../z80ctc.h"
#include "../z80pio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "floppy.h"
#include "display.h"
#include "iobus.h"
#include "iotrap.h"
#include "keyboard.h"
#include "memory.h"
#include "pac2.h"

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
	crtc = new HD46505(this, emu);
	pio0 = new I8255(this, emu);
	pio0->set_device_name(_T("8255 PIO (Display)"));
	pio1 = new I8255(this, emu);
	pio1->set_device_name(_T("8255 PIO (Display/Memory)"));
	pio2 = new I8255(this, emu);
	pio2->set_device_name(_T("8255 PIO (Sound/Data Recorder)"));
	io = new IO(this, emu);
	io->space = 0x100;
	flipflop = new LS393(this, emu); // LS74
	not_remote = new NOT(this, emu);
	pcm = new PCM1BIT(this, emu);
	psg0 = new SN76489AN(this, emu);
	psg0->set_device_name(_T("SN76489AN PSG #1"));
	psg1 = new SN76489AN(this, emu);
	psg1->set_device_name(_T("SN76489AN PSG #2"));
	fdc = new UPD765A(this, emu);
	fdc->set_context_noise_seek(new NOISE(this, emu));
	fdc->set_context_noise_head_down(new NOISE(this, emu));
	fdc->set_context_noise_head_up(new NOISE(this, emu));
	cpu = new Z80(this, emu);
	ctc = new Z80CTC(this, emu);
	pio = new Z80PIO(this, emu);
	
	floppy = new FLOPPY(this, emu);
	display = new DISPLAY(this, emu);
	iobus = new IOBUS(this, emu);
	iotrap = new IOTRAP(this, emu);
	key = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	pac2 = new PAC2(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	event->set_context_sound(psg0);
	event->set_context_sound(psg1);
	event->set_context_sound(drec);
	event->set_context_sound(fdc->get_context_noise_seek());
	event->set_context_sound(fdc->get_context_noise_head_down());
	event->set_context_sound(fdc->get_context_noise_head_up());
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	drec->set_context_ear(pio2, SIG_I8255_PORT_B, 0x20);
	crtc->set_context_disp(pio0, SIG_I8255_PORT_B, 8);
	crtc->set_context_vsync(pio0, SIG_I8255_PORT_B, 0x20);
	pio0->set_context_port_a(display, SIG_DISPLAY_I8255_0_A, 0xff, 0);
	pio1->set_context_port_a(memory, SIG_MEMORY_I8255_1_A, 0xff, 0);
	pio1->set_context_port_b(display, SIG_DISPLAY_I8255_1_B, 0xff, 0);
	pio1->set_context_port_b(memory, SIG_MEMORY_I8255_1_B, 0xff, 0);
	pio1->set_context_port_c(display, SIG_DISPLAY_I8255_1_C, 0xff, 0);
	pio1->set_context_port_c(memory, SIG_MEMORY_I8255_1_C, 0xff, 0);
	pio2->set_context_port_a(pcm, SIG_PCM1BIT_MUTE, 0x02, 0);
	pio2->set_context_port_a(psg0, SIG_SN76489AN_MUTE, 0x02, 0);
	pio2->set_context_port_a(psg1, SIG_SN76489AN_MUTE, 0x02, 0);
	pio2->set_context_port_a(drec, SIG_DATAREC_MIC, 0x10, 0);
	pio2->set_context_port_a(not_remote, SIG_NOT_INPUT, 0x20, 0);
	pio2->set_context_port_a(iotrap, SIG_IOTRAP_I8255_2_A, 0xff, 0);
	pio2->set_context_port_c(iotrap, SIG_IOTRAP_I8255_2_C, 0xff, 0);
	flipflop->set_context_1qa(pcm, SIG_PCM1BIT_SIGNAL, 1);
	not_remote->set_context_out(drec, SIG_DATAREC_REMOTE, 1);
	fdc->set_context_irq(floppy, SIG_FLOPPY_INTR, 1);
	ctc->set_context_zc0(ctc, SIG_Z80CTC_TRIG_1, 1);
	ctc->set_context_zc1(flipflop, SIG_LS393_CLK, 1);
	ctc->set_context_zc2(ctc, SIG_Z80CTC_TRIG_3, 1);
	ctc->set_constant_clock(0, CPU_CLOCKS);
	ctc->set_constant_clock(2, CPU_CLOCKS);
	pio->set_context_port_a(pcm, SIG_PCM1BIT_ON, 0x80, 0);
	pio->set_context_port_a(key, SIG_KEYBOARD_Z80PIO_A, 0xff, 0);
	
	display->set_vram_ptr(memory->get_vram());
	display->set_pal_ptr(memory->get_pal());
	display->set_regs_ptr(crtc->get_regs());
	floppy->set_context_fdc(fdc);
	iobus->set_context_io(io);
	iobus->set_ram_ptr(memory->get_ram());
	iotrap->set_context_cpu(cpu);
	iotrap->set_context_pio2(pio2);
	key->set_context_pio(pio);
	memory->set_context_iobus(iobus);
	memory->set_context_pio0(pio0);
	memory->set_context_pio2(pio2);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(iobus);
	cpu->set_context_intr(ctc);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// z80 family daisy chain
	ctc->set_context_intr(cpu, 0);
	ctc->set_context_child(pio);
	pio->set_context_intr(cpu, 1);
	
	// i/o bus
	io->set_iomap_range_rw(0x08, 0x0b, pio0);
	io->set_iomap_range_rw(0x0c, 0x0f, pio1);
	io->set_iomap_range_rw(0x10, 0x11, crtc);
	io->set_iomap_range_rw(0x18, 0x1b, pac2);
	io->set_iomap_range_rw(0x20, 0x23, pio2);
	io->set_iomap_range_rw(0x28, 0x2b, ctc);
	io->set_iomap_alias_rw(0x30, pio, 0);
	io->set_iomap_alias_rw(0x31, pio, 2);
	io->set_iomap_alias_w(0x32, pio, 1);
	io->set_iomap_alias_w(0x33, pio, 3);
	io->set_iomap_single_w(0x3a, psg0);
	io->set_iomap_single_w(0x3b, psg1);
	io->set_iomap_single_w(0x3c, memory);
	io->set_iomap_single_w(0xe0, floppy);
	io->set_iomap_single_w(0xe2, floppy);
	io->set_iomap_range_rw(0xe4, 0xe5, fdc);
	io->set_iomap_single_rw(0xe6, floppy);
	
	// initialize and reset all devices except the event manager
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	for(DEVICE* device = first_device; device; device = device->next_device) {
//		if(device->this_device_id != event->this_device_id) {
			device->reset();
//		}
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
//	for(DEVICE* device = first_device; device; device = device->next_device) {
//		device->reset();
//	}
	event->reset();
	memory->reset();
	iotrap->do_reset();
	
	// set initial port status
#ifdef _LCD
	pio0->write_signal(SIG_I8255_PORT_B, 0, (0x10 | 0x40));
#else
	pio0->write_signal(SIG_I8255_PORT_B, 0x10, (0x10 | 0x40));
#endif
	pcm->write_signal(SIG_PCM1BIT_ON, 0, 1);
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
	pcm->initialize_sound(rate, 3600);
	psg0->initialize_sound(rate, 1996800, 3600);
	psg1->initialize_sound(rate, 1996800, 3600);
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
		psg0->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		psg1->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 2) {
		pcm->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 3) {
		drec->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 4) {
		fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
		fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 5) {
		drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

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

void VM::load_binary(int drv, const _TCHAR* file_path)
{
	pac2->open_rampac2(drv, file_path);
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

