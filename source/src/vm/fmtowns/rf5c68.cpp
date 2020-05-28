/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[ADPCM RF6C68 with RAM]
*/

#include "../../common.h"
#include "./rf5c68.h"
#include "../debugger.h"

void RF5C68::initialize()
{
	// DAC
	memset(wave_memory, 0x00, sizeof(wave_memory));
	dac_bank = 0;
	dac_ch = 0;
	for(int i = 0; i < 8; i++) {
		dac_addr_st[i].d = 0x00;
		dac_env[i] = 0x0000080;
		dac_pan[(i << 1) + 0] = 0x000000f;
		dac_pan[(i << 1) + 1] = 0x000000f;
		dac_ls[i].d = 0x0000;
		dac_fd[i].d = 0x0000;
		dac_onoff[i] = false;
		dac_addr[i] = 0x00000000;
		dac_force_load[i] = false;
	}
	dac_bank = 0;
	dac_ch = 0;
	sample_buffer = NULL;
	if(d_debugger != NULL) {
		d_debugger->set_device_name(_T("Debugger (RICOH RF5C68)"));
		d_debugger->set_context_mem(this);
		d_debugger->set_context_io(vm->dummy);
	}
}

void RF5C68::release()
{
	if(sample_buffer != NULL) free(sample_buffer);
	sample_buffer = NULL;
}

