/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

	[ floppy ]
*/

#include "floppy.h"
#include "../mb8877.h"
#include "../disk.h"

void FLOPPY::reset()
{
	for(int i = 0; i < 4; i++) {
		d_fdc->set_drive_type(i, DRIVE_TYPE_2HD);
	}
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0x7fff) {
	case 0xdc:
		// drive reg
		d_fdc->write_signal(SIG_MB8877_DRIVEREG, data, 3);
		d_fdc->write_signal(SIG_MB8877_MOTOR, data, 0x80);
		break;
	case 0xdd:
		// side reg
		d_fdc->write_signal(SIG_MB8877_SIDEREG, data, 1);
		break;
	case 0xde:
		break;
	case 0xdf:
		for(int i = 0; i < 4; i++) {
			if(data & 1) {
				d_fdc->set_drive_type(i, DRIVE_TYPE_2HD);
			} else {
				if(d_fdc->get_media_type(i) == MEDIA_TYPE_2DD) {
					d_fdc->set_drive_type(i, DRIVE_TYPE_2DD);
				} else {
					d_fdc->set_drive_type(i, DRIVE_TYPE_2D);
				}
			}
		}
		break;
	}
}

