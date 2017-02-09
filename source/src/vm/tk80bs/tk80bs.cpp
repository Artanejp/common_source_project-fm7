/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'
	NEC TK-80 Emulator 'eTK-80'
	NEC TK-85 Emulator 'eTK-85'

	Author : Takeda.Toshiya
	Date   : 2008.08.26 -

	[ virtual machine ]
*/

#include "tk80bs.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../i8080.h"
#if defined(_TK80BS)
#include "../i8251.h"
#include "../io.h"
#elif defined(_TK80) || defined(_TK85)
#include "../datarec.h"
#endif
#include "../i8255.h"
//#include "../memory.h"
#include "../pcm1bit.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#if defined(_TK80BS)
#include "cmt.h"
#endif
#include "display.h"
#include "keyboard.h"
#include "membus.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
#if defined(_TK80BS)
	// check configs
//	boot_mode = config.boot_mode;
	boot_mode = -1;
#endif	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
#if defined(_TK80BS)
	sio_b = new I8251(this, emu);	// on TK-80BS
	pio_b = new I8255(this, emu);
	memio = new IO(this, emu);
#elif defined(_TK80) || defined(_TK85)
	drec = new DATAREC(this, emu);
#endif
	pio_t = new I8255(this, emu);	// on TK-80
//	memory = new MEMORY(this, emu);
	
	pcm0 = new PCM1BIT(this, emu);
	pcm1 = new PCM1BIT(this, emu);
	
#if defined(_TK80BS)
	cmt = new CMT(this, emu);
#endif
	display = new DISPLAY(this, emu);
	keyboard = new KEYBOARD(this, emu);
	memory = new MEMBUS(this, emu);
	cpu = new I8080(this, emu);

#if defined(_USE_QT)
	dummy->set_device_name(_T("1st Dummy"));
#if defined(_TK80BS)
	sio_b->set_device_name(_T("i8251 SIO(TK-80BS/CMT)"));
	pio_b->set_device_name(_T("i8255 PIO(TK-80BS/DISPLAY)"));
	memio->set_device_name(_T("I/O"));
	pio_t->set_device_name(_T("i8255 PIO(TK-80/SOUND/KEYBOARD/DISPLAY)"));
#else
	pio_t->set_device_name(_T("i8255 PIO(TK-80/SOUND/KEYBOARD)"));
#endif
	pcm0->set_device_name(_T("PCM SOUND #1"));
	pcm1->set_device_name(_T("PCM SOUND #2"));
	cpu->set_device_name(_T("CPU(i8080)"));
#endif	
#if defined(_USE_QT)
#if defined(_TK80BS)
	cmt->set_device_name(_T("CMT I/F"));
#elif defined(_TK80) || defined(_TK85)
	drec->set_device_name(_T("Data Recorder."));
#endif
	display->set_device_name(_T("DISPLAY"));
	keyboard->set_device_name(_T("KEYBOARD"));
	memory->set_device_name(_T("MEMORY BUS"));
#endif	
	// set contexts
	event->set_context_cpu(cpu);
	event->set_context_sound(pcm0);
	event->set_context_sound(pcm1);
#if defined(_TK80) || defined(_TK85)
	event->set_context_sound(drec);
#endif
	
/*	8255 on TK-80
	
	PA	key matrix
	PB0	serial in
	PC0	serial out
	PC1	sound #1
	PC2	sound #2
	PC4-6	key column
	PC7	dma disable
*/
#if defined(_TK80BS)
	sio_b->set_context_out(cmt, SIG_CMT_OUT);
	pio_b->set_context_port_c(display, SIG_DISPLAY_MODE, 3, 0);
#elif defined(_TK80)
	// FIXME
	drec->set_context_ear(pio_t, SIG_I8255_PORT_B, 1);
	pio_t->set_context_port_c(drec, SIG_DATAREC_MIC, 1);
