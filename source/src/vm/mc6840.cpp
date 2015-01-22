/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2013.11.08-

	[ mc6840 ]
*/

#include "mc6840.h"
#include "../fileio.h"

#define EVENT_CH0	0
#define EVENT_CH1	1
#define EVENT_CH2	2
#define EVENT_ALL	3

void MC6840::initialize()
{
	if(timer[0].freq != 0 && timer[0].freq == timer[1].freq && timer[0].freq == timer[2].freq) {
		register_event(tihs, EVENT_ALL, 1000000.0 / timer[0].freq, true, NULL);
	} else {
		for(int ch = 0; ch < 3; ch++) {
			if(timer[ch].freq != 0) {
				register_event(tihs, EVENT_CH0 + ch, 1000000.0 / timer[ch].freq, true, NULL);
			}
		}
	}
}

void MC6840::reset()
{
	for(int ch = 0; ch < 3; ch++) {
		timer[ch].counter = 0xffff;
		timer[ch].latch = 0xffff;
		timer[ch].control = (ch == 0) ? 1 : 0;
		timer[ch].in_pin = false;
		timer[ch].out_pin = true;
		timer[ch].signal = true;
		timer[ch].once = false;
		timer[ch].clocks = 0;
		timer[ch].prescaler = 0;
	}
	status = status_read = 0;
}

void MC6840::write_io8(uint32 addr, uint32 data)
{
	static const int chs[8] = {-1, 1, 0, 0, 1, 1, 2, 2};
	addr &= 7;
	int ch = chs[addr];
	
	switch(addr) {
	case 0:
	case 1:
		set_control(ch, data);
		break;
	case 2:
	case 4:
	case 6:
		timer[ch].latch = timer[ch].latch_hi | data;
		if(!(timer[ch].control & 0x10)) {
			set_counter(ch);
		}
		break;
	case 3:
	case 5:
	case 7:
		timer[ch].latch_hi = data << 8;
		break;
	}
}

uint32 MC6840::read_io8(uint32 addr)
{
	static const int chs[8] = {-1, 1, 0, 0, 1, 1, 2, 2};
	addr &= 7;
	int ch = chs[addr];
	
	switch(addr) {
	case 1:
		status_read |= status;
		return status;
	case 2:
	case 4:
	case 6:
		if(status_read & (1 << ch)) {
			set_irq(ch, false);
		}
		return timer[ch].counter_lo;
	case 3:
	case 5:
	case 7:
		timer[ch].counter_lo = timer[ch].counter & 0xff;
		return timer[ch].counter_read >> 8;
	}
	return 0xff;
}

void MC6840::event_callback(int event_id, int err)
{
	switch(event_id) {
	case EVENT_CH0:
	case EVENT_CH1:
	case EVENT_CH2:
		{
			int ch = event_id - EVENT_CH0;
			if(!(timer[ch].control & 2)) {
				input_clocks(ch, 1);
			}
		}
		break;
	case EVENT_ALL:
		for(int ch = 0; ch < 3; ch++) {
			if(!(timer[ch].control & 2)) {
				input_clocks(ch, 1);
			}
		}
		break;
	}
}

void MC6840::write_signal(int id, uint32 data, uint32 mask)
{
	int ch = id - SIG_MC6840_CLOCK_0;
	bool signal = ((data & mask) != 0);
	
	if(timer[ch].in_pin && !signal) {
		if(timer[ch].control & 2) {
			input_clocks(ch, 1);
		}
	}
	timer[ch].in_pin = signal;
}

void MC6840::set_control(int ch, uint32 data)
{
	if(ch == -1) {
		ch = (timer[1].control & 1) ? 0 : 2;
	}
	if(ch == 0 && !(timer[0].control & 1) && (data & 1)) {
		for(int i = 0; i < 3; i++) {
			set_counter(ch);
		}
	}
	if((timer[ch].control & 0x80) != (data & 0x80)) {
		set_signal(ch, timer[ch].signal);
	}
	timer[ch].control = data;
}

