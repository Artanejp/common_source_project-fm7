/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.18-

	[ fdc pack ]
*/

#include "fdcpack.h"
#include "../upd765a.h"

void FDCPACK::write_io8(uint32_t addr, uint32_t data)
{
	if(addr < 0xff00) {
		switch(addr & 7) {
		case 0:
		case 1:
			d_fdc->write_signal(SIG_UPD765A_MOTOR, 1, 1);
			break;
		case 2:
		case 3:
			d_fdc->write_signal(SIG_UPD765A_TC, 1, 1);
			break;
		case 5:
			// data register
			d_fdc->write_io8(1, data);
			break;
		case 6:
			// data register + dack
			d_fdc->write_dma_io8(1, data);
			break;
		}
	}
}

uint32_t FDCPACK::read_io8(uint32_t addr)
{
	if(addr < 0xff00) {
		switch(addr & 7) {
		case 4:
			// status register
			return d_fdc->read_io8(0);
		case 5:
			// status register
			return d_fdc->read_io8(1);
		case 6:
			// data register + dack
			return d_fdc->read_dma_io8(1);
		}
	} else if(0xff00 <= addr && addr < 0xff80) {
		return 0x04; // device id
	}
	return 0xff;
}