void RF5C68::reset()
{
	is_mute = true; // OK?
	for(int i = 0; i < 8; i++) {
		dac_addr_st[i].d = 0x00;
		dac_env[i] = 0x0000080;
		dac_pan[(i << 1) + 0] = 0x0000008;
		dac_pan[(i << 1) + 1] = 0x0000008;
		dac_ls[i].d = 0x0000;
		dac_fd[i].d = 0x0000;
		dac_onoff[i] = false;
		dac_addr[i] = 0x00000000;
		dac_force_load[i] = false;
	}
	for(int i = 0; i < 16; i++) {
		dac_tmpval[i] = 0x00000000;
	}
	if((sample_buffer != NULL) && (sample_length > 0)) {
		memset(sample_buffer, 0x00, sample_length * sizeof(int32_t) * 2);
	}
	read_pointer = 0;
	sample_pointer = 0;
	sample_words = 0;
	
	if(mix_rate > 0) {
		sample_tick_us = 1.0e6 / ((double)mix_rate);
	} else {
		sample_tick_us = 0;
	}
	mix_factor = (int)(dac_rate * 4096.0 / (double)mix_rate);
	mix_count = 0;
	dac_on = false;
	
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
			return dac_pan[(local_ch << 1) + 0];
			break;
		case SIG_RF5C68_REG_RPAN:
			return dac_pan[(local_ch << 1) + 1];
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
		__DECL_ALIGNED(16) uint8_t tmpval[8] = {0};
__DECL_VECTORIZED_LOOP
			for(int i = 0; i < 16; i++) {
				dac_tmpval[i] = 0;
			}
			for(int ch = 0; ch < 8; ch++) {
				if(dac_onoff[ch]) {
					uint32_t addr_old = (dac_addr[ch] >> 11) & 0xffff;
					uint32_t addr_new;
					if((addr_old & 0x0fff) == 0x0fff) {
						// Will beyond of boundary
						write_signals(&interrupt_boundary, ((addr_old & 0xe000) >> 13) | 0x00000008);
//						out_debug_log(_T("PCM INTERRUPT CH=%d ADDR=%04X"), ch, addr_old & 0xffff);
					}
				
					int chd = ch << 1;
					tmpval[ch] = wave_memory[addr_old & 0xffff];
					if(tmpval[ch] == 0xff) {
						// Loop
						dac_addr[ch] = ((uint32_t)(dac_ls[ch].w.l)) << 11;
						addr_old = ((uint32_t)(dac_ls[ch].w.l));
						tmpval[ch] = wave_memory[addr_old & 0xffff];
						if(tmpval[ch] == 0xff) {
							tmpval[ch] = 0x00;
							dac_onoff[ch] = false; // STOP
							continue; // This channel will stop
						} 
					} 
					dac_addr[ch] += dac_fd[ch].d;
//					dac_addr[ch] = dac_addr[ch] & ((1 << 28) - 1);
				}
			}
			__DECL_ALIGNED(16) bool sign[16] = {false};
			__DECL_ALIGNED(16) int32_t val[16] = {0};
__DECL_VECTORIZED_LOOP			
			for(int ch = 0; ch < 8; ch++) {
				int chd = ch << 1;
				sign[chd + 0] = (dac_onoff[ch]) ? ((tmpval[ch] & 0x80) == 0) : false; // 0 = minus
				sign[chd + 1] = sign[chd + 0];
				val[chd + 0] = (dac_onoff[ch]) ? (tmpval[ch] & 0x7f) : 0;
				val[chd + 1] = val[chd + 0];
			}
			// VAL = VAL * ENV * PAN
__DECL_VECTORIZED_LOOP			
			for(int chd = 0; chd < 16; chd++) {
				val[chd] = val[chd] * dac_env[chd >> 1];
				val[chd] = val[chd] * dac_pan[chd];
			}
			// Sign
__DECL_VECTORIZED_LOOP			
			for(int chd = 0; chd < 16; chd++) {
				dac_tmpval[chd] = (sign[chd]) ? -val[chd] : val[chd];
			}
			// Re-Init sample buffer
			if((sample_buffer != NULL) /*&& (sample_words < sample_length)*/){
				int32_t* np = &(sample_buffer[sample_pointer << 1]);
				__DECL_ALIGNED(8) int32_t lr[2];
				for(int i = 0; i < 2; i++) {
					lr[i] = 0;
				}
				// ADD or SUB
__DECL_VECTORIZED_LOOP
				for(int chd = 0; chd < 16; chd++) {
					lr[chd & 1] += dac_tmpval[chd];
				}
/*
			static const int32_t uplimit = 127 << 6;
			static const int32_t lowlimit  = -(127 << 6);
__DECL_VECTORIZED_LOOP
				for(int chd = 0; chd < 2; chd++) {
					if(lr[chd] > uplimit) {
						lr[chd] = uplimit;
					}
					if(lr[chd] < lowlimit) {
						lr[chd] = lowlimit;
					}
					lr[chd] >>= 2;
				}
*/
__DECL_VECTORIZED_LOOP
				for(int chd = 0; chd < 2; chd++) {
					lr[chd] >>= 2;
				}
				np[0] = lr[0];
				np[1] = lr[1];
			}
			sample_pointer = (sample_pointer + 1) % sample_length;
			sample_words++;
			sample_words = (sample_words >= sample_length) ? sample_length : sample_words;
		}
		break;
	case SIG_RF5C68_CLEAR_INTR:
		write_signals(&interrupt_boundary, 0x80000000);
		break;
	case SIG_RF5C68_SET_ALL_INTR:
		write_signals(&interrupt_boundary, 0x80000008);
		break;
	case SIG_RF5C68_MUTE:
		{
			bool old_is_mute = is_mute;
			is_mute = ((data & mask) != 0) ? true : false;
			if((is_mute != old_is_mute) && !(is_mute)) {
				sample_words = 0;
				sample_pointer = 0;
				read_pointer = 0;
				if((sample_buffer != NULL) && (sample_length > 0)) {
					memset(sample_buffer, 0x00, sizeof(int32_t) * 2 * sample_length);
				}
			}
		}
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
//		out_debug_log(_T("DAC REG 00 (ENV) CH=%d RAW=%02X"),
//					  dac_ch, data);
		break;
	case 0x01: // PAN
		dac_pan[(dac_ch << 1) + 0] = data & 0x0f;
		dac_pan[(dac_ch << 1) + 1] = (data & 0xf0) >> 4;
//		out_debug_log(_T("DAC REG 01 (PAN) CH=%d L=%01X R=%01X"),
//					  dac_ch, data & 0x0f, (data & 0xf0) >> 4);
		break;
	case 0x02: // FDL
		dac_fd[dac_ch].b.l = data & 0xff;
//		out_debug_log(_T("DAC REG 02 (FD LOW) CH=%d RAW=%02X"),
//					  dac_ch, data);
		break;
	case 0x03: // FDH
		dac_fd[dac_ch].b.h = data & 0xff;
//		out_debug_log(_T("DAC REG 03 (FD HIGH) CH=%d RAW=%02X"),
//					  dac_ch, data);
		break;
	case 0x04: // LSL
		dac_ls[dac_ch].b.l = data & 0xff;
//		out_debug_log(_T("DAC REG 04 (LS) CH=%d RAW=%02X"),
//					  dac_ch, data);
		break;
	case 0x05: // LSH
		dac_ls[dac_ch].b.h = data & 0xff;
