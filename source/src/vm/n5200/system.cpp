/*
	NEC N5200 Emulator 'eN5200'

	Author : Takeda.Toshiya
	Date   : 2009.06.04-

	[ system i/o ]
*/

#include "system.h"
#include "../i8237.h"

void SYSTEM::reset()
{
	mode = 0;
	nmi_enb = false;
}

void SYSTEM::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case 0x21:
		d_dma->write_signal(SIG_I8237_BANK0 + 1, data, 0xff);
		break;
	case 0x23:
		d_dma->write_signal(SIG_I8237_BANK0 + 2, data, 0xff);
		break;
	case 0x25:
		d_dma->write_signal(SIG_I8237_BANK0 + 3, data, 0xff);
		break;
	case 0x27:
		d_dma->write_signal(SIG_I8237_BANK0 + 0, data, 0xff);
		break;
	case 0x29:
		if((data & 0x0c) == 0) {
			d_dma->write_signal(SIG_I8237_MASK0 + (data & 3), 0, 0xff);
		} else if((data & 0x0c) == 4) {
			d_dma->write_signal(SIG_I8237_MASK0 + (data & 3), 0x0f, 0xff);
		} else if((data & 0x0c) == 0x0c) {
			d_dma->write_signal(SIG_I8237_MASK0 + (data & 3), 0xff, 0xff);
		}
	case 0x3b:
		mode = data;
		break;
	case 0x50:
		nmi_enb = false;
		break;
	case 0x52:
		nmi_enb = true;
		break;
	}
}

uint32_t SYSTEM::read_io8(uint32_t addr)
{
	switch(addr) {
	case 0x39:
		return mode;
	}
	return 0xff;
}

