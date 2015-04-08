/*
 * FM-7 Main I/O [fm7_mainio.h]
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Jan 03, 2015 : Initial
 *
 */

#include "fm7_mainio.h"

#include "../mc6809.h"
#include "../z80.h"

#include "../datarec.h"

// TEST
//#include <SDL2/SDL.h>

void FM7_MAINIO::initialize(void)
{
	int i;
	event_beep = -1;
	event_timerirq = -1;
	bootmode = config.boot_mode & 3;
#if defined(_FM77AV_VARIANTS)
	opn_psg_77av = true;
#else
	//opn_psg_77av = true;
	opn_psg_77av = false;
#endif
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	boot_ram = false;
#endif 
#if defined(_FM77AV_VARIANTS)
	enable_initiator = true;
#endif
#ifdef HAS_MMR
	mmr_enabled = false;
	mmr_fast = false;
	window_enabled = false;
	mmr_segment = 0x00;
	for(i = 0x00; i < 0x80; i++) mmr_table[i] = 0;
	//	for(i = 0x00; i < 0x10; i++) mmr_table[i] = 0x30 + i;
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
}

void FM7_MAINIO::reset(void)
{
	int i, j;
	uint8 data;
	//if(event_beep >= 0) cancel_event(this, event_beep);
	if(event_timerirq >= 0) cancel_event(this, event_timerirq);
	event_beep = -1;
	beep_snd = true;
	beep_flag = false;
	extdet_neg = false;
   
	stat_romrammode = true;
	bootmode = config.boot_mode & 3;
	if(bootmode == 0) { // IF BASIC BOOT THEN ROM
		stat_romrammode = true;
	} else { // ELSE RAM
		stat_romrammode = false;
	}
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	boot_ram = false;
	kaddress.d = 0;
#endif 
#if defined(_FM77AV_VARIANTS)
	mode320 = false;
	sub_monitor_type = 0x00;
#endif
	
#ifdef HAS_MMR
	mmr_enabled = false;
	mmr_fast = false;
	window_enabled = false;
#endif
	clock_fast = false;
	if(config.cpu_type == 0) clock_fast = true;
   
	reset_sound();
	
	irqmask_reg0 = 0x00;
	irqstat_bak = false;
	firqstat_bak = false;
	// FD03
	irqmask_mfd = true;
	irqmask_timer = true;
	irqmask_printer = true;
	irqmask_keyboard = true;
	irqstat_reg0 = 0xff;
	irqstat_timer = false;
	irqstat_printer = false;
	irqstat_keyboard = false;
   
	// FD04
	//firq_break_key = false; // bit1, ON = '0'.
	firq_sub_attention = false; // bit0, ON = '0'.
	// FD05
	extdet_neg = false;
	sub_cancel = false; // bit6 : '1' Cancel req.

	nmi_count = 0;
	reset_fdc();
   
	register_event(this, EVENT_TIMERIRQ_ON, 10000.0 / 4.9152, true, &event_timerirq); // TIMER IRQ
	mainmem->reset();
	//maincpu->reset();
}


