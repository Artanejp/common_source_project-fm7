/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.15-

	[ AY-3-8910 / YM2203 / YM2608 ]
*/

#include "ym2203.h"
#include <math.h>

#define EVENT_FM_TIMER	0

#ifdef SUPPORT_MAME_FM_DLL
static bool dont_create_multiple_chips = false;
#endif

void YM2203::initialize()
{
#ifdef HAS_YM2608
	if(is_ym2608) {
		opna = new FM::OPNA;
	} else
#endif
	opn = new FM::OPN;

#ifdef SUPPORT_MAME_FM_DLL

   //	fmdll = new CFMDLL(_T("mamefm.dll"));
	fmdll = new CFMDLL(config.fmgen_dll_path);
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
#ifdef HAS_YM2608
	if(is_ym2608) {
		delete opna;
	} else
#endif
	delete opn;
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->Release(dllchip);
	}
	delete fmdll;
#endif
}

void YM2203::reset()
{
#ifdef HAS_YM2608
	if(is_ym2608) {
		opna->Reset();
	} else
#endif
	opn->Reset();
#ifdef SUPPORT_MAME_FM_DLL
	if(dllchip) {
		fmdll->Reset(dllchip);
	}
	memset(port_log, 0, sizeof(port_log));
#endif
	fnum2 = 0;
#ifdef HAS_YM2608
	fnum21 = 0;
#endif
	
	// stop timer
	timer_event_id = -1;
	this->set_reg(0x27, 0);
	
#ifdef SUPPORT_YM2203_PORT
	port[0].first = port[1].first = true;
	port[0].wreg = port[1].wreg = 0;//0xff;
#ifdef YM2203_PORT_MODE
	mode = YM2203_PORT_MODE;
#else
	mode = 0;
#endif
#endif
	irq_prev = busy = false;
}

#ifdef HAS_YM2608
#define amask (is_ym2608 ? 3 : 1)
#else
#define amask 1
#endif

void YM2203::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & amask) {
	case 0:
#ifdef HAS_YM_SERIES
		ch = data;
		// write dummy data for prescaler
		if(0x2d <= ch && ch <= 0x2f) {
			update_count();
			this->set_reg(ch, 0);
			update_interrupt();
			clock_busy = get_current_clock();
			busy = true;
		}
#else
		ch = data & 0x0f;
#endif
		break;
	case 1:
#ifdef SUPPORT_YM2203_PORT
		if(ch == 7) {
#ifdef YM2203_PORT_MODE
			mode = (data & 0x3f) | YM2203_PORT_MODE;
#else
			mode = data;
#endif
		} else if(ch == 14) {
#ifdef SUPPORT_YM2203_PORT_A
			if(port[0].wreg != data || port[0].first) {
				write_signals(&port[0].outputs, data);
				port[0].wreg = data;
				port[0].first = false;
			}
#endif
		} else if(ch == 15) {
#ifdef SUPPORT_YM2203_PORT_B
			if(port[1].wreg != data || port[1].first) {
				write_signals(&port[1].outputs, data);
				port[1].wreg = data;
				port[1].first = false;
			}
#endif
		}
#endif
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
#ifdef HAS_YM_SERIES
			update_interrupt();
			clock_busy = get_current_clock();
			busy = true;
#endif
		}
		break;
#ifdef HAS_YM2608
	case 2:
		ch1 = data1 = data;
		break;
	case 3:
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
		break;
#endif
	}
}

uint32_t YM2203::read_io8(uint32_t addr)
{
	switch(addr & amask) {
#ifdef HAS_YM_SERIES
	case 0:
		{
			/* BUSY : x : x : x : x : x : FLAGB : FLAGA */
			update_count();
			update_interrupt();
			uint32_t status;
#ifdef HAS_YM2608
			if(is_ym2608) {
				status = opna->ReadStatus() & ~0x80;
			} else
#endif
			status = opn->ReadStatus() & ~0x80;
			if(busy) {
				// from PC-88 machine language master bible (XM8 version 1.00)
#ifdef HAS_YM2608
				if (get_passed_usec(clock_busy) < (is_ym2608 ? 4.25 : 2.13)) {
#else
				if (get_passed_usec(clock_busy) < 2.13) {
#endif
					status |= 0x80;
				} else {
					busy = false;
				}
			}
			return status;
		}
#endif
	case 1:
#ifdef SUPPORT_YM2203_PORT
		if(ch == 14) {
#ifdef SUPPORT_YM2203_PORT_A
			return (mode & 0x40) ? port[0].wreg : port[0].rreg;
#endif
		} else if(ch == 15) {
#ifdef SUPPORT_YM2203_PORT_B
			return (mode & 0x80) ? port[1].wreg : port[1].rreg;
#endif
		}
#endif
#ifdef HAS_YM2608
		if(is_ym2608) {
			return opna->GetReg(ch);
		} else
#endif
		return opn->GetReg(ch);
#ifdef HAS_YM2608
	case 2:
		{
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
	case 3:
		if(ch1 == 8) {
			return opna->GetReg(0x100 | ch1);
//		} else if(ch1 == 0x0f) {
//			return 0x80; // from mame fm.c
		}
		return data1;
#endif
	}
	return 0xff;
}

void YM2203::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_YM2203_MUTE) {
		mute = ((data & mask) != 0);
#ifdef SUPPORT_YM2203_PORT_A
	} else if(id == SIG_YM2203_PORT_A) {
		port[0].rreg = (port[0].rreg & ~mask) | (data & mask);
#endif
#ifdef SUPPORT_YM2203_PORT_B
	} else if(id == SIG_YM2203_PORT_B) {
		port[1].rreg = (port[1].rreg & ~mask) | (data & mask);
#endif
	}
}

