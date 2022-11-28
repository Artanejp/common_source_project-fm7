/*
	Systems Formulate BUBCOM80 Emulator 'eBUBCOM80'

	Author : Takeda.Toshiya
	Date   : 2018.05.11-

	[ rtc ]
*/

#include "rtc.h"

#define EVENT_1HZ	0

void RTC::initialize()
{
	get_host_time(&cur_time);
	
	register_event_by_clock(this, EVENT_1HZ, CPU_CLOCKS, true, NULL);
}

void RTC::reset()
{
	tmp_time = cur_time; // ???
	ctrl = 0;
}

#define SET_DECIMAL_HI(t, v) \
	t %= 10; \
	t += (v) * 10
#define SET_DECIMAL_LO(t, v) \
	t /= 10; \
	t *= 10; \
	t += (v)
#define GET_DECIMAL_HI(t) TO_BCD_HI(t)
#define GET_DECIMAL_LO(t) TO_BCD_LO(t)

void RTC::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x3fed:
		if(ctrl == 0x80 && data != 0x80) {
			cur_time = tmp_time;
			cur_time.initialized = true;
		}
		if(data == 0xc0) {
			tmp_time = cur_time;
		}
		ctrl = data;
		break;
	case 0x3fec:
		SET_DECIMAL_HI(tmp_time.year, data & 0x0f);
		tmp_time.update_year();
		tmp_time.update_day_of_week();
		break;
	case 0x3feb:
		SET_DECIMAL_LO(tmp_time.year, data & 0x0f);
		tmp_time.update_day_of_week();
		break;
	case 0x3fea:
		SET_DECIMAL_HI(tmp_time.month, data & 0x01);
		tmp_time.update_day_of_week();
		break;
	case 0x3fe9:
		SET_DECIMAL_LO(tmp_time.month, data & 0x0f);
		tmp_time.update_day_of_week();
		break;
	case 0x3fe8:
		SET_DECIMAL_HI(tmp_time.day, data & 0x03);
		tmp_time.update_day_of_week();
		break;
	case 0x3fe7:
		SET_DECIMAL_LO(tmp_time.day, data & 0x0f);
		tmp_time.update_day_of_week();
		break;
	case 0x3fe6:
//		tmp_time.day_of_week = data & 0x07;
		break;
	case 0x3fe5:
		SET_DECIMAL_HI(tmp_time.hour, data & 0x03);
		break;
	case 0x3fe4:
		SET_DECIMAL_LO(tmp_time.hour, data & 0x0f);
		break;
	case 0x3fe3:
		SET_DECIMAL_HI(tmp_time.minute, data & 0x07);
		break;
	case 0x3fe2:
		SET_DECIMAL_LO(tmp_time.minute, data & 0x0f);
		break;
	case 0x3fe1:
		SET_DECIMAL_HI(tmp_time.second, data & 0x07);
		break;
	case 0x3fe0:
		SET_DECIMAL_LO(tmp_time.second, data & 0x0f);
		break;
	}
}

uint32_t RTC::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x3fed:
		return ctrl;
	case 0x3fec:
		return GET_DECIMAL_HI(tmp_time.year);
	case 0x3feb:
		return GET_DECIMAL_LO(tmp_time.year);
	case 0x3fea:
		return GET_DECIMAL_HI(tmp_time.month);
	case 0x3fe9:
		return GET_DECIMAL_LO(tmp_time.month);
	case 0x3fe8:
		return GET_DECIMAL_HI(tmp_time.day);
	case 0x3fe7:
		return GET_DECIMAL_LO(tmp_time.day);
	case 0x3fe6:
		return tmp_time.day_of_week;
	case 0x3fe5:
		return GET_DECIMAL_HI(tmp_time.hour);
	case 0x3fe4:
		return GET_DECIMAL_LO(tmp_time.hour);
	case 0x3fe3:
		return GET_DECIMAL_HI(tmp_time.minute);
	case 0x3fe2:
		return GET_DECIMAL_LO(tmp_time.minute);
	case 0x3fe1:
		return GET_DECIMAL_HI(tmp_time.second);
	case 0x3fe0:
		return GET_DECIMAL_LO(tmp_time.second);
	}
	return 0xff;
}

void RTC::event_callback(int event_id, int err)
{
	if(event_id == EVENT_1HZ) {
		// update calendar
		if(cur_time.initialized) {
			cur_time.increment();
		} else {
			get_host_time(&cur_time);	// resync
			cur_time.initialized = true;
		}
	}
}

#define STATE_VERSION	1

bool RTC::process_state(FILEIO* state_fio, bool loading)
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
	if(!tmp_time.process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(ctrl);
	return true;
}

