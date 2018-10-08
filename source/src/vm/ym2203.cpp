/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.15-

	[ YM2203 / YM2608 ]
*/

#include "ym2203.h"
#include <math.h>

#define EVENT_FM_TIMER	0

#ifdef SUPPORT_MAME_FM_DLL
// thanks PC8801MA‰ü
#include "fmdll/fmdll.h"
static CFMDLL* fmdll = NULL;
static int chip_reference_counter = 0;
static bool dont_create_multiple_chips = false;
#endif

void YM2203::initialize()
{
	DEVICE::initialize();
	_HAS_YM2608 = osd->check_feature(_T("HAS_YM2608"));
	_HAS_AY_3_8910 = osd->check_feature(_T("HAS_AY_3_8910"));
	_HAS_AY_3_8912 = osd->check_feature(_T("HAS_AY_3_8912"));
	_HAS_AY_3_8913 = osd->check_feature(_T("HAS_AY_3_8913"));
	if(!_HAS_AY_3_8913) {
		if(_HAS_AY_3_8912) {
			_SUPPORT_YM2203_PORT_A = true;
		} else {
			_SUPPORT_YM2203_PORT_A = true;
			_SUPPORT_YM2203_PORT_B = true;
		}			
	}
	if(!(_HAS_AY_3_8910 || _HAS_AY_3_8912 || _HAS_AY_3_8913)) _HAS_YM_SERIES = true;
	if(_SUPPORT_YM2203_PORT_A || _SUPPORT_YM2203_PORT_B) _SUPPORT_YM2203_PORT = true;
	_IS_YM2203_PORT_MODE = osd->check_feature(_T("YM2203_PORT_MODE"));
	if(_IS_YM2203_PORT_MODE) {
		_YM2203_PORT_MODE = osd->get_feature_uint8_value(_T("YM2203_PORT_MODE"));
	}
//#ifdef SUPPORT_YM2203_PORT
//	if(_SUPPORT_YM2203_PORT) {
//		for(int i = 0; i < 2; i++) {
//			initialize_output_signals(&port[i].outputs);
//			port[i].wreg = port[i].rreg = 0;//0xff;
//		}
//	}
//#endif
//#ifdef HAS_YM_SERIES
//	if(_HAS_YM_SERIES) {
//		initialize_output_signals(&outputs_irq);
//	}
//#endif
//#ifdef HAS_YM2608
//		is_ym2608 = true;
//#endif
	_amask = ((is_ym2608) && (_HAS_YM2608)) ? 3 : 1;
//#ifdef HAS_YM2608
	if((_HAS_YM2608) && (is_ym2608)) {
		opna = new FM::OPNA;
	} else {
//#endif
		opn = new FM::OPN;
	}
//#ifdef HAS_YM2608
	if((_HAS_YM2608) && (is_ym2608)) {
		set_device_name(_T("YM2608 OPNA"));
	} else {
//#else		
		set_device_name(_T("YM2203 OPN"));
	}
//#endif		

#ifdef SUPPORT_MAME_FM_DLL

	if(!fmdll) {
//		fmdll = new CFMDLL(_T("mamefm.dll"));
		fmdll = new CFMDLL(config.fmgen_dll_path);
	}
	dllchip = NULL;
#endif
	register_vline_event(this);
	mute = false;
	clock_prev = clock_accum = clock_busy = 0;

	left_volume = right_volume = 256;
	v_left_volume = v_right_volume = 256;
}

void YM2203::release()
{
//#ifdef HAS_YM2608
	if(is_ym2608 && _HAS_YM2608) {
		delete opna;
	} else
//#endif
	delete opn;
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->Release(dllchip);
		dllchip = NULL;
		chip_reference_counter--;
	}
	if(fmdll && !chip_reference_counter) {
		delete fmdll;
		fmdll = NULL;
	}
#endif
}

void YM2203::reset()
{
	touch_sound();
//#ifdef HAS_YM2608
	if(is_ym2608 && _HAS_YM2608) {
		opna->Reset();
	} else
//#endif
	opn->Reset();
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->Reset(dllchip);
	}
	memset(port_log, 0, sizeof(port_log));
#endif
	fnum2 = 0;
//#ifdef HAS_YM2608
	fnum21 = 0;
