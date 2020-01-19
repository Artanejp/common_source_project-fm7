/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[ADPCM RF6C68 with RAM]
*/

#include "../../common.h"
#include "./rf5c68.h"

#define EVENT_DAC_SAMPLE 1

void RF5C68::initialize()
{
	// DAC
	memset(wave_memory, 0x00, sizeof(wave_memory));


	dac_on = false;
	dac_bank = 0;
	dac_ch = 0;
	for(int i = 0; i < 8; i++) {
		dac_addr_st[i].d = 0x00;
		dac_env[i] = 0x0000000;
		dac_lpan[i] = 0x000000f;
		dac_rpan[i] = 0x000000f;
		dac_tmpval_l[i] = 0x00000000;
		dac_tmpval_r[i] = 0x00000000;
		dac_ls[i].d = 0x0000;
		dac_fd[i].d = 0x0000;
		dac_onoff[i] = false;
		dac_addr[i] = 0x00000000;
		dac_force_load[i] = true;
	}
	dac_on = false;
	dac_bank = 0;
	dac_ch = 0;

}

void RF5C68::reset()
{
	is_mute = true; // OK?
	for(int i = 0; i < 8; i++) {
		dac_addr_st[i].d = 0x00;
		dac_env[i] = 0x0000000;
		dac_lpan[i] = 0x000000f;
		dac_rpan[i] = 0x000000f;
		dac_tmpval_l[i] = 0x00000000;
		dac_tmpval_r[i] = 0x00000000;
		dac_ls[i].d = 0x0000;
		dac_fd[i].d = 0x0000;
		dac_onoff[i] = false;
		dac_addr[i] = 0x00000000;
		dac_force_load[i] = true;
	}
	if((sample_buffer != NULL) && (sample_length > 0)) {
		memset(sample_buffer, 0x00, sample_length * sizeof(int32_t) * 2);
	}
	if(event_dac_sample != -1) {
		cancel_event(this, event_dac_sample);
		event_dac_sample = -1;
	}
	if(mix_rate > 0) {
		sample_tick_us = 1.0e6 / ((double)mix_rate);
		register_event(this, EVENT_DAC_SAMPLE, sample_tick_us, true, &event_dac_sample);
	} else {
		sample_tick_us = 0;
	}
	sample_count = 0;
}

uint32_t RF5C68::read_signal(int ch)
{
	if(ch >= SIG_RF5C68_REG_ADDR_ST) {
		if(ch >= 0x100) return 0x00;
		int major_num = ch & 0xf8;
		int local_ch = ch & 0x07;
		switch(major_num) {
		case SIG_RF5C68_REG_ADDR_ST:
			return dac_addr_st[local_ch].d;
			break;
		case SIG_RF5C68_REG_ADDR:
			return dac_addr[local_ch];
			break;
		case SIG_RF5C68_REG_ENV:
			return dac_env[local_ch];
			break;
		case SIG_RF5C68_REG_LPAN:
			return dac_lpan[local_ch];
			break;
		case SIG_RF5C68_REG_RPAN:
			return dac_rpan[local_ch];
			break;
		case SIG_RF5C68_REG_LS:
			return dac_ls[local_ch].d;
			break;
		case SIG_RF5C68_REG_FD:
			return dac_fd[local_ch].d;
			break;
		case SIG_RF5C68_FORCE_LOAD:
			return ((dac_force_load[local_ch]) ? 0xffffffff : 0x00000000);
			break;
		default:
			break;
		}
	} else {
		switch(ch) {
		case SIG_RF5C68_MUTE:
			return ((is_mute) ? 0xffffffff : 0x00000000);
			break;
		case SIG_RF5C68_REG_ON:
			return ((dac_on) ? 0xffffffff : 0x00000000);
			break;
		case SIG_RF5C68_REG_BANK:
			return dac_bank;
			break;
		case SIG_RF5C68_REG_CH:
			return dac_ch;
			break;
		default:
			break;
		}
	}
	return 0x00;
}

