/*
 * FM-7 Main I/O [fm7_mainio.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jan 03, 2015 : Initial
 *
 */

#include "fm7.h"
#include "fm7_mainio.h"

#include "../mc6809.h"
#include "../z80.h"

#include "../datarec.h"
#include "../i8251.h"
#if defined(HAS_DMA)
#include "hd6844.h"
#endif
# if defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV20EX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
#include "../and.h"
#endif
#include "./fm7_mainmem.h"
#include "./fm7_display.h"
#include "./fm7_keyboard.h"
#include "./kanjirom.h"
#include "./joystick.h"
#if defined(CAPABLE_JCOMMCARD)
#include "./jcommcard.h"
#endif

FM7_MAINIO::FM7_MAINIO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
	int i;
	for(i = 0; i < 3; i++) {
		opn[i] = NULL;
	}
	for(i = 0; i < 3; i++) {
		uart[i] = NULL;
		uart_enabled[i] = false;
	}
# if defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV20EX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	rs232c_enabled = false;
	rs232c_dtr = NULL;
# else
	rs232c_enabled = true;
	rs232c_dcd = false;
# endif
# if !defined(_FM77AV_VARIANTS)
	psg = NULL;
#endif

	drec = NULL;
	pcm1bit = NULL;
	joystick = NULL;
	fdc = NULL;
# if defined(HAS_2HD)
 	fdc_2HD = NULL;
	event_2hd_nmi = -1;
	nmi_delay = 300;
# endif
	printer = NULL;
	
	kanjiclass1 = NULL;
	kanjiclass2 = NULL;

	display = NULL;
	keyboard = NULL;
	maincpu = NULL;
	mainmem = NULL;
	subcpu = NULL;
#ifdef WITH_Z80
	z80 = NULL;
#endif	
#if defined(CAPABLE_JCOMMCARD)
	jcommcard = NULL;
#endif
	// FD00
	clock_fast = true;
	lpt_strobe = false;  // bit6
	lpt_slctin = false;  // bit7
	// FD01
	lpt_outdata = 0x00;
	// FD02
	cmt_indat = true; // bit7
	cmt_invert = false; // Invert signal
	irqstat_reg0 = 0xff;
	
	// FD04
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX) 
	stat_kanjirom = true;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
#elif defined(_FM77_VARIANTS)
	stat_fdmode_2hd = false; //  R/W : bit6, '0' = 2HD, '1' = 2DD. FM-77 Only.
	stat_kanjirom = true;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
#endif	
	firq_break_key = false; // bit1, ON = '0'.
	firq_sub_attention = false; // bit0, ON = '0'.
	intmode_fdc = false; // bit2, '0' = normal, '1' = SFD.
#if defined(HAS_2HD)	
	drqstat_fdc_2hd = false;
	irqstat_fdc_2hd = false;
#endif
	// FD05
	extdet_neg = false;

	// FD0B
	// FD0D
	// FD0F
	// FD15/ FD46 / FD51
	connect_opn = false;
	connect_whg = false;
	connect_thg = false;
		
	for(i = 0; i < 3; i++) {
		opn_address[i] = 0x00;
		opn_data[i] = 0x00;
		opn_cmdreg[i] = 0;
		opn_prescaler_type[i] = 1;
	}
	// FD17
	mouse_enable = false;
	// FD18-FD1F
	connect_fdc = false;
	fdc_statreg = 0x00;
	fdc_cmdreg = 0x00;
	fdc_trackreg = 0x00;
	fdc_sectreg = 0x00;
	fdc_datareg = 0x00;
	fdc_headreg = 0x00;
	fdc_drvsel = 0x00;
	fdc_motor = false;
	// FD20, FD21, FD22, FD23
	connect_kanjiroml1 = false;
#if defined(_FM77AV_VARIANTS)
	// FD2C, FD2D, FD2E, FD2F
	connect_kanjiroml2 = false;
#endif		
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	boot_ram = false;
#endif

#if defined(HAS_DMA)
	dmac = NULL;
#endif	
#if defined(_FM77_VARIANTS) || defined(_FM8)
	bootmode = config.boot_mode & 7;
#else
	bootmode = config.boot_mode & 3;
#endif
	memset(io_w_latch, 0xff, 0x100);
	initialize_output_signals(&clock_status);
	initialize_output_signals(&printer_reset_bus);
	initialize_output_signals(&printer_strobe_bus);
	initialize_output_signals(&printer_select_bus);
	initialize_output_signals(&irq_bus);
	initialize_output_signals(&firq_bus);
	initialize_output_signals(&nmi_bus);
	
	set_device_name(_T("MAIN I/O"));
}

FM7_MAINIO::~FM7_MAINIO()
{
}


void FM7_MAINIO::initialize()
{
	event_beep = -1;
	event_beep_oneshot = -1;
	event_timerirq = -1;
	event_fdc_motor = -1;
	lpt_type = config.printer_type;
	fdc_cmdreg = 0x00;
#if defined(HAS_2HD)
	event_fdc_motor_2HD = -1;
#endif
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	boot_ram = false;
# if defined(_FM77_VARIANTS)
	stat_fdmode_2hd = false;
	stat_kanjirom = true;
# endif	
#endif
#if defined(_FM77AV_VARIANTS)
	reg_fd12 = 0xbc; // 0b10111100
#endif		
#if defined(_FM77_VARIANTS) || defined(_FM8)
	bootmode = config.boot_mode & 7;
#else
	bootmode = config.boot_mode & 3;
#endif
#if defined(HAS_2HD)
	event_2hd_nmi = -1;
	nmi_delay = 300;
#endif
	//reset_printer();
}

void FM7_MAINIO::reset()
{
	if(event_beep >= 0) cancel_event(this, event_beep);
	event_beep = -1;
	if(event_beep_oneshot >= 0) cancel_event(this, event_beep_oneshot);
	event_beep_oneshot = -1;
	if(event_timerirq >= 0) cancel_event(this, event_timerirq);
	beep_snd = true;
	beep_flag = false;
	register_event(this, EVENT_BEEP_CYCLE, (1000.0 * 1000.0) / (1200.0 * 2.0), true, &event_beep);
	// Sound
#if defined(HAS_2HD)
	drqstat_fdc_2hd = false;
	irqstat_fdc_2hd = false;
	if(event_2hd_nmi >= 0) cancel_event(this, event_2hd_nmi);
	event_2hd_nmi = -1;
#endif
#if defined(_FM77AV_VARIANTS)
	hotreset = false;
#endif
	// Around boot rom
#if defined(_FM77_VARIANTS)
	boot_ram = false;
#elif defined(_FM77AV_VARIANTS)
	boot_ram = true;
#endif
	// FD05
	sub_cancel = false; // bit6 : '1' Cancel req.
	sub_halt = false; // bit6 : '1' Cancel req.
	sub_cancel_bak = sub_cancel; // bit6 : '1' Cancel req.
	sub_halt_bak = sub_halt; // bit6 : '1' Cancel req.
	req_z80run = false; // OK?
	z80_run = false;

	// FD02
	cmt_indat = true; // bit7
	cmt_invert = false; // Invert signal
	lpt_det2 = true;
	lpt_det1 = true;
	lpt_type = config.printer_type;
	reset_printer();
	
# if defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV20EX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	rs232c_enabled = false;
# else
	rs232c_enabled = true;
# endif

#if defined(_FM77AV_VARIANTS)
	sub_monitor_type = 0x00;
#endif
	if(config.cpu_type == 0) {
		clock_fast = true;
	} else {
		clock_fast = false;
	}
	write_signals(&clock_status, clock_fast ? 0xffffffff : 0);
   
	// FD03
	irqmask_syndet = true;
	irqmask_rxrdy = true;
	irqmask_txrdy = true;
	irqmask_mfd = true;
	irqmask_timer = true;
	irqmask_printer = true;
	irqmask_keyboard = true;
	irqstat_reg0 = 0xff;
	
	// FD06,07
	intstat_syndet = false;
	intstat_rxrdy = false;
	intstat_txrdy = false;
	irqstat_timer = false;
	irqstat_printer = false;
	irqstat_keyboard = false;
  
	irqreq_syndet = false;
	irqreq_rxrdy = false;
	irqreq_txrdy = false;
	irqreq_printer = false;
	irqreq_keyboard = false;

	modem_irqmask_rxrdy = modem_irqmask_txrdy = true;
	modem_syndet = modem_rxrdy = modem_txrdy = false;
	midi_uart_irqmask = midi_syndet = midi_rxrdy = midi_txrdy = false;
	// FD00
	if(drec != NULL) {
		call_write_signal(drec, SIG_DATAREC_MIC, 0x00, 0x01);
		call_write_signal(drec, SIG_DATAREC_REMOTE, 0x00, 0x02);
	}
	reset_fdc();
	reset_sound();
	
	// FD04
	firq_break_key = (keyboard->read_signal(SIG_FM7KEY_BREAK_KEY) != 0x00000000); // bit1, ON = '0'.
	intmode_fdc = false;
	set_sub_attention(false);	
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX) 
	stat_kanjirom = true;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
