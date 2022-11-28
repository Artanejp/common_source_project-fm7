/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.11-

	[ RP-5C01 / RP-5C15 ]
*/

#include "rp5c01.h"

#define EVENT_1SEC	0
#define EVENT_16HZ	1

void RP5C01::initialize()
{
#ifndef HAS_RP5C15
	// load ram image
	memset(ram, 0, sizeof(ram));
	modified = false;
	
	// FIXME: we need to consider the multiple chips case
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("RP5C01.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	}
	delete fio;
#endif
	
	// initialize rtc
	memset(regs, 0, sizeof(regs));
	regs[0x0a] = 1;
	regs[0x0d] = 8;
	regs[0x0f] = 0xc;
	alarm = pulse_1hz = pulse_16hz = false;
	count_16hz = 0;
	
	get_host_time(&cur_time);
	read_from_cur_time();
	
	// register events
	register_event(this, EVENT_1SEC, 1000000, true, &register_id);
	register_event(this, EVENT_16HZ, 1000000 / 32, true, NULL);
}

void RP5C01::release()
{
#ifndef HAS_RP5C15
	// save ram image
	if(modified) {
		// FIXME: we need to consider the multiple chips case
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("RP5C01.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(ram, sizeof(ram), 1);
			fio->Fclose();
		}
		delete fio;
	}
#endif
}

void RP5C01::write_io8(uint32_t addr, uint32_t data)
{
	addr &= 0x0f;
	if(addr <= 0x0c) {
#ifndef HAS_RP5C15
		switch(regs[0x0d] & 3) {
#else
		switch(regs[0x0d] & 1) {
#endif
		case 0:
			if(time[addr] != data) {
				time[addr] = data;
				write_to_cur_time();
			}
			return;
#ifndef HAS_RP5C15
		case 2:
			if(ram[addr] != data) {
				ram[addr] = data;
				modified = true;
			}
			return;
		case 3:
			if(ram[addr + 13] != data) {
				ram[addr + 13] = data;
				modified = true;
			}
			return;
#endif
		}
	}
	
	uint8_t tmp = regs[addr] ^ data;
	regs[addr] = data;
	
	if(addr == 0x0a) {
		if(tmp & 1) {
			// am/pm is changed
			read_from_cur_time();
		}
	} else if(addr == 0x0f) {
#ifndef HAS_RP5C15
		switch(regs[0x0d] & 3) {
#else
		switch(regs[0x0d] & 1) {
#endif
		case 0:
			if(data & 3) {
				// timer reset
			}
			break;
		case 1:
#ifndef HAS_RP5C15
		case 2:
		case 3:
#endif
			if(data & 2) {
				// timer reset
			}
			if(data & 1) {
				if(alarm) {
					alarm = false;
					update_pulse();
				}
			}
			break;
		}
	}
}

uint32_t RP5C01::read_io8(uint32_t addr)
{
	addr &= 0x0f;
	if(addr <= 0x0c) {
#ifndef HAS_RP5C15
		switch(regs[0x0d] & 3) {
#else
		switch(regs[0x0d] & 1) {
#endif
		case 0:
			return time[addr];
#ifndef HAS_RP5C15
		case 2:
			return ram[addr];
		case 3:
			return ram[addr + 13];
#endif
		}
	}
	if(addr == 0x0b) {
		for(int i = 0; i < 3; i++) {
			if(LEAP_YEAR(cur_time.year - i)) {
				return i;
			}
		}
		return 3;
	}
	return regs[addr];
}

void RP5C01::event_callback(int event_id, int err)
{
	if(event_id == EVENT_1SEC) {
		if(cur_time.initialized) {
			cur_time.increment();
		} else {
			get_host_time(&cur_time);	// resync
			cur_time.initialized = true;
		}
		if(regs[0x0d] & 8) {
			read_from_cur_time();
			if(regs[0x0d] & 4) {
				update_pulse();
			}
		}
	} else if(event_id == EVENT_16HZ) {
		bool update = false;
		// 1Hz
		if(++count_16hz == 16) {
			pulse_1hz = !pulse_1hz;
			if(!(regs[0x0f] & 8)) {
				update = true;
			}
			count_16hz = 0;
		}
		// 16Hz
		pulse_16hz = !pulse_16hz;
		if(!(regs[0x0f] & 4)) {
			update = true;
		}
		if(update) {
			update_pulse();
		}
	}
	
	// update signal
}

void RP5C01::update_pulse()
{
	bool pulse = false;
	
	if(regs[0x0d] & 4) {
		pulse |= alarm;
	}
	if(!(regs[0x0f] & 8)) {
		pulse |= pulse_1hz;
	}
	if(!(regs[0x0f] & 4)) {
		pulse |= pulse_16hz;
	}
	write_signals(&outputs_pulse, pulse ? 0 : 0xffffffff);
}

#define MODE_12H !(regs[0x0a] & 1)

void RP5C01::read_from_cur_time()
{
	int hour = MODE_12H ? (cur_time.hour % 12) : cur_time.hour;
	int ampm = (MODE_12H && cur_time.hour >= 12) ? 2 : 0;
	
	time[ 0] = TO_BCD_LO(cur_time.second);
	time[ 1] = TO_BCD_HI(cur_time.second);
	time[ 2] = TO_BCD_LO(cur_time.minute);
	time[ 3] = TO_BCD_HI(cur_time.minute);
	time[ 4] = TO_BCD_LO(hour);
	time[ 5] = TO_BCD_HI(hour) | ampm;
	time[ 6] = cur_time.day_of_week;
	time[ 7] = TO_BCD_LO(cur_time.day);
	time[ 8] = TO_BCD_HI(cur_time.day);
	time[ 9] = TO_BCD_LO(cur_time.month);
	time[10] = TO_BCD_HI(cur_time.month);
	time[11] = TO_BCD_LO(cur_time.year);
	time[12] = TO_BCD_HI(cur_time.year);
	
	// check alarm
	static const uint8_t mask[9] = {0, 0, 0x0f, 0x07, 0x0f, 0x03, 0x07, 0x0f, 0x03};
	bool tmp = true;
	
	for(int i = 3; i < 9; i++) {
		if((time[i] & mask[i]) != (regs[i] & mask[i])) {
			tmp = false;
			break;
		}
	}
	if(tmp) {
		alarm = true;
	}
}

void RP5C01::write_to_cur_time()
{
	cur_time.second = time[0] + (time[1] & 7) * 10;
	cur_time.minute = time[2] + (time[3] & 7) * 10;
	if(MODE_12H) {
		cur_time.hour = time[4] + (time[5] & 1) * 10 + (time[5] & 2 ? 12 : 0);
	} else {
		cur_time.hour = time[4] + (time[5] & 3) * 10;
	}
//	cur_time.day_of_week = time[6];
	cur_time.day = time[7] + (time[8] & 3) * 10;
	cur_time.month = time[9] + (time[10] & 1) * 10;
	cur_time.year = time[11] + time[12] * 10;
	cur_time.update_year();
	cur_time.update_day_of_week();
	
	// restart events
	cancel_event(this, register_id);
	register_event(this, EVENT_1SEC, 1000000, true, &register_id);
}

#define STATE_VERSION	1

bool RP5C01::process_state(FILEIO* state_fio, bool loading)
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
	state_fio->StateArray(regs, sizeof(regs), 1);
	state_fio->StateArray(time, sizeof(time), 1);
#ifndef HAS_RP5C15
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(modified);
#endif
	state_fio->StateValue(alarm);
	state_fio->StateValue(pulse_1hz);
	state_fio->StateValue(pulse_16hz);
	state_fio->StateValue(count_16hz);
	return true;
}

