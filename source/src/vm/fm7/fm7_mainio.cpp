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


void FM7_MAINIO::initialize(void)
{
#if defined(_FM8)
	clock_fast = false;
#else
	clock_fast = true;
#endif
	//	connect_fdc = true;
	event_beep = -1;
	event_timerirq = -1;
	bootmode = config.boot_mode & 3;
	register_event(this, EVENT_TIMERIRQ_ON, 4069.0 / 2.0, true, &event_timerirq); // TIMER IRQ
#if defined(_FM77AV_VARIANTS)
	opn_psg_77av = true;
#else
	opn_psg_77av = false;
#endif
}

void FM7_MAINIO::reset(void)
{
	int i;
	if(event_beep >= 0) cancel_event(this, event_beep);
	if(event_timerirq >= 0) cancel_event(this, event_timerirq);
	event_beep = -1;
	register_event(this, EVENT_TIMERIRQ_ON, 4069.0 / 2.0, true, &event_timerirq); // TIMER IRQ
	if(connect_fdc) fdc->reset();
	stat_romrammode = true;
	bootmode = config.boot_mode & 3;
	pcm1bit->write_signal(SIG_PCM1BIT_MUTE, 0x01, 0x01);
	psg_data = 0;
	psg_cmdreg = 0;
	psg_address = 0;
   	for(i = 0; i < 3; i++) {
		opn_data[i] = 0;
   		opn_cmdreg[i] = 0;
   		opn_address[i] = 0;
	}

//   maincpu->reset();
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
	beep_flag = ((data & 0xc0) != 0);
	pcm1bit->write_signal(SIG_PCM1BIT_MUTE, ~data, 0b00000001);
	//beep->write_signal(SIG_BEEP_MUTE, ~data , 0b00000001);
	if(beep_flag) {
		beep_snd = true;
		if(event_beep < 0) register_event(this, EVENT_BEEP_CYCLE, (1000.0 * 1000.0) / (1200.0 * 2.0), true, &event_beep);
		pcm1bit->write_signal(SIG_PCM1BIT_ON, 1, 1);
	} else {
		if(event_beep >= 0) cancel_event(this, event_beep);
		event_beep = -1;
		pcm1bit->write_signal(SIG_PCM1BIT_ON, 0, 1);
	}
	if((data & 0x40) != 0) {
		// BEEP ON, after 205ms, BEEP OFF.  
		register_event(this, EVENT_BEEP_OFF, 205.0 * 1000.0, false, NULL); // NEXT CYCLE
	}
}

void FM7_MAINIO::set_irq_timer(bool flag)
{
	if(flag) {
		irqstat_reg0 &= 0b11111011;
		if(!irqmask_timer) do_irq(true);
	} else {
		irqstat_reg0 |= 0b00000100;
		if(!irqmask_timer) do_irq(false);
	}
	//printf("IRQ TIMER: %d MASK=%d\n", flag, irqmask_timer);
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
	if(flag) {
		irqstat_reg0 &= 0b11111110;
		if(!irqmask_keyboard) do_irq(true);
		return;
	} else {
		irqstat_reg0 |= 0b00000001;
		if(!irqmask_keyboard) do_irq(false);
	}
}

void FM7_MAINIO::set_irq_mfd(bool flag)
{
	fdc_irq = flag;
	if(flag &&  connect_fdc) {
		irqstat_fdc |= 0b01000000;
	}
	if((flag == false) && connect_fdc){
		irqstat_fdc &= 0b10111111;
	}
	if(!irqmask_mfd) do_irq(flag);
	return;
}

void FM7_MAINIO::set_drq_mfd(bool flag)
{
	fdc_irq = flag;
	if(flag &&  connect_fdc) {
		irqstat_fdc |= 0b10000000;
	}
	if((flag == false) && connect_fdc){
		irqstat_fdc &= 0b01111111;
	}
	if(!irqmask_mfd) do_irq(flag);
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
	if(firq_sub_attention) {
		firq_sub_attention = false;
	}
//	do_firq(false);
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
	return val;
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
#ifdef WITH_Z80
	if((val & 0b00000001) != 0) {
		maincpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		//z80->write_signal(SIG_CPU_BUSREQ, 0, 1);
	} else {
		maincpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
		//z80->write_signal(SIG_CPU_BUSREQ, 1, 1);
	}
#endif
}



