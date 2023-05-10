
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
	for(int ch = 0; ch < 4; ch++) {
		end_req[ch] = false;
		end_stat[ch] = false;

		is_16bit_transfer[ch] = (((dma[ch].mode & 0x01) == 1) && b16);
		write_signals(&outputs_tc[ch], ((tc & (1 << ch)) != 0) ? 0xffffffff : 0);
		write_signals(&outputs_ube[ch], (is_16bit_transfer[ch] || force_16bit_transfer[ch]) ? 0xffffffff : 0);

	}
//	dma_wrap_reg = 0x00;
//	b16 = 2; // Fixed 16bit.
}

void TOWNS_DMAC::write_io8(uint32_t addr, uint32_t data)
{
//	if((addr & 0x0f) == 0x0c) out_debug_log("WRITE REG: %08X %08X", addr, data);
//	out_debug_log("WRITE REG: %04X %02X", addr, data);
	data &= 0xff;
	uint8_t cmd_bak = cmd;
	bool need_transfer = false;
	switch(addr & 0x0f) {
	case 0x00:
		UPD71071::write_io8(0, data);
		for(int ch = 0; ch < 4; ch++) {
			is_16bit_transfer[ch] = (((dma[ch].mode & 0x01) == 1) && b16);
			if(data & 1) {
				end_req[ch] = false;
				end_stat[ch] = false;
				write_signals(&outputs_ube[ch], (is_16bit_transfer[ch] || force_16bit_transfer[ch]) ? 0xffffffff : 0);
			}
			write_signals(&outputs_tc[ch], ((tc & (1 << ch)) != 0) ? 0xffffffff : 0);
		}
		out_debug_log(_T("RESET from I/O; B16=%s"), ((b16 & 2) != 0) ? _T("16bit") : _T("8bit"));
		break;
	case 0x03:
		// OK?
		end_req[selch] = false;
		end_stat[selch] = false;
		UPD71071::write_io8(addr, data);
		//tc = tc & ~(1 << selch);
		//write_signals(&outputs_tc[selch], 0);
		break;
	case 0x04:
		dma[selch].bareg = (dma[selch].bareg & 0xffffff00) | data;
//		if(!base) {
			dma[selch].areg = (dma[selch].areg & 0xffffff00) | data;
//		}
		break;
	case 0x05:
		dma[selch].bareg = (dma[selch].bareg & 0xffff00ff) | (data << 8);
//		if(!base) {
			dma[selch].areg = (dma[selch].areg & 0xffff00ff) | (data << 8);
//		}
		break;
	case 0x06:
		dma[selch].bareg = (dma[selch].bareg & 0xff00ffff) | (data << 16);
//		if(!base) {
			dma[selch].areg = (dma[selch].areg & 0xff00ffff) | (data << 16);
//		}
		break;
	case 0x07:
		dma[selch].bareg = (dma[selch].bareg & 0x00ffffff) | (data << 24);
//		if(!base) {
			dma[selch].areg = (dma[selch].areg & 0x00ffffff) | (data << 24);
//		}
		break;
	case 0x08:
	case 0x09:
		UPD71071::write_io8(addr, data);
		if(((cmd & 0x04) == 0) && ((cmd_bak & 0x04) != 0)) {
			out_debug_log(_T("CMD CHANGED from %04X to 0x%04X : MAYBE START to TRANSFER"),
						  cmd_bak, cmd);
			static const _TCHAR *dir[4] = {
				_T("VERIFY"), _T("I/O->MEM"), _T("MEM->I/O"), _T("INVALID")
			};
			for(int ch = 0; ch < 4; ch++) {
				out_debug_log(_T("CH%d AREG=%08X CREG=%04X BAREG=%08X BCREG=%04X REQ=%d MASK=%d MODE=%02X %s"),
							  ch, dma[ch].areg, dma[ch].creg, dma[ch].bareg, dma[ch].bcreg,
							  ((req | sreq) >> ch) & 1,
							  (mask >> ch) & 1,
							  dma[ch].mode,
							  dir[(dma[ch].mode >> 2) & 3]);
			}
		} else {
			out_debug_log(_T("CMD CHANGED from %04X to 0x%04X"), cmd_bak, cmd);
		}
		break;
	case 0x0a:
		UPD71071::write_io8(addr, data);
		is_16bit_transfer[selch] = (((dma[selch].mode & 0x01) == 1) && b16);
		if((data & 0x04) == 0) {
			end_req[selch] = false;
			end_stat[selch] = false;
		}
		write_signals(&outputs_ube[selch], (is_16bit_transfer[selch] || force_16bit_transfer[selch]) ? 0xffffffff : 0);
		out_debug_log(_T("MODE CHANGED at CH.%d to 0x%02X Request 16bit=%s"), selch, dma[selch].mode,
					  (is_16bit_transfer[selch]) ? _T("Yes") : _T("NO"));
		break;
//	case 0x0b:
//		UPD71071::write_io8(addr, data);
//		// Q: Is reset TC* ? 20230429 K.O
//		for(int ch = 0; ch < 4; ch++) {
//			write_signals(&outputs_tc[ch], 0);
//		}
//		break;
	case 0x0f:
		UPD71071::write_io8(addr, data);
		// Add trigger of transfer by SREQ.
		need_transfer = (!(_SINGLE_MODE_DMA) && ((cmd & 0x04) == 0));
		break;
	default:
		UPD71071::write_io8(addr, data);
		break;
	}
	if(need_transfer) {
		do_dma();
	}
}

