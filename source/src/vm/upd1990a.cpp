/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.19-

	[ uPD1990A / uPD4990A ]
*/

#include "upd1990a.h"

#define EVENT_1SEC	0
#define EVENT_TP	1

void UPD1990A::initialize()
{
	// initialize rtc
	get_host_time(&cur_time);
	
	// register events
	register_event(this, EVENT_1SEC, 1000000.0, true, &register_id_1sec);
	register_id_tp = -1;
}

void UPD1990A::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_UPD1990A_CLK) {
		bool next = ((data & mask) != 0);
		if(!clk && next) {
			if((mode & 0x0f) == 1) {
				uint64_t bit = 1;
#ifdef HAS_UPD4990A
				if(mode & 0x80) {
					bit <<= (52 - 1);
				} else {
#endif
					bit <<= (40 - 1);
#ifdef HAS_UPD4990A
				}
#endif
				shift_data >>= 1;
				if(din) {
					shift_data |= bit;
				} else {
					shift_data &= ~bit;
				}
				// output LSB
				dout = (uint32_t)(shift_data & 1);
				dout_changed = true;
				write_signals(&outputs_dout, (shift_data & 1) ? 0xffffffff : 0);
			}
#ifdef HAS_UPD4990A
			shift_cmd = (shift_cmd >> 1) | (din ? 8 : 0);
#endif
		}
		clk = next;
	} else if(id == SIG_UPD1990A_STB) {
		bool next = ((data & mask) != 0);
		if(!stb && next/* && !clk*/) {
#ifdef HAS_UPD4990A
			if(cmd == 7) {
				mode = shift_cmd | 0x80;
			} else {
#endif
				mode = cmd;
#ifdef HAS_UPD4990A
			}
#endif
			switch(mode & 0x0f) {
			case 0x02:
				{
					uint64_t tmp = shift_data;
					cur_time.second = FROM_BCD(tmp);
					tmp >>= 8;
					cur_time.minute = FROM_BCD(tmp);
					tmp >>= 8;
					cur_time.hour = FROM_BCD(tmp);
					tmp >>= 8;
					cur_time.day = FROM_BCD(tmp);
					tmp >>= 8;
					cur_time.day_of_week = tmp & 0x0f;
					tmp >>= 4;
					cur_time.month = tmp & 0x0f;
#ifdef HAS_UPD4990A
					if(mode & 0x80) {
						tmp >>= 4;
						cur_time.year = FROM_BCD(tmp);
						cur_time.update_year();
						cur_time.update_day_of_week();
					}
#endif
				}
				hold = true;
				break;
			case 0x03:
				// after all bits are read, lsb of second data can be read
				shift_data = TO_BCD(cur_time.second);
				//shift_data = 0;
#ifdef HAS_UPD4990A
				if(mode & 0x80) {
					shift_data <<= 8;
					shift_data |= TO_BCD(cur_time.year);
				}
#endif
				shift_data <<= 4;
				shift_data |= cur_time.month;
				shift_data <<= 4;
				shift_data |= cur_time.day_of_week;
				shift_data <<= 8;
				shift_data |= TO_BCD(cur_time.day);
				shift_data <<= 8;
				shift_data |= TO_BCD(cur_time.hour);
				shift_data <<= 8;
				shift_data |= TO_BCD(cur_time.minute);
				shift_data <<= 8;
				shift_data |= TO_BCD(cur_time.second);
				// output LSB
				dout = (uint32_t)(shift_data & 1);
				dout_changed = true;
				write_signals(&outputs_dout, (shift_data & 1) ? 0xffffffff : 0);
				break;
			case 0x04:
			case 0x05:
			case 0x06:
#ifdef HAS_UPD4990A
			case 0x07:
			case 0x08:
			case 0x09:
			case 0x0a:
			case 0x0b:
#endif
				if(tpmode != (mode & 0x0f)) {
					if(outputs_tp.count != 0) {
						static const double tbl[] = {
							1000000.0 / 128.0,	// 64Hz
							1000000.0 / 512.0,	// 256Hz
							1000000.0 / 2048.0,	// 2048Hz
#ifdef HAS_UPD4990A
							1000000.0 / 4098.0,	// 4096Hz
							1000000.0,		// 1sec
							1000000.0 * 10,		// 10sec
							1000000.0 * 30,		// 30sec
							1000000.0 * 60		// 60sec
#endif
						};
						if(register_id_tp != -1) {
							cancel_event(this, register_id_tp);
							register_id_tp = -1;
						}
						register_event(this, EVENT_TP, tbl[(mode & 0x0f) - 4], true, &register_id_tp);
					}
					tpmode = mode & 0x0f;
				}
				break;
			}
			// reset counter hold
			switch(mode & 0x0f) {
			case 0x00:
			case 0x01:
			case 0x03:
				if(hold) {
					// restart event
					cancel_event(this, register_id_1sec);
					register_event(this, EVENT_1SEC, 1000000.0, true, &register_id_1sec);
					hold = false;
				}
				break;
			}
		}
		stb = next;
	} else if(id == SIG_UPD1990A_CMD) {
		cmd = (cmd & ~mask) | (data & mask);
		cmd &= 7;
	} else if(id == SIG_UPD1990A_C0) {
		cmd = (cmd & ~1) | (data & mask ? 1 : 0);
		cmd &= 7;
	} else if(id == SIG_UPD1990A_C1) {
		cmd = (cmd & ~2) | (data & mask ? 2 : 0);
		cmd &= 7;
	} else if(id == SIG_UPD1990A_C2) {
		cmd = (cmd & ~4) | (data & mask ? 4 : 0);
		cmd &= 7;
	} else if(id == SIG_UPD1990A_DIN) {
		din = ((data & mask) != 0);
	}
}

void UPD1990A::event_callback(int event_id, int err)
{
	if(event_id == EVENT_1SEC) {
		if(cur_time.initialized) {
			if(!hold) {
				cur_time.increment();
			}
			if(dout_changed) {
				dout_changed = false;
			} else {
				dout = cur_time.second & 1;
				write_signals(&outputs_dout, (cur_time.second & 1) ? 0xffffffff : 0);
			}
		} else {
			get_host_time(&cur_time);	// resync
			cur_time.initialized = true;
		}
	} else if(event_id == EVENT_TP) {
		write_signals(&outputs_tp, tp ? 0xffffffff : 0);
		tp = !tp;
	}
}

#define STATE_VERSION	2

bool UPD1990A::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(!cur_time.process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(register_id_1sec);
	state_fio->StateValue(cmd);
	state_fio->StateValue(mode);
	state_fio->StateValue(tpmode);
	state_fio->StateValue(shift_data);
	state_fio->StateValue(clk);
	state_fio->StateValue(stb);
	state_fio->StateValue(din);
	state_fio->StateValue(hold);
	state_fio->StateValue(tp);
	state_fio->StateValue(dout);
	state_fio->StateValue(dout_changed);
	state_fio->StateValue(register_id_tp);
#ifdef HAS_UPD4990A
	state_fio->StateValue(shift_cmd);
#endif
	return true;
}

