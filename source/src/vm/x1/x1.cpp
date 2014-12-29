/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2009.03.11-

	[ virtual machine ]
*/

#include "x1.h"
#include "../../emu.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../disk.h"
#include "../hd46505.h"
#include "../i8255.h"
#include "../mb8877.h"
#include "../ym2151.h"
#include "../ym2203.h"
#include "../z80.h"
#include "../z80ctc.h"
#include "../z80sio.h"
#ifdef _X1TURBO_FEATURE
#include "../z80dma.h"
#endif

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "display.h"
#include "emm.h"
#include "floppy.h"
#include "io.h"
#include "joystick.h"
#include "memory.h"
#include "mouse.h"
#include "printer.h"
#include "psub.h"

#include "../mcs48.h"
#include "../upd1990a.h"
#include "sub.h"
#include "keyboard.h"

#ifdef _X1TWIN
#include "../huc6280.h"
#include "../pcengine/pce.h"
#endif

#include "../../fileio.h"

// ----------------------------------------------------------------------------
// initialize
// ----------------------------------------------------------------------------

VM::VM(EMU* parent_emu) : emu(parent_emu)
{
	FILEIO* fio = new FILEIO();
	pseudo_sub_cpu = !(fio->IsFileExists(emu->bios_path(SUB_ROM_FILE_NAME)) && fio->IsFileExists(emu->bios_path(KBD_ROM_FILE_NAME)));
	delete fio;
	
	sound_device_type = config.sound_device_type;
	
	// create devices
	first_device = last_device = NULL;
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	drec = new DATAREC(this, emu);
	crtc = new HD46505(this, emu);
	pio = new I8255(this, emu);
	fdc = new MB8877(this, emu);
	psg = new YM2203(this, emu);
	cpu = new Z80(this, emu);
	ctc = new Z80CTC(this, emu);
	sio = new Z80SIO(this, emu);
	if(sound_device_type >= 1) {
		opm1 = new YM2151(this, emu);
		ctc1 = new Z80CTC(this, emu);
	}
	if(sound_device_type == 2) {
		opm2 = new YM2151(this, emu);
		ctc2 = new Z80CTC(this, emu);
	}
#ifdef _X1TURBO_FEATURE
	dma = new Z80DMA(this, emu);
#endif
	
	display = new DISPLAY(this, emu);
	emm = new EMM(this, emu);
	floppy = new FLOPPY(this, emu);
	io = new IO(this, emu);
	joy = new JOYSTICK(this, emu);
	memory = new MEMORY(this, emu);
	mouse = new MOUSE(this, emu);
	printer = new PRINTER(this, emu);
	
	if(pseudo_sub_cpu) {
		psub = new PSUB(this, emu);
		cpu_sub = NULL;
		cpu_kbd = NULL;
	} else {
		// sub cpu
		cpu_sub = new MCS48(this, emu);
		pio_sub = new I8255(this, emu);
		rtc_sub = new UPD1990A(this, emu);
		sub = new SUB(this, emu);
		
		// keyboard
		cpu_kbd = new MCS48(this, emu);
		kbd = new KEYBOARD(this, emu);
	}
	
	// set contexts
	event->set_context_cpu(cpu);
	if(!pseudo_sub_cpu) {
		event->set_context_cpu(cpu_sub, 6000000);
		event->set_context_cpu(cpu_kbd, 6000000);
	}
	if(sound_device_type >= 1) {
		event->set_context_sound(opm1);
	}
	if(sound_device_type == 2) {
		event->set_context_sound(opm2);
	}
	event->set_context_sound(psg);
	
	drec->set_context_out(pio, SIG_I8255_PORT_B, 0x02);
	crtc->set_context_vblank(display, SIG_DISPLAY_VBLANK, 1);
	crtc->set_context_vblank(pio, SIG_I8255_PORT_B, 0x80);
	crtc->set_context_vsync(pio, SIG_I8255_PORT_B, 0x04);
	pio->set_context_port_a(printer, SIG_PRINTER_OUT, 0xff, 0);
	pio->set_context_port_c(drec, SIG_DATAREC_OUT, 0x01, 0);
	pio->set_context_port_c(display, SIG_DISPLAY_COLUMN40, 0x40, 0);
	pio->set_context_port_c(io, SIG_IO_MODE, 0x60, 0);
	pio->set_context_port_c(printer, SIG_PRINTER_STB, 0x80, 0);
#ifdef _X1TURBO_FEATURE
	fdc->set_context_drq(dma, SIG_Z80DMA_READY, 1);
#endif
	ctc->set_context_zc0(ctc, SIG_Z80CTC_TRIG_3, 1);
	ctc->set_constant_clock(1, CPU_CLOCKS >> 1);
	ctc->set_constant_clock(2, CPU_CLOCKS >> 1);
	sio->set_context_rts1(mouse, SIG_MOUSE_RTS, 1);
	if(sound_device_type >= 1) {
		ctc1->set_context_zc0(ctc1, SIG_Z80CTC_TRIG_3, 1);
//		ctc1->set_constant_clock(1, CPU_CLOCKS >> 1);
//		ctc1->set_constant_clock(2, CPU_CLOCKS >> 1);
	}
	if(sound_device_type == 2) {
		ctc2->set_context_zc0(ctc2, SIG_Z80CTC_TRIG_3, 1);
//		ctc2->set_constant_clock(1, CPU_CLOCKS >> 1);
//		ctc2->set_constant_clock(2, CPU_CLOCKS >> 1);
	}
#ifdef _X1TURBO_FEATURE
	dma->set_context_memory(memory);
	dma->set_context_io(io);
#endif
	
#ifdef _X1TURBO_FEATURE
	display->set_context_cpu(cpu);
	display->set_context_crtc(crtc);
#endif
	display->set_vram_ptr(io->get_vram());
	display->set_regs_ptr(crtc->get_regs());
	floppy->set_context_fdc(fdc);
	io->set_context_cpu(cpu);
	joy->set_context_psg(psg);
#ifdef _X1TURBO_FEATURE
	memory->set_context_pio(pio);
#endif
	mouse->set_context_sio(sio);
	
	if(pseudo_sub_cpu) {
		drec->set_context_remote(psub, SIG_PSUB_TAPE_REMOTE, 1);
		drec->set_context_end(psub, SIG_PSUB_TAPE_END, 1);
		psub->set_context_pio(pio);
		psub->set_context_drec(drec);
	} else {
		// sub cpu
		cpu_sub->set_context_mem(new MCS48MEM(this, emu));
		cpu_sub->set_context_io(sub);
#ifdef USE_DEBUGGER
		cpu_sub->set_context_debugger(new DEBUGGER(this, emu));
#endif
		drec->set_context_end(sub, SIG_SUB_TAPE_END, 1);
		drec->set_context_apss(sub, SIG_SUB_TAPE_APSS, 1);
		pio_sub->set_context_port_c(sub, SIG_SUB_PIO_PORT_C, 0x80, 0);
		// pc1:break -> pb0 of 8255(main)
		pio_sub->set_context_port_c(pio, SIG_I8255_PORT_B, 0x02, -1);
		// pc5:ibf -> pb6 of 8255(main)
		pio_sub->set_context_port_c(pio, SIG_I8255_PORT_B, 0x20, 1);
		// pc7:obf -> pb5 of 8255(main)
		pio_sub->set_context_port_c(pio, SIG_I8255_PORT_B, 0x80, -2);
		// pc7:obf -> pb7 of 8255(sub)
		pio_sub->set_context_port_c(pio_sub, SIG_I8255_PORT_B, 0x80, 0);
		
		sub->set_context_pio(pio_sub);
		sub->set_context_rtc(rtc_sub);
		sub->set_context_drec(drec);
		
		// keyboard
		cpu_kbd->set_context_mem(new MCS48MEM(this, emu));
		cpu_kbd->set_context_io(kbd);
#ifdef USE_DEBUGGER
		cpu_kbd->set_context_debugger(new DEBUGGER(this, emu));
#endif
		kbd->set_context_cpu(cpu_sub);
	}
	
	// cpu bus
	cpu->set_context_mem(memory);
	cpu->set_context_io(io);
#if defined(_X1TURBO_FEATURE) && defined(SINGLE_MODE_DMA)
	cpu->set_context_dma(dma);
#endif
#ifdef USE_DEBUGGER
	cpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	
	// z80 family daisy chain
	DEVICE* parent_dev = NULL;
	int level = 0;
	
	#define Z80_DAISY_CHAIN(dev) { \
		if(parent_dev == NULL) { \
			cpu->set_context_intr(dev); \
		} else { \
			parent_dev->set_context_child(dev); \
		} \
		dev->set_context_intr(cpu, level++); \
		parent_dev = dev; \
	}