#elif defined(_FM77_VARIANTS)
	stat_fdmode_2hd = false; //  R/W : bit6, '0' = 2HD, '1' = 2DD. FM-77 Only.
	stat_kanjirom = true;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
#endif
	write_signals(&firq_bus, 0);
	
#if defined(HAS_DMA)
	intstat_dma = false;
	dma_addr = 0;
#endif
#if defined(_FM77AV_VARIANTS)
	reg_fd12 = 0xbc; // 0b10111100
#endif
#if defined(WITH_Z80)
	if(z80 != NULL) call_write_signal(z80, SIG_CPU_BUSREQ, 0xffffffff, 0xffffffff);
#endif
	irqreg_fdc = 0xff; //0b11111111;
	irqstat_fdc = false;
	call_write_signal(maincpu, SIG_CPU_BUSREQ, 0, 0xffffffff);
	call_write_signal(maincpu, SIG_CPU_HALTREQ, 0, 0xffffffff);
	
	intstat_whg = false;
	intstat_thg = false;
	// FD17
	intstat_opn = false;
	intstat_mouse = false;
	do_irq();

//#if !defined(_FM8)
	register_event(this, EVENT_TIMERIRQ_ON, 10000.0 / 4.9152, true, &event_timerirq); // TIMER IRQ
//#endif
#if defined(_FM77_VARIANTS) || defined(_FM8)
	bootmode = config.boot_mode & 7;
#else
	bootmode = config.boot_mode & 3;
#endif
	memset(io_w_latch, 0xff, 0x100);
}

void FM7_MAINIO::reset_printer()
{
	lpt_slctin = false;
	lpt_strobe = false;
	// FD01
	lpt_outdata = 0x00;
	write_signals(&printer_strobe_bus, 0);
	write_signals(&printer_select_bus, 0xffffffff);
	write_signals(&printer_reset_bus, 0xffffffff);
	register_event(this, EVENT_PRINTER_RESET_COMPLETED, 5.0 * 1000.0, false, NULL);
	if(lpt_type == 0) {
		printer->write_signal(SIG_PRINTER_STROBE, 0x00, 0xff);
	}
	lpt_busy = false;
	lpt_error_inv = true;
	lpt_ackng_inv = true;
	lpt_pe = false;

}

void FM7_MAINIO::set_clockmode(uint8_t flags)
{
	bool f = clock_fast;
	if((flags & FM7_MAINCLOCK_SLOW) != 0) {
		clock_fast = false;
	} else {
		clock_fast = true;
	}
	if(f != clock_fast) {
		write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
	}
}

uint8_t FM7_MAINIO::get_clockmode(void)
{
	if(!clock_fast) return FM7_MAINCLOCK_SLOW;
	return FM7_MAINCLOCK_HIGH;
}


uint8_t FM7_MAINIO::get_port_fd00(void)
{
	uint8_t ret           = 0x7e; //0b01111110;
	if(keyboard->read_data8(0x00) != 0) ret |= 0x80; // High bit.
	if(clock_fast) ret |= 0x01; //0b00000001;
	return ret;
}
  
void FM7_MAINIO::set_port_fd00(uint8_t data)
{
	if(drec != NULL) {
		call_write_signal(drec, SIG_DATAREC_MIC, data, 0x01);
		call_write_signal(drec, SIG_DATAREC_REMOTE, data, 0x02);
	}	
	lpt_slctin = ((data & 0x80) == 0);
	lpt_strobe = ((data & 0x40) != 0);
	write_signals(&printer_strobe_bus, lpt_strobe ? 0xffffffff : 0);
	write_signals(&printer_select_bus, lpt_slctin ? 0xffffffff : 0);
	if((lpt_type == 0) && (lpt_slctin)) {
		printer->write_signal(SIG_PRINTER_STROBE, lpt_strobe ? 0xff : 0x00, 0xff);
	}
}
   
uint8_t FM7_MAINIO::get_port_fd02(void)
{
	uint8_t ret;
	bool ack_bak = lpt_ackng_inv;

	ret = (cmt_indat) ? 0xff : 0x7f; // CMT
	if(lpt_type == 0) {
		lpt_busy = (printer->read_signal(SIG_PRINTER_BUSY) != 0);
		lpt_error_inv = true;
		lpt_ackng_inv = (printer->read_signal(SIG_PRINTER_ACK) != 0);
		lpt_pe = false;
	} else if((lpt_type == 1) || (lpt_type == 2)) {
		lpt_pe = (joystick->read_data8(lpt_type + 1) != 0); // check joy port;
		lpt_busy = true;
		lpt_error_inv = true;
		lpt_ackng_inv = true;
	} else {
		lpt_busy = true;
		lpt_error_inv = true;
		lpt_ackng_inv = true;
		lpt_pe = true;
	}
	ret &= (lpt_busy)      ? 0xff : ~0x01;
	ret &= (lpt_error_inv) ? 0xff : ~0x02;
	ret &= (lpt_ackng_inv) ? 0xff : ~0x04;
	ret &= (lpt_pe)        ? 0xff : ~0x08;
	ret &= (lpt_det1)      ? 0xff : ~0x10;
	ret &= (lpt_det2)      ? 0xff : ~0x20;
	if((lpt_ackng_inv == true) && (ack_bak == false)) set_irq_printer(true);
	return ret;
}

void FM7_MAINIO::set_port_fd02(uint8_t val)
{
//#if !defined(_FM8)	
	irqmask_reg0 = val;
	bool syndetirq_bak = irqmask_syndet;
	//bool rxrdyirq_bak = irqmask_rxrdy;
	//bool txrdyirq_bak = irqmask_txrdy;
	
	bool keyirq_bak = irqmask_keyboard;
	//bool timerirq_bak = irqmask_timer;
	//bool printerirq_bak = irqmask_printer;
	//bool mfdirq_bak = irqmask_mfd;
	
	//	if((val & 0b00010000) != 0) {
	if((val & 0x80) != 0) {
		irqmask_syndet = false;
	} else {
		irqmask_syndet = true;
	}
	if(syndetirq_bak != irqmask_syndet) {
   		set_irq_txrdy(irqreq_syndet);
	}
	if((val & 0x40) != 0) {
		irqmask_rxrdy = false;
	} else {
		irqmask_rxrdy = true;
	}
//	if(rxrdyirq_bak != irqmask_rxrdy) {
//		set_irq_rxrdy(irqreq_rxrdy);
//	}
	if((val & 0x20) != 0) {
		irqmask_txrdy = false;
	} else {
		irqmask_txrdy = true;
	}
//	if(txrdyirq_bak != irqmask_txrdy) {
	// 		set_irq_txrdy(irqreq_txrdy);
//	}
	
	if((val & 0x10) != 0) {
		irqmask_mfd = false;
	} else {
		irqmask_mfd = true;
	}

	if((val & 0x04) != 0) {
		irqmask_timer = false;
	} else {
		irqmask_timer = true;
	}
//	if(timerirq_bak != irqmask_timer) {
// 		set_irq_timer(false);
//	}

	if((val & 0x02) != 0) {
		irqmask_printer = false;
	} else {
		irqmask_printer = true;
	}
//	if(printerirq_bak != irqmask_printer) {
//   		set_irq_printer(irqreq_printer);
//	}
   
	if((val & 0x01) != 0) {
		irqmask_keyboard = false;
	} else {
		irqmask_keyboard = true;
	}
	if(keyirq_bak != irqmask_keyboard) {
		call_write_signal(display, SIG_FM7_SUB_KEY_MASK, irqmask_keyboard ? 1 : 0, 1); 
		set_irq_keyboard(irqreq_keyboard);
	}
//#endif	
		return;
}

void FM7_MAINIO::set_irq_syndet(bool flag)
{
	bool backup = intstat_syndet;
	irqreq_syndet = flag;
	if(flag && !(irqmask_syndet)) {
		//irqstat_reg0 &= ~0x80; //~0x20;
		intstat_syndet = true;	   
	} else {
		//	irqstat_reg0 |= 0x80;
		intstat_syndet = false;	   
	}
	if(backup != intstat_syndet) do_irq();
}


void FM7_MAINIO::set_irq_rxrdy(bool flag)
{
	bool backup = intstat_rxrdy;
	irqreq_rxrdy = flag;
	if(flag && !(irqmask_rxrdy)) {
		//irqstat_reg0 &= ~0x40; //~0x20;
		intstat_rxrdy = true;	   
	} else {
		//irqstat_reg0 |= 0x40;
		intstat_rxrdy = false;	   
	}
	if(backup != intstat_rxrdy) do_irq();
}



void FM7_MAINIO::set_irq_txrdy(bool flag)
{
	bool backup = intstat_txrdy;
	irqreq_txrdy = flag;
	if(flag && !(irqmask_txrdy)) {
		//irqstat_reg0 &= ~0x20; //~0x20;
		intstat_txrdy = true;	   
	} else {
		//irqstat_reg0 |= 0x20;
		intstat_txrdy = false;	   
	}
	if(backup != intstat_txrdy) do_irq();
}


