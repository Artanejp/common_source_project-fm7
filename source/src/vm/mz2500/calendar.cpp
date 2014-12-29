/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.01 -

	[ calendar ]
*/

#include "calendar.h"

void CALENDAR::write_io8(uint32 addr, uint32 data)
{
	d_rtc->write_io8(addr >> 8, data);
}

uint32 CALENDAR::read_io8(uint32 addr)
{
	return d_rtc->read_io8(addr >> 8);
}

