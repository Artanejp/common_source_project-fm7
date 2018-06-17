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

#include "../pcm1bit.h"
#include "../datarec.h"
#include "../ym2203.h"
#include "fm7_mainio.h"
#include "../ay_3_891x.h"
#include "../../config.h"
#include "./joystick.h"

void FM7_MAINIO::reset_sound(void)
{
	int i, j;

//#if !defined(_FM8)
	for(i = 0; i < 3; i++) {
		opn_data[i]= 0;
		opn_cmdreg[i] = 0;
		opn_address[i] = 0;
		opn_stat[i] = 0;
		opn_ch3mode[i] = 0x00;
		opn_prescaler_type[i] = 1;
		memset(opn_regs[i], 0x00, 0x100 * sizeof(uint8_t));
		if(opn[i] != NULL) {
			opn[i]->reset();
			opn[i]->write_data8(0, 0x2e);
			opn[i]->write_data8(1, 0);	// set prescaler
			opn[i]->write_data8(0, 0x27);
			opn[i]->write_data8(1, 0x00);
#if 0			
			for(int ch = 0x00; ch < 0x0e; ch++) { // PSG from XM7.
				if(ch == 7) {
					opn[i]->set_reg(ch, 0xff); 
				} else {
					opn[i]->set_reg(ch, 0x00);
				}
			}
			for(int ch = 0x30; ch < 0x40; ch++) {  // MUL, DT from XM7.
				if((ch & 0x03) != 3) {
					opn[i]->set_reg(ch, 0);
				}
			}
			for(int ch = 0x40; ch < 0x50; ch++) { // TL
				if((ch & 0x03) != 3) {
					opn[i]->set_reg(ch, 0x7f);
					opn_regs[i][ch] = 0x7f;
				}
			}
			for(int ch = 0x50; ch < 0x60; ch++) { // AR
				if((ch & 0x03) != 3) {
					opn[i]->set_reg(ch, 0x1f);
					opn_regs[i][ch] = 0x1f;
				}
			}
			for(int ch = 0x60; ch < 0xb4; ch++) { // MISC
				if((ch & 0x03) != 3) {
					opn[i]->set_reg(ch, 0x7f);
					opn_regs[i][ch] = 0x7f;
				}
			}
			for(int ch = 0x80; ch < 0x90; ch++) { // SL/RR
				if((ch & 0x03) != 3) {
					opn[i]->set_reg(ch, 0xff);
					opn_regs[i][ch] = 0xff;
				}
			}
			// Note
			for(j = 0; j < 3; j++) {
				opn[i]->set_reg(0x28, j | 0xfe);
				opn_regs[i][0x28] = (uint8_t)(j | 0xfe);
				//opn_keys[i][j] = j | 0xfe;
			}
			write_opn_reg(i, 0x2e, 0);
			write_opn_reg(i, 0x27, 0);
#endif
		}
	}
//#endif
#if !defined(_FM77AV_VARIANTS)
	if(psg != NULL) {
		psg->reset();
	}
#endif
 #if defined(_FM77AV_VARIANTS)
	opn_psg_77av = true;
 #else
	opn_psg_77av = false;
 #endif
	connect_opn = connect_whg = connect_thg = false;
	if(opn_psg_77av) connect_opn = true;

	connect_thg = false;
	connect_whg = false;
#if defined(_FM77AV_VARIANTS)	
	connect_opn = true;
#else
	connect_opn = false;
#endif
	switch(config.sound_type) {
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
	pcm1bit->write_signal(SIG_PCM1BIT_MUTE, 0x01, 0x01);
	pcm1bit->write_signal(SIG_PCM1BIT_ON, 0x00, 0x01);
	
	opn[0]->write_signal(SIG_YM2203_MUTE, !connect_opn ? 0xffffffff : 0x00000000, 0xffffffff);
	opn[1]->write_signal(SIG_YM2203_MUTE, !connect_whg ? 0xffffffff : 0x00000000, 0xffffffff);
	opn[2]->write_signal(SIG_YM2203_MUTE, !connect_thg ? 0xffffffff : 0x00000000, 0xffffffff);
# if !defined(_FM77AV_VARIANTS)
#  if defined(USE_AY_3_8910_AS_PSG)
	psg->write_signal(SIG_AY_3_891X_MUTE, 0x00000000, 0xffffffff);
#  else
	psg->write_signal(SIG_YM2203_MUTE, 0x00000000, 0xffffffff);
#  endif
# endif
}

// After loading state, OPN's prescaler needs to reset.
void FM7_MAINIO::restore_opn(void)
{
#if 1
	for(int i = 0; i < 3; i++) {
		if(opn[i] != NULL) {
			int pre = opn_prescaler_type[i];
			if((pre >= 0) && (pre < 3)) {
				write_opn_reg(i, pre + 0x2d, 0);
			}
			// Key
			write_opn_reg(i, 0x27, opn_regs[i][0x27]);
			//for(int j = 0; j < 3; j++) {
				//opn[i]->set_reg(0x28, opn_keys[i][j]);
			//	write_opn_reg(i, 0x28, (uint8_t)j);
			//}
			for(int j = 0x0; j < 0x0d; j++) {
				if((j < 0x08) || (j > 0x0a)) {
					write_opn_reg(i, j, opn_regs[i][j]);
				} else {
					write_opn_reg(i, j, 0);
				}
			}
			for(int j = 0x30; j < 0xb4; j++) {
				write_opn_reg(i, j, opn_regs[i][j]);
			}
			
		}
	}
# if !defined(_FM77AV_VARIANTS)
	if(psg != NULL) {
		//psg->set_reg(0x27, opn_regs[3][0x27]);
		psg->set_reg(0x2e, 0);
	}
#endif
#endif
}

void FM7_MAINIO::set_psg(uint8_t val)
{
	if(opn_psg_77av) return set_opn(0, val); // 77AV ETC
	set_opn(3, val);
}

uint8_t FM7_MAINIO::get_psg(void)
{
	if(opn_psg_77av) {
		return get_opn(0);
	}
	return get_opn(3);
}

/*
 * $fd0d : After 77AV, this is OPN.
 */
void FM7_MAINIO::set_psg_cmd(uint8_t cmd)
{
	cmd = cmd & 0x03;
	if(opn_psg_77av) {
		set_opn_cmd(0, cmd);
		return;
	}
	set_opn_cmd(3, cmd);
	return;
}

// OPN
// Write to FD16, same as 
void FM7_MAINIO::write_opn_reg(int index, uint32_t addr, uint32_t data)
{
	//out_debug_log("OPN_WRITE: #%d REG=%02x VAL=%02x\n", index, addr, data);
# if !defined(_FM77AV_VARIANTS)
	if(index == 3) { // PSG
	  	psg->write_io8(0, addr & 0x0f);
		psg->write_io8(1, data);
		opn_regs[index][addr] = data;
		return;
	}
# endif

	if((addr >= 0x2d) && (addr < 0x30)) {
		opn_prescaler_type[index] = addr - 0x2d;
		opn[index]->write_io8(0, addr);
		
		opn_regs[index][addr] = 0;
		return;
	} else if(addr == 0x27) {
		//if(opn_ch3mode[index] == data) { // From XM7.
		//	opn_regs[index][addr] = data;
		//	return;
		//}
		opn_ch3mode[index] = data & 0xc0;
	} else  if(addr == 0x28) {
		//opn_keys[index][data & 3] = data;
	}/* else if(addr == 0xff) { // Check : from XM7.
		if((opn_regs[index][0x27] & 0xc0) != 0x80) {
			opn_regs[index][addr] = data;
			return;
		}
		}*/
	opn[index]->write_io8(0, addr);
	opn[index]->write_io8(1, data);
	opn_regs[index][addr] = data;
	return;
}

void FM7_MAINIO::set_opn(int index, uint8_t val)
{
	if((index > 3) || (index < 0)) return;
	if((index == 0) && (!connect_opn)) return;
	if((index == 1) && (!connect_whg)) return;
	if((index == 2) && (!connect_thg)) return;
	if((index == 3) && (opn_psg_77av)) return;
# if !defined(_FM77AV_VARIANTS)	
	if(index == 3) {
		if(psg == NULL) return;
	} else
# endif
	if(opn[index] == NULL) {
		return;
	}

	opn_data[index] = val;
	switch(opn_cmdreg[index]){
		case 0: // High inpedance
		case 1: // Read Data
			break;
		case 2: // Write Data
			write_opn_reg(index, opn_address[index], opn_data[index]);
			break;
		case 3: // Register address
			if(index == 3) {
				opn_address[index] = val & 0x0f;
			} else {
				opn_address[index] = val;
//#if !defined(_FM8)
				if((val > 0x2c) && (val < 0x30)) {
					opn_prescaler_type[index] = val - 0x2d;
					opn_data[index] = 0;
					opn[index]->write_io8(0, val);
					opn[index]->write_io8(1, 0);
				}

//#endif
			}
			break;
		default:
			break;
	}
}

uint8_t FM7_MAINIO::get_opn(int index)
{
	uint8_t val = 0xff;
	if((index > 2) || (index < 0)) return val;
	if((index == 0) && (!connect_opn)) return val;
	if((index == 1) && (!connect_whg)) return val;
	if((index == 2) && (!connect_thg)) return val;
	if((index == 3) && (opn_psg_77av)) return val;
# if !defined(_FM77AV_VARIANTS)	
	if(index == 3) {
		if(psg == NULL) return val;
	} else
# endif
		if(opn[index] == NULL) {
		return val;
	}

	switch(opn_cmdreg[index]) {
		case 0:
		case 1:
		case 2:
		case 3:
			val = opn_data[index];
			break;
//#if !defined(_FM8)
		case 4:
			opn_stat[index] = opn[index]->read_io8(0) & 0x03;
			if(index != 3) val = opn_stat[index];
	   		break;
		case 0x09:
			if(index != 0) return 0xff;
			if(opn_address[0] == 0x0e) {
				//return opn[0]->read_signal(SIG_YM2203_PORT_A);
				return joystick->read_data8(0);
			}
			return 0x00;
			break;
//#endif
		default:
	 		break;
		}
		return val;
}
  /*
   * $fd16?
   */
void FM7_MAINIO::set_opn_cmd(int index, uint8_t cmd)
{
	if((index >= 4) || (index < 0)) return;
	if((index == 0) && (!connect_opn)) return;
	if((index == 1) && (!connect_whg)) return;
	if((index == 2) && (!connect_thg)) return;
	if((index == 3) && (opn_psg_77av)) return ;
# if !defined(_FM77AV_VARIANTS)	
	if(index == 3) {
		if(psg == NULL) return;
	}
# endif
	static const uint32_t mask[16] = { // Parameter is related by XM7. Thanks Ryu.
		0xff, 0x0f, 0xff, 0x0f,
		0xff, 0x0f, 0x1f, 0xff,
		0x1f, 0x1f, 0x1f, 0xff,
		0xff, 0x0f, 0xff, 0xff
	};
	opn_cmdreg[index] = cmd & 0x0f;
	uint8_t val = opn_data[index];
        switch(opn_cmdreg[index]) {
		case 0:
			break;
		case 1:
#if !defined(_FM77AV_VARIANTS)	
			if(index == 3) {
				psg->write_io8(0, opn_address[index]);
				opn_data[index] = psg->read_io8(1);
			} else {
				opn[index]->write_io8(0, opn_address[index]);
				opn_data[index] = opn[index]->read_io8(1);
			}				
#else
			opn[index]->write_io8(0, opn_address[index]);
			opn_data[index] = opn[index]->read_io8(1);
#endif
			if(opn_address[index] <= 0x0f) {
				opn_data[index] &= mask[opn_address[index]];
			}
			break;
		case 2:
			write_opn_reg(index, opn_address[index], opn_data[index]);
	 		break;
		case 3: // Register address
			if(index == 3) {
				opn_address[index] = val & 0x0f;
			} else {
				opn_address[index] = val;
//#if !defined(_FM8)
				if((val > 0x2c) && (val < 0x30)) {
					opn_prescaler_type[index] = val - 0x2d;
					opn_data[index] = 0;
					opn[index]->write_io8(0, val);
					opn[index]->write_io8(1, 0);
				}
//#endif
			}

			break;
//#if !defined(_FM8)
		case 4:
			opn_stat[index] = opn[index]->read_io8(0) & 0x03;
	   		break;
//#endif
	 	default:
	   		break;
	}
	return;
}

uint8_t FM7_MAINIO::get_extirq_whg(void)
{
	uint8_t val = 0xff;
	if(intstat_whg && connect_whg) val &= ~0x08;
	return val;
}

uint8_t FM7_MAINIO::get_extirq_thg(void)
{
	uint8_t val = 0xff;
	if(intstat_thg && connect_thg) val &= ~0x08;
	return val;
}

void FM7_MAINIO::opn_note_on(int index)
{
	uint8_t r;
//#if !defined(_FM8)
	if((index < 0) || (index >= 3)) return;
	// Not on for CSM mode. From XM7. Thanks, Ryu.
	r = opn_ch3mode[index];
	if ((r & 0xc0) == 0x80) {
		opn[index]->write_io8(0, 0x27);
		opn[index]->write_io8(1, opn_ch3mode[index] & 0xc0);
	}
//#endif	
}


void FM7_MAINIO::set_beep(uint32_t data) // fd03
{
	bool flag = ((data & 0xc0) != 0);
	pcm1bit->write_signal(SIG_PCM1BIT_MUTE, ~data, 0x01);
	if(flag != beep_flag) {
		if(flag) {
			beep_snd = true;
			pcm1bit->write_signal(SIG_PCM1BIT_SIGNAL, 1, 1);
			pcm1bit->write_signal(SIG_PCM1BIT_ON, 1, 1);
			//if(event_beep <= -1) {
			//	register_event(this, EVENT_BEEP_CYCLE, (1000.0 * 1000.0) / (1200.0 * 2.0), true, &event_beep);
			//}		
		} else {
			beep_snd = false;
			pcm1bit->write_signal(SIG_PCM1BIT_SIGNAL, 0, 1);
			pcm1bit->write_signal(SIG_PCM1BIT_ON, 0, 1);
			//if(event_beep > -1) {
			//	cancel_event(this, event_beep);
			//	event_beep = -1;
			//}		
		}
		beep_flag = flag;
	}
	if((data & 0x40) != 0) {
		// BEEP ON, after 205ms, BEEP OFF.  
		set_beep_oneshot();
	}
}

// SIGNAL / FM7_MAINIO_BEEP
void FM7_MAINIO::set_beep_oneshot(void) // SUB:D4xx
{
	beep_snd = true;
	beep_flag = true;
	pcm1bit->write_signal(SIG_PCM1BIT_ON, 1, 1);
	if(event_beep_oneshot >= 0) cancel_event(this, event_beep_oneshot);
	register_event(this, EVENT_BEEP_OFF, 205.0 * 1000.0, false, &event_beep_oneshot); // NEXT CYCLE
	//if(event_beep <= -1) {
	//	register_event(this, EVENT_BEEP_CYCLE, (1000.0 * 1000.0) / (1200.0 * 2.0), true, &event_beep);
	//}		
}

// EVENT_BEEP_OFF
void FM7_MAINIO::event_beep_off(void)
{
	beep_flag = false;
	beep_snd = false;
	pcm1bit->write_signal(SIG_PCM1BIT_ON, 0, 1);
	event_beep_oneshot = -1;
	//if(event_beep >= 0) cancel_event(this, event_beep);
	//event_beep = -1;
}

// EVENT_BEEP_CYCLE
void FM7_MAINIO::event_beep_cycle(void)
{
	beep_snd = !beep_snd;
	if(beep_flag) {
		pcm1bit->write_signal(SIG_PCM1BIT_SIGNAL, beep_snd ? 1 : 0, 1);
	}
}

#include "../../statesub.h"

void FM7_MAINIO::decl_state_opn(void)
{

	DECL_STATE_ENTRY_BOOL(connect_opn);
	DECL_STATE_ENTRY_BOOL(connect_whg);
	DECL_STATE_ENTRY_BOOL(connect_thg);

	DECL_STATE_ENTRY_BOOL(opn_psg_77av);
	DECL_STATE_ENTRY_UINT8_ARRAY(opn_address, 4);
	DECL_STATE_ENTRY_UINT8_ARRAY(opn_data, 4);
	DECL_STATE_ENTRY_UINT8_ARRAY(opn_stat, 4);
	DECL_STATE_ENTRY_UINT8_ARRAY(opn_cmdreg, 4);
	DECL_STATE_ENTRY_UINT8_ARRAY(opn_ch3mode, 4);
	DECL_STATE_ENTRY_UINT8_ARRAY(opn_prescaler_type, 4);
	DECL_STATE_ENTRY_2D_ARRAY(opn_regs, 4, 0x100);
}
