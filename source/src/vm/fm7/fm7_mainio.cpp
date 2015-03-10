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

#define EVENT_UPDATE_T77 3


FM7_MAINIO::FM7_MAINIO(VM *parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
	int i;
}  

void FM7_MAINIO::initialize(void)
{
#if defined(_FM8)
	clock_fast = false;
#else
	clock_fast = true;
#endif
   
}

void FM7_MAINIO::set_clockmode(uint8 flags)
{
	if(flags == FM7_MAINCLOCK_SLOW) {
		clock_fast = false;
	} else {
		clock_fast = true;
	}
}

uint8 FM7_MAINIO::get_clockmode(void)
{
	if(clock_fast) return FM7_MAINCLOCK_SLOW;
	return FM7_MAINCLOCK_HIGH;
}


uint8 FM7_MAINIO::get_port_fd00(void)
{
	uint8 ret = 0;
	if(kbd_bit8) ret |= 0x80;
	if(clock_fast) ret |= 0x01;
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
	ret = (cmt_indat) ? 0x80 : 0x00; // CMT 
	return ret;
}

void FM7_MAINIO::set_port_fd02(uint8 val)
{
	irqmask_reg0 = val;
	if((val & 0b00010000) != 0) {
		irqmask_mfd = false;
	} else {
		irqmask_mfd = true;
	}
	if((val & 0b00000100) != 0) {
		irqmask_timer = false;
	} else {
		irqmask_timer = true;
	}
	if((val & 0b00000010) != 0) {
		irqmask_printer = false;
	} else {
		irqmask_printer = true;
	}
	if((val & 0b00000001) != 0) {
		irqmask_keyboard = false;
	} else {
		irqmask_keyboard = true;
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

void FM7_MAINIO::do_irq(bool flag)
{
	if(flag) {
		maincpu->write_signal(SIG_CPU_IRQ, 1, 1);
	} else {
		maincpu->write_signal(SIG_CPU_IRQ, 0, 1);
	}
}


void FM7_MAINIO::set_beep(uint32 data) // fd03
{
	beep->write_signal(SIG_BEEP_ON, data, 0b11000000);
	beep->write_signal(SIG_BEEP_MUTE, data , 0b00000001);
	if((data & 0x40) != 0) {
		// BEEP ON, after 205ms, BEEP OFF.  
		register_event(this, EVENT_BEEP_OFF, 205.0 * 1000.0, false, NULL); // NEXT CYCLE
	}
}

void FM7_MAINIO::set_irq_timer(bool flag)
{
	if(flag && !irqmask_timer) {
		irqstat_reg0 &= 0b11111011;
		do_irq(true);
		return;
	}
	//
	if(flag == false) {
		irqstat_reg0 |= 0b00000100;
	}
	do_irq(false);
}

void FM7_MAINIO::set_irq_printer(bool flag)
{
	if(flag && !irqmask_printer) {
		irqstat_reg0 &= 0b11111101;
		do_irq(true);
		return;
	}
	if(flag == false) {
		irqstat_reg0 |= 0b00000010;
	}
	do_irq(false);
}

void FM7_MAINIO::set_irq_keyboard(bool flag)
{
	if(flag && !irqmask_keyboard) {
		irqstat_reg0 &= 0b11111110;
		do_irq(true);
		return;
	}
	if(flag == false) {
		irqstat_reg0 |= 0b00000001;
	}
	do_irq(false);
}

void FM7_MAINIO::set_irq_mfd(bool flag)
{
	fdc_irq = flag;
	if(flag && !irqmask_mfd && fdc_connected) {
		irqstat_fdc |= 0b01000000;
		do_irq(true);
		return;
	}
	if((flag == false) && fdc_connected){
		irqstat_fdc &= 0b10111111;
	}
	return;
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

void FM7_MAINIO::do_firq(bool flag)
{
	if(flag) {
		maincpu->write_signal(SIG_CPU_FIRQ, 1, 1);
	} else {
		maincpu->write_signal(SIG_CPU_FIRQ, 0, 1);
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
	uint8 val = 0b11111100;
	if(!firq_break_key)     val |= 0b00000010;
	if(!firq_sub_attention) val |= 0b00000001;
	return val;
}

void FM7_MAINIO::set_fd04(uint8 val)
{
	// NOOP?
}

  // FD05
 uint8 FM7_MAINIO::get_fd05(void)
{
	uint8 val = 0b01111110;
	if(sub_busy)    val |= 0b10000000;
	if(!extdet_neg) val |= 0b00000001;
}

 void FM7_MAINIO::set_fd05(uint8 val)
{
	display->write_signal(SIG_FM7_SUB_HALT, val, 0b10000000);
	display->write_signal(SIG_FM7_SUB_CANCEL, val, 0b01000000);
	if((val & 0b10000000) == 0) {
		sub_haltreq = false;
	} else {
		sub_haltreq = true;
	}
	if((val & 0b00000001) != 0) {
		maincpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		//z80->write_signal(SIG_CPU_BUSREQ, 0, 1);
	} else {
		maincpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
		//z80->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
}



void FM7_MAINIO::set_extdet(bool flag)
{
	extdet_neg = flag;
}

void FM7_MAINIO::set_psg(uint8 val)
{
	if(psg == NULL) return set_opn(val, 0); // 77AV ETC
	switch(psg_cmdreg & 0x03){
		case 0: // High inpedance
			return;
			break;
		case 1: // Read Data
			//psg_data = psg->read_io8(1);
			break;
		case 2: // Write Data
			psg->write_io(1, val & 0x00ff);
			psg->write_signal(SIG_YM2203_MUTE, 0x01, 0x01); // Okay?
			break;
		case 3: // Register address
			psg_address = val & 0x0f;
			psg->write_io8(0, psg_address);
			break;
	}
}

uint8 FM7_MAINIO::get_psg(void)
{
	uint8 val = 0xff;
	if(psg == NULL) return get_opn(0); // 77AV ETC
	switch(psg_cmdreg & 0x03) {
		case 0:
			val = 0xff;
			break;
		case 1:
			val = psg->read_io8(1);
			psg_data = val & 0x00ff;
			break;
		case 2:
			val = 0xff; // Write conflict
			break;
		case 3:
			val = psg->read_io8(1);
			psg_address = val;
			break;
	}
	return val;
}

/*
 * $fd0d : After 77AV, this is OPN.
 */
void FM7_MAINIO::set_psg_cmd(uint32 cmd)
{
	if(opn_psg_77av) {
		set_opn_cmd(cmd);
		return;
	}
	psg_cmdreg = (uint8)(cmd & 0b00000011);
	return;
}

uint8 FM7_MAINIO::get_psg_cmd(void)
{
	if(opn_psg_77av) return get_opn_cmd(); 
	return ((psg_cmdreg & 0b00000011) | 0b11111100);
}



// Kanji ROM, FD20 AND FD21 (or SUBSYSTEM)
void FM7_MAINIO::write_kanjiaddr_hi(uint8 addr)
{
	if(!connect_kanjirom1) return;
	kaddress_hi = addr;
	return;
}

void FM7_MAINIO::write_kanjiaddr_lo(uint8 addr)
{
	if(!connect_kanjirom1) return;
	kaddress_lo = addr;
	return;
}

uint8 FM7_MAINIO::read_kanjidata_left(void)
{
	uint32 addr;
    
	if(!connect_kanjirom1) return 0xff;
	addr = ((kaddress_hi & 0xff) * 256) + (kaddress_lo * 0xff);
	addr = addr * 2;
	if(kanjiclass1) {
		return kanjiclass1->read_data8(addr);
	} else {
		return 0xff;
	}
}

uint8 FM7_MAINIO::read_kanjidata_right(void)
{
	uint32 addr;
    
	if(!connect_kanjirom1) return 0xff;
	addr = ((kaddress_hi & 0xff) * 256) + (kaddress_lo * 0xff);
	addr = addr * 2 + 1;
	if(kanjiclass1) {
		return kanjiclass1->read_data8(addr);
	} else {
		return 0xff;
	}
}

// OPN
// Write to FD16, same as 
 void FM7_MAINIO::set_opn(uint8 val, int index)
{
	switch(index) {
  		case 0: // OPN
    			if(!opn_connected) return;
			break;
		case 1: // WHG
			if(!whg_connected) return;
			break;
		case 2: // THG
			if(!thg_connected) return;
			break;
		default:
			return;
			break;
	}
	if((opn_cmdreg[index] & 0b00001000) != 0) {
		// Read Joystick
		return;
	}
	if((opn_cmdreg[index] & 0b00000100) != 0) {
		// Read Status
		return;
	}
	switch(opn_cmdreg[index] & 0x03){
		case 0: // High inpedance
			return;
			break;
		case 1: // Read Data
      			//psg_data = psg->read_io8(1);
			break;
		case 2: // Write Data
			opn_data[index] = val & 0x00ff;
			opn[index]->write_io(1, val & 0x00ff);
			opn[index]->write_signal(SIG_YM2203_MUTE, 0x01, 0x01); // Okay?
			break;
		case 3: // Register address
			opn_address[index] = val & 0x0f;
			opn[index]->write_io8(0, psg_address);
			break;
	}
}

 uint8 FM7_MAINIO::get_opn(int index)
{
	uint8 val = 0xff;
	switch(index) {
  		case 0: // OPN
    			if(!opn_connected) return val;
			break;
		case 1: // WHG
			if(!whg_connected) return val;
			break;
		case 2: // THG
			if(!thg_connected) return val;
			break;
		default:
			return val;
			break;
	}
	if((opn_cmdreg[index] & 0b00001000) != 0) {
		// Read Joystick
		val = opn[index]->read_io8(1); // opn->joystick?
		opn_data[index] = val & 0x00ff;
		return val;
	}
	if((opn_cmdreg[index] & 0b00000100) != 0) {
		// Read Status
		val = opn[index]->read_io8(0);
		opn_stat[index] = val & 0b00ff;
		return val;
	}
	switch(opn_cmdreg[index] & 0x03) {
		case 0:
			val = 0xff;
			break;
		case 1:
			val = opn[index]->read_io8(1);
			opn_data[index] = val & 0x00ff;
			break;
		case 2:
			val = 0xff; // Write conflict
			break;
		case 3:
			val = opn[index]->read_io8(1);
			opn_address[index] = val;
			break;
	}
	return val;
}
  /*
   * $fd16?
   */
void FM7_MAINIO::set_opn_cmd(uint32 cmd)
{
	if(!connect_opn) return;
	opn_cmdreg[0] = (uint8)(cmd & 0b00001111);
	return;
}

uint8 FM7_MAINIO::get_opn_cmd(void)
{
	if(!connect_opn) return 0xff;
	return ((opn_cmdreg[0] & 0b00001111) | 0b11110000);
}

void FM7_MAINIO::wait()
{
	int waitfactor; // If MMR of TWR enabled, factor = 3.
			    // If memory, factor = 2?
	if(!ioaccess_wait) {
		waitfactor = 2;
		ioaccess_wait = true;
	} else { // Not MMR, TWR or enabled FAST MMR mode
		waitfactor = 2; // If(MMR or TWR) and NOT FAST MMR factor = 3, else factor = 2
		ioaccess_wait =false;
	} 
	  
	if(waitfactor <= 0) return;
	if(!clock_fast) return;
	waitcount++;
	if(waitcount >= waitfactor) {
		maincpu->set_extra_icount(1);
		waitcount = 0;
	}
}

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
				if(clock_fast) {
					subclocks = 2000000; // Hz
				} else {
					subclocks =  999000; // Hz
				}
				vm->event->set_cpu_clock(this->maincpu, clocks);
				vm->event->set_cpu_clock(this->subcpu,  subclocks);
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
		case FM7_SUB_BUSY:
			sub_busy = val_b;
			break;
		case FM7_MAINIO_EXTDET:
			extdet_neg = !val_b;
			break;
		case FM7_MAINIO_BEEP:
			beep->write_signal(SIG_BEEP_ON, data, mask);
			register_event(this, EVENT_BEEP_OFF, 205.0 * 1000.0, false, NULL); // NEXT CYCLE
			break;
		case FM7_MAINIO_OPNPORTA_CHANGED:
			opnport_a = data & mask;
			break;
		case FM7_MAINIO_OPNPORTB_CHANGED:
			opnport_a = data & mask;
			break;
		case FM7_MAINIO_PSG_IRQ:
			break;
		case FM7_MAINIO_OPN_IRQ:
			intstat_opn = val_b;
			do_irq(val_b);
       			break;
		case FM7_MAINIO_WHG_IRQ:
			intstat_whg = val_b;
			do_irq(val_b);
       			break;
		case FM7_MAINIO_THG_IRQ:
			intstat_thg = val_b;
			do_irq(val_b);
       			break;
		case FM7_MAINIO_FDC_DRQ:
			fdc_drq = val_b;
			if(fdc_drq) {
				irqstat_fdc |= 0b10000000;
			} else {
				irqstat_fdc &= 0b01111111;
			}
			set_irq_mfd(val_b);
			break;
		case FM7_MAINIO_FDC_IRQ:
			fdc_irq = val_b;
			if(val_b) {
				irqstat_fdc |= 0b01000000;
			} else {
				irqstat_fdc &= 0b10111111;
			}
			set_irq_mfd(val_b);
			break;
	}
	
}


uint8 FM7_MAINIO::fdc_getdrqirq(void)
{
	return irqstat_fdc;
}

 uint8 FM7_MAINIO::get_irqstat_fd03(void)
{
	uint8 val = 0b11111000;
	bool extirq = false;
	
	extirq = instat_opn | intstat_mouse | fdc_irq;
	extirq = extirq | intstat_thg | intstat_whg;
	//extirq = extirq | intstat_syndet | intstat_rxrdy | intstat_txrdy;
	if(extirq) val &= 0b11110111;
	val &= irqstat_reg0;
	return val;
}

 uint8 FM7_MAINIO::get_extirq_fd17(void)
{
	uint8 val = 0xff;
	if(intstat_opn)   val &= 0b11110111;
	if(intstat_mouse) val &= 0b11111011;
	return val;
}

 void FM7_MAINIO::set_ext_fd17(uint8 data)
{
	if((data & 0b00000100) != 0) {
		mouse_enable = true;
	} else {
		mouse_enable = false;
	}
   
}

/* FDD */

void FM7_MAINIO::set_fdc_stat(uint8 val)
{
	if(fdc == NULL) return;
	fdc_statreg = val;
	fdc->write_io8(0, val & 0x00ff);
}

uint8 FM7_MAINIO::get_fdc_stat(void)
{
	if(fdc == NULL) return 0xff;
	this->write_signal(FM7_MAINIO_FDC_IRQ, 0, 1);
	fdc_statreg =  fdc->read_io8(0);
	return fdc_statreg;
}

void FM7_MAINIO::set_fdc_track(uint8 val)
{
	if(fdc == NULL) return;
	// if mode is 2DD and type-of-image = 2D then val >>= 1;
	fdc_trackreg = val;
	fdc->write_io8(1, val & 0x00ff);
}

uint8 FM7_MAINIO::get_fdc_track(void)
{
	if(fdc == NULL) return 0xff;
	fdc_trackreg = fdc->read_io8(1);
	return fdc_trackreg;
}

void FM7_MAINIO::set_fdc_sector(uint8 val)
{
	if(fdc == NULL) return;
	fdc_sectreg = val;
	fdc->write_io8(2, val & 0x00ff);
}

uint8 FM7_MAINIO::get_fdc_sector(void)
{
	if(fdc == NULL) return 0xff;
	fdc_sectreg = fdc->read_io8(2);
	return fdc_sectreg;
}
  
void FM7_MAINIO::set_fdc_data(uint8 val)
{
	if(!connect_fdc) return;
	fdc_datareg = val;
	fdc->write_io8(3, val & 0x00ff);
}

uint8 FM7_MAINIO::get_fdc_data(void)
{
	if(!connect_fdc) return 0xff;
	fdc_datareg = fdc->read_io(3);
	return fdc_datareg;
}

uint8 FM7_MAINIO::get_fdc_motor(void)
{
	uint8 val = 0x00;
	if(fdc == NULL) return 0xff;
	if(fdc_motor) val = 0x80;
	val = val | (fdc_drvsel & 0x03);
	return val;
}
  
void FM7_MAINIO::set_fdc_fd1c(uint8 val)
{
	if(fdc == NULL) return;
	fdc_headreg = (val & 0x01) | 0xfe;
	fdc->write_signal(SIG_MB8877_SIDEREG, val, 0x01);
}

uint8 FM7_MAINIO::get_fdc_fd1c(void)
{
	if(fdc == NULL) return 0xff;
	return fdc_headreg;
}

void FM7_MAINIO::set_fdc_fd1d(uint8 val)
{
	if(fdc == NULL) return;
	if((val & 0x80) != 0) {
		fdc_motor = true;
	} else {
		fdc_motor = false;
	}
	fdc->write_signal(SIG_MB8877_DRIVEREG, val, 0x07);
	fdc->write_signal(SIG_MB8877_MOTOR, val, 0x80);
	fdc_drvsel = val;
}
   

uint32 FM7_MAINIO::read_data8(uint32 addr)
{

	if(addr < 0x100) this->wait();
	//addr = addr & 0xff; //
	switch(addr) {
		case 0x00: // FD00
		case 0x100: // D400 (SUB)
			return (uint32) get_port_fd00();
			break;
		case 0x01: // FD01
		case 0x101: // D401
			return (uint32) kbd_bit7_0;
			break;
		case 0x02: // FD02
			return (uint32) get_port_fd02();
			break;
		case 0x03: // FD03
			return (uint32) get_irqstat_fd03();
			break;
		case 0x04: // FD04
			return (uint32) get_fd04();
			break;
		case 0x05: // FD05
			return (uint32) get_fd05();
			break;
		case 0x06: // RS-232C
		case 0x07:
			return 0xff;
			break;
		case 0x08: // Light pen
		case 0x09:
    		case 0x0a:
			return 0xff;
			break;
		case 0x0d:
			return (uint32) get_psg_cmd();
			break;
		case 0x0e:
			return (uint32) get_psg();
			break;
		case 0x0f: // FD0F
		  	read_fd0f();
			return 0x00ff;
			break;
		case 0x15: // OPN CMD
			return (uint32) get_opn_cmd();
			break;
		case 0x16: // OPN DATA
			return (uint32) get_opn();
			break;
		case 0x17:
			return (uint32) get_extirq_fd17();
			break;
		case 0x18: // FDC: STATUS
			return (uint32) get_fdc_stat();
			break;
		case 0x19: // FDC: Track
			return (uint32) get_fdc_track();
			break;
		case 0x1a: // FDC: Sector
			return (uint32) get_fdc_sector();
			break;
		case 0x1b: // FDC: Data
			return (uint32) get_fdc_data();
			break;
		case 0x1c:
			return (uint32) get_fdc_fd1c();
			break;
		case 0x1d:
			return (uint32) get_fdc_motor();
			break;
		case 0x1f:
			return (uint32) irqstat_fdc;
			break;
		case 0x20: // Kanji ROM
			return (uint32) read_kanjidata_left();
			break;
		case 0x21: // Kanji ROM
			return (uint32) read_kanjidata_right();
			break;
		case 0x37: // Multi page
			return (uint32)display->read_data8(DISPLAY_ADDR_MULTIPAGE);
			break;
		default:
			break;
	}
	if((addr < 0x40) && (addr >= 0x38)) {
		addr = (addr - 0x38) + DISPLAY_ADDR_DPALETTE;
		return (uint32) display->read_data8(addr);
	}
	if((addr >= 0x10000) && (addr < 0x30000)) { // Kanjirom Level1
	  if(!connect_kanjirom1) return 0xff;
	  if(kanjiclass1) {
		return kanjiclass1->read_data8(addr - 0x10000);
	  } else {
		return 0xff;
	  }
	}
	if((addr >= 0x30000) && (addr < 0x40000)) { // Kanjirom Level2
	  if(!connect_kanjirom2) return 0xff;
	  if(kanjiclass2) {
		return kanjiclass2->read_data8(addr - 0x30000);
	  } else {
		return 0xff;
	  }
	}
	
	// Another:
	return 0xff;
}

void FM7_MAINIO::write_data8(uint32 addr, uint32 data)
{

	this->wait();
	addr = addr & 0xff; //
	data = data & 0xff;

	switch(addr) {
		case 0x00: // FD00
			set_port_fd00((uint8)data);
			return;
			break;
		case 0x01: // FD01
			// set_lptdata_fd01((uint8)data);
			return;
			break;
		case 0x02: // FD02
			set_port_fd02((uint8)data);
			return;
			break;
		case 0x03: // FD03
			set_beep(data);
			return;
			break;
		case 0x04: // FD04
			// set_flags_fd04(data);
			return;
			break;
		case 0x05: // FD05
	  		set_fd05((uint8)data);
			return;
			break;
		case 0x06: // RS-232C
		case 0x07:
			return;
			break;
		case 0x08: // Light pen
		case 0x09:
		case 0x0a:
			return;
			break;
		case 0x0d:
			set_psg_cmd((uint8)data);
			return;
			break;
		case 0x0e:
			set_psg((uint8)data);
			return;
			break;
		case 0x0f: // FD0F
			write_fd0f();
			return;
			break;
		case 0x15: // OPN CMD
			return set_opn_cmd((uint8)data);
			break;
		case 0x16: // OPN DATA
			return set_opn((uint8)data);
			break;
		case 0x17:
			set_ext_fd17((uint8)data);
			return;
			break;
		case 0x18: // FDC: STATUS
			return set_fdc_stat((uint8)data);
			break;
		case 0x19: // FDC: Track
			return set_fdc_track((uint8)data);
			break;
		case 0x1a: // FDC: Sector
			return set_fdc_sector((uint8)data);
			break;
      		case 0x1b: // FDC: Data
			return set_fdc_data((uint8)data);
			break;
		case 0x1c:
			return set_fdc_fd1c((uint8)data);
			break;
		case 0x1d:
			return set_fdc_motor((uint8)data);
			break;
		case 0x1f: // ??
			return;
			break;
		case 0x20: // Kanji ROM
			return write_kanjiaddr_hi((uint8)data);
			break;
		case 0x21: // Kanji ROM
			return write_kanjiaddr_lo((uint8)data);
			break;
		case 0x37: // Multi page
			return display->write_data8(DISPLAY_ADDR_MULTIPAGE, (uint8)data);
			break;
		default:
			break;
	}
	if((addr < 0x40) && (addr >= 0x38)) {
		addr = (addr - 0x38) + DISPLAY_ADDR_DPALETTE;
		display->write_data8(addr, (uint8)data);
		return;
	}
	// Another:
	return;
}

void FM7_MAINIO::event_callback(int event_id, int err)
{
	switch(event_id) {
		case EVENT_BEEP_OFF:
			beep->write_signal(SIG_BEEP_ON, 0x00, 0x01);
			break;
		default:
			break;
	}
}