#elif defined(_TK85)
	drec->set_context_ear(cpu, SIG_I8085_SID, 1);
	cpu->set_context_sod(drec, SIG_DATAREC_MIC, 1);
#endif
	pio_t->set_context_port_c(pcm0, SIG_PCM1BIT_SIGNAL, 2, 0);
	pio_t->set_context_port_c(pcm1, SIG_PCM1BIT_SIGNAL, 4, 0);
	pio_t->set_context_port_c(keyboard, SIG_KEYBOARD_COLUMN, 0x70, 0);
#if defined(_TK80BS) || defined(_TK80)
	pio_t->set_context_port_c(display, SIG_DISPLAY_DMA, 0x80, 0);
#elif defined(_TK85)
	pio_t->set_context_port_c(memory, SIG_MEMBUS_PC7, 0x80, 0);
#endif
	// Sound:: Force realtime rendering. This is temporally fix. 20161024 K.O
	pcm0->set_realtime_render(true);
	pcm1->set_realtime_render(true);
	
#if defined(_TK80BS)
	cmt->set_context_sio(sio_b);
	display->set_vram_ptr(vram);
	keyboard->set_context_pio_b(pio_b);
	keyboard->set_context_cpu(cpu);
#endif
	display->set_led_ptr(ram + 0x3f8);
	keyboard->set_context_pio_t(pio_t);
	memory->set_context_cpu(cpu);
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(pio_t);
	cpu->set_context_intr(keyboard);
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// memory bus
	memset(mon, 0xff, sizeof(mon));
#if defined(_TK80BS)
	memset(bsmon, 0xff, sizeof(bsmon));
#endif
	memset(ext, 0xff, sizeof(ext));
	
#if defined(_TK80BS)
	static const uint8_t top[3] = {0xc3, 0x00, 0xf0};
	static const uint8_t rst[3] = {0xc3, 0xdd, 0x83};
	
	if(!memory->read_bios(_T("TK80.ROM"), mon, sizeof(mon))) {
		// default
		memcpy(mon, top, sizeof(top));
		memcpy(mon + 0x38, rst, sizeof(rst));
	}
	if(memory->read_bios(_T("BSMON.ROM"), bsmon, sizeof(bsmon))) {
		// patch
		memcpy(mon + 0x38, rst, sizeof(rst));
	}
#elif defined(_TK80)
	memory->read_bios(_T("TK80.ROM"), mon, sizeof(mon));
#elif defined(_TK85)
	memory->read_bios(_T("TK85.ROM"), mon, sizeof(mon));
#endif
	memory->read_bios(_T("EXT.ROM"), ext, sizeof(ext));
	
	memory->set_memory_r(0x0000, 0x07ff, mon);
	memory->set_memory_r(0x0c00, 0x7bff, ext);
#if defined(_TK80BS)
	memory->set_memory_mapped_io_rw(0x7c00, 0x7dff, memio);
	memory->set_memory_rw(0x7e00, 0x7fff, vram);
#endif
	memory->set_memory_rw(0x8000, 0xcfff, ram);
#if defined(_TK80BS)
	memory->set_memory_r(0xd000, 0xefff, basic);
	memory->set_memory_r(0xf000, 0xffff, bsmon);
	
	// memory mapped i/o
	memio->set_iomap_alias_rw(0x7df8, sio_b, 0);
	memio->set_iomap_alias_rw(0x7df9, sio_b, 1);
	memio->set_iomap_alias_rw(0x7dfc, pio_b, 0);
	memio->set_iomap_alias_rw(0x7dfd, pio_b, 1);
	memio->set_iomap_alias_rw(0x7dfe, pio_b, 2);
	memio->set_iomap_alias_w(0x7dff, pio_b, 3);
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
#if defined(_TK80BS)
	// load basic rom
	if(boot_mode != config.boot_mode) {
		memset(basic, 0xff, sizeof(basic));
		if(config.boot_mode == 0) {
			memory->read_bios(_T("LV1BASIC.ROM"), basic + 0x1000, 0x1000);
		} else {
			memory->read_bios(_T("LV2BASIC.ROM"), basic, sizeof(basic));
		}
		boot_mode = config.boot_mode;
		
		memset(ram, 0, sizeof(ram));
	}
	
	// initialize screen
	emu->reload_bitmap();
	draw_ranges = 8;
	memset(vram, 0x20, sizeof(vram));