void FM7_MAINIO::set_clockmode(uint8 flags)
{
	uint f = clock_fast;
	if((flags & FM7_MAINCLOCK_SLOW) != 0) {
		clock_fast = false;
	} else {
		clock_fast = true;
	}
	if(f != clock_fast) {
		this->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
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
	if(kbd_bit8)   ret |= 0x80; //0b10000000;
	if(clock_fast) ret |= 0x01; //0b00000001;
	return ret;
}
  
void FM7_MAINIO::set_port_fd00(uint8 data)
{
       drec->write_signal(SIG_DATAREC_OUT, data, 0x01);
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
	bool keyirq_bak = irqmask_keyboard;
	bool timerirq_bak = irqmask_timer;
	bool printerirq_bak = irqmask_printer;
	bool mfdirq_bak = irqmask_mfd;
	bool flag;
	//	if((val & 0b00010000) != 0) {
	if((val & 0x10) != 0) {
	irqmask_mfd = false;
	} else {
		irqmask_mfd = true;
	}
	if(mfdirq_bak != irqmask_mfd) {
   		flag = irqstat_fdc;
   		set_irq_mfd(flag);
	}
	//if((val & 0b00000100) != 0) {
	if((val & 0x04) != 0) {
		irqmask_timer = false;
	} else {
		irqmask_timer = true;
	}
	if(timerirq_bak != irqmask_timer) {
   		flag = irqstat_timer;
   		set_irq_timer(flag);
	}
	//if((val & 0b00000010) != 0) {
	if((val & 0x02) != 0) {
		irqmask_printer = false;
	} else {
		irqmask_printer = true;
	}
	if(printerirq_bak != irqmask_printer) {
   		flag = irqstat_printer;
   		set_irq_printer(flag);
	}
   
	//if((val & 0b00000001) != 0) {
	if((val & 0x01) != 0) {
		irqmask_keyboard = false;
	} else {
		irqmask_keyboard = true;
	}
	if(keyirq_bak != irqmask_keyboard) {
   		flag = irqstat_keyboard;
		flag = flag & !irqmask_keyboard;
		display->write_signal(SIG_FM7_SUB_KEY_FIRQ, flag ? 1 : 0, 1);
		//printf("KEYBOARD: Interrupted %d\n", flag);
		irqmask_keyboard = flag;
		do_irq(flag);
	}
   
	return;
}


uint32 FM7_MAINIO::get_keyboard(void)
{
	uint32 kbd_data = (uint32) kbd_bit7_0;
	kbd_data &= 0x0ff;
	if(kbd_bit8) kbd_data |= 0x0100;
	return kbd_data;
}



void FM7_MAINIO::set_irq_timer(bool flag)
{
	uint8 backup = irqstat_reg0;
	if(flag && !(irqmask_timer)) {
		irqstat_reg0 &= ~0x04;
		irqstat_timer = true;	   
		if(backup != irqstat_reg0) do_irq(true);
	} else {
		irqstat_reg0 |= 0x04;
		irqstat_timer = false;	   
		if(backup != irqstat_reg0) do_irq(false);
	}
	//printf("IRQ TIMER: %02x MASK=%d\n", irqstat_reg0, irqmask_timer);
}

void FM7_MAINIO::set_irq_printer(bool flag)
{
	uint8 backup = irqstat_reg0;
	if(flag && !(irqmask_printer)) {
		irqstat_reg0 &= ~0x02;
		irqstat_printer = true;	   
		if(backup != irqstat_reg0) do_irq(true);
	} else {
		irqstat_reg0 |= 0x02;
		irqstat_printer = false;	   
		if(backup != irqstat_reg0) do_irq(false);
	}
//	if(!irqmask_printer || !flag) do_irq(flag);
}

void FM7_MAINIO::set_irq_keyboard(bool flag)
{
	uint8 backup = irqstat_reg0;
	if(irqmask_keyboard) return;
	if(flag) {
		irqstat_reg0 &= ~0x01;
		irqstat_keyboard = true;
		if(backup != irqstat_reg0) do_irq(true);
	} else {
		//irqstat_reg0 &= 0b11111110;
		irqstat_reg0 |= 0x01;
		irqstat_keyboard = false;	   
		if(backup != irqstat_reg0) do_irq(false);
	}
   	//printf("MAIN: KEYBOARD: IRQ=%d\n", flag && !(irqmask_keyboard));
}

void FM7_MAINIO::set_keyboard(uint32 data)
{
	if((data & 0x100) != 0){
		kbd_bit8 = true;
	} else {
		kbd_bit8 = false;
	}
	kbd_bit7_0 = (data & 0xff);
}


void FM7_MAINIO::do_irq(bool flag)
{
	bool intstat;
	intstat = irqstat_timer | irqstat_keyboard | irqstat_printer;
	intstat = intstat | irqstat_fdc;
       	intstat = intstat | intstat_opn | intstat_whg | intstat_thg;
       	intstat = intstat | intstat_mouse;
   
	if(irqstat_bak == intstat) return;
	//printf("%08d : IRQ: REG0=%02x FDC=%02x, stat=%d\n", SDL_GetTicks(), irqstat_reg0, irqstat_fdc, intstat);
	if(intstat) {
		maincpu->write_signal(SIG_CPU_IRQ, 1, 1);
	} else {
		maincpu->write_signal(SIG_CPU_IRQ, 0, 1);
	}
	irqstat_bak = intstat;
}

void FM7_MAINIO::do_firq(bool flag)
{
	bool firq_stat;
	firq_stat = firq_break_key || firq_sub_attention; 
	//printf("%08d : FIRQ: break=%d attn=%d stat = %d\n", SDL_GetTicks(), firq_break_key, firq_sub_attention, firq_stat);
	//if(firqstat_bak == firq_stat) return;
	if(firq_stat) {
		maincpu->write_signal(SIG_CPU_FIRQ, 1, 1);
	} else {
		maincpu->write_signal(SIG_CPU_FIRQ, 0, 1);
	}
	firqstat_bak = firq_stat;
}

void FM7_MAINIO::do_nmi(bool flag)
{
	if(flag) {
		if(nmi_count >= 0x7ff0) {
	  		nmi_count = 0x7ff0;
			return;
		}
		nmi_count++;
		if(nmi_count <= 1) maincpu->write_signal(SIG_CPU_NMI, 1, 1);
	} else {
		if(nmi_count <= 0) {
			nmi_count = 0;
			return;
		}
		nmi_count--;
		if(nmi_count == 0) maincpu->write_signal(SIG_CPU_NMI, 0, 1);
	}
}


void FM7_MAINIO::set_break_key(bool pressed)
{
	firq_break_key = pressed;
	do_firq(pressed);
}

void FM7_MAINIO::set_sub_attention(bool flag)
{
	firq_sub_attention = flag;
	do_firq(flag); 
}
  

uint8 FM7_MAINIO::get_fd04(void)
{
	uint8 val = display->read_signal(SIG_DISPLAY_BUSY) | ~0x83;
	if(!firq_break_key)     val |= 0x02;
	if(!firq_sub_attention) {
		val |= 0x01;
	}
	set_sub_attention(false);   
	return val;
}

void FM7_MAINIO::set_fd04(uint8 val)
{
	// NOOP?
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	display->write_signal(SIG_DISPLAY_EXTRA_MODE, val, 0xff);
#endif
}

  // FD05
 uint8 FM7_MAINIO::get_fd05(void)
{
	uint8 val = display->read_signal(SIG_DISPLAY_BUSY) | ~0x81;
	if(!extdet_neg) val |= 0x01;
	return val;
}

 void FM7_MAINIO::set_fd05(uint8 val)
{
	display->write_signal(SIG_FM7_SUB_CANCEL, val, 0x40); // HACK
	display->write_signal(SIG_DISPLAY_HALT,   val, 0x80);
#ifdef WITH_Z80
	if((val & 0x01) != 0) {
		//maincpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		//z80->write_signal(SIG_CPU_BUSREQ, 0, 1);
	} else {
		//maincpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
		//z80->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
#endif
}

void FM7_MAINIO::set_extdet(bool flag)
{
	extdet_neg = flag;
}

// Kanji ROM, FD20 AND FD21 (or SUBSYSTEM)
void FM7_MAINIO::write_kanjiaddr_hi(uint8 addr)
{
	if(!connect_kanjiroml1) return;
	kaddress.b.h = addr;
	return;
}

void FM7_MAINIO::write_kanjiaddr_lo(uint8 addr)
{
	if(!connect_kanjiroml1) return;
	kaddress.b.l = addr;
	return;
}

uint8 FM7_MAINIO::read_kanjidata_left(void)
{
	uint32 addr;
    
	if(!connect_kanjiroml1) return 0xff;
	addr = kaddress.w.l;
	addr = addr << 1;
	//printf("KANJI MAIN CLASS1 ADDR: %05x\n", kaddress.w.l);
	if(kanjiclass1) {
		return kanjiclass1->read_data8(addr);
	} else {
		return 0xff;
	}
}

uint8 FM7_MAINIO::read_kanjidata_right(void)
{
	uint32 addr;
    
	if(!connect_kanjiroml1) return 0xff;
	addr = kaddress.w.l;
	addr = (addr << 1) + 1;
	if(kanjiclass1) {
		return kanjiclass1->read_data8(addr);
	} else {
		return 0xff;
	}
}

#ifdef CAPABLE_KANJICLASS2
// Kanji ROM, FD20 AND FD21 (or SUBSYSTEM)
void FM7_MAINIO::write_kanjiaddr_hi_l2(uint8 addr)
{
	if(!connect_kanjiroml2) return;
	kaddress_l2.b.h = addr;
	return;
}

void FM7_MAINIO::write_kanjiaddr_lo_l2(uint8 addr)
{
	if(!connect_kanjiroml2) return;
	kaddress_l2.b.l = addr;
	return;
}

uint8 FM7_MAINIO::read_kanjidata_left_l2(void)
{
	uint32 addr;
    
	if(!connect_kanjiroml2) return 0xff;
	addr = kaddress.w.l;
	addr = addr << 1;
	if(kanjiclass2) {
		return kanjiclass2->read_data8(addr);
	} else {
		return 0xff;
	}
}

uint8 FM7_MAINIO::read_kanjidata_right_l2(void)
{
	uint32 addr;
    
	if(!connect_kanjiroml2) return 0xff;
	addr = kaddress_l2.w.l;
	addr = (addr << 1) + 0x01;
	if(kanjiclass2) {
		return kanjiclass2->read_data8(addr);
	} else {
		return 0xff;
	}
}
#endif



void FM7_MAINIO::write_signal(int id, uint32 data, uint32 mask)
{
	bool val_b;
	val_b = ((data & mask) != 0);
  
	switch(id) {
		case FM7_MAINIO_CLOCKMODE: // fd00
			if(val_b) {
				clock_fast = true;
			} else {
				clock_fast = false;
			}
			{
				uint32 clocks;
				uint32 subclocks;
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
				if(mmr_enabled) {
					if(mmr_fast) {
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
		case FM7_MAINIO_PUSH_KEYBOARD:
			set_keyboard(data & 0x1ff);
			break;
			// FD04
		case FM7_MAINIO_PUSH_BREAK:
			set_break_key(val_b);
			break;
		case FM7_MAINIO_SUB_ATTENTION:
			set_sub_attention(val_b);
			break;
			// FD05
		case FM7_MAINIO_SUB_BUSY:
			break;
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
			do_irq(val_b);
       			break;
		case FM7_MAINIO_WHG_IRQ:
			if(!connect_whg) break;
			intstat_whg = val_b;
			do_irq(val_b);
       			break;
		case FM7_MAINIO_THG_IRQ:
			if(!connect_thg) break;
			intstat_thg = val_b;
			do_irq(val_b);
       			break;
		case FM7_MAINIO_FDC_DRQ:
			set_drq_mfd(val_b);
			break;
		case FM7_MAINIO_FDC_IRQ:
			set_irq_mfd(val_b);
			break;
		case FM7_MAINIO_KANJI1_ADDR_HIGH:
	  		kaddress.b.h = data;
			break;
		case FM7_MAINIO_KANJI1_ADDR_LOW:
	  		kaddress.b.l = data;
			break;
#if defined(CAPABLE_KANJI_CLASS2)
		case FM7_MAINIO_KANJI2_ADDR_HIGH:
	  		kaddress_l2.b.h = data;
			break;
		case FM7_MAINIO_KANJI2_ADDR_LOW:
	  		kaddress_l2.b.l = data;
			break;
#endif
	}
	
}


 uint8 FM7_MAINIO::get_irqstat_fd03(void)
{
	uint8 val;
	bool extirq = false;
	
	extirq = irqstat_fdc | intstat_opn | intstat_whg | intstat_thg;
	
	//extirq = extirq | intstat_syndet | intstat_rxrdy | intstat_txrdy;
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
	if(intstat_opn)   val &= ~0x08;
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
	uint8 retval;
	retval = (display->read_signal(SIG_DISPLAY_MODE320) != 0) ? 0x40 : 0;
	retval |= display->read_signal(SIG_DISPLAY_VSYNC);
	retval |= display->read_signal(SIG_DISPLAY_DISPLAY);
	retval |= ~0x43;
	return retval;
}
#endif

uint32 FM7_MAINIO::read_signal(uint32 addr)
{
	uint32 retval = 0xffffffff;
	switch(addr) {
	}
	return retval;
}

uint32 FM7_MAINIO::read_data8(uint32 addr)
{
	uint32 retval;
	if(addr == FM7_MAINIO_IS_BASICROM) {
		retval = 0;
		if(stat_bootsw_basic) retval = 0xffffffff;
		return retval;
	} else if(addr == FM7_MAINIO_BOOTMODE) {
		retval = bootmode & 0x03;
#if defined(_FM77) || defined(_FM77L2) || defined(_FM77L4) || defined(_FM77AV_VARIANTS)
		if(boot_ram) retval = 4;
#endif
		return retval;
	} else if(addr == FM7_MAINIO_READ_FD0F) {
		if(stat_romrammode) return 0xffffffff;
		return 0;
	} else if(addr == FM7_MAINIO_CLOCKMODE) {
		return (uint32)get_clockmode();
	}
#if defined(HAS_MMR)	
	else if(addr == FM7_MAINIO_MMR_ENABLED) {
		retval = (mmr_enabled) ? 0xffffffff:0x00000000;
		return retval;
	} else if(addr == FM7_MAINIO_WINDOW_ENABLED) {
		retval = (window_enabled) ? 0xffffffff:0x00000000;
		return retval;
	} else if(addr == FM7_MAINIO_WINDOW_OFFSET) {
		retval = (uint32)window_offset;
		return retval;
	} else if(addr == FM7_MAINIO_MMR_SEGMENT) {
		retval = (uint32) mmr_segment;
		return retval;
	} else if((addr >= FM7_MAINIO_MMR_BANK) &&  (addr < (FM7_MAINIO_MMR_BANK + 16))) {
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
		retval = (uint32)mmr_table[(addr - FM7_MAINIO_MMR_BANK) | (mmr_segment & 7) * 16];
#else	
		retval = (uint32)mmr_table[(addr - FM7_MAINIO_MMR_BANK) | (mmr_segment & 3)  * 16] & 0x3f;
#endif
		return retval;
	}
#endif
#if defined(_FM77AV_VARIANTS)
	else if(addr == FM7_MAINIO_INITROM_ENABLED) {
		retval = (enable_initiator) ? 0xffffffff : 0x00000000;
		return retval;
	} else if(addr == FM7_MAINIO_MODE320) {
		retval = display->read_signal(SIG_DISPLAY_MODE320);
		return retval;
	} else if(addr == FM7_MAINIO_SUBMONITOR_ROM) {
		retval = sub_monitor_type & 0x03;
		return retval;
	}  else if(addr == FM7_MAINIO_SUBMONITOR_RAM) {
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
		retval = ((sub_monitor_type & 0x04) != 0) ? 0xffffffff : 0x00000000;
#else
		retval = 0;
#endif
		return retval;
	}
#endif
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
	else if(addr == FM7_MAINIO_EXTBANK) {
	} else if(addr == FM7_MAINIO_EXTROM) {
	}
#endif
	//if((addr >= 0x0006) && (addr != 0x1f)) printf("MAINIO: READ: %08x DATA=%08x\n", addr);
	addr = addr & 0xff;
	retval = 0xff;
#if defined(HAS_MMR)
	if((addr < 0x90) && (addr >= 0x80)) {
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
		return mmr_table[addr - 0x80 + mmr_segment * 16];
#else
		return mmr_table[addr - 0x80 + (mmr_segment & 0x03) * 16];
#endif	  
	}
#endif
	//	if((addr >= 0x0006) && !(addr == 0x1f) && !(addr == 0x0b)) printf("MAINIO: READ: %08x \n", addr);
	switch(addr) {
		case 0x00: // FD00
			retval = (uint32) get_port_fd00();
			break;
		case 0x01: // FD01
			display->write_signal(SIG_FM7_SUB_KEY_FIRQ, 0, 1);
			set_irq_keyboard(false);
			retval = (uint32) kbd_bit7_0;
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
			retval = (bootmode == 0) ? 0xfe : 0xff;
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
			if(boot_ram) retval |= 0x01;
			if(window_enabled) retval |= 0x40;
			if(mmr_enabled) retval |= 0x80;
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
}

void FM7_MAINIO::write_data8(uint32 addr, uint32 data)
{
	bool flag;
	if(addr == FM7_MAINIO_BOOTMODE) {
		bootmode = data & 0x03;
		return;
	} else if(addr == FM7_MAINIO_CLOCKMODE) {
		set_clockmode((uint8)data);
		return;
	}
	//if((addr >= 0x0006) && !(addr == 0x1f)) printf("MAINIO: WRITE: %08x DATA=%08x\n", addr, data);
	
	data = data & 0xff;
	addr = addr & 0xff;
#if defined(HAS_MMR)
        if((addr < 0x90) && (addr >= 0x80)) {
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
		mmr_table[addr - 0x80 + mmr_segment * 16] = data;
#else
		mmr_table[addr - 0x80 + (mmr_segment & 0x03) * 16] = data & 0x3f;
#endif
		//printf("MMR: Write access segment=%02x addr=%02x page=%02x\n", mmr_segment, addr - 0x80, data);
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
			// set_flags_fd04(data);
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
			flag = enable_initiator;
			//printf("INITIATOR ENABLE = %02x\n", data);
			enable_initiator = ((data & 0x02) == 0) ? true : false;
			if(flag != enable_initiator) {
			  mainmem->reset();
			  //this->reset();
			}
			break;
		case 0x12:
			//mode320 = ((data & 0x40) != 0);
			display->write_signal(SIG_DISPLAY_MODE320, data,  0x40);
			break;
		case 0x13:
			display->write_signal(SIG_FM7_SUB_BANK, data, 0x07);
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
		case 0x1f: // ??
			return;
			break;
		case 0x20: // Kanji ROM
		case 0x2c: // Kanji ROM(DUP)
			write_kanjiaddr_hi((uint8)data);
#if defined(CAPABLE_KANJI_CLASS2)
			write_kanjiaddr_hi_l2((uint8)data);
#endif
			break;
		case 0x21: // Kanji ROM
		case 0x2d: // Kanji ROM(DUP)
			write_kanjiaddr_lo((uint8)data);
#if defined(CAPABLE_KANJI_CLASS2)
			write_kanjiaddr_lo_l2((uint8)data);
#endif
			break;
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
			display->write_signal(SIG_FM7_SUB_MULTIPAGE, data, 0x00ff);
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
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
			mmr_segment = data & 7;
#else
			//			printf("MMR SEGMENT: %02x\n", data & 3);
			mmr_segment = data & 3;
#endif			
			break;
		case 0x92:
			window_offset = data & 0x00ff;
			break;
		case 0x93:
			if((data & 0x01) == 0) {
				boot_ram = false;
			} else {
				boot_ram = true;
			}	  
			if((data & 0x40) == 0) {
				window_enabled = false;
			} else {
				window_enabled = true;
			}
			flag = mmr_enabled;
			if((data & 0x80) == 0) {
				mmr_enabled = false;
			} else {
				mmr_enabled = true;
			}
			if(flag != mmr_enabled) {
				this->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
			}
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
	}	// Another:
	return;
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
			if(!irqmask_timer) set_irq_timer(true);
			//register_event(this, EVENT_TIMERIRQ_OFF, 10000.0 / (4.9152 * 2.0) , false, NULL); // TIMER IRQ
			break;
		case EVENT_TIMERIRQ_OFF:
			if(!irqmask_timer) set_irq_timer(false);
			//register_event(this, EVENT_TIMERIRQ_ON, 2035, false, NULL); // TIMER ON
			break;
		case EVENT_FD_MOTOR_ON:
			set_fdc_motor(true);
			break;
		case EVENT_FD_MOTOR_OFF:
			set_fdc_motor(false);
			break;
		default:
			break;
	}
}


void FM7_MAINIO::update_config(void)
{
	switch(config.cpu_type){
		case 0:
			clock_fast = true;
			break;
		case 1:
			clock_fast = false;
			break;
	}
	//	this->write_signal(FM7_MAINIO_CLOCKMODE, clock_fast ? 1 : 0, 1);
}
