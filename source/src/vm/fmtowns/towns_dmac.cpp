
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
	dma_high_address = 0x00000000;
	b16 = 2; // Fixed 16bit.
}

void TOWNS_DMAC::write_io8(uint32_t addr, uint32_t data)
{
//	if((addr & 0x0f) == 0x0c) out_debug_log("WRITE REG: %08X %08X", addr, data);
	out_debug_log("WRITE REG: %04X %02X", addr, data);
	switch(addr & 0x0f) {
	case 0x07:
		dma_high_address = (data & 0xff) << 24;
		return;
		break;
//	case 0x08:
//		cmd = (cmd & 0xff00) | (data & 0xfb);
//		return;
//		break;
	default:
		break;
	}
	UPD71071::write_io8(addr, data);
}

uint32_t TOWNS_DMAC::read_io8(uint32_t addr)
{
	uint32_t val;
	pair32_t nval;
	if((addr & 0x0f) == 7) {
		return (dma_high_address >> 24);
	}
	return UPD71071::read_io8(addr);
}
#if 0
// Note: DATABUS will be 16bit wide. 20200131 K.O
void TOWNS_DMAC::do_dma_verify_16bit(int c)
{
	// verify
	bool is_master = false;
	if(d_dma != NULL) {
		is_master = true;
	}
	if(is_master) {
		if((c == 1) || (c == 3)) {
			if(b16 != 0) {
				UPD71071::do_dma_verify_16bit(c);
				return;
			}
		}
	} else if(b16 != 0) {
		UPD71071::do_dma_verify_16bit(c);
		return;
	}
	pair16_t d;
	d.b.l = dma[c].dev->read_dma_io8(0);
	d.b.h = dma[c].dev->read_dma_io8(0);
	// update temporary register
	tmp = d.w;

}
void TOWNS_DMAC::do_dma_dev_to_mem_16bit(int c)
{
	// io -> memory
	uint32_t val;
	pair16_t d;
	bool is_master = false;
	if(d_dma != NULL) {
		is_master = true;
	}
	if(is_master) {
		if((c == 1) || (c == 3)) {
			if(b16 != 0) {
				UPD71071::do_dma_dev_to_mem_16bit(c);
				return;
			}
		}
	} else if(b16 != 0) {
		UPD71071::do_dma_dev_to_mem_16bit(c);
		return;
	}
	d.b.l = dma[c].dev->read_dma_io8(0);
	d.b.h = dma[c].dev->read_dma_io8(0);
	val = d.w;
	write_signals(&outputs_wrote_mem_byte, dma[c].areg);
	if(_USE_DEBUGGER) {
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			d_debugger->write_via_debugger_data16(dma[c].areg, val);
		} else {
			this->write_via_debugger_data16(dma[c].areg, val);
		}
	} else {
		this->write_via_debugger_data16(dma[c].areg, val);
	}							
	// update temporary register
	tmp = val;

}

void TOWNS_DMAC::do_dma_mem_to_dev_16bit(int c)
{
	// memory -> io
	uint32_t val;
	bool is_master = false;
	if(d_dma != NULL) {
		is_master = true;
	}
	if(is_master) {
		if((c == 1) || (c == 3)) {
			if(b16 != 0) {
				UPD71071::do_dma_mem_to_dev_16bit(c);
				return;
			}
		}
	} else if(b16 != 0) {
		UPD71071::do_dma_mem_to_dev_16bit(c);
		return;
	}
	if(_USE_DEBUGGER) {
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			val = d_debugger->read_via_debugger_data16(dma[c].areg);
		} else {
			val = this->read_via_debugger_data16(dma[c].areg);
		}
	} else {
		val = this->read_via_debugger_data16(dma[c].areg);
	}
	pair16_t d;
	d.w = val;
	dma[c].dev->write_dma_io8(0, d.b.l);
	dma[c].dev->write_dma_io8(0, d.b.h);
//	dma[c].dev->write_dma_io16(0, val);
	// update temporary register
	tmp = val;
}
#endif
	
void TOWNS_DMAC::do_dma_inc_dec_ptr_8bit(int c)
{
	// Note: FM-Towns may extend to 32bit.
	if(dma[c].mode & 0x20) {
		dma[c].areg = (((dma[c].areg - 1) & 0x00ffffff) | dma_high_address) & dma_addr_mask;
	} else {
		dma[c].areg = (((dma[c].areg + 1) & 0x00ffffff) | dma_high_address) & dma_addr_mask;
	}
}

void TOWNS_DMAC::do_dma_inc_dec_ptr_16bit(int c)
{
	// Note: FM-Towns may extend to 32bit.
	if(dma[c].mode & 0x20) {
		dma[c].areg = (((dma[c].areg - 2) & 0x00ffffff) | dma_high_address) & dma_addr_mask;
	} else {
		dma[c].areg = (((dma[c].areg + 2) & 0x00ffffff) | dma_high_address) & dma_addr_mask;
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
	} else if(id == SIG_TOWNS_DMAC_HIGH_ADDRESS) {
		return dma_high_address;
	}
	return UPD71071::read_signal(id);
}

void TOWNS_DMAC::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_TOWNS_DMAC_ADDR_REG) {
		dma_addr_reg = data & 3;
//		this->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, data, mask);
	} else if(id == SIG_TOWNS_DMAC_WRAP_REG) {
		dma_wrap_reg = data;
//		this->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, data, mask);
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
	out_debug_log(_T("WRITE 8BIT ADDR %08X to DATA:%02X"), addr, data);
	d_mem->write_dma_data8(addr & dma_addr_mask, data);
}

uint32_t TOWNS_DMAC::read_via_debugger_data8(uint32_t addr)
{
	return d_mem->read_dma_data8(addr & dma_addr_mask);
}

void TOWNS_DMAC::write_via_debugger_data16(uint32_t addr, uint32_t data)
{
//	out_debug_log(_T("WRITE 16BIT DATA:%04X"), data);
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
					  _T("ADDR_MASK=%08X ADDR_REG=%02X ADDR_WRAP=%02X HIGH ADDRESS=%02X\n")
					  , sbuf, dma_addr_mask, dma_addr_reg, dma_wrap_reg, dma_high_address >> 24);
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
	state_fio->StateValue(dma_high_address);

	return true;
}
}