#ifndef _X1TURBO_FEATURE
	Z80_DAISY_CHAIN(sio);	// CZ-8BM2
	Z80_DAISY_CHAIN(ctc);
#endif
	if(sound_device_type >= 1) {
		Z80_DAISY_CHAIN(ctc1);
	}
	if(sound_device_type == 2) {
		Z80_DAISY_CHAIN(ctc2);
	}
#ifdef _X1TURBO_FEATURE
	Z80_DAISY_CHAIN(sio);
	Z80_DAISY_CHAIN(dma);
	Z80_DAISY_CHAIN(ctc);
#endif
	if(pseudo_sub_cpu) {
		Z80_DAISY_CHAIN(psub);
	} else {
		Z80_DAISY_CHAIN(sub);
	}
	
	// i/o bus
	if(sound_device_type >= 1) {
		io->set_iomap_single_w(0x700, opm1);
		io->set_iovalue_single_r(0x700, 0x00);
		io->set_iomap_single_rw(0x701, opm1);
#ifdef _X1TURBOZ
		io->set_flipflop_single_rw(0x704, 0x00);
#else
		io->set_iomap_range_rw(0x704, 0x707, ctc1);
#endif
	}
	if(sound_device_type == 2) {
		io->set_iomap_single_w(0x708, opm2);
		io->set_iovalue_single_r(0x708, 0x00);
		io->set_iomap_single_rw(0x709, opm2);
		io->set_iomap_range_rw(0x70c, 0x70f, ctc2);
	}