void YM2203::event_vline(int v, int clock)
{
	update_count();
#ifdef HAS_YM_SERIES
	update_interrupt();
#endif
}

void YM2203::event_callback(int event_id, int error)
{
	update_count();
#ifdef HAS_YM_SERIES
	update_interrupt();
#endif
	timer_event_id = -1;
	update_event();
}

void YM2203::update_count()
{
	clock_accum += clock_const * get_passed_clock(clock_prev);
	uint32_t count = clock_accum >> 20;
	if(count) {
#ifdef HAS_YM2608
		if(is_ym2608) {
			opna->Count(count);
		} else
#endif
		opn->Count(count);
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
#ifdef HAS_YM2608
	if(is_ym2608) {
		count = opna->GetNextEvent();
	} else
#endif
	count = opn->GetNextEvent();
	
	if(count > 0) {
#ifdef HAS_YM2608
		if(is_ym2608) {
			register_event(this, EVENT_FM_TIMER, 1000000.0 / (double)chip_clock * (double)count * 2.0, false, &timer_event_id);
		} else
#endif
		register_event(this, EVENT_FM_TIMER, 1000000.0 / (double)chip_clock * (double)count, false, &timer_event_id);
	}
}

#ifdef HAS_YM_SERIES
void YM2203::update_interrupt()
{
	bool irq;
#ifdef HAS_YM2608
	if(is_ym2608) {
		irq = opna->ReadIRQ();
	} else
#endif
	irq = opn->ReadIRQ();
	if(!irq_prev && irq) {
		write_signals(&outputs_irq, 0xffffffff);
	} else if(irq_prev && !irq) {
		write_signals(&outputs_irq, 0);
	}
	irq_prev = irq;
}
#endif

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
	   
#ifdef HAS_YM2608
		if(is_ym2608) {
			opna->Mix(dbuffer, cnt);
		} else
#endif
		opn->Mix(dbuffer, cnt);
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

void YM2203::set_volume(int ch, int decibel_l, int decibel_r)
{
	v_right_volume = (int)(pow(10.0, (double)decibel_vol / 10.0) * (double)right_volume);
	v_left_volume = (int)(pow(10.0, (double)decibel_vol / 10.0) * (double)left_volume);
	if(ch == 0) {
#ifdef HAS_YM2608
		if(is_ym2608) {
			opna->SetVolumeFM(base_decibel_fm + decibel_l, base_decibel_fm + decibel_r);
		} else
#endif
		opn->SetVolumeFM(base_decibel_fm + decibel_l, base_decibel_fm + decibel_r);
#ifdef SUPPORT_MAME_FM_DLL
		if(dllchip) {
			fmdll->SetVolumeFM(dllchip, base_decibel_fm + decibel_l);
		}
#endif
	} else if(ch == 1) {
#ifdef HAS_YM2608
		if(is_ym2608) {
			opna->SetVolumePSG(base_decibel_psg + decibel_l, base_decibel_psg + decibel_r);
		} else
#endif
		opn->SetVolumePSG(base_decibel_psg + decibel_l, base_decibel_psg + decibel_r);
#ifdef SUPPORT_MAME_FM_DLL
		if(dllchip) {
			fmdll->SetVolumePSG(dllchip, base_decibel_psg + decibel_l);
		}
#endif
#ifdef HAS_YM2608
	} else if(ch == 2) {
		if(is_ym2608) {
			opna->SetVolumeADPCM(decibel_l, decibel_r);
		}
	} else if(ch == 3) {
		if(is_ym2608) {
			opna->SetVolumeRhythmTotal(decibel_l, decibel_r);
		}
#endif
	}
}

void YM2203::initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg)
{
#ifdef HAS_YM2608
	if(is_ym2608) {
		opna->Init(clock, rate, false, get_application_path());
		opna->SetVolumeFM(decibel_fm, decibel_fm);
		opna->SetVolumePSG(decibel_psg, decibel_psg);
	} else {
#endif
		opn->Init(clock, rate, false, NULL);
		opn->SetVolumeFM(decibel_fm, decibel_fm);
		opn->SetVolumePSG(decibel_psg, decibel_psg);
#ifdef HAS_YM2608
	}
#endif
	base_decibel_fm = decibel_fm;
	base_decibel_psg = decibel_psg;
	
#ifdef SUPPORT_MAME_FM_DLL
	if(!dont_create_multiple_chips) {
#ifdef HAS_YM2608
		if(is_ym2608) {
			fmdll->Create((LPVOID*)&dllchip, clock, rate);
		} else
#endif
		fmdll->Create((LPVOID*)&dllchip, clock * 2, rate);
		if(dllchip) {
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
#ifdef HAS_YM2608
			if(is_ym2608) {
				opna->SetChannelMask(mask);
			} else
#endif
			opn->SetChannelMask(mask);
			fmdll->SetChannelMask(dllchip, ~mask);
		}
	}
#endif
	chip_clock = clock;
}

