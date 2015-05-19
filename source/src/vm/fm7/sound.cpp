/*
 * FM-7 Main I/O [sound.cpp]
 *  - PSG, OPN, and Buzzer.
 *
 * Author: K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *   Mar 19, 2015 : Initial, split from fm7_mainio.cpp.
 *
 */

//#include "../beep.h"
#include "../pcm1bit.h"

#include "fm7_mainio.h"
#include "../../config.h"

// TEST
//#include <SDL2/SDL.h>

void FM7_MAINIO::reset_sound(void)
{
	int i, j, k;

#if defined(_FM77AV_VARIANTS)
	k = 3;
#else
	k = 4;
#endif
	for(i = 0; i < k; i++) {
		opn_data[i]= 0;
		opn_cmdreg[i] = 0;
		opn_address[i] = 0;
		opn_stat[i] = 0;
		opn_ch3mode[i] = 0x00;
		if(opn[i] != NULL) {
			opn[i]->reset();
			opn[i]->write_data8(0, 0x2e);
			opn[i]->write_data8(1, 0);	// set prescaler
			opn[i]->write_data8(0, 0x27);
			opn[i]->write_data8(1, 0x00);
			for(j = 0; j < 3; j++) {
				opn[i]->SetReg(0x28, j | 0xfe);
			}
			//opn[i]->write_signal(SIG_YM2203_PORT_A, 0xff, 0xff);
			//opn[i]->write_signal(SIG_YM2203_PORT_B, 0xff, 0xff);
		}
	   
	}

	connect_opn = connect_whg = connect_thg = false;
	if(opn_psg_77av) connect_opn = true;

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
	//beep->write_signal(SIG_BEEP_MUTE, 0x01, 0x01);
	//beep->write_signal(SIG_BEEP_ON, 0x00, 0x01);
	pcm1bit->write_signal(SIG_PCM1BIT_MUTE, 0x01, 0x01);
	pcm1bit->write_signal(SIG_PCM1BIT_ON, 0x00, 0x01);

}


void FM7_MAINIO::set_psg(uint8 val)
{
	if(opn_psg_77av) return set_opn(0, val); // 77AV ETC
	//printf("PSG: Set ADDR=%02x REG=%02x DATA=%02x STAT=%02x\n", opn_address[3], opn_cmdreg[3], val, opn_stat[3]);
	set_opn(3, val);
}

uint8 FM7_MAINIO::get_psg(void)
{
	uint8 val = 0xff;
	if(opn_psg_77av) {
		return get_opn(0);
	}
	//printf("PSG: Got ADDR=%02x REG=%02x DATA=%02x STAT=%02x\n", opn_address[3], opn_cmdreg[3], opn_data[3], opn_stat[3]);
	return get_opn(3);
}

/*
 * $fd0d : After 77AV, this is OPN.
 */
void FM7_MAINIO::set_psg_cmd(uint8 cmd)
{
	cmd = cmd & 0x03;
	if(opn_psg_77av) {
		set_opn_cmd(0, cmd);
		return;
	}
	set_opn_cmd(3, cmd);
	//printf("PSG: Set CMD ADDR=%02x REG=%02x DATA=%02x STAT=%02x\n", opn_address[3], cmd, opn_data[3], opn_stat[3]);
	return;
}

// OPN
// Write to FD16, same as 
void FM7_MAINIO::write_opn_reg(int index, uint32 addr, uint32 data)
{
	uint8 r;
	int i;
	uint8 j;
	//	opn_regs[index][addr] = data;
	if(index == 3) { // PSG
	  	opn[index]->write_io8(0, addr & 0x0f);
		opn[index]->write_io8(1, data);
		return;
	}
	if((addr >= 0x2d) && (addr < 0x30)) {
		opn[index]->write_io8(0, addr);
		return;
	}
	if(addr == 0x27) {
		opn_ch3mode[index] = data & 0xc0;
	}
	//if(addr == 0x28) {
	//printf("OPN: #%d SET reg $%02x to %02x\n", index, addr, data);
	//}
	opn[index]->write_io8(0, addr);
	opn[index]->write_io8(1, data);
	return;
}