//#endif
	
	// stop timer
	timer_event_id = -1;
	this->set_reg(0x27, 0);
	
//#ifdef SUPPORT_YM2203_PORT
	if(_SUPPORT_YM2203_PORT) {
		port[0].first = port[1].first = true;
		port[0].wreg = port[1].wreg = 0;//0xff;
//#ifdef YM2203_PORT_MODE
		if(_IS_YM2203_PORT_MODE) {
			mode = _YM2203_PORT_MODE;
		} else {
//#else
			mode = 0;
		}
	}
//#endif
//#endif
	irq_prev = busy = false;
}

//#ifdef HAS_YM2608
//#define amask (is_ym2608 ? 3 : 1)
//#else
//#define amask 1
//#endif

void YM2203::write_io8(uint32_t addr, uint32_t data)
{
	
	_amask = (is_ym2608) ? 3 : 1;
	switch(addr & _amask) {
	case 0:
//#ifdef HAS_YM_SERIES
		if(_HAS_YM_SERIES) {
			ch = data;
			// write dummy data for prescaler
			if(0x2d <= ch && ch <= 0x2f) {
				update_count();
				this->set_reg(ch, 0);
				update_interrupt();
				clock_busy = get_current_clock();
				busy = true;
			}
		} else {
//#else
			ch = data & 0x0f;
		}
//#endif
		break;
	case 1:
//#ifdef SUPPORT_YM2203_PORT
		if(_SUPPORT_YM2203_PORT) {
			if(ch == 7) {
//#ifdef YM2203_PORT_MODE
				if(_IS_YM2203_PORT_MODE) {
					mode = (data & 0x3f) | _YM2203_PORT_MODE;
				} else {
//#else
					mode = data;
				}
//#endif
			} else if(ch == 14) {
//#ifdef SUPPORT_YM2203_PORT_A
				if(_SUPPORT_YM2203_PORT_A) {
					if(port[0].wreg != data || port[0].first) {
						write_signals(&port[0].outputs, data);
						port[0].wreg = data;
						port[0].first = false;
					}
				}
//#endif
			} else if(ch == 15) {
//#ifdef SUPPORT_YM2203_PORT_B
				if(_SUPPORT_YM2203_PORT_B) {
					if(port[1].wreg != data || port[1].first) {
						write_signals(&port[1].outputs, data);
						port[1].wreg = data;
						port[1].first = false;
					}
				}
			}
//#endif
		}
//#endif
		if(0x2d <= ch && ch <= 0x2f) {
			// don't write again for prescaler
		} else if(0xa4 <= ch && ch <= 0xa6) {
			// XM8 version 1.20
			fnum2 = data;
		} else {
			update_count();
			// XM8 version 1.20
			if(0xa0 <= ch && ch <= 0xa2) {
				this->set_reg(ch + 4, fnum2);
			}
			this->set_reg(ch, data);
			if(ch == 0x27) {
				update_event();
			}
//#ifdef HAS_YM_SERIES
			if(_HAS_YM_SERIES) {
				update_interrupt();
				clock_busy = get_current_clock();
				busy = true;
			}
//#endif
		}
		break;
//#ifdef HAS_YM2608
	case 2:
		if(_HAS_YM2608) {
			ch1 = data1 = data;
		}
		break;
	case 3:
		if(_HAS_YM2608) {
			if(0xa4 <= ch1 && ch1 <= 0xa6) {
				// XM8 version 1.20
				fnum21 = data;
			} else {
				update_count();
				// XM8 version 1.20
				if(0xa0 <= ch1 && ch1 <= 0xa2) {
					this->set_reg(0x100 | (ch1 + 4), fnum21);
				}
				this->set_reg(0x100 | ch1, data);
				data1 = data;
				update_interrupt();
				clock_busy = get_current_clock();
				busy = true;
			}
		}
		break;
	default:
		break;
//#endif
	}
}

