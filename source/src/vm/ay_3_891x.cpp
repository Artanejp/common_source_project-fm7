/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.15-

	[ AY-3-8910 / AY-3-8912 / AY-3-8913 ]
*/

#include "ay_3_891x.h"
#include "debugger.h"

#define EVENT_FM_TIMER	0

void AY_3_891X::initialize()
{
	DEVICE::initialize();
	opn = new FM::OPN;
	opn->is_ay3_891x = true;
	
	register_vline_event(this);
	mute = false;
	clock_prev = clock_accum = clock_busy = 0;

	use_lpf = false;
	use_hpf = false;
	lpf_freq = 1;
	hpf_freq = 1;
	lpf_quality = 1.0;
	hpf_quality = 1.0;
	
	_HAS_AY_3_8910 = osd->check_feature(_T("HAS_AY_3_8910"));
	_HAS_AY_3_8912 = osd->check_feature(_T("HAS_AY_3_8912"));
	_HAS_AY_3_8913 = osd->check_feature(_T("HAS_AY_3_8913"));
	_SUPPORT_AY_3_891X_PORT_A = false;
	_SUPPORT_AY_3_891X_PORT_B = false;
	_IS_AY_3_891X_PORT_MODE = osd->check_feature(_T("AY_3_891X_PORT_MODE"));
	_AY_3_891X_PORT_MODE = osd->get_feature_uint32_value(_T("AY_3_891X_PORT_MODE"));

	if(_HAS_AY_3_8912) {
		// AY-3-8912: port b is not supported
		_SUPPORT_AY_3_891X_PORT_A = true;
	} else if(!(_HAS_AY_3_8913)) {
		// AY-3-8913: both port a and port b are not supported
		// AY-3-8910: both port a and port b are supported
		_SUPPORT_AY_3_891X_PORT_A = true;
		_SUPPORT_AY_3_891X_PORT_B = true;
	}		
	_SUPPORT_AY_3_891X_PORT = ((_SUPPORT_AY_3_891X_PORT_A) || (_SUPPORT_AY_3_891X_PORT_B));
	if(_HAS_AY_3_8910) {
		set_device_name(_T("AY-3-8910 PSG"));
	} else if(_HAS_AY_3_8912) {
		set_device_name(_T("AY-3-8912 PSG"));
	} else if(_HAS_AY_3_8913) {
		set_device_name(_T("AY-3-8913 PSG"));
	}


	if(d_debugger != NULL) {
		d_debugger->set_device_name(_T("Debugger (AY-3-891X PSG)"));
		d_debugger->set_context_mem(this);
		d_debugger->set_context_io(vm->dummy);
	}
}

void AY_3_891X::release()
{
	delete opn;
	DEVICE::release();
}

void AY_3_891X::reset()
{
	touch_sound();
	opn->Reset();
	
	// stop timer
	timer_event_id = -1;
	this->set_reg(0x27, 0);
	
	if(_SUPPORT_AY_3_891X_PORT) {
		port[0].first = port[1].first = true;
		port[0].wreg = port[1].wreg = 0;//0xff;
		if(_IS_AY_3_891X_PORT_MODE) {
			mode = _AY_3_891X_PORT_MODE;
		} else {
			mode = 0;
		}
	}
	irq_prev = busy = false;
}

void AY_3_891X::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 1) {
	case 0:
		ch = data & 0x0f;
		break;
	case 1:
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			d_debugger->write_via_debugger_data8(ch, data);
		} else
		this->write_via_debugger_data8(ch, data);
		break;
	}
}

uint32_t AY_3_891X::read_io8(uint32_t addr)
{
	switch(addr & 1) {
	case 1:
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			return d_debugger->read_via_debugger_data8(ch);
		} else
		return this->read_via_debugger_data8(ch);
	}
	return 0xff;
}

void AY_3_891X::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
	if(addr < 16) {
		if(_SUPPORT_AY_3_891X_PORT) {
			if(addr == 7) {
				if(_IS_AY_3_891X_PORT_MODE) {
					mode = (data & 0x3f) | _AY_3_891X_PORT_MODE;
				} else {
					mode = data;
				}
			} else if(addr == 14) {
				if(_SUPPORT_AY_3_891X_PORT_A) {
					if(port[0].wreg != data || port[0].first) {
						write_signals(&port[0].outputs, data);
						port[0].wreg = data;
						port[0].first = false;
					}
				}
			} else if(addr == 15) {
				if(_SUPPORT_AY_3_891X_PORT_B) {
					if(port[1].wreg != data || port[1].first) {
						write_signals(&port[1].outputs, data);
						port[1].wreg = data;
						port[1].first = false;
					}
				}
			}
		}
		update_count();
		this->set_reg(addr, data);
	}
}