//		out_debug_log(_T("DAC REG 05 (ADDR STEP HIGH) CH=%d RAW=%02X"),
//					  dac_ch, data);
		break;
	case 0x06: // ST
		dac_addr_st[dac_ch].d = 0;
		dac_addr_st[dac_ch].b.h = data & 0xff;
		dac_addr[dac_ch] = (uint32_t)(dac_addr_st[dac_ch].w.l) << 11;
//		out_debug_log(_T("DAC REG 06 (ADDR STEP HIGH) CH=%d RAW=%02X"),
//					  dac_ch, data);
		break;
	case 0x07: // Control
		{
			bool old_dac_on = dac_on;
			dac_on = ((data & 0x80) != 0) ? true : false;
			if((data & 0x40) != 0) { // CB2-0
				dac_ch = data & 0x07;
			} else { // WB3-0
				dac_bank = ((data & 0x0f) << 12);
			}
			if((dac_on != old_dac_on) && !(dac_on)) {
				sample_pointer = 0;
				sample_words = 0;
				read_pointer = 0;
				if((sample_buffer != NULL) && (sample_length > 0)) {
					memset(sample_buffer, 0x00, sizeof(int32_t) * 2 * sample_length);
				}
			}
		}
//		out_debug_log(_T("DAC REG 07 RAW=%02X ON=%s CH=%d BANK=%04X"),
//					   data,
//					   (dac_on) ? _T("ON ") :_T("OFF"),
//					   dac_ch, dac_bank);
		break;
	case 0x08: // ON/OFF per CH
		{
			uint32_t mask = 0x01;
			for(int i = 0; i < 8; i++) {
				bool onoff = dac_onoff[i];
				if((mask & data) == 0) {
					dac_onoff[i] = true;
				} else {
					dac_onoff[i] = false;
				}
//				if(!(onoff) && (dac_onoff[i])) { // Force reload
//					dac_addr[ch] = (uint32_t)(dac_addr_st[ch].w.l) << 11;
//				}
				mask <<= 1;
			}
		}
//		out_debug_log(_T("DAC REG 08 (DAC/ONOFF) RAW=%02X"),
//					   data);
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
uint32_t RF5C68::read_memory_mapped_io8(uint32_t addr)
{
	addr = (addr & 0xfff) | dac_bank;
	if(d_debugger != NULL && d_debugger->now_device_debugging) {
		return d_debugger->read_via_debugger_data8(addr);
	} else {
		if(dac_on) {
			return 0xff;
		}
		return read_via_debugger_data8(addr);	
	}
	return 0xff;
}

uint32_t RF5C68::read_memory_mapped_io16(uint32_t addr)
{
	addr = (addr & 0xfff) | dac_bank;
	if(d_debugger != NULL && d_debugger->now_device_debugging) {
		return d_debugger->read_via_debugger_data16(addr);
	} else {
		if(dac_on) {
			return 0xffff;
		}
		return read_via_debugger_data16(addr);	
	}
	return 0xffff;
}

void RF5C68::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	addr = (addr & 0xfff) | dac_bank;
	// if(dac_on) don't write <- Is correct?
	if(d_debugger != NULL && d_debugger->now_device_debugging) {
		d_debugger->write_via_debugger_data8(addr, data);
	} else {
//		if(!dac_on) {
			write_via_debugger_data8(addr, data);
//			return;
//		}
	}
}

void RF5C68::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	addr = (addr & 0xfff) | dac_bank;
	if(d_debugger != NULL && d_debugger->now_device_debugging) {
		d_debugger->write_via_debugger_data16(addr, data);
	} else {
//		if(!dac_on) {
			write_via_debugger_data16(addr, data);
//			return;
//		}
	}
}	

void RF5C68::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l - 4);
	volume_r = decibel_to_volume(decibel_r - 4);
}


void RF5C68::get_sample(int32_t *v, int words)
{
	if(words > sample_words) words = sample_words;
	if(words <= 0) return;
	if(v == NULL) return;
	switch(words) {
	case 1:
		v[2] = sample_buffer[(read_pointer << 1) + 0];
		v[3] = sample_buffer[(read_pointer << 1) + 1];
		break;
	default:
		{
			int nptr = read_pointer - words + 1;
			int nwords = words << 1;
			if(nptr < 0) nptr = read_pointer + sample_length - words;
			for(int i = 0; i < nwords; i += 2) {
				v[i + 0] = sample_buffer[(nptr << 1) + 0];
				v[i + 1] = sample_buffer[(nptr << 1) + 1];
				nptr = (nptr + 1) % sample_length;
			}
		}
		break;
	}		
}

