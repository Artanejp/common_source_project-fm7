/*
	NEC N5200 Emulator 'eN5200'

	Author : Takeda.Toshiya
	Date   : 2008.06.11 -

	[ floppy ]
*/

#include "floppy.h"
#include "../i8237.h"

void FLOPPY::reset()
{
	chgreg = 3;
}

void FLOPPY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0x92:
	case 0xca:
//		if(((addr >> 4) ^ chgreg) & 1) {
//			break;
//		}
		d_fdc->write_io8(1, data);
		break;
	case 0x94:
	case 0xcc:
//		if(((addr >> 4) ^ chgreg) & 1) {
//			break;
//		}
		if((ctrlreg ^ data) & 0x10) {
			// mode chang
//			fdcstatusreset();
//			fdc_dmaready(0);
//			dmac_check();
		}
		ctrlreg = data;
		break;
	case 0xbe:
		chgreg = data;
		break;
	}
}

uint32_t FLOPPY::read_io8(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0x90:
	case 0xc8:
//		if(((addr >> 4) ^ chgreg) & 1) {
//			break;
//		}
		return d_fdc->read_io8(0);
	case 0x92:
	case 0xca:
//		if(((addr >> 4) ^ chgreg) & 1) {
//			break;
//		}
		return d_fdc->read_io8(1);
	case 0x94:
	case 0xcc:
//		if(((addr >> 4) ^ chgreg) & 1) {
//			break;
//		}
		return (addr & 0x10) ? 0x40 : 0x70;
	case 0xbe:
		return (chgreg & 3) | 8;
	}
	return addr & 0xff;
}

void FLOPPY::write_signal(int id, uint32_t data, uint32_t mask)
{
	static const int dma_ids[2] = { SIG_I8237_CH3, SIG_I8237_CH2 };
	
	// drq from fdc
	d_dma->write_signal(dma_ids[chgreg & 1], data, mask);
}

