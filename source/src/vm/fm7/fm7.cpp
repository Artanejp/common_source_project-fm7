
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
#include "../memory.h"
#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif


#include "../datarec.h"
#include "../disk.h"

#include "../mc6809.h"
#include "../z80.h"
#include "../ym2203.h"
#include "../mb8877.h"
//#include "../beep.h"
#include "../pcm1bit.h"
#include "../ym2203.h"
#if defined(_FM77AV_VARIANTS)
#include "./mb61vh010.h"
#endif

#include "./fm7_mainio.h"
#include "./fm7_mainmem.h"
#include "./fm7_display.h"
#include "./fm7_keyboard.h"
#include "./joystick.h"

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
	kanjiclass1 = new KANJIROM(this, emu, false);
#ifdef CAPABLE_KANJI_CLASS2
	kanjiclass2 = new KANJIROM(this, emu, true);
#endif
	joystick  = new JOYSTICK(this, emu);
	
	// I/Os
	drec = new DATAREC(this, emu);
	pcm1bit = new PCM1BIT(this, emu);
//	beep = new BEEP(this, emu);
	fdc  = new MB8877(this, emu);
	
	opn[0] = new YM2203(this, emu); // OPN
	opn[1] = new YM2203(this, emu); // WHG
	opn[2] = new YM2203(this, emu); // THG
#if !defined(_FM77AV_VARIANTS)
	psg = new YM2203(this, emu);
#endif
	display = new DISPLAY(this, emu);
	mainio  = new FM7_MAINIO(this, emu);
	mainmem = new FM7_MAINMEM(this, emu);
	keyboard = new KEYBOARD(this, emu);
#if defined(_FM77AV_VARIANTS)
	alu = new MB61VH010(this, emu);
#endif	
		
	// basic devices
	subcpu = new MC6809(this, emu);
#ifdef WITH_Z80
	z80cpu = new Z80(this, emu);
#endif
	maincpu = new MC6809(this, emu);
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
	uint32 mainclock;
	uint32 subclock;
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
#if defined(_FM77AV40) || defined(_FM77AV20)
	event->set_context_cpu(dummycpu, MAINCLOCK_FAST_MMR * 2);
#else
	event->set_context_cpu(dummycpu, SUBCLOCK_NORMAL * 2);
#endif
	
#if defined(_FM8)
	mainclock = MAINCLOCK_SLOW;
	subclock = SUBCLOCK_SLOW;
#else
	if(config.cpu_type == 0) {
		// 2MHz
		subclock = SUBCLOCK_NORMAL;
		mainclock = MAINCLOCK_NORMAL;
	} else {
		// 1.2MHz
		mainclock = MAINCLOCK_SLOW;
		subclock = SUBCLOCK_SLOW;
	}
#endif
#if defined(_FM77AV40) || defined(_FM77AV20)
	event->set_context_cpu(maincpu, MAINCLOCK_FAST_MMR);
#else
	event->set_context_cpu(maincpu, MAINCLOCK_NORMAL);
#endif	
	event->set_context_cpu(subcpu,  SUBCLOCK_NORMAL);
   
#ifdef WITH_Z80
	event->set_context_cpu(z80cpu,  4000000);
	z80cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
#endif

	event->set_context_sound(pcm1bit);
#if !defined(_FM77AV_VARIANTS)
	mainio->set_context_psg(psg);
	event->set_context_sound(psg);
#endif
	event->set_context_sound(opn[0]);
	event->set_context_sound(opn[1]);
	event->set_context_sound(opn[2]);
	event->set_context_sound(drec);
	//event->register_vline_event(display);
	event->register_frame_event(display);
	//event->register_vline_event(mainio);
   
	mainio->set_context_maincpu(maincpu);
	mainio->set_context_subcpu(subcpu);
	
	mainio->set_context_display(display);
	mainio->set_context_kanjirom_class1(kanjiclass1);
	mainio->set_context_mainmem(mainmem);
	mainio->set_context_keyboard(keyboard);
   
#if defined(CAPABLE_KANJI_CLASS2)
        mainio->set_context_kanjirom_class2(kanjiclass2);
#endif

	keyboard->set_context_break_line(mainio, FM7_MAINIO_PUSH_BREAK, 0xffffffff);
	keyboard->set_context_int_line(mainio, FM7_MAINIO_KEYBOARDIRQ, 0xffffffff);
	keyboard->set_context_int_line(display, SIG_FM7_SUB_KEY_FIRQ, 0xffffffff);
	
	//keyboard->set_context_rxrdy(keyboard, SIG_FM7KEY_RXRDY, 0x01);
	keyboard->set_context_rxrdy(display, SIG_FM7KEY_RXRDY, 0x01);
	
	//keyboard->set_context_key_ack(keyboard, SIG_FM7KEY_ACK, 0x01);
	keyboard->set_context_key_ack(display, SIG_FM7KEY_ACK, 0x01);
   
	drec->set_context_out(mainio, FM7_MAINIO_CMT_RECV, 0xffffffff);
	//drec->set_context_remote(mainio, FM7_MAINIO_CMT_REMOTE, 0xffffffff);
	mainio->set_context_datarec(drec);
	mainmem->set_context_mainio(mainio);
	mainmem->set_context_display(display);
	mainmem->set_context_maincpu(maincpu);
  
	display->set_context_mainio(mainio);
	display->set_context_subcpu(subcpu);
	display->set_context_keyboard(keyboard);
	subcpu->set_context_bus_halt(display, SIG_FM7_SUB_HALT, 0xffffffff);
	subcpu->set_context_bus_halt(mainmem, SIG_FM7_SUB_HALT, 0xffffffff);

	display->set_context_kanjiclass1(kanjiclass1);
