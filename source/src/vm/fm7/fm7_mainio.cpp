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
#if defined(HAS_DMA)
#include "hd6844.h"
#endif

// TEST
#if !defined(_MSC_VER)
#include <SDL2/SDL.h>
#endif

FM7_MAINIO::FM7_MAINIO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
	int i;
	p_vm = parent_vm;
	p_emu = parent_emu;
	kanjiclass1 = NULL;
	kanjiclass2 = NULL;
	opn_psg_77av = false;
	// FD00
	clock_fast = true;
	lpt_strobe = false;
	lpt_slctin = false;
	// FD01
	lpt_outdata = 0x00;
	// FD02
	cmt_indat = false; // bit7
	cmt_invert = false; // Invert signal
	lpt_det2 = false;
	lpt_det1 = false;
	lpt_pe = false;
	lpt_ackng_inv = false;
	lpt_error_inv = false;
	lpt_busy = false;
	// FD04
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX) 
	stat_kanjirom = true;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
#elif defined(_FM77_VARIANTS)
	stat_fdmode_2hd = false; //  R/W : bit6, '0' = 2HD, '1' = 2DD. FM-77 Only.
	stat_kanjirom = true;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
	stat_400linecard = false;//  R/W : bit4, '0' = connected. FM-77 Only.
#endif	
	firq_break_key = false; // bit1, ON = '0'.
	firq_sub_attention = false; // bit0, ON = '0'.
	intmode_fdc = false; // bit2, '0' = normal, '1' = SFD.
	// FD05
	extdet_neg = false;
#ifdef WITH_Z80
	z80_sel = false;    // bit0 : '1' = Z80. Maybe only FM-7/77.
#endif
	// FD06,07
	intstat_syndet = false;
	intstat_rxrdy = false;
	intstat_txrdy = false;
	// FD0B
	// FD0D
	// FD0F
	stat_romrammode = true; // ROM ON
	
	// FD15/ FD46 / FD51
	connect_opn = false;
	connect_whg = false;
	connect_thg = false;
		
	for(i = 0; i < 3; i++) {
		opn_address[i] = 0x00;
		opn_data[i] = 0x00;
		opn_cmdreg[i] = 0;
	}
	joyport_a = 0x00;
	joyport_b = 0x00;
		
	intstat_whg = false;
	intstat_thg = false;
	// FD17
	intstat_opn = false;
	intstat_mouse = false;
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
	irqstat_fdc = 0;
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
	memset(io_w_latch, 0xff, 0x100);
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
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	boot_ram = false;
# if defined(_FM77_VARIANTS)
	stat_fdmode_2hd = false;
	stat_kanjirom = true;
	stat_400linecard = false;
#  if defined(_FM77L4)
	stat_400linecard = true;
#  endif	
# endif	
#endif
	irqmask_syndet = true;
	irqmask_rxrdy = true;
	irqmask_txrdy = true;
	irqmask_mfd = true;
	irqmask_timer = true;
	irqmask_printer = true;
	irqmask_keyboard = true;
	irqstat_reg0 = 0xff;
	
	intstat_syndet = false;
	intstat_rxrdy = false;
	intstat_txrdy = false;
	irqstat_timer = false;
	irqstat_printer = false;
	irqstat_keyboard = false;
  
	irqreq_syndet = false;
	irqreq_rxrdy = false;
	irqreq_txrdy = false;
	irqreq_timer = false;
	irqreq_printer = false;
	irqreq_keyboard = false;
#if defined(_FM77AV_VARIANTS)
	reg_fd12 = 0x00;
#endif		

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
   
#if defined(_FM77AV_VARIANTS)
	opn_psg_77av = true;
	hotreset = false;
#else
	opn_psg_77av = false;
#endif
	connect_opn = false;
	connect_thg = false;
	connect_whg = false;
#if defined(_FM77_VARIANTS)
	boot_ram = (mainmem->read_signal(FM7_MAINIO_BOOTRAM_RW) == 0) ? false : true;
#endif
#if defined(_FM77AV_VARIANTS)
	//enable_initiator = true;
	//mainmem->write_signal(FM7_MAINIO_INITROM_ENABLED, (enable_initiator) ? 0xffffffff : 0 , 0xffffffff);
	boot_ram = (mainmem->read_signal(FM7_MAINIO_BOOTRAM_RW) == 0) ? false : true;
#endif
	// FD05
	extdet_neg = false;
	sub_cancel = false; // bit6 : '1' Cancel req.
	sub_halt = false; // bit6 : '1' Cancel req.
	sub_cancel_bak = sub_cancel; // bit6 : '1' Cancel req.
	sub_halt_bak = sub_halt; // bit6 : '1' Cancel req.
	//sub_busy = false;
   
	extdet_neg = false;
   
	//stat_romrammode = true;
	// IF BASIC BOOT THEN ROM
	// ELSE RAM
	mainmem->write_signal(FM7_MAINIO_PUSH_FD0F, ((config.boot_mode & 3) == 0) ? 0xffffffff : 0, 0xffffffff);
#if defined(_FM77AV_VARIANTS)
	sub_monitor_type = 0x00;
#endif
	
#ifdef HAS_MMR
	mainmem->write_signal(FM7_MAINIO_WINDOW_ENABLED, 0, 0xffffffff);
	mainmem->write_data8(FM7_MAINIO_WINDOW_OFFSET, 0x00);
	mainmem->write_signal(FM7_MAINIO_FASTMMR_ENABLED, 0, 0xffffffff);
	mainmem->write_signal(FM7_MAINIO_MMR_ENABLED, 0, 0xffffffff);
	//mainmem->write_data8(FM7_MAINIO_MMR_SEGMENT, mmr_segment);