#ifdef _X1TURBO_FEATURE
	io->set_iomap_single_rw(0xb00, memory);
#endif
	io->set_iomap_range_rw(0xd00, 0xd03, emm);
	io->set_iomap_range_r(0xe80, 0xe81, display);
	io->set_iomap_range_w(0xe80, 0xe82, display);
	io->set_iomap_range_rw(0xff8, 0xffb, fdc);
	io->set_iomap_single_w(0xffc, floppy);
#ifdef _X1TURBO_FEATURE
	io->set_iomap_range_r(0xffc, 0xfff, floppy);
#endif
	io->set_iomap_range_rw(0x1000, 0x17ff, display);
	for(int i = 0x1800; i <= 0x18ff; i += 0x10) {
		io->set_iomap_range_rw(i, i + 1, crtc);
	}
	if(pseudo_sub_cpu) {
		io->set_iomap_range_rw(0x1900, 0x19ff, psub);
	} else {
		io->set_iomap_range_rw(0x1900, 0x19ff, sub);
	}
	for(int i = 0x1a00; i <= 0x1aff; i += 4) {
		io->set_iomap_range_rw(i, i + 3, pio);
	}
	for(int i = 0x1b00; i <= 0x1bff; i++) {
		io->set_iomap_alias_rw(i, psg, 1);
	}
	for(int i = 0x1c00; i <= 0x1cff; i++) {
		io->set_iomap_alias_w(i, psg, 0);
	}
	io->set_iomap_range_w(0x1d00, 0x1eff, memory);
