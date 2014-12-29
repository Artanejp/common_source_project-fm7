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
#include "../ls393.h"
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
#include "io.h"
#include "iotrap.h"
#include "keyboard.h"
#include "memory.h"
#include "pac2.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	crtc = new HD46505(this, emu);
	pio0 = new I8255(this, emu);
	pio1 = new I8255(this, emu);
	pio2 = new I8255(this, emu);
	flipflop = new LS393(this, emu); // LS74
	not = new NOT(this, emu);
	pcm = new PCM1BIT(this, emu);
	psg0 = new SN76489AN(this, emu);
	psg1 = new SN76489AN(this, emu);
	fdc = new UPD765A(this, emu);
	cpu = new Z80(this, emu);
	ctc = new Z80CTC(this, emu);
	pio = new Z80PIO(this, emu);
	
	floppy = new FLOPPY(this, emu);
	display = new DISPLAY(this, emu);
	io = new IO(this, emu);
	iotrap = new IOTRAP(this, emu);
	key = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	pac2 = new PAC2(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	event->set_context_sound(psg0);
	event->set_context_sound(psg1);
	
	drec->set_context_out(pio2, SIG_I8255_PORT_B, 0x20);
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
	pio2->set_context_port_a(drec, SIG_DATAREC_OUT, 0x10, 0);
	pio2->set_context_port_a(not, SIG_NOT_INPUT, 0x20, 0);
	pio2->set_context_port_a(iotrap, SIG_IOTRAP_I8255_2_A, 0xff, 0);
	pio2->set_context_port_c(iotrap, SIG_IOTRAP_I8255_2_C, 0xff, 0);
	flipflop->set_context_1qa(pcm, SIG_PCM1BIT_SIGNAL, 1);
	not->set_context_out(drec, SIG_DATAREC_REMOTE, 1);
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
	io->set_ram_ptr(memory->get_ram());
	iotrap->set_context_cpu(cpu);
	iotrap->set_context_pio2(pio2);
	key->set_context_pio(pio);
	memory->set_context_io(io);
	memory->set_context_pio0(pio0);
	memory->set_context_pio2(pio2);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
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
		if(device->this_device_id != event->this_device_id) {
			device->reset();
		}
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

double VM::frame_rate()
{
	return event->frame_rate();
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

int VM::access_lamp()
{
	uint32 status = fdc->read_signal(0);
	return (status & (1 | 4)) ? 1 : (status & (2 | 8)) ? 2 : 0;
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	
	// init sound gen
	pcm->init(rate, 3600);
	psg0->init(rate, 1996800, 3600);
	psg1->init(rate, 1996800, 3600);
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

void VM::play_tape(_TCHAR* file_path)
{
	drec->play_tape(file_path);
}

void VM::rec_tape(_TCHAR* file_path)
{
	drec->rec_tape(file_path);
}

void VM::close_tape()
{
	drec->close_tape();
}

bool VM::tape_inserted()
{
	return drec->tape_inserted();
}

void VM::load_binary(int drv, _TCHAR* file_path)
{
	pac2->open_rampac2(drv, file_path);
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

