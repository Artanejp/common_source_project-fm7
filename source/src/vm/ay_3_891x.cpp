/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.15-

	[ AY-3-8910 / AY-3-8912 / AY-3-8913 ]
*/

#include "ay_3_891x.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#define EVENT_FM_TIMER	0

void AY_3_891X::initialize()
{
	opn = new FM::OPN;
	opn->is_ay3_891x = true;
	
	register_vline_event(this);
	mute = false;
	clock_prev = clock_accum = clock_busy = 0;
	
#ifdef USE_DEBUGGER
	if(d_debugger != NULL) {
		d_debugger->set_device_name(_T("Debugger (AY-3-891X PSG)"));
		d_debugger->set_context_mem(this);
		d_debugger->set_context_io(vm->dummy);
	}
#endif
}

void AY_3_891X::release()
{
	delete opn;
}

void AY_3_891X::reset()
{
	touch_sound();
	opn->Reset();
	
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
#ifdef USE_DEBUGGER
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			d_debugger->write_via_debugger_data8(ch, data);
		} else
#endif
		this->write_via_debugger_data8(ch, data);
		break;
	}
}

uint32_t AY_3_891X::read_io8(uint32_t addr)
{
	switch(addr & 1) {
	case 1:
#ifdef USE_DEBUGGER
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			return d_debugger->read_via_debugger_data8(ch);
		} else
#endif
		return this->read_via_debugger_data8(ch);
	}
	return 0xff;
}

void AY_3_891X::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
	if(addr < 16) {
#ifdef SUPPORT_AY_3_891X_PORT
		if(addr == 7) {
#ifdef AY_3_891X_PORT_MODE
			mode = (data & 0x3f) | AY_3_891X_PORT_MODE;
#else
			mode = data;
#endif
		} else if(addr == 14) {
#ifdef SUPPORT_AY_3_891X_PORT_A
			if(port[0].wreg != data || port[0].first) {
				write_signals(&port[0].outputs, data);
				port[0].wreg = data;
				port[0].first = false;
			}
#endif
		} else if(addr == 15) {
#ifdef SUPPORT_AY_3_891X_PORT_B
			if(port[1].wreg != data || port[1].first) {
				write_signals(&port[1].outputs, data);
				port[1].wreg = data;
				port[1].first = false;
			}
#endif
		}
#endif
		update_count();
		this->set_reg(addr, data);
	}
}

uint32_t AY_3_891X::read_via_debugger_data8(uint32_t addr)
{
	if(addr < 16) {
#ifdef SUPPORT_AY_3_891X_PORT
		if(addr == 14) {
#ifdef SUPPORT_AY_3_891X_PORT_A
			return (mode & 0x40) ? port[0].wreg : port[0].rreg;
#endif
		} else if(addr == 15) {
#ifdef SUPPORT_AY_3_891X_PORT_B
			return (mode & 0x80) ? port[1].wreg : port[1].rreg;
#endif
		}
#endif
		return opn->GetReg(addr);
	}
	return 0;
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
#ifdef SUPPORT_AY_3_891X_PORT
	for(int i = 0; i < 2; i++) {
		state_fio->StateValue(port[i].wreg);
		state_fio->StateValue(port[i].rreg);
		state_fio->StateValue(port[i].first);
	}
	state_fio->StateValue(mode);
#endif
	state_fio->StateValue(chip_clock);
	state_fio->StateValue(irq_prev);
	state_fio->StateValue(mute);
	state_fio->StateValue(clock_prev);
	state_fio->StateValue(clock_accum);
	state_fio->StateValue(clock_const);
	state_fio->StateValue(clock_busy);
	state_fio->StateValue(timer_event_id);
	state_fio->StateValue(busy);
	return true;
}