void FM7_MAINIO::set_extdet(bool flag)
{
	extdet_neg = flag;
}

void FM7_MAINIO::set_psg(uint8 val)
{
	if(opn_psg_77av) return set_opn(0, val); // 77AV ETC
	psg_data = val;
	switch(psg_cmdreg){
		case 0: // High inpedance
			break;
		case 1: // Read Data
	   		break;
		case 2: // Write Data
			psg->write_io8(0, psg_address);
			psg->write_io8(1, val & 0x00ff);
			//psg->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?
			break;
		case 3: // Register address
			psg_address = val & 0x0f;
			break;
	}
}

uint8 FM7_MAINIO::get_psg(void)
{
	uint8 val = 0xff;
	if(opn_psg_77av) {
		return get_opn(0);
	}
	switch(psg_cmdreg) {
		case 0:
		case 1:
		case 2:
		case 3:
			val = psg_data;
			break;
	}
	return val;
}

/*
 * $fd0d : After 77AV, this is OPN.
 */
void FM7_MAINIO::set_psg_cmd(uint8 cmd)
{
	if(opn_psg_77av) {
		set_opn_cmd(0, cmd);
		return;
	}
	psg_cmdreg = (uint8)(cmd & 0b00000011);
        switch(psg_cmdreg) {
		case 0:
			break;
		case 1:
			psg->write_io8(0, psg_address);
			psg_data = psg->read_io8(1);
			//printf("PSG READ DATA %02x from REG ADDR=%02x\n", psg_data, psg_address);
	 		break;
		case 2:
			//printf("PSG WRITE DATA %02x to REG ADDR=%02x\n", psg_data, psg_address);
			psg->write_io8(0, psg_address);
			psg->write_io8(1, psg_data);
			//psg->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?
	 		break;
	 	case 3:
			psg_address = psg_data;
			//printf("PSG REG ADDR=%02x\n", psg_address);
			break;
	 	default:
	   		break;
	}
   
	return;
}




// Kanji ROM, FD20 AND FD21 (or SUBSYSTEM)
void FM7_MAINIO::write_kanjiaddr_hi(uint8 addr)
{
	if(!connect_kanjiroml1) return;
	kaddress_hi = addr;
	return;
}

void FM7_MAINIO::write_kanjiaddr_lo(uint8 addr)
{
	if(!connect_kanjiroml1) return;
	kaddress_lo = addr;
	return;
}

uint8 FM7_MAINIO::read_kanjidata_left(void)
{
	uint32 addr;
    
	if(!connect_kanjiroml1) return 0xff;
	addr = ((uint32)kaddress_hi << 8) | (uint32)kaddress_lo;
	addr = addr << 1;
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
	addr = ((uint32)kaddress_hi << 8) | (uint32)kaddress_lo;
	addr = (addr << 1) | 1;
	if(kanjiclass1) {
		return kanjiclass1->read_data8(addr);
	} else {
		return 0xff;
	}
}

#ifdef _FM77AV_VARIANTS
// Kanji ROM, FD20 AND FD21 (or SUBSYSTEM)
void FM7_MAINIO::write_kanjiaddr_hi_l2(uint8 addr)
{
	if(!connect_kanjiroml2) return;
	kaddress_hi_l2 = addr;
	return;
}

void FM7_MAINIO::write_kanjiaddr_lo_l2(uint8 addr)
{
	if(!connect_kanjiroml2) return;
	kaddress_lo_l2 = addr;
	return;
}

