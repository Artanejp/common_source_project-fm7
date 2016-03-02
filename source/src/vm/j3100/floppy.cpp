/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

	[ diskette i/f ]
*/

#include "floppy.h"
#include "../disk.h"
#include "../upd765a.h"

void FLOPPY::initialize()
{
	ctrl_reg = 0;
}

void FLOPPY::reset()
{
	for(int i = 0; i < 4; i++) {
		d_fdc->set_drive_type(i, DRIVE_TYPE_2HD);
	}
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x3f2:
		// bit0-1: select drive
		d_fdc->write_signal(SIG_UPD765A_DRVSEL, data, 3);
		// bit3: reset fdc
		if((ctrl_reg & 4) && !(data & 4)) {
			d_fdc->reset();
		}
		// bit3: enable irq/drq
		d_fdc->write_signal(SIG_UPD765A_IRQ_MASK, !data, 8);
		d_fdc->write_signal(SIG_UPD765A_DRQ_MASK, !data, 8);
		// bit4: fdd #a motor
		// bit5: fdd #b motor
		ctrl_reg = data;
		break;
	case 0x3f7:
		switch(data & 3) {
		case 0:	// 500kbps (2hd)
			d_fdc->set_drive_type(ctrl_reg & 3, DRIVE_TYPE_2HD);
			break;
		case 1:	// 300kbps (2dd)
		case 2:	// 250kbps (2dd)
			d_fdc->set_drive_type(ctrl_reg & 3, DRIVE_TYPE_2DD);
			break;
		}
		break;
	}
}

uint32_t FLOPPY::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x3f7:
		// bit7: 1=ejected
		return d_fdc->disk_ejected(ctrl_reg & 3) ? 0x8f : 0x0f;
	}
	return 0xff;
}

