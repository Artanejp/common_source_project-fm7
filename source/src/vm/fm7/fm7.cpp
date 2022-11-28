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
#include "../prnfile.h"

#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#include "../datarec.h"
#include "../disk.h"

#include "../mc6809.h"
#include "../z80.h"
#include "../mb8877.h"
#include "../noise.h"

#include "../pcm1bit.h"
#include "../ym2203.h"
#include "../ay_3_891x.h"
#include "../and.h"
#include "../or.h"
#include "../i8251.h"

#if defined(_FM77AV_VARIANTS)
#include "mb61vh010.h"
#include "../beep.h"
#endif
#if defined(HAS_DMA)
#include "hd6844.h"
#endif
#if defined(_FM77L4)
#include "../hd46505.h"
#endif

#if defined(_FM8)
#include "./bubblecasette.h"
#endif
#if defined(_FM8)
#include "./fm8_mainio.h"
#else
#include "./fm7_mainio.h"
#endif
#include "./fm7_mainmem.h"
#include "./fm7_display.h"
#include "./fm7_keyboard.h"
#include "./joystick.h"

#include "./kanjirom.h"
#if defined(CAPABLE_JCOMMCARD)
#include "./jcommcard.h"
#endif

VM::VM(EMU* parent_emu): VM_TEMPLATE(parent_emu)
{
	
	first_device = last_device = NULL;
#if defined(_FM8)
	psg = NULL;
#else	
# if defined(_FM77AV_VARIANTS)
	opn[0] = opn[1] = opn[2] = NULL;
# else   
	opn[0] = opn[1] = opn[2] = NULL;
	psg = NULL; 
# endif
#endif
	for(int i = 0; i < 3; i++) uart[i] = NULL;
	
	dummy = new DEVICE(this, emu);	// must be 1st device
	event = new EVENT(this, emu);	// must be 2nd device

	//dummycpu = new DEVICE(this, emu);
	maincpu = new MC6809(this, emu);
	subcpu = new MC6809(this, emu);
	g_substat_display = new AND(this, emu);
	g_substat_mainhalt = new AND(this, emu);

	
#ifdef WITH_Z80
	if((config.dipswitch & FM7_DIPSW_Z80CARD_ON) != 0) {
		z80cpu = new Z80(this, emu);
		g_mainstat = new AND(this, emu);
		g_intr = new OR(this, emu);
		
		g_intr_irq = new AND(this, emu);
		g_intr_firq = new AND(this, emu);
		g_nmi = new AND(this, emu);
	} else {
		z80cpu = NULL;
		g_mainstat = NULL;
		g_intr = NULL;

		g_intr_irq = NULL;
		g_intr_firq = NULL;
		g_nmi = NULL;
	}
#endif
#if defined(CAPABLE_JCOMMCARD)
	if((config.dipswitch & FM7_DIPSW_JSUBCARD_ON) != 0) {
		jsubcpu = new MC6809(this, emu);
		jcommcard = new FM7_JCOMMCARD(this, emu);
		g_jsubhalt = new AND(this, emu);
	} else {
		jsubcpu = NULL;
		jcommcard = NULL;
		g_jsubhalt = NULL;
	}
#endif
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV20) \
	|| defined(_FM77AV20EX) || defined(_FM8)
	uart[0] = new I8251(this, emu);
# else
#  if defined(CAPABLE_JCOMMCARD)
	if((config.dipswitch & FM7_DIPSW_JSUBCARD_ON) != 0) uart[0] = new I8251(this, emu);
#  endif
	if(((config.dipswitch & FM7_DIPSW_RS232C_ON) != 0) && (uart[0] == NULL)) uart[0] = new I8251(this, emu);
# endif
	if((config.dipswitch & FM7_DIPSW_MODEM_ON) != 0) uart[1] = new I8251(this, emu);
	if((config.dipswitch & FM7_DIPSW_MIDI_ON) != 0) uart[2] = new I8251(this, emu);

		
#if defined(_FM77AV_VARIANTS)
	alu = new MB61VH010(this, emu);
	keyboard_beep = new BEEP(this, emu);
#endif	
	// basic devices
	// I/Os
#if defined(_FM8)
#  if defined(USE_AY_3_8910_AS_PSG)
	psg = new AY_3_891X(this, emu);
#  else
	psg = new YM2203(this, emu);
#  endif
#  ifdef USE_DEBUGGER
	psg->set_context_debugger(new DEBUGGER(this, emu));
#  endif
#else	
	opn[0] = new YM2203(this, emu); // OPN
	opn[1] = new YM2203(this, emu); // WHG
	opn[2] = new YM2203(this, emu); // THG
#  ifdef USE_DEBUGGER
	opn[0]->set_context_debugger(new DEBUGGER(this, emu));
	opn[1]->set_context_debugger(new DEBUGGER(this, emu));
	opn[2]->set_context_debugger(new DEBUGGER(this, emu));
#  endif
# if !defined(_FM77AV_VARIANTS)
#  if defined(USE_AY_3_8910_AS_PSG)
	psg = new AY_3_891X(this, emu);
#  else
	psg = new YM2203(this, emu);
#  endif
#  ifdef USE_DEBUGGER
	psg->set_context_debugger(new DEBUGGER(this, emu));
#  endif
# endif	
#endif

#if defined(HAS_DMA)
	dmac = new HD6844(this, emu);
#endif   
#if defined(_FM8)
	for(int i = 0; i < 2; i++) bubble_casette[i] = new BUBBLECASETTE(this, emu);