uint8 FM7_MAINIO::read_kanjidata_left_l2(void)
{
	uint32 addr;
    
	if(!connect_kanjiroml2) return 0xff;
	addr = ((uint32)kaddress_hi_l2 << 8) | (uint32)kaddress_lo_l2;
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
	addr = ((uint32)kaddress_hi_l2 << 8) | (uint32)kaddress_lo_l2;
	addr = (addr << 1) | 0x01;
	if(kanjiclass2) {
		return kanjiclass2->read_data8(addr);
	} else {
		return 0xff;
	}
}
#endif
// OPN
// Write to FD16, same as 
void FM7_MAINIO::set_opn(int index, uint8 val)
{
	//printf("OPN %d WRITE %02x \n", index, val);
	if((index > 2) || (index < 0)) return;
	if((index == 0) && (!connect_opn)) return;
	if((index == 1) && (!connect_whg)) return;
	if((index == 2) && (!connect_thg)) return;
   
	if(opn[index] == NULL) return;
   
	opn_data[index] = val;
	switch(opn_cmdreg[index]){
		case 0: // High inpedance
			break;
		case 1: // Read Data
			opn[index]->write_io8(0, opn_address[index]);
			opn_data[index] = opn[index]->read_io8(1);
			break;
		case 2: // Write Data
			//printf("OPN %d WRITE DATA %02x to REG ADDR=%02x\n", index, val, opn_address[index]);
			opn[index]->write_io8(0, opn_address[index]);
			opn[index]->write_io8(1, val & 0x00ff);
			//opn[index]->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?
			break;
		case 3: // Register address
			opn_address[index] = val & 0xff;
			//printf("OPN %d REG ADDR=%02x\n", index, opn_address[index]);
	   		if((val > 0x2c) && (val < 0x30)) {
				opn_data[index] = 0;
				opn[index]->write_io8(0, opn_address[index]);
				opn[index]->write_io8(1, 0);
			}
			break;
	   
	}
}

uint32 FM7_MAINIO::update_joystatus(int index)
{
	uint32 *joybuf = p_emu->joy_buffer();
	uint32 val = 0xff;
        if((joybuf[index] & 0x01) != 0) val &= ~0x01;  
        if((joybuf[index] & 0x02) != 0) val &= ~0x02;  
        if((joybuf[index] & 0x04) != 0) val &= ~0x04;  
        if((joybuf[index] & 0x08) != 0) val &= ~0x08;
        if((joybuf[index] & 0x10) != 0) val &= ~0x10;  
        if((joybuf[index] & 0x20) != 0) val &= ~0x20;  
        if((joybuf[index] & 0x40) != 0) val &= ~0x10;  
        if((joybuf[index] & 0x80) != 0) val &= ~0x20;  
	return val;
}

uint8 FM7_MAINIO::get_opn(int index)
{
	uint8 val = 0xff;
	if((index > 2) || (index < 0)) return 0xff;
	if((index == 0) && (!connect_opn)) return 0xff;
	if((index == 1) && (!connect_whg)) return 0xff;
	if((index == 2) && (!connect_thg)) return 0xff;
   
	if(opn[index] == NULL) return 0xff;
   
	if(opn_cmdreg[index] == 0b00001001) {
		//printf("OPN: JOY PORT ADDR=%02x\n", opn_address[index]);
		// Read Joystick
	        if(opn_address[index] == 0x0e) {
			joyport_a = update_joystatus(0);
			joyport_b = update_joystatus(1);
			opn[index]->write_io8(0, 0x0f);
			val = opn[index]->read_io8(1);
			//printf("JOY: A=%02x B=%02x Reg15=%02x\n", val, joyport_a, joyport_b); 
			if((val & 0x20) == 0x20) return joyport_a;
			if((val & 0x50) == 0x50) return joyport_b;
			return 0xff;
		}
		return 0x00;
	}
	if(opn_cmdreg[index] == 0b00000100) {
		// Read Status
		opn_stat[index] = opn[index]->read_io8(0);
		return opn_stat[index];
	}
	switch(opn_cmdreg[index]) {
		case 0:
		case 1:
		case 2:
		case 3:
			val = opn_data[index];
			break;
	}
	return val;
}
  /*
   * $fd16?
   */
