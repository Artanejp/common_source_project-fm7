
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
	dma_addr_mask = 0xffffffff; // OK?
//	dma_addr_mask = 0x000fffff; // OK?
	for(int i = 0; i < 4; i++) {
		creg_set[i] = false;
		bcreg_set[i] = false;
	}
//	b16 = 2; // Fixed 16bit.
}

void TOWNS_DMAC::write_io16(uint32_t addr, uint32_t data)
{
	pair32_t _d, _bd;
//	out_debug_log(_T("OUT16 %04X,%04X"), addr & 0xffff, data & 0xffff);
//	if(b16 != 0) {
	switch(addr & 0x0e) {
	case 0x02:
		if(base == 0) {
			creg_set[selch] = true;
		}
		bcreg_set[selch] = true;
		break;
	case 0x06:
		if(base == 0) {
			_d.d = dma[selch].areg;
			_d.w.h = data;
			dma[selch].areg = _d.d;
		}
		_d.d = dma[selch].bareg;
		_d.w.h = data;
		dma[selch].bareg = _d.d;
		return;
		break;
#if 0
	case 0x08:
		if((data & 0x04) != (cmd & 0x04)) {
			if((data & 0x04) == 0) {
				out_debug_log(_T("START TRANSFER:CH=%d CMD=%04X -> %04X AREG=%08X BAREG=%08X CREG=%04X BCREG=%04X"),
							  selch,
							  cmd, data & 0xffff,
							  dma[selch].areg, dma[selch].bareg,
							  dma[selch].creg, dma[selch].bcreg
					);
			} else {
				out_debug_log(_T("CLEAR TRANSFER:CH=%d CMD=%04X -> %04X AREG=%08X BAREG=%08X CREG=%04X BCREG=%04X"),
							  selch,
							  cmd, data & 0xffff,
							  dma[selch].areg, dma[selch].bareg,
							  dma[selch].creg, dma[selch].bcreg
					);
			}
		}
		break;
#endif
	}
	UPD71071::write_io16(addr, data);
}

void TOWNS_DMAC::write_io8(uint32_t addr, uint32_t data)
{
//	if((addr & 0x0f) == 0x0c) out_debug_log("WRITE REG: %08X %08X", addr, data);
//	out_debug_log("WRITE REG: %04X %02X", addr, data);
	uint naddr;
	pair32_t _d;
	pair32_t _bd;
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
		dma[selch].bareg = manipulate_a_byte_from_dword_le(dma[selch].bareg, 3, data);
		if(base == 0) {
			dma[selch].areg = manipulate_a_byte_from_dword_le(dma[selch].areg, 3, data);
		}
		return;
		break;
#if 0
	case 0x08:
		if((data & 0x04) != (cmd & 0x04)) {
			if((data & 0x04) == 0) {
				out_debug_log(_T("START TRANSFER:CH=%d CMD=%04X -> %04X AREG=%08X BAREG=%08X CREG=%04X BCREG=%04X"),
							  selch,
							  cmd, (cmd & 0xff00) | (data & 0x00ff),
							  dma[selch].areg, dma[selch].bareg,
							  dma[selch].creg, dma[selch].bcreg
					);
			} else {
				out_debug_log(_T("CLEAR TRANSFER:CH=%d CMD=%04X -> %04X  AREG=%08X BAREG=%08X CREG=%04X BCREG=%04X"),
							  selch,
							  cmd, (cmd & 0xff00) | (data & 0x00ff),
							  dma[selch].areg, dma[selch].bareg,
							  dma[selch].creg, dma[selch].bcreg
					);
			}
		}
#endif
		break;
	case 0x0a:
//		if((selch == 3)) {
//			out_debug_log(_T("SET MODE[%d] to %02X"), selch, data);
//		}
		break;
	case 0x0e:
		if(((data | req) & 0x08) != 0) {
			//	out_debug_log(_T("TRANSFER ENABLE@REG0E DATA=%02X"), data);
		}
		break;
	case 0x0f:
		// Note: This is *temporaly* workaround for 16bit transfer mode with 8bit bus.
		// 20200318 K.O
#if 0
#if !defined(USE_QUEUED_SCSI_TRANSFER)
		if((dma[selch].is_16bit) && !(inputs_ube[selch])) {
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
#endif
		break;
	default:
		break;
	}
	UPD71071::write_io8(addr, data);
}

uint32_t TOWNS_DMAC::read_io16(uint32_t addr)
{
	switch(addr & 0x0e) {
	case 0x06:
		if(base == 0) {
			return ((dma[selch].areg >> 16) & 0xffff);
		} else {
			return ((dma[selch].bareg >> 16) & 0xffff);
		}
		break;
	default:
//			return read_io8(addr & 0x0e);
		break;
	}
	return UPD71071::read_io16(addr);
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
	__LIKELY_IF(dma_wrap_reg != 0) {
		uint32_t high_a = dma[c].areg & 0xff000000;
		if(dma[c].mode & 0x20) {
			dma[c].areg = dma[c].areg - 1;
		} else {
			dma[c].areg = dma[c].areg + 1;
		}
		dma[c].areg = ((dma[c].areg & 0x00ffffff) | high_a) & dma_addr_mask;
	} else {
		if(dma[c].mode & 0x20) {
			dma[c].areg = (dma[c].areg - 1) & dma_addr_mask;
		} else {
			dma[c].areg = (dma[c].areg + 1) & dma_addr_mask;
		}
	}
}

