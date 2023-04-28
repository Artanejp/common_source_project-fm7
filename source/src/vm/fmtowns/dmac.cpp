
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
  	dma_wrap = true;
//	dma_wrap_reg = 0x00;
//	b16 = 2; // Fixed 16bit.
}

void TOWNS_DMAC::write_io8(uint32_t addr, uint32_t data)
{
//	if((addr & 0x0f) == 0x0c) out_debug_log("WRITE REG: %08X %08X", addr, data);
//	out_debug_log("WRITE REG: %04X %02X", addr, data);
	data &= 0xff;
	switch(addr & 0x0f) {
	case 0x04:
		dma[selch].bareg = (dma[selch].bareg & 0xffffff00) | data;
//		if(!base) {
			dma[selch].areg = (dma[selch].areg & 0xffffff00) | data;
//		}
		return;
		break;
	case 0x05:
		dma[selch].bareg = (dma[selch].bareg & 0xffff00ff) | (data << 8);
//		if(!base) {
			dma[selch].areg = (dma[selch].areg & 0xffff00ff) | (data << 8);
//		}
		return;
		break;
	case 0x06:
		dma[selch].bareg = (dma[selch].bareg & 0xff00ffff) | (data << 16);
//		if(!base) {
			dma[selch].areg = (dma[selch].areg & 0xff00ffff) | (data << 16);
//		}
		return;
		break;
	case 0x07:
		dma[selch].bareg = (dma[selch].bareg & 0x00ffffff) | (data << 24);
//		if(!base) {
			dma[selch].areg = (dma[selch].areg & 0x00ffffff) | (data << 24);
//		}
		return;
		break;
	default:
		break;
	}
	UPD71071::write_io8(addr, data);
}

uint32_t TOWNS_DMAC::read_io8(uint32_t addr)
{
	switch(addr & 0x0f) {
	case 0x07:
		if(base) {
			return (dma[selch].bareg >> 24) & 0xff;
		} else {
			return (dma[selch].areg >> 24) & 0xff;
		}
		break;
	}
	return UPD71071::read_io8(addr);
}

void TOWNS_DMAC::inc_dec_ptr_a_byte(const int c, const bool inc)
{
	// Note: FM-Towns may extend to 32bit.
#if 0
	uint32_t incdec = (inc) ? 1 : UINT32_MAX;
	uint32_t addr = dma[c].areg &   0x00ffffff;
	uint32_t high_a = dma[c].areg & 0xff000000;
	__LIKELY_IF(dma_wrap) {
		addr = (addr + incdec) & 0x00ffffff;
		addr = addr | high_a;
	} else {
		addr = (addr + incdec) & 0xffffffff;
	}
	dma[c].areg = addr;
#else
	uint32_t addr = dma[c].areg;
	if(inc) {
		addr = addr + 1;
	} else {
		addr = addr - 1;
	}
	__LIKELY_IF(dma_wrap) {
		uint32_t high_a = dma[c].areg & 0xff000000;
		addr = addr & 0x00ffffff;
		addr = addr | high_a;
 	}
	dma[c].areg = addr;
#endif

}

void TOWNS_DMAC::inc_dec_ptr_two_bytes(const int c, const bool inc)
{
	// Note: FM-Towns may extend to 32bit.
#if 0
	uint32_t incdec = (inc) ? 2 : (UINT32_MAX - 1);
	uint32_t addr = dma[c].areg &   0x00ffffff;
	uint32_t high_a = dma[c].areg & 0xff000000;
	__LIKELY_IF(dma_wrap) {
		addr = (addr + incdec) & 0x00ffffff;
		addr = addr | high_a;
	} else {
		addr = (addr + incdec) & 0xffffffff;
	}
	dma[c].areg = addr;
#else
	uint32_t addr = dma[c].areg;
	if(inc) {
		addr = addr + 2;
	} else {
		addr = addr - 2;
	}
	__LIKELY_IF(dma_wrap) {
		uint32_t high_a = dma[c].areg & 0xff000000;
		addr = addr & 0x00ffffff;
		addr = addr | high_a;
 	}
	dma[c].areg = addr;
#endif
}