#endif
	switch(config.cpu_type){
		case 0:
			clock_fast = true;
			break;
		case 1:
			clock_fast = false;
			break;
	}
	this->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
	//mainmem->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
   
	// FD03
	irqmask_syndet = true;
	irqmask_rxrdy = true;
	irqmask_txrdy = true;
	irqmask_mfd = true;
	irqmask_timer = true;
	irqmask_printer = true;
	irqmask_keyboard = true;
	irqstat_reg0 = 0xff;
	
	intstat_syndet = false;
	intstat_rxrdy = false;
	intstat_txrdy = false;
	irqstat_timer = false;
	irqstat_printer = false;
	irqstat_keyboard = false;
  
	irqreq_syndet = false;
	irqreq_rxrdy = false;
	irqreq_txrdy = false;
	irqreq_timer = false;
	irqreq_printer = false;
	irqreq_keyboard = false;
	// FD00
	drec->write_signal(SIG_DATAREC_MIC, 0x00, 0x01);
	drec->write_signal(SIG_DATAREC_REMOTE, 0x00, 0x02);
	reset_fdc();
	reset_sound();
	
	// FD04
	firq_break_key = (keyboard->read_signal(SIG_FM7KEY_BREAK_KEY) != 0x00000000); // bit1, ON = '0'.
	set_sub_attention(false);	
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX) 
	stat_kanjirom = true;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
#elif defined(_FM77_VARIANTS)
	stat_fdmode_2hd = false; //  R/W : bit6, '0' = 2HD, '1' = 2DD. FM-77 Only.
	stat_kanjirom = true;    //  R/W : bit5, '0' = sub, '1' = main. FM-77 Only.
#endif	
	//display->write_signal(SIG_FM7_SUB_KEY_MASK, 1, 1); 
	//display->write_signal(SIG_FM7_SUB_KEY_FIRQ, 0, 1);
	maincpu->write_signal(SIG_CPU_FIRQ, 0, 1);
#if defined(HAS_DMA)
	intstat_dma = false;
	dma_addr = 0;
#endif
#if defined(_FM77AV_VARIANTS)
	reg_fd12 = 0x00;
#endif		
	register_event(this, EVENT_TIMERIRQ_ON, 10000.0 / 4.9152, true, &event_timerirq); // TIMER IRQ
	memset(io_w_latch, 0xff, 0x100);
}


void FM7_MAINIO::set_clockmode(uint8 flags)
{
	bool f = clock_fast;
	if((flags & FM7_MAINCLOCK_SLOW) != 0) {
		clock_fast = false;
	} else {
		clock_fast = true;
	}
	if(f != clock_fast) {
		this->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
		//mainmem->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
	}
}

uint8 FM7_MAINIO::get_clockmode(void)
{
	if(!clock_fast) return FM7_MAINCLOCK_SLOW;
	return FM7_MAINCLOCK_HIGH;
}


uint8 FM7_MAINIO::get_port_fd00(void)
{
	uint8 ret           = 0x7e; //0b01111110;
	if(keyboard->read_data8(0x00) != 0) ret |= 0x80; // High bit.
	if(clock_fast) ret |= 0x01; //0b00000001;
	return ret;
}
  
void FM7_MAINIO::set_port_fd00(uint8 data)
{
       drec->write_signal(SIG_DATAREC_MIC, data, 0x01);
       drec->write_signal(SIG_DATAREC_REMOTE, data, 0x02);
}
   
uint8 FM7_MAINIO::get_port_fd02(void)
{
	uint8 ret;
	// Still unimplemented printer.
	ret = (cmt_indat) ? 0xff : 0x7f; // CMT 
	return ret;
}

void FM7_MAINIO::set_port_fd02(uint8 val)
{
	irqmask_reg0 = val;
	bool syndetirq_bak = irqmask_syndet;
	bool rxrdyirq_bak = irqmask_rxrdy;
	bool txrdyirq_bak = irqmask_txrdy;
	
	bool keyirq_bak = irqmask_keyboard;
	bool timerirq_bak = irqmask_timer;
	bool printerirq_bak = irqmask_printer;
	bool mfdirq_bak = irqmask_mfd;
	
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
	if(rxrdyirq_bak != irqmask_rxrdy) {
		set_irq_rxrdy(irqreq_rxrdy);
	}
	if((val & 0x20) != 0) {
		irqmask_txrdy = false;
	} else {
		irqmask_txrdy = true;
	}
	if(txrdyirq_bak != irqmask_txrdy) {
   		set_irq_txrdy(irqreq_txrdy);
	}
	
	if((val & 0x10) != 0) {
		irqmask_mfd = false;
	} else {
		irqmask_mfd = true;
	}
	if(mfdirq_bak != irqmask_mfd) {
   		set_irq_mfd(irqreq_fdc);
	}

	if((val & 0x04) != 0) {
		irqmask_timer = false;
	} else {
		irqmask_timer = true;
	}
	if(timerirq_bak != irqmask_timer) {
   		set_irq_timer(irqreq_timer);
	}

	if((val & 0x02) != 0) {
		irqmask_printer = false;
	} else {
		irqmask_printer = true;
	}
	if(printerirq_bak != irqmask_printer) {
   		set_irq_printer(irqreq_printer);
	}
   
	if((val & 0x01) != 0) {
		irqmask_keyboard = false;
	} else {
		irqmask_keyboard = true;
	}
	if(keyirq_bak != irqmask_keyboard) {
		display->write_signal(SIG_FM7_SUB_KEY_MASK, irqmask_keyboard ? 1 : 0, 1); 
		set_irq_keyboard(irqreq_keyboard);
	}
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
	//printf("IRQ TIMER: %02x MASK=%d\n", irqstat_reg0, irqmask_timer);
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
	//printf("IRQ TIMER: %02x MASK=%d\n", irqstat_reg0, irqmask_timer);
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
	//printf("IRQ TIMER: %02x MASK=%d\n", irqstat_reg0, irqmask_timer);
}