void FM7_MAINIO::set_opn_cmd(int index, uint8 cmd)
{
	if((index > 2) || (index < 0)) return;
	if((index == 0) && (!connect_opn)) return;
	if((index == 1) && (!connect_whg)) return;
	if((index == 2) && (!connect_thg)) return;
   
	opn_cmdreg[index] = cmd & 0b00001111;
        switch(opn_cmdreg[index]) {
		case 0:
			break;
		case 1:
			opn[index]->write_io8(0, opn_address[index]);
			opn_data[index] = opn[index]->read_io8(1);
			//printf("OPN %d READ DATA %02x from REG ADDR=%02x\n", index, opn_data[index], opn_address[index]);
	 		break;
		case 2:
			//printf("OPN %d WRITE DATA %02x to REG ADDR=%02x\n", index, opn_data[index], opn_address[index]);
			opn[index]->write_io8(0, opn_address[index]);
			opn[index]->write_io8(1, opn_data[index]);
			//opn[index]->write_signal(SIG_YM2203_MUTE, 0x00, 0x01); // Okay?
	 		break;
	 	case 3:
			opn_address[index] = opn_data[index];
			//printf("OPN %d REG ADDR=%02x\n", index, opn_address[index]);
	   		if((opn_data[index] > 0x2c) && (opn_data[index] < 0x30)) {
				opn_data[index] = 0;
				opn[index]->write_io8(0, opn_address[index]);
				opn[index]->write_io8(1, 0);
			}
			break;
	 	default:
	   		break;
	}
   
	//printf("OPN %d SET REG ADDR=%02x\n", index, opn_cmdreg[index]);
	return;
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
				p_vm->set_cpu_clock(this->maincpu, clocks);
				p_vm->set_cpu_clock(this->subcpu,  subclocks);
			}
			break;
		case FM7_MAINIO_CMT_RECV: // FD02
			cmt_indat = val_b ^ cmt_invert;
			break;
		case FM7_MAINIO_CMT_INVERT: // FD02
			cmt_invert = val_b;
			break;
		case FM7_MAINIO_TIMERIRQ: //
			//set_irq_timer(val_b);
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
			sub_busy = val_b;
			break;
		case FM7_MAINIO_EXTDET:
			extdet_neg = !val_b;
			break;
		case FM7_MAINIO_BEEP:
			beep_flag = true;
			beep_snd = true;
			register_event(this, EVENT_BEEP_CYCLE, (1000.0 * 1000.0) / (1200.0 * 2.0), true, &event_beep);
			pcm1bit->write_signal(SIG_PCM1BIT_ON, 1, 1);
			register_event(this, EVENT_BEEP_OFF, 205.0 * 1000.0, false, NULL); // NEXT CYCLE
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
			set_drq_mfd(val_b);
			break;
		case FM7_MAINIO_FDC_IRQ:
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
	
	extirq = fdc_irq;
	
	extirq = extirq;
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
	//if(intstat_opn || intstat_mouse) do_irq(false);
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


uint8 FM7_MAINIO::get_extirq_whg(void)
{
	uint8 val = 0xff;
	if(intstat_whg) val &= ~0x08;
	return val;
}

uint8 FM7_MAINIO::get_extirq_thg(void)
{
	uint8 val = 0xff;
	if(intstat_thg) val &= ~0x08;
	return val;
}


/* FDD */

void FM7_MAINIO::set_fdc_cmd(uint8 val)
{
	if(!connect_fdc) return;
	fdc_cmdreg = val;
	fdc->write_io8(0, val & 0x00ff);
}

uint8 FM7_MAINIO::get_fdc_stat(void)
{
	if(!connect_fdc) return 0xff;
	this->write_signal(FM7_MAINIO_FDC_IRQ, 0, 1);
	fdc_statreg =  fdc->read_io8(0);
	return fdc_statreg;
}

void FM7_MAINIO::set_fdc_track(uint8 val)
{
	if(!connect_fdc) return;
	// if mode is 2DD and type-of-image = 2D then val >>= 1;
	fdc_trackreg = val;
	fdc->write_io8(1, val & 0x00ff);
}

uint8 FM7_MAINIO::get_fdc_track(void)
{
	if(!connect_fdc) return 0xff;
	fdc_trackreg = fdc->read_io8(1);
	return fdc_trackreg;
}

void FM7_MAINIO::set_fdc_sector(uint8 val)
{
	if(!connect_fdc) return;
	fdc_sectreg = val;
	fdc->write_io8(2, val & 0x00ff);
}

uint8 FM7_MAINIO::get_fdc_sector(void)
{
	if(!connect_fdc) return 0xff;
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
	fdc_datareg = fdc->read_io8(3);
	return fdc_datareg;
}

