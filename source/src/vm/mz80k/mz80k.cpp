/*
	SHARP MZ-80K Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	SHARP MZ-80A Emulator 'EmuZ-80A'
	Modify : Hideki Suga
	Date   : 2014.12.10 -

	[ virtual machine ]
*/

#include "mz80k.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#if defined(_MZ1200) || defined(_MZ80A)
#include "../and.h"
#endif
#include "../datarec.h"
#include "../i8253.h"
#include "../i8255.h"
#include "../ls393.h"
#include "../pcm1bit.h"
#include "../z80.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
#include "keyboard.h"
#include "memory.h"

#ifdef SUPPORT_MZ80AIF
#include "../io.h"
#include "../disk.h"
#include "../mb8877.h"
#include "floppy.h"
#endif

#include "../../fileio.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
#if defined(_MZ1200) || defined(_MZ80A)
	and = new AND(this, emu);
#endif
	drec = new DATAREC(this, emu);
	ctc = new I8253(this, emu);
	pio = new I8255(this, emu);
	counter = new LS393(this, emu);
	pcm = new PCM1BIT(this, emu);
	cpu = new Z80(this, emu);
	
	display = new DISPLAY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	
#ifdef SUPPORT_MZ80AIF
	io = new IO(this, emu);
	fdc = new MB8877(this, emu);	// mb8866
	floppy = new FLOPPY(this, emu);
#endif
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	
#if defined(_MZ1200) || defined(_MZ80A)
	and->set_context_out(cpu, SIG_CPU_IRQ, 1);
	and->set_mask(SIG_AND_BIT_0 | SIG_AND_BIT_1);
#endif
	drec->set_context_out(pio, SIG_I8255_PORT_C, 0x20);
	drec->set_context_remote(pio, SIG_I8255_PORT_C, 0x10);
	ctc->set_context_ch0(counter, SIG_LS393_CLK, 1);
	ctc->set_context_ch1(ctc, SIG_I8253_CLOCK_2, 1);
#if defined(_MZ1200) || defined(_MZ80A)
	ctc->set_context_ch2(and, SIG_AND_BIT_0, 1);
#else
	ctc->set_context_ch2(cpu, SIG_CPU_IRQ, 1);
#endif
	ctc->set_constant_clock(0, 2000000);
	ctc->set_constant_clock(1, 31250);
	pio->set_context_port_a(keyboard, SIG_KEYBOARD_COLUMN, 0x0f, 0);
	pio->set_context_port_c(display, SIG_DISPLAY_VGATE, 1, 0);
	pio->set_context_port_c(drec, SIG_DATAREC_OUT, 2, 0);
#if defined(_MZ1200) || defined(_MZ80A)
	pio->set_context_port_c(and, SIG_AND_BIT_1, 4, 0);
#endif
	pio->set_context_port_c(drec, SIG_DATAREC_TRIG, 8, 0);
	counter->set_context_1qa(pcm, SIG_PCM1BIT_SIGNAL, 1);
	
	display->set_vram_ptr(memory->get_vram());
#if defined(_MZ80A)
	display->set_e200_ptr(memory->get_e200());
#endif
	keyboard->set_context_pio(pio);
	memory->set_context_ctc(ctc);
	memory->set_context_pio(pio);
#if defined(_MZ1200) || defined(_MZ80A)
	memory->set_context_disp(display);
#endif
	
#ifdef SUPPORT_MZ80AIF
	fdc->set_context_irq(memory, SIG_MEMORY_FDC_IRQ, 1);
	fdc->set_context_drq(memory, SIG_MEMORY_FDC_DRQ, 1);
	floppy->set_context_fdc(fdc);
#endif
	
	// cpu bus
	cpu->set_context_mem(memory);
#ifdef SUPPORT_MZ80AIF
	cpu->set_context_io(io);
#else
	cpu->set_context_io(dummy);
#endif
	cpu->set_context_intr(dummy);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
#ifdef SUPPORT_MZ80AIF
	// i/o bus
	io->set_iomap_range_rw(0xd8, 0xdb, fdc);
	io->set_iomap_range_w(0xdc, 0xdd, floppy);
#endif
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
#ifdef SUPPORT_MZ80AIF
	for(int i = 0; i < MAX_DRIVE; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2DD);
	}
#endif
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
#if defined(_MZ1200) || defined(_MZ80A)
	and->write_signal(SIG_AND_BIT_0, 0, 1);	// CLOCK = L
	and->write_signal(SIG_AND_BIT_1, 1, 1);	// INTMASK = H
#endif
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
	display->draw_screen();
}

#ifdef SUPPORT_MZ80AIF
int VM::access_lamp()
{
	uint32 status = fdc->read_signal(0);
	return (status & (1 | 4)) ? 1 : (status & (2 | 8)) ? 2 : 0;
}
#endif

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	pcm->init(rate, 8000);
}

uint16* VM::create_sound(int* extra_frames)
{
	return event->create_sound(extra_frames);
}

int VM::sound_buffer_ptr()
{
	return event->sound_buffer_ptr();
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

#ifdef SUPPORT_MZ80AIF
void VM::open_disk(int drv, _TCHAR* file_path, int offset)
{
	fdc->open_disk(drv, file_path, offset);
}

void VM::close_disk(int drv)
{
	fdc->close_disk(drv);
}

bool VM::disk_inserted(int drv)
{
	return fdc->disk_inserted(drv);
}
#endif

void VM::play_tape(_TCHAR* file_path)
{
	drec->play_tape(file_path);
	drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
}

void VM::rec_tape(_TCHAR* file_path)
{
	drec->rec_tape(file_path);
	drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
}

void VM::close_tape()
{
	drec->close_tape();
	drec->write_signal(SIG_DATAREC_REMOTE, 0, 0);
}

bool VM::tape_inserted()
{
	return drec->tape_inserted();
}

void VM::push_play()
{
	drec->write_signal(SIG_DATAREC_REMOTE, 1, 1);
}

void VM::push_stop()
{
	drec->write_signal(SIG_DATAREC_REMOTE, 0, 0);
}

bool VM::now_skip()
{
	return event->now_skip();
}

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
}

#define STATE_VERSION	2

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