void TOWNS_DMAC::do_dma_inc_dec_ptr_16bit(int c)
{
	// Note: FM-Towns may extend to 32bit.
	__LIKELY_IF(dma_wrap_reg != 0) {
		uint32_t high_a = dma[c].areg & 0xff000000;
		if(dma[c].mode & 0x20) {
			dma[c].areg = dma[c].areg - 2;
		} else {
			dma[c].areg = dma[c].areg + 2;
		}
		dma[c].areg = ((dma[c].areg & 0x00ffffff) | high_a) & dma_addr_mask;
	} else {
		if(dma[c].mode & 0x20) {
			dma[c].areg = (dma[c].areg - 2) & dma_addr_mask;
		} else {
			dma[c].areg = (dma[c].areg + 2) & dma_addr_mask;
		}
	}
}

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

uint32_t TOWNS_DMAC::read_signal(int id)
{
	if(id == SIG_TOWNS_DMAC_WRAP_REG) {
		return dma_wrap_reg;
	} else if(id == SIG_TOWNS_DMAC_ADDR_MASK) {
		return dma_addr_mask;
	}
	return UPD71071::read_signal(id);
}

void TOWNS_DMAC::write_signal(int id, uint32_t data, uint32_t _mask)
{
	if(id == SIG_TOWNS_DMAC_WRAP_REG) {
		dma_wrap_reg = data;
//		this->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, data, mask);
	} else if(id == SIG_TOWNS_DMAC_ADDR_MASK) {
		// From eFMR50 / memory.cpp / update_dma_addr_mask()
		dma_addr_mask = data;
	} else {
		// Fallthrough.
//		if(id == SIG_UPD71071_CH1) {
//			out_debug_log(_T("DRQ from SCSI %02X %02X"), data, mask);
//		}
		UPD71071::write_signal(id, data, _mask);
	}
}

void TOWNS_DMAC::do_dma_dev_to_mem_8bit(int c)
{
	// io -> memory
	uint32_t val;
	uint32_t addr = dma[c].areg;
	reset_ube(c);
	val = dma[c].dev->read_dma_io8(0);

	// update temporary register
	tmp = (tmp >> 8) | (val << 8);
	bool _debugging = false;
	if((d_debugger != NULL) && (__USE_DEBUGGER)) {
		_debugging = d_debugger->now_device_debugging;
	}
	__UNLIKELY_IF(_debugging) {
			d_debugger->write_via_debugger_data8(addr, val);
	} else {
		write_via_debugger_data8(addr, val);
	}
}

void TOWNS_DMAC::do_dma_mem_to_dev_8bit(int c)
{
	// memory -> io
	uint32_t val;
	uint32_t addr = dma[c].areg;
	reset_ube(c);
	bool _debugging = false;
	if((d_debugger != NULL) && (__USE_DEBUGGER)) {
		_debugging = d_debugger->now_device_debugging;
	}
	__UNLIKELY_IF(_debugging) {
		val = d_debugger->read_via_debugger_data8(addr);
	} else {
		val = read_via_debugger_data8(addr);
	}
	// update temporary register
	tmp = (tmp >> 8) | (val << 8);

	dma[c].dev->write_dma_io8(0, val);
}

void TOWNS_DMAC::do_dma_dev_to_mem_16bit(int c)
{
	// io -> memory
	uint32_t val;
	uint32_t addr = dma[c].areg;
	set_ube(c);
	val = dma[c].dev->read_dma_io16(0);
	// update temporary register
	tmp = val;

	bool _debugging = false;
	if((d_debugger != NULL) && (__USE_DEBUGGER)) {
		_debugging = d_debugger->now_device_debugging;
	}
/*
	if((addr & 1) != 0) {
		// If odd address, write a byte.
		uint32_t tval = (val >> 8) & 0xff;
		__UNLIKELY_IF(_debugging) {
			d_debugger->write_via_debugger_data8(addr, tval);
		} else {
			write_via_debugger_data8(addr, tval);
		}
	} else {
*/
		// 16bit
		__UNLIKELY_IF(_debugging) {
			d_debugger->write_via_debugger_data16(addr, val);
		} else {
			write_via_debugger_data16(addr, val);
		}
//	}
}

void TOWNS_DMAC::do_dma_mem_to_dev_16bit(int c)
{
	// memory -> io
	uint32_t val;
	uint32_t addr = dma[c].areg;
	set_ube(c);
	bool _debugging = false;
	if((d_debugger != NULL) && (__USE_DEBUGGER)) {
		_debugging = d_debugger->now_device_debugging;
	}
	__UNLIKELY_IF(_debugging) {
		val = d_debugger->read_via_debugger_data16(addr);
	} else {
		val = this->read_via_debugger_data16(addr);
	}
//	if((addr & 1) != 0) {
//		// If odd address, read a high byte.
//		val = (val >> 8) & 0xff;
//	}
	// update temporary register
	tmp = val;

	dma[c].dev->write_dma_io16(0, val);
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
					  _T("16Bit=%s ADDR_MASK=%08X ADDR_WRAP=%02X \n")
					  _T("SELECT CH=%d BASE=%02X REQ=%02X SREQ=%02X MASK=%02X TC=%02X ")
					  _T("CMD=%04X TMP=%04X\n")
					  _T("%s")
					  _T("%s")
					  _T("%s")
					  _T("%s"),
					  (b16 != 0) ? _T("YES") : _T("NO"), dma_addr_mask, dma_wrap_reg,
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

#define STATE_VERSION	3

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
	state_fio->StateValue(dma_addr_mask);
	state_fio->StateArray(creg_set, sizeof(creg_set), 1);
	state_fio->StateArray(bcreg_set, sizeof(bcreg_set), 1);

	return true;
}
}