void FM7_MAINIO::set_irq_timer(bool flag)
{
	if(flag) {
		irqstat_reg0 &= 0xfb; //~0x04;
		irqstat_timer = true;	   
	} else {
		irqstat_reg0 |= 0x04;
		irqstat_timer = false;	   
	}
	do_irq();
}

void FM7_MAINIO::set_irq_printer(bool flag)
{
	irqreq_printer = flag;
	if(flag && !(irqmask_printer)) {
		irqstat_reg0 &= ~0x02;
		irqstat_printer = true;	   
	} else {
		irqstat_reg0 |= 0x02;
		irqstat_printer = false;	   
	}
	do_irq();
}

void FM7_MAINIO::set_irq_keyboard(bool flag)
{
	//uint8_t backup = irqstat_reg0;
   	//printf("MAIN: KEYBOARD: IRQ=%d MASK=%d\n", flag ,irqmask_keyboard);
	irqreq_keyboard = flag;
	if(flag && !irqmask_keyboard) {
		irqstat_reg0 &= 0xfe;
		irqstat_keyboard = true;
	} else {
		irqstat_reg0 |= 0x01;
		irqstat_keyboard = false;	   
	}
	do_irq();
}


void FM7_MAINIO::do_irq(void)
{
	bool intstat;
	uint32_t nval;
	
	intstat = irqstat_timer | irqstat_keyboard | irqstat_printer;
	intstat = intstat | intstat_opn | intstat_whg | intstat_thg;
   	intstat = intstat | intstat_txrdy | intstat_rxrdy | intstat_syndet;
	intstat = intstat | intstat_mouse;
	intstat = intstat | ((modem_irqmask_txrdy & modem_txrdy) | (modem_irqmask_rxrdy & modem_rxrdy) | modem_syndet);
	intstat = intstat | ((!midi_uart_irqmask) & (midi_syndet | midi_txrdy | midi_rxrdy));
# if defined(HAS_DMA)
	intstat = intstat | intstat_dma;
# endif
# if defined(HAS_2HD)
	if(intmode_fdc) {
		intstat = intstat | irqstat_fdc_2hd;
	} else {
		intstat = intstat | irqstat_fdc;
	}
# else
	intstat = intstat | irqstat_fdc;
#endif
	nval = (intstat) ? 0xffffffff : 0;
	write_signals(&irq_bus, nval);
}

void FM7_MAINIO::do_firq(void)
{
	bool firq_stat;
	uint32_t nval;
	firq_stat = firq_break_key | firq_sub_attention;
#if defined(HAS_2HD)
	if(intmode_fdc) {
		firq_stat = drqstat_fdc_2hd;  // 20180226 BREAK KEY AND ATTENTION MUST BE MASK IF FIRQ USING FOR FDC's DRQ.Thanks to Haserin.
			//}
	}
#endif
	nval = (firq_stat) ? 0xffffffff : 0;
	write_signals(&firq_bus, nval);
}

void FM7_MAINIO::do_nmi(bool flag)
{
	write_signals(&nmi_bus, (flag) ? 0xffffffff : 0);
}


void FM7_MAINIO::set_break_key(bool pressed)
{
	firq_break_key = pressed;
	do_firq();
}

void FM7_MAINIO::set_sub_attention(bool flag)
{
	firq_sub_attention = flag;
	do_firq();
}
  

uint8_t FM7_MAINIO::get_fd04(void)
{
	uint8_t val = 0x00;
	bool f;
	if(display->read_signal(SIG_DISPLAY_BUSY) != 0) val |= 0x80;
	
	//f = keyboard->read_signal(SIG_FM7KEY_BREAK_KEY);
	f = firq_break_key;
	if(!f) val |= 0x02;
	if(!firq_sub_attention) {
		val |= 0x01;
	}
#if defined(_FM77_VARIANTS)
	if(!stat_fdmode_2hd)  val |= 0x40;
	if(stat_kanjirom)     val |= 0x20;
	//if(intmode_fdc)      val |= 0x04;  // OK?
	val |= 0x04;
# if defined(_FM77L4)
	val |= (display->read_signal(SIG_DISPLAY_EXTRA_MODE) & 0x18);
# else
	val |= 0x18; // For NON-77L4, these bit must be '1'.Thanks to Ryu Takegami.
# endif
#elif defined(_FM8)
	val |= 0x78;
	// ToDo: MEMORY PARITY ERRO STATUS (if error, bit3 = '1').
#elif defined(_FM77AV_VARIANTS)
	val |= 0x6c;
	val |= 0x10; // ToDo: Hack for OS-9 for 77AV.Really need?
#else
	val |= 0x7c;
#endif	
#if defined(HAS_2HD)
	if(drqstat_fdc_2hd) {
		drqstat_fdc_2hd = false;
		if(!firq_sub_attention) do_firq();
	}
#endif
	if(firq_sub_attention) {
		set_sub_attention(false);
		//printf("Attention \n");
	} 
#if defined(_FM77AV_VARIANTS)
	if(hotreset) {
		if(mainmem->read_signal(FM7_MAINIO_INITROM_ENABLED) == 0) {
			set_break_key(false);
			hotreset = false;
		}
	}
#endif
	return val;
}

void FM7_MAINIO::set_fd04(uint8_t val)
{
	// NOOP?
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	call_write_signal(display, SIG_DISPLAY_EXTRA_MODE, val, 0xff);
	stat_kanjirom = ((val & 0x20) != 0);
#elif defined(_FM77_VARIANTS)
	call_write_signal(display, SIG_DISPLAY_EXTRA_MODE, val, 0xff);
	stat_kanjirom    = ((val & 0x20) != 0);
#endif
#if defined(HAS_2HD)
	stat_fdmode_2hd  = ((val & 0x40) == 0);
	intmode_fdc      = ((val & 0x04) == 0); // I/O port definition at Oh!FM 1985-5? is inverted status.
	bool tmpf        = intmode_fdc;
	if(tmpf != intmode_fdc) {
		do_firq();
	}
#endif	
}

// FD05
uint8_t FM7_MAINIO::get_fd05(void)
{
	uint8_t val = 0x7e;
	if(display->read_signal(SIG_DISPLAY_BUSY) != 0) val |= 0x80;
	if(!extdet_neg) val |= 0x01;
	return val;
}

void FM7_MAINIO::set_fd05(uint8_t val)
{
	sub_cancel = ((val & 0x40) != 0) ? true : false;
	sub_halt   = ((val & 0x80) != 0) ? true : false;
	//if(sub_halt != sub_halt_bak) {
	call_write_signal(display, SIG_DISPLAY_HALT,  (sub_halt) ? 0xff : 0x00, 0xff);
	//}
	sub_halt_bak = sub_halt;

	//if(sub_cancel != sub_cancel_bak) {
	call_write_signal(display, SIG_FM7_SUB_CANCEL, (sub_cancel) ? 0xff : 0x00, 0xff); // HACK
	//}
	sub_cancel_bak = sub_cancel;
#ifdef WITH_Z80
	if((val & 0x01) != 0) {
		call_write_signal(maincpu, SIG_CPU_HALTREQ, 1, 1);
		req_z80run = true;
	} else {
		req_z80run = false;
		if(z80 != NULL) call_write_signal(z80, SIG_CPU_BUSREQ, 1, 1);
	}
#endif
}

void FM7_MAINIO::set_extdet(bool flag)
{
	extdet_neg = flag;
}

void FM7_MAINIO::write_fd0f(void)
{
	call_write_signal(mainmem, FM7_MAINIO_PUSH_FD0F, 0, 0xffffffff);
}

uint8_t FM7_MAINIO::read_fd0f(void)
{
	call_write_signal(mainmem, FM7_MAINIO_PUSH_FD0F, 0xffffffff, 0xffffffff);
	return 0xff;
}

bool FM7_MAINIO::get_rommode_fd0f(void)
{
	return (mainmem->read_signal(FM7_MAINIO_PUSH_FD0F) == 0) ? false : true;
}


// Kanji ROM, FD20 AND FD21 (or SUBSYSTEM)
void FM7_MAINIO::write_kanjiaddr_hi(uint8_t addr)
{
#if defined(CAPABLE_JCOMMCARD) && !(defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS))
	if(jcommcard != NULL) {
		jcommcard->write_io8(0, (uint32_t)addr);
		return;
	}
#endif
	if(!connect_kanjiroml1) {
		return;
	}
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
	if(!stat_kanjirom) return;
#endif	
	kanjiclass1->write_data8(KANJIROM_ADDR_HI, addr);
	return;
}

void FM7_MAINIO::write_kanjiaddr_lo(uint8_t addr)
{
#if defined(CAPABLE_JCOMMCARD) && !(defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS))
	if(jcommcard != NULL) {
		jcommcard->write_io8(1, (uint32_t)addr);
		return;
	}
#endif
	if(!connect_kanjiroml1) {
		return;
	}
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
	if(!stat_kanjirom) return;
#endif	
	kanjiclass1->write_data8(KANJIROM_ADDR_LO, addr);
	return;
}