#endif
	drec = NULL;
	drec = new DATAREC(this, emu);
	drec->set_context_noise_play(new NOISE(this, emu));
	drec->set_context_noise_stop(new NOISE(this, emu));
	drec->set_context_noise_fast(new NOISE(this, emu));
	pcm1bit = new PCM1BIT(this, emu);

	connect_320kfdc = connect_1Mfdc = false;
	fdc = NULL;
#if defined(HAS_2HD)
	fdc_2HD = NULL;
#endif

#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	if((config.dipswitch & FM7_DIPSW_CONNECT_320KFDC) != 0) {
		fdc = new MB8877(this, emu);
		fdc->set_context_noise_seek(new NOISE(this, emu));
		fdc->set_context_noise_head_down(new NOISE(this, emu));
		fdc->set_context_noise_head_up(new NOISE(this, emu));
		connect_320kfdc = true;
	}
#else
	{
		fdc = new MB8877(this, emu);
		fdc->set_context_noise_seek(new NOISE(this, emu));
		fdc->set_context_noise_head_down(new NOISE(this, emu));
		fdc->set_context_noise_head_up(new NOISE(this, emu));
		connect_320kfdc = true;
	}
#endif		
#if defined(HAS_2HD)
	if((config.dipswitch & FM7_DIPSW_CONNECT_1MFDC) != 0) {
		fdc_2HD = new MB8877(this, emu);
		fdc_2HD->set_context_noise_seek(new NOISE(this, emu));
		fdc_2HD->set_context_noise_head_down(new NOISE(this, emu));
		fdc_2HD->set_context_noise_head_up(new NOISE(this, emu));
		connect_1Mfdc = true;
	}
#endif
	
	joystick  = new JOYSTICK(this, emu);
	printer = new PRNFILE(this, emu);
#if defined(_FM77L4)
	l4crtc = new HD46505(this, emu);;
#endif

#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	if((config.dipswitch & FM7_DIPSW_CONNECT_KANJIROM) != 0) {
		kanjiclass1 = new KANJIROM(this, emu, false);
	} else {
		kanjiclass1 = NULL;
	}
#else
	kanjiclass1 = new KANJIROM(this, emu, false);
#endif	
#ifdef CAPABLE_KANJI_CLASS2
	kanjiclass2 = new KANJIROM(this, emu, true);
#endif

	keyboard = new KEYBOARD(this, emu);
	//display = new DISPLAY(this, emu);
#if defined(_FM8)
	mainio  = new FM8_MAINIO(this, emu);
#else
	mainio  = new FM7_MAINIO(this, emu);
#endif
	mainmem = new FM7_MAINMEM(this, emu);
	display = new DISPLAY(this, emu);


# if defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV20EX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	g_rs232c_dtr = new AND(this, emu);
	g_rs232c_dtr->set_mask(SIG_AND_BIT_0);
	g_rs232c_dtr->set_mask(SIG_AND_BIT_1);

	// DCD
#endif
#ifdef WITH_Z80
	if(g_mainstat != NULL) {
		g_mainstat->set_mask(SIG_AND_BIT_0);
		g_mainstat->set_mask(SIG_AND_BIT_1);
		maincpu->set_context_bus_ba(g_mainstat, SIG_AND_BIT_0, 0xffffffff);
		maincpu->set_context_bus_bs(g_mainstat, SIG_AND_BIT_1, 0xffffffff);
		g_mainstat->set_context_out(mainio, FM7_MAINIO_RUN_Z80, 0xffffffff);
	}
	if(z80cpu != NULL) {
		z80cpu->set_context_busack(mainio, FM7_MAINIO_RUN_6809, 0xffffffff);
		mainio->set_context_z80cpu(z80cpu);
	}
#endif
#if defined(_USE_QT)
	event->set_device_name(_T("EVENT"));
	dummy->set_device_name(_T("1st Dummy"));
	
	maincpu->set_device_name(_T("MAINCPU(MC6809B)"));
	subcpu->set_device_name(_T("SUBCPU(MC6809B)"));
#if defined(CAPABLE_JCOMMCARD)
	if(jsubcpu != NULL) {
		jsubcpu->set_device_name(_T("J.COMM BOARD CPU(MC6809)"));
	}
	if(jcommcard != NULL) {
		jcommcard->set_device_name(_T("Japanese COMM BOARD"));
	}
	if(g_jsubhalt != NULL) {
		g_jsubhalt->set_device_name(_T("J.COMM BOARD HALT(MC6809)"));
	}
# endif
# ifdef WITH_Z80
	if(z80cpu != NULL) z80cpu->set_device_name(_T("Z80 CPU BOARD"));
# endif
	if(fdc != NULL) fdc->set_device_name(_T("MB8877 FDC(320KB)"));
#if defined(HAS_2HD)
	if(fdc_2HD != NULL) fdc_2HD->set_device_name(_T("MB8877 FDC(1MB/2HD)"));
#endif	
	if(uart[0] != NULL) {
		uart[0]->set_device_name(_T("RS-232C BOARD(I8251 SIO)"));
	}
# if defined(CAPABLE_JCOMMCARD)
	if((config.dipswitch & FM7_DIPSW_JSUBCARD_ON) != 0) {
		if(uart[0] != NULL) uart[0]->set_device_name(_T("J.COMM BOARD RS-232C(I8251 SIO)"));
	}
# elif defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV20EX) || \
	defined(_FM77AV40EX) || defined(_FM77AV40SX) || defined(_FM8)
	if(uart[0] != NULL) uart[0]->set_device_name(_T("RS-232C(I8251 SIO)"));
# endif
		
	if(uart[1] != NULL) {
		uart[1]->set_device_name(_T("MODEM BOARD(I8251 SIO)"));
	}
	if(uart[2] != NULL) {
		uart[2]->set_device_name(_T("MIDI BOARD(I8251 SIO)"));
	}
						
	// basic devices
	// I/Os
