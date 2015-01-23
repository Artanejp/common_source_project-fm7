/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'

	Author : Takeda.Toshiya
	Date   : 2006.12.28 -

	[ virtual machine ]
*/

#include "pasopia.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../disk.h"
#include "../hd46505.h"
#include "../i8255.h"
#include "../io.h"
#include "../ls393.h"
#include "../not.h"
#include "../pcm1bit.h"
#include "../upd765a.h"
#include "../z80.h"
#include "../z80ctc.h"
#include "../z80pio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "floppy.h"
#include "display.h"
#include "keyboard.h"
#include "memory.h"
#include "pac2.h"

#include "../../fileio.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	boot_mode = config.boot_mode;
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	crtc = new HD46505(this, emu);
	pio0 = new I8255(this, emu);
	pio1 = new I8255(this, emu);
	pio2 = new I8255(this, emu);
	io = new IO(this, emu);
	flipflop = new LS393(this, emu); // LS74
	not = new NOT(this, emu);
	pcm = new PCM1BIT(this, emu);
	fdc = new UPD765A(this, emu);
	cpu = new Z80(this, emu);
	ctc = new Z80CTC(this, emu);
	pio = new Z80PIO(this, emu);
	
	floppy = new FLOPPY(this, emu);
	display = new DISPLAY(this, emu);
	key = new KEYBOARD(this, emu);
	memory = new MEMORY(this, emu);
	pac2 = new PAC2(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm);
	
	drec->set_context_out(pio2, SIG_I8255_PORT_B, 0x20);
	crtc->set_context_disp(pio1, SIG_I8255_PORT_B, 8);
	crtc->set_context_vsync(pio1, SIG_I8255_PORT_B, 0x20);
	crtc->set_context_hsync(pio1, SIG_I8255_PORT_B, 0x40);
	pio0->set_context_port_a(memory, SIG_MEMORY_I8255_0_A, 0xff, 0);
	pio0->set_context_port_b(memory, SIG_MEMORY_I8255_0_B, 0xff, 0);
	pio1->set_context_port_a(display, SIG_DISPLAY_I8255_1_A, 0xff, 0);
	pio1->set_context_port_c(memory, SIG_MEMORY_I8255_1_C, 0xff, 0);
	pio2->set_context_port_a(pcm, SIG_PCM1BIT_MUTE, 0x02, 0);
	pio2->set_context_port_a(drec, SIG_DATAREC_OUT, 0x10, 0);
	pio2->set_context_port_a(not, SIG_NOT_INPUT, 0x20, 0);
	flipflop->set_context_1qa(pcm, SIG_PCM1BIT_SIGNAL, 1);
	not->set_context_out(drec, SIG_DATAREC_REMOTE, 1);
	fdc->set_context_irq(floppy, SIG_FLOPPY_INTR, 1);
	ctc->set_context_zc1(flipflop, SIG_LS393_CLK, 1);
	ctc->set_context_zc2(ctc, SIG_Z80CTC_TRIG_3, 1);
	ctc->set_constant_clock(0, CPU_CLOCKS);
	ctc->set_constant_clock(1, CPU_CLOCKS);
	ctc->set_constant_clock(2, CPU_CLOCKS);
	pio->set_context_port_a(pcm, SIG_PCM1BIT_ON, 0x80, 0);
	pio->set_context_port_a(key, SIG_KEYBOARD_Z80PIO_A, 0xff, 0);
	
	display->set_context_crtc(crtc);
	display->set_vram_ptr(memory->get_vram());
	display->set_attr_ptr(memory->get_attr());
	display->set_regs_ptr(crtc->get_regs());
	floppy->set_context_fdc(fdc);
	floppy->supported = (boot_mode != MODE_OABASIC_NO_DISK);
	key->set_context_pio(pio);
	memory->set_context_pio0(pio0);
	memory->set_context_pio1(pio1);
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
/*
pio0	0	8255	vram laddr
	1	8255	vram data
	2	8255
	3	8255
pio1	8	8255	crtc mode
	9	8255	
	a	8255	vram addr, attrib & strobe
	b	8255
	10	crtc	index
	11	crtc	regs
	18	pac2
	19	pac2
	1a	pac2
	1b	pac2
pio2	20	8255	out cmt, sound
	21	8255	in cmt
	22	8255	in memory map
	23	8255
	28	z80ctc
	29	z80ctc
	2a	z80ctc
	2b	z80ctc
	30	z80pio
	31	z80pio
	32	z80pio
	33	z80pio
	38	printer
	3c	memory	out memory map
*/
	// i/o bus
	io->set_iomap_range_rw(0x00, 0x03, pio0);
	io->set_iomap_range_rw(0x08, 0x0b, pio1);
	io->set_iomap_range_r(0x10, 0x11, crtc);
	io->set_iomap_range_w(0x10, 0x11, display);
	io->set_iomap_range_rw(0x18, 0x1b, pac2);
	io->set_iomap_range_rw(0x20, 0x23, pio2);
	io->set_iomap_range_rw(0x28, 0x2b, ctc);
	io->set_iomap_alias_rw(0x30, pio, 0);
	io->set_iomap_alias_rw(0x31, pio, 2);
	io->set_iomap_alias_w(0x32, pio, 1);
	io->set_iomap_alias_w(0x33, pio, 3);
	io->set_iomap_single_w(0x3c, memory);
	io->set_iomap_single_w(0xe0, floppy);
	io->set_iomap_single_w(0xe2, floppy);
	io->set_iomap_range_rw(0xe4, 0xe5, floppy);
	io->set_iomap_single_rw(0xe6, floppy);
	
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
	
	// set initial port status
#ifdef _LCD
	pio1->write_signal(SIG_I8255_PORT_B, 0, 0x10);
#else
	pio1->write_signal(SIG_I8255_PORT_B, 0xffffffff, 0x10);
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
	if(drv == 0) {
		pac2->open_rampac2(file_path);
	}
}

bool VM::now_skip()
{
	return event->now_skip();
}

void VM::update_config()
{
	if(boot_mode != config.boot_mode) {
		// boot mode is changed !!!
		boot_mode = config.boot_mode;
		memory->load_ipl();
		floppy->supported = (boot_mode != MODE_OABASIC_NO_DISK);
		// reset virtual machine
		reset();
	} else {
		for(DEVICE* device = first_device; device; device = device->next_device) {
			device->update_config();
		}
	}
}

#define STATE_VERSION	1

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
	state_fio->FputInt32(boot_mode);
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
	boot_mode = state_fio->FgetInt32();
	return true;
}

