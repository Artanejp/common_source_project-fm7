
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
//	dma_addr_mask = 0x000fffff; // OK?
	for(int i = 0; i < 4; i++) {
		dma_high_address[i] = 0x00000000;
		dma_high_address_bak[i] = 0x00000000;
		creg_set[i] = false;
		bcreg_set[i] = false;
	}
//	b16 = 2; // Fixed 16bit.
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
		// Note: This is *temporaly* workaround for 16bit transfer mode with 8bit bus.
		// 20200318 K.O
		if(base == 0) {
			creg_set[selch] = true;
		}
		bcreg_set[selch] = true;
		break;
	case 0x07:
		if(base == 0) {
			dma_high_address[selch] = (data & 0xff) << 24;
		}
		dma_high_address_bak[selch] = (data & 0xff) << 24;
		return;
		break;
	case 0x08:
//		if((data & 0x04) != (cmd & 0x04)) {
//			out_debug_log(_T("TRANSFER: CMD=%04X -> %04X CH=%d\nADDR=%08X"), cmd, (cmd & 0xff00) | (data & 0xff), selch, (dma[selch].areg & 0x00ffffff) | (dma_high_address[selch]));
			
//		}
		break;
	case 0x0a:
//		out_debug_log(_T("SET MODE[%d] to %02X"), selch, data);
		break;
	case 0x0e:
		if(((data | req) & 0x08) != 0) {
			//	out_debug_log(_T("TRANSFER ENABLE@REG0E DATA=%02X"), data);
		}
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
		bcreg_set[selch] = false;
		creg_set[selch] = false;
#endif
#if 0
		if((data & 0x08) == 0) {
			out_debug_log(_T("START CDROM DMA MODE=%02X ADDR=%08X COUNT=%04X"),
						  dma[3].mode, (dma[3].areg & 0xffffff) | dma_high_address[3],
						  dma[3].creg);
		}
#endif
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
		if(base == 0) {
			return (dma_high_address[selch] >> 24);
		}
		return (dma_high_address_bak[selch] >> 24);
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

bool TOWNS_DMAC::do_dma_epilogue(int c)
{
	if(dma[c].creg == 0) {  // OK?
		// TC
		if(dma[c].mode & 0x10) {
			dma_high_address[c] = dma_high_address_bak[c];
		}
	}
	return UPD71071::do_dma_epilogue(c);
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

void TOWNS_DMAC::write_signal(int id, uint32_t data, uint32_t _mask)
{
	if(id == SIG_TOWNS_DMAC_ADDR_REG) {
		dma_addr_reg = data & 3;
//		this->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, data, mask);
	} else if(id == SIG_TOWNS_DMAC_WRAP_REG) {
		dma_wrap_reg = data;
//		this->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, data, mask);
	} else if(id == SIG_TOWNS_DMAC_ADDR_MASK) {
		// From eFMR50 / memory.cpp / update_dma_addr_mask()
		
#if 0
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
#else
		dma_addr_mask = data;
#endif
	} else {
		// Fallthrough.
//		if(id == SIG_UPD71071_CH1) {
//			out_debug_log(_T("DRQ from SCSI %02X %02X"), data, mask);
//		}
		UPD71071::write_signal(id, data, _mask);
	}
}		


void TOWNS_DMAC::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
	addr = (dma_high_address[selch] & 0xff000000) | (addr & 0x00ffffff);
//	if(addr == 0xD000) out_debug_log(_T("WRITE 8BIT ADDR %08X to DATA:%02X"), addr, data);
	d_mem->write_dma_data8(addr, data);
}

uint32_t TOWNS_DMAC::read_via_debugger_data8(uint32_t addr)
{
	addr = (dma_high_address[selch] & 0xff000000) | (addr & 0x00ffffff);
	return d_mem->read_dma_data8(addr);
}

void TOWNS_DMAC::write_via_debugger_data16(uint32_t addr, uint32_t data)
{
//	out_debug_log(_T("WRITE 16BIT DATA:%04X"), data);
	addr = (dma_high_address[selch] & 0xff000000) | (addr & 0x00ffffff);
	d_mem->write_dma_data16(addr, data);
}

uint32_t TOWNS_DMAC::read_via_debugger_data16(uint32_t addr)
{
	addr = (dma_high_address[selch] & 0xff000000) | (addr & 0x00ffffff);
	return d_mem->read_dma_data16(addr);
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
					  _T("SELECT CH=%d BASE=%02X REQ=%02X SREQ=%02X MASK=%02X TC=%02X ")
					  _T("CMD=%04X TMP=%04X\n")
					  _T("%s")
					  _T("%s")
					  _T("%s")
					  _T("%s"),
					  (b16) ? _T("YES") : _T("NO"), dma_addr_mask, dma_addr_reg, dma_wrap_reg,
					  selch, base, req, sreq, mask, tc,
					  cmd, tmp,
					  sbuf[0],
					  sbuf[1],
					  sbuf[2],
					  sbuf[3]);
		return true;
	}
	return false;
}
	
#define STATE_VERSION	2
	
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
	state_fio->StateArray(dma_high_address_bak, sizeof(dma_high_address_bak), 1);
	
	state_fio->StateArray(creg_set, sizeof(creg_set), 1);
	state_fio->StateArray(bcreg_set, sizeof(bcreg_set), 1);

	return true;
}
}