# if defined(_FM8)
	psg->set_device_name(_T("AY-3-8910 PSG"));
# else	
	opn[0]->set_device_name(_T("YM2203 OPN"));
	opn[1]->set_device_name(_T("YM2203 WHG"));
	opn[2]->set_device_name(_T("YM2203 THG"));
#  if !defined(_FM77AV_VARIANTS)
	psg->set_device_name(_T("AY-3-8910 PSG"));
#  endif
	pcm1bit->set_device_name(_T("BEEP"));
	printer->set_device_name(_T("PRINTER I/F"));
# if defined(_FM77AV_VARIANTS)
	keyboard_beep->set_device_name(_T("BEEP(KEYBOARD)"));
# endif	
	if(kanjiclass1 != NULL) kanjiclass1->set_device_name(_T("KANJI ROM CLASS1"));
# ifdef CAPABLE_KANJI_CLASS2
	if(kanjiclass2 != NULL) kanjiclass2->set_device_name(_T("KANJI ROM CLASS2"));
# endif
# if defined(_FM8)
	bubble_casette[0]->set_device_name(_T("BUBBLE CASETTE #0"));
	bubble_casette[1]->set_device_name(_T("BUBBLE CASETTE #1"));
# endif	
#endif
# if defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV20EX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	g_rs232c_dtr->set_device_name(_T("RS232C DTR(AND)"));
#endif
#if defined(_FM77L4)
	l4crtc->set_device_name(_T("CRTC OF 400LINES BOARD"));
#endif
#ifdef WITH_Z80
	if(g_intr != NULL) g_intr->set_device_name(_T("Z80 INTR(OR)"));
	if(g_mainstat != NULL) g_mainstat->set_device_name(_T("Z80 HALT/RUN(AND)"));
	if(g_intr_irq != NULL) g_intr_irq->set_device_name(_T("Z80 IRQ(AND)"));
	if(g_intr_firq != NULL) g_intr_firq->set_device_name(_T("Z80 FIRQ(AND)"));
	if(g_nmi != NULL) g_nmi->set_device_name(_T("Z80 NMI(AND)"));
#endif
	g_substat_display->set_device_name(_T("DISPLAY STATUS(AND)"));
	g_substat_mainhalt->set_device_name(_T("SUBSYSTEM HALT STATUS(AND)"));
#endif	
	this->connect_bus();
	
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

void VM::connect_bus(void)
{
	uint32_t mainclock;
	uint32_t subclock;

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
	event->set_frames_per_sec(FRAMES_PER_SEC);
	event->set_lines_per_frame(LINES_PER_FRAME);
	mainclock = CPU_CLOCKS; // Hack
	subclock = SUBCLOCK_NORMAL;

	event->set_context_cpu(maincpu, mainclock);
	event->set_context_cpu(subcpu,  subclock);	
   
#ifdef WITH_Z80
	if(z80cpu != NULL) {
		event->set_context_cpu(z80cpu,  4000000);
		z80cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);

		g_intr_irq->set_mask(SIG_AND_BIT_0);
		g_intr_irq->set_mask(SIG_AND_BIT_1);
		
		g_intr_firq->set_mask(SIG_AND_BIT_0);
		g_intr_firq->set_mask(SIG_AND_BIT_1);
	
		g_nmi->set_mask(SIG_AND_BIT_0);
		g_nmi->set_mask(SIG_AND_BIT_1);

		mainio->set_context_irq(g_intr_irq, SIG_AND_BIT_1, 0xffffffff);
		g_intr_irq->set_context_out(g_intr, SIG_OR_BIT_0, 0xffffffff);
	
		mainio->set_context_firq(g_intr_firq, SIG_AND_BIT_1, 0xffffffff);
		g_intr_firq->set_context_out(g_intr, SIG_OR_BIT_0, 0xffffffff);
		
		g_intr->set_context_out(z80cpu, SIG_CPU_IRQ, 0xffffffff);

		mainio->set_context_nmi(g_nmi, SIG_AND_BIT_1, 0xffffffff);
		g_nmi->set_context_out(z80cpu, SIG_CPU_NMI, 0xffffffff);
	}
	maincpu->write_signal(SIG_CPU_HALTREQ, 0, 1);
#endif
#if defined(CAPABLE_JCOMMCARD)
	if((jsubcpu != NULL) && (jcommcard != NULL)) {
		event->set_context_cpu(jsubcpu,  JCOMMCARD_CLOCK);
		jcommcard->set_context_cpu(jsubcpu);
		if(g_jsubhalt != NULL) {
			g_jsubhalt->set_mask(SIG_AND_BIT_0);
			g_jsubhalt->set_mask(SIG_AND_BIT_1);
		
			jsubcpu->set_context_bus_ba(g_jsubhalt, SIG_AND_BIT_0, 0xffffffff);
			jsubcpu->set_context_bus_bs(g_jsubhalt, SIG_AND_BIT_1, 0xffffffff);
			g_jsubhalt->set_context_out(jcommcard, FM7_JCOMMCARD_BUS_HALT, 0xffffffff);
			mainio->set_context_jcommcard(jcommcard);
		}
	}
#endif
	event->set_context_sound(pcm1bit);
#if defined(_FM8)
	event->set_context_sound(psg);
	if(drec != NULL) event->set_context_sound(drec);
#else
	event->set_context_sound(opn[0]);
	event->set_context_sound(opn[1]);
	event->set_context_sound(opn[2]);
# if !defined(_FM77AV_VARIANTS)
	event->set_context_sound(psg);