void FM7_MAINIO::set_irq_timer(bool flag)
{
	uint8 backup = irqstat_reg0;
	irqreq_timer = flag;
	if(flag && !(irqmask_timer)) {
		irqstat_reg0 &= 0xfb; //~0x04;
		irqstat_timer = true;	   
	} else {
		irqstat_reg0 |= 0x04;
		irqstat_timer = false;	   
	}
	if(backup != irqstat_reg0) do_irq();
	//printf("IRQ TIMER: %02x MASK=%d\n", irqstat_reg0, irqmask_timer);
}

void FM7_MAINIO::set_irq_printer(bool flag)
{
	uint8 backup = irqstat_reg0;
	irqreq_printer = flag;
	if(flag && !(irqmask_printer)) {
		irqstat_reg0 &= ~0x02;
		irqstat_printer = true;	   
		if(backup != irqstat_reg0) do_irq();
	} else {
		irqstat_reg0 |= 0x02;
		irqstat_printer = false;	   
		if(backup != irqstat_reg0) do_irq();
	}
//	if(!irqmask_printer || !flag) do_irq();
}

void FM7_MAINIO::set_irq_keyboard(bool flag)
{
	//uint8 backup = irqstat_reg0;
   	//printf("MAIN: KEYBOARD: IRQ=%d MASK=%d\n", flag ,irqmask_keyboard);
	irqreq_keyboard = flag;
	if(flag && !irqmask_keyboard) {
		irqstat_reg0 &= 0xfe;
		irqstat_keyboard = true;
	} else {
		irqstat_reg0 |= 0x01;
		irqstat_keyboard = false;	   
	}
	//if(irqstat_reg0 != backup) do_irq();
	do_irq();
}


void FM7_MAINIO::do_irq(void)
{
	bool intstat;
	intstat = irqstat_timer | irqstat_keyboard | irqstat_printer;
	intstat = intstat | irqstat_fdc;
	intstat = intstat | intstat_opn | intstat_whg | intstat_thg;
   	intstat = intstat | intstat_txrdy | intstat_rxrdy | intstat_syndet;
	intstat = intstat | intstat_mouse;
#if defined(HAS_DMA)
	intstat = intstat | intstat_dma;
#endif
	//printf("%08d : IRQ: REG0=%02x FDC=%02x, stat=%d\n", SDL_GetTicks(), irqstat_reg0, irqstat_fdc, intstat);
	if(intstat) {
		maincpu->write_signal(SIG_CPU_IRQ, 1, 1);
	} else {
		maincpu->write_signal(SIG_CPU_IRQ, 0, 1);
	}
}

void FM7_MAINIO::do_firq(void)
{
	bool firq_stat;
	firq_stat = firq_break_key | firq_sub_attention; 
	if(firq_stat) {
		maincpu->write_signal(SIG_CPU_FIRQ, 1, 1);
	} else {
		maincpu->write_signal(SIG_CPU_FIRQ, 0, 1);
	}
}