void RF5C68::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch)
	{
	case SIG_RF5C68_DAC_PERIOD:
		if(dac_on) {
			for(int ch = 0; ch < 8; ch++) {
				if(dac_onoff[ch]) {
					uint32_t addr_old = (dac_addr[ch] & 0x7fffffff) >> 11;
					uint32_t addr_new;
					dac_addr[ch] += dac_fd[ch].d;
					dac_addr[ch] = dac_addr[ch] & 0x7fffffff;
					addr_new = dac_addr[ch] >> 11;
					if((addr_old != addr_new) || (dac_force_load[ch])) {
					    pair32_t tmpval;
						tmpval.b.l = wave_memory[addr_new & 0xffff];
						if((addr_new & 0xf000) != (addr_old & 0xf000)) { // Boundary
							if((addr_new & 0x1000) != 0) {
								write_signals(&interrupt_boundary, ((addr_new & 0xe000) >> 13) | 0x00000008);
							}
						}
						if(dac_force_load[ch]) {
							dac_addr[ch] = (uint32_t)(dac_addr_st[ch].w.l) << 11;
							addr_new = dac_addr[ch] >> 11;
							tmpval.b.l = wave_memory[addr_new & 0xffff];
						}
						dac_force_load[ch] = false;
						
						if(tmpval.b.l == 0xff) {
							// Skip
							dac_force_load[ch] = true;
							// Q: Is clear data reg?
							dac_tmpval_l[ch] = 0;
							dac_tmpval_r[ch] = 0;
						} else { // Normal OP
							uint32_t sign = tmpval.d & 0x80;
							uint32_t val = tmpval.d & 0x7f;
							uint32_t lval, rval;
							val = val * dac_env[ch];
							lval = val * dac_lpan[ch];
							rval = val * dac_rpan[ch];
					   		if(sign != 0) { // ADD
								dac_tmpval_l[ch] += lval;
								dac_tmpval_r[ch] += rval;
							} else { // SUB
								dac_tmpval_l[ch] -= lval;
								dac_tmpval_r[ch] -= rval;
							}
							// Limiter
							if(dac_tmpval_l[ch] >= (127 << 6)) {
								dac_tmpval_l[ch] = 127 << 6;
							} else if(dac_tmpval_l[ch] < -(127 << 6)) {
								dac_tmpval_l[ch] = -(127 << 6);
							} 
							if(dac_tmpval_r[ch] >= (127 << 6)) {
								dac_tmpval_r[ch] = 127 << 6;
							} else if(dac_tmpval_r[ch] < -(127 << 6)) {
								dac_tmpval_r[ch] = -(127 << 6);
							}
						}
					}
				} else {
					dac_tmpval_l[ch] = 0;
					dac_tmpval_r[ch] = 0;
				}
			}
		}
		break;
	case SIG_RF5C68_CLEAR_INTR:
		write_signals(&interrupt_boundary, 0x80000000);
		break;
	case SIG_RF5C68_SET_ALL_INTR:
		write_signals(&interrupt_boundary, 0x80000008);
		break;
	case SIG_RF5C68_MUTE:
		is_mute = ((data & mask) != 0) ? true : false;
		break;
	default:
		break;
	}
}

void RF5C68::write_io8(uint32_t addr, uint32_t data)
{
	uint32_t naddr = addr & 0x0f;
	switch(naddr) {
	case 0x00: // ENV
		dac_env[dac_ch] = data & 0xff;
		break;
	case 0x01: // PAN
		dac_lpan[dac_ch] = data & 0x0f;
		dac_rpan[dac_ch] = (data & 0xf0) >> 4;
		break;
	case 0x02: // FDL
		dac_fd[dac_ch].b.l = data & 0xff;
		break;
	case 0x03: // FDH
		dac_fd[dac_ch].b.h = data & 0xff;
		break;
	case 0x04: // LSL
		dac_ls[dac_ch].b.l = data & 0xff;
		break;
	case 0x05: // LSH
		dac_ls[dac_ch].b.h = data & 0xff;
		break;
	case 0x06: // ST
		dac_addr_st[dac_ch].d = 0;
		dac_addr_st[dac_ch].b.h = data & 0xff;
		break;
	case 0x07: // Control
		dac_on = ((data & 0x80) != 0) ? true : false;
		if((data & 0x40) != 0) { // CB2-0
			dac_ch = data & 0x07;
		} else { // WB3-0
			dac_bank = ((data & 0x0cf) << 12);
		}
		break;
	case 0x08: // ON/OFF per CH
		{
			uint32_t mask = 0x01;
			for(int i = 0; i < 8; i++) {
				bool onoff = dac_onoff[i];
				if((mask & data) != 0) {
					dac_onoff[i] = true;
				} else {
					dac_onoff[i] = false;
				}
				if(onoff != dac_onoff[i]) {
					dac_force_load[i] = true;
				}
			}
			mask <<= 1;
		}
		break;
	default:
		break;
	}
}

uint32_t RF5C68::read_io8(uint32_t addr)
{
	return 0xff;
}

// Read PCM memory
uint32_t RF5C68::read_data8(uint32_t addr)
{
	if(dac_on) {
		return 0xff;
	}
	// dac_off
	return wave_memory[(addr & 0x0fff) | dac_bank];
}

void RF5C68::write_data8(uint32_t addr, uint32_t data)
{
	// if(dac_on) don't write <- Is correct?
	if(!dac_on) {
		wave_memory[(addr & 0x0fff) | dac_bank] = (uint8_t)data;
	}
}

void RF5C68::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
}