uint32_t YM2203::read_io8(uint32_t addr)
{
	if(_HAS_YM_SERIES) {
		_amask = (is_ym2608) ? 3 : 1;
		switch(addr & _amask) {
//#ifdef HAS_YM_SERIES
		case 0:
			{
				/* BUSY : x : x : x : x : x : FLAGB : FLAGA */
				update_count();
				update_interrupt();
				uint32_t status;
//#ifdef HAS_YM2608
				if(is_ym2608 && _HAS_YM2608) {
					status = opna->ReadStatus() & ~0x80;
				} else {
//#endif
					status = opn->ReadStatus() & ~0x80;
				}
				if(busy) {
					// from PC-88 machine language master bible (XM8 version 1.00)
//#ifdef HAS_YM2608
					if(_HAS_YM2608) {
						if (get_passed_usec(clock_busy) < (is_ym2608 ? 4.25 : 2.13)) {
							status |= 0x80;
						} else {
							busy = false;
						}
					} else {
//#else
						if (get_passed_usec(clock_busy) < 2.13) {
							status |= 0x80;
						} else {
							busy = false;
						}
					}
//#endif
				}
				return status;
			}
//#endif
			break;
		case 1:
//#ifdef SUPPORT_YM2203_PORT
			if(_SUPPORT_YM2203_PORT) {
				if(ch == 14) {
//#ifdef SUPPORT_YM2203_PORT_A
					if(_SUPPORT_YM2203_PORT_A) {
						return (mode & 0x40) ? port[0].wreg : port[0].rreg;
					}
//#endif
				} else if(ch == 15) {
//#ifdef SUPPORT_YM2203_PORT_B
					if(_SUPPORT_YM2203_PORT_B) {
						return (mode & 0x80) ? port[1].wreg : port[1].rreg;
					}
//#endif
				}
			}
//#endif
//#ifdef HAS_YM2608
			if(is_ym2608 && _HAS_YM2608) {
				return opna->GetReg(ch);
			} else {
//#endif
				return opn->GetReg(ch);
			}
			break;
//#ifdef HAS_YM2608
		case 2:
			if(_HAS_YM2608) {
				/* BUSY : x : PCMBUSY : ZERO : BRDY : EOS : FLAGB : FLAGA */
				update_count();
				update_interrupt();
				uint32_t status = opna->ReadStatusEx() & ~0x80;
				if(busy) {
					// FIXME: we need to investigate the correct busy period
					if(get_passed_usec(clock_busy) < 8) {
						status |= 0x80;
					} else {
						busy = false;
					}
				}
				return status;
			}
			break;
		case 3:
			if(_HAS_YM2608) {
				if(ch1 == 8) {
					return opna->GetReg(0x100 | ch1);
//		} else if(ch1 == 0x0f) {
//			return 0x80; // from mame fm.c
				}
				return data1;
			}
			break;
//#endif
		default:
			break;
		}
	}
	return 0xff;
}

void YM2203::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_YM2203_MUTE) {
		mute = ((data & mask) != 0);
//#ifdef SUPPORT_YM2203_PORT_A
	} else if((id == SIG_YM2203_PORT_A) && _SUPPORT_YM2203_PORT_A) {
		port[0].rreg = (port[0].rreg & ~mask) | (data & mask);
//#endif
//#ifdef SUPPORT_YM2203_PORT_B
	} else if((id == SIG_YM2203_PORT_B) && _SUPPORT_YM2203_PORT_B) {
		port[1].rreg = (port[1].rreg & ~mask) | (data & mask);
//#endif
	}
}

uint32_t YM2203::read_signal(int id)
{
	if((id == SIG_YM2203_PORT_A) && _SUPPORT_YM2203_PORT_A) {
		return (uint32_t)port[0].rreg;
//#endif
//#ifdef SUPPORT_YM2203_PORT_B
	} else if((id == SIG_YM2203_PORT_B) && _SUPPORT_YM2203_PORT_B) {
		return (uint32_t)port[1].rreg;
//#endif
	}
	return 0x00;
}	

void YM2203::event_vline(int v, int clock)
{
	update_count();
//#ifdef HAS_YM_SERIES
	if(_HAS_YM_SERIES) {
		update_interrupt();
	}
//#endif
}

void YM2203::event_callback(int event_id, int error)
{
	update_count();
//#ifdef HAS_YM_SERIES
	if(_HAS_YM_SERIES) {
		update_interrupt();
	}
//#endif
	timer_event_id = -1;
	update_event();
}