uint32_t AY_3_891X::read_via_debugger_data8(uint32_t addr)
{
	if(addr < 16) {
		if(_SUPPORT_AY_3_891X_PORT) {
			if(addr == 14) {
				if(_SUPPORT_AY_3_891X_PORT_A) {
					return (mode & 0x40) ? port[0].wreg : port[0].rreg;
				}
			} else if(addr == 15) {
				if(_SUPPORT_AY_3_891X_PORT_B) {
					return (mode & 0x80) ? port[1].wreg : port[1].rreg;
				}
			}
		}
		return opn->GetReg(addr);
	}
	return 0;
}

void AY_3_891X::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_AY_3_891X_MUTE) {
		mute = ((data & mask) != 0);

	} else if(id == SIG_AY_3_891X_PORT_A) {
		if(_SUPPORT_AY_3_891X_PORT_A) {
			port[0].rreg = (port[0].rreg & ~mask) | (data & mask);
		}

	} else if(id == SIG_AY_3_891X_PORT_B) {
		if(_SUPPORT_AY_3_891X_PORT_B) {
			port[1].rreg = (port[1].rreg & ~mask) | (data & mask);
		}
	}
}

void AY_3_891X::event_vline(int v, int clock)
{
	update_count();
}

void AY_3_891X::event_callback(int event_id, int error)
{
	update_count();
	timer_event_id = -1;
	update_event();
}

void AY_3_891X::update_count()
{
	clock_accum += clock_const * get_passed_clock(clock_prev);
	uint32_t count = clock_accum >> 20;
	if(count) {
		opn->Count(count);
		clock_accum -= count << 20;
	}
	clock_prev = get_current_clock();
}

void AY_3_891X::update_event()
{
	if(timer_event_id != -1) {
		cancel_event(this, timer_event_id);
		timer_event_id = -1;
	}
	
	int count;
	count = opn->GetNextEvent();
	
	if(count > 0) {
		register_event(this, EVENT_FM_TIMER, 1000000.0 / (double)chip_clock * (double)count, false, &timer_event_id);
	}
}

void AY_3_891X::mix(int32_t* buffer, int cnt)
{
	if(cnt > 0 && !mute) {
		if((use_lpf) || (use_hpf)) {
			int32_t* p;
			int32_t p_h[cnt * 2];
			int32_t p_l[cnt * 2] = {0};
			p = p_l;
			opn->Mix(p, cnt);
			if(use_lpf) {
				if(use_hpf) {
					calc_low_pass_filter(p_h, p, sample_rate, lpf_freq, cnt, lpf_quality, false);
					p = p_h;
				} else {
					calc_low_pass_filter(buffer, p, sample_rate, lpf_freq, cnt, lpf_quality, true);
				}					
			}
			if(use_hpf) {
				calc_high_pass_filter(buffer, p, sample_rate, hpf_freq, cnt, hpf_quality, true);
			}
		} else {
			opn->Mix(buffer, cnt);
		}
	}
}

void AY_3_891X::set_volume(int ch, int decibel_l, int decibel_r)
{
	if(ch == 0) {
		opn->SetVolumeFM(base_decibel_fm + decibel_l, base_decibel_fm + decibel_r);
	} else if(ch == 1) {
		opn->SetVolumePSG(base_decibel_psg + decibel_l, base_decibel_psg + decibel_r);
	}
}

void AY_3_891X::set_high_pass_filter_freq(int freq, double quality)
{
	if((freq < 0) || (freq >= (sample_rate / 2))) {
		hpf_freq = 1;
		use_hpf = false;
		hpf_quality = 1.0;
	} else {
		hpf_freq = freq;
		use_hpf = true;
		hpf_quality = quality;
	}
}

void AY_3_891X::set_low_pass_filter_freq(int freq, double quality)
{
	if((freq < 0) || (freq >= (sample_rate / 2))) {
		lpf_freq = 1;
		use_lpf = false;
		lpf_quality = 1.0;
	} else {
		lpf_freq = freq;
		use_lpf = true;
		lpf_quality = quality;
	}
}

