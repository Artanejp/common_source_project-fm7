
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
	for(int i = 0; i < 4; i++) {
		dma_high_address[i] = 0x00000000;
		creg_set[i] = false;
		bcreg_set[i] = false;
	}
//	b16 = 2; // Fixed 16bit.
}

bool TOWNS_DMAC::do_dma_prologue(int c)
{
	uint8_t bit = 1 << c;
	if(dma[c].creg-- == 0) {  // OK?
		// TC
		if(dma[c].mode & 0x10) {
			// auto initialize
			dma[c].areg = dma[c].bareg;
			dma[c].creg = dma[c].bcreg;
		} else {
			mask |= bit;
		}
		req &= ~bit;
		sreq &= ~bit;
		tc |= bit;
						
		write_signals(&outputs_tc, 0xffffffff);
		return true;
	}
	if(_SINGLE_MODE_DMA) {
		// Note: At FM-Towns, SCSI's DMAC will be set after
		//       SCSI bus phase become DATA IN/DATA OUT.
		//       Before bus phase became DATA IN/DATA OUT,
		//       DMAC mode and state was unstable (and ASSERTED
		//       DRQ came from SCSI before this state change).
		// ToDo: Stop correctly before setting.
		//       -- 20200316 K.O
		if(((dma[c].mode & 0xc0) == 0x40) || ((dma[c].mode & 0xc0) == 0x00)) {
			// single mode or demand mode
			req &= ~bit;
			sreq &= ~bit;
			return true;
		}
	}
	return false;
}

	
void TOWNS_DMAC::write_io8(uint32_t addr, uint32_t data)
{
//	if((addr & 0x0f) == 0x0c) out_debug_log("WRITE REG: %08X %08X", addr, data);
//	out_debug_log("WRITE REG: %04X %02X", addr, data);
	uint naddr;
	switch(addr & 0x0f) {
	case 0x00:
//		out_debug_log(_T("RESET REG(00h) to %02X"), data);
		break;
	case 0x02:
	case 0x03:
		naddr = (addr & 0x0f) - 2;
		if(base == 0) {
			pair16_t nc;
			nc.w = dma[selch].creg;
			switch(naddr) {
			case 0:
				nc.b.l = data;
				break;
			case 1:
				nc.b.h = data;
				break;
			}
			
			dma[selch].creg = nc.w;
			// Note: This is *temporaly* workaround for 16bit transfer mode with 8bit bus.
			// 20200318 K.O
			creg_set[selch] = true;
			//out_debug_log(_T("CH%d CREG=%04X"), selch, dma[selch].creg);
		}
		{
			pair16_t nc;
			nc.w = dma[selch].bcreg;
			switch(naddr) {
			case 0:
				nc.b.l = data;
				break;
			case 1:
				nc.b.h = data;
				break;
			}
			dma[selch].bcreg = nc.w;
			// Note: This is *temporaly* workaround for 16bit transfer mode with 8bit bus.
			// 20200318 K.O
			bcreg_set[selch] = true;
			//out_debug_log(_T("CH%d BCREG=%04X"), selch, dma[selch].bcreg);
		}
		return;
		break;
	case 0x04:
	case 0x05:
	case 0x06:
		naddr = (addr & 0x0f) - 4;
		if(base == 0) {
			pair32_t na;
			na.d = dma[selch].areg;
			switch(naddr) {
			case 0:
				na.b.l = data;
				break;
			case 1:
				na.b.h = data;
				break;
			case 2:
				na.b.h2 = data;
				break;
			}
			dma[selch].areg = na.d;
		}
		{
			pair32_t na;
			na.d = dma[selch].bareg;
			switch(naddr) {
			case 0:
				na.b.l = data;
				break;
			case 1:
				na.b.h = data;
				break;
			case 2:
				na.b.h2 = data;
				break;
			}
			dma[selch].bareg = na.d;
		}
		return;
		break;
	case 0x07:
		dma_high_address[selch] = (data & 0xff) << 24;
		return;
		break;
	case 0x0a:
//		out_debug_log(_T("SET MODE[%d] to %02X"), selch, data);
		break;
	case 0x0f:
		// Note: This is *temporaly* workaround for 16bit transfer mode with 8bit bus.
		// 20200318 K.O
#if !defined(USE_QUEUED_SCSI_TRANSFER)
		if((dma[selch].is_16bit) && (b16)) {
			if(creg_set[selch]) {
				dma[selch].creg <<= 1;
				dma[selch].creg++;
				creg_set[selch] = false;
			}
			if(bcreg_set[selch]) {
				dma[selch].bcreg <<= 1;
				dma[selch].bcreg++;
				bcreg_set[selch] = false;
			}
			}
#endif
		if((data & 0x02) == 0) {
			out_debug_log(_T("START SCSI DMA MODE=%02X ADDR=%08X COUNT=%04X"),
						  dma[1].mode, (dma[1].areg & 0xffffff) | dma_high_address[1],
						  dma[1].creg);
		}
		break;
	default:
		break;
	}
	UPD71071::write_io8(addr, data);
}
uint32_t TOWNS_DMAC::read_io8(uint32_t addr)
{
	uint32_t val;
	pair32_t nval;
	switch(addr & 0x0f) {
	case 0x01:
		return (base << 3) | (1 << (selch & 3));
		break;
	case 0x07:
		return (dma_high_address[selch] >> 24);
		break;
	}
	return UPD71071::read_io8(addr);
}
	