# endif
	event->set_context_sound(drec);
	if(fdc != NULL) {
		event->set_context_sound(fdc->get_context_noise_seek());
		event->set_context_sound(fdc->get_context_noise_head_down());
		event->set_context_sound(fdc->get_context_noise_head_up());
	}
#if defined(HAS_2HD)
	if(fdc_2HD != NULL) {
		event->set_context_sound(fdc_2HD->get_context_noise_seek());
		event->set_context_sound(fdc_2HD->get_context_noise_head_down());
		event->set_context_sound(fdc_2HD->get_context_noise_head_up());
	}
#endif
	if(drec != NULL) {
		event->set_context_sound(drec->get_context_noise_play());
		event->set_context_sound(drec->get_context_noise_stop());
		event->set_context_sound(drec->get_context_noise_fast());
	}
# if defined(_FM77AV_VARIANTS)
	event->set_context_sound(keyboard_beep);
# endif
#endif   
#if !defined(_FM77AV_VARIANTS) && !defined(_FM77L4)
	//event->register_vline_event(display);
	//event->register_frame_event(display);
#endif	
	mainio->set_context_maincpu(maincpu);
	mainio->set_context_subcpu(subcpu);
	
	mainio->set_context_display(display);
	mainio->set_context_irq(maincpu, SIG_CPU_IRQ, 0xffffffff);
	mainio->set_context_firq(maincpu, SIG_CPU_FIRQ, 0xffffffff);
	mainio->set_context_nmi(maincpu, SIG_CPU_NMI, 0xffffffff);
#if defined(_FM77AV) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
	if((config.dipswitch & FM7_DIPSW_RS232C_ON) != 0) 	mainio->set_context_uart(0, uart[0]); /* $FD06- : RS232C */
#else
	mainio->set_context_uart(0, uart[0]);
#endif
	mainio->set_context_uart(1, uart[1]); /* $FD40- : MODEM */
	mainio->set_context_uart(2, uart[2]); /* $FDEA- : MIDI */

#if defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV20EX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	mainio->set_context_rs232c_dtr(g_rs232c_dtr);
	if(uart[0] != NULL) uart[0]->set_context_dtr(g_rs232c_dtr, SIG_AND_BIT_1, 0xffffffff);
#endif
	if(uart[0] != NULL) {
		uart[0]->set_context_rxrdy(mainio, FM7_MAINIO_UART0_RXRDY, 0xffffffff);
		uart[0]->set_context_txrdy(mainio, FM7_MAINIO_UART0_TXRDY, 0xffffffff);
		uart[0]->set_context_syndet(mainio, FM7_MAINIO_UART0_SYNDET, 0xffffffff);
	}
	if(uart[1] != NULL) {
		uart[1]->set_context_rxrdy(mainio, FM7_MAINIO_MODEM_RXRDY, 0xffffffff);
		uart[1]->set_context_txrdy(mainio, FM7_MAINIO_MODEM_TXRDY, 0xffffffff);
		uart[1]->set_context_syndet(mainio, FM7_MAINIO_MODEM_SYNDET, 0xffffffff);
	}
	if(uart[2] != NULL) {
		uart[2]->set_context_rxrdy(mainio, FM7_MAINIO_MIDI_RXRDY, 0xffffffff);
		uart[2]->set_context_txrdy(mainio, FM7_MAINIO_MIDI_TXRDY, 0xffffffff);
		uart[2]->set_context_syndet(mainio, FM7_MAINIO_MIDI_SYNDET, 0xffffffff);
	}
	
#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	if((config.dipswitch & FM7_DIPSW_CONNECT_KANJIROM) != 0) {
		mainio->set_context_kanjirom_class1(kanjiclass1);
	}
#else
	mainio->set_context_kanjirom_class1(kanjiclass1);
#endif	
	mainio->set_context_mainmem(mainmem);
	mainio->set_context_keyboard(keyboard);
	mainio->set_context_printer(printer);
	mainio->set_context_printer_reset(printer, SIG_PRINTER_RESET, 0xffffffff);
	mainio->set_context_printer_strobe(printer, SIG_PRINTER_STROBE, 0xffffffff);
	mainio->set_context_printer_select(printer, SIG_PRINTER_SELECT, 0xffffffff);
#if defined(CAPABLE_KANJI_CLASS2)
	mainio->set_context_kanjirom_class2(kanjiclass2);
#endif
#if defined(_FM8)
	for(int i = 0; i < 2; i++) mainio->set_context_bubble(bubble_casette[i], i);
#endif	
	keyboard->set_context_break_line(mainio, FM7_MAINIO_PUSH_BREAK, 0xffffffff);
	keyboard->set_context_int_line(mainio, FM7_MAINIO_KEYBOARDIRQ, 0xffffffff);
	keyboard->set_context_int_line(display, SIG_FM7_SUB_KEY_FIRQ, 0xffffffff);
#if defined(_FM77AV_VARIANTS)
	keyboard->set_context_beep(keyboard_beep);
#endif	
	keyboard->set_context_rxrdy(display, SIG_FM7KEY_RXRDY, 0x01);
	keyboard->set_context_key_ack(display, SIG_FM7KEY_ACK, 0x01);
   
	if(drec != NULL) {
		drec->set_context_ear(mainio, FM7_MAINIO_CMT_RECV, 0xffffffff);
		//drec->set_context_remote(mainio, FM7_MAINIO_CMT_REMOTE, 0xffffffff);
		mainio->set_context_datarec(drec);
	}
	mainmem->set_context_mainio(mainio);
	mainmem->set_context_display(display);
	mainmem->set_context_maincpu(maincpu);
#if defined(CAPABLE_DICTROM)
	mainmem->set_context_kanjirom_class1(kanjiclass1);
#endif  
	display->set_context_mainio(mainio);
	display->set_context_subcpu(subcpu);
	display->set_context_keyboard(keyboard);