uint8 FM7_MAINIO::get_fdc_motor(void)
{
	uint8 val = 0x00;
	if(!connect_fdc) return 0xff;
	if(fdc_motor) val = 0x80;
	val = val | (fdc_drvsel & 0x03);
	return val;
}
  
void FM7_MAINIO::set_fdc_fd1c(uint8 val)
{
	if(!connect_fdc) return;
	fdc_headreg = (val & 0x01) | 0xfe;
	fdc->write_signal(SIG_MB8877_SIDEREG, val, 0x01);
}

uint8 FM7_MAINIO::get_fdc_fd1c(void)
{
	if(!connect_fdc) return 0xff;
	return fdc_headreg;
}

void FM7_MAINIO::set_fdc_fd1d(uint8 val)
{
	if(!connect_fdc) return;
	if((val & 0x80) != 0) {
		fdc_motor = true;
	} else {
		fdc_motor = false;
	}
	//	fdc->write_signal(SIG_MB8877_DRIVEREG, val, 0x07);
	fdc->write_signal(SIG_MB8877_DRIVEREG, val, 0x03);
	fdc->write_signal(SIG_MB8877_MOTOR, val, 0x80);
	fdc_drvsel = val;
}
   
uint32 FM7_MAINIO::read_signal(uint32 addr)
{
	uint32 retval = 0xffffffff;
	switch(addr) {
	}
	return retval;
}

uint32 FM7_MAINIO::read_data8(uint32 addr)
{
	if(addr == FM7_MAINIO_IS_BASICROM) {
		uint32 retval = 0;
		if(stat_bootsw_basic) retval = 0xffffffff;
		return retval;
	} else if(addr == FM7_MAINIO_BOOTMODE) {
		uint32 retval = bootmode & 0x03;
#if defined(_FM77) || defined(_FM77L2) || defined(_FM77L4) || defined(_FM77AV_VARIANTS)
		if(bootram) retval = 4;
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
		uint32 retval = (mmr_enabled) ? 0xffffffff:0x00000000;
		return retval;
	} else if(addr == FM7_MAINIO_WINDOW_ENABLED) {
		uint32 retval = (window_enabled) ? 0xffffffff:0x00000000;
		return retval;
	} else if(addr == FM7_MAINIO_MMR_SEGMENT) {
		uint32 retval = (uint32) mmr_segment;
		return retval;
	} else if((addr >= FM7_MAINIO_MMR_BANK) &&  (addr < (FM7_MAINIO_MMR_BANK + 64))) {
		uint32 retval = (uint32)mmr_table[addr - FM7_MAINIO_MMR_BANK];
		return retval;
	}
#endif
#if defined(_FM77AV_VARIANTS)
	else if(addr == FM7_MAINIO_INITROM_ENABLED) {
	}
#endif
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
	else if(addr == FM7_MAINIO_EXTBANK) {
	} else if(addr == FM7_MAINIO_EXTROM) {
	}
#endif
	//addr = addr & 0xff; //
	//	printf("Main I/O READ: %04x\n", addr);
	switch(addr) {
		case 0x00: // FD00
		case 0x100: // D400 (SUB)
			return (uint32) get_port_fd00();
			break;
		case 0x01: // FD01
		case 0x101: // D401
			display->write_signal(SIG_FM7_SUB_KEY_FIRQ, 0, 1);
			set_irq_keyboard(false);
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
			return 0xff;
			break;
		case 0x0e:
			return (uint32) get_psg();
			break;
		case 0x0f: // FD0F
		  	read_fd0f();
			return 0x00ff;
			break;
		case 0x15: // OPN CMD
			return (uint32) 0xff;
			break;
		case 0x16: // OPN DATA
			return (uint32) get_opn(0);
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
			return (uint32) fdc_getdrqirq();
			break;
		case 0x22: // Kanji ROM
			return (uint32) read_kanjidata_left();
			break;
		case 0x23: // Kanji ROM
			return (uint32) read_kanjidata_right();
			break;
#if defined(_FM77AV_VARIANTS)
		case 0x2e: // Kanji ROM Level2
			return (uint32) read_kanjidata_left_l2();
			break;
		case 0x2f: // Kanji ROM Level2
			return (uint32) read_kanjidata_right_l2();
			break;
#endif
		case 0x37: // Multi page
			return (uint32)display->read_data8(DISPLAY_ADDR_MULTIPAGE);
			break;
		case 0x45: // WHG CMD
			return (uint32) 0xff;
			break;
		case 0x46: // WHG DATA
			return (uint32) get_opn(1);
			break;
		case 0x47:
			return (uint32) get_extirq_whg();
			break;
		case 0x51: // THG CMD
			return (uint32) 0xff;
			break;
		case 0x52: // THG DATA
			return (uint32) get_opn(2);
			break;
		case 0x53:
			return (uint32) get_extirq_thg();
			break;
		default:
			break;
	}
	if((addr < 0x40) && (addr >= 0x38)) {
		addr = (addr - 0x38) + FM7_SUBMEM_OFFSET_DPALETTE;
		return (uint32) display->read_data8(addr);
	}
	// Another:
	return 0xff;
}