void AY_3_891X::initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg)
{
	sample_rate = rate;
	opn->Init(clock, rate, false, NULL);
	opn->SetVolumeFM(decibel_fm, decibel_fm);
	opn->SetVolumePSG(decibel_psg, decibel_psg);
	
	base_decibel_fm = decibel_fm;
	base_decibel_psg = decibel_psg;
	
	chip_clock = clock;
}

void AY_3_891X::set_reg(uint32_t addr, uint32_t data)
{
	touch_sound();
	opn->SetReg(addr, data);
}

void AY_3_891X::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	clock_const = (uint32_t)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks + 0.5);
}

bool AY_3_891X::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if((reg[0] == 'R') || (reg[0] == 'r')) {
		if(strlen(reg) >= 2) {
			_TCHAR *eptr;
			int regnum = _tcstol(&(reg[1]), &eptr, 16);
			if(regnum >= 16) {
				return false;
			}
			opn->SetReg((uint32_t)regnum, data);
			return true;
		}
		return false;
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		ch = data;
		return true;
	}
	if(_SUPPORT_AY_3_891X_PORT) {
		if(_tcsicmp(reg, _T("PAR")) == 0) {
			port[0].rreg = data;
		} else if(_tcsicmp(reg, _T("PAW")) == 0) {
			port[0].wreg = data;
			write_signals(&port[0].outputs, data);
		} else if(_tcsicmp(reg, _T("PBR")) == 0) {
			port[1].rreg = data;
		} else if(_tcsicmp(reg, _T("PBW")) == 0) {
			port[1].wreg = data;
			write_signals(&port[1].outputs, data);
		} else if(_tcsicmp(reg, _T("MODE")) == 0) {
			mode = data;
		} else {
			return false;
		}
		return true;
	}
	return false;
}

bool AY_3_891X::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	_TCHAR tmps[512] = {0};
	if(_SUPPORT_AY_3_891X_PORT) {
		my_stprintf_s(tmps, 512, _T("PORTA: PAR/PAW = %02X/%02X  PORTB: PBR/PBW = %02X/%02X MODE=%02X\n")
					  , port[0].rreg, port[0].wreg, port[1].rreg, port[1].wreg, mode
			);
	}
	_TCHAR tmps2[16 * 12 + 16] = {0};
	_TCHAR tmps3[16];
	memset(tmps3, 0x00, sizeof(tmps3));
	my_stprintf_s(tmps3, 15, _T("+%02d :"), 0);
	_tcsncat(tmps2, tmps3, sizeof(tmps2) - 1);
	for(uint32_t j = 0; j < 16; j++) {
		memset(tmps3, 0x00, sizeof(tmps3));
		my_stprintf_s(tmps3, 7, _T(" %02X"), opn->GetReg(j));
		_tcsncat(tmps2, tmps3, sizeof(tmps2) - 1);
	}
	_tcsncat(tmps2, "\n", sizeof(tmps2) - 1);
	my_stprintf_s(buffer, buffer_len - 1, _T("%sCH=%02X  CHIP_CLOCK=%d\nREG : +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n%s"),
				  tmps, ch, chip_clock, tmps2);
	return true;
}

#define STATE_VERSION	5

bool AY_3_891X::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	if(!opn->ProcessState((void *)state_fio, loading)) {
 		return false;
 	}
	state_fio->StateValue(ch);
	if(_SUPPORT_AY_3_891X_PORT) {
		for(int i = 0; i < 2; i++) {
			state_fio->StateValue(port[i].wreg);
			state_fio->StateValue(port[i].rreg);
			state_fio->StateValue(port[i].first);
		}
	}
	state_fio->StateValue(mode);
	state_fio->StateValue(chip_clock);
	state_fio->StateValue(irq_prev);
	state_fio->StateValue(mute);
	state_fio->StateValue(clock_prev);
	state_fio->StateValue(clock_accum);
	state_fio->StateValue(clock_const);
	state_fio->StateValue(clock_busy);
	state_fio->StateValue(timer_event_id);
	state_fio->StateValue(busy);

	state_fio->StateValue(use_lpf);
	state_fio->StateValue(use_hpf);
	state_fio->StateValue(lpf_freq);
	state_fio->StateValue(hpf_freq);
	state_fio->StateValue(lpf_quality);
	state_fio->StateValue(hpf_quality);
	
 	return true;
}