void RF5C68::event_callback(int id, int err)
{
	if(id == EVENT_DAC_SAMPLE) {
		int32_t lval, rval;
		lval = 0;
		rval = 0;
		if(sample_count < sample_length) {
			if(dac_on) {
				for(int ch = 0; ch < 8; ch++) {
					if(dac_onoff[ch]) {
						lval = lval + (dac_tmpval_l[ch] << 0);
						rval = rval + (dac_tmpval_r[ch] << 0);
					}
				}
			}
			sample_buffer[sample_count << 1] = lval;
			sample_buffer[(sample_count << 1) + 1] = rval;
			sample_count++;
		}
	}
}

void RF5C68::mix(int32_t* buffer, int cnt)
{
	
	int32_t lval, rval;
	// ToDo: supress pop noise.
	if(sample_length < cnt) cnt = sample_length;
	if(sample_length < sample_count) sample_count = sample_length;
	if(cnt <= 0) return;
	if(is_mute) return;
	
	if(sample_buffer != NULL) {
		for(int i = 0; i < (cnt << 1); i += 2) {
			// ToDo: interpoolate.
			buffer[i]     += apply_volume(sample_buffer[i],     volume_l);
			buffer[i + 1] += apply_volume(sample_buffer[i + 1], volume_r);
		}
		if(sample_count > cnt) {
			sample_count -= cnt;
			memcpy(&(sample_buffer[0]), &(sample_buffer[cnt * 2]), sample_count * sizeof(int32_t) * 2);
			memset(&(sample_buffer[cnt * 2]), 0x00, (sample_length - sample_count)  * sizeof(int32_t) * 2);
		} else {
			memset(&(sample_buffer[0]), 0x00, sample_length * sizeof(int32_t) * 2);
			sample_count = 0;
		}
	}
}

void RF5C68::initialize_sound(int sample_rate, int samples)
{
	if((sample_rate > 0) && (samples > 0)) {
		mix_rate = sample_rate;
		sample_length = samples;
		if(sample_buffer != NULL) {
			free(sample_buffer);
		}
		sample_buffer = (int32_t*)malloc(sample_length * sizeof(int32_t) * 2);
		if(sample_buffer != NULL) {
			memset(sample_buffer, 0x00, sample_length * sizeof(int32_t) * 2);
		}
		if(event_dac_sample != -1) {
			cancel_event(this, event_dac_sample);
			event_dac_sample = -1;
		}
		if(mix_rate > 0) {
			// TOWNS::ADPCM::event_callback() -> SIG_RF5C68_DAC_PERIOD(=DRIVE CLOCK from VM)
			// -> RF5C68::event_callback()(=AUDIO MIXING CLOCK by EMU)
			// -> RF5C68::mix() -> OSD::SOUND
			sample_tick_us = 1.0e6 / ((double)mix_rate);
			register_event(this, EVENT_DAC_SAMPLE, sample_tick_us, true, &event_dac_sample);
		}
	} else {
		if(sample_buffer != NULL) {
			free(sample_buffer);
		}
		sample_buffer = NULL;
		sample_length = 0;
		mix_rate = 0;
		if(event_dac_sample != -1) {
			cancel_event(this, event_dac_sample);
			event_dac_sample = -1;
		}
		sample_tick_us = 0.0;
	}
	sample_count = 0;
}

#define STATE_VERSION	1

bool RF5C68::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(dac_on);
	state_fio->StateValue(dac_bank);
	state_fio->StateValue(dac_ch);
	state_fio->StateValue(dac_on);
	state_fio->StateValue(is_mute);
	state_fio->StateArray(dac_onoff, sizeof(dac_onoff), 1);
	state_fio->StateArray(dac_addr_st, sizeof(dac_addr_st), 1);	
	state_fio->StateArray(dac_addr, sizeof(dac_addr), 1);
	state_fio->StateArray(dac_env, sizeof(dac_env), 1);
	state_fio->StateArray(dac_lpan, sizeof(dac_lpan), 1);
	state_fio->StateArray(dac_rpan, sizeof(dac_rpan), 1);
	state_fio->StateArray(dac_ls, sizeof(dac_ls), 1);
	state_fio->StateArray(dac_fd, sizeof(dac_fd), 1);
	state_fio->StateArray(dac_force_load, sizeof(dac_force_load), 1);
	state_fio->StateArray(dac_tmpval_l, sizeof(dac_tmpval_l), 1);
	state_fio->StateArray(dac_tmpval_r, sizeof(dac_tmpval_r), 1);
	
	state_fio->StateArray(wave_memory, sizeof(wave_memory), 1);
	state_fio->StateValue(event_dac_sample);

	// Post Process
	if(loading) {
		if(event_dac_sample != -1) {
			cancel_event(this, event_dac_sample);
			event_dac_sample = -1;
		}
		if(mix_rate > 0) {
			sample_tick_us = 1.0e6 / ((double)mix_rate);
			register_event(this, EVENT_DAC_SAMPLE, sample_tick_us, true, &event_dac_sample);
		} else {
			sample_tick_us = 0;
		}
		sample_count = 0;
	}
	return true;
}
