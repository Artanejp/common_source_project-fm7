/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.06.11 -

	[ floppy ]
*/

#include "floppy.h"
#include "../upd765a.h"

void FLOPPY::reset()
{
	chgreg = 3;
	ctrlreg = 0x80;
}

void FLOPPY::write_io8(uint32 addr, uint32 data)
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
		chgreg = data;
		break;
	}
}

uint32 FLOPPY::read_io8(uint32 addr)
{
	switch(addr & 0xffff) {
	case 0xc8:
		return d_fdc->read_io8(0);
	case 0xca:
		return d_fdc->read_io8(1);
	case 0xcc:
		return (d_fdc->disk_inserted() ? 0x10 : 0) | 0x64;
	case 0xbe:
		return (chgreg & 0x03) | 0x08;
	}
	return addr & 0xff;
}
