/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2021.02.14-

	[ floppy ]
*/

#include "floppy.h"
#include "../mc6843.h"

uint32_t FLOPPY::read_io8(uint32_t addr)
{
	uint32_t value = 0xff;
	
	switch(addr & 0xffff) {
	case 0xe189:
		value  = d_fdc->is_disk_inserted(0) && d_fdc->is_disk_protected(0) ? 1 : 0;
		value |= d_fdc->is_disk_inserted(1) && d_fdc->is_disk_protected(1) ? 2 : 0;
		break;
	default:
		break;
	}
	return value;
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0xe18a:
		// drive reg
		switch(data & 0x0f) {
		case 0x01:
			d_fdc->write_signal(SIG_MC6843_DRIVEREG, 0, 3);
			break;
		case 0x02:
			d_fdc->write_signal(SIG_MC6843_DRIVEREG, 1, 3);
			break;
		case 0x04:
			d_fdc->write_signal(SIG_MC6843_DRIVEREG, 2, 3);
			break;
		case 0x08:
//			d_fdc->write_signal(SIG_MC6843_DRIVEREG, 3, 3);
			break;
		}
		break;
	case 0xe18c:
		// ??? only clear
		break;
	default:
		break;
	}
}
