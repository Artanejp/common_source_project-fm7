
#include "./dmac.h"
#include "debugger.h"

namespace FMTOWNS {
void TOWNS_DMAC::initialize()
{
	UPD71071::initialize();
}

void TOWNS_DMAC::reset()
{
	UPD71071::reset();
  	dma_wrap_reg = 0xff;
//	dma_wrap_reg = 0x00;
//	b16 = 2; // Fixed 16bit.
}

void TOWNS_DMAC::write_io8(uint32_t addr, uint32_t data)
{
//	if((addr & 0x0f) == 0x0c) out_debug_log("WRITE REG: %08X %08X", addr, data);
//	out_debug_log("WRITE REG: %04X %02X", addr, data);
	uint naddr;
	pair32_t _d;
	pair32_t _bd;
	switch(addr & 0x0f) {
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		dma[selch].bareg = manipulate_a_byte_from_dword_le(dma[selch].bareg, (addr & 0x0f) - 4, data);
		if(base == 0) {
			dma[selch].areg = manipulate_a_byte_from_dword_le(dma[selch].areg,  (addr & 0x0f) - 4, data);
		}
		dma[selch].end = false; // OK?
		dma[selch].endreq = false; // OK?
		return;
		break;
	default:
		break;
	}
	UPD71071::write_io8(addr, data);
}

uint32_t TOWNS_DMAC::read_io8(uint32_t addr)
{
	uint32_t val;
	pair32_t _d;
	switch(addr & 0x0f) {
	case 0x07:
		if(base == 0) {
			_d.d = dma[selch].areg;
		} else {
			_d.d = dma[selch].bareg;
		}
		return (uint32_t)(_d.b.h3);
		break;
	}
	return UPD71071::read_io8(addr);
}

void TOWNS_DMAC::do_dma_inc_dec_ptr_8bit(int c)
{
	// Note: FM-Towns may extend to 32bit.
#if 0
	uint32_t incdec = ((dma[c].mode & 0x20) == 0) ? 1 : UINT32_MAX;
	uint32_t addr = dma[c].areg &   0x00ffffff;
	uint32_t high_a = dma[c].areg & 0xff000000;
	__LIKELY_IF(dma_wrap_reg != 0) {
		addr = (addr + incdec) & 0x00ffffff;
		addr = addr | high_a;
	} else {
		addr = (addr + incdec) & 0xffffffff;
	}
	dma[c].areg = addr;
#else
	uint32_t addr = dma[c].areg;
	if(dma[c].mode & 0x20) {
		addr = addr + 1;
	} else {
		addr = addr - 1;
	}
	__LIKELY_IF(dma_wrap_reg != 0) {
		uint32_t high_a = dma[c].areg & 0xff000000;
		addr = addr & 0x00ffffff;
		addr = addr | high_a;
 	}
	dma[c].areg = addr;
#endif
}

void TOWNS_DMAC::do_dma_inc_dec_ptr_16bit(int c)
{
	// Note: FM-Towns may extend to 32bit.
#if 0
	uint32_t incdec = ((dma[c].mode & 0x20) == 0) ? 2 : (UINT32_MAX - 1);
	uint32_t addr = dma[c].areg &   0x00ffffff;
	uint32_t high_a = dma[c].areg & 0xff000000;
	__LIKELY_IF(dma_wrap_reg != 0) {
		addr = (addr + incdec) & 0x00ffffff;
		addr = addr | high_a;
	} else {
		addr = (addr + incdec) & 0xffffffff;
	}
	dma[c].areg = addr;
#else
	uint32_t addr = dma[c].areg;
	if(dma[c].mode & 0x20) {
		addr = addr + 2;
	} else {
		addr = addr - 2;
	}
	__LIKELY_IF(dma_wrap_reg != 0) {
		uint32_t high_a = dma[c].areg & 0xff000000;
		addr = addr & 0x00ffffff;
		addr = addr | high_a;
 	}
	dma[c].areg = addr;
#endif
}

#if 0 /* For Debug */
bool TOWNS_DMAC::do_dma_epilogue(int c)
{
	if((dma[c].creg == 0) || ((dma[c].endreq) && !(dma[c].end) && ((dma[c].mode & 0xc0) != 0x40))) {  // OK?
		// TC
		bool is_tc = false;
		if((dma[c].end) || (dma[c].endreq)) is_tc = true;
		// TC
		if(dma[c].bcreg < (dma[c].creg - 1)) {
			is_tc = true;
		}
		if(is_tc) {
	#if 0
			out_debug_log(_T("TRANSFER COMPLETED:CH=%d  AREG=%08X BAREG=%08X CREG=%08X BCREG=%08X"),
						  c,
						  (dma[c].areg & 0xffffffff) ,
						  (dma[c].bareg & 0xffffffff) ,
						  dma[c].creg & 0x00ffffff,
						  dma[c].bcreg & 0x00ffffff
				);
	#endif
		}
	}
	return UPD71071::do_dma_epilogue(c);
}
#endif

uint32_t TOWNS_DMAC::read_signal(int id)
{
	if(id == SIG_TOWNS_DMAC_WRAP_REG) {
		return dma_wrap_reg;
	}
	return UPD71071::read_signal(id);
}

void TOWNS_DMAC::write_signal(int id, uint32_t data, uint32_t _mask)
{
	if(id == SIG_TOWNS_DMAC_WRAP_REG) {
		dma_wrap_reg = data;
//		this->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, data, mask);
	} else {
		// Fallthrough.
//		if(id == SIG_UPD71071_CH1) {
//			out_debug_log(_T("DRQ from SCSI %02X %02X"), data, mask);
//		}
		UPD71071::write_signal(id, data, _mask);
	}
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
					  dma[i].areg,
					  dma[i].creg,
					  dma[i].bareg,
					  dma[i].bcreg,
					  ((req | sreq) >> 0) & 1,
					  (mask >> 0) & 1,
					  dma[i].mode,
					  dir[(dma[i].mode >> 2) & 3]
			);
	}
	{
		my_stprintf_s(buffer, buffer_len,
					  _T("16Bit=%s ADDR_WRAP=%02X \n")
					  _T("SELECT CH=%d BASE=%02X REQ=%02X SREQ=%02X MASK=%02X TC=%02X ")
					  _T("CMD=%04X TMP=%04X\n")
					  _T("%s")
					  _T("%s")
					  _T("%s")
					  _T("%s"),
					  (b16 != 0) ? _T("YES") : _T("NO"), dma_wrap_reg,
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

#define STATE_VERSION	4

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
	state_fio->StateValue(dma_wrap_reg);
	return true;
}
}
