/*
	ASCII MSX2 Emulator 'yaMSX2'

	Author : umaiboux
	Date   : 2014.12.xx-

	modified by Takeda.Toshiya

	[ rtc i/f ]
*/

#include "rtcif.h"

void RTCIF::write_io8(uint32 addr, uint32 data)
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

uint32 RTCIF::read_io8(uint32 addr)
{
	return d_rtc->read_io8(adrs);
}

#define STATE_VERSION	1

void RTCIF::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(adrs);
}

bool RTCIF::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	adrs = state_fio->FgetUint8();
	return true;
}

