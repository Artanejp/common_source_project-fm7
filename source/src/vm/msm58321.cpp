/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.05.02-

	[ MSM58321/MSM5832 ]
*/

#include "msm58321.h"
#include "../fileio.h"

#define EVENT_BUSY	0
#define EVENT_INC	1
#define EVENT_PULSE	2

#ifndef MSM58321_START_DAY
#define MSM58321_START_DAY 0
#endif
#ifndef MSM58321_START_YEAR
#define MSM58321_START_YEAR 0
#endif

void MSM58321::initialize()
{
	// init rtc
	memset(regs, 0, sizeof(regs));
	regs[5] = 8; // 24h
	regs[15] = 0x0f;
	wreg = regnum = 0;
	cs = true;
	rd = wr = addr_wr = busy = hold = false;
	count_1024hz = count_1s = count_1m = count_1h = 0;
	
	emu->get_host_time(&cur_time);
	read_from_cur_time();
	
	// register events
#ifdef HAS_MSM5832
	register_event(this, EVENT_INC, 1000000.0, true, &register_id);
#else
	register_event(this, EVENT_BUSY, 1000000.0, true, &register_id);
#endif
	register_event(this, EVENT_PULSE, 1000000.0 / 8192.0, true, NULL);	// 122.1 usec
}

void MSM58321::event_callback(int event_id, int err)
{
	if(event_id == EVENT_BUSY) {
		set_busy(true);
		register_event(this, EVENT_INC, 430, false, NULL);
	} else if(event_id == EVENT_INC) {
		if(cur_time.initialized) {
			cur_time.increment();
		} else {
			emu->get_host_time(&cur_time);	// resync
			cur_time.initialized = true;
		}
		if(!hold) {
			read_from_cur_time();
			if(regnum <= 12) {
				output_data();
			}
		}
		set_busy(false);
	} else if(event_id == EVENT_PULSE) {
		if(++count_1024hz == 4) {
			count_1024hz = 0;
			regs[15] ^= 1;
		}
		if(++count_1s = 8192) {
			count_1s = 0;
			regs[15] &= ~2;
		} else {
			regs[15] |= 2;
		}
		if(++count_1m = 60 * 8192) {
			count_1m = 0;
			regs[15] &= ~4;
		} else {
			regs[15] |= 4;
		}
		if(++count_1h = 3600 * 8192) {
			count_1h = 0;
			regs[15] &= ~8;
		} else {
			regs[15] |= 8;
		}
		regs[14] = regs[15];
		if(regnum == 14 || regnum == 15) {
			output_data();
		}
	}
}

void MSM58321::read_from_cur_time()
{
	// update clock
	int hour = (regs[5] & 8) ? cur_time.hour : (cur_time.hour % 12);
	int ampm = (cur_time.hour > 11) ? 4 : 0;
	
	regs[ 0] = TO_BCD_LO(cur_time.second);
	regs[ 1] = TO_BCD_HI(cur_time.second);
	regs[ 2] = TO_BCD_LO(cur_time.minute);
	regs[ 3] = TO_BCD_HI(cur_time.minute);
	regs[ 4] = TO_BCD_LO(hour);
	regs[ 5] = TO_BCD_HI(hour) | ampm | (regs[5] & 8);
	regs[ 6] = cur_time.day_of_week;
	regs[ 7] = TO_BCD_LO(cur_time.day - MSM58321_START_DAY);
	regs[ 8] = TO_BCD_HI(cur_time.day - MSM58321_START_DAY) | (regs[8] & 0x0c);
	regs[ 9] = TO_BCD_LO(cur_time.month);
	regs[10] = TO_BCD_HI(cur_time.month);
	regs[11] = TO_BCD_LO(cur_time.year - MSM58321_START_YEAR);
	regs[12] = TO_BCD_HI(cur_time.year - MSM58321_START_YEAR);
}

