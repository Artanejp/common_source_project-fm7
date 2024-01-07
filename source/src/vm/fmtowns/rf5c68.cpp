/*
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.29-

	[ADPCM RF6C68 with RAM]
*/

#include "../../common.h"
#include "./rf5c68.h"
#include "../debugger.h"
#include "../../types/util_sound.h"

#define EVENT_DAC_PERIOD	1
#define EVENT_LPF_PERIOD	2

void RF5C68::initialize()
{
	DEVICE::initialize();
	_RF5C68_DRIVEN_BY_EXTERNAL_CLOCK = osd->check_feature(_T("RF5C68_DRIVEN_BY_EXTERNAL_CLOCK"));

	memset(wave_memory, 0xff, sizeof(wave_memory));

	// DAC
	volume_l = volume_r = 1024;
	is_mute = true;
	mix_factor = 4096;
	event_dac = -1;
	event_lpf = -1;
	mix_count = 0;
	
	if(d_debugger != NULL) {
		d_debugger->set_device_name(_T("Debugger (RICOH RF5C68)"));
		d_debugger->set_context_mem(this);
		d_debugger->set_context_io(vm->dummy);
	}
}

void RF5C68::release()
{
}

void RF5C68::reset()
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	stop_dac_clock();
	touch_sound();
	
	if(event_lpf >= 0) cancel_event(this, event_lpf);
	event_lpf = -1;
	
	dac_on = false; // OK?
	is_mute = true; // OK?
	
	memset(wave_memory, 0xff, sizeof(wave_memory));
	for(int i = 0; i < 8; i++) {
		dac_addr_st[i].d = 0x00;
		dac_env[(i << 1) + 0] = 0x00000ff;
		dac_env[(i << 1) + 1] = 0x00000ff;
		dac_pan[(i << 1) + 0] = 0x0000000;
		dac_pan[(i << 1) + 1] = 0x0000000;
		dac_ls[i].d = 0x0000;
		dac_fd[i].d = 0x0000;
		dac_onoff[i] = false;
		dac_addr[i] = 0x00000000;
		dac_force_load[i] = true;
	}
	
	lastsample_l = 0;
	lastsample_r = 0;
	prevsample_l = 0;
	prevsample_r = 0;

	clear_buffer();
	set_mix_factor();

	start_dac_clock();
}

void RF5C68::start_dac_clock()
{
	if((event_dac < 0) && !(_RF5C68_DRIVEN_BY_EXTERNAL_CLOCK) && (dac_rate > 0.0)) {
		register_event(this, EVENT_DAC_PERIOD,
					   1.0e6 / dac_rate, true, &event_dac);
	}
}

void RF5C68::stop_dac_clock()
{
	if(event_dac >= 0) {
		cancel_event(this, event_dac);
	}
	event_dac = -1;
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
			return dac_env[(local_ch << 1) + 0];
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
			return ((is_mute.load()) ? 0xffffffff : 0x00000000);
			break;
		case SIG_RF5C68_REG_ON:
			return ((dac_on) ? 0xffffffff : 0x00000000);
			break;
		case SIG_RF5C68_REG_BANK:
			return (dac_bank >> 12) & 0x0f;
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

void RF5C68::do_dac_period()
{
	int32_t lr[2] = {lastsample_l, lastsample_r};
	if(dac_on) {
		lr[0] = 0;
		lr[1] = 0;
		__DECL_ALIGNED(16) uint8_t tmpval[8] = {0};
		__DECL_ALIGNED(16) int32_t val[16] = {0};
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
				int8_t rawval = tmpval[ch] & 0x7f;
				if((tmpval[ch] & 0x80) == 0) {
					rawval = -rawval;
				}
				val[chd + 0] = rawval;
				val[chd + 1] = rawval;
			}
		}
		// VAL = VAL * ENV * PAN
		__DECL_VECTORIZED_LOOP
		for(int chd = 0; chd < 16; chd++) {
			val[chd] *= dac_env[chd];
		}
		__DECL_VECTORIZED_LOOP
		for(int chd = 0; chd < 16; chd++) {
			val[chd] *= dac_pan[chd];
		}
		// Shrink 5bits (Technical data book figure I-5-10.)
		__DECL_VECTORIZED_LOOP
		for(int chd = 0; chd < 16; chd++) {
			val[chd] >>= 5;
		}
		// ToDo: Clamping
		__DECL_VECTORIZED_LOOP
		for(int chd = 0; chd < 16; chd++) {
			lr[chd & 1] += val[chd];
		}
		for(int i = 0; i < 2; i++) {
			lr[i] &= 0xffffffc0; // Discard lower 6bits.
			// Clamping
			__UNLIKELY_IF(lr[i] > 65535) {
				lr[i] = 65535;
			}
			__UNLIKELY_IF(lr[i] < -65535) {
				lr[i] = -65535;
			}
			lr[i] <<= 2; // Expand volume (Hacks for CSP)
		}
		//for(int i = 0; i < 2; i++) {
		//	lr[i] >>= 2;
		//}
		// Re-Init sample buffer
		//	touch_sound();
	}
	lastsample_l = lr[0];
	lastsample_r = lr[1];
	touch_sound();
}