uint32_t TOWNS_DMAC::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	switch(addr & 0x0f) {
	case 0x07:
		if(base) {
			val = (dma[selch].bareg >> 24) & 0xff;
		} else {
			val = (dma[selch].areg >> 24) & 0xff;
		}
		break;
	case 0x0b:
		val = UPD71071::read_io8(addr);
		// Q: Is reset TC* ? 20230429 K.O
		for(int ch = 0; ch < 4; ch++) {
			write_signals(&outputs_tc[ch], 0);
		}
		break;
	default:
		val = UPD71071::read_io8(addr);
		break;
	}
	return val;
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

uint32_t TOWNS_DMAC::read_16bit_from_device(DEVICE* dev, uint32_t addr, int* wait)
{

	__UNLIKELY_IF(dev == nullptr) {
		if(wait != nullptr) {
			*wait = 0; // ToDo
		}
		return 0xffff;
	}
	pair32_t tmp;
	int wait_h, wait_l;
	tmp.d = 0;
//	tmp.b.l = dev->read_dma_io8w(addr, &wait_l);
//	tmp.b.h = dev->read_dma_io8w(addr, &wait_h);
//	__LIKELY_IF(wait != nullptr) {
//		*wait = wait_l + wait_h;
//	}
	tmp.w.l = dev->read_dma_io16w(addr, wait);
	return tmp.d;
}

void TOWNS_DMAC::write_16bit_to_device(DEVICE* dev, uint32_t addr, uint32_t data, int* wait)
{

	__UNLIKELY_IF(dev == nullptr) {
		if(wait != nullptr) {
			*wait = 0; // ToDo
		}
		return;
	}
	pair32_t tmp;
	int wait_h, wait_l;

	tmp.d = data;
//	dev->write_dma_io8w(addr, tmp.b.l, &wait_l);
//	dev->write_dma_io8w(addr, tmp.b.h, &wait_h);
//	__LIKELY_IF(wait != nullptr) {
//		*wait = wait_l + wait_h;
//	}
	dev->write_dma_io16w(addr, tmp.w.l, wait);
}

uint32_t TOWNS_DMAC::read_16bit_from_memory(uint32_t addr, int* wait, bool is_use_debugger)
{
	__UNLIKELY_IF(is_use_debugger) {
		return d_debugger->read_via_debugger_data16w(addr, wait);
	} else {
		return this->read_via_debugger_data16w(addr, wait);
	}
}

void TOWNS_DMAC::write_16bit_to_memory(uint32_t addr, uint32_t data, int* wait, bool is_use_debugger)
{
	__UNLIKELY_IF(is_use_debugger) {
		d_debugger->write_via_debugger_data16w(addr, data, wait);
	} else {
		this->write_via_debugger_data16w(addr, data, wait);
	}
}

