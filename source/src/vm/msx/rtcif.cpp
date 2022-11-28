/*
	ASCII MSX2 Emulator 'yaMSX2'

	Author : umaiboux
	Date   : 2014.12.xx-

	modified by Takeda.Toshiya

	[ rtc i/f ]
*/

#include "rtcif.h"

void RTCIF::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 1) {
	case 0:
		adrs = data & 0x0f;
		break;
	case 1:
		d_rtc->write_io8(adrs, data & 0x0f);
		break;
	}
}

uint32_t RTCIF::read_io8(uint32_t addr)
{
	return d_rtc->read_io8(adrs);
}

#define STATE_VERSION	1

bool RTCIF::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(adrs);
	return true;
}