void FM7_MAINIO::set_opn(int index, uint8 val)
{
	//printf("OPN %d WRITE %02x \n", index, val);
	if((index > 3) || (index < 0)) return;
	if((index == 0) && (!connect_opn)) return;
	if((index == 1) && (!connect_whg)) return;
	if((index == 2) && (!connect_thg)) return;
	if((index == 3) && (opn_psg_77av)) return;
	if(opn[index] == NULL) return;
   
	opn_data[index] = val;
	switch(opn_cmdreg[index]){
		case 0: // High inpedance
			break;
		case 1: // Read Data
			break;
		case 2: // Write Data
			write_opn_reg(index, opn_address[index], opn_data[index]);
			break;
		case 3: // Register address
			if(index != 3) {
				opn_address[index] = val & 0xff;
			} else {
				opn_address[index] = val & 0x0f;
			}
			if((val > 0x2c) && (val < 0x30)) {
				opn_data[index] = 0;
				opn[index]->write_io8(0, opn_address[index]);
				opn[index]->write_io8(1, 0);
			}
			//opn[index]->write_io8(0, opn_address[index]);
			//printf("OPN %d REG ADDR=%02x\n", index, opn_address[index]);
			break;
	   
	}
}

uint32 FM7_MAINIO::update_joystatus(int index)
{
	uint32 *joybuf = p_emu->joy_buffer();
	uint32 val = 0xff;
        if((joybuf[index] & 0x01) != 0) val &= ~0x01;  // UP?
        if((joybuf[index] & 0x02) != 0) val &= ~0x02;  // DOWN?
        if((joybuf[index] & 0x04) != 0) val &= ~0x04;  // LEFT?
        if((joybuf[index] & 0x08) != 0) val &= ~0x08;  // RIGHT?
        if((joybuf[index] & 0x10) != 0) val &= ~0x10;  // Button A
        if((joybuf[index] & 0x20) != 0) val &= ~0x20;  // Button B
        if((joybuf[index] & 0x40) != 0) val &= ~0x10;  // Button A'
        if((joybuf[index] & 0x80) != 0) val &= ~0x20;  // Button B'
	return val;
}

uint8 FM7_MAINIO::get_opn(int index)
{
//	uint8 val = 0xff;
	uint8 val = 0x00;
	if((index > 2) || (index < 0)) return 0x00;
	if((index == 0) && (!connect_opn)) return 0x00;
	if((index == 1) && (!connect_whg)) return 0x00;
	if((index == 2) && (!connect_thg)) return 0x00;
	if((index == 3) && (opn_psg_77av)) return 0x00;
	   
	if(opn[index] == NULL) return 0x00;
	switch(opn_cmdreg[index]) {
		case 0:
			//val = 0xff;
			//break;
		case 1:
		case 2:
		case 3:
			val = opn_data[index];
			break;
		case 4:
			opn_stat[index] = opn[index]->read_io8(0) & 0x03;
			if(index != 3) val = opn_stat[index];
	   		break;
	case 0x09:
	   	if(index != 0) return 0x00;
	        if(opn_address[0] == 0x0e) {
			joyport_a = update_joystatus(0);
			joyport_b = update_joystatus(1);
			opn[0]->write_io8(0, 0x0f);
			val = opn[0]->read_io8(1);
			//printf("JOY: A=%02x B=%02x Reg15=%02x\n", val, joyport_a, joyport_b); 
			if((val & 0x20) == 0x20) return joyport_a;
			if((val & 0x50) == 0x50) return joyport_b;
			return 0xff;
		}
		return 0x00;
		break;
	 default:
	 	break;
	}
	return val;
}
  /*
   * $fd16?
   */