void YM2203::update_count()
{
	clock_accum += clock_const * get_passed_clock(clock_prev);
	uint32_t count = clock_accum >> 20;
	if(count) {
//#ifdef HAS_YM2608
		if(is_ym2608 && _HAS_YM2608) {
			opna->Count(count);
		} else {
//#endif
			opn->Count(count);
		}
		clock_accum -= count << 20;
	}
	clock_prev = get_current_clock();
}

void YM2203::update_event()
{
	if(timer_event_id != -1) {
		cancel_event(this, timer_event_id);
		timer_event_id = -1;
	}
	
	int count;
//#ifdef HAS_YM2608
	if(is_ym2608 && _HAS_YM2608) {
		count = opna->GetNextEvent();
	} else {
//#endif
		count = opn->GetNextEvent();
	}
	if(count > 0) {
//#ifdef HAS_YM2608
		if(is_ym2608 && _HAS_YM2608) {
			register_event(this, EVENT_FM_TIMER, 1000000.0 / (double)chip_clock * (double)count * 2.0, false, &timer_event_id);
		} else {
//#endif
			register_event(this, EVENT_FM_TIMER, 1000000.0 / (double)chip_clock * (double)count, false, &timer_event_id);
		}
	}
}

//#ifdef HAS_YM_SERIES
void YM2203::update_interrupt()
{
	if(_HAS_YM_SERIES) {
		bool irq;
//#ifdef HAS_YM2608
		if(is_ym2608 && _HAS_YM2608) {
			irq = opna->ReadIRQ();
		} else {
//#endif
			irq = opn->ReadIRQ();
		}
		if(!irq_prev && irq) {
			write_signals(&outputs_irq, 0xffffffff);
		} else if(irq_prev && !irq) {
			write_signals(&outputs_irq, 0);
		}
		irq_prev = irq;
	}
}
//#endif

inline int32_t VCALC(int32_t x, int32_t y)
{
	x = x * y;
	x = x >> 8;
	return x;
}

inline int32_t SATURATION_ADD(int32_t x, int32_t y)
{
	x = x + y;
	if(x < -0x8000) x = -0x8000;
	if(x >  0x7fff) x =  0x7fff;
	return x;
}


void YM2203::mix(int32_t* buffer, int cnt)
{
	if(cnt > 0 && !mute) {
		int32_t *dbuffer = (int32_t *)malloc((cnt * 2 + 2) * sizeof(int32_t));
		memset((void *)dbuffer, 0x00, (cnt * 2 + 2) * sizeof(int32_t));
	   
//#ifdef HAS_YM2608
		if(is_ym2608 && _HAS_YM2608) {
			opna->Mix(dbuffer, cnt);
		} else {
//#endif
			opn->Mix(dbuffer, cnt);
		}
#ifdef SUPPORT_MAME_FM_DLL
		if(dllchip) {
			fmdll->Mix(dllchip, dbuffer, cnt);
		}
#endif
		int32_t *p = dbuffer;
		int32_t *q = buffer;
		int32_t tmp[8];
		int32_t tvol[8] = {v_left_volume, v_right_volume,
				 v_left_volume, v_right_volume,
				 v_left_volume, v_right_volume,
				 v_left_volume, v_right_volume};
		int i;
		// More EXCEPTS to optimize to SIMD features.
		for(i = 0; i < cnt / 4; i++) {
			tmp[0] = VCALC(p[0], tvol[0]);
			tmp[1] = VCALC(p[1], tvol[1]);
			tmp[2] = VCALC(p[2], tvol[2]);
			tmp[3] = VCALC(p[3], tvol[3]);
			tmp[4] = VCALC(p[4], tvol[4]);
			tmp[5] = VCALC(p[5], tvol[5]);
			tmp[6] = VCALC(p[6], tvol[6]);
			tmp[7] = VCALC(p[7], tvol[7]);

			q[0] = SATURATION_ADD(q[0], tmp[0]);
			q[1] = SATURATION_ADD(q[1], tmp[1]);
			q[2] = SATURATION_ADD(q[2], tmp[2]);
			q[3] = SATURATION_ADD(q[3], tmp[3]);
		   
			q[4] = SATURATION_ADD(q[4], tmp[4]);
			q[5] = SATURATION_ADD(q[5], tmp[5]);
			q[6] = SATURATION_ADD(q[6], tmp[6]);
			q[7] = SATURATION_ADD(q[7], tmp[7]);
			q += 8;
			p += 8;
		}
		if((cnt & 3) != 0) {
			for(i = 0; i < (cnt & 3); i++) {
				tmp[0] = VCALC(p[0], tvol[0]);
				tmp[1] = VCALC(p[1], tvol[1]);
			   
				q[0] = SATURATION_ADD(q[0], tmp[0]);
				q[1] = SATURATION_ADD(q[1], tmp[1]);
				q += 2;
				p += 2;
			}
		}
		free(dbuffer);
	}
}

