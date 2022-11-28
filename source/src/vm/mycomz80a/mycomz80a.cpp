/*
	Japan Electronics College MYCOMZ-80A Emulator 'eMYCOMZ-80A'

	Author : Takeda.Toshiya
	Date   : 2009.05.13-

	[ virtual machine ]
*/

#include "mycomz80a.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../hd46505.h"
#include "../i8255.h"
#include "../io.h"
#include "../msm58321.h"
#include "../noise.h"
#include "../sn76489an.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
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
	
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
	crtc = new HD46505(this, emu);
	pio1 = new I8255(this, emu);
	pio1->set_device_name(_T("8255 PIO (VRAM/Keyboard)"));
	pio2 = new I8255(this, emu);
	pio2->set_device_name(_T("8255 PIO (Display/Keyboard/CMT)"));
	pio3 = new I8255(this, emu);
	pio3->set_device_name(_T("8255 PIO (FDC/RTC)"));
	io = new IO(this, emu);
	rtc = new MSM58321(this, emu);	// MSM5832
	psg = new SN76489AN(this, emu);
	cpu = new Z80(this, emu);
	
	display = new DISPLAY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(psg);
	event->set_context_sound(drec);
	event->set_context_sound(drec->get_context_noise_play());
	event->set_context_sound(drec->get_context_noise_stop());
	event->set_context_sound(drec->get_context_noise_fast());
	
	//	00	out	system control
	//	01	in/out	vram data
	//	02	out	crtc addr
	//	03	in/out	crtc data
	//	04-07	in/ou	pio1
	//		pa0-7	out	vram addr low, psg data
	//		pb0-7	in	keyboard data
	//		pc0-2	out	vram addr high
	//		pc3	out	fdc format write
	//		pc4	in	keyboard s2
	//		pc5	in	keyboard s3
	//		pc6	in	keyboard s4 (motor on/off)
	//		pc7	in	keyboard s5 (slow)
	//	08-0b	in/ou	pio2
	//		pa0	in	keyboard strobe (1=pressed)
	//		pa1	in	keyboard shift (1=pressed)
	//		pa2	in	cmt in
	//		pa3-6	in	printer ctrl
	//		pa7	in	crtc disptmg
	//		pb0-7	out	printer data
	//		pc0	out	printer strobe
	//		pc1	out	printer reset
	//		pc2	out	cmt out
	//		pc3	out	cmt remote
	//		pc4	out	psg we
	//		pc5	out	psg cs
	//		pc6	out	display chr/cg (0=chr,1=cg)
	//		pc7	out	display freq (0=80column,1=40column)
	//	0c-0f	in/ou	pio3
	//		pa0-6	in	fdc control
	//		pb0-3	in/out	rtc data
	//		pc0-3	out	rtc address
	//		pc4	out	rtc hold
	//		pc5	out	rtc rd
	//		pc6	out	rtc wr
	//		pc7	out	rtc cs
	drec->set_context_ear(pio2, SIG_I8255_PORT_A, 4);
	crtc->set_context_disp(pio2, SIG_I8255_PORT_A, 0x80);
	pio1->set_context_port_a(display, SIG_DISPLAY_ADDR_L, 0xff, 0);
	pio1->set_context_port_a(psg, SIG_SN76489AN_DATA, 0xff, 0);
	pio1->set_context_port_c(display, SIG_DISPLAY_ADDR_H, 7, 0);
	pio2->set_context_port_c(drec, SIG_DATAREC_MIC, 4, 0);
	pio2->set_context_port_c(drec, SIG_DATAREC_REMOTE, 8, 0);
	pio2->set_context_port_c(psg, SIG_SN76489AN_WE, 0x10, 0);
	pio2->set_context_port_c(psg, SIG_SN76489AN_CS, 0x20, 0);
	pio2->set_context_port_c(display, SIG_DISPLAY_MODE, 0xc0, 0);
	pio3->set_context_port_b(rtc, SIG_MSM58321_DATA, 0x0f, 0);
	pio3->set_context_port_c(rtc, SIG_MSM5832_ADDR, 0x0f, 0);
	pio3->set_context_port_c(rtc, SIG_MSM5832_HOLD, 0x10, 0);
	pio3->set_context_port_c(rtc, SIG_MSM58321_READ, 0x20, 0);
	pio3->set_context_port_c(rtc, SIG_MSM58321_WRITE, 0x40, 0);
	pio3->set_context_port_c(rtc, SIG_MSM58321_CS, 0x80, 0);
	rtc->set_context_data(pio3, SIG_I8255_PORT_B, 0x0f, 0);
	
	display->set_context_crtc(crtc);
	display->set_regs_ptr(crtc->get_regs());
	keyboard->set_context_cpu(cpu);
	keyboard->set_context_pio1(pio1);
	keyboard->set_context_pio2(pio2);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// i/o bus
	io->set_iomap_single_w(0x00, memory);
	io->set_iomap_single_rw(0x01, display);
	io->set_iomap_range_rw(0x02, 0x03, crtc);
	io->set_iomap_range_rw(0x04, 0x07, pio1);
	io->set_iomap_range_rw(0x08, 0x0b, pio2);
	io->set_iomap_range_rw(0x0c, 0x0f, pio3);
	
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
	psg->initialize_sound(rate, 2500800, 10000);
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
	} else if(ch == 1) {
		drec->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 2) {
		drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
		drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
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

bool VM::get_caps_locked()
{
	return keyboard->get_caps_locked();
}

bool VM::get_kana_locked()
{
	return keyboard->get_kana_locked();
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

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