#endif
	
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
	
	// init 8255 on TK-80
	pio_t->write_io8(0xfb, 0x92);
	pio_t->write_signal(SIG_I8255_PORT_A, 0xff, 0xff);
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

#if defined(_TK80BS)
int VM::max_draw_ranges()
{
	return draw_ranges;
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
	pcm0->initialize_sound(rate, 8000);
	pcm1->initialize_sound(rate, 8000);
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
		pcm0->set_volume(0, decibel_l, decibel_r);
	} else if(ch == 1) {
		pcm1->set_volume(0, decibel_l, decibel_r);
#if defined(_TK80) || defined(_TK85)
	} else if(ch == 2) {
		drec->set_volume(0, decibel_l, decibel_r);
#endif
	}
}
#endif

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
#if defined(_TK85)
	if(code == 0x97) {
		cpu->write_signal(SIG_CPU_NMI, 1, 1);
		return;
	}
#endif
	keyboard->key_down(code);
}

void VM::key_up(int code)
{
#if defined(_TK85)
	if(code == 0x97) {
		return;
	}
#endif
	keyboard->key_up(code);
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

void VM::play_tape(const _TCHAR* file_path)
{
#if defined(_TK80BS)
	cmt->play_tape(file_path);
#elif defined(_TK80) || defined(_TK85)
	drec->play_tape(file_path);
	drec->set_remote(true);
#endif
}

void VM::rec_tape(const _TCHAR* file_path)
{
#if defined(_TK80BS)
	cmt->rec_tape(file_path);
#elif defined(_TK80) || defined(_TK85)
	drec->rec_tape(file_path);
	drec->set_remote(true);
#endif
}

void VM::close_tape()
{
#if defined(_TK80BS)
	cmt->close_tape();
#elif defined(_TK80) || defined(_TK85)
	drec->close_tape();
	drec->set_remote(false);
#endif
}

bool VM::is_tape_inserted()
{
#if defined(_TK80BS)
	return cmt->is_tape_inserted();
#elif defined(_TK80) || defined(_TK85)
	return drec->is_tape_inserted();
#endif
}

#if defined(_TK80) || defined(_TK85)
bool VM::is_tape_playing()
{
	return drec->is_tape_playing();
}

bool VM::is_tape_recording()
{
	return drec->is_tape_recording();
}

int VM::get_tape_position()
{
	return drec->get_tape_position();
}
#endif

bool VM::is_frame_skippable()
{
//	return event->is_frame_skippable();
	return false;
}

void VM::update_config()
{
#if defined(_TK80BS)
	if(boot_mode != config.boot_mode) {
		// boot mode is changed !!!
//		boot_mode = config.boot_mode;
		reset();
	} else {
#endif
		for(DEVICE* device = first_device; device; device = device->next_device) {
			device->update_config();
		}
#if defined(_TK80BS)
	}
#endif
}

#define STATE_VERSION	3

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
	state_fio->Fwrite(ram, sizeof(ram), 1);
#if defined(_TK80BS)
	state_fio->Fwrite(vram, sizeof(vram), 1);
	state_fio->FputInt32(boot_mode);
//	state_fio->FputInt32(draw_ranges);
#endif
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
	state_fio->Fread(ram, sizeof(ram), 1);
#if defined(_TK80BS)
	state_fio->Fread(vram, sizeof(vram), 1);
	boot_mode = state_fio->FgetInt32();
//	draw_ranges = state_fio->FgetInt32();
	
	// post process
	emu->reload_bitmap();
	draw_ranges = 8;
#endif
	return true;
}

