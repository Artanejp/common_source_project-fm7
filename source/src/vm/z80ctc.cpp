/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ Z80CTC ]
*/

#include "z80ctc.h"
#include "../fileio.h"

#define EVENT_COUNTER	0
#define EVENT_TIMER	4

void Z80CTC::reset()
{
	for(int ch = 0; ch < 4; ch++) {
		counter[ch].count = counter[ch].constant = 256;
		counter[ch].clocks = 0;
		counter[ch].control = 0;
		counter[ch].slope = false;
		counter[ch].prescaler = 256;
		counter[ch].freeze = counter[ch].start = counter[ch].latch = false;
		counter[ch].clock_id = counter[ch].sysclock_id = -1;
		counter[ch].first_constant = true;
		// interrupt
		counter[ch].req_intr = false;
		counter[ch].in_service = false;
		counter[ch].vector = ch << 1;
	}
	iei = oei = true;
}

void Z80CTC::write_io8(uint32 addr, uint32 data)
{
	int ch = addr & 3;
	if(counter[ch].latch) {
		// time constant
		counter[ch].constant = data ? data : 256;
		counter[ch].latch = false;
		if(counter[ch].freeze || counter[ch].first_constant) {
			counter[ch].count = counter[ch].constant;
			counter[ch].clocks = 0;
			counter[ch].freeze = false;
			counter[ch].first_constant = false;
			update_event(ch, 0);
		}
	} else {
		if(data & 1) {
			// control word
			counter[ch].prescaler = (data & 0x20) ? 256 : 16;
			counter[ch].latch = ((data & 4) != 0);
			counter[ch].freeze = ((data & 2) != 0);
			counter[ch].start = (counter[ch].freq || !(data & 8));
			counter[ch].control = data;
			counter[ch].slope = ((data & 0x10) != 0);
			if(!(data & 0x80) && counter[ch].req_intr) {
				counter[ch].req_intr = false;
				update_intr();
			}
			update_event(ch, 0);
		} else if(ch == 0) {
			// vector
			counter[0].vector = (data & 0xf8) | 0;
			counter[1].vector = (data & 0xf8) | 2;
			counter[2].vector = (data & 0xf8) | 4;
			counter[3].vector = (data & 0xf8) | 6;
		}
	}
}

uint32 Z80CTC::read_io8(uint32 addr)
{
	int ch = addr & 3;
	// update counter
	if(counter[ch].clock_id != -1) {
		int passed = passed_clock(counter[ch].prev);
		uint32 input = (uint32)(counter[ch].freq * passed / cpu_clocks);
		if(counter[ch].input <= input) {
			input = counter[ch].input - 1;
		}
		if(input > 0) {
			input_clock(ch, input);
			// cancel and re-register event
			cancel_event(this, counter[ch].clock_id);
			counter[ch].input -= input;
			counter[ch].period -= passed;
			counter[ch].prev = current_clock();
			register_event_by_clock(this, EVENT_COUNTER + ch, counter[ch].period, false, &counter[ch].clock_id);
		}
	} else if(counter[ch].sysclock_id != -1) {
		int passed = passed_clock(counter[ch].prev);
#ifdef Z80CTC_CLOCKS
		uint32 input = (uint32)(passed * Z80CTC_CLOCKS / cpu_clocks);
#else
		uint32 input = passed;
#endif
		if(counter[ch].input <= input) {
			input = counter[ch].input - 1;
		}
		if(input > 0) {
			input_sysclock(ch, input);
			// cancel and re-register event
			cancel_event(this, counter[ch].sysclock_id);
			counter[ch].input -= passed;
			counter[ch].period -= passed;
			counter[ch].prev = current_clock();
			register_event_by_clock(this, EVENT_TIMER + ch, counter[ch].period, false, &counter[ch].sysclock_id);
		}
	}
	return counter[ch].count & 0xff;
}

void Z80CTC::event_callback(int event_id, int err)
{
	int ch = event_id & 3;
	if(event_id & 4) {
		input_sysclock(ch, counter[ch].input);
		counter[ch].sysclock_id = -1;
	} else {
		input_clock(ch, counter[ch].input);
		counter[ch].clock_id = -1;
	}
	update_event(ch, err);
}

