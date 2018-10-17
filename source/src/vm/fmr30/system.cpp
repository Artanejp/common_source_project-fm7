/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

	[ system ]
*/

#include "system.h"

void SYSTEM::initialize()
{
	arr = nmistat = 0;
	nmimask = 0xf0;
}

void SYSTEM::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0x46:
		nmistat &= ~data;
		break;
	case 0x47:
		nmimask = data;
		break;
	case 0xff00:
		arr = data;
		break;
	}
}

uint32_t SYSTEM::read_io8(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0x18:
		// modem:	no
		// scsi:	yes
		// fd23:	3.5inch
		// ext-ram:	yes (1mb)
		// co-pro	no
		return 0x24;
	case 0x20:
	case 0x21:
		// isrr high/low
		return 0;
	case 0x46:
		return nmistat;
	case 0x47:
		return nmimask;
	case 0xff00:
		return arr;
	}
	return 0xff;
}

#define STATE_VERSION	1

#include "../../statesub.h"

void SYSTEM::decl_state()
{
	enter_decl_state(STATE_VERSION);
	
	DECL_STATE_ENTRY_UINT8(arr);
	DECL_STATE_ENTRY_UINT8(nmistat);
	DECL_STATE_ENTRY_UINT8(nmimask);

	leave_decl_state();
}

void SYSTEM::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}

//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputUint8(arr);
//	state_fio->FputUint8(nmistat);
//	state_fio->FputUint8(nmimask);
}

bool SYSTEM::load_state(FILEIO* state_fio)
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
//	arr = state_fio->FgetUint8();
//	nmistat = state_fio->FgetUint8();
//	nmimask = state_fio->FgetUint8();
	return true;
}

bool SYSTEM::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateUint8(arr);
	state_fio->StateUint8(nmistat);
	state_fio->StateUint8(nmimask);
	return true;
}
