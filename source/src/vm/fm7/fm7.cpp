/*
 * FM7 -> VM
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History:
 *   Feb 27, 2015 : Initial
 */

#include "fm7.h"
#include "../../emu.h"
#include "../../config.h"
#include "../device.h"
#include "../event.h"

#include "../datarec.h"
#include "../disk.h"

#include "../mc6809.h"
#include "../z80.h"
#include "../ym2203.h"
#include "../mb8877.h"

#include "./fm7_mainio.h"
#include "./fm7_mainmem.h"
#include "./fm7_display.h"
#include "./fm7_keyboard.h"

#include "./kanjirom.h"

VM::VM(EMU* parent_emu): emu(parent_emu)
{
	
	first_device = last_device = NULL;
	connect_opn = false;
	connect_whg = false;
	connect_thg = false;
	opn[0] = opn[1] = opn[2] = psg = NULL; 
   
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device
	
	dummycpu = new DEVICE(this, emu);
	maincpu = new MC6809(this, emu);
	subcpu = new MC6809(this, emu);
#ifdef WITH_Z80
	z80cpu = new Z80(this, emu);
#endif
	// basic devices
	mainmem = new FM7_MAINMEM(this, emu);
	mainio  = new FM7_MAINIO(this, emu);
	
	display = new DISPLAY(this, emu);
	keyboard = new KEYBOARD(this, emu);

	// I/Os
	drec = new DATAREC(this, emu);
	pcm1bit = new PCM1BIT(this, emu);
	fdc  = new MB8877(this, emu);
	
	switch(config.sound_device_type) {
		case 0:
			break;
		case 1:
	   		connect_opn = true;
	   		break;
		case 2:
	   		connect_whg = true;
	   		break;
		case 3:
	   		connect_whg = true;
	   		connect_opn = true;
	   		break;
		case 4:
	   		connect_thg = true;
	   		break;
		case 5:
	 		connect_thg = true;
	   		connect_opn = true;
	   		break;
		case 6:
	   		connect_thg = true;
	   		connect_whg = true;
	   		break;
		case 7:
	   		connect_thg = true;
	   		connect_whg = true;
	   		connect_opn = true;
	   		break;
	}
   
	if(config.sound_device_type != 0) {
		if(connect_opn) opn[0] = new YM2203(this, emu); // OPN
		if(connect_whg) opn[1] = new YM2203(this, emu); // WHG
		if(connect_thg) opn[2] = new YM2203(this, emu); // THG
	}
   
#if !defined(_FM77AV_VARIANTS)
	psg = new YM2203(this, emu);
#endif
	kanjiclass1 = new KANJIROM(this, emu, false);
#ifdef CAPABLE_KANJI_CLASS2
	kanjiclass2 = new KANJIROM(this, emu, true);
#endif
	connect_bus();
	initialize();
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


void VM::initialize(void)
{
#if defined(_FM8) || defined(_FM7)
	cycle_steal = false;
#else
	cycle_steal = true;
#endif
	clock_low = false;

}


void VM::connect_bus(void)
{
	int i;
	
	/*
	 * CLASS CONSTRUCTION
	 *
	 * VM 
	 *  |-> MAINCPU -> MAINMEM -> MAINIO -> MAIN DEVICES
	 *  |             |        |      
	 *  | -> SUBCPU  -> SUBMEM  -> SUBIO -> SUB DEVICES
	 *  | -> DISPLAY
	 *  | -> KEYBOARD
	 *
	 *  MAINMEM can access SUBMEM/IO, when SUBCPU is halted.
	 *  MAINMEM and SUBMEM can access DISPLAY and KEYBOARD with exclusive.
	 *  MAINCPU can access MAINMEM.
	 *  SUBCPU  can access SUBMEM.
	 *  DISPLAY : R/W from MAINCPU and SUBCPU.
	 *  KEYBOARD : R/W
	 *
	 */
	event->set_frames_per_sec(60.00);
	event->set_lines_per_frame(400);
	event->set_context_cpu(dummycpu, 8000000);
#if defined(_FM8)
	event->set_context_cpu(maincpu, 1095000);
	event->set_context_cpu(subcpu,   999000);
#else
	event->set_context_cpu(maincpu, 1794000);
	event->set_context_cpu(subcpu,  2000000);
#endif
#ifdef WITH_Z80
	event->set_context_cpu(z80cpu,  4000000);
	z80cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
#endif
	mainio->set_context_maincpu(maincpu);
	mainio->set_context_subcpu(subcpu);
	
	mainio->set_context_display(display);
        mainio->set_context_kanjirom_class1(kanjiclass1);
        mainio->set_context_mainmem(mainmem);
   
#if defined(_FM77AV_VARIANTS)
        mainio->set_context_kanjirom_class2(kanjiclass2);
#endif

	keyboard->set_context_break_line(mainio, FM7_MAINIO_PUSH_BREAK, 0xffffffff);
	keyboard->set_context_mainio(mainio);
	keyboard->set_context_display(display);
   
	drec->set_context_out(mainio, FM7_MAINIO_CMT_RECV, 0xffffffff);
	//drec->set_context_remote(mainio, FM7_MAINIO_CMT_REMOTE, 0xffffffff);
	mainio->set_context_datarec(drec);
  
	display->set_context_mainio(mainio);
	display->set_context_subcpu(subcpu);
	display->set_context_keyboard(keyboard);
        display->set_context_kanjiclass1(kanjiclass1);
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
        display->set_context_kanjiclass2(kanjiclass2);
#endif   
	// Palette, VSYNC, HSYNC, Multi-page, display mode. 
	mainio->set_context_display(display);
	
	//FDC
	mainio->set_context_fdc(fdc);
	fdc->set_context_irq(mainio, FM7_MAINIO_FDC_IRQ, 0x1);
	fdc->set_context_drq(mainio, FM7_MAINIO_FDC_DRQ, 0x1);
	// SOUND
	mainio->set_context_beep(pcm1bit);
	event->set_context_sound(pcm1bit);
	
	if(connect_opn) {
		opn[0]->set_context_irq(mainio, FM7_MAINIO_OPN_IRQ, 0xffffffff);
		opn[0]->set_context_port_a(mainio, FM7_MAINIO_OPNPORTA_CHANGED, 0xff, 0);
		opn[0]->set_context_port_b(mainio, FM7_MAINIO_OPNPORTB_CHANGED, 0xff, 0);
		mainio->set_context_opn(opn[0], 0);
	}
	if(connect_whg) {
		opn[1]->set_context_irq(mainio, FM7_MAINIO_WHG_IRQ, 0xffffffff);
		mainio->set_context_opn(opn[1], 1);
	}
   
	if(connect_thg) {
		opn[2]->set_context_irq(mainio, FM7_MAINIO_THG_IRQ, 0xffffffff);
		mainio->set_context_opn(opn[2], 2);
	}
   
#if !defined(_FM77AV_VARIANTS)
	if(psg != NULL) {
		mainio->set_context_psg(psg);
		event->set_context_sound(psg);
	}
#endif
	if(connect_opn) {
		event->set_context_sound(opn[0]);
	}
	if(connect_whg) {
    		event->set_context_sound(opn[1]);
	}
	if(connect_thg) {
		event->set_context_sound(opn[2]);
	}
#ifdef DATAREC_SOUND
	event->set_context_sound(drec);
#endif
	mainmem->set_context_mainio(mainio);
	mainmem->set_context_display(display);
   
	maincpu->set_context_mem(mainmem);
	subcpu->set_context_mem(display);

	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	maincpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
	subcpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
   
	for(int i = 0; i < 2; i++) {
#if defined(_FM77AV20) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		fdc->set_drive_type(i, DRIVE_TYPE_2DD);
#else
		fdc->set_drive_type(i, DRIVE_TYPE_2D);
#endif
//		fdc->set_drive_rpm(i, 300);
//		fdc->set_drive_mfm(i, true);
	}
#if defined(_FM77) || defined(_FM77L4)
	for(int i = 2; i < 4; i++) {
		fdc->set_drive_type(i, DRIVE_TYPE_2HD);
//		fdc->set_drive_rpm(i, 300);
//		fdc->set_drive_mfm(i, true);
	}
#endif
	
}  

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
	//update_dipswitch();
}

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
	// BREAK + RESET
	mainio->write_signal(FM7_MAINIO_PUSH_BREAK, 1, 1);
	event->register_event(mainio, EVENT_UP_BREAK, 2000.0 * 1000.0, false, NULL);
	mainio->reset();
	display->reset();
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
		return maincpu;
	} else if(index == 1) {
		return subcpu;
	}