void YM2203::set_volume(int _ch, int decibel_l, int decibel_r)
{
	v_right_volume = (int)(pow(10.0, (double)decibel_vol / 10.0) * (double)right_volume);
	v_left_volume = (int)(pow(10.0, (double)decibel_vol / 10.0) * (double)left_volume);
	if(_ch == 0) {
//#ifdef HAS_YM2608
		if(is_ym2608 && _HAS_YM2608) {
			opna->SetVolumeFM(base_decibel_fm + decibel_l, base_decibel_fm + decibel_r);
		} else {
//#endif
			opn->SetVolumeFM(base_decibel_fm + decibel_l, base_decibel_fm + decibel_r);
		}
#ifdef SUPPORT_MAME_FM_DLL
		if(dllchip) {
			fmdll->SetVolumeFM(dllchip, base_decibel_fm + decibel_l);
		}
#endif
	} else if(_ch == 1) {
//#ifdef HAS_YM2608
		if(is_ym2608 && _HAS_YM2608) {
			opna->SetVolumePSG(base_decibel_psg + decibel_l, base_decibel_psg + decibel_r);
		} else {
//#endif
			opn->SetVolumePSG(base_decibel_psg + decibel_l, base_decibel_psg + decibel_r);
		}
#ifdef SUPPORT_MAME_FM_DLL
		if(dllchip) {
			fmdll->SetVolumePSG(dllchip, base_decibel_psg + decibel_l);
		}
#endif
//#ifdef HAS_YM2608
	} else if(_ch == 2) {
		if(is_ym2608 && _HAS_YM2608) {
			opna->SetVolumeADPCM(decibel_l, decibel_r);
		}
	} else if(_ch == 3) {
		if(is_ym2608 && _HAS_YM2608) {
			opna->SetVolumeRhythmTotal(decibel_l, decibel_r);
		}
//#endif
	}
}

void YM2203::initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg)
{
//#ifdef HAS_YM2608
	if(is_ym2608 && _HAS_YM2608) {
		opna->Init(clock, rate, false, get_application_path());
		opna->SetVolumeFM(decibel_fm, decibel_fm);
		opna->SetVolumePSG(decibel_psg, decibel_psg);
	} else {
//#endif
		opn->Init(clock, rate, false, NULL);
		opn->SetVolumeFM(decibel_fm, decibel_fm);
		opn->SetVolumePSG(decibel_psg, decibel_psg);
//#ifdef HAS_YM2608
	}
//#endif
	base_decibel_fm = decibel_fm;
	base_decibel_psg = decibel_psg;
	
#ifdef SUPPORT_MAME_FM_DLL
	if(!dont_create_multiple_chips) {
//#ifdef HAS_YM2608
		if(is_ym2608 && _HAS_YM2608) {
			fmdll->Create((LPVOID*)&dllchip, clock, rate);
		} else
//#endif
		fmdll->Create((LPVOID*)&dllchip, clock * 2, rate);
		if(dllchip) {
			chip_reference_counter++;
			
			fmdll->SetVolumeFM(dllchip, decibel_fm);
			fmdll->SetVolumePSG(dllchip, decibel_psg);
			
			DWORD mask = 0;
			DWORD dwCaps = fmdll->GetCaps(dllchip);
			if((dwCaps & SUPPORT_MULTIPLE) != SUPPORT_MULTIPLE) {
				dont_create_multiple_chips = true;
			}
			if((dwCaps & SUPPORT_FM_A) == SUPPORT_FM_A) {
				mask = 0x07;
			}
			if((dwCaps & SUPPORT_FM_B) == SUPPORT_FM_B) {
				mask |= 0x38;
			}
			if((dwCaps & SUPPORT_PSG) == SUPPORT_PSG) {
				mask |= 0x1c0;
			}
			if((dwCaps & SUPPORT_ADPCM_B) == SUPPORT_ADPCM_B) {
				mask |= 0x200;
			}
			if((dwCaps & SUPPORT_RHYTHM) == SUPPORT_RHYTHM) {
				mask |= 0xfc00;
			}
//#ifdef HAS_YM2608
			if(is_ym2608 && _HAS_YM2608) {
				opna->SetChannelMask(mask);
			} else
//#endif
			opn->SetChannelMask(mask);
			fmdll->SetChannelMask(dllchip, ~mask);
		}
	}
#endif
	chip_clock = clock;
}