void FM7_MAINIO::write_data8(uint32 addr, uint32 data)
{
	if(addr == FM7_MAINIO_BOOTMODE) {
		bootmode = data & 0x03;
		return;
	} else if(addr == FM7_MAINIO_CLOCKMODE) {
		set_clockmode((uint8)data);
		return;
	}
	addr = addr & 0xff; //
	data = data & 0xff;
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
			set_psg_cmd(data);
			break;
		case 0x0e:
			set_psg(data);
			break;
		case 0x0f: // FD0F
			write_fd0f();
			break;
		case 0x15: // OPN CMD
			set_opn_cmd(0, data);
			break;
		case 0x16: // OPN DATA
			set_opn(0, data);
			break;
		case 0x17:
			set_ext_fd17((uint8)data);
			break;
		case 0x18: // FDC: COMMAND
			set_fdc_cmd((uint8)data);
			break;
		case 0x19: // FDC: Track
			set_fdc_track((uint8)data);
			break;
		case 0x1a: // FDC: Sector
			set_fdc_sector((uint8)data);
			break;
      		case 0x1b: // FDC: Data
			set_fdc_data((uint8)data);
			break;
		case 0x1c:
			set_fdc_fd1c((uint8)data);
			break;
		case 0x1d:
			set_fdc_fd1d((uint8)data);
			break;
		case 0x1f: // ??
			return;
			break;
		case 0x20: // Kanji ROM
			write_kanjiaddr_hi((uint8)data);
			break;
		case 0x21: // Kanji ROM
			write_kanjiaddr_lo((uint8)data);
			break;
#if defined(_FM77AV_VARIANTS)
		case 0x2c: // Kanji ROM
			write_kanjiaddr_hi_l2((uint8)data);
			break;
		case 0x2d: // Kanji ROM
			write_kanjiaddr_lo_l2((uint8)data);
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
#if defined(_FM77) || defined(_FM77L2) || defined(_FM77L4) || defined(_FM77AV_VARIANTS)
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
			if((data & 0x80) == 0) {
				mmr_enabled = false;
			} else {
				mmr_enabled = true;
			}
			break;
#endif
		default:
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
			//beep->write_signal(SIG_BEEP_ON, 0x00, 0x01);
			beep_flag = false;
			beep_snd = false;
			if(event_beep >= 0) {
				cancel_event(this, event_beep);
				event_beep = -1;
			}
			pcm1bit->write_signal(SIG_PCM1BIT_ON, 0, 1);
			break;
		case EVENT_BEEP_CYCLE:
			beep_snd = !beep_snd;
			pcm1bit->write_signal(SIG_PCM1BIT_SIGNAL, beep_snd ? 1 : 0, 1);
			break;
		case EVENT_UP_BREAK:
			set_break_key(false);
			break;
		case EVENT_TIMERIRQ_ON:
			set_irq_timer(true);
			register_event(this, EVENT_TIMERIRQ_OFF, 2035.0 / 2.0, false, NULL); // TIMER OFF
			break;
		case EVENT_TIMERIRQ_OFF:
			set_irq_timer(false);
			//register_event(this, EVENT_TIMERIRQ_ON, 2035, false, NULL); // TIMER ON
			break;
		default:
			break;
	}
}


