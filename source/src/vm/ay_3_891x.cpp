/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2017.02.02-

	[ AY-3-8910 / 8912 / 8913 ]
	History:
	  2017-02-02: Fork from YM2203.
*/

#include "ay_3_891x.h"
#include <math.h>

#define EVENT_PSG_TIMER	0


void AY_3_891X::initialize()
{
	psg = new PSG_AY_3_891X;

	register_vline_event(this);
	mute = false;
	clock_prev = clock_accum = clock_busy = 0;

	left_volume = right_volume = 256;
	v_left_volume = v_right_volume = 256;
}

void AY_3_891X::release()
{
	delete psg;
}

void AY_3_891X::reset()
{
	touch_sound();
	psg->Reset();
	fnum2 = 0;
	
	// stop timer
#ifdef SUPPORT_AY_3_891X_PORT
	port[0].first = port[1].first = true;
	port[0].wreg = port[1].wreg = 0;//0xff;
#ifdef AY_3_891X_PORT_MODE
	mode = AY_3_891X_PORT_MODE;
#else
	mode = 0;
#endif
#endif
	irq_prev = busy = false;
}

#define amask 1

void AY_3_891X::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & amask) {
	case 0:
		ch = data & 0x0f;
		break;
	case 1:
#ifdef SUPPORT_AY_3_891X_PORT
		if(ch == 7) {
#ifdef AY_3_891X_PORT_MODE
			mode = (data & 0x3f) | AY_3_891X_PORT_MODE;
#else
			mode = data;
#endif
		} else if(ch == 14) {
#ifdef SUPPORT_AY_3_891X_PORT_A
			if(port[0].wreg != data || port[0].first) {
				write_signals(&port[0].outputs, data);
				port[0].wreg = data;
				port[0].first = false;
			}
#endif
		} else if(ch == 15) {
#ifdef SUPPORT_AY_3_891X_PORT_B
			if(port[1].wreg != data || port[1].first) {
				write_signals(&port[1].outputs, data);
				port[1].wreg = data;
				port[1].first = false;
			}
#endif
		}
#endif
	     {
			update_count();
		// XM8 version 1.20
			this->set_reg(ch, data);
		}
		break;
	}
}

uint32_t AY_3_891X::read_io8(uint32_t addr)
{
	switch(addr & amask) {
	case 1:
#ifdef SUPPORT_AY_3_891X_PORT
		if(ch == 14) {
#ifdef SUPPORT_AY_3_891X_PORT_A
			return (mode & 0x40) ? port[0].wreg : port[0].rreg;
#endif
		} else if(ch == 15) {
#ifdef SUPPORT_AY_3_891X_PORT_B
			return (mode & 0x80) ? port[1].wreg : port[1].rreg;
#endif
		}
#endif
		return psg->GetReg(ch);
	}
	return 0xff;
}

void AY_3_891X::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_AY_3_891X_MUTE) {
		mute = ((data & mask) != 0);
#ifdef SUPPORT_AY_3_891X_PORT_A
	} else if(id == SIG_AY_3_891X_PORT_A) {
		port[0].rreg = (port[0].rreg & ~mask) | (data & mask);
#endif
#ifdef SUPPORT_AY_3_891X_PORT_B
	} else if(id == SIG_AY_3_891X_PORT_B) {
		port[1].rreg = (port[1].rreg & ~mask) | (data & mask);
#endif
	}
}

void AY_3_891X::event_vline(int v, int clock)
{
	update_count();
}

void AY_3_891X::event_callback(int event_id, int error)
{
	update_count();
	//timer_event_id = -1;
	update_event();
}

void AY_3_891X::update_count()
{
	clock_accum += clock_const * get_passed_clock(clock_prev);
	uint32_t count = clock_accum >> 20;
	if(count) {
		//psg->Count(count);
		clock_accum -= count << 20;
	}
	clock_prev = get_current_clock();
}

void AY_3_891X::update_event()
{
	//if(timer_event_id != -1) {
	//	cancel_event(this, timer_event_id);
	//	timer_event_id = -1;
	//}
	
	//int count;
	//count = psg->GetNextEvent();
	
	//if(count > 0) {
	//	register_event(this, EVENT_PSG_TIMER, 1000000.0 / (double)chip_clock * (double)count, false, &timer_event_id);
	//}
}


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


void AY_3_891X::mix(int32_t* buffer, int cnt)
{
	if(cnt > 0 && !mute) {
		int32_t *dbuffer = (int32_t *)malloc((cnt * 2 + 2) * sizeof(int32_t));
		memset((void *)dbuffer, 0x00, (cnt * 2 + 2) * sizeof(int32_t));
	   
		psg->Mix(dbuffer, cnt);
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

void AY_3_891X::set_volume(int ch, int decibel_l, int decibel_r)
{
	v_right_volume = (int)(pow(10.0, (double)decibel_vol / 10.0) * (double)right_volume);
	v_left_volume = (int)(pow(10.0, (double)decibel_vol / 10.0) * (double)left_volume);
	if(ch == 0) {
		psg->SetVolume(base_decibel_psg + decibel_l, base_decibel_psg + decibel_r);
	}
}

void AY_3_891X::initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg)
{
	//psg->Init(clock, rate, false, NULL);
	psg->Init(clock, rate);
	psg->SetVolume(decibel_psg, decibel_psg);
	base_decibel_psg = decibel_psg;
	chip_clock = clock;
}

void AY_3_891X::set_reg(uint32_t addr, uint32_t data)
{
	touch_sound();
	if((addr >= 0x2d) && (addr <= 0x2f)) {
		psg->SetPrescaler(addr - 0x2d);
		return;
	}
	psg->SetReg(addr, data);
}

void AY_3_891X::update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
{
	clock_const = (uint32_t)((double)chip_clock * 1024.0 * 1024.0 / (double)new_clocks + 0.5);
}

#define STATE_VERSION	1

void AY_3_891X::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	psg->SaveState((void *)state_fio);
	state_fio->FputUint8(ch);
	state_fio->FputUint8(fnum2);
#ifdef SUPPORT_AY_3_891X_PORT
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
	//state_fio->FputInt32(timer_event_id);
	state_fio->FputBool(busy);
	state_fio->FputInt32(decibel_vol);
	state_fio->FputInt32(left_volume);
	state_fio->FputInt32(right_volume);
	state_fio->FputInt32(v_left_volume);
	state_fio->FputInt32(v_right_volume);
}

bool AY_3_891X::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	if(!psg->LoadState((void *)state_fio)) {
		return false;
	}
	ch = state_fio->FgetUint8();
	fnum2 = state_fio->FgetUint8();
#ifdef SUPPORT_AY_3_891X_PORT
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
	//timer_event_id = state_fio->FgetInt32();
	busy = state_fio->FgetBool();

	decibel_vol = state_fio->FgetInt32();
	left_volume = state_fio->FgetInt32();
	right_volume = state_fio->FgetInt32();
	v_left_volume = state_fio->FgetInt32();
	v_right_volume = state_fio->FgetInt32();
	//touch_sound();

	return true;
}