#ifndef _X1TURBO_FEATURE
	io->set_iomap_range_rw(0x1f98, 0x1f9b, sio);	// CZ-8BM2
	io->set_iomap_range_rw(0x1fa8, 0x1fab, ctc);
#else
	io->set_iomap_range_rw(0x1f80, 0x1f8f, dma);
	io->set_iomap_range_rw(0x1f90, 0x1f93, sio);
	io->set_iomap_range_rw(0x1fa0, 0x1fa3, ctc);
#ifdef _X1TURBOZ
	io->set_iomap_single_rw(0x1fd0, display);
	io->set_iomap_single_rw(0x1fe0, display);
#else
	io->set_iomap_single_w(0x1fd0, display);
	io->set_iomap_single_w(0x1fe0, display);
#endif
	// 0x1ff0: dipswitch
//	io->set_iovalue_single_r(0x1ff0, 0x00);
	update_dipswitch();
#endif
	io->set_iomap_range_rw(0x2000, 0x3fff, display);	// tvram
	
#ifdef _X1TWIN
	// init PC Engine
	pceevent = new EVENT(this, emu);
	pceevent->set_frames_per_sec(PCE_FRAMES_PER_SEC);
	pceevent->set_lines_per_frame(PCE_LINES_PER_FRAME);
	
	pcecpu = new HUC6280(this, emu);
	pcecpu->set_context_event_manager(pceevent);
	pce = new PCE(this, emu);
	pce->set_context_event_manager(pceevent);
	
	pceevent->set_context_cpu(pcecpu, PCE_CPU_CLOCKS);
	pceevent->set_context_sound(pce);
	
	pcecpu->set_context_mem(pce);
	pcecpu->set_context_io(pce);
#ifdef USE_DEBUGGER
//	pcecpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	pce->set_context_cpu(pcecpu);
#endif
	
	// initialize all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	if(!pseudo_sub_cpu) {
		// load rom images after cpustate is allocated
		cpu_sub->load_rom_image(emu->bios_path(SUB_ROM_FILE_NAME));
		cpu_kbd->load_rom_image(emu->bios_path(KBD_ROM_FILE_NAME));
		
		// patch to set the current year
		uint8 *rom = cpu_sub->get_rom_ptr();
		sub->rom_crc32 = getcrc32(rom, 0x800);	// 2KB
		if(rom[0x23] == 0xb9 && rom[0x24] == 0x35 && rom[0x25] == 0xb1) {
			cur_time_t cur_time;
			emu->get_host_time(&cur_time);
			rom[0x26] = TO_BCD(cur_time.year);
		}
	}
	for(int i = 0; i < MAX_DRIVE; i++) {
#ifdef _X1TURBO_FEATURE
		fdc->set_drive_type(i, DRIVE_TYPE_2DD);
#else
		fdc->set_drive_type(i, DRIVE_TYPE_2D);
#endif
//		fdc->set_drive_rpm(i, 300);
//		fdc->set_drive_mfm(i, true);
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
	psg->SetReg(0x2e, 0);	// set prescaler
}

void VM::special_reset()
{
	// nmi reset
	cpu->write_signal(SIG_CPU_NMI, 1, 1);
}

void VM::run()
{
	event->drive();
#ifdef _X1TWIN
	if(pce->cart_inserted()) {
		pceevent->drive();
	}
#endif
}

