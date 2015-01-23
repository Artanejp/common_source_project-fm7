/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ 8bit i/o bus ]
*/

#include "iobus.h"
#include "../../fileio.h"

void IOBUS::write_io8(uint32 addr, uint32 data)
{
	if(mio) {
		mio = false;
		ram[addr & 0xffff] = data;
	} else {
		d_io->write_io8(addr, data);
	}
}

uint32 IOBUS::read_io8(uint32 addr)
{
	if(mio) {
		mio = false;
		return ram[addr & 0xffff];
	} else {
		return d_io->read_io8(addr);
	}
}

void IOBUS::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_IOBUS_MIO) {
		mio = ((data & mask) != 0);
	}
}

#define STATE_VERSION	1

void IOBUS::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputBool(mio);
}

bool IOBUS::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	mio = state_fio->FgetBool();
	return true;
}

