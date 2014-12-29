/*
	CASIO FP-200 Emulator 'eFP-200'

	Author : Takeda.Toshiya
	Date   : 2013.03.21-

	[ virtual machine ]
*/

#include "fp200.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../i8080.h"
#include "../memory.h"
#include "../rp5c01.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "io.h"

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
	
	cpu = new I8080(this, emu);	// i8085
	memory = new MEMORY(this, emu);
	rtc = new RP5C01(this, emu);
	
	io = new IO(this, emu);
	
	// set contexts
	event->set_context_cpu(cpu);
	
	cpu->set_context_sod(io, SIG_IO_SOD, 1);
	
	io->set_context_cpu(cpu);
	io->set_context_rtc(rtc);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
	cpu->set_context_intr(io);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// memory bus
	memset(rom, 0xff, sizeof(rom));
	memset(ram, 0, sizeof(ram));
	
	memory->read_bios(_T("BIOS.ROM"), rom, sizeof(rom));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("RAM.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
	
	memory->set_memory_r(0x0000, 0x7fff, rom);
	memory->set_memory_rw(0x8000, 0xffff, ram);
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
}

VM::~VM()
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("RAM.BIN")), FILEIO_WRITE_BINARY)) {
		fio->Fwrite(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
	
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
	io->draw_screen();
}

// ----------------------------------------------------------------------------
// soud manager
// ----------------------------------------------------------------------------

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
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
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	if(!repeat) {
		io->key_down(code);
	}
}

void VM::key_up(int code)
{
	io->key_up();
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::play_tape(_TCHAR* file_path)
{
	io->play_tape(file_path);
}

void VM::rec_tape(_TCHAR* file_path)
{
	io->rec_tape(file_path);
}

void VM::close_tape()
{
	io->close_tape();
}

bool VM::tape_inserted()
{
	return io->tape_inserted();
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