void YM2203::set_reg(uint32_t addr, uint32_t data)
{
	touch_sound();
//#ifdef HAS_YM2608
	if(is_ym2608 && _HAS_YM2608) {
		opna->SetReg(addr, data);
	} else {
//#endif
		if((addr & 0xf0) == 0x10) {
			return;
		}
		if(addr == 0x22) {
			data = 0x00;
		} else if(addr == 0x29) {
			data = 0x03;
		} else if(addr >= 0xb4) {
			data = 0xc0;
		}
		opn->SetReg(addr, data);
//#ifdef HAS_YM2608
	}
//#endif
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->SetReg(dllchip, addr, data);
	}
	if(0x2d <= addr && addr <= 0x2f) {
		port_log[0x2d].written = port_log[0x2e].written = port_log[0x2f].written = false;
	}
	port_log[addr].written = true;
	port_log[addr].data = data;
#endif
}

void YM2203::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
//#ifdef HAS_YM2608
	if(is_ym2608 && _HAS_YM2608) {
		clock_const = (uint32_t)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks / 2.0 + 0.5);
	} else
//#endif
	clock_const = (uint32_t)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks + 0.5);
}

#define STATE_VERSION	4

bool YM2203::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}

	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
 	}
	if(_HAS_YM2608) {
		if(!opna->ProcessState((void *)state_fio, loading)) {
 			return false;
 		}
	} else {
		if(!opn->ProcessState((void *)state_fio, loading)) {
			return false;
		}
	}
#ifdef SUPPORT_MAME_FM_DLL
	state_fio->StateBuffer(port_log, sizeof(port_log), 1);
#endif
	state_fio->StateUint8(ch);
	state_fio->StateUint8(fnum2);
	if(_HAS_YM2608) {
		state_fio->StateUint8(ch1);
		state_fio->StateUint8(data1);
		state_fio->StateUint8(fnum21);
	}
 	for(int i = 0; i < 2; i++) {
		state_fio->StateUint8(port[i].wreg);
		state_fio->StateUint8(port[i].rreg);
		state_fio->StateBool(port[i].first);
 	}
	state_fio->StateUint8(mode);
	state_fio->StateInt32(chip_clock);
	state_fio->StateBool(irq_prev);
	state_fio->StateBool(mute);
	state_fio->StateUint32(clock_prev);
	state_fio->StateUint32(clock_accum);
	state_fio->StateUint32(clock_const);
	state_fio->StateUint32(clock_busy);
	state_fio->StateInt32(timer_event_id);
	state_fio->StateBool(busy);
 	
#ifdef SUPPORT_MAME_FM_DLL
 	// post process
	if(loading && dllchip) {
		fmdll->Reset(dllchip);
		for(int i = 0; i < 0x200; i++) {
			// write fnum2 before fnum1
			int _ch = ((i >= 0xa0 && i <= 0xaf) || (i >= 0x1a0 && i <= 0x1a7)) ? (i ^ 4) : i;
			if(port_log[ch].written) {
				fmdll->SetReg(dllchip, _ch, port_log[ch].data);
			}
		}
		if(_HAS_YM2608) {
			if(is_ym2608) {
				BYTE *dest = fmdll->GetADPCMBuffer(dllchip);
				if(dest != NULL) {
					memcpy(dest, opna->GetADPCMBuffer(), 0x40000);
				}
			}
		}
	}
#endif
	return true;
}