uint8_t FM7_MAINIO::read_kanjidata_left(void)
{
#if defined(CAPABLE_JCOMMCARD) && !(defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS))
	if(jcommcard != NULL) {
		return (uint8_t)(jcommcard->read_io8(2));
	}
#endif
	if(!connect_kanjiroml1) {
		return 0xff;
	}
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
	if(!stat_kanjirom) return 0xff;
#endif	
	//printf("KANJI MAIN CLASS1 ADDR: %05x\n", kaddress.w.l);
	if(kanjiclass1) {
		return kanjiclass1->read_data8(KANJIROM_DATA_HI);
	} else {
		return 0xff;
	}
}

uint8_t FM7_MAINIO::read_kanjidata_right(void)
{
#if defined(CAPABLE_JCOMMCARD) && !(defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS))
	if(jcommcard != NULL) {
		return (uint8_t)(jcommcard->read_io8(3));
	}
#endif
	if(!connect_kanjiroml1) {
		return 0xff;
	}
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
	if(!stat_kanjirom) return 0xff;
#endif	
	if(kanjiclass1) {
		return kanjiclass1->read_data8(KANJIROM_DATA_LO);
	} else {
		return 0xff;
	}
}

#ifdef CAPABLE_KANJI_CLASS2
// Kanji ROM, FD20 AND FD21 (or SUBSYSTEM)
void FM7_MAINIO::write_kanjiaddr_hi_l2(uint8_t addr)
{
	if(!connect_kanjiroml2) return;
	if(!stat_kanjirom) return;
	kanjiclass2->write_data8(KANJIROM_ADDR_HI, addr);
	return;
}

void FM7_MAINIO::write_kanjiaddr_lo_l2(uint8_t addr)
{
	if(!connect_kanjiroml2) return;
	if(!stat_kanjirom) return;
	kanjiclass2->write_data8(KANJIROM_ADDR_LO, addr);
	
	return;
}

uint8_t FM7_MAINIO::read_kanjidata_left_l2(void)
{
	if(!connect_kanjiroml2) return 0xff;
	if(!stat_kanjirom) return 0xff;
	
	if(kanjiclass2) {
		return kanjiclass2->read_data8(KANJIROM_DATA_HI);
	} else {
		return 0xff;
	}
}

uint8_t FM7_MAINIO::read_kanjidata_right_l2(void)
{
	if(!connect_kanjiroml2) return 0xff;
	if(!stat_kanjirom) return 0xff;
	
	if(kanjiclass2) {
		return kanjiclass2->read_data8(KANJIROM_DATA_LO);
	} else {
		return 0xff;
	}
}
#endif

uint32_t FM7_MAINIO::read_signal(int id)
{
	uint32_t retval;
	switch(id) {
	case FM7_MAINIO_KEYBOARDIRQ_MASK:
		retval = (irqmask_keyboard) ? 0xffffffff : 0x00000000;
		break;
	default:
		retval = 0xffffffff;
		break;
	}
	return retval;
}


void FM7_MAINIO::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool val_b;
	val_b = ((data & mask) != 0);
  
	switch(id) {
	case SIG_FM7_SUB_HALT:
		mainmem->write_signal(SIG_FM7_SUB_HALT, data, mask);
		break;
	case FM7_MAINIO_CLOCKMODE: // fd00
		if(val_b) {
			clock_fast = true;
		} else {
			clock_fast = false;
		}
		write_signals(&clock_status, clock_fast ? 0xffffffff : 0);
		break;
	case FM7_MAINIO_CMT_RECV: // FD02
		cmt_indat = val_b ^ cmt_invert;
		break;
	case FM7_MAINIO_CMT_INVERT: // FD02
		cmt_invert = val_b;
		break;
	case FM7_MAINIO_TIMERIRQ: //
		set_irq_timer(val_b);
		break;
	case FM7_MAINIO_LPTIRQ: //
		set_irq_printer(val_b);
		break;
	case FM7_MAINIO_LPT_BUSY:
		lpt_busy = val_b;
		break;
	case FM7_MAINIO_LPT_ERROR:
		lpt_error_inv = val_b;
		break;
	case FM7_MAINIO_LPT_ACK:
		{
			bool f = lpt_ackng_inv;
			lpt_ackng_inv = val_b;
			if((lpt_ackng_inv == true) && (f == false)) set_irq_printer(true);
		}
		break;
	case FM7_MAINIO_LPT_PAPER_EMPTY:
		lpt_busy = val_b;
		break;
	case FM7_MAINIO_LPT_DET1:
		lpt_det1 = val_b;
		break;
	case FM7_MAINIO_LPT_DET2:
		lpt_det2 = val_b;
		break;
	case FM7_MAINIO_KEYBOARDIRQ: //
		set_irq_keyboard(val_b);
		break;
		// FD04
	case FM7_MAINIO_PUSH_BREAK:
		set_break_key(val_b);
		break;
#if defined(FM77AV_VARIANTS)	
	case FM7_MAINIO_HOT_RESET:
		hotreset = val_b;
		break;
#endif
	case FM7_MAINIO_SUB_ATTENTION:
		if(val_b) set_sub_attention(true);
		break;
		// FD05
	case FM7_MAINIO_EXTDET:
		extdet_neg = !val_b;
		break;
	case FM7_MAINIO_BEEP:
		set_beep_oneshot();
		break;
	case FM7_MAINIO_PSG_IRQ:
		break;
#if defined(WITH_Z80)
	case FM7_MAINIO_RUN_Z80:
		if((req_z80run)/*  && (val_b) */) {
			if(z80 != NULL) call_write_signal(z80, SIG_CPU_BUSREQ, 0, 1);
			z80_run = true;
			//z80->reset(); // OK?
		}
		break;
	case FM7_MAINIO_RUN_6809:
		if(!(req_z80run) /* && (val_b) */ && (z80_run)) {
			z80_run = false;
			// Wait dead cycle?
			call_write_signal(maincpu, SIG_CPU_HALTREQ, 0, 1);
		}
		break;
#endif
	case FM7_MAINIO_UART0_RXRDY:
		set_irq_rxrdy(val_b & rs232c_enabled);
		break;
	case FM7_MAINIO_UART0_TXRDY:
		set_irq_txrdy(val_b & rs232c_enabled);
		break;
	case FM7_MAINIO_UART0_SYNDET:
		set_irq_syndet(val_b & rs232c_enabled);
		break;
	case FM7_MAINIO_UART0_DCD:
		rs232c_dcd = val_b;
		break;
	case FM7_MAINIO_MODEM_TXRDY:
		modem_txrdy = val_b;
		do_irq();
		break;
	case FM7_MAINIO_MODEM_RXRDY:
		modem_rxrdy = val_b;
		do_irq();
		break;
	case FM7_MAINIO_MODEM_SYNDET:
		modem_syndet = val_b;
		do_irq();
		break;
	case FM7_MAINIO_MIDI_TXRDY:
		midi_txrdy = val_b;
		do_irq();
		break;
	case FM7_MAINIO_MIDI_RXRDY:
		midi_rxrdy = val_b;
		do_irq();
		break;
	case FM7_MAINIO_MIDI_SYNDET:
		midi_syndet = val_b;
		do_irq();
		break;
			
//#if !defined(_FM8)			
	case FM7_MAINIO_OPN_IRQ:
		if(!connect_opn) break;
		intstat_opn = val_b;
		do_irq();
		break;
	case FM7_MAINIO_WHG_IRQ:
		if(!connect_whg) break;
		intstat_whg = val_b;
		do_irq();
		break;
	case FM7_MAINIO_THG_IRQ:
		if(!connect_thg) break;
		intstat_thg = val_b;
		do_irq();
		break;
//#endif			
	case FM7_MAINIO_FDC_DRQ:
		set_drq_mfd(val_b);
		break;
	case FM7_MAINIO_FDC_IRQ:
		set_irq_mfd(val_b);
		break;
#if defined(HAS_2HD)
	case FM7_MAINIO_FDC_DRQ_2HD:
		set_drq_mfd_2HD(val_b);
		break;
	case FM7_MAINIO_FDC_IRQ_2HD:
		set_irq_mfd_2HD(val_b);
		break;
#endif
#if defined(HAS_DMA)
	case FM7_MAINIO_DMA_INT:
		intstat_dma = val_b;
		do_irq();
		break;
#endif
#if defined(_FM77AV_VARIANTS)
	case SIG_DISPLAY_DISPLAY:
		if(val_b) {
			reg_fd12 |= 0x02;
		} else {
			reg_fd12 &= ~0x02;
		}
		break;
	case SIG_DISPLAY_VSYNC:
		if(val_b) {
			reg_fd12 |= 0x01;
		} else {
			reg_fd12 &= ~0x01;
		}
		break;
	case SIG_DISPLAY_MODE320:
		if(val_b) {
			reg_fd12 |= 0x40;
		} else {
			reg_fd12 &= ~0x40;
		}
		break;
#endif			
	}
}