#if defined(_FM77L4)
	display->set_context_l4crtc(l4crtc);
#endif
	
	mainio->set_context_clock_status(mainmem, FM7_MAINIO_CLOCKMODE, 0xffffffff);
	mainio->set_context_clock_status(display, SIG_DISPLAY_CLOCK, 0xffffffff);

	g_substat_display->set_mask(SIG_AND_BIT_0);
	g_substat_display->set_mask(SIG_AND_BIT_1);
	subcpu->set_context_bus_ba(g_substat_display, SIG_AND_BIT_0, 0xffffffff);
	subcpu->set_context_bus_bs(g_substat_display, SIG_AND_BIT_1, 0xffffffff);
	g_substat_display->set_context_out(display, SIG_FM7_SUB_HALT, 0xffffffff);

	g_substat_mainhalt->set_mask(SIG_AND_BIT_0);
	g_substat_mainhalt->set_mask(SIG_AND_BIT_1);
	subcpu->set_context_bus_ba(g_substat_mainhalt, SIG_AND_BIT_0, 0xffffffff);
	subcpu->set_context_bus_bs(g_substat_mainhalt, SIG_AND_BIT_1, 0xffffffff);
	g_substat_mainhalt->set_context_out(mainmem, SIG_FM7_SUB_HALT, 0xffffffff);

#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	display->set_context_kanjiclass1(kanjiclass1);
#endif	
#if defined(CAPABLE_KANJI_CLASS2)
	display->set_context_kanjiclass2(kanjiclass2);
#endif   
#if defined(_FM77AV_VARIANTS)
	display->set_context_alu(alu);
	alu->set_context_memory(display);
	alu->set_direct_access_offset(DISPLAY_VRAM_DIRECT_ACCESS);
#endif	
	// Palette, VSYNC, HSYNC, Multi-page, display mode. 
	mainio->set_context_display(display);
 
#if defined(_FM8) || (_FM7) || (_FMNEW7)
	if(connect_320kfdc) {
#endif
		if(fdc != NULL) {
			//FDC
			fdc->set_context_irq(mainio, FM7_MAINIO_FDC_IRQ, 0x1);
			fdc->set_context_drq(mainio, FM7_MAINIO_FDC_DRQ, 0x1);
			mainio->set_context_fdc(fdc);
		}
#if defined(_FM8) || (_FM7) || (_FMNEW7)
	}
#endif	
#if defined(HAS_2HD)
	if(connect_1Mfdc && (fdc_2HD != NULL)) {
		//FDC
		fdc_2HD->set_context_irq(mainio, FM7_MAINIO_FDC_IRQ_2HD, 0x1);
		fdc_2HD->set_context_drq(mainio, FM7_MAINIO_FDC_DRQ_2HD, 0x1);
		mainio->set_context_fdc_2HD(fdc_2HD);
	}
#endif	
	// SOUND
	mainio->set_context_beep(pcm1bit);
#if defined(_FM8)	
	mainio->set_context_psg(psg);
#else
# if !defined(_FM77AV_VARIANTS)
	mainio->set_context_psg(psg);
# endif
	opn[0]->set_context_irq(mainio, FM7_MAINIO_OPN_IRQ, 0xffffffff);
	mainio->set_context_opn(opn[0], 0);
	joystick->set_context_opn(opn[0]);
	mainio->set_context_joystick(joystick);
	opn[0]->set_context_port_b(joystick, FM7_JOYSTICK_MOUSE_STROBE, 0xff, 0);
	
	opn[1]->set_context_irq(mainio, FM7_MAINIO_WHG_IRQ, 0xffffffff);
	mainio->set_context_opn(opn[1], 1);
	opn[2]->set_context_irq(mainio, FM7_MAINIO_THG_IRQ, 0xffffffff);
	mainio->set_context_opn(opn[2], 2);
#endif   
	//subcpu->set_context_bus_clr(display, SIG_FM7_SUB_USE_CLR, 0x0000000f);
   
	event->register_frame_event(joystick);
#if defined(HAS_DMA)
	dmac->set_context_src(fdc, 0);
	dmac->set_context_dst(mainmem, 0);
	dmac->set_context_int_line(mainio, FM7_MAINIO_DMA_INT, 0xffffffff);
	dmac->set_context_busreq_line(maincpu, 1, SIG_CPU_BUSREQ, 0xffffffff);
	mainio->set_context_dmac(dmac);
#endif
	
	// MEMORIES must set before initialize().
	//dummycpu->set_context_mem(dummy);
	maincpu->set_context_mem(mainmem);
	subcpu->set_context_mem(display);
	//dummycpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
#ifdef WITH_Z80
	if(z80cpu != NULL) z80cpu->set_context_mem(mainmem);
#endif
#if defined(CAPABLE_JCOMMCARD)
	if((jsubcpu != NULL) && (jcommcard != NULL)) {
		jsubcpu->set_context_mem(jcommcard);
	}
#endif
#ifdef USE_DEBUGGER
	maincpu->set_context_debugger(new DEBUGGER(this, emu));
	subcpu->set_context_debugger(new DEBUGGER(this, emu));
# ifdef WITH_Z80
	if(z80cpu != NULL) z80cpu->set_context_debugger(new DEBUGGER(this, emu));
# endif
# if defined(CAPABLE_JCOMMCARD)
	if(jsubcpu != NULL) {
		jsubcpu->set_context_debugger(new DEBUGGER(this, emu));
	}
# endif
#endif

#if defined(__GIT_REPO_VERSION)
	strncpy(_git_revision, __GIT_REPO_VERSION, sizeof(_git_revision) - 1);
#endif
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->initialize();
	}

