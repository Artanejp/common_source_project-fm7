/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.15-

	[ AY-3-8910 / AY-3-8912 / AY-3-8913 ]
*/

#include "ay_3_891x.h"

#define EVENT_FM_TIMER	0

void AY_3_891X::initialize()
{
	DEVICE::initialize();
	opn = new FM::OPN;
	opn->is_ay3_891x = true;
	
	register_vline_event(this);
	mute = false;
	clock_prev = clock_accum = clock_busy = 0;
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
	fnum2 = 0;
	
	// stop timer
	timer_event_id = -1;
	this->set_reg(0x27, 0);
	
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

void AY_3_891X::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 1) {
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
		}
		break;
	}
}

uint32_t AY_3_891X::read_io8(uint32_t addr)
{
	switch(addr & 1) {
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
		return opn->GetReg(ch);
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
		opn->Mix(buffer, cnt);
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

void AY_3_891X::initialize_sound(int rate, int clock, int samples, int decibel_fm, int decibel_psg)
{
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

#define STATE_VERSION	4

#include "../statesub.h"

void AY_3_891X::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_UINT8(ch);
	DECL_STATE_ENTRY_UINT8(fnum2);
#ifdef SUPPORT_AY_3_891X_PORT
	for(int i = 0; i < 2; i++) {
		DECL_STATE_ENTRY_UINT8_MEMBER((port[i].wreg), i);
		DECL_STATE_ENTRY_UINT8_MEMBER((port[i].rreg), i);
		DECL_STATE_ENTRY_BOOL_MEMBER((port[i].first), i);
	}
	DECL_STATE_ENTRY_UINT8(mode);
#endif
	DECL_STATE_ENTRY_INT32(chip_clock);
	DECL_STATE_ENTRY_BOOL(irq_prev);
	DECL_STATE_ENTRY_BOOL(mute);
	DECL_STATE_ENTRY_UINT32(clock_prev);
	DECL_STATE_ENTRY_UINT32(clock_accum);
	DECL_STATE_ENTRY_UINT32(clock_const);
	DECL_STATE_ENTRY_UINT32(clock_busy);
	DECL_STATE_ENTRY_INT32(timer_event_id);
	DECL_STATE_ENTRY_BOOL(busy);

	leave_decl_state();

	opn->DeclState();
}

void AY_3_891X::save_state(FILEIO* state_fio)
{
	//state_fio->FputUint32(STATE_VERSION);
	//state_fio->FputInt32(this_device_id);
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
	
	opn->SaveState((void *)state_fio);
	//state_fio->FputUint8(ch);
	//state_fio->FputUint8(fnum2);
#ifdef SUPPORT_AY_3_891X_PORT
	//for(int i = 0; i < 2; i++) {
	//	state_fio->FputUint8(port[i].wreg);
	//	state_fio->FputUint8(port[i].rreg);
	//	state_fio->FputBool(port[i].first);
	//}
	//state_fio->FputUint8(mode);
#endif
	//state_fio->FputInt32(chip_clock);
	//state_fio->FputBool(irq_prev);
	//state_fio->FputBool(mute);
	//state_fio->FputUint32(clock_prev);
	//state_fio->FputUint32(clock_accum);
	//state_fio->FputUint32(clock_const);
	//state_fio->FputUint32(clock_busy);
	//state_fio->FputInt32(timer_event_id);
	//state_fio->FputBool(busy);
}

bool AY_3_891X::load_state(FILEIO* state_fio)
{
	//if(state_fio->FgetUint32() != STATE_VERSION) {
	//	return false;
	//}
	//if(state_fio->FgetInt32() != this_device_id) {
	//	return false;
	//}
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) return false;
	if(!opn->LoadState((void *)state_fio)) {
		return false;
	}
	//ch = state_fio->FgetUint8();
	//fnum2 = state_fio->FgetUint8();
#ifdef SUPPORT_AY_3_891X_PORT
	//for(int i = 0; i < 2; i++) {
	//	port[i].wreg = state_fio->FgetUint8();
	//	port[i].rreg = state_fio->FgetUint8();
	//	port[i].first = state_fio->FgetBool();
	//}
	//mode = state_fio->FgetUint8();
#endif
	//chip_clock = state_fio->FgetInt32();
	//irq_prev = state_fio->FgetBool();
	//mute = state_fio->FgetBool();
	//clock_prev = state_fio->FgetUint32();
	//clock_accum = state_fio->FgetUint32();
	//clock_const = state_fio->FgetUint32();
	//clock_busy = state_fio->FgetUint32();
	//timer_event_id = state_fio->FgetInt32();
	//busy = state_fio->FgetBool();
	return true;
}