uint8_t FM7_MAINIO::get_irqstat_fd03(void)
{
	uint8_t val;
	bool extirq;
	uint8_t backup = irqstat_reg0;
   
	extirq = intstat_opn | intstat_whg | intstat_thg;
	extirq = extirq | intstat_syndet | intstat_rxrdy | intstat_txrdy;
# if defined(HAS_2HD)
	if(intmode_fdc) {
		extirq = extirq | irqstat_fdc_2hd;
	} else {
		extirq = extirq | irqstat_fdc;
	}
# else
	extirq = extirq | irqstat_fdc;
# endif
# if defined(HAS_DMA)
	extirq = extirq | intstat_dma;
# endif   
	if(extirq) {
		irqstat_reg0 &= ~0x08;
	} else {
		irqstat_reg0 |= 0x08;
	}
	val = irqstat_reg0 | 0xf0;
	// Not call do_irq() twice. 20151221 
	irqstat_timer = false;
	irqstat_printer = false;
	irqstat_reg0 |= 0x06;
	//if(backup != irqstat_reg0) do_irq();
	//this->out_debug_log(_T("IO: Check IRQ Status."));
	do_irq();
	return val;

}

uint8_t FM7_MAINIO::get_extirq_fd17(void)
{
	uint8_t val = 0xff;
	if(intstat_opn && connect_opn)   val &= ~0x08;
	if(intstat_mouse) val &= ~0x04;
	return val;
}

void FM7_MAINIO::set_ext_fd17(uint8_t data)
{
	if((data & 0x04) != 0) {
		mouse_enable = true;
	} else {
		mouse_enable = false;
	}
}

#if defined(_FM77AV_VARIANTS)
// FD12
uint8_t FM7_MAINIO::subsystem_read_status(void)
{
	return reg_fd12;
}
#endif


uint32_t FM7_MAINIO::read_io8(uint32_t addr)
{ // This is only for debug.
	addr = addr & 0xfff;
	if(addr < 0x100) {
		return io_w_latch[addr];
	} else if(addr < 0x500) {
		uint32_t ofset = addr & 0xff;
		unsigned int opnbank = (addr - 0x100) >> 8;
		return opn_regs[opnbank][ofset];
	} else if(addr < 0x600) {
		return mainmem->read_data8(addr - 0x500 + FM7_MAINIO_MMR_BANK);
	}
	return 0xff;
}

uint32_t FM7_MAINIO::read_dma_io8(uint32_t addr)
{
	return this->read_data8(addr & 0xff);
}

uint32_t FM7_MAINIO::read_dma_data8(uint32_t addr)
{
	return this->read_data8(addr & 0xff);
}

uint32_t FM7_MAINIO::read_data8(uint32_t addr)
{
	uint32_t retval;
	uint32_t mmr_segment;
	if(addr < FM7_MAINIO_IS_BASICROM) {   
		addr = addr & 0xff;
		retval = 0xff;
#if defined(HAS_MMR)
		if((addr < 0x90) && (addr >= 0x80)) {
			mmr_segment = mainmem->read_data8(FM7_MAINIO_MMR_SEGMENT);
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || \
	defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
			mmr_segment &= 0x07;
# else		   
			mmr_segment &= 0x03;
# endif
			return mainmem->read_data8(addr - 0x80 + FM7_MAINIO_MMR_BANK + mmr_segment * 16);
		}
#endif
		//	if((addr >= 0x0006) && !(addr == 0x1f) && !(addr == 0x0b)) printf("MAINIO: READ: %08x \n", addr);
		switch(addr) {
		case 0x00: // FD00
			retval = (uint32_t) get_port_fd00();
			break;
		case 0x01: // FD01
//#if !defined(_FM8)			
			retval = keyboard->read_data8(0x01) & 0xff;
//#endif			
				break;
		case 0x02: // FD02
			retval = (uint32_t) get_port_fd02();
			break;
		case 0x03: // FD03
//#if !defined(_FM8)
			retval = (uint32_t) get_irqstat_fd03();
//#endif			
				break;
		case 0x04: // FD04
			retval = (uint32_t) get_fd04();
			break;
		case 0x05: // FD05
			retval = (uint32_t) get_fd05();
			break;
		case 0x06: // RS-232C
		case 0x07:
			if(uart_enabled[0] && rs232c_enabled) {
				if(uart[0] != NULL) retval = uart[0]->read_io8(addr & 1);
			}
			break;
		case 0x08: // Light pen
		case 0x09:
   		case 0x0a:
			break;
#if defined(_FM77AV_VARIANTS)
		case 0x0b:
			retval = ((config.boot_mode & 3) == 0) ? 0xfe : 0xff;
#if defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV20EX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			retval &= (uint32_t)(~0x04);
			if(!rs232c_dcd) retval |= 0x04;
#endif
			break;
#endif			
		case 0x0e: // PSG DATA
			retval = (uint32_t) get_psg();
			//printf("PSG DATA READ val=%02x\n", retval);
			break;
		case 0x0f: // FD0F
		  	read_fd0f();
			retval = 0xff;
			break;
#if defined(_FM77AV_VARIANTS)
		case 0x12:
			retval = subsystem_read_status();  
			break;
#endif
			//printf("OPN CMD READ \n");
			break;
		case 0x16: // OPN DATA
			retval = (uint32_t) get_opn(0);
			break;
		case 0x17:
			retval = (uint32_t) get_extirq_fd17();
			break;

#if defined(HAS_2HD)
		case 0x18: // FDC: STATUS
			if(stat_fdmode_2hd) {
				retval = (uint32_t) get_fdc_stat_2HD();
			} else {
				retval = (uint32_t) get_fdc_stat();
			}
		    break;
			//printf("FDC: READ STATUS %02x PC=%04x\n", retval, maincpu->get_pc()); 
			break;
		case 0x19: // FDC: Track
			if(stat_fdmode_2hd) {
				retval = (uint32_t) get_fdc_track_2HD();
			} else {
				retval = (uint32_t) get_fdc_track();
			}
			//printf("FDC: READ TRACK REG %02x\n", retval); 
			break;
		case 0x1a: // FDC: Sector
			if(stat_fdmode_2hd) {
				retval = (uint32_t) get_fdc_sector_2HD();
			} else {
				retval = (uint32_t) get_fdc_sector();
			}
			//printf("FDC: READ SECTOR REG %02x\n", retval); 
			break;
		case 0x1b: // FDC: Data
			if(stat_fdmode_2hd) {
				retval = (uint32_t) get_fdc_data_2HD();
			} else {
				retval = (uint32_t) get_fdc_data();
			}
			break;
		case 0x1c:
			if(stat_fdmode_2hd) {
				retval = (uint32_t) get_fdc_fd1c_2HD();
			} else {
				retval = (uint32_t) get_fdc_fd1c();
			}
			//printf("FDC: READ HEAD REG %02x\n", retval); 
			break;
		case 0x1d:
			if(stat_fdmode_2hd) {
				retval = (uint32_t) get_fdc_motor_2HD();
			} else {
				retval = (uint32_t) get_fdc_motor();
			}
			//printf("FDC: READ MOTOR REG %02x\n", retval); 
			break;
		case 0x1e:
			if(stat_fdmode_2hd) {
				retval = (uint32_t) get_fdc_fd1e_2HD();
			} else {
				retval = (uint32_t) get_fdc_fd1e();
			}
			//printf("FDC: READ MOTOR REG %02x\n", retval); 
			break;
		case 0x1f:
			if(stat_fdmode_2hd) {
				retval = (uint32_t) fdc_getdrqirq_2HD();
			} else {
				retval = (uint32_t) fdc_getdrqirq();
			}
			break;
#else
		case 0x18: // FDC: STATUS
			retval = (uint32_t) get_fdc_stat();
		    break;
			//printf("FDC: READ STATUS %02x PC=%04x\n", retval, maincpu->get_pc()); 
			break;
		case 0x19: // FDC: Track
			retval = (uint32_t) get_fdc_track();
			//printf("FDC: READ TRACK REG %02x\n", retval); 
			break;
		case 0x1a: // FDC: Sector
			retval = (uint32_t) get_fdc_sector();
			//printf("FDC: READ SECTOR REG %02x\n", retval); 
			break;
		case 0x1b: // FDC: Data
			retval = (uint32_t) get_fdc_data();
			break;
		case 0x1c:
			retval = (uint32_t) get_fdc_fd1c();
			//printf("FDC: READ HEAD REG %02x\n", retval); 
			break;
		case 0x1d:
			retval = (uint32_t) get_fdc_motor();
			//printf("FDC: READ MOTOR REG %02x\n", retval); 
			break;
		case 0x1e:
			retval = (uint32_t) get_fdc_fd1e();
			//printf("FDC: READ MOTOR REG %02x\n", retval); 
			break;
		case 0x1f:
			retval = (uint32_t) fdc_getdrqirq();
			break;
#endif
		case 0x22: // Kanji ROM
			retval = (uint32_t) read_kanjidata_left();
			break;
		case 0x23: // Kanji ROM
			retval = (uint32_t) read_kanjidata_right();
			break;
#if defined(CAPABLE_JCOMMCARD)
		case 0x28:
		case 0x29:
# if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		case 0x2a:
		case 0x2b:
# endif
			if(jcommcard != NULL) {
				retval = (uint32_t)(jcommcard->read_io8(addr));
			} else {
				retval = 0xff;
			}
			break;
#endif			
#if defined(CAPABLE_KANJI_CLASS2)
		case 0x2e: // Kanji ROM Level2
			retval = (uint32_t) read_kanjidata_left_l2();
			break;
		case 0x2f: // Kanji ROM Level2
			retval = (uint32_t) read_kanjidata_right_l2();
			break;
#endif
//#if !defined(_FM8)			
		case 0x37: // Multi page
			//retval = (uint32_t)display->read_data8(DISPLAY_ADDR_MULTIPAGE);
			break;
		case 0x40: // MODEM
		case 0x41:
			if(uart_enabled[1]) {
				if(uart[1] != NULL) retval = uart[1]->read_io8(addr & 1);
			}
			break;
		case 0x42:
			break;
		case 0x45: // WHG CMD
			break;
		case 0x46: // WHG DATA
			retval = (uint32_t) get_opn(1);
			break;
		case 0x47:
			retval = (uint32_t) get_extirq_whg();
			break;
		case 0x51: // THG CMD
			break;
		case 0x52: // THG DATA
			retval = (uint32_t) get_opn(2);
			break;
		case 0x53:
			retval = (uint32_t) get_extirq_thg();
			break;
//#endif			
#if defined(HAS_MMR)
		case 0x93:
			retval = 0x3e;
			//retval = 0x00;
			if(mainmem->read_signal(FM7_MAINIO_BOOTRAM_RW) != 0)     retval |= 0x01;
			if(mainmem->read_signal(FM7_MAINIO_WINDOW_ENABLED) != 0) retval |= 0x40;
			if(mainmem->read_signal(FM7_MAINIO_MMR_ENABLED) != 0)    retval |= 0x80;
			break;
#endif
#if defined(_FM77AV40SX) || defined(_FM77AV40EX)
		case 0x95:
			retval = 0x77;
			if(mainmem->read_signal(FM7_MAINIO_FASTMMR_ENABLED) != 0) retval |= 0x08;
			if(mainmem->read_signal(FM7_MAINIO_EXTROM)  != 0) retval |= 0x80;
			break;
#endif			
#if defined(HAS_DMA)
		case 0x98:
			retval = dma_addr;
			break;
		case 0x99:
			retval = dmac->read_data8(dma_addr);
			break;
#endif			
		case 0xea: // MIDI
		case 0xeb:
			if(uart_enabled[2]) {
				if(uart[2] != NULL) retval = uart[2]->read_io8(addr & 1);
			}
			break;
		default:
			break;
		}
//#if !defined(_FM8)			
			if((addr < 0x40) && (addr >= 0x38)) {
				addr = (addr - 0x38) + FM7_SUBMEM_OFFSET_DPALETTE;
				return (uint32_t) display->read_data8(addr);
			}
//#endif		
				// Another:
			return retval;
	} else if(addr == FM7_MAINIO_CLOCKMODE) {
		return (uint32_t)get_clockmode();
	}
#if defined(_FM77AV_VARIANTS)
	else if(addr == FM7_MAINIO_SUBMONITOR_ROM) {
		retval = sub_monitor_type & 0x03;
		return retval;
	}  else if(addr == FM7_MAINIO_SUBMONITOR_RAM) {
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
		retval = ((sub_monitor_type & 0x04) != 0) ? 0xffffffff : 0x00000000;
#else
		retval = 0;
#endif
		return retval;
	}
#endif
	//if((addr >= 0x0006) && (addr != 0x1f)) printf("MAINIO: READ: %08x DATA=%08x\n", addr);
	return 0xff;
}

