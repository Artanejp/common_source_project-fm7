/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.17 -

	[ floppy ]
*/

#include "floppy.h"
#include "../upd765a.h"

void FLOPPY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0x72:
		// data register + dack
		d_fdc->write_dma_io8(addr, data);
		break;
	case 0x73:
		// motor on/off
		d_fdc->write_signal(SIG_UPD765A_MOTOR, data, 1);
		break;
	case 0x74:
		// tc on
		d_fdc->write_signal(SIG_UPD765A_TC, 1, 1);
		break;
	}
}

uint32 FLOPPY::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
	case 0x72:
		// data register + dack
		return d_fdc->read_dma_io8(addr);
	case 0x73:
		return drq ? 0xff : 0x7f;
	}
	return 0xff;
}

void FLOPPY::write_signal(int id, uint32 data, uint32 mask)
{
	drq = ((data & mask) != 0);
}