void TOWNS_DMAC::do_dma()
{
	// check DDMA
	if(cmd & 4) {
		return;
	}

	// run dma
	bool is_use_debugger = false;
	if(__USE_DEBUGGER) {
		if(d_debugger != NULL) {
			is_use_debugger = d_debugger->now_device_debugging;
		}
	}

	for(int c = 0; c < 4; c++) {
		uint8_t bit = 1 << c;
		if(((req | sreq) & bit) && !(mask & bit)) {
			// execute dma
			while((req | sreq) & bit) {
				int wait = 0, wait_r = 0, wait_w = 0;
				bool compressed = ((cmd & 0x08) != 0);
				bool exptended = ((cmd & 0x20) != 0);

				if(!running) {
					wait += 2; // S0
					running = true;
				}
				if((dma[c].mode & 0x01) == 1) {
					// ToDo: Will check WORD transfer mode for FM-Towns.(mode.bit0 = '1).
					// 16bit transfer mode
					if((dma[c].mode & 0x0c) == 0x00) {
						// verify
						uint32_t val = dma[c].dev->read_dma_io16w(0, &wait_r);
						if(is_use_debugger) {
							val = d_debugger->read_via_debugger_data16w(dma[c].areg, &wait_w);
						} else {
							val = this->read_via_debugger_data16w(dma[c].areg, &wait_w);
						}
						wait += compressed ? 5 : 7;
						if(exptended) wait += wait_r + wait_w;
						// update temporary register
						tmp = val;
					} else if((dma[c].mode & 0x0c) == 0x04) {
						// io -> memory
						uint32_t val = dma[c].dev->read_dma_io16w(0, &wait_r);
						if(is_use_debugger) {
							d_debugger->write_via_debugger_data16w(dma[c].areg, val, &wait_w);
						} else {
							this->write_via_debugger_data16w(dma[c].areg, val, &wait_w);
						}
						wait += compressed ? 5 : 7;
						if(exptended) wait += wait_r + wait_w;
						// update temporary register
						tmp = val;
					} else if((dma[c].mode & 0x0c) == 0x08) {
						// memory -> io
						uint32_t val;
						if(is_use_debugger) {
							val = d_debugger->read_via_debugger_data16w(dma[c].areg, &wait_r);
						} else {
							val = this->read_via_debugger_data16w(dma[c].areg, &wait_r);
						}
						dma[c].dev->write_dma_io16w(0, val, &wait_w);
						wait += compressed ? 5 : 7;
						if(exptended) wait += wait_r + wait_w;
						// update temporary register
						tmp = val;
					}
					inc_dec_ptr_two_bytes(c, !(dma[c].mode & 0x20));
				} else {
					// 8bit transfer mode
					if((dma[c].mode & 0x0c) == 0x00) {
						// verify
						uint32_t val = dma[c].dev->read_dma_io8w(0, &wait_r);
						if(is_use_debugger) {
							val = d_debugger->read_via_debugger_data8w(dma[c].areg, &wait_w);
						} else {
							val = this->read_via_debugger_data8w(dma[c].areg, &wait_w);
						}
						wait += compressed ? 5 : 7;
						if(exptended) wait += wait_r + wait_w;
						// update temporary register
						tmp = (tmp >> 8) | (val << 8);
					} else if((dma[c].mode & 0x0c) == 0x04) {
						// io -> memory
						uint32_t val = dma[c].dev->read_dma_io8w(0, &wait_r);
						if(is_use_debugger) {
							d_debugger->write_via_debugger_data8w(dma[c].areg, val, &wait_w);
						} else {
							this->write_via_debugger_data8w(dma[c].areg, val, &wait_w);
						}
						wait += compressed ? 5 : 7;
						if(exptended) wait += wait_r + wait_w;
						// update temporary register
						tmp = (tmp >> 8) | (val << 8);
					} else if((dma[c].mode & 0x0c) == 0x08) {
						// memory -> io
						uint32_t val;
						if(is_use_debugger) {
							val = d_debugger->read_via_debugger_data8w(dma[c].areg, &wait_r);
						} else {
							val = this->read_via_debugger_data8w(dma[c].areg, &wait_r);
						}
						dma[c].dev->write_dma_io8w(0, val, &wait_w);
						wait += compressed ? 5 : 7;
						if(exptended) wait += wait_r + wait_w;
						// update temporary register
						tmp = (tmp >> 8) | (val << 8);
					}
					inc_dec_ptr_a_byte(c, !(dma[c].mode & 0x20));
				}
				if(d_cpu != NULL) d_cpu->set_extra_clock(wait);

				if(dma[c].creg-- == 0) {
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
					running = false;
					write_signals(&outputs_tc[c], 0xffffffff);
				} else if((dma[c].mode & 0xc0) == 0x40) {
					// single mode
					running = false;
					if(_SINGLE_MODE_DMA) {
						break;
					}
				}
			}
		}
	}
	if(_SINGLE_MODE_DMA) {
		if(d_dma) {
			d_dma->do_dma();
		}
	}
}

uint32_t TOWNS_DMAC::read_signal(int id)
{
	if(id == SIG_TOWNS_DMAC_WRAP) {
		return (dma_wrap) ? 0xffffffff : 0;
	}
	return UPD71071::read_signal(id);
}

void TOWNS_DMAC::write_signal(int id, uint32_t data, uint32_t _mask)
{
	if(id == SIG_TOWNS_DMAC_WRAP) {
		dma_wrap = (data  != 0) ? true : false;
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
					  _T("16Bit=%s ADDR_WRAP=%s \n")
					  _T("SELECT CH=%d BASE=%02X REQ=%02X SREQ=%02X MASK=%02X TC=%02X ")
					  _T("CMD=%04X TMP=%04X\n")
					  _T("%s")
					  _T("%s")
					  _T("%s")
					  _T("%s"),
					  (b16 != 0) ? _T("YES") : _T("NO"), (dma_wrap) ? _T("YES") : _T("NO"),
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

#define STATE_VERSION	5

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
	state_fio->StateValue(dma_wrap);
	return true;
}
}