void FM7_MAINIO::write_dma_io8(uint32_t addr, uint32_t data)
{
	this->write_data8(addr & 0xff, data);
}

void FM7_MAINIO::write_dma_data8(uint32_t addr, uint32_t data)
{
	this->write_data8(addr & 0xff, data);
}


void FM7_MAINIO::write_data8(uint32_t addr, uint32_t data)
{
	bool flag;
	uint32_t mmr_segment;
	if(addr < FM7_MAINIO_IS_BASICROM) {
		addr = addr & 0xff;
		io_w_latch[addr] = data;
#if defined(HAS_MMR)
		if((addr < 0x90) && (addr >= 0x80)) {
			mmr_segment = mainmem->read_data8(FM7_MAINIO_MMR_SEGMENT);
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || \
	defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
			mmr_segment &= 0x07;
# else		   
			mmr_segment &= 0x03;
# endif
			mainmem->write_data8(FM7_MAINIO_MMR_BANK + mmr_segment * 16 + addr - 0x80, data);
			return;
		}
#endif
		switch(addr) {
		case 0x00: // FD00
			set_port_fd00((uint8_t)data);
			return;
			break;
		case 0x01: // FD01
			lpt_outdata = (uint8_t)data;
			switch(lpt_type) {
			case 0: // Write to file
				printer->write_signal(SIG_PRINTER_DATA, data, 0xff);
				break;
			case 1:
			case 2:
				joystick->write_data8(0x01, data);
				break;
			}
			break;
		case 0x02: // FD02
			set_port_fd02((uint8_t)data);
			break;
		case 0x03: // FD03
			set_beep(data);
			break;
		case 0x04: // FD04
			set_fd04(data);
			break;
		case 0x05: // FD05
	  		set_fd05((uint8_t)data);
			break;
		case 0x06: // RS-232C
		case 0x07:
			if(uart_enabled[0] && rs232c_enabled) {
				if(uart[0] != NULL) uart[0]->write_io8(addr & 1, data);
			}
			break;
		case 0x08: // Light pen
		case 0x09:
		case 0x0a:
			break;
		case 0x0c:
#if defined(_FM77AV20) || defined(_FM77AV40) || defined(_FM77AV20EX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
			if(rs232c_dtr != NULL) {
				if((data & 0x04) != 0) {
					rs232c_dtr->write_signal(SIG_AND_BIT_1, 0, 1);
				} else {
					rs232c_dtr->write_signal(SIG_AND_BIT_1, 1, 1);
				}
			}
			rs232c_enabled = ((data & 0x01) != 0) ? true : false;
#endif
			break;
		case 0x0d:
			//printf("PSG CMD WRITE val=%02x\n", data);
			set_psg_cmd(data);
			break;
		case 0x0e:
			//printf("PSG DATA WRITE val=%02x\n", data);
			set_psg(data);
			break;
		case 0x0f: // FD0F
			write_fd0f();
			break;
#if defined(_FM77AV_VARIANTS)
		case 0x10:
			flag = ((data & 0x02) == 0) ? true : false;
			call_write_signal(mainmem, FM7_MAINIO_INITROM_ENABLED, (flag) ? 0xffffffff : 0 , 0xffffffff);
			break;
		case 0x12:
			call_write_signal(display, SIG_DISPLAY_MODE320, data,  0x40);
			reg_fd12 &= ~0x40;
			reg_fd12 |= (data & 0x40);
			break;
		case 0x13:
			sub_monitor_type = data & 0x07;
			call_write_signal(display, SIG_FM7_SUB_BANK, sub_monitor_type, 0x07);
			break;
#endif
		case 0x15: // OPN CMD
			//printf("OPN CMD WRITE val=%02x\n", data);
			set_opn_cmd(0, data);
			break;
		case 0x16: // OPN DATA
			//printf("OPN DATA WRITE val=%02x\n", data);
			set_opn(0, data);
			break;
		case 0x17:
			set_ext_fd17((uint8_t)data);
			break;
#if defined(HAS_2HD)
		case 0x18: // FDC: COMMAND
			if(stat_fdmode_2hd) {
				set_fdc_cmd_2HD((uint8_t)data);
			} else {
				set_fdc_cmd((uint8_t)data);
			}
			//printf("FDC: WRITE CMD %02x\n", data); 
			break;
		case 0x19: // FDC: Track
			if(stat_fdmode_2hd) {
				set_fdc_track_2HD((uint8_t)data);
			} else {
				set_fdc_track((uint8_t)data);
			}
			//printf("FDC: WRITE TRACK REG %02x\n", data); 
			break;
		case 0x1a: // FDC: Sector
			if(stat_fdmode_2hd) {
				set_fdc_sector_2HD((uint8_t)data);
			} else {
				set_fdc_sector((uint8_t)data);
			}
			//printf("FDC: WRITE SECTOR REG %02x\n", data); 
			break;
   		case 0x1b: // FDC: Data
			if(stat_fdmode_2hd) {
				set_fdc_data_2HD((uint8_t)data);
			} else {
				set_fdc_data((uint8_t)data);
			}
			break;
		case 0x1c:
			if(stat_fdmode_2hd) {
				set_fdc_fd1c_2HD((uint8_t)data);
			} else {
				set_fdc_fd1c((uint8_t)data);
			}
			//printf("FDC: WRITE HEAD REG %02x\n", data); 
			break;
		case 0x1d:
			if(stat_fdmode_2hd) {
				set_fdc_fd1d_2HD((uint8_t)data);
			} else {
				set_fdc_fd1d((uint8_t)data);
			}
			//printf("FDC: WRITE MOTOR REG %02x\n", data); 
			break;
		case 0x1e:
			if(stat_fdmode_2hd) {
				set_fdc_fd1e_2HD((uint8_t)data);
			} else {
				set_fdc_fd1e((uint8_t)data);
			}
			break;
		case 0x1f: // ??
			return;
			break;
#else
		case 0x18: // FDC: COMMAND
			set_fdc_cmd((uint8_t)data);
			//printf("FDC: WRITE CMD %02x\n", data); 
			break;
		case 0x19: // FDC: Track
			set_fdc_track((uint8_t)data);
			//printf("FDC: WRITE TRACK REG %02x\n", data); 
			break;
		case 0x1a: // FDC: Sector
			set_fdc_sector((uint8_t)data);
			//printf("FDC: WRITE SECTOR REG %02x\n", data); 
			break;
   		case 0x1b: // FDC: Data
			set_fdc_data((uint8_t)data);
			break;
		case 0x1c:
			set_fdc_fd1c((uint8_t)data);
			//printf("FDC: WRITE HEAD REG %02x\n", data); 
			break;
		case 0x1d:
			set_fdc_fd1d((uint8_t)data);
			//printf("FDC: WRITE MOTOR REG %02x\n", data); 
			break;
		case 0x1e:
			set_fdc_fd1e((uint8_t)data);
			break;
		case 0x1f: // ??
			return;
			break;
#endif
		case 0x20: // Kanji ROM
			write_kanjiaddr_hi((uint8_t)data);
			break;

		case 0x21: // Kanji ROM
			write_kanjiaddr_lo((uint8_t)data);
			break;
#if defined(CAPABLE_JCOMMCARD)
# if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		case 0x28:
		case 0x29:
# endif
		case 0x2a:
		case 0x2b:
			if(jcommcard != NULL) jcommcard->write_io8(addr, data);
			break;
#endif			
		case 0x2c: // Kanji ROM(DUP)
#if defined(CAPABLE_KANJI_CLASS2)
			write_kanjiaddr_hi_l2((uint8_t)data);
#else			
			//write_kanjiaddr_hi((uint8_t)data);
#endif
			break;
		case 0x2d: // Kanji ROM(DUP)
#if defined(CAPABLE_KANJI_CLASS2)
			write_kanjiaddr_lo_l2((uint8_t)data);
#else
			//write_kanjiaddr_lo((uint8_t)data);
#endif
			break;
#if defined(CAPABLE_DICTROM)
		case 0x2e: // 
			call_write_signal(mainmem, FM7_MAINIO_EXTBANK, data, 0xff);
			break;
#endif			
#if defined(_FM77AV_VARIANTS)
		case 0x30:
			display->write_data8(FM7_SUBMEM_OFFSET_APALETTE_HI, data);
			break;
		case 0x31:
			display->write_data8(FM7_SUBMEM_OFFSET_APALETTE_LO, data);
			break;
		case 0x32:
			display->write_data8(FM7_SUBMEM_OFFSET_APALETTE_B, data);
			break;
		case 0x33:
			display->write_data8(FM7_SUBMEM_OFFSET_APALETTE_R, data);
			break;
		case 0x34:
			display->write_data8(FM7_SUBMEM_OFFSET_APALETTE_G, data);
			break;
#endif
//#if !defined(_FM8)			
		case 0x37: // Multi page
			call_write_signal(display, SIG_DISPLAY_MULTIPAGE, data, 0x00ff);
			break;
		case 0x40: // MODEM
		case 0x41:
			if(uart_enabled[1]) {
				if(uart[1] != NULL) uart[1]->write_io8(addr & 1, data);
			}
			break;
		case 0x42:
			modem_irqmask_txrdy = ((data & 0x20) != 0);
			modem_irqmask_rxrdy = ((data & 0x40) != 0);
			do_irq();
			break;
			
		case 0x45: // WHG CMD
			set_opn_cmd(1, data);
			break;
		case 0x46: // WHG DATA
			set_opn(1, data);
			break;
		case 0x47:
			break;
		case 0x51: // THG CMD
			set_opn_cmd(2, data);
			break;
		case 0x52: // THG DATA
			set_opn(2, data);
			break;
		case 0x53:
			break;
//#endif			
#if defined(HAS_MMR)
		case 0x90:
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
			mmr_segment = data & 7;
#else
			//			printf("MMR SEGMENT: %02x\n", data & 3);
			mmr_segment = data & 3;
#endif			
			mainmem->write_data8(FM7_MAINIO_MMR_SEGMENT, (uint32_t)mmr_segment);
			break;
		case 0x92:
			mainmem->write_data8(FM7_MAINIO_WINDOW_OFFSET, (uint32_t)(data & 0x00ff));
			break;
		case 0x93:
			call_write_signal(mainmem, FM7_MAINIO_BOOTRAM_RW, data, 0x01);
			call_write_signal(mainmem, FM7_MAINIO_WINDOW_ENABLED, data , 0x40);
			//this->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
			//mainmem->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
			call_write_signal(mainmem, FM7_MAINIO_MMR_ENABLED, data, 0x80);
			//}
			break;
#endif
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
		case 0x94:
			call_write_signal(mainmem, FM7_MAINIO_MMR_EXTENDED, data, 0x80);
			call_write_signal(mainmem, FM7_MAINIO_MEM_REFRESH_FAST, data, 0x04);
			call_write_signal(mainmem, FM7_MAINIO_WINDOW_FAST , data, 0x01);

			break;
# if defined(_FM77AV40SX) || defined(_FM77AV40EX)
		case 0x95:
			call_write_signal(mainmem, FM7_MAINIO_FASTMMR_ENABLED, data, 0x08);
			call_write_signal(mainmem, FM7_MAINIO_EXTROM, data , 0x80);
			break;
# endif
#endif			
#if defined(HAS_DMA)
		case 0x98:
			dma_addr = data & 0x1f;
			break;
		case 0x99:
			dmac->write_data8(dma_addr, data);
			//this->out_debug_log(_T("IO: Wrote DMA %02x to reg %02x\n"), data, dma_addr);
			break;
#endif
			// MIDI
		case 0xea: 
		case 0xeb:
			if(uart_enabled[2]) {
				if(uart[2] != NULL) uart[2]->write_io8(addr & 1, data);
			}
			break;
		default:
			//printf("MAIN: Write I/O Addr=%08x DATA=%02x\n", addr, data); 
			break;
		}
		if((addr < 0x40) && (addr >= 0x38)) {
			addr = (addr - 0x38) | FM7_SUBMEM_OFFSET_DPALETTE;
			display->write_data8(addr, (uint8_t)data);
			return;
		}// Another:
		return;
	} else if(addr == FM7_MAINIO_CLOCKMODE) {
		set_clockmode((uint8_t)data);
		return;
	}
	//if((addr >= 0x0006) && !(addr == 0x1f)) printf("MAINIO: WRITE: %08x DATA=%08x\n", addr, data);
}

