/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'

	Author : Takeda.Toshiya
	Date   : 2012.03.01 -

	[ floppy ]
*/

#include "floppy.h"
#include "../upd765a.h"

void FLOPPY::initialize()
{
	intr = false;
}

void FLOPPY::write_io8(uint32 addr, uint32 data)
{
	if(!supported) {
		// OA-BASIC without floppy drives
		return;
	}
	
	switch(addr & 0xff) {
	case 0xe0:
		// tc off
		d_fdc->write_signal(SIG_UPD765A_TC, 0, 1);
		break;
	case 0xe2:
		// tc on
		d_fdc->write_signal(SIG_UPD765A_TC, 1, 1);
		break;
	case 0xe4:
	case 0xe5:
		d_fdc->write_io8(addr, data);
		break;
	case 0xe6:
		// fdc reset
		if(data & 0x80) {
			d_fdc->reset();
		}
		// motor on/off
		d_fdc->write_signal(SIG_UPD765A_MOTOR, data, 0x40);
		break;
	}
}

uint32 FLOPPY::read_io8(uint32 addr)
{
	if(!supported) {
		// OA-BASIC without floppy drives
		return 0xff;
	}
	
	switch(addr & 0xff) {
	case 0xe4:
	case 0xe5:
		return d_fdc->read_io8(addr);
	case 0xe6:
		// fdc intr
		return intr ? 0x80 : 0;
	}
	return 0xff;
}

void FLOPPY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_FLOPPY_INTR) {
		intr = ((data & mask) != 0);
	}
}

