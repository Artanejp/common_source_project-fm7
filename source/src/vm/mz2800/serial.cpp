/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2015.01.17 -

	[ serial ]
*/

#include "serial.h"
#include "../z80sio.h"

void SERIAL::reset()
{
	addr_a0 = true;
}

void SERIAL::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xa0:
	case 0xa1:
	case 0xa2:
	case 0xa3:
		if(addr_a0) {
			d_sio->write_io8(addr, data);
		}
		break;
	case 0xb0:
	case 0xb1:
	case 0xb2:
	case 0xb3:
		if(!addr_a0) {
			d_sio->write_io8(addr, data);
		}
		break;
	case 0xcd:
		addr_a0 = ((data & 0x80) == 0);
		d_sio->set_tx_clock(0, (4000000.0 / 13.0) / (1 << ((data >> 3) & 7)));
		d_sio->set_rx_clock(0, (4000000.0 / 13.0) / (1 << ((data >> 3) & 7)));
		d_sio->set_tx_clock(1, (4000000.0 / 13.0) / (1 << ((data >> 0) & 7)));
		d_sio->set_rx_clock(1, (4000000.0 / 13.0) / (1 << ((data >> 0) & 7)));
		break;
	}
}

uint32_t SERIAL::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xa0:
	case 0xa1:
	case 0xa2:
	case 0xa3:
		if(addr_a0) {
			return d_sio->read_io8(addr);
		}
		break;
	case 0xb0:
	case 0xb1:
	case 0xb2:
	case 0xb3:
		if(!addr_a0) {
			return d_sio->read_io8(addr);
		}
		break;
	}
	return 0xff;
}

#define STATE_VERSION	1

#include "../../statesub.h"

void SERIAL::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_UINT32(addr_a0);
	
	leave_decl_state();
}

void SERIAL::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
//	
//	state_fio->FputBool(addr_a0);
}

bool SERIAL::load_state(FILEIO* state_fio)
{
	bool mb = false;
	if(state_entry != NULL) {
		mb = state_entry->load_state(state_fio);
	}
	if(!mb) {
		return false;
	}
//	if(state_fio->FgetUint32() != STATE_VERSION) {
//		return false;
//	}
//	if(state_fio->FgetInt32() != this_device_id) {
//		return false;
//	}
//	addr_a0 = state_fio->FgetBool();
	return true;
}

bool SERIAL::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateBool(addr_a0);
	return true;
}