void Z80CTC::write_signal(int id, uint32 data, uint32 mask)
{
	int ch = id & 3;
#if 1
	if(data & mask) {
		input_clock(ch, 1);
		update_event(ch, 0);
	}
#else
	// more correct implements...
	bool next = ((data & mask) != 0);
	if(counter[ch].prev_in != next) {
		if(counter[ch].slope == next) {
			input_clock(ch, 1);
			update_event(ch, 0);
		}
		counter[ch].prev_in = next;
	}
#endif
}

void Z80CTC::input_clock(int ch, int clock)
{
	if(!(counter[ch].control & 0x40)) {
		// now timer mode, start timer and quit !!!
		counter[ch].start = true;
		return;
	}
	if(counter[ch].freeze) {
		return;
	}
	
	// update counter
	counter[ch].count -= clock;
	while(counter[ch].count <= 0) {
		counter[ch].count += counter[ch].constant;
		if(counter[ch].control & 0x80) {
			counter[ch].req_intr = true;
			update_intr();
		}
		write_signals(&counter[ch].outputs, 0xffffffff);
		write_signals(&counter[ch].outputs, 0);
	}
}

void Z80CTC::input_sysclock(int ch, int clock)
{
	if(counter[ch].control & 0x40) {
		// now counter mode, quit !!!
		return;
	}
	if(!counter[ch].start || counter[ch].freeze) {
		return;
	}
	counter[ch].clocks += clock;
	int input = counter[ch].clocks >> (counter[ch].prescaler == 256 ? 8 : 4);
	counter[ch].clocks &= counter[ch].prescaler - 1;
	
	// update counter
	counter[ch].count -= input;
	while(counter[ch].count <= 0) {
		counter[ch].count += counter[ch].constant;
		if(counter[ch].control & 0x80) {
			counter[ch].req_intr = true;
			update_intr();
		}
		write_signals(&counter[ch].outputs, 0xffffffff);
		write_signals(&counter[ch].outputs, 0);
	}
}

void Z80CTC::update_event(int ch, int err)
{
	if(counter[ch].control & 0x40) {
		// counter mode
		if(counter[ch].sysclock_id != -1) {
			cancel_event(this, counter[ch].sysclock_id);
		}
		counter[ch].sysclock_id = -1;
		
		if(counter[ch].freeze) {
			if(counter[ch].clock_id != -1) {
				cancel_event(this, counter[ch].clock_id);
			}
			counter[ch].clock_id = -1;
			return;
		}
		if(counter[ch].clock_id == -1 && counter[ch].freq) {
			counter[ch].input = counter[ch].count;
			counter[ch].period = (uint32)(cpu_clocks * counter[ch].input / counter[ch].freq) + err;
			counter[ch].prev = current_clock() + err;
			register_event_by_clock(this, EVENT_COUNTER + ch, counter[ch].period, false, &counter[ch].clock_id);
		}
	} else {
		// timer mode
		if(counter[ch].clock_id != -1) {
			cancel_event(this, counter[ch].clock_id);
		}
		counter[ch].clock_id = -1;
		
		if(!counter[ch].start || counter[ch].freeze) {
			if(counter[ch].sysclock_id != -1) {
				cancel_event(this, counter[ch].sysclock_id);
			}
			counter[ch].sysclock_id = -1;
			return;
		}
		if(counter[ch].sysclock_id == -1) {
			counter[ch].input = counter[ch].count * counter[ch].prescaler - counter[ch].clocks;
#ifdef Z80CTC_CLOCKS
			counter[ch].period = (uint32)(counter[ch].input * cpu_clocks / Z80CTC_CLOCKS) + err;
#else
			counter[ch].period = counter[ch].input + err;
#endif
			counter[ch].prev = current_clock() + err;
			register_event_by_clock(this, EVENT_TIMER + ch, counter[ch].period, false, &counter[ch].sysclock_id);
		}
	}
}

void Z80CTC::set_intr_iei(bool val)
{
	if(iei != val) {
		iei = val;
		update_intr();
	}
}

#define set_intr_oei(val) { \
	if(oei != val) { \
		oei = val; \
		if(d_child) { \
			d_child->set_intr_iei(oei); \
		} \
	} \
}

