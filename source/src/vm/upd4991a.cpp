/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.06.10-

	[ uPD4991A ]
*/

#include "upd4991a.h"

void UPD4991A::initialize()
{
	// initialize rtc
	memset(regs, 0, sizeof(regs));
	ctrl1 = ctrl2 = mode = 0;
	
	get_host_time(&cur_time);
	read_from_cur_time();
	
	// register event
	register_event(this, 0, 1000000.0, true, &register_id);
}

void UPD4991A::write_io8(uint32_t addr, uint32_t data)
{
	addr &= 0x0f;
	if(addr <= 12) {
		if(mode == 0 || mode == 3) {
			if(regs[0][addr] != data) {
				regs[0][addr] = data;
				write_to_cur_time();
			}
		} else if(mode == 1) {
			regs[1][addr] = data;
		} else if(mode == 2) {
			uint8_t tmp = regs[2][addr] ^ data;
			regs[2][addr] = data;
			// am/pm is changed ?
			if(addr == 12 && (tmp & 8)) {
				read_from_cur_time();
			}
		} else {
		}
	} else if(addr == 13) {
		ctrl1 = data;
	} else if(addr == 14) {
		ctrl2 = data;
	} else if(addr == 15) {
		mode = data & 0x0b;
	}
}

uint32_t UPD4991A::read_io8(uint32_t addr)
{
	addr &= 0x0f;
	if(addr <= 12) {
		if(mode == 0 || mode == 3) {
			return regs[0][addr];
		} else if(mode == 1 || mode == 2) {
			return regs[mode][addr];
		}
	} else if(addr == 14) {
		return ctrl2;
	}
	return 0x0f;
}

void UPD4991A::event_callback(int event_id, int err)
{
	// update clock
	if(cur_time.initialized) {
		cur_time.increment();
	} else {
		get_host_time(&cur_time);	// resync
		cur_time.initialized = true;
	}
	
	if(!(ctrl1 & 8)) {
		read_from_cur_time();
	}
}

#define MODE_12H !(regs[2][12] & 8)

void UPD4991A::read_from_cur_time()
{
	int hour = MODE_12H ? (cur_time.hour % 12) : cur_time.hour;
	int ampm = (MODE_12H && cur_time.hour >= 12) ? 4 : 0;
	
	regs[0][ 0] = TO_BCD_LO(cur_time.second);
	regs[0][ 1] = TO_BCD_HI(cur_time.second);
	regs[0][ 2] = TO_BCD_LO(cur_time.minute);
	regs[0][ 3] = TO_BCD_HI(cur_time.minute);
	regs[0][ 4] = TO_BCD_LO(hour);
	regs[0][ 5] = TO_BCD_HI(hour) | ampm;
	regs[0][ 6] = cur_time.day_of_week;
	regs[0][ 7] = TO_BCD_LO(cur_time.day);
	regs[0][ 8] = TO_BCD_HI(cur_time.day);
	regs[0][ 9] = TO_BCD_LO(cur_time.month);
	regs[0][10] = TO_BCD_HI(cur_time.month);
	regs[0][11] = TO_BCD_LO(cur_time.year);
	regs[0][12] = TO_BCD_HI(cur_time.year);
	
	// TODO: check alarm
}

void UPD4991A::write_to_cur_time()
{
	cur_time.second = regs[0][0] + (regs[0][1] & 7) * 10;
	cur_time.minute = regs[0][2] + (regs[0][3] & 7) * 10;
	if(MODE_12H) {
		cur_time.hour = regs[0][4] + (regs[0][5] & 1) * 10 + (regs[0][5] & 4 ? 12 : 0);
	} else {
		cur_time.hour = regs[0][4] + (regs[0][5] & 3) * 10;
	}
//	cur_time.day_of_week = regs[0][6];
	cur_time.day = regs[0][7] + (regs[0][8] & 3) * 10;
	cur_time.month = regs[0][9] + (regs[0][10] & 1) * 10;
	cur_time.year = regs[0][11] + regs[0][12] * 10;
	cur_time.update_year();
	cur_time.update_day_of_week();
	
	// restart event
	cancel_event(this, register_id);
	register_event(this, 0, 1000000.0, true, &register_id);
}

#define STATE_VERSION	1

bool UPD4991A::process_state(FILEIO* state_fio, bool loading)
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
	state_fio->StateValue(register_id);
	state_fio->StateArray(&regs[0][0], sizeof(regs), 1);
	state_fio->StateValue(ctrl1);
	state_fio->StateValue(ctrl2);
	state_fio->StateValue(mode);
	return true;
}

