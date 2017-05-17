/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2017.05.06-

	[ rtc i/f ]
*/

#include "calendar.h"
#include "../msm58321.h"

void CALENDAR::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0xf036:
		d_rtc->write_signal(SIG_MSM5832_ADDR, data, 0x0f);
		break;
	case 0xf037:
		d_rtc->write_signal(SIG_MSM58321_CS, 1, 1);
		d_rtc->write_signal(SIG_MSM58321_WRITE, 0, 0);
		d_rtc->write_signal(SIG_MSM58321_DATA, data, 0xff);
		d_rtc->write_signal(SIG_MSM58321_WRITE, 1, 1);
		d_rtc->write_signal(SIG_MSM58321_CS, 0, 0);
		break;
	}
}

uint32_t CALENDAR::read_io8(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0xf037:
		return d_rtc->read_signal(SIG_MSM58321_DATA);
	}
	return 0xff;
}

