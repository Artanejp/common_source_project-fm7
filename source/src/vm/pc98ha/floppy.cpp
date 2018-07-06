/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.06.11 -

	[ floppy ]
*/

#include "floppy.h"
#include "../disk.h"
#include "../upd765a.h"

void FLOPPY::reset()
{
/*
	for(int i = 0; i < 4; i++) {
		d_fdc->set_drive_type(i, DRIVE_TYPE_2HD);
	}
*/
	ctrlreg = 0x80;
	modereg = 0x03;
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0xca:
		d_fdc->write_io8(1, data);
		break;
	case 0xcc:
		if(!(ctrlreg & 0x80) && (data & 0x80)) {
			d_fdc->reset();
		}
		d_fdc->write_signal(SIG_UPD765A_FREADY, data, 0x40);
		d_fdc->write_signal(SIG_UPD765A_MOTOR, data, 0x08);
		ctrlreg = data;
		break;
	case 0xbe:
/*
		if(!(modereg & 2) && (data & 2)) {
			d_fdc->set_drive_type(0, DRIVE_TYPE_2HD);
			d_fdc->set_drive_type(1, DRIVE_TYPE_2HD);
		} else if((modereg & 2) && !(data & 2)) {
			d_fdc->set_drive_type(0, DRIVE_TYPE_2DD);
			d_fdc->set_drive_type(1, DRIVE_TYPE_2DD);
		}
*/
		modereg = data;
		break;
	}
}

uint32_t FLOPPY::read_io8(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0xc8:
		return d_fdc->read_io8(0);
	case 0xca:
		return d_fdc->read_io8(1);
	case 0xcc:
		return (d_fdc->is_disk_inserted() ? 0x10 : 0) | 0x64;
	case 0xbe:
		return (modereg & 0x03) | 0x08;
	}
	return addr & 0xff;
}

#define STATE_VERSION	1


#include "../../statesub.h"

void FLOPPY::decl_state()
{
	enter_decl_state(STATE_VERSION);

	DECL_STATE_ENTRY_UINT8(ctrlreg);
	DECL_STATE_ENTRY_UINT8(modereg);

	leave_decl_state();
}

void FLOPPY::save_state(FILEIO* state_fio)
{
	if(state_entry != NULL) {
		state_entry->save_state(state_fio);
	}
//	state_fio->FputUint32(STATE_VERSION);
//	state_fio->FputInt32(this_device_id);
	
//	state_fio->FputUint8(ctrlreg);
//	state_fio->FputUint8(modereg);
}

bool FLOPPY::load_state(FILEIO* state_fio)
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
//	ctrlreg = state_fio->FgetUint8();
//	modereg = state_fio->FgetUint8();
	return true;
}

