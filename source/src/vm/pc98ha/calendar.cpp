/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.08.14 -

	[ calendar ]
*/

#include "calendar.h"
#ifndef _PC98HA
#include "../upd1990a.h"
#endif

void CALENDAR::initialize()
{
#ifdef _PC98HA
	ch = 0;
#endif
}

void CALENDAR::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
#ifdef _PC98HA
	case 0x22:
		ch = data & 0x0f;
		break;
	case 0x23:
		d_rtc->write_io8(ch, data & 0x0f);
		break;
#else
	case 0x20:
		d_rtc->write_signal(SIG_UPD1990A_CMD, data, 0x07);
		d_rtc->write_signal(SIG_UPD1990A_DIN, data, 0x20);
		d_rtc->write_signal(SIG_UPD1990A_STB, data, 0x08);
		d_rtc->write_signal(SIG_UPD1990A_CLK, data, 0x10);
		break;
#endif
	}
}

uint32_t CALENDAR::read_io8(uint32_t addr)
{
#ifdef _PC98HA
	switch(addr & 0xffff) {
	case 0x23:
		return d_rtc->read_io8(ch) & 0x0f;
	}
#endif
	return 0xff;
}

#ifdef _PC98HA
#define STATE_VERSION	1

bool CALENDAR::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(ch);
	return true;
}
#endif

