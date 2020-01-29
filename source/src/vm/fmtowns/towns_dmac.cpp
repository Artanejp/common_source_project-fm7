
#include "./towns_dmac.h"
#include "debugger.h"

namespace FMTOWNS {
void TOWNS_DMAC::initialize()
{
	UPD71071::initialize();
}

void TOWNS_DMAC::reset()
{
	UPD71071::reset();
	dma_wrap_reg = 0;
	dma_addr_reg = 0;
	dma_addr_mask = 0xffffffff; // OK?
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
		na.b.l = data & 0xff;
		addr_modified = true;
		break;
	case 0x05:
		nba.b.h = data & 0xff;
		na.b.h = data & 0xff;
		addr_modified = true;
		break;
	case 0x06:
		nba.b.h2 = data & 0xff;
		na.b.h2 = data & 0xff;
		addr_modified = true;
		break;
	case 0x07:
		nba.b.h3 = data & 0xff;
		na.b.h3 = data & 0xff;
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
			val = nval.b.l;
			break;
		case 0x05:
			val = nval.b.h;
			break;
		case 0x06:
			val = nval.b.h2;
			break;
		case 0x07:
			val = nval.b.h3;
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
		dma[c].areg = (dma[c].areg - 1) & dma_addr_mask;
	} else {
		dma[c].areg = (dma[c].areg + 1) & dma_addr_mask;
	}
}

void TOWNS_DMAC::do_dma_inc_dec_ptr_16bit(int c)
{
	// Note: FM-Towns may extend to 32bit.
	if(dma[c].mode & 0x20) {
		dma[c].areg = (dma[c].areg - 2) & dma_addr_mask;
	} else {
		dma[c].areg = (dma[c].areg + 2) & dma_addr_mask;
	}
}

uint32_t TOWNS_DMAC::read_signal(int id)
{
	if(id == SIG_TOWNS_DMAC_ADDR_REG) {
		return dma_addr_reg;
	} else if(SIG_TOWNS_DMAC_WRAP_REG) {
		return dma_wrap_reg;
	} else if(id == SIG_TOWNS_DMAC_ADDR_MASK) {
		return dma_addr_mask;
	}
	return UPD71071::read_signal(id);
}

void TOWNS_DMAC::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_TOWNS_DMAC_ADDR_REG) {
		dma_addr_reg = data & 3;
		this->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, data, mask);
	} else if(id == SIG_TOWNS_DMAC_WRAP_REG) {
		dma_wrap_reg = data;
		this->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, data, mask);
	} else if(id == SIG_TOWNS_DMAC_ADDR_MASK) {
		// From eFMR50 / memory.cpp / update_dma_addr_mask()
		switch(dma_addr_reg & 3) {
		case 0:
			dma_addr_mask = data;
			break;
		case 1:
			if(!(dma_wrap_reg & 1) && (data == 0x000fffff)) {
				dma_addr_mask = 0x000fffff;
			} else {
				dma_addr_mask = 0x00ffffff;
			}
			break;
		default:
			if(!(dma_wrap_reg & 1) && (data == 0x000fffff)) {
				dma_addr_mask = 0x000fffff;
			} else {
				dma_addr_mask = 0xffffffff;
			}
			break;
		}
	} else {
		// Fallthrough.
		UPD71071::write_signal(id, data, mask);
	}
}		


void TOWNS_DMAC::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
	d_mem->write_dma_data8(addr & dma_addr_mask, data);
}

uint32_t TOWNS_DMAC::read_via_debugger_data8(uint32_t addr)
{
	return d_mem->read_dma_data8(addr & dma_addr_mask);
}

void TOWNS_DMAC::write_via_debugger_data16(uint32_t addr, uint32_t data)
{
	d_mem->write_dma_data16(addr & dma_addr_mask, data);
}

uint32_t TOWNS_DMAC::read_via_debugger_data16(uint32_t addr)
{
	return d_mem->read_dma_data16(addr & dma_addr_mask);
}

// note: if SINGLE_MODE_DMA is defined, do_dma() is called in every machine cycle
bool TOWNS_DMAC::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	if(buffer == NULL) return false;
	_TCHAR sbuf[4096] = {0};
	if(UPD71071::get_debug_regs_info(sbuf, 4096)) {
		my_stprintf_s(buffer, buffer_len,
					  _T("%s\n")
					  _T("ADDR_MASK=%08X ADDR_REG=%02X ADDR_WRAP=%02X\n")
					  , sbuf, dma_addr_mask, dma_addr_reg, dma_wrap_reg);
		return true;
	}
	return false;
}
	
#define STATE_VERSION	1
	
bool TOWNS_DMAC::process_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	if(!(UPD71071::process_state(state_fio, loading))) {
		return false;
	}
	state_fio->StateValue(dma_addr_reg);
	state_fio->StateValue(dma_wrap_reg);
	state_fio->StateValue(dma_addr_mask);

	return true;
}
}