#if defined(WITH_Z80)
	if(g_intr_irq != NULL) g_intr_irq->write_signal(SIG_AND_BIT_0, ((config.dipswitch & FM7_DIPSW_Z80_IRQ_ON) != 0) ? 1 : 0, 1);
	if(g_intr_firq != NULL) g_intr_firq->write_signal(SIG_AND_BIT_0, ((config.dipswitch & FM7_DIPSW_Z80_FIRQ_ON) != 0) ? 1 : 0, 1);
	if(g_nmi != NULL) g_nmi->write_signal(SIG_AND_BIT_0, ((config.dipswitch & FM7_DIPSW_Z80_NMI_ON) != 0) ? 1 : 0, 1);
#endif
	// Disks
#if defined(_FM8) || (_FM7) || (_FMNEW7)
	if(connect_320kfdc) {
#endif
		if(fdc != NULL) {
			for(int i = 0; i < 4; i++) {
#if defined(_FM77AV20) || defined(_FM77AV20EX) || \
	defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
				fdc->set_drive_type(i, DRIVE_TYPE_2DD);
#else
				fdc->set_drive_type(i, DRIVE_TYPE_2D);
#endif
				fdc->set_drive_mfm(i, true);
				fdc->set_drive_rpm(i, 300);
			}
		}
#if defined(_FM8) || (_FM7) || (_FMNEW7)
	}
#endif	
	
#if defined(HAS_2HD)
	if(connect_1Mfdc && (fdc_2HD != NULL)) {
// ToDo: Implement another FDC for 1MB (2HD or 8''), this is used by FM-8 to FM-77? Not FM77AV or later? I still know this.
		for(int i = 0; i < 2; i++) {
			fdc_2HD->set_drive_type(i, DRIVE_TYPE_2HD);
			fdc_2HD->set_drive_mfm(i, true);
			fdc_2HD->set_drive_rpm(i, 300);
		}
	}
#endif	
}  

void VM::update_config()
{
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->update_config();
	}
	update_dipswitch();
}

void VM::reset()
{
	// reset all devices
	for(DEVICE* device = first_device; device; device = device->next_device) {
		device->reset();
	}
#if !defined(_FM77AV_VARIANTS) || defined(_FM8)
# if defined(USE_AY_3_8910_AS_PSG)
	psg->set_reg(0x2e, 0);	// set prescaler
	psg->write_signal(SIG_AY_3_891X_MUTE, 0x00, 0x01); // Okay?
# else	
	psg->set_reg(0x27, 0); // stop timer
	psg->set_reg(0x2e, 0);	// set prescaler
	psg->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?
#endif
#endif
#if !defined(_FM8)
	for(int i = 0; i < 3; i++) {
		opn[i]->set_reg(0x27, 0); // stop timer
		opn[i]->set_reg(0x2e, 0);	// set prescaler
		opn[i]->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?
	}
#endif
	//display->reset(); // 20180618 K.O for RELICS
}

void VM::special_reset()
{
	// BREAK + RESET
	mainio->reset();
	mainmem->reset();
	//mainio->write_signal(FM7_MAINIO_PUSH_BREAK, 1, 1);
	//keyboard->write_signal(SIG_FM7KEY_OVERRIDE_PRESS_BREAK, 0xffffffff, 0xffffffff);
	
#if defined(FM77AV_VARIANTS)	
	mainio->write_signal(FM7_MAINIO_HOT_RESET, 1, 1);
#endif	
	display->reset();
	subcpu->reset();
	maincpu->reset();
	mainio->write_signal(FM7_MAINIO_PUSH_BREAK, 1, 1);
	keyboard->write_signal(SIG_FM7KEY_OVERRIDE_PRESS_BREAK, 0xffffffff, 0xffffffff);
	event->register_event(mainio, EVENT_UP_BREAK, 1000.0 * 1000.0, false, NULL);
}

void VM::run()
{
	event->drive();
}

