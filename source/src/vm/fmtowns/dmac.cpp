
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
	if(b16 != 0) {
		switch(addr & 0x0f) {
		case 0x02:
		case 0x03:
			if(base == 0) {
				creg_set[selch] = true;
				dma[selch].creg = data & 0xffff;
			}
			dma[selch].bcreg = data & 0xffff;
			bcreg_set[selch] = true;
			return;
			break;
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			_d.d = dma[selch].areg;
			_bd.d = dma[selch].bareg;
			if((addr & 0x0f) < 6) {
				if(base == 0) {
					_d.w.l = (data & 0xffff);
					dma[selch].areg = _d.d;
				}
				_bd.w.l = (data & 0xffff);
				dma[selch].bareg = _bd.d;
			} else {
				if(base == 0) {
					_d.w.h = (data & 0xffff);
					dma[selch].areg = _d.d;
				}
				_bd.w.h = (data & 0xffff);
				dma[selch].bareg = _bd.d;
			}
			break;
		case 0x08:
		case 0x09:
			cmd = data & 0xffff;
			if(((data & 0x04) != (cmd & 0x04)) && (selch == 3)) {
				if((data & 0x04) == 0) {
					out_debug_log(_T("TRANSFER: CMD=%04X -> %04X CH=%d\nADDR=%08X"), cmd, (cmd & 0xff00) | (data & 0xff), selch, dma[selch].areg);
				}
			}
			break;
		default:
//			write_io8(addr & 0x0e, data);
			write_io8(addr, data);
			break;
		}
	} else {
		write_io8(addr, data);
//		write_io8((addr & 0x0e) + 0, data);
//		write_io8((addr & 0x0e) + 1, data);
	}
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
		_d.d  = dma[selch].areg;
		_bd.d = dma[selch].bareg;
		_d.b.h3  = data;
		_bd.b.h3 = data;
		if(base == 0) {
			dma[selch].areg = _d.d;
		}
		dma[selch].bareg = _bd.d;
		return;
		break;
	case 0x08:
		if(((data & 0x04) != (cmd & 0x04)) && (selch == 3)) {
			if((data & 0x04) != 0) break;
			out_debug_log(_T("TRANSFER: CMD=%04X -> %04X CH=%d\nADDR=%08X"), cmd, (cmd & 0xff00) | (data & 0xff), selch, dma[selch].areg);
		}
		break;
	case 0x0a:
		if((selch == 3)) {
			out_debug_log(_T("SET MODE[%d] to %02X"), selch, data);
		}
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
		break;
	default:
		break;
	}
	UPD71071::write_io8(addr, data);
}

uint32_t TOWNS_DMAC::read_io16(uint32_t addr)
{
	if(b16 != 0) {
		switch(addr & 0x0f) {
		case 0x02:
		case 0x03:
			if(base == 0) {
				return (dma[selch].creg & 0xffff);
			} else {
				return (dma[selch].bcreg & 0xffff);
			}
			break;
		case 0x04:
		case 0x05:
			if(base == 0) {
				return (dma[selch].areg & 0xffff);
			} else {
				return (dma[selch].bareg & 0xffff);
			}
			break;
		case 0x06:
		case 0x07:
			if(base == 0) {
				return ((dma[selch].areg >> 16) & 0xffff);
			} else {
				return ((dma[selch].bareg >> 16) & 0xffff);
			}
			break;
		case 0x08:
		case 0x09:
			return (uint32_t)(cmd & 0xffff);
			break;
		default:
			return read_io8(addr);
//			return read_io8(addr & 0x0e);
			break;
		}
	} else {
		pair16_t _d;
		_d.w = 0;
		_d.b.l = read_io8(addr);
//		_d.b.l = read_io8((addr & 0x0e) + 0);
//		_d.b.h = read_io8((addr & 0x0e) + 1);
		return (uint32_t)(_d.w);
	}
}

uint32_t TOWNS_DMAC::read_io8(uint32_t addr)
{
	uint32_t val;
	pair32_t _d;
	switch(addr & 0x0f) {
	case 0x01:
		return (base << 3) | (1 << (selch & 3));
		break;
	case 0x02:
	case 0x03:
		if(base == 0) {
			_d.d = dma[selch].creg;
#if !defined(USE_QUEUED_SCSI_TRANSFER)
			if((dma[selch].is_16bit) && !(inputs_ube[selch])) {
				if(!(creg_set[selch])) {
					_d.d >>= 1;
				}
			}
#endif
		} else {
			_d.d = dma[selch].bcreg;
#if !defined(USE_QUEUED_SCSI_TRANSFER)
			if((dma[selch].is_16bit) && !(inputs_ube[selch])) {
				if(!(bcreg_set[selch])) {
					_d.d >>= 1;
				}
			}
#endif
		}
		switch(addr & 0x0f) {
		case 2:
			return _d.b.l;
			break;
		case 3:
			return _d.b.h;
			break;
		}
		break;
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
	if(dma_wrap_reg != 0) {
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
	if(dma_wrap_reg != 0) {
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
	if(dma[c].creg == 0) {  // OK?
		// TC
		if(c == 3) {
			out_debug_log(_T("TRANSFER COMPLETED CH.3: AREG=%08X BAREG=%08X CREG=%08X BCREG=%08X"),
						  (dma[c].areg & 0xffffffff) ,
						  (dma[c].bareg & 0xffffffff) ,
						  dma[c].creg & 0x00ffffff,
						  dma[c].bcreg & 0x00ffffff
				);
						  
		}
	}
	return UPD71071::do_dma_epilogue(c);
}
	
uint32_t TOWNS_DMAC::read_signal(int id)
{
	if(SIG_TOWNS_DMAC_WRAP_REG) {
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
	
	if(_USE_DEBUGGER) {
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			d_debugger->write_via_debugger_data8(addr, val);
		} else {
			write_via_debugger_data8(addr, val);
		}
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
	if(_USE_DEBUGGER) {
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			val = d_debugger->read_via_debugger_data8(addr);
		} else {
			val = read_via_debugger_data8(addr);
		}
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
/*	
	if((addr & 1) != 0) {
		// If odd address, write a byte.
		uint32_t tval = (val >> 8) & 0xff;
		if(_USE_DEBUGGER) {
			if(d_debugger != NULL && d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data8(addr, tval);
			} else {
				write_via_debugger_data8(addr, tval);
			}
		} else {
			write_via_debugger_data8(addr, tval);
		}
	} else {
*/
		// 16bit
		if(_USE_DEBUGGER) {
			if(d_debugger != NULL && d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data16(addr, val);
			} else {
				write_via_debugger_data16(addr, val);
			}
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
	if(_USE_DEBUGGER) {
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			val = d_debugger->read_via_debugger_data16(addr);
		} else {
			val = this->read_via_debugger_data16(addr);
		}
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
