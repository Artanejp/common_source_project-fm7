/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

	[ rtc ]
*/

#include "rtc.h"
#include "../i8259.h"

#define EVENT_1HZ	0
#define EVENT_32HZ	1
#define EVENT_DONE	2

#define POWON	8
#define TCNT	34
#define CKHM	35
#define CKL	36
#define POWOF	36
#define POFMI	37
#define POFH	38
#define POFD	39

void RTC::initialize()
{
	// load rtc regs image
	memset(regs, 0, sizeof(regs));
	regs[POWON] = 0x10;	// cleared
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("RTC.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(regs + 8, 32, 1);
		fio->Fclose();
	}
	delete fio;
	
	// init registers
//	regs[POWON] &= 0x1f;	// local power on
//	regs[POWOF] = 0x80;	// program power off
	regs[POWON] = 0x10;	// cleared
	regs[POWOF] = 0x20;	// illegal power off
	regs[TCNT] = 0;
	update_checksum();
	
	rtcmr = rtdsr = 0;
	
	// update calendar
	get_host_time(&cur_time);
	read_from_cur_time();
	
	// register event
	register_event_by_clock(this, EVENT_1HZ, CPU_CLOCKS, true, &register_id);
	register_event_by_clock(this, EVENT_32HZ, CPU_CLOCKS >> 5, true, NULL);
}

void RTC::release()
{
	// set power off time
	regs[POFMI] = TO_BCD(cur_time.minute);
	regs[POFH] = TO_BCD(cur_time.hour);
	regs[POFD] = TO_BCD(cur_time.day);
	
	// save rtc regs image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("RTC.BIN")), FILEIO_WRITE_BINARY)) {
		fio->Fwrite(regs + 8, 32, 1);
		fio->Fclose();
	}
	delete fio;
}

void RTC::write_io16(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0:
		rtcmr = data;
		break;
	case 2:
		// echo reset
		rtdsr &= ~(data & 0xe);
		update_intr();
		break;
	case 4:
		if(!(rtdsr & 1)) {
			rtadr = data;
			rtdsr |= 1;
			// register event
			register_event(this, EVENT_DONE, 100, false, NULL);
		}
		break;
	case 6:
		rtobr = data;
		break;
	}
}

uint32_t RTC::read_io16(uint32_t addr)
{
	switch(addr) {
	case 2:
		return rtdsr;
	case 6:
		return rtibr;
	}
	return 0xffff;
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
		read_from_cur_time();
		
		// 1sec interrupt
		rtdsr |= 4;
		update_intr();
	} else if(event_id == EVENT_32HZ) {
		// update tcnt
		regs[TCNT]++;
	} else if(event_id == EVENT_DONE) {
		int ch = (rtadr >> 1) & 0x3f;
		if(rtadr & 1) {
			// invalid address
		} else if(rtadr & 0x80) {
			// write
			if(ch <= 6) {
				regs[ch] = (uint8_t)rtobr;
				write_to_cur_time();
			} else if(ch == POWON) {
				regs[ch] = (regs[ch] & 0xe0) | (rtobr & 0x1f);
				if((rtobr & 0xe0) == 0xc0) {
					// reipl
					regs[ch] = (regs[ch] & 0x1f) | 0xc0;
					vm->reset();
				} else if((rtobr & 0xe0) == 0xe0) {
					// power off
					emu->power_off();
				}
				update_checksum();
			} else if(7 <= ch && ch < 32) {
				regs[ch] = (uint8_t)rtobr;
				update_checksum();
			}
		} else {
			// read
			if(ch < 40) {
				rtibr = regs[ch];
			}
		}
		// update flags
		rtdsr &= ~1;
		rtdsr |= 2;
		update_intr();
	}
}

void RTC::read_from_cur_time()
{
	regs[0] = TO_BCD(cur_time.second);
	regs[1] = TO_BCD(cur_time.minute);
	regs[2] = TO_BCD(cur_time.hour);
	regs[3] = cur_time.day_of_week;
	regs[4] = TO_BCD(cur_time.day);
	regs[5] = TO_BCD(cur_time.month);
	regs[6] = TO_BCD(cur_time.year);
}

void RTC::write_to_cur_time()
{
	cur_time.second = FROM_BCD(regs[0]);
	cur_time.minute = FROM_BCD(regs[1]);
	cur_time.hour = FROM_BCD(regs[2]);
//	cur_time.day_of_week = regs[3];
	cur_time.day = FROM_BCD(regs[4]);
	cur_time.month = FROM_BCD(regs[5]);
	cur_time.year = FROM_BCD(regs[6]);
	cur_time.update_year();
	cur_time.update_day_of_week();
	
	// restart event
	cancel_event(this, register_id);
	register_event_by_clock(this, EVENT_1HZ, CPU_CLOCKS, true, &register_id);
}

void RTC::update_checksum()
{
	int sum = 0;
	for(int i = 8; i < 32; i++) {
		sum += regs[i] & 0xf;
		sum += (regs[i] >> 4) & 0xf;
	}
	uint8_t ckh = (sum >> 6) & 0xf;
	uint8_t ckm = (sum >> 2) & 0xf;
	uint8_t ckl = (sum >> 0) & 3;
	
	regs[CKHM] = ckh | (ckm << 4);
	regs[CKL] = (regs[CKL] & 0xf0) | ckl | 0xc;
}

void RTC::update_intr()
{
	d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR1, (rtcmr & rtdsr & 0xe) ? 1 : 0, 1);
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
	state_fio->StateValue(register_id);
	state_fio->StateValue(rtcmr);
	state_fio->StateValue(rtdsr);
	state_fio->StateValue(rtadr);
	state_fio->StateValue(rtobr);
	state_fio->StateValue(rtibr);
	state_fio->StateArray(regs, sizeof(regs), 1);
	return true;
}

