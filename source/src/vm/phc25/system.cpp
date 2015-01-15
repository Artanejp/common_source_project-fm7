/*
	SANYO PHC-25 Emulator 'ePHC-25'
	SEIKO MAP-1010 Emulator 'eMAP-1010'

	Author : Takeda.Toshiya
	Date   : 2010.08.03-

	[ system port ]
*/

#include "system.h"
#include "../datarec.h"
#include "../mc6847.h"
#include "../../fileio.h"

void SYSTEM::initialize()
{
	sysport = 0;
}

void SYSTEM::reset()
{
	d_vdp->write_signal(SIG_MC6847_INTEXT, 1, 1);
}

void SYSTEM::write_io8(uint32 addr, uint32 data)
{
	d_drec->write_signal(SIG_DATAREC_OUT, data, 0x01);
	d_drec->write_signal(SIG_DATAREC_REMOTE, ~data, 0x02);
	// bit2 : kana lock led ???
	// bit3 : printer strobe
	d_vdp->write_signal(SIG_MC6847_GM, (data & 0x20) ? 7 : 6, 7);
	d_vdp->write_signal(SIG_MC6847_CSS, data, 0x40);
	d_vdp->write_signal(SIG_MC6847_AG, data, 0x80);
}

uint32 SYSTEM::read_io8(uint32 addr)
{
	return sysport;
}

void SYSTEM::write_signal(int id, uint32 data, uint32 mask)
{
	sysport = (sysport & ~mask) | (data & mask);
}

#define STATE_VERSION	1

void SYSTEM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint8(sysport);
}

bool SYSTEM::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	sysport = state_fio->FgetUint8();
	return true;
}