void YM2203::set_reg(uint32_t addr, uint32_t data)
{
#ifdef HAS_YM2608
	if(is_ym2608) {
		opna->SetReg(addr, data);
	} else
#endif
	opn->SetReg(addr, data);
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
#ifdef HAS_YM2608
	if(is_ym2608) {
		clock_const = (uint32_t)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks / 2.0 + 0.5);
	} else
#endif
	clock_const = (uint32_t)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks + 0.5);
}

#define STATE_VERSION	4

void YM2203::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
#ifdef HAS_YM2608
	if(is_ym2608) {
		opna->SaveState((void *)state_fio);;
	} else
#endif
	opn->SaveState((void *)state_fio);
#ifdef SUPPORT_MAME_FM_DLL
	state_fio->Fwrite(port_log, sizeof(port_log), 1);
#endif
	state_fio->FputUint8(ch);
	state_fio->FputUint8(fnum2);
#ifdef HAS_YM2608
	state_fio->FputUint8(ch1);
	state_fio->FputUint8(data1);
	state_fio->FputUint8(fnum21);
#endif
#ifdef SUPPORT_YM2203_PORT
	for(int i = 0; i < 2; i++) {
		state_fio->FputUint8(port[i].wreg);
		state_fio->FputUint8(port[i].rreg);
		state_fio->FputBool(port[i].first);
	}
	state_fio->FputUint8(mode);
#endif
	state_fio->FputInt32(chip_clock);
	state_fio->FputBool(irq_prev);
	state_fio->FputBool(mute);
	state_fio->FputUint32(clock_prev);
	state_fio->FputUint32(clock_accum);
	state_fio->FputUint32(clock_const);
	state_fio->FputUint32(clock_busy);
	state_fio->FputInt32(timer_event_id);
	state_fio->FputBool(busy);
	state_fio->FputInt32(decibel_vol);
	state_fio->FputInt32(left_volume);
	state_fio->FputInt32(right_volume);
	state_fio->FputInt32(v_left_volume);
	state_fio->FputInt32(v_right_volume);
}

bool YM2203::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
#ifdef HAS_YM2608
	if(is_ym2608) {
		if(!opna->LoadState((void *)state_fio)) {
			return false;
		}
	} else {
#endif
		if(!opn->LoadState((void *)state_fio)) {
			return false;
		}
#ifdef HAS_YM2608
	}
#endif
#ifdef SUPPORT_MAME_FM_DLL
	state_fio->Fread(port_log, sizeof(port_log), 1);
#endif
	ch = state_fio->FgetUint8();
	fnum2 = state_fio->FgetUint8();
#ifdef HAS_YM2608
	ch1 = state_fio->FgetUint8();
	data1 = state_fio->FgetUint8();
	fnum21 = state_fio->FgetUint8();
#endif
#ifdef SUPPORT_YM2203_PORT
	for(int i = 0; i < 2; i++) {
		port[i].wreg = state_fio->FgetUint8();
		port[i].rreg = state_fio->FgetUint8();
		port[i].first = state_fio->FgetBool();
	}
	mode = state_fio->FgetUint8();
#endif
	chip_clock = state_fio->FgetInt32();
	irq_prev = state_fio->FgetBool();
	mute = state_fio->FgetBool();
	clock_prev = state_fio->FgetUint32();
	clock_accum = state_fio->FgetUint32();
	clock_const = state_fio->FgetUint32();
	clock_busy = state_fio->FgetUint32();
	timer_event_id = state_fio->FgetInt32();
	busy = state_fio->FgetBool();

#ifdef SUPPORT_MAME_FM_DLL
	// post process
	if(dllchip) {
		fmdll->Reset(dllchip);
		for(int i = 0; i < 0x200; i++) {
			// write fnum2 begore fnum1
			int ch = ((i >= 0xa0 && i <= 0xaf) || (i >= 0x1a0 && i <= 0x1a7)) ? (i ^ 4) : i;
			if(port_log[ch].written) {
				fmdll->SetReg(dllchip, ch, port_log[ch].data);
			}
		}
	}
#endif
	decibel_vol = state_fio->FgetInt32();
	left_volume = state_fio->FgetInt32();
	right_volume = state_fio->FgetInt32();
	v_left_volume = state_fio->FgetInt32();
	v_right_volume = state_fio->FgetInt32();

	return true;
}