void FM7_MAINIO::do_nmi(bool flag)
{
	maincpu->write_signal(SIG_CPU_NMI, flag ? 1 : 0, 1);
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
  

uint8 FM7_MAINIO::get_fd04(void)
{
	uint8 val = 0x00;
	if(display->read_signal(SIG_DISPLAY_BUSY) != 0) val |= 0x80;
	if(!firq_break_key) val |= 0x02;
	if(!firq_sub_attention) {
		val |= 0x01;
	}
#if defined(_FM77_VARIANTS)
	if(stat_fdmode_2hd) val |= 0x40;
	if(stat_kanjirom)   val |= 0x20;
	if(stat_400linecard) val |= 0x10;
	if((display->read_signal(SIG_DISPLAY_EXTRA_MODE) & 0x04) != 0x00) val |= 0x04;
#else
	val |= 0x7c;
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

void FM7_MAINIO::set_fd04(uint8 val)
{
	// NOOP?
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	display->write_signal(SIG_DISPLAY_EXTRA_MODE, val, 0xff);
	stat_kanjirom = ((val & 0x20) != 0);
#elif defined(_FM77_VARIANTS)
	display->write_signal(SIG_DISPLAY_EXTRA_MODE, val, 0xff);
	stat_fdmode_2hd  = ((val & 0x40) != 0);
	stat_kanjirom    = ((val & 0x20) != 0);
	stat_400linecard = ((val & 0x10) != 0);
#endif	
}

  // FD05
 uint8 FM7_MAINIO::get_fd05(void)
{
	uint8 val = 0x7e;
	//val = (sub_busy) ? 0xfe : 0x7e;
	if(display->read_signal(SIG_DISPLAY_BUSY) != 0) val |= 0x80;
	if(!extdet_neg) val |= 0x01;
	//printf("FD05: READ: %d VAL=%02x\n", SDL_GetTicks(), val);
	return val;
}

 void FM7_MAINIO::set_fd05(uint8 val)
{
	sub_cancel = ((val & 0x40) != 0) ? true : false;
	sub_halt   = ((val & 0x80) != 0) ? true : false;
	//if(sub_halt != sub_halt_bak) {
		display->write_signal(SIG_DISPLAY_HALT,  (sub_halt) ? 0xff : 0x00, 0xff);
	//}
	sub_halt_bak = sub_halt;

	//if(sub_cancel != sub_cancel_bak) {
		display->write_signal(SIG_FM7_SUB_CANCEL, (sub_cancel) ? 0xff : 0x00, 0xff); // HACK
	//}
	sub_cancel_bak = sub_cancel;
#ifdef WITH_Z80
	if((val & 0x01) != 0) {
		maincpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		z80->write_signal(SIG_CPU_BUSREQ, 0, 1);
	} else {
		maincpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
		z80->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
#endif
}

void FM7_MAINIO::set_extdet(bool flag)
{
	extdet_neg = flag;
}

void FM7_MAINIO::write_fd0f(void)
{
	mainmem->write_signal(FM7_MAINIO_PUSH_FD0F, 0, 0xffffffff);
}
uint8 FM7_MAINIO::read_fd0f(void)
{
	mainmem->write_signal(FM7_MAINIO_PUSH_FD0F, 0xffffffff, 0xffffffff);
	return 0xff;
}

bool FM7_MAINIO::get_rommode_fd0f(void)
{
	return (mainmem->read_signal(FM7_MAINIO_PUSH_FD0F) == 0) ? false : true;
}


// Kanji ROM, FD20 AND FD21 (or SUBSYSTEM)
void FM7_MAINIO::write_kanjiaddr_hi(uint8 addr)
{
	if(!connect_kanjiroml1) return;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
	if(!stat_kanjirom) return;
#endif	
	kanjiclass1->write_data8(KANJIROM_ADDR_HI, addr);
	return;
}

void FM7_MAINIO::write_kanjiaddr_lo(uint8 addr)
{
	if(!connect_kanjiroml1) return;
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
    defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
	if(!stat_kanjirom) return;
#endif	
	kanjiclass1->write_data8(KANJIROM_ADDR_LO, addr);
	return;
}

uint8 FM7_MAINIO::read_kanjidata_left(void)
{
	if(!connect_kanjiroml1) return 0xff;
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

uint8 FM7_MAINIO::read_kanjidata_right(void)
{
	if(!connect_kanjiroml1) return 0xff;
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
void FM7_MAINIO::write_kanjiaddr_hi_l2(uint8 addr)
{
	if(!connect_kanjiroml2) return;
	if(!stat_kanjirom) return;
	kanjiclass2->write_data8(KANJIROM_ADDR_HI, addr);
	return;
}

void FM7_MAINIO::write_kanjiaddr_lo_l2(uint8 addr)
{
	if(!connect_kanjiroml2) return;
	if(!stat_kanjirom) return;
	kanjiclass2->write_data8(KANJIROM_ADDR_LO, addr);
	
	return;
}

uint8 FM7_MAINIO::read_kanjidata_left_l2(void)
{
	if(!connect_kanjiroml2) return 0xff;
	if(!stat_kanjirom) return 0xff;
	
	if(kanjiclass2) {
		return kanjiclass2->read_data8(KANJIROM_DATA_HI);
	} else {
		return 0xff;
	}
}

uint8 FM7_MAINIO::read_kanjidata_right_l2(void)
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


uint32 FM7_MAINIO::read_signal(int id)
{
	uint32 retval;
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


void FM7_MAINIO::write_signal(int id, uint32 data, uint32 mask)
{
	bool val_b;
	val_b = ((data & mask) != 0);
  
	switch(id) {
	  //case SIG_FM7_SUB_HALT:
	  //	mainmem->write_signal(SIG_FM7_SUB_HALT, data, mask);
	  //		break;
		case FM7_MAINIO_CLOCKMODE: // fd00
			if(val_b) {
				clock_fast = true;
			} else {
				clock_fast = false;
			}
			{
#if 0
				uint32 clocks = 1794000;
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
				if(mainmem->read_signal(FM7_MAINIO_MMR_ENABLED) != 0) {
					if(mainmem->read_signal(FM7_MAINIO_FASTMMR_ENABLED)) {
						if(clock_fast) {
							clocks = 2016000; // Hz
						} else {
							clocks = 1230502; // (2016 * 1095 / 1794)[KHz]
						}
					} else {
						if(clock_fast) {
							clocks = 1565000; // Hz
						} else {
							clocks =  955226; // (1565 * 1095 / 1794)[KHz]
						}
					}
				} else {
					if(clock_fast) {
						clocks = 1794000; // Hz 
					} else {
						clocks = 1095000; // Hz
					}
				}
#else // 7/8
				if(clock_fast) {
					clocks = 1794000; // Hz 
				} else {
					clocks = 1095000; // Hz
				}
#endif
				p_vm->set_cpu_clock(this->maincpu, clocks);
#endif			   
				mainmem->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
				display->write_signal(SIG_DISPLAY_CLOCK, clock_fast ? 1 : 0, 1);
			}
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
		case FM7_MAINIO_JOYPORTA_CHANGED:
			joyport_a = data & mask;
			break;
		case FM7_MAINIO_JOYPORTB_CHANGED:
			joyport_b = data & mask;
			break;
		case FM7_MAINIO_PSG_IRQ:
			break;
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
		case FM7_MAINIO_FDC_DRQ:
			set_drq_mfd(val_b);
			break;
		case FM7_MAINIO_FDC_IRQ:
			set_irq_mfd(val_b);
			break;
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


 uint8 FM7_MAINIO::get_irqstat_fd03(void)
{
	uint8 val;
	bool extirq;
	
	extirq = irqstat_fdc | intstat_opn | intstat_whg | intstat_thg;
	extirq = extirq | intstat_syndet | intstat_rxrdy | intstat_txrdy;
#if defined(HAS_DMA)
	extirq = extirq | intstat_dma;
#endif   
	if(extirq) {
		irqstat_reg0 &= ~0x08;
	} else {
		irqstat_reg0 |= 0x08;
	}
	val = irqstat_reg0 | 0xf0;
	set_irq_timer(false);
	set_irq_printer(false);
	return val;
}

uint8 FM7_MAINIO::get_extirq_fd17(void)
{
	uint8 val = 0xff;
	if(intstat_opn && connect_opn)   val &= ~0x08;
	if(intstat_mouse) val &= ~0x04;
	//if(!intstat_opn && !intstat_mouse) do_irq(false);
	return val;
}

void FM7_MAINIO::set_ext_fd17(uint8 data)
{
	if((data & 0x04) != 0) {
		mouse_enable = true;
	} else {
		mouse_enable = false;
	}
   
}
#if defined(_FM77AV_VARIANTS)
// FD12
uint8 FM7_MAINIO::subsystem_read_status(void)
{
	return reg_fd12;
}
#endif


uint32 FM7_MAINIO::read_io8(uint32 addr)
{ // This is only for debug.
	addr = addr & 0xfff;
	if(addr < 0x100) {
		return io_w_latch[addr];
	} else if(addr < 0x500) {
		uint32 ofset = addr & 0xff;
		uint opnbank = (addr - 0x100) >> 8;
		return opn_regs[opnbank][ofset];
	} else if(addr < 0x600) {
		return mainmem->read_data8(addr - 0x500 + FM7_MAINIO_MMR_BANK);
	}
	return 0xff;
}

uint32 FM7_MAINIO::read_dma_io8(uint32 addr)
{
	return this->read_data8(addr & 0xff);
}

uint32 FM7_MAINIO::read_dma_data8(uint32 addr)
{
	return this->read_data8(addr & 0xff);
}

uint32 FM7_MAINIO::read_data8(uint32 addr)
{
	uint32 retval;
	uint32 mmr_segment;
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
			retval = (uint32) get_port_fd00();
			break;
		case 0x01: // FD01
			retval = keyboard->read_data8(0x01) & 0xff;
			break;
		case 0x02: // FD02
			retval = (uint32) get_port_fd02();
			break;
		case 0x03: // FD03
			retval = (uint32) get_irqstat_fd03();
			break;
		case 0x04: // FD04
			retval = (uint32) get_fd04();
			break;
		case 0x05: // FD05
		        retval = (uint32) get_fd05();
			break;
		case 0x06: // RS-232C
		case 0x07:
			break;
		case 0x08: // Light pen
		case 0x09:
    		case 0x0a:
			break;
#if defined(_FM77AV_VARIANTS)
		case 0x0b:
			retval = ((config.boot_mode & 3) == 0) ? 0xfe : 0xff;
			break;
#endif			
		case 0x0e: // PSG DATA
			retval = (uint32) get_psg();
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
		case 0x15: // OPN CMD
			//printf("OPN CMD READ \n");
			break;
		case 0x16: // OPN DATA
			retval = (uint32) get_opn(0);
			//printf("OPN DATA READ val=%02x\n", retval);
			break;
		case 0x17:
			retval = (uint32) get_extirq_fd17();
			break;
		case 0x18: // FDC: STATUS
		  	retval = (uint32) get_fdc_stat();
			//printf("FDC: READ STATUS %02x PC=%04x\n", retval, maincpu->get_pc()); 
			break;
		case 0x19: // FDC: Track
			retval = (uint32) get_fdc_track();
			//printf("FDC: READ TRACK REG %02x\n", retval); 
			break;
		case 0x1a: // FDC: Sector
			retval = (uint32) get_fdc_sector();
			//printf("FDC: READ SECTOR REG %02x\n", retval); 
			break;
		case 0x1b: // FDC: Data
			retval = (uint32) get_fdc_data();
			break;
		case 0x1c:
			retval = (uint32) get_fdc_fd1c();
			//printf("FDC: READ HEAD REG %02x\n", retval); 
			break;
		case 0x1d:
			retval = (uint32) get_fdc_motor();
			//printf("FDC: READ MOTOR REG %02x\n", retval); 
			break;
		case 0x1e:
			retval = (uint32) get_fdc_fd1e();
			//printf("FDC: READ MOTOR REG %02x\n", retval); 
			break;
		case 0x1f:
			retval = (uint32) fdc_getdrqirq();
			break;
		case 0x22: // Kanji ROM
			retval = (uint32) read_kanjidata_left();
			break;
		case 0x23: // Kanji ROM
			retval = (uint32) read_kanjidata_right();
			break;
#if defined(CAPABLE_KANJI_CLASS2)
		case 0x2e: // Kanji ROM Level2
			retval = (uint32) read_kanjidata_left_l2();
			break;
		case 0x2f: // Kanji ROM Level2
			retval = (uint32) read_kanjidata_right_l2();
			break;
#endif
		case 0x37: // Multi page
			//retval = (uint32)display->read_data8(DISPLAY_ADDR_MULTIPAGE);
			break;
		case 0x45: // WHG CMD
			break;
		case 0x46: // WHG DATA
			retval = (uint32) get_opn(1);
			break;
		case 0x47:
			retval = (uint32) get_extirq_whg();
			break;
		case 0x51: // THG CMD
			break;
		case 0x52: // THG DATA
		        retval = (uint32) get_opn(2);
			break;
		case 0x53:
			retval = (uint32) get_extirq_thg();
			break;
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
			//p_emu->out_debug_log(_T("IO: Read DMA %02x from reg %02x\n"), retval, dma_addr);
			break;
#endif			
		default:
			//printf("MAIN: Read another I/O Addr=%08x\n", addr); 
			break;
		}
		if((addr < 0x40) && (addr >= 0x38)) {
			addr = (addr - 0x38) + FM7_SUBMEM_OFFSET_DPALETTE;
			return (uint32) display->read_data8(addr);
		}
		// Another:
		return retval;
	} else if(addr == FM7_MAINIO_CLOCKMODE) {
		return (uint32)get_clockmode();
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

void FM7_MAINIO::write_dma_io8(uint32 addr, uint32 data)
{
	this->write_data8(addr & 0xff, data);
}

void FM7_MAINIO::write_dma_data8(uint32 addr, uint32 data)
{
	this->write_data8(addr & 0xff, data);
}


void FM7_MAINIO::write_data8(uint32 addr, uint32 data)
{
	bool flag;
	uint32 mmr_segment;
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
			set_port_fd00((uint8)data);
			return;
			break;
		case 0x01: // FD01
			// set_lptdata_fd01((uint8)data);
			break;
		case 0x02: // FD02
			set_port_fd02((uint8)data);
			break;
		case 0x03: // FD03
			set_beep(data);
			break;
		case 0x04: // FD04
			set_fd04(data);
			break;
		case 0x05: // FD05
	  		set_fd05((uint8)data);
			break;
		case 0x06: // RS-232C
		case 0x07:
			break;
		case 0x08: // Light pen
		case 0x09:
		case 0x0a:
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
			mainmem->write_signal(FM7_MAINIO_INITROM_ENABLED, (flag) ? 0xffffffff : 0 , 0xffffffff);
			break;
		case 0x12:
			display->write_signal(SIG_DISPLAY_MODE320, data,  0x40);
			reg_fd12 &= ~0x40;
			reg_fd12 |= (data & 0x40);
			break;
		case 0x13:
			sub_monitor_type = data & 0x07;
				display->write_signal(SIG_FM7_SUB_BANK, sub_monitor_type, 0x07);
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
			set_ext_fd17((uint8)data);
			break;
		case 0x18: // FDC: COMMAND
			set_fdc_cmd((uint8)data);
			//printf("FDC: WRITE CMD %02x\n", data); 
			break;
		case 0x19: // FDC: Track
			set_fdc_track((uint8)data);
			//printf("FDC: WRITE TRACK REG %02x\n", data); 
			break;
		case 0x1a: // FDC: Sector
			set_fdc_sector((uint8)data);
			//printf("FDC: WRITE SECTOR REG %02x\n", data); 
			break;
   		case 0x1b: // FDC: Data
			set_fdc_data((uint8)data);
			break;
		case 0x1c:
			set_fdc_fd1c((uint8)data);
			//printf("FDC: WRITE HEAD REG %02x\n", data); 
			break;
		case 0x1d:
			set_fdc_fd1d((uint8)data);
			//printf("FDC: WRITE MOTOR REG %02x\n", data); 
			break;
		case 0x1e:
			set_fdc_fd1e((uint8)data);
			break;
		case 0x1f: // ??
			return;
			break;
		case 0x20: // Kanji ROM
			write_kanjiaddr_hi((uint8)data);
			break;
		case 0x2c: // Kanji ROM(DUP)
#if defined(CAPABLE_KANJI_CLASS2)
			write_kanjiaddr_hi_l2((uint8)data);
#else			
			write_kanjiaddr_hi((uint8)data);
#endif
			break;
		case 0x21: // Kanji ROM
			write_kanjiaddr_lo((uint8)data);
			break;
		case 0x2d: // Kanji ROM(DUP)
#if defined(CAPABLE_KANJI_CLASS2)
			write_kanjiaddr_lo_l2((uint8)data);
#else
			write_kanjiaddr_lo((uint8)data);
#endif
			break;
#if defined(CAPABLE_DICTROM)
		case 0x2e: // 
			mainmem->write_signal(FM7_MAINIO_EXTBANK, data, 0xff);
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
		case 0x37: // Multi page
			display->write_signal(SIG_DISPLAY_MULTIPAGE, data, 0x00ff);
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
#if defined(HAS_MMR)
		case 0x90:
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
			mmr_segment = data & 7;
#else
			//			printf("MMR SEGMENT: %02x\n", data & 3);
			mmr_segment = data & 3;
#endif			
			mainmem->write_data8(FM7_MAINIO_MMR_SEGMENT, (uint32)mmr_segment);
			break;
		case 0x92:
			mainmem->write_data8(FM7_MAINIO_WINDOW_OFFSET, (uint32)(data & 0x00ff));
			break;
		case 0x93:
   			mainmem->write_signal(FM7_MAINIO_BOOTRAM_RW, data, 0x01);
			mainmem->write_signal(FM7_MAINIO_WINDOW_ENABLED, data , 0x40);
			//this->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
			//mainmem->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
			mainmem->write_signal(FM7_MAINIO_MMR_ENABLED, data, 0x80);
			//}
			break;
#endif
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
		case 0x94:
			mainmem->write_signal(FM7_MAINIO_MMR_EXTENDED, data, 0x80);
			mainmem->write_signal(FM7_MAINMEM_REFRESH_FAST, data, 0x04);
			mainmem->write_signal(FM7_MAINIO_WINDOW_FAST , data, 0x01);

			break;
# if defined(_FM77AV40SX) || defined(_FM77AV40EX)
		case 0x95:
			mainmem->write_signal(FM7_MAINIO_FASTMMR_ENABLED, data, 0x08);
			mainmem->write_signal(FM7_MAINIO_EXTROM, data , 0x80);
			break;
# endif
#endif			
#if defined(HAS_DMA)
		case 0x98:
			dma_addr = data & 0x1f;
			break;
		case 0x99:
			dmac->write_data8(dma_addr, data);
			//p_emu->out_debug_log(_T("IO: Wrote DMA %02x to reg %02x\n"), data, dma_addr);
			break;
#endif			
		default:
			//printf("MAIN: Write I/O Addr=%08x DATA=%02x\n", addr, data); 
			break;
		}
		if((addr < 0x40) && (addr >= 0x38)) {
			addr = (addr - 0x38) | FM7_SUBMEM_OFFSET_DPALETTE;
			display->write_data8(addr, (uint8)data);
			return;
		}// Another:
		return;
	} else if(addr == FM7_MAINIO_CLOCKMODE) {
		set_clockmode((uint8)data);
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
			set_break_key(false);
			break;
		case EVENT_TIMERIRQ_ON:
			//if(!irqmask_timer) set_irq_timer(true);
			set_irq_timer(true);
			break;
		case EVENT_FD_MOTOR_ON:
			set_fdc_motor(true);
			event_fdc_motor = -1;
			break;
		case EVENT_FD_MOTOR_OFF:
			set_fdc_motor(false);
			event_fdc_motor = -1;
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
	this->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
	//mainmem->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
}

void FM7_MAINIO::event_vline(int v, int clock)
{
}

#define STATE_VERSION 3
void FM7_MAINIO::save_state(FILEIO *state_fio)
{
	int ch;
	int addr;
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);

	// Version 1
	{
		for(addr = 0; addr < 0x100; addr++) state_fio->FputUint8(io_w_latch[addr]);
		// FD00
		state_fio->FputBool(clock_fast);
		state_fio->FputBool(lpt_strobe);
		state_fio->FputBool(lpt_slctin);
		state_fio->FputBool(beep_flag);
		state_fio->FputBool(beep_snd);
	
		// FD01
		state_fio->FputUint8(lpt_outdata);
		// FD02
		state_fio->FputBool(cmt_indat);
		state_fio->FputBool(cmt_invert);
		state_fio->FputBool(lpt_det2);
		state_fio->FputBool(lpt_det1);
		state_fio->FputBool(lpt_pe);
		state_fio->FputBool(lpt_ackng_inv);
		state_fio->FputBool(lpt_error_inv);
		state_fio->FputUint8(irqmask_reg0);
		
		state_fio->FputBool(irqmask_syndet);
		state_fio->FputBool(irqmask_rxrdy);
		state_fio->FputBool(irqmask_txrdy);
		state_fio->FputBool(irqmask_mfd);
		state_fio->FputBool(irqmask_timer);
		state_fio->FputBool(irqmask_printer);
		state_fio->FputBool(irqmask_keyboard);

		state_fio->FputBool(irqreq_syndet);
		state_fio->FputBool(irqreq_rxrdy);
		state_fio->FputBool(irqreq_txrdy);
		state_fio->FputBool(irqreq_fdc);
		state_fio->FputBool(irqreq_timer);
		state_fio->FputBool(irqreq_printer);
		state_fio->FputBool(irqreq_keyboard);
		// FD03
		state_fio->FputUint8(irqstat_reg0);
		
		state_fio->FputBool(irqstat_timer);
		state_fio->FputBool(irqstat_printer);
		state_fio->FputBool(irqstat_keyboard);
		
		// FD04
#if defined(_FM77_VARIANTS)
		state_fio->FputBool(stat_fdmode_2hd);
		state_fio->FputBool(stat_kanjirom);
		state_fio->FputBool(stat_400linecard);
#elif defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
      defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
		state_fio->FputBool(stat_kanjirom);
#endif
		state_fio->FputBool(firq_break_key);
		state_fio->FputBool(firq_sub_attention);
		
		state_fio->FputBool(intmode_fdc);
		// FD05
		state_fio->FputBool(extdet_neg);
		//state_fio->FputBool(sub_busy);
		state_fio->FputBool(sub_halt);
		//state_fio->FputBool(sub_halt_bak);
		state_fio->FputBool(sub_cancel);
		//state_fio->FputBool(sub_cancel_bak);
#if defined(WITH_Z80)	
		state_fio->FputBool(z80_sel);
#endif	
		// FD06, 07
		state_fio->FputBool(intstat_syndet);
		state_fio->FputBool(intstat_rxrdy);
		state_fio->FputBool(intstat_txrdy);

	// FD0B
	// FD0F
	
		state_fio->FputBool(connect_opn);
		state_fio->FputBool(connect_whg);
		state_fio->FputBool(connect_thg);
		
		state_fio->FputBool(opn_psg_77av);
	
		for(ch = 0; ch < 4; ch++) {
			state_fio->FputUint32_BE(opn_address[ch]);
			state_fio->FputUint32_BE(opn_data[ch]);
			state_fio->FputUint32_BE(opn_stat[ch]);
			state_fio->FputUint32_BE(opn_cmdreg[ch]);
			state_fio->FputUint32_BE(opn_ch3mode[ch]);
		}
		state_fio->FputUint32_BE(joyport_a);
		state_fio->FputUint32_BE(joyport_b);

		state_fio->FputBool(intstat_opn);
		state_fio->FputBool(intstat_mouse);
		state_fio->FputBool(mouse_enable);
	
		state_fio->FputBool(intstat_whg);
		state_fio->FputBool(intstat_thg);

	// FDC
		state_fio->FputBool(connect_fdc);
		state_fio->FputUint8(fdc_statreg);
		state_fio->FputUint8(fdc_cmdreg);
		state_fio->FputUint8(fdc_trackreg);
		state_fio->FputUint8(fdc_sectreg);
		state_fio->FputUint8(fdc_datareg);
		state_fio->FputUint8(fdc_headreg);
		state_fio->FputUint8(fdc_drvsel);
		state_fio->FputUint8(irqreg_fdc);
		state_fio->FputBool(fdc_motor);
		state_fio->FputBool(irqstat_fdc);
		// KANJI ROM
		state_fio->FputBool(connect_kanjiroml1);
#if defined(_FM77AV_VARIANTS)
		state_fio->FputBool(connect_kanjiroml2);
	
		state_fio->FputBool(boot_ram);
		state_fio->FputBool(hotreset);
		// FD13
		state_fio->FputUint8(sub_monitor_type);
#endif	
		// MMR
	}
	//V2
	{
		state_fio->FputInt32_BE(event_beep);
		state_fio->FputInt32_BE(event_beep_oneshot);
		state_fio->FputInt32_BE(event_timerirq);
	}		
	{ // V3
		state_fio->FputInt32_BE(event_fdc_motor);
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)|| \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
		for(ch = 0; ch < 4; ch++) state_fio->FputUint8(fdc_drive_table[ch]);
		state_fio->FputUint8(fdc_reg_fd1e);
#endif	
#if defined(HAS_DMA)
		state_fio->FputBool(intstat_dma);
		state_fio->FputUint8(dma_addr & 0x1f);
#endif			
#if defined(_FM77AV_VARIANTS)
		state_fio->FputUint8(reg_fd12);
#endif		
	}		
}

bool FM7_MAINIO::load_state(FILEIO *state_fio)
{
	int ch;
	int addr;
	//bool stat = false;
	uint32 version;
	
	version = state_fio->FgetUint32_BE();
	if(this_device_id != state_fio->FgetInt32_BE()) return false;

	if(version >= 1) {
		for(addr = 0; addr < 0x100; addr++) io_w_latch[addr] = state_fio->FgetUint8();
		// FD00
		clock_fast = state_fio->FgetBool();
		lpt_strobe = state_fio->FgetBool();
		lpt_slctin = state_fio->FgetBool();
		beep_flag  = state_fio->FgetBool();
		beep_snd = state_fio->FgetBool();
	
		// FD01
		lpt_outdata = state_fio->FgetUint8();
		// FD02
		cmt_indat = state_fio->FgetBool();
		cmt_invert = state_fio->FgetBool();
		lpt_det2 = state_fio->FgetBool();
		lpt_det1 = state_fio->FgetBool();
		lpt_pe = state_fio->FgetBool();
		lpt_ackng_inv = state_fio->FgetBool();
		lpt_error_inv = state_fio->FgetBool();
		irqmask_reg0 = state_fio->FgetUint8();

		irqmask_syndet = state_fio->FgetBool();
		irqmask_rxrdy = state_fio->FgetBool();
		irqmask_txrdy = state_fio->FgetBool();
		irqmask_mfd = state_fio->FgetBool();
		irqmask_timer = state_fio->FgetBool();
		irqmask_printer = state_fio->FgetBool();
		irqmask_keyboard = state_fio->FgetBool();

		irqreq_syndet = state_fio->FgetBool();
		irqreq_rxrdy = state_fio->FgetBool();
		irqreq_txrdy = state_fio->FgetBool();
		irqreq_fdc = state_fio->FgetBool();
		irqreq_timer = state_fio->FgetBool();
		irqreq_printer = state_fio->FgetBool();
		irqreq_keyboard = state_fio->FgetBool();
		// FD03
		irqstat_reg0 = state_fio->FgetUint8();

		irqstat_timer = state_fio->FgetBool();
		irqstat_printer = state_fio->FgetBool();
		irqstat_keyboard = state_fio->FgetBool();
	
		// FD04
#if defined(_FM77_VARIANTS)
		stat_fdmode_2hd = state_fio->FgetBool();
		stat_kanjirom = state_fio->FgetBool();
		stat_400linecard = state_fio->FgetBool();
#elif defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
      defined(_FM77AV20) || defined(_FM77AV20EX) || defined(_FM77AV20SX)
		stat_kanjirom = state_fio->FgetBool();
#endif
		firq_break_key = state_fio->FgetBool();
		firq_sub_attention = state_fio->FgetBool();
		
		intmode_fdc = state_fio->FgetBool();
		// FD05
		extdet_neg = state_fio->FgetBool();
		//sub_busy = state_fio->FgetBool();
		sub_halt = state_fio->FgetBool();
		//sub_halt_bak = state_fio->FgetBool();
		sub_cancel = state_fio->FgetBool();
		//sub_cancel_bak = state_fio->FgetBool();
#if defined(WITH_Z80)	
		z80_sel = state_fio->FgetBool();
#endif	
		// FD06, 07
		intstat_syndet = state_fio->FgetBool();
		intstat_rxrdy = state_fio->FgetBool();
		intstat_txrdy = state_fio->FgetBool();

		// FD0B
		// FD0F
		connect_opn = state_fio->FgetBool();
		connect_whg = state_fio->FgetBool();
		connect_thg = state_fio->FgetBool();

		opn_psg_77av = state_fio->FgetBool();
	
		for(ch = 0; ch < 4; ch++) {
			opn_address[ch] = state_fio->FgetUint32_BE();
			opn_data[ch] = state_fio->FgetUint32_BE();
			opn_stat[ch] = state_fio->FgetUint32_BE();
			opn_cmdreg[ch] = state_fio->FgetUint32_BE();
			opn_ch3mode[ch] = state_fio->FgetUint32_BE();
		}
		joyport_a = state_fio->FgetUint32_BE();
		joyport_b = state_fio->FgetUint32_BE();

		intstat_opn = state_fio->FgetBool();
		intstat_mouse = state_fio->FgetBool();
		mouse_enable = state_fio->FgetBool();
	
		intstat_whg = state_fio->FgetBool();
		intstat_thg = state_fio->FgetBool();

		// FDC
		connect_fdc = state_fio->FgetBool();
		fdc_statreg = state_fio->FgetUint8();
		fdc_cmdreg = state_fio->FgetUint8();
		fdc_trackreg = state_fio->FgetUint8();
		fdc_sectreg = state_fio->FgetUint8();
		fdc_datareg = state_fio->FgetUint8();
		fdc_headreg = state_fio->FgetUint8();
		fdc_drvsel = state_fio->FgetUint8();
		irqreg_fdc = state_fio->FgetUint8();
		fdc_motor = state_fio->FgetBool();
		irqstat_fdc = state_fio->FgetBool();

		// KANJI ROM
		connect_kanjiroml1 = state_fio->FgetBool();
#if defined(_FM77AV_VARIANTS)
		connect_kanjiroml2 = state_fio->FgetBool();
		boot_ram = state_fio->FgetBool();
		hotreset = state_fio->FgetBool();
		// FD13
		sub_monitor_type = state_fio->FgetUint8();
#endif	
	}
	if(version >= 2) {
		event_beep = state_fio->FgetInt32_BE();
		event_beep_oneshot = state_fio->FgetInt32_BE();
		event_timerirq = state_fio->FgetInt32_BE();
	}
	// V2
	if(version >= 3) { // V3
		event_fdc_motor = state_fio->FgetInt32_BE();
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)|| \
    defined(_FM77AV20) || defined(_FM77AV20SX) || defined(_FM77AV20EX)
		for(ch = 0; ch < 4; ch++) fdc_drive_table[ch] = state_fio->FgetUint8();
		fdc_reg_fd1e = state_fio->FgetUint8();
#endif	
#if defined(HAS_DMA)
		intstat_dma = state_fio->FgetBool();
		dma_addr = (uint32)(state_fio->FgetUint8() & 0x1f);
#endif			
#if defined(_FM77AV_VARIANTS)
		reg_fd12 = state_fio->FgetUint8();
#endif		
	}		
	return true;
}
	  