void MC6840::set_counter(int ch)
{
	timer[ch].counter = timer[ch].latch;
	timer[ch].prescaler = (timer[ch].control & 1) ? 3 : 0;
	set_irq(ch, false);
	set_signal(ch, false);
	timer[ch].once = false;
}

void MC6840::input_clocks(int ch, int clocks)
{
	timer[ch].clocks += clocks;
	int count = timer[ch].clocks >> timer[ch].prescaler;
	timer[ch].clocks -= count << timer[ch].prescaler;
	
	if(!(timer[0].control & 1)) {
		if(timer[ch].control & 4) {
			int lo_counter = timer[ch].counter & 0xff;
			int hi_counter = timer[ch].counter >> 8;
			lo_counter -= count;
			while(lo_counter < 0) {
				lo_counter += (timer[ch].latch & 0xff) + 1;
				if(--hi_counter < 0) {
					hi_counter += (timer[ch].latch >> 8) + 1;
					set_irq(ch, true);
				}
			}
			set_signal(ch, (hi_counter == 0));
			timer[ch].counter = (uint16)(lo_counter | (hi_counter << 8));
		} else {
			int counter = timer[ch].counter;
			counter -= count;
			while(counter < 0) {
				counter += timer[ch].latch + 1;
				set_irq(ch, true);
				set_signal(ch, !timer[ch].signal);
			}
			timer[ch].counter = (uint16)counter;
		}
	}
}

void MC6840::set_irq(int ch, bool signal)
{
	if(signal) {
		status |= (1 << ch);
		status_read &= ~(1 << ch);
	} else {
		status &= ~(1 << ch);
	}
	bool prev = ((status & 0x80) != 0);
	bool next = ((status & 1) && (timer[0].control & 0x40)) || ((status & 2) && (timer[1].control & 0x40)) || ((status & 4) && (timer[2].control & 0x40));
	
	if(prev != next) {
		status ^= 0x80;
		write_signals(&outputs_irq, next ? 0xffffffff : 0);
	}
}

void MC6840::set_signal(int ch, bool signal)
{
	if(timer[ch].signal && !signal && (timer[ch].control & 0x20) {
		timer[ch].once = true;
	}
	timer[ch].signal = signal;
	
	if(!(timer[ch].control & 0x80) || timer[ch].once) {
		signal = false;
	}
	if(timer[ch].out_pin != signal) {
		write_signals(&timer[ch].outputs, signal ? 0xffffffff : 0);
		timer[ch].out_pin = signal;
	}
}

#define STATE_VERSION	1

void MC6840::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	for(int i = 0; i < 3; i++) {
		state_fio->FputUint16(timer[i].counter);
		state_fio->FputUint16(timer[i].latch);
		state_fio->FputUint8(timer[i].counter_lo);
		state_fio->FputUint8(timer[i].latch_hi);
		state_fio->FputUint8(timer[i].control);
		state_fio->FputBool(timer[i].in_pin);
		state_fio->FputBool(timer[i].out_pin);
		state_fio->FputBool(timer[i].signal);
		state_fio->FputBool(timer[i].once);
		state_fio->FputInt32(timer[i].clocks);
		state_fio->FputInt32(timer[i].prescaler);
		state_fio->FputInt32(timer[i].freq);
	}
	state_fio->FputUint8(status);
	state_fio->FputUint8(status_read);
}

bool MC6840::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	for(int i = 0; i < 3; i++) {
		timer[i].counter = state_fio->FgetUint16();
		timer[i].latch = state_fio->FgetUint16();
		timer[i].counter_lo = state_fio->FgetUint8();
		timer[i].latch_hi = state_fio->FgetUint8();
		timer[i].control = state_fio->FgetUint8();
		timer[i].in_pin = state_fio->FgetBool();
		timer[i].out_pin = state_fio->FgetBool();
		timer[i].signal = state_fio->FgetBool();
		timer[i].once = state_fio->FgetBool();
		timer[i].clocks = state_fio->FgetInt32();
		timer[i].prescaler = state_fio->FgetInt32();
		timer[i].freq = state_fio->FgetInt32();
	}
	status = state_fio->FgetUint8();
	status_read = state_fio->FgetUint8();
	return true;
}