double VM::frame_rate()
{
#ifdef _X1TWIN
	if(pce->cart_inserted()) {
		return pceevent->frame_rate();
	}
#endif
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
	} else if(index == 1) {
		return cpu_sub;
	} else if(index == 2) {
		return cpu_kbd;
#ifdef _X1TWIN
	} else if(index == 3 && pce->cart_inserted()) {
		return pcecpu;
#endif
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
#ifdef _X1TWIN
	if(pce->cart_inserted()) {
		pce->draw_screen();
	}
#endif
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
#ifdef _X1TWIN
	pceevent->initialize_sound(rate, samples);
#endif
	
	// init sound gen
	if(sound_device_type >= 1) {
		opm1->init(rate, 4000000, samples, 0);
	}
	if(sound_device_type == 2) {
		opm2->init(rate, 4000000, samples, 0);
	}
	psg->init(rate, 2000000, samples, 0, 0);
#ifdef _X1TWIN
	pce->initialize_sound(rate);
#endif
}

uint16* VM::create_sound(int* extra_frames)
{
#ifdef _X1TWIN
	if(pce->cart_inserted()) {
		uint16* buffer = pceevent->create_sound(extra_frames);
		for(int i = 0; i < *extra_frames; i++) {
			event->drive();
		}
		return buffer;
	}
#endif
	return event->create_sound(extra_frames);
}

int VM::sound_buffer_ptr()
{
#ifdef _X1TWIN
	if(pce->cart_inserted()) {
		return pceevent->sound_buffer_ptr();
	}
#endif
	return event->sound_buffer_ptr();
}

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
#ifdef _X1TWIN
	if(!repeat && !pce->cart_inserted()) {
#else
	if(!repeat) {
#endif
		if(pseudo_sub_cpu) {
			psub->key_down(code, false);
		} else {
			kbd->key_down(code, false);
		}
	}
}

void VM::key_up(int code)
{
#ifdef _X1TWIN
	if(!pce->cart_inserted()) {
#endif
		if(pseudo_sub_cpu) {
			psub->key_up(code);
		} else {
			//kbd->key_up(code);
		}
#ifdef _X1TWIN
	}
#endif
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
	bool value = drec->play_tape(file_path);
	if(pseudo_sub_cpu) {
		psub->close_tape();
		psub->play_tape(value);
	} else {
		sub->close_tape();
		sub->play_tape(value);
	}
}

void VM::rec_tape(_TCHAR* file_path)
{
	bool value = drec->rec_tape(file_path);
	if(pseudo_sub_cpu) {
		psub->close_tape();
		psub->rec_tape(value);
	} else {
		sub->close_tape();
		sub->rec_tape(value);
	}
}

void VM::close_tape()
{
	drec->close_tape();
	if(pseudo_sub_cpu) {
		psub->close_tape();
	} else {
		sub->close_tape();
	}
}

bool VM::tape_inserted()
{
	return drec->tape_inserted();
}

bool VM::now_skip()
{
#ifdef _X1TWIN
	if(pce->cart_inserted()) {
		return pceevent->now_skip();
	}
#endif
	return event->now_skip();
}

#ifdef _X1TWIN
void VM::open_cart(int drv, _TCHAR* file_path)
{
	if(drv == 0) {
		pce->open_cart(file_path);
		pce->reset();
		pcecpu->reset();
	}
}

void VM::close_cart(int drv)
{
	if(drv == 0) {
		pce->close_cart();
		pce->reset();
		pcecpu->reset();
	}
}

bool VM::cart_inserted(int drv)
{
	if(drv == 0) {
		return pce->cart_inserted();
	} else {
		return false;
	}
}
#endif

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
#ifdef _X1TURBO_FEATURE
	update_dipswitch();
#endif
}

#ifdef _X1TURBO_FEATURE
void VM::update_dipswitch()
{
	// bit0		0=High 1=Standard
	// bit1-3	0=5"2D 4=5"2HD
	io->set_iovalue_single_r(0x1ff0, config.monitor_type & 1);
}
#endif

#define STATE_VERSION	1

void VM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->save_state(state_fio);
	}
	state_fio->FputBool(pseudo_sub_cpu);
	state_fio->FputInt32(sound_device_type);
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
	pseudo_sub_cpu = state_fio->FgetBool();
	sound_device_type = state_fio->FgetInt32();
	return true;
}