void FM7_MAINIO::event_callback(int event_id, int err)
{
//	printf("MAIN EVENT id=%d\n", event_id);
	switch(event_id) {
	case EVENT_BEEP_OFF:
		event_beep_off();
		break;
	case EVENT_BEEP_CYCLE:
		event_beep_cycle();
		break;
	case EVENT_UP_BREAK:
		call_write_signal(keyboard, SIG_FM7KEY_OVERRIDE_PRESS_BREAK, 0, 0xffffffff);
		set_break_key(false);
		break;
//#if !defined(_FM8)
	case EVENT_TIMERIRQ_ON:
		//if(!irqmask_timer) set_irq_timer(true);
		set_irq_timer(!irqmask_timer);
		break;
//#endif			
	case EVENT_FD_MOTOR_ON:
		set_fdc_motor(true);
		event_fdc_motor = -1;
		break;
	case EVENT_FD_MOTOR_OFF:
		set_fdc_motor(false);
		event_fdc_motor = -1;
		break;
#if defined(HAS_2HD)
	case EVENT_FD_MOTOR_ON_2HD:
		set_fdc_motor_2HD(true);
		event_fdc_motor_2HD = -1;
		break;
	case EVENT_FD_MOTOR_OFF_2HD:
		set_fdc_motor_2HD(false);
		event_fdc_motor_2HD = -1;
		break;
	case EVENT_FD_NMI_2HD:
		do_nmi(true);
		event_2hd_nmi = -1;
		break;
#endif
	case EVENT_PRINTER_RESET_COMPLETED:			
		write_signals(&printer_reset_bus, 0x00);
		break;
	default:
		break;
	}
}


void FM7_MAINIO::update_config()
{
	switch(config.cpu_type){
	case 0:
		clock_fast = true;
		break;
	case 1:
		clock_fast = false;
		break;
	}
	write_signals(&clock_status, clock_fast ? 0xffffffff : 0);
}

