/*
	EPS TRN Junior Emulator 'eTRNJunior'

	Author : Takeda.Toshiya
	Date   : 2022.07.02-

	[ virtual machine ]
*/

#include "trnjr.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../i8255.h"
#include "../io.h"
#include "../midi.h"
#include "../speaker.h"
#include "../tmpz84c015.h"
#include "../z80.h"
#include "../z80ctc.h"
#include "../z80sio.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
#include "membus.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : VM_TEMPLATE(parent_emu)
{
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	pio1 = new I8255(this, emu);
	pio1->set_device_name(_T("8255 PIO (7-Seg/Keyboard)"));
	pio2 = new I8255(this, emu);
	pio2->set_device_name(_T("8255 PIO (ROM Writer)"));
	io = new IO(this, emu);
	midi = new MIDI(this, emu);
	speaker = new SPEAKER(this, emu);
	// TMPZ84C013
	cpudev = new TMPZ84C015(this, emu);
	cpudev->set_context_ctc(new Z80CTC(this, emu));
	cpudev->set_context_sio(new Z80SIO(this, emu));
	cpu = new Z80(this, emu);
	
	display = new DISPLAY(this, emu);
	memory = new MEMBUS(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(speaker);
	
	cpudev->set_context_ctc_zc1(cpudev, SIG_TMPZ84C015_SIO_TX_CLK_CH0, 1);
	cpudev->set_context_ctc_zc1(cpudev, SIG_TMPZ84C015_SIO_RX_CLK_CH0, 1);
	cpudev->set_context_ctc_zc2(cpudev, SIG_TMPZ84C015_SIO_TX_CLK_CH1, 1);
	cpudev->set_context_ctc_zc2(cpudev, SIG_TMPZ84C015_SIO_RX_CLK_CH1, 1);
	cpudev->set_context_ctc_zc3(cpu, SIG_CPU_NMI, 1);
	cpudev->set_context_sio_send(1, midi, SIG_MIDI_OUT);
	pio1->set_context_port_a(display, SIG_DISPLAY_PORT_A, 0xff, 0);
	pio1->set_context_port_b(speaker, SIG_SPEAKER_SAMPLE, 0xff, 0);
	pio1->set_context_port_c(display, SIG_DISPLAY_PORT_C, 0xf0, 0);
	midi->set_context_in(cpudev, SIG_TMPZ84C015_SIO_RECV_CH1, 0xff);
	
	display->set_context_pio(pio1);
	memory->set_context_cpudev(cpudev);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(cpudev);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// z80 family daisy chain
	cpudev->set_context_intr(cpu, 0);
	
	// memory bus
	memset(ram, 0x00, sizeof(ram));
	memset(rom, 0xff, sizeof(rom));
	
	memory->read_bios(_T("MON.ROM"), rom, sizeof(rom));
	
	memory->set_memory_r (0x0000, 0x7fff, rom);
	memory->set_memory_rw(0x8000, 0xffff, ram);
	
	// i/o bus
	cpudev->set_iomap(io);
	io->set_iomap_range_rw(0x60, 0x63, pio1);
	io->set_iomap_range_rw(0x64, 0x67, pio2);
//	io->set_iomap_range_rw(0x6c, 0x6f, fdc ); // uPD72605
	
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
	speaker->initialize_sound(rate, 8000);
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
		speaker->set_volume(0, decibel_l, decibel_r);
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
}

void VM::key_up(int code)
{
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::load_binary(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		memory->read_image(file_path, ram, sizeof(ram));
	}
}

void VM::save_binary(int drv, const _TCHAR* file_path)
{
	if(drv == 0) {
		memory->write_image(file_path, ram, sizeof(ram));
	}
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

#define STATE_VERSION	1

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
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}