void FM7_MAINIO::set_opn_cmd(int index, uint8 cmd)
{
	if((index > 4) || (index < 0)) return;
	if((index == 0) && (!connect_opn)) return;
	if((index == 1) && (!connect_whg)) return;
	if((index == 2) && (!connect_thg)) return;
	if((index == 3) && (opn_psg_77av)) return ;
	uint32 mask[16] = { // Parameter is related by XM7. Thanks Ryu.
		0xff, 0x0f, 0xff, 0x0f,
		0xff, 0x0f, 0x1f, 0xff,
		0x1f, 0x1f, 0x1f, 0xff,
		0xff, 0x0f, 0xff, 0xff
	};
	opn_cmdreg[index] = cmd & 0x0f;
	uint8 val = opn_data[index];
        switch(opn_cmdreg[index]) {
		case 0:
			break;
		case 1:
			opn[index]->write_io8(0, opn_address[index]);
			opn_data[index] = opn[index]->read_io8(1);
			if(opn_address[index] <= 0x0f) {
				opn_data[index] &= mask[opn_address[index]];
			}
			break;
		case 2:
			//opn[index]->SetReg(opn_address[index], opn_data[index]);
			write_opn_reg(index, opn_address[index], opn_data[index]);
	 		break;
	 	case 3:
			if(index != 3) {
				opn_address[index] = val & 0xff;
			} else {
				opn_address[index] = val & 0x0f;
			}
			if((val > 0x2c) && (val < 0x30)) {
				opn_data[index] = 0;
				opn[index]->write_io8(0, opn_address[index]);
				opn[index]->write_io8(1, 0);
			}
			break;
		case 4:
			opn_stat[index] = opn[index]->read_io8(0) & 0x03;
			//if(index != 3) val = opn_stat[index];
	   		break;
	 	default:
	   		break;
	}
	return;
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

void FM7_MAINIO::opn_note_on(int index)
{
	uint8 r;
	if((index < 0) || (index >= 3)) return;
	// Not on for CSM mode. From XM7. Thanks, Ryu.
	r = opn_ch3mode[index];
	if ((r & 0xc0) == 0x80) {
		opn[index]->write_io8(0, 0x27);
		opn[index]->write_io8(1, opn_ch3mode[index] & 0xc0);
	}
}


void FM7_MAINIO::set_beep(uint32 data) // fd03
{
	bool flag = ((data & 0xc0) != 0);
	pcm1bit->write_signal(SIG_PCM1BIT_MUTE, ~data, 0x01);
	//beep->write_signal(SIG_BEEP_MUTE, ~data, 0x01);
	if(flag != beep_flag) {
		if(flag) {
			beep_snd = true;
			pcm1bit->write_signal(SIG_PCM1BIT_SIGNAL, 1, 1);
			pcm1bit->write_signal(SIG_PCM1BIT_ON, 1, 1);
			//beep->write_signal(SIG_BEEP_ON, 1, 1);
		} else {
			beep_snd = false;
			pcm1bit->write_signal(SIG_PCM1BIT_SIGNAL, 0, 1);
			pcm1bit->write_signal(SIG_PCM1BIT_ON, 0, 1);
			//beep->write_signal(SIG_BEEP_ON, 0, 1);
		}
		beep_flag = flag;
	}
	if((data & 0x40) != 0) {
		// BEEP ON, after 205ms, BEEP OFF.  
		if(event_beep_oneshot >= 0) cancel_event(this, event_beep_oneshot);
		register_event(this, EVENT_BEEP_OFF, 205.0 * 1000.0, false, &event_beep_oneshot); // NEXT CYCLE
	}
}

// SIGNAL / FM7_MAINIO_BEEP
void FM7_MAINIO::set_beep_oneshot(void) // SUB:D4xx
{

	beep_snd = true;
	pcm1bit->write_signal(SIG_PCM1BIT_ON, 1, 1);
	//beep->write_signal(SIG_BEEP_ON, 1, 1);
	if(event_beep_oneshot >= 0) cancel_event(this, event_beep_oneshot);
	register_event(this, EVENT_BEEP_OFF, 205.0 * 1000.0, false, NULL); // NEXT CYCLE
}

// EVENT_BEEP_OFF
void FM7_MAINIO::event_beep_off(void)
{
	beep_flag = false;
	beep_snd = false;
	pcm1bit->write_signal(SIG_PCM1BIT_ON, 0, 1);
	event_beep_oneshot = -1;
	//beep->write_signal(SIG_BEEP_ON, 0, 1);
}

// EVENT_BEEP_CYCLE
void FM7_MAINIO::event_beep_cycle(void)
{
	beep_snd = !beep_snd;
	//if(!beep_flag) return;   
	pcm1bit->write_signal(SIG_PCM1BIT_SIGNAL, beep_snd ? 1 : 0, 1);
}