void RF5C68::lpf_threetap(int32_t *v, int &lval, int &rval)
{
	if(v == NULL) return ;
	static const int fact2 = 1800;
	static const int fact0 = 450;
	static const int fact4 = 4096 - (fact2 + fact0);
	lval = (v[4] * fact4 + v[2] * fact2 + v[0] * fact0) >> 12;
	rval = (v[5] * fact4 + v[3] * fact2 + v[1] * fact0) >> 12;
}

void RF5C68::mix(int32_t* buffer, int cnt)
{
	
	int32_t lval, rval = 0;
	int32_t lval2, rval2 = 0;
	// ToDo: supress pop noise.
	if(cnt <= 0) return;
	if(is_mute) return;
	
	if((sample_buffer != NULL) && (sample_length > 0)) {
		__DECL_ALIGNED(16) int32_t val[16] = {0}; // 0,1 : before / 2,3 : after
		// ToDo: mix_freq <= dac_freq ; mix_factor >= 4096.
		if(mix_factor < 4096) {
			get_sample(val, 3);
			lpf_threetap(val, lval, rval);
			for(int i = 0; i < (cnt << 1); i += 2) {
				int32_t interp_p = mix_count;
				int32_t interp_n = 4096 - mix_count;
				lval2 = (interp_p * (val[4] - lval)) >> 12;
				rval2 = (interp_p * (val[5] - rval)) >> 12;
				lval2 = lval + lval2;
				rval2 = rval + rval2;
				lval2 = apply_volume(lval2, volume_l) >> 1;
				rval2 = apply_volume(rval2, volume_r) >> 1;
				// ToDo: interpoolate.
				buffer[i]     += lval2;
				buffer[i + 1] += rval2; 
				mix_count += mix_factor;
				if(mix_count >= 4096) {
//				out_debug_log(_T("MIX COUNT=%d FACTOR=%d"), mix_count, mix_factor);
					int n = mix_count >> 12;
					int old_rptr = read_pointer;
					read_pointer += n;
					if((old_rptr < sample_pointer) && (read_pointer >= sample_pointer)) {
						// Overshoot read opinter
						read_pointer = sample_pointer - 1;
						if(read_pointer < 0) read_pointer = sample_length - 1;
						if(read_pointer <= 0) read_pointer = 0;
					} else if((old_rptr >= sample_pointer)) {
						if(old_rptr < (sample_pointer + sample_length - sample_words)) {
							read_pointer = (sample_pointer + sample_length - sample_words);
						}
					}
					read_pointer = read_pointer % sample_length;		
					if(sample_words > 0) {
						// Reload data
						memset(val, 0x00, sizeof(val));
						get_sample(val, 3);
					} else {
						val[0] = val[2];
						val[1] = val[3];
						val[2] = 0;
						val[3] = 0;
					}
					lpf_threetap(val, lval, rval);
//				if(sample_words < 0) sample_words = 0;
					mix_count -= (n << 12);
				}
			}
		} else 	if(mix_factor == 4096) {
			// ToDo: Interpoolate
			get_sample(val, 4);
			lpf_threetap(val, lval, rval);
			for(int i = 0; i < (cnt << 1); i += 2) {
				lval2 = apply_volume(lval, volume_l) >> 1;
				rval2 = apply_volume(rval, volume_r) >> 1;
				buffer[i]     += lval2;
				buffer[i + 1] += rval2; 
				read_pointer = (read_pointer + 1) % sample_length;
				get_sample(val, 4);
				lpf_threetap(val, lval, rval);
			}
		} else { // MIX_FACTOR > 1.0
			// ToDo: Correct downsampling.
			get_sample(val, 8);
			lpf_threetap(val, lval, rval);
			for(int i = 0; i < (cnt << 1); i += 2) {
				lval2 = apply_volume(lval, volume_l) >> 1;
				rval2 = apply_volume(rval, volume_r) >> 1;
				buffer[i]     += lval2;
				buffer[i + 1] += rval2;
				
				int n = mix_count >> 12;
				read_pointer += n;
				read_pointer = read_pointer % sample_length;
				if(sample_words > 0) {
					// Reload data
					memset(val, 0x00, sizeof(val));
					get_sample(val, 8);
				} else {
					val[0] = val[2];
					val[1] = val[3];
					val[2] = 0;
					val[3] = 0;
				}
				mix_count -= (n << 12);
				lpf_threetap(val, lval, rval);
			}
		}
	}
}