bool TOWNS_DMAC::do_dma_per_channel(int ch, bool is_use_debugger, bool force_exit)
{
	int c = ch & 3;
	uint8_t bit = 1 << c;
	// This is workaround for FM-Towns's SCSI.
	bool is_16bit = (is_16bit_transfer[c] || force_16bit_transfer[c]);
	write_signals(&outputs_ube[c], (is_16bit) ? 0xffffffff : 0x00000000); // Reset UBE
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
			if(is_16bit) {
				// ToDo: Will check WORD transfer mode for FM-Towns.(mode.bit0 = '1).
				// 16bit transfer mode
				if((dma[c].mode & 0x0c) == 0x00) {
					// verify
					uint32_t val = read_16bit_from_device(dma[c].dev, 0, &wait_r);
					read_16bit_from_memory(dma[c].areg, &wait_w, is_use_debugger);
					wait += compressed ? 5 : 7;
					if(exptended) wait += wait_r + wait_w;
					// update temporary register
					tmp = val;
				} else if((dma[c].mode & 0x0c) == 0x04) {
					// io -> memory
					uint32_t val = read_16bit_from_device(dma[c].dev, 0, &wait_r);
					write_16bit_to_memory(dma[c].areg, val, &wait_w, is_use_debugger);
					wait += compressed ? 5 : 7;
					if(exptended) wait += wait_r + wait_w;
					// update temporary register
					tmp = val;
				} else if((dma[c].mode & 0x0c) == 0x08) {
					// memory -> io
					uint32_t val;
					val = read_16bit_from_memory(dma[c].areg, &wait_r, is_use_debugger);
					write_16bit_to_device(dma[c].dev, 0, val, &wait_w);
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

			// Note: At FM-Towns, SCSI's DMAC will be set after
			//       SCSI bus phase become DATA IN/DATA OUT.
			//       Before bus phase became DATA IN/DATA OUT,
			//       DMAC mode and state was unstable (and ASSERTED
			//       DRQ came from SCSI before this state change).
			// ToDo: Stop correctly before setting.
			__UNLIKELY_IF((dma[c].creg == 0) || ((end_req[c]) && !(end_stat[c]) && ((dma[c].mode & 0xc0) != 0x40))) {  // OK?
				// TC
				bool is_tc = false;
				if((end_req[c]) && !(end_stat[c]) && ((dma[c].mode & 0xc0) != 0x40)) {
					end_stat[c] = true;
					is_tc = true;
				}
				if(dma[c].creg == 0) {
					is_tc |= true;
				}
				dma[c].creg--;
				if(dma[c].mode & 0x10) {
					// auto initialize
					dma[c].areg = dma[c].bareg;
					dma[c].creg = dma[c].bcreg;
				} else {
					mask |= bit;
				}
				req &= ~bit;
				sreq &= ~bit;
				if(is_tc) {
					tc |= bit;
				}
				running = false;
				if(is_tc) {
					write_signals(&outputs_tc[c], 0xffffffff);
				}
				if(((dma[c].mode & 0xc0) == 0x40) || (_SINGLE_MODE_DMA)) {
					return true; // Single mode
				}
			} else {
				dma[c].creg--;
				uint8_t _mode = dma[c].mode & 0xc0;
				if((_mode == 0x40) || (_mode == 0x00)) {
					req &= ~bit;
					sreq &= ~bit;
					if(_mode == 0x40) {
						// Single
						running = false;
					}
					if(_SINGLE_MODE_DMA) {
						return true;
					}
				}
			}
			__UNLIKELY_IF(force_exit) {
				break;
			}
		}
	}
	return false;
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
		if(do_dma_per_channel(c, is_use_debugger, false)) {
			break;
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
	} else if((id >= SIG_TOWNS_DMAC_EOT_CH0) && (id <= SIG_TOWNS_DMAC_EOT_CH3)) {
		if((cmd & 0x04) == 0) {
			int ch = id - SIG_TOWNS_DMAC_EOT_CH0;
			switch(dma[ch].mode & 0xc0) {
			case 0x00: // Demand
			case 0x80: // Block
				end_req[ch] = true;
				break;
			//case 0xc0: // Cascade
			//case 0x40: // Single
			default:
				break;
			}
		}
	} else {
		// Fallthrough.
		__LIKELY_IF((id >= SIG_UPD71071_CH0) && (id <= SIG_UPD71071_CH3)) {
	//		out_debug_log(_T("DRQ#%d %s"), ch, ((data & _mask) != 0) ? _T("ON ") : _T("OFF"));
			int ch = (id - SIG_UPD71071_CH0) & 3;
			uint8_t bit = 1 << ch;
			if(data & _mask) {
				if(!(req & bit)) {
					req |= bit;
					if(!(_SINGLE_MODE_DMA)) {
						if((_mask & bit) == 0) { // MASK register MASKS DRQ.20200918 K.O
							// Without #define SINGLE_MODE_DMA ,
							// DMA trasfer is triggerd by SIGNAL or writing I/O 0Eh.
							bool is_use_debugger = false;
							if(__USE_DEBUGGER) {
								if(d_debugger != NULL) {
									is_use_debugger = d_debugger->now_device_debugging;
								}
							}
							do_dma_per_channel(ch, is_use_debugger, true);
							req &= ~bit;
						}
					}
				}
			} else {
				req &= ~bit;
			}
		} else {
			UPD71071::write_signal(id, data, _mask);
		}
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

#define STATE_VERSION	6

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
	state_fio->StateArray(is_16bit_transfer, sizeof(is_16bit_transfer), 1);
	state_fio->StateArray(end_stat, sizeof(end_stat), 1);
	state_fio->StateArray(end_stat, sizeof(end_stat), 1);
	return true;
}
}