#if defined(CAPABLE_KANJI_CLASS2)
	display->set_context_kanjiclass2(kanjiclass2);
#endif   
#if defined(_FM77AV_VARIANTS)
	display->set_context_alu(alu);
	alu->set_context_memory(display);
#endif	
	// Palette, VSYNC, HSYNC, Multi-page, display mode. 
	mainio->set_context_display(display);
	
	//FDC
	mainio->set_context_fdc(fdc);
	fdc->set_context_irq(mainio, FM7_MAINIO_FDC_IRQ, 0x1);
	fdc->set_context_drq(mainio, FM7_MAINIO_FDC_DRQ, 0x1);
	// SOUND
	mainio->set_context_beep(pcm1bit);
	//mainio->set_context_beep(beep);
	
	opn[0]->set_context_irq(mainio, FM7_MAINIO_OPN_IRQ, 0xffffffff);
	mainio->set_context_opn(opn[0], 0);
	joystick->set_context_opn(opn[0]);
	mainio->set_context_joystick(joystick);
	
	opn[1]->set_context_irq(mainio, FM7_MAINIO_WHG_IRQ, 0xffffffff);
	mainio->set_context_opn(opn[1], 1);
	opn[2]->set_context_irq(mainio, FM7_MAINIO_THG_IRQ, 0xffffffff);
	mainio->set_context_opn(opn[2], 2);
   
	subcpu->set_context_bus_halt(display, SIG_FM7_SUB_HALT, 0xffffffff);
	subcpu->set_context_bus_clr(display, SIG_FM7_SUB_USE_CLR, 0x0000000f);
   
	maincpu->set_context_mem(mainmem);
	subcpu->set_context_mem(display);
#ifdef USE_DEBUGGER
	maincpu->set_context_debugger(new DEBUGGER(this, emu));
	subcpu->set_context_debugger(new DEBUGGER(this, emu));
#endif
	event->register_frame_event(joystick);
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}
	//maincpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
	//subcpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
   
	for(int i = 0; i < 2; i++) {
#if defined(_FM77AV20) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
		fdc->set_drive_type(i, DRIVE_TYPE_2DD);
#else
		fdc->set_drive_type(i, DRIVE_TYPE_2D);
#endif
		fdc->set_drive_rpm(i, 360);
		fdc->set_drive_mfm(i, true);
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
#if !defined(_FM8)
	switch(config.cpu_type){
		case 0:
	       		event->set_secondary_cpu_clock(maincpu, MAINCLOCK_NORMAL);
			break;
		case 1:
	       		event->set_secondary_cpu_clock(maincpu, MAINCLOCK_SLOW);
			break;
	}
#endif

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
	opn[0]->SetReg(0x2e, 0);	// set prescaler
	opn[1]->SetReg(0x2e, 0);	// set prescaler
	opn[2]->SetReg(0x2e, 0);	// set prescaler

	// Init OPN/PSG.
	// Parameters from XM7.
	opn[0]->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?
	opn[1]->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?
	opn[2]->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?

#if !defined(_FM77AV_VARIANTS)
	psg->SetReg(0x27, 0); // stop timer
	psg->SetReg(0x2e, 0);	// set prescaler
	psg->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?
#endif	
}

void VM::special_reset()
{
	// BREAK + RESET
	mainio->write_signal(FM7_MAINIO_PUSH_BREAK, 1, 1);
	mainio->reset();
	mainmem->reset();
	
#if defined(FM77AV_VARIANTS)	
	mainio->write_signal(FM7_MAINIO_HOT_RESET, 1, 1);
#endif	
	display->reset();
	maincpu->reset();
	mainio->write_signal(FM7_MAINIO_PUSH_BREAK, 1, 1);
	event->register_event(mainio, EVENT_UP_BREAK, 10000.0 * 1000.0, false, NULL);
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
	opn[0]->init(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
	opn[1]->init(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
	opn[2]->init(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
#if !defined(_FM77AV_VARIANTS)   
	psg->init(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
#endif   
	pcm1bit->init(rate, 2000);
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

#if defined(USE_DISK_WRITE_PROTECT)
void VM::write_protect_fd(int drv, bool flag)
{
	fdc->write_protect_fd(drv, flag);
}

bool VM::is_write_protect_fd(int drv)
{
        return fdc->is_write_protect_fd(drv);
}
#endif

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

#if defined(USE_TAPE_PTR)
int VM::get_tape_ptr(void)
{
        return drec->get_tape_ptr();
}
#endif

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
	event->set_secondary_cpu_clock(cpu, clocks);
}

#define STATE_VERSION	1