void RF5C68::initialize_sound(int sample_rate, int samples)
{
	if((sample_rate > 0) && (samples > 0)) {
		mix_rate = sample_rate;
		sample_length = samples;
		mix_factor = (int)(dac_rate * 4096.0 / (double)mix_rate);
		mix_count = 0;
		read_pointer = 0;
		sample_pointer = 0;
		sample_words = 0;
		
		if(sample_buffer != NULL) free(sample_buffer);
		sample_buffer = (int32_t*)malloc(sample_length * sizeof(int32_t) * 2);
		if(sample_buffer != NULL) {
			memset(sample_buffer, 0x00, sample_length * sizeof(int32_t) * 2);
		}
		sample_tick_us = 1.0e6 / ((double)mix_rate);
	} else {
		if(sample_buffer != NULL) {
			free(sample_buffer);
		}
		sample_buffer = NULL;
		sample_length = 0;
		mix_rate = 0;
		sample_tick_us = 0.0;
	}
	sample_count = 0;
}

void RF5C68::write_debug_data8(uint32_t addr, uint32_t data)
{
	wave_memory[addr & 0xffff] = data;
}

uint32_t RF5C68::read_debug_data8(uint32_t addr)
{
	return wave_memory[addr & 0xffff];
}

void RF5C68::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
	wave_memory[addr] = data;
}

void RF5C68::write_via_debugger_data16(uint32_t addr, uint32_t data)
{
	pair32_t _b;
	_b.d = data;
	wave_memory[addr + 0] = _b.b.l;
	wave_memory[(addr + 1) & 0xffff] = _b.b.h;
}


uint32_t RF5C68::read_via_debugger_data8(uint32_t addr)
{
	return wave_memory[addr];
}

uint32_t RF5C68::read_via_debugger_data16(uint32_t addr)
{
	pair16_t _b;
	_b.b.l = wave_memory[addr + 0];
	_b.b.h = wave_memory[(addr + 1) & 0xffff];
	return _b.w;
}

bool RF5C68::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	_TCHAR sbuf[8][512] = {0};
	_TCHAR sbuf2[4096] = {0};
	for(int i = 0; i < 8; i++) {
		my_stprintf_s(sbuf[i], sizeof(sbuf[i]),
					  _T("CH%d: %s: ENV=%02X LPAN=%02X RPAN=%02X FD=%04X LS=%04X ADDR=%08X ADDR_ST=%08X\n")
					  , i, (dac_onoff[i]) ? _T("ON ") : _T("OFF")
					  , dac_env[i], dac_pan[(i << 1) + 0], dac_pan[(i << 1) + 1]
					  , dac_fd[i].w.l, dac_ls[i].w.l
					  , dac_addr[i], dac_addr_st[i].w.l
			);
	}
	for(int i = 0; i < 8; i++) {
		my_tcscat_s(sbuf2, sizeof(sbuf2), sbuf[i]);
	}
	my_stprintf_s(buffer, buffer_len,
				  _T("DAC %s BANK=%01X CH=%d MUTE=%s\n")
				  _T("%s")
				  , (dac_on) ? _T("ON ") : _T("OFF"), dac_bank, dac_ch, (is_mute) ? _T("ON ") : _T("OFF")
				  , sbuf2);
	return true;
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
	state_fio->StateValue(is_mute);
	state_fio->StateArray(dac_onoff, sizeof(dac_onoff), 1);
	state_fio->StateArray(dac_addr_st, sizeof(dac_addr_st), 1);	
	state_fio->StateArray(dac_addr, sizeof(dac_addr), 1);
	state_fio->StateArray(dac_env, sizeof(dac_env), 1);
	state_fio->StateArray(dac_pan, sizeof(dac_pan), 1);
	state_fio->StateArray(dac_ls, sizeof(dac_ls), 1);
	state_fio->StateArray(dac_fd, sizeof(dac_fd), 1);
	state_fio->StateArray(dac_force_load, sizeof(dac_force_load), 1);
	state_fio->StateArray(dac_tmpval, sizeof(dac_tmpval), 1);

	state_fio->StateValue(sample_words);
	state_fio->StateValue(sample_pointer);
	state_fio->StateValue(read_pointer);

	state_fio->StateValue(mix_factor);
	state_fio->StateValue(mix_count);
	state_fio->StateValue(dac_rate);
	
	state_fio->StateArray(wave_memory, sizeof(wave_memory), 1);

	// Post Process
	if(loading) {
		if(mix_rate > 0) {
			sample_tick_us = 1.0e6 / ((double)mix_rate);
		} else {
			sample_tick_us = 0;
		}
		sample_count = 0;
	}
	return true;
}
