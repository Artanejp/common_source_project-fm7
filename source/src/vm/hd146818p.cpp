/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.10.11 -

	[ HD146818P ]
*/

#include "hd146818p.h"

#define EVENT_1SEC	0
#define EVENT_SQW	1

// [DV2-DV0][RS3-RS0]
static const int periodic_intr_rate[3][16] = {
	{0,   1,   2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384},	// 4.194304 MHz
	{0,   1,   2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384},	// 1.048576 MHz
	{0, 128, 256, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384}	// 32.768kHz
};

void HD146818P::initialize()
{
	// load ram image
	memset(regs, 0, sizeof(regs));
	modified = false;
	
	// FIXME: we need to consider the multiple chips case
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("HD146818P.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(regs + 14, 50, 1);
		fio->Fclose();
	}
	delete fio;
	
	// initialize
	ch = period = 0;
	intr = sqw = false;
	register_id_sqw = -1;
	
	get_host_time(&cur_time);
	read_from_cur_time();
	
	// register event
	register_event(this, EVENT_1SEC, 1000000, true, &register_id_1sec);
}

void HD146818P::release()
{
	if(modified) {
		// FIXME: we need to consider the multiple chips case
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("HD146818P.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(regs + 14, 50, 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void HD146818P::reset()
{
	regs[0x0b] &= ~0x78;
	regs[0x0c] = 0;
}

void HD146818P::write_io8(uint32_t addr, uint32_t data)
{
	if(addr & 1) {
		ch = data & 0x3f;
	} else {
		if(ch <= 9) {
			regs[ch] = data;
			if(!(ch == 1 || ch == 3 || ch == 5)) {
				write_to_cur_time();
			}
			if(ch <= 5) {
				check_alarm();
				update_intr();
			}
		} else if(ch == 0x0a) {
			// periodic interrupt
			int dv = (data >> 4) & 7, next = 0;
			if(dv < 3) {
				next = periodic_intr_rate[dv][data & 0x0f];
			}
			if(next != period) {
				if(register_id_sqw != -1) {
					cancel_event(this, register_id_sqw);
					register_id_sqw = -1;
				}
				if(next) {
					// raise event twice per one period
					register_event(this, EVENT_SQW, 1000000.0 / 65536.0 * next, true, &register_id_sqw);
				}
				period = next;
			}
			regs[ch] = data & 0x7f;	// always UIP=0
		} else if(ch == 0x0b) {
			if((regs[0x0b] & 8) && !(data & 8)) {
				// keep sqw = L when sqwe = 0
				write_signals(&outputs_sqw, 0);
			}
			bool tmp = (((regs[ch] ^ data) & 4) != 0);
			regs[ch] = data;
			if(tmp) {
				read_from_cur_time();
				check_alarm();
			}
			update_intr();
		} else if(ch > 0x0d) {
			// internal ram
			if(regs[ch] != data) {
				regs[ch] = data;
				modified = true;
			}
		}
	}
}

uint32_t HD146818P::read_io8(uint32_t addr)
{
	if(addr & 1) {
		return 0xff;
	} else {
		uint8_t val = regs[ch];
		if(ch == 0x0c) {
			regs[0x0c] = 0;
			update_intr();
		}
		return val;
	}
}

#define TO_BCD_BIN(v)	((regs[0x0b] & 4) ? (v) : TO_BCD(v))
#define FROM_BCD_BIN(v)	((regs[0x0b] & 4) ? (v) : FROM_BCD(v))

void HD146818P::event_callback(int event_id, int err)
{
	if(event_id == EVENT_1SEC) {
		if(cur_time.initialized) {
			cur_time.increment();
		} else {
			get_host_time(&cur_time);	// resync
			cur_time.initialized = true;
		}
		read_from_cur_time();
		regs[0x0c] |= 0x10; // updated
		check_alarm();
		update_intr();
	} else if(event_id == EVENT_SQW) {
		// periodic interrupt
		if((sqw = !sqw) == true) {
			regs[0x0c] |= 0x40;
			update_intr();
		}
		// square wave
		if(regs[0x0b] & 8) {
			// output sqw when sqwe = 1
			write_signals(&outputs_sqw, sqw ? 0xffffffff : 0);
		}
	}
}

void HD146818P::read_from_cur_time()
{
	int hour = (regs[0x0b] & 2) ? cur_time.hour : (cur_time.hour % 12);
	int ampm = (regs[0x0b] & 2) ? 0 : (cur_time.hour > 11) ? 0x80 : 0;
	
	regs[0] = TO_BCD_BIN(cur_time.second);
	regs[2] = TO_BCD_BIN(cur_time.minute);
	regs[4] = TO_BCD_BIN(hour) | ampm;
	regs[6] = cur_time.day_of_week + 1;
	regs[7] = TO_BCD_BIN(cur_time.day);
	regs[8] = TO_BCD_BIN(cur_time.month);
	regs[9] = TO_BCD_BIN(cur_time.year);
}

void HD146818P::write_to_cur_time()
{
	cur_time.second = FROM_BCD_BIN(regs[0] & 0x7f);
	cur_time.minute = FROM_BCD_BIN(regs[2] & 0x7f);
	if(regs[0x0b] & 2) {
		cur_time.hour = FROM_BCD_BIN(regs[4] & 0x3f);
	} else {
		cur_time.hour = FROM_BCD_BIN(regs[4] & 0x1f);
		if(regs[4] & 0x80) {
			cur_time.hour += 12;
		}
	}
//	cur_time.day_of_week = regs[6] - 1;
	cur_time.day = FROM_BCD_BIN(regs[7]);
	cur_time.month = FROM_BCD_BIN(regs[8]);
	cur_time.year = FROM_BCD_BIN(regs[9]);
	cur_time.update_year();
	cur_time.update_day_of_week();
	
	// restart event
	cancel_event(this, register_id_1sec);
	register_event(this, EVENT_1SEC, 1000000, true, &register_id_1sec);
}

void HD146818P::check_alarm()
{
	if(regs[0] == regs[1] && regs[2] == regs[3] && regs[4] == regs[5]) {
		regs[0x0c] |= 0x20;
	}
}

void HD146818P::update_intr()
{
	bool next = ((regs[0x0b] & regs[0x0c] & 0x70) != 0);
	if(intr != next) {
		write_signals(&outputs_intr, next ? 0xffffffff : 0);
		intr = next;
	}
}

#define STATE_VERSION	1

bool HD146818P::process_state(FILEIO* state_fio, bool loading)
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
	state_fio->StateArray(regs, sizeof(regs), 1);
	state_fio->StateValue(ch);
	state_fio->StateValue(period);
	state_fio->StateValue(register_id_sqw);
	state_fio->StateValue(intr);
	state_fio->StateValue(sqw);
	state_fio->StateValue(modified);
	return true;
}

