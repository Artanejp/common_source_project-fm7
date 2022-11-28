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

bool FLOPPY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(ctrlreg);
	state_fio->StateValue(modereg);
	return true;
}