double VM::get_frame_rate()
{
	return event->get_frame_rate();
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
#if defined(WITH_Z80)
	else if(index == 2) {
# if defined(CAPABLE_JCOMMCARD)
		if(z80cpu == NULL) {
			return jsubcpu;
		}
# endif
		return z80cpu;
	}
# if defined(CAPABLE_JCOMMCARD)
	else if(index == 3) {
		return jsubcpu;
	}
# endif
#else
# if defined(CAPABLE_JCOMMCARD)
	else if(index == 2) {
		return jsubcpu;
	}
# endif
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

void VM::initialize_sound(int rate, int samples)
{
	// init sound manager
	event->initialize_sound(rate, samples);
	// init sound gen
#if defined(_FM8)
	psg->initialize_sound(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
#else	
	opn[0]->initialize_sound(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
	opn[1]->initialize_sound(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
	opn[2]->initialize_sound(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
# if !defined(_FM77AV_VARIANTS)   
	psg->initialize_sound(rate, (int)(4.9152 * 1000.0 * 1000.0 / 4.0), samples, 0, 0);
# endif
# if defined(_FM77AV_VARIANTS)
	keyboard_beep->initialize_sound(rate, 2400.0, 512);
# endif
#endif	
	pcm1bit->initialize_sound(rate, 6000);
	//drec->initialize_sound(rate, 0);
}

uint16_t* VM::create_sound(int* extra_frames)
{
	uint16_t* p = event->create_sound(extra_frames);
	return p;
}

int VM::get_sound_buffer_ptr()
{
	int pos = event->get_sound_buffer_ptr();
	return pos; 
}

#ifdef USE_SOUND_VOLUME
void VM::set_sound_device_volume(int ch, int decibel_l, int decibel_r)
{
#if !defined(_FM77AV_VARIANTS)
	if(ch-- == 0) {
		psg->set_volume(0, decibel_l, decibel_r);
		psg->set_volume(1, decibel_l, decibel_r);
	} else
#endif
#if !defined(_FM8)		
	if(ch-- == 0) {
		opn[0]->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		opn[0]->set_volume(1, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		opn[1]->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		opn[1]->set_volume(1, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		opn[2]->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		opn[2]->set_volume(1, decibel_l, decibel_r);
	} else
#endif	
	if(ch-- == 0) {
		pcm1bit->set_volume(0, decibel_l, decibel_r);
	} else if(ch-- == 0) {
		if(drec != NULL) drec->set_volume(0, decibel_l, decibel_r);
	}
#if defined(_FM77AV_VARIANTS)
	else if(ch-- == 0) {
		keyboard_beep->set_volume(0, decibel_l, decibel_r);
	}
#endif
	else if(ch-- == 0) {
		if(fdc != NULL) {
			fdc->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
			fdc->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
			fdc->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
		}
	} 
#if defined(HAS_2HD)
	else if(ch-- == 0) {
		if(fdc_2HD != NULL) {
			fdc_2HD->get_context_noise_seek()->set_volume(0, decibel_l, decibel_r);
			fdc_2HD->get_context_noise_head_down()->set_volume(0, decibel_l, decibel_r);
			fdc_2HD->get_context_noise_head_up()->set_volume(0, decibel_l, decibel_r);
		}
	} 
#endif
	else if(ch-- == 0) {
		if(drec != NULL) {
			drec->get_context_noise_play()->set_volume(0, decibel_l, decibel_r);
			drec->get_context_noise_stop()->set_volume(0, decibel_l, decibel_r);
			drec->get_context_noise_fast()->set_volume(0, decibel_l, decibel_r);
		}
	}
}
#endif

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

bool VM::get_caps_locked()
{
	return keyboard->get_caps_locked();
}

bool VM::get_kana_locked()
{
	return keyboard->get_kana_locked();
}

// Get INS status.Important with FM-7 series (^_^;
uint32_t VM::get_led_status()
{
	return keyboard->read_signal(SIG_FM7KEY_LED_STATUS);
}


// ----------------------------------------------------------------------------
// user interface
// ----------------------------------------------------------------------------

void VM::open_floppy_disk(int drv, const _TCHAR* file_path, int bank)
{
	if(drv < 0) return;
#if defined(HAS_2HD)
	if(drv < 2) {
		if(fdc != NULL) {
			fdc->open_disk(drv, file_path, bank);
		}
	} else {
		if(fdc_2HD != NULL) {
			fdc_2HD->open_disk(drv - 2, file_path, bank);
		}
	}
#else
	if(fdc != NULL) {
		fdc->open_disk(drv, file_path, bank);
	}
#endif
}

void VM::close_floppy_disk(int drv)
{
#if defined(HAS_2HD)
	if(drv < 2) {
		if(fdc != NULL) fdc->close_disk(drv);
	} else {
		if(fdc_2HD != NULL) fdc_2HD->close_disk(drv - 2);
	}		
#else
	if(fdc != NULL) {
		fdc->close_disk(drv);
	}
#endif
}

bool VM::is_floppy_disk_inserted(int drv)
{
#if defined(HAS_2HD)
	if((fdc != NULL) && (drv < 2)) {
		return fdc->is_disk_inserted(drv);
	} else if(fdc_2HD != NULL) {
		return fdc_2HD->is_disk_inserted(drv - 2);
	} else {
		return false;
	}
#else
	if(fdc != NULL) {
		return fdc->is_disk_inserted(drv);
	} else {
		return false;
	}
#endif
}

void VM::is_floppy_disk_protected(int drv, bool value)
{
#if defined(HAS_2HD)
	if((fdc != NULL) && (drv < 2)) {
		fdc->is_disk_protected(drv, value);
	} else if(fdc_2HD != NULL) {
		fdc_2HD->is_disk_protected(drv - 2, value);
	}		
#else
	if(fdc != NULL) {
		fdc->is_disk_protected(drv, value);
	}
#endif
}

bool VM::is_floppy_disk_protected(int drv)
{
#if defined(HAS_2HD)
	if((fdc != NULL) && (drv < 2)) {
		return fdc->is_disk_protected(drv);
	} else if(fdc_2HD != NULL) {
		return fdc_2HD->is_disk_protected(drv);
	} else {
		return false;
	}
#else
	if(fdc != NULL) {
		return fdc->is_disk_protected(drv);
	} else {
		return false;
	}
#endif
}

uint32_t VM::is_floppy_disk_accessed()
{
	uint32_t v = 0;
#if defined(HAS_2HD)
	uint32_t v1, v2;
	v1 = v2 = 0;
# if defined(_FM8) || (_FM7) || (_FMNEW7)
	if(connect_320kfdc) {
# endif
		if(fdc != NULL) v1 = fdc->read_signal(0);
# if defined(_FM8) || (_FM7) || (_FMNEW7)
	}
# endif
	if(connect_1Mfdc) {
		if(fdc_2HD != NULL) v2 = fdc_2HD->read_signal(0);
	}
	v1 = v1 & 0x03;
	v2 = (v2 & 0x03) << 2;
	v = v1 | v2;
	return v;
#else
# if defined(_FM8) || (_FM7) || (_FMNEW7)
	if(connect_320kfdc) {
# endif		
		v = fdc->read_signal(0);
# if defined(_FM8) || (_FM7) || (_FMNEW7)
	}
# endif
	return v;
#endif
}

void VM::play_tape(int drv, const _TCHAR* file_path)
{
	if(drec != NULL) {
		bool remote = drec->get_remote();
		
		if(drec->play_tape(file_path) && remote) {
			// if machine already sets remote on, start playing now
			push_play(drv);
		}
	}
}

void VM::rec_tape(int drv, const _TCHAR* file_path)
{
	if(drec != NULL) {
		bool remote = drec->get_remote();
		
		if(drec->rec_tape(file_path) && remote) {
			// if machine already sets remote on, start recording now
			push_play(drv);
		}
	}
}

void VM::close_tape(int drv)
{
	if(drec != NULL) {
		emu->lock_vm();
		drec->close_tape();
		emu->unlock_vm();
		drec->set_remote(false);
	}
}

bool VM::is_tape_inserted(int drv)
{
	if(drec != NULL) {
		return drec->is_tape_inserted();
	}
	return false;
}

bool VM::is_tape_playing(int drv)
{
	if(drec != NULL) {
		return drec->is_tape_playing();
	}
	return false;
}

bool VM::is_tape_recording(int drv)
{
	if(drec != NULL) {
		return drec->is_tape_recording();
	}
	return false;
}

int VM::get_tape_position(int drv)
{
	if(drec != NULL) {
		return drec->get_tape_position();
	}
	return 0;
}

const _TCHAR* VM::get_tape_message(int drv)
{
	if(drec != NULL) {
		return drec->get_message();
	}
	return NULL;
}

void VM::push_play(int drv)
{
	if(drec != NULL) {
		drec->set_remote(false);
		drec->set_ff_rew(0);
		drec->set_remote(true);
	}
}


void VM::push_stop(int drv)
{
	if(drec != NULL) {
		drec->set_remote(false);
	}
}

void VM::push_fast_forward(int drv)
{
	if(drec != NULL) {
		drec->set_remote(false);
		drec->set_ff_rew(1);
		drec->set_remote(true);
	}
}

void VM::push_fast_rewind(int drv)
{
	if(drec != NULL) {
		drec->set_remote(false);
		drec->set_ff_rew(-1);
		drec->set_remote(true);
	}
}

void VM::push_apss_forward(int drv)
{
	if(drec != NULL) {
		drec->do_apss(1);
	}
}

void VM::push_apss_rewind(int drv)
{
	if(drec != NULL) {
		drec->do_apss(-1);
	}
}

bool VM::is_frame_skippable()
{
	return event->is_frame_skippable();
}

void VM::update_dipswitch()
{
#if defined(WITH_Z80)
	if(g_intr_irq != NULL) g_intr_irq->write_signal(SIG_AND_BIT_0, ((config.dipswitch & FM7_DIPSW_Z80_IRQ_ON) != 0) ? 1 : 0, 1);
	if(g_intr_firq != NULL) g_intr_firq->write_signal(SIG_AND_BIT_0, ((config.dipswitch & FM7_DIPSW_Z80_FIRQ_ON) != 0) ? 1 : 0, 1);
	if(g_nmi != NULL) g_nmi->write_signal(SIG_AND_BIT_0, ((config.dipswitch & FM7_DIPSW_Z80_NMI_ON) != 0) ? 1 : 0, 1);
#endif
}

void VM::set_cpu_clock(DEVICE *cpu, uint32_t clocks) {
	event->set_secondary_cpu_clock(cpu, clocks);
}

#if defined(USE_BUBBLE)
void VM::open_bubble_casette(int drv, const _TCHAR *path, int bank)
{
	if((drv >= 2) || (drv < 0)) return;
	if(bubble_casette[drv] == NULL) return;
	bubble_casette[drv]->open((_TCHAR *)path, bank);
}

void VM::close_bubble_casette(int drv)
{
	if((drv >= 2) || (drv < 0)) return;
	if(bubble_casette[drv] == NULL) return;
	bubble_casette[drv]->close();
}

bool VM::is_bubble_casette_inserted(int drv)
{
	if((drv >= 2) || (drv < 0)) return false;
	if(bubble_casette[drv] == NULL) return false;
	return bubble_casette[drv]->is_bubble_inserted();
}

bool VM::is_bubble_casette_protected(int drv)
{
	if((drv >= 2) || (drv < 0)) return false;
	if(bubble_casette[drv] == NULL) return false;
	return bubble_casette[drv]->is_bubble_protected();
}

void VM::is_bubble_casette_protected(int drv, bool flag)
{
	if((drv >= 2) || (drv < 0)) return;
	if(bubble_casette[drv] == NULL) return;
	bubble_casette[drv]->set_bubble_protect(flag);
}
#endif

void VM::set_vm_frame_rate(double fps)
{
   if(event != NULL) event->set_frames_per_sec(fps);
}


#define STATE_VERSION	12

bool VM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	state_fio->StateValue(connect_320kfdc);
	state_fio->StateValue(connect_1Mfdc);
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
	if(loading) {
		update_config();
	}
	//mainio->restore_opn();

	return true;
}

#ifdef USE_DIG_RESOLUTION
void VM::get_screen_resolution(int *w, int *h)
{
	switch(display->get_screen_mode()) {
	case DISPLAY_MODE_8_200L:
		*w = 640;
		*h = 200;
		break;
	case DISPLAY_MODE_8_400L:
	case DISPLAY_MODE_1_400L:
		*w = 640;
		*h = 400;
		break;
	case DISPLAY_MODE_4096:
	case DISPLAY_MODE_256k:
		*w = 320;
		*h = 200;
		break;
	default:
		*w = 640;
		*h = 200;
		break;
	}
}
#endif

bool VM::is_screen_changed()
{
	bool f = true;
#if defined(USE_MINIMUM_RENDERING)
	f = display->screen_update();
	display->reset_screen_update();
#endif	
	return f;
}