void MSM58321::write_to_cur_time()
{
	cur_time.second = regs[0] + (regs[1] & 7) * 10;
	cur_time.minute = regs[2] + (regs[3] & 7) * 10;
	cur_time.hour = regs[4] + (regs[5] & 3) * 10;
	if(!(regs[5] & 8)) {
		cur_time.hour %= 12;
		if(regs[5] & 4) {
			cur_time.hour += 12;
		}
	}
//	cur_time.day_of_week = regs[6] & 7;
	cur_time.day = regs[7] + (regs[8] & 3) * 10;
	cur_time.day += MSM58321_START_DAY;
	cur_time.month = regs[9] + (regs[10] & 1) * 10;
	cur_time.year = regs[11] + regs[12] * 10;
	cur_time.year += MSM58321_START_YEAR;
	cur_time.update_year();
	cur_time.update_day_of_week();
	
	// restart event
	cancel_event(this, register_id);
#ifdef HAS_MSM5832
	register_event(this, EVENT_INC, 1000000.0, true, &register_id);
#else
	register_event(this, EVENT_BUSY, 1000000.0, true, &register_id);
#endif
}

void MSM58321::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_MSM58321_DATA) {
		wreg = (data & mask) | (wreg & ~mask);
	} else if(id == SIG_MSM58321_CS) {
		bool next = ((data & mask) != 0);
//		if(!cs && next) {
//			if(wr) {
//				regs[regnum] = wreg & 0x0f;
//				if(regnum <= 12) {
//					write_to_cur_time();
//				}
//			}
//			if(addr_wr) {
//				regnum = wreg & 0x0f;
//			}
//		}
		cs = next;
		output_data();
	} else if(id == SIG_MSM58321_READ) {
		rd = ((data & mask) != 0);
		output_data();
	} else if(id == SIG_MSM58321_WRITE) {
		bool next = ((data & mask) != 0);
		if(!wr && next && cs) {
			regs[regnum] = wreg & 0x0f;
			if(regnum <= 12) {
				write_to_cur_time();
			}
		}
		wr = next;
	} else if(id == SIG_MSM58321_ADDR_WRITE) {
		bool next = ((data & mask) != 0);
		if(addr_wr && !next && cs) {
			regnum = wreg & 0x0f;
			output_data();
		}
		addr_wr = next;
	} else if(id == SIG_MSM5832_ADDR) {
		regnum = (data & mask) | (regnum & ~mask);
		output_data();
	} else if(id == SIG_MSM5832_HOLD) {
		hold = ((data & mask) != 0);
	}
}

void MSM58321::output_data()
{
	if(cs && rd) {
		write_signals(&outputs_data, regs[regnum]);
	}
}

void MSM58321::set_busy(bool val)
{
#ifndef HAS_MSM5832
	if(busy != val) {
		write_signals(&outputs_busy, busy ? 0 : 0xffffffff);	// negative
	}
#endif
	busy = val;
}

#define STATE_VERSION	1

void MSM58321::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	cur_time.save_state((void *)state_fio);
	state_fio->FputInt32(register_id);
	state_fio->Fwrite(regs, sizeof(regs), 1);
	state_fio->FputUint8(wreg);
	state_fio->FputUint8(regnum);
	state_fio->FputBool(cs);
	state_fio->FputBool(rd);
	state_fio->FputBool(wr);
	state_fio->FputBool(addr_wr);
	state_fio->FputBool(busy);
	state_fio->FputBool(hold);
	state_fio->FputInt32(count_1024hz);
	state_fio->FputInt32(count_1s);
	state_fio->FputInt32(count_1m);
	state_fio->FputInt32(count_1h);
}

bool MSM58321::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	if(!cur_time.load_state((void *)state_fio)) {
		return false;
	}
	register_id = state_fio->FgetInt32();
	state_fio->Fread(regs, sizeof(regs), 1);
	wreg = state_fio->FgetUint8();
	regnum = state_fio->FgetUint8();
	cs = state_fio->FgetBool();
	rd = state_fio->FgetBool();
	wr = state_fio->FgetBool();
	addr_wr = state_fio->FgetBool();
	busy = state_fio->FgetBool();
	hold = state_fio->FgetBool();
	count_1024hz = state_fio->FgetInt32();
	count_1s = state_fio->FgetInt32();
	count_1m = state_fio->FgetInt32();
	count_1h = state_fio->FgetInt32();
	return true;
}

