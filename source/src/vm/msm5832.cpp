/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.05.02-

	[ MSM58321/MSM5832 ]
*/

#include "vm.h"
//#include "../emu.h"
#include "msm58321.h"

#define EVENT_BUSY	0
#define EVENT_INC	1
#define EVENT_PULSE	2

#if defined(Q_OS_WIN)
DLL_PREFIX_I struct cur_time_s cur_time;
#endif

MSM5832::MSM5832(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : MSM58321_BASE(parent_vm, parent_emu)
{
		set_device_name(_T("MSM5832 RTC"));
}

void MSM5832::initialize()
{
	DEVICE::initialize();
	// init rtc
	memset(regs, 0, sizeof(regs));
	regs[5] = 8; // 24h
	regs[15] = 0x0f;
	wreg = regnum = 0;
	cs = true;
	rd = wr = addr_wr = busy = hold = false;
	count_1024hz = count_1s = count_1m = count_1h = 0;
	
	get_host_time(&cur_time);
	read_from_cur_time();
	
	// register events
#ifdef MSM58321_START_DAY
	start_day = MSM58321_START_DAY;
#endif
#ifdef MSM58321_START_YEAR
	start_year = MSM58321_START_YEAR;
#endif
	register_event(this, EVENT_INC, 1000000.0, true, &register_id);
	register_event(this, EVENT_PULSE, 1000000.0 / 8192.0, true, NULL);	// 122.1 usec
}


void MSM5832::read_from_cur_time()
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
	regs[ 7] = TO_BCD_LO(cur_time.day - start_day);
	regs[ 8] = TO_BCD_HI(cur_time.day - start_day) | (regs[8] & 0x0c);
	regs[ 9] = TO_BCD_LO(cur_time.month);
	regs[10] = TO_BCD_HI(cur_time.month);
	regs[11] = TO_BCD_LO(cur_time.year - start_year);
	regs[12] = TO_BCD_HI(cur_time.year - start_year);
}

void MSM5832::write_to_cur_time()
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
	cur_time.day += start_day;
	cur_time.month = regs[9] + (regs[10] & 1) * 10;
	cur_time.year = regs[11] + regs[12] * 10;
	cur_time.year += start_year;
	cur_time.update_year();
	cur_time.update_day_of_week();
	
	// restart event
	cancel_event(this, register_id);
	register_event(this, EVENT_INC, 1000000.0, true, &register_id);
}

void MSM5832::set_busy(bool val)
{
	busy = val;
}
