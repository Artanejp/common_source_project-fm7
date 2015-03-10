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
//#include "./fm7_keyboard.h"

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

	// I/Os
	drec = new DATAREC(this, emu);
	//	keyboard = new FM7_KEYBOARD(this, emu);
	beep = new BEEP(this, emu);
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
   
	if(connect_opn) opn[0] = new YM2203(this, emu); // OPN
	if(connect_whg) opn[1] = new YM2203(this, emu); // WHG
	if(connect_thg) opn[2] = new YM2203(this, emu); // THG
#if !defined(_FM77AV_VARIANTS)
	psg = new YM2203(this, emu);
#endif
	kanjiclass1 = new KANJIROM(this, emu, false);
#ifdef CAPABLE_KANJI_CLASS2
	kanjiclass2 = new KANJIROM(this, emu, true);
#endif
  
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
        
	//	keyboard->set_context_mainio(mainio);
	//keyboard->set_context_subio(subio);
  
	maincpu->set_context_mem(mainmem);
   
	subcpu->set_context_mem(display);
 
	mainio->set_context_maincpu(maincpu);
	mainio->set_context_display(display);
        mainio->set_context_kanjirom_class1(kanjiclass1);
#if defined(_FM77AV_VARIANTS)
        mainio->set_context_kanjirom_class2(kanjiclass2);
#endif	
	drec->set_context_out(mainio, FM7_MAINIO_CMT_RECV, 0xffffffff);
	//drec->set_context_remote(mainio, FM7_MAINIO_CMT_REMOTE, 0xffffffff);
  
	display->set_context_mainio(mainio);
	display->set_context_subcpu(subcpu);
        display->set_context_kanjiclass1(kanjiclass1);
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
        display->set_context_kanjiclass2(kanjiclass2);
#endif   
	// Palette, VSYNC, HSYNC, Multi-page, display mode. 
	mainio->set_context_display(display);
	
	//FDC
	fdc->set_context_irq(mainio, FM7_MAINIO_FDC_IRQ, 0xffffffff);
	fdc->set_context_drq(mainio, FM7_MAINIO_FDC_DRQ, 0xffffffff);
	mainio->set_context_fdc(fdc);
	// SOUND
	mainio->set_context_beep(beep);
	event->set_context_sound(beep);
	
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
	if(psg != NULL) event->set_context_sound(psg);
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
}  

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
	//update_dipswitch();
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
