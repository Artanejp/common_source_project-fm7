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

#include "../../statesub.h"

void IOBUS::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_BOOL(mio);

	leave_decl_state();
}

void IOBUS::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputBool(mio);
}

bool IOBUS::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) return false;
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	mio = state_fio->FgetBool();
	return true;
}

