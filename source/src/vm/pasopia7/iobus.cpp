/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ 8bit i/o bus ]
*/

#include "iobus.h"

void IOBUS::reset()
{
	mio = false;
}

void IOBUS::write_io8(uint32_t addr, uint32_t data)
{
	if(mio) {
		mio = false;
		ram[addr & 0xffff] = data;
	} else {
		d_io->write_io8(addr, data);
	}
}

uint32_t IOBUS::read_io8(uint32_t addr)
{
	if(mio) {
		mio = false;
		return ram[addr & 0xffff];
	} else {
		return d_io->read_io8(addr);
	}
}

void IOBUS::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_IOBUS_MIO) {
		mio = ((data & mask) != 0);
	}
}

#define STATE_VERSION	1

bool IOBUS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(mio);
	return true;
}