void RF5C68::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch)
	{
	case SIG_RF5C68_DAC_PERIOD:
		if(_RF5C68_DRIVEN_BY_EXTERNAL_CLOCK) {
			do_dac_period();
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
			bool mute_bak = is_mute.load();
			bool mute_new = ((data & mask) != 0) ? true : false;
			//if(mute_bak != mute_new) {
			//	touch_sound();
			//}
			is_mute = mute_new;
		}
		break;
	case SIG_RF5C68_REG_BANK:
		dac_bank = (data & 0x0f) << 12;
		break;
	default:
		break;
	}
}



void RF5C68::event_callback(int id,  int err)
{
	switch(id) {
	case EVENT_DAC_PERIOD:
		if(!(_RF5C68_DRIVEN_BY_EXTERNAL_CLOCK)) {
			do_dac_period();
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
		dac_env[(dac_ch << 1) + 0] = data & 0xff;
		dac_env[(dac_ch << 1) + 1] = data & 0xff;
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
		dac_force_load[dac_ch] = false;
//		out_debug_log(_T("DAC REG 06 (ADDR STEP HIGH) CH=%d RAW=%02X"),
//					  dac_ch, data);
		break;
	case 0x07: // Control
		{
			//bool dac_on_bak = dac_on;
			dac_on = ((data & 0x80) != 0) ? true : false;
			//__UNLIKELY_IF(dac_on_bak != dac_on) {
			//	touch_sound();
			//}
			if((data & 0x40) != 0) { // CB2-0
				dac_ch = data & 0x07;
			} else { // WB3-0
				dac_bank = ((data & 0x0f) << 12);
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
				if((!(onoff) && (dac_onoff[i])) || (dac_force_load[i])) { // Force reload
					dac_addr[i] = (uint32_t)(dac_addr_st[i].w.l) << 11;
					dac_force_load[i] = false;
				}
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
uint32_t RF5C68::read_memory_mapped_io8w(uint32_t addr, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 6; // OK?
	}
	__UNLIKELY_IF(addr >= 0x1000) return 0xff; // This is workaround.
	addr = (addr & 0xfff) | dac_bank;
	if(d_debugger != NULL && d_debugger->now_device_debugging) {
		return d_debugger->read_via_debugger_data8(addr);
	} else {
//		if(dac_on) {
//			return 0xff;
//		}
		return wave_memory[addr];
	}
	return 0xff;
}


void RF5C68::write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 6; // OK?
	}
	__UNLIKELY_IF(addr >= 0x1000) return; // This is workaround.
	addr = (addr & 0xfff) | dac_bank;
	// if(dac_on) don't write <- Is correct?
	if(d_debugger != NULL && d_debugger->now_device_debugging) {
		d_debugger->write_via_debugger_data8(addr, data);
	} else {
//		if(dac_on) {
//			return;
//		}
		wave_memory[addr] = data;
	}
}

uint32_t RF5C68::read_dma_data8w(uint32_t addr, int* wait)
{
	__UNLIKELY_IF(addr >= 0x1000) return 0xff; // This is workaround.
	addr = (addr & 0xfff) | dac_bank;
	uint32_t val = wave_memory[addr];
	__LIKELY_IF(wait != NULL) { // Normally Ignore DMA wait.
		*wait = 0;
	}
	return val;
}


void RF5C68::write_dma_data8w(uint32_t addr, uint32_t data, int* wait)
{
	__UNLIKELY_IF(addr >= 0x1000) return; // This is workaround.
	addr = (addr & 0xfff) | dac_bank;
	wave_memory[addr] = data;
	__LIKELY_IF(wait != NULL) { // Normally Ignore DMA wait.
		*wait = 0;
	}
}

void RF5C68::set_volume(int ch, int decibel_l, int decibel_r)
{
	volume_l = decibel_to_volume(decibel_l);
	volume_r = decibel_to_volume(decibel_r);
	touch_sound();
}

void RF5C68::lpf_threetap(int32_t *v, int &lval, int &rval)
{
	__UNLIKELY_IF(v == NULL) return ;
	static const int fact2 = 1800;
	static const int fact0 = 450;
	static const int fact4 = 4096 - (fact2 + fact0);
	lval = (v[4] * fact4 + v[2] * fact2 + v[0] * fact0) >> 12;
	rval = (v[5] * fact4 + v[3] * fact2 + v[1] * fact0) >> 12;
}

void RF5C68::mix(int32_t* buffer, int cnt)
{
	// ToDo: Synchronize at start of frame. 20240106 K.O
	// ToDo: supress pop noise.
	__UNLIKELY_IF(buffer == NULL) return;
	__UNLIKELY_IF(cnt <= 0) return;
	__UNLIKELY_IF(!(is_initialized.load())) return;
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	
	int32_t* bp = buffer;
	{
		int64_t lval, rval;
		// ToDo: mix_freq <= dac_freq ; mix_factor >= 4096.
		int mix_factor_bak = mix_factor.load();
		for(int sptr = 0; sptr < cnt; sptr++) {
			//ToDo : Downsampling and interpolate.
			{
				lval = (int64_t)(lastsample_l.load());
				rval = (int64_t)(lastsample_r.load());
				int64_t diff_l = (int64_t)(prevsample_l.load()) - lval;
				int64_t diff_r = (int64_t)(prevsample_r.load()) - rval;
				int64_t mix_count_bak = mix_count;
				int __n = mix_count >> 12;
				__UNLIKELY_IF(__n > 0) {
					prevsample_l = (int32_t)lval;
					prevsample_r = (int32_t)rval;
					mix_count -= (__n << 12);
				}
				mix_count += mix_factor;
				__LIKELY_IF((is_initialized.load()) && !(is_mute.load())) {
					if(is_interpolate.load()) {
						diff_l = ((diff_l << 16) * mix_count_bak) / ((__n + 1) << 12);
						diff_r = ((diff_r << 16) * mix_count_bak) / ((__n + 1) << 12);
						lval += (diff_l >> 16);
						rval += (diff_r >> 16);
					}
					int32_t true_lval = apply_volume((int32_t)lval, volume_l.load());
					int32_t true_rval = apply_volume((int32_t)rval, volume_r.load());
					bp[0] += true_lval;
					bp[1] += true_rval;
					bp += 2;
				}
			}
		}
	}
}

void RF5C68::initialize_sound(int sample_rate, int samples)
{
	std::lock_guard<std::recursive_mutex> locker(m_locker);
	if((sample_rate > 0) && (samples > 0)) {
		mix_rate = sample_rate;
		int mix_factor_bak = mix_factor.load();
		set_mix_factor();
		if(!(is_initialized.load()) || (mix_factor_bak != mix_factor.load())) {
			clear_buffer();
		}
		is_initialized = true;
	}
}

void RF5C68::write_debug_data8(uint32_t addr, uint32_t data)
{
	wave_memory[addr & 0xffff] = data;
}

uint32_t RF5C68::read_debug_data8(uint32_t addr)
{
	return wave_memory[addr & 0xffff];
}


bool RF5C68::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	_TCHAR sbuf[8][512] = {0};
	_TCHAR sbuf2[4096] = {0};
	for(int i = 0; i < 8; i++) {
		my_stprintf_s(sbuf[i], sizeof(sbuf[i]),
					  _T("CH%d: %s: ENV=%02X LPAN=%02X RPAN=%02X FD=%04X LS=%04X ADDR=%08X ADDR_ST=%08X\n")
					  , i, (dac_onoff[i]) ? _T("ON ") : _T("OFF")
					  , dac_env[i << 1], dac_pan[(i << 1) + 0], dac_pan[(i << 1) + 1]
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
				  , (dac_on) ? _T("ON ") : _T("OFF"), (dac_bank >> 12) & 0x0f, dac_ch, (is_mute.load()) ? _T("ON ") : _T("OFF")
				  , sbuf2);
	return true;
}

#define STATE_VERSION	5

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
	bool is_mute_tmp = is_mute.load();
	state_fio->StateValue(is_mute_tmp);
	if(loading) {
		is_mute = is_mute_tmp;
	}
	
	state_fio->StateArray(dac_onoff, sizeof(dac_onoff), 1);
	state_fio->StateArray(dac_addr_st, sizeof(dac_addr_st), 1);
	state_fio->StateArray(dac_addr, sizeof(dac_addr), 1);

	uint32_t env_bak[8];
	for(int ii = 0; ii < 8; ii++) {
		env_bak[ii] = dac_env[ii << 1];
	}
	state_fio->StateArray(env_bak, sizeof(env_bak), 1);
	if(loading) {
		for(int ii = 0; ii < 8; ii++) {
			dac_env[(ii << 1) + 0] = env_bak[ii];
			dac_env[(ii << 1) + 1] = env_bak[ii];
		}
	}

	state_fio->StateArray(dac_pan, sizeof(dac_pan), 1);
	state_fio->StateArray(dac_ls, sizeof(dac_ls), 1);
	state_fio->StateArray(dac_fd, sizeof(dac_fd), 1);
	state_fio->StateArray(dac_force_load, sizeof(dac_force_load), 1);

	// These vars use std::atomic<int> , below are workaround.
	
	state_fio->StateValue(dac_rate);
	state_fio->StateValue(lpf_cutoff);

	int32_t lastsample_l_bak = lastsample_l.load();
	int32_t lastsample_r_bak = lastsample_r.load();
	int32_t prevsample_l_bak = prevsample_l.load();
	int32_t prevsample_r_bak = prevsample_r.load();

	state_fio->StateValue(lastsample_l_bak);
	state_fio->StateValue(lastsample_r_bak);
	state_fio->StateValue(prevsample_l_bak);
	state_fio->StateValue(prevsample_r_bak);
	if(loading) {
		lastsample_l = lastsample_l_bak;
		lastsample_r = lastsample_r_bak;
		prevsample_l = prevsample_l_bak;
		prevsample_r = prevsample_r_bak;
	}
	state_fio->StateArray(wave_memory, sizeof(wave_memory), 1);

	// Post Process
	return true;
}