#if defined(_WITH_Z80)
	else if(index == 2) {
		return z80cpu;
	}
#endif
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

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	// init sound gen
	if(connect_opn) opn[0]->init(rate, 1228800, samples, 0, 0);
	if(connect_whg) opn[1]->init(rate, 1228800, samples, 0, 0);
	if(connect_thg) opn[2]->init(rate, 1228800, samples, 0, 0);
#if !defined(_FM77AV_VARIANTS)   
	psg->init(rate, 1228800, samples, 0, 0);
#endif   
	pcm1bit->init(rate, 8000);
	//drec->init_pcm(rate, 0);
}

uint16* VM::create_sound(int* extra_frames)
{
	uint16* p = event->create_sound(extra_frames);
	return p;
}

int VM::sound_buffer_ptr()
{
	int pos = event->sound_buffer_ptr();
	return pos; 
}

// ----------------------------------------------------------------------------
// notify key
// ----------------------------------------------------------------------------

void VM::key_down(int code, bool repeat)
{
	if(!repeat) {
		keyboard->key_down(code);
	}
}

void VM::key_up(int code)
{
	keyboard->key_up(code);
}

// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_disk(int drv, _TCHAR* file_path, int bank)
{
	fdc->open_disk(drv, file_path, bank);
}

void VM::close_disk(int drv)
{
	fdc->close_disk(drv);
}

bool VM::disk_inserted(int drv)
{
	return fdc->disk_inserted(drv);
}
 
void VM::write_protect_fd(int drv, bool flag)
{
	fdc->write_protect_fd(drv, flag);
}

bool VM::is_write_protect_fd(int drv)
{
        return fdc->is_write_protect_fd(drv);
}

void VM::play_tape(_TCHAR* file_path)
{
	bool value = drec->play_tape(file_path);
}

void VM::rec_tape(_TCHAR* file_path)
{
	bool value = drec->rec_tape(file_path);
}

void VM::close_tape()
{
	drec->close_tape();
}

bool VM::tape_inserted()
{
	return drec->tape_inserted();
}

int VM::get_tape_ptr(void)
{
        return drec->get_tape_ptr();
}

bool VM::now_skip()
{
	return event->now_skip();
}

void VM::update_dipswitch()
{
	// bit0		0=High 1=Standard
	// bit2		0=5"2D 1=5"2HD
  //	io->set_iovalue_single_r(0x1ff0, (config.monitor_type & 1) | ((config.drive_type & 1) << 2));
}

void VM::set_cpu_clock(DEVICE *cpu, uint32 clocks) {
	event->set_cpu_clock(cpu, clocks);
}

#define STATE_VERSION	1