void Z80CTC::update_intr()
{
	bool next;
	
	// set oei signal
	if((next = iei) == true) {
		for(int ch = 0; ch < 4; ch++) {
			if(counter[ch].in_service) {
				next = false;
				break;
			}
		}
	}
	set_intr_oei(next);
	
	// set int signal
	if((next = iei) == true) {
		next = false;
		for(int ch = 0; ch < 4; ch++) {
			if(counter[ch].in_service) {
				break;
			}
			if(counter[ch].req_intr) {
				next = true;
				break;
			}
		}
	}
	if(d_cpu) {
		d_cpu->set_intr_line(next, true, intr_bit);
	}
}

uint32 Z80CTC::intr_ack()
{
	// ack (M1=IORQ=L)
	for(int ch = 0; ch < 4; ch++) {
		if(counter[ch].in_service) {
			// invalid interrupt status
			return 0xff;
		} else if(counter[ch].req_intr) {
			counter[ch].req_intr = false;
			counter[ch].in_service = true;
			update_intr();
			return counter[ch].vector;
		}
	}
	if(d_child) {
		return d_child->intr_ack();
	}
	return 0xff;
}

void Z80CTC::intr_reti()
{
	// detect RETI
	for(int ch = 0; ch < 4; ch++) {
		if(counter[ch].in_service) {
			counter[ch].in_service = false;
			counter[ch].req_intr = false; // ???
			update_intr();
			return;
		}
	}
	if(d_child) {
		d_child->intr_reti();
	}
}

#define STATE_VERSION	1

void Z80CTC::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	for(int i = 0; i < 4; i++) {
		state_fio->FputUint8(counter[i].control);
		state_fio->FputBool(counter[i].slope);
		state_fio->FputUint16(counter[i].count);
		state_fio->FputUint16(counter[i].constant);
		state_fio->FputUint8(counter[i].vector);
		state_fio->FputInt32(counter[i].clocks);
		state_fio->FputInt32(counter[i].prescaler);
		state_fio->FputBool(counter[i].freeze);
		state_fio->FputBool(counter[i].start);
		state_fio->FputBool(counter[i].latch);
		state_fio->FputBool(counter[i].prev_in);
		state_fio->FputBool(counter[i].first_constant);
		state_fio->FputUint64(counter[i].freq);
		state_fio->FputInt32(counter[i].clock_id);
		state_fio->FputInt32(counter[i].sysclock_id);
		state_fio->FputUint32(counter[i].input);
		state_fio->FputUint32(counter[i].period);
		state_fio->FputUint32(counter[i].prev);
		state_fio->FputBool(counter[i].req_intr);
		state_fio->FputBool(counter[i].in_service);
	}
	state_fio->FputUint64(cpu_clocks);
	state_fio->FputBool(iei);
	state_fio->FputBool(oei);
	state_fio->FputUint32(intr_bit);
}

bool Z80CTC::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	for(int i = 0; i < 4; i++) {
		counter[i].control = state_fio->FgetUint8();
		counter[i].slope = state_fio->FgetBool();
		counter[i].count = state_fio->FgetUint16();
		counter[i].constant = state_fio->FgetUint16();
		counter[i].vector = state_fio->FgetUint8();
		counter[i].clocks = state_fio->FgetInt32();
		counter[i].prescaler = state_fio->FgetInt32();
		counter[i].freeze = state_fio->FgetBool();
		counter[i].start = state_fio->FgetBool();
		counter[i].latch = state_fio->FgetBool();
		counter[i].prev_in = state_fio->FgetBool();
		counter[i].first_constant = state_fio->FgetBool();
		counter[i].freq = state_fio->FgetUint64();
		counter[i].clock_id = state_fio->FgetInt32();
		counter[i].sysclock_id = state_fio->FgetInt32();
		counter[i].input = state_fio->FgetUint32();
		counter[i].period = state_fio->FgetUint32();
		counter[i].prev = state_fio->FgetUint32();
		counter[i].req_intr = state_fio->FgetBool();
		counter[i].in_service = state_fio->FgetBool();
	}
	cpu_clocks = state_fio->FgetUint64();
	iei = state_fio->FgetBool();
	oei = state_fio->FgetBool();
	intr_bit = state_fio->FgetUint32();
	return true;
}

