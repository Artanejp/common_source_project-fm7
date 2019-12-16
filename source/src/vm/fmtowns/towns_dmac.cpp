
#include "../towns_dmac.h"
#include "debugger.h"

namespace FMTOWNS {
void TOWNS_DMAC::initialize()
{
	UPD71071::initialize();
}

void TOWNS_DMAC::reset()
{
	UPD71071::reset();
}

void TOWNS_DMAC::write_io8(uint32_t addr, uint32_t data)
{
	pair32_t na, nba;
	bool addr_modified = false;
	nba.d = dma[selch].bareg;
	na.d = dma[selch].areg;
	
	switch(addr & 0x0f) {
	case 0x04:
		nba.b.l = data & 0xff;
		ba.b.l = data & 0xff;
		addr_modified = true;
		break;
	case 0x05:
		nba.b.h = data & 0xff;
		ba.b.h = data & 0xff;
		addr_modified = true;
		break;
	case 0x06:
		nba.b.h2 = data & 0xff;
		ba.b.h2 = data & 0xff;
		addr_modified = true;
		break;
	case 0x07:
		nba.b.h3 = data & 0xff;
		ba.b.h3 = data & 0xff;
		addr_modified = true;
		break;
	}
	if(addr_modified) {
		dma[selch].bareg = nba.d;
//		if(!base) {
		dma[selch].areg = na.d;
//		}
		return;
	}
	UPD71071::write_io8(addr, data);
}

uint32_t TOWNS_DMAC::read_io8(uint32_t addr)
{
	uint32_t val;
	pair32_t nval;
	if(((addr & 0x0f) >= 4) && ((addr & 0x0f) <= 7)) {
		if(base) {
			nval.d = dma[selch].bareg;
		} else {
			nval.d = dma[selch].areg;
		}
		switch(addr & 0x0f) {
		case 0x04:
			val = nval.l;
			break;
		case 0x05:
			val = nval.h;
			break;
		case 0x06:
			val = nval.h2;
			break;
		case 0x07:
			val = nval.h3;
			break;
		}
		return val;
	}
	return UPD71071::read_io8(addr);
}

void TOWNS_DMAC::do_dma_inc_dec_ptr_8bit(int c)
{
	// Note: FM-Towns may extend to 32bit.
	if(dma[c].mode & 0x20) {
		dma[c].areg = (dma[c].areg - 1) & 0xffffffff;
	} else {
		dma[c].areg = (dma[c].areg + 1) & 0xffffffff;
	}
}

void TOWNS_DMAC::do_dma_inc_dec_ptr_16bit(int c)
{
	// Note: FM-Towns may extend to 32bit.
	if(dma[c].mode & 0x20) {
		dma[c].areg = (dma[c].areg - 2) & 0xffffffff;
	} else {
		dma[c].areg = (dma[c].areg + 2) & 0xffffffff;
	}
}
	
		
