/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.05.02 -

	[ scsi ]
*/

#include "scsi.h"
#include "../../fileio.h"

// phase
#define PHASE_BUSFREE		0
#define PHASE_ARBITRATION	1
#define PHASE_SELECTION		2
#define PHASE_RESELECTION	3
#define PHASE_COMMAND		4
#define PHASE_EXECUTE		5
#define PHASE_MSG_IN		6
#define PHASE_MSG_OUT		7
#define PHASE_DATA_IN		8
#define PHASE_DATA_OUT		9
#define PHASE_STATUS		10

// control register
#define CTRL_WEN	0x80
#define CTRL_IMSK	0x40
#define CTRL_ATN	0x10
#define CTRL_SEL	0x04
#define CTRL_DMAE	0x02
#define CTRL_RST	0x01

// status register
#define STAT_REQ	0x80
#define STAT_IO		0x40
#define STAT_MSG	0x20
#define STAT_CD		0x10
#define STAT_BUSY	0x08
#define STAT_INT	0x02
#define STAT_PERR	0x01

// DMA:	SIG_UPD71071_CH1
// IRQ:	SIG_I8259_CHIP1 | SIG_I8259_IR0

void SCSI::initialize()
{
	phase = PHASE_BUSFREE;
	ctrlreg = datareg = statreg = 0;
}

void SCSI::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xffff) {
	case 0xc30:
		// data register
		datareg = data;
		break;
	case 0xc32:
		// control register
		if((ctrlreg & CTRL_RST) && ~(data & CTRL_RST)) {
			// reset
			statreg = 0;
		}
		if(~(ctrlreg & CTRL_SEL) && (data & CTRL_SEL)) {
			// sel
//			statreg |= 8;
//			datareg = 0x80;
		}
		ctrlreg = data;
		break;
	}
}

uint32 SCSI::read_io8(uint32 addr)
{
//	uint32 val;
	
	switch(addr & 0xffff) {
	case 0xc30:
		// data register
		return 0;
	case 0xc32:
		// status register
		return 0;
//		val = statreg;
//		statreg &= ~8;
//		return val;
	}
	return 0xff;
}

void SCSI::write_dma_io8(uint32 addr, uint32 data)
{
	write_io8(0xc30, data);
}

uint32 SCSI::read_dma_io8(uint32 addr)
{
	return read_io8(0xc30);
}

