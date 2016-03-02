/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.28-

	[ dma bank register ]
*/

#include "dmareg.h"
#include "../i8237.h"

void DMAREG::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
#ifdef TYPE_SL
	case 0x81:
		d_dma->write_signal(SIG_I8237_BANK2, data, 0x0f);
		break;
	case 0x82:
		d_dma->write_signal(SIG_I8237_BANK3, data, 0x0f);
		break;
	case 0x83:
		d_dma->write_signal(SIG_I8237_BANK1, data, 0x0f);
		break;
#else
	case 0x81:
		d_dma->write_signal(SIG_I8237_BANK2, data, 0xff);
		break;
	case 0x82:
		d_dma->write_signal(SIG_I8237_BANK3, data, 0xff);
		break;
	case 0x83:
		d_dma->write_signal(SIG_I8237_BANK1, data, 0xff);
		break;
	case 0x87:
		d_dma->write_signal(SIG_I8237_BANK0, data, 0xff);
		break;
	case 0x89:
		d_dma2->write_signal(SIG_I8237_BANK2, data >> 1, 0x7f);
		break;
	case 0x8a:
		d_dma2->write_signal(SIG_I8237_BANK3, data >> 1, 0x7f);
		break;
	case 0x8b:
		d_dma2->write_signal(SIG_I8237_BANK1, data >> 1, 0x7f);
		break;
	case 0x8f:
		d_dma2->write_signal(SIG_I8237_BANK0, data >> 1, 0x7f);
		break;
#endif
	}
}