void FM7_MAINIO::event_vline(int v, int clock)
{
}

#define STATE_VERSION 17

bool FM7_MAINIO::decl_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}

	state_fio->StateArray(io_w_latch, sizeof(io_w_latch), 1);
	
	state_fio->StateValue(clock_fast);
	state_fio->StateValue(lpt_strobe);
	state_fio->StateValue(lpt_slctin);
	state_fio->StateValue(beep_flag);
	state_fio->StateValue(beep_snd);
	
		// FD01
	state_fio->StateValue(lpt_outdata);
	// FD02
	state_fio->StateValue(cmt_indat);
	state_fio->StateValue(cmt_invert);
	state_fio->StateValue(lpt_det2);
	state_fio->StateValue(lpt_det1);
	state_fio->StateValue(lpt_pe);
	state_fio->StateValue(lpt_ackng_inv);
	state_fio->StateValue(lpt_error_inv);
	state_fio->StateValue(irqmask_reg0);
	
	state_fio->StateValue(irqmask_syndet);
	state_fio->StateValue(irqmask_rxrdy);
	state_fio->StateValue(irqmask_txrdy);
	state_fio->StateValue(irqmask_mfd);
	state_fio->StateValue(irqmask_timer);
	state_fio->StateValue(irqmask_printer);
	state_fio->StateValue(irqmask_keyboard);

	state_fio->StateValue(irqreq_syndet);
	state_fio->StateValue(irqreq_rxrdy);
	state_fio->StateValue(irqreq_txrdy);
	state_fio->StateValue(irqreq_printer);
	state_fio->StateValue(irqreq_keyboard);
	state_fio->StateValue(irqstat_reg0);
		
	state_fio->StateValue(irqstat_timer);
	state_fio->StateValue(irqstat_printer);
	state_fio->StateValue(irqstat_keyboard);
		
		// FD04
#if defined(_FM77_VARIANTS)
	state_fio->StateValue(stat_fdmode_2hd);
	state_fio->StateValue(stat_kanjirom);
#elif defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
	state_fio->StateValue(stat_kanjirom);
#endif
	state_fio->StateValue(firq_break_key);
	state_fio->StateValue(firq_sub_attention);
	
	state_fio->StateValue(intmode_fdc);
	// FD05
	state_fio->StateValue(extdet_neg);
	state_fio->StateValue(sub_halt);
	state_fio->StateValue(sub_cancel);
	// FD06, 07
	state_fio->StateValue(intstat_syndet);
	state_fio->StateValue(intstat_rxrdy);
	state_fio->StateValue(intstat_txrdy);
	
	
	state_fio->StateValue(intstat_opn);
	state_fio->StateValue(intstat_mouse);
	state_fio->StateValue(mouse_enable);
	
	state_fio->StateValue(intstat_whg);
	state_fio->StateValue(intstat_thg);
	
	// FDC
	state_fio->StateValue(connect_fdc);
	state_fio->StateValue(fdc_statreg);
	state_fio->StateValue(fdc_cmdreg);
	state_fio->StateValue(fdc_trackreg);
	state_fio->StateValue(fdc_sectreg);
	state_fio->StateValue(fdc_datareg);
	state_fio->StateValue(fdc_headreg);
	state_fio->StateValue(fdc_drvsel);
	state_fio->StateValue(irqreg_fdc);
	state_fio->StateValue(fdc_motor);
	state_fio->StateValue(irqstat_fdc);
	// KANJI ROM
	state_fio->StateValue(connect_kanjiroml1);
#if defined(_FM77AV_VARIANTS)
	state_fio->StateValue(connect_kanjiroml2);
	
	state_fio->StateValue(boot_ram);
	state_fio->StateValue(hotreset);
		// FD13
	state_fio->StateValue(sub_monitor_type);
#endif	
		// MMR
	//V2
	{
		state_fio->StateValue(event_beep);
		state_fio->StateValue(event_beep_oneshot);
		state_fio->StateValue(event_timerirq);
	}		
	{ // V3
		state_fio->StateValue(event_fdc_motor);
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)|| \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
		state_fio->StateArray(fdc_drive_table, sizeof(fdc_drive_table), 1);
		state_fio->StateValue(fdc_reg_fd1e);
#endif	
#if defined(HAS_DMA)
		state_fio->StateValue(intstat_dma);
		state_fio->StateValue(dma_addr);
#endif			
#if defined(_FM77AV_VARIANTS)
		state_fio->StateValue(reg_fd12);
#endif		
	}
// FD05
	state_fio->StateValue(req_z80run);
	state_fio->StateValue(z80_run);

	// UART
	state_fio->StateValue(rs232c_enabled);
	state_fio->StateValue(rs232c_dcd);
	state_fio->StateArray(uart_enabled, sizeof(uart_enabled), 1);
	
	state_fio->StateValue(modem_irqmask_rxrdy);
	state_fio->StateValue(modem_irqmask_txrdy);
	state_fio->StateValue(modem_syndet);
	state_fio->StateValue(modem_rxrdy);
	state_fio->StateValue(modem_txrdy);

	state_fio->StateValue(midi_uart_irqmask);
	state_fio->StateValue(midi_syndet);
	state_fio->StateValue(midi_rxrdy);
	state_fio->StateValue(midi_txrdy);
#if defined(HAS_2HD)
	state_fio->StateValue(event_fdc_motor_2HD);
	state_fio->StateValue(connect_fdc_2HD);
	state_fio->StateValue(fdc_2HD_statreg);
	state_fio->StateValue(fdc_2HD_cmdreg);
	state_fio->StateValue(fdc_2HD_trackreg);
	state_fio->StateValue(fdc_2HD_sectreg);
	state_fio->StateValue(fdc_2HD_datareg);
	state_fio->StateValue(fdc_2HD_headreg);
	state_fio->StateValue(fdc_2HD_drvsel);
	state_fio->StateValue(irqreg_fdc_2HD);
	state_fio->StateValue(fdc_2HD_motor);
	//state_fio->FputBool(irqstat_fdc);
	state_fio->StateValue(event_2hd_nmi);
	state_fio->StateValue(nmi_delay);
	state_fio->StateValue(irqstat_fdc_2hd);
	state_fio->StateValue(drqstat_fdc_2hd);
#endif
	
	if(!decl_state_opn(state_fio, loading)) {
		return false;
	}
	
	return true;
}
void FM7_MAINIO::save_state(FILEIO *state_fio)
{
	decl_state(state_fio, false);
#if 0
	// Debug
	for(int i = 0; i < 3; i++) {
		out_debug_log("OPN#%d registers (to Save)", i);
		out_debug_log("ADDR: +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +a +b +c +d +e +f");
		for(int ladr = 0; ladr < 0x100; ladr += 0x10) {
			out_debug_log("+%02x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
						  ladr,
						  opn_regs[i][ladr + 0],  opn_regs[i][ladr + 1],  opn_regs[i][ladr + 2],  opn_regs[i][ladr + 3],
						  opn_regs[i][ladr + 4],  opn_regs[i][ladr + 5],  opn_regs[i][ladr + 6],  opn_regs[i][ladr + 7],
						  opn_regs[i][ladr + 8],  opn_regs[i][ladr + 9],  opn_regs[i][ladr + 10], opn_regs[i][ladr + 11],
						  opn_regs[i][ladr + 12], opn_regs[i][ladr + 13], opn_regs[i][ladr + 14], opn_regs[i][ladr + 15]);
		}
	}
#endif	
}

bool FM7_MAINIO::load_state(FILEIO *state_fio)
{
	int ch;
	int addr;
	//bool stat = false;
	uint32_t version;
	bool mb = decl_state(state_fio, true);
	if(mb) {
#if defined(HAS_DMA)
		dma_addr = dma_addr & 0x1f;
#endif
	}
#if 0
	for(int i = 0; i < 3; i++) {
		out_debug_log("OPN#%d registers (Loaded)", i);
		out_debug_log("ADDR: +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +a +b +c +d +e +f");
		for(int ladr = 0; ladr < 0x100; ladr += 0x10) {
			out_debug_log("+%02x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
						  ladr,
						  opn_regs[i][ladr + 0],  opn_regs[i][ladr + 1],  opn_regs[i][ladr + 2],  opn_regs[i][ladr + 3],
						  opn_regs[i][ladr + 4],  opn_regs[i][ladr + 5],  opn_regs[i][ladr + 6],  opn_regs[i][ladr + 7],
						  opn_regs[i][ladr + 8],  opn_regs[i][ladr + 9],  opn_regs[i][ladr + 10], opn_regs[i][ladr + 11],
						  opn_regs[i][ladr + 12], opn_regs[i][ladr + 13], opn_regs[i][ladr + 14], opn_regs[i][ladr + 15]);
		}
	}
#endif
	this->out_debug_log(_T("Load State: MAINIO: id=%d stat=%s\n"), this_device_id, (mb) ? _T("OK") : _T("NG"));
	return mb;
}
	  