void TOWNS_DMAC::do_dma_inc_dec_ptr_8bit(int c)
{
	// Note: FM-Towns may extend to 32bit.
	if(dma[c].mode & 0x20) {
		dma[c].areg = (((dma[c].areg - 1) & 0x00ffffff) | dma_high_address[c]) & dma_addr_mask;
	} else {
		dma[c].areg = (((dma[c].areg + 1) & 0x00ffffff) | dma_high_address[c]) & dma_addr_mask;
	}
}

void TOWNS_DMAC::do_dma_inc_dec_ptr_16bit(int c)
{
	// Note: FM-Towns may extend to 32bit.
	if(dma[c].mode & 0x20) {
		dma[c].areg = (((dma[c].areg - 2) & 0x00ffffff) | dma_high_address[c]) & dma_addr_mask;
	} else {
		dma[c].areg = (((dma[c].areg + 2) & 0x00ffffff) | dma_high_address[c]) & dma_addr_mask;
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
	} else if((id >= SIG_TOWNS_DMAC_HIGH_ADDRESS) && (id <= (SIG_TOWNS_DMAC_HIGH_ADDRESS + 3))) {
		int ch = id - SIG_TOWNS_DMAC_HIGH_ADDRESS;
		return dma_high_address[ch];
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
//		if(id == SIG_UPD71071_CH1) {
//			out_debug_log(_T("DRQ from SCSI %02X %02X"), data, mask);
//		}
		UPD71071::write_signal(id, data, mask);
	}
}		


void TOWNS_DMAC::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
//	out_debug_log(_T("WRITE 8BIT ADDR %08X to DATA:%02X"), addr, data);
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
	static const _TCHAR *dir[4] = {
		_T("VERIFY"), _T("I/O->MEM"), _T("MEM->I/O"), _T("INVALID")
	};
	if(buffer == NULL) return false;
	_TCHAR sbuf[4][512] = {0};
	for(int i = 0; i < 4; i++) {
		my_stprintf_s(sbuf[i], 512,
		  _T("CH%d AREG=%08X CREG=%04X BAREG=%08X BCREG=%04X REQ=%d MASK=%d MODE=%02X %s\n"),
					  i,
					  (dma_high_address[i] & 0xff000000) | (dma[i].areg & 0x00ffffff),
					  dma[i].creg,
					  (dma_high_address[i] & 0xff000000) | (dma[i].bareg & 0x00ffffff),
					  dma[i].bcreg,
					  ((req | sreq) >> 0) & 1,
					  (mask >> 0) & 1,
					  dma[i].mode,
					  dir[(dma[i].mode >> 2) & 3]
			);
	}
	{
		my_stprintf_s(buffer, buffer_len,
					  _T("16Bit=%s ADDR_MASK=%08X ADDR_REG=%02X ADDR_WRAP=%02X \n")
					  _T("%s")
					  _T("%s")
					  _T("%s")
					  _T("%s")
					  , (b16) ? _T("YES") : _T("NO"),
					  dma_addr_mask,
					  dma_addr_reg,
					  dma_wrap_reg,
					  sbuf[0],
					  sbuf[1],
					  sbuf[2],
					  sbuf[3]);
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
	state_fio->StateArray(dma_high_address, sizeof(dma_high_address), 1);
	
	state_fio->StateArray(creg_set, sizeof(creg_set), 1);
	state_fio->StateArray(bcreg_set, sizeof(bcreg_set), 1);

	return true;
}
}
