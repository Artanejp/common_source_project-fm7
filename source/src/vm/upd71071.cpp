/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ uPD71071 ]
*/

#include "upd71071.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

void UPD71071::initialize()
{
	for(int i = 0; i < 4; i++) {
		dma[i].areg = dma[i].bareg = 0;
		dma[i].creg = dma[i].bcreg = 0;
	}
#ifdef USE_DEBUGGER
	if(d_debugger != NULL) {
		d_debugger->set_device_name(_T("Debugger (uPD71071 DMAC)"));
		d_debugger->set_context_mem(this);
		d_debugger->set_context_io(vm->dummy);
	}
#endif
}

void UPD71071::reset()
{
	for(int i = 0; i < 4; i++) {
		dma[i].mode = 0x04;
	}
	b16 = selch = base = 0;
	cmd = tmp = 0;
	req = sreq = tc = 0;
	mask = 0x0f;
}

void UPD71071::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0x0f) {
	case 0x00:
		if(data & 1) {
			// dma reset
			for(int i = 0; i < 4; i++) {
				dma[i].mode = 0x04;
			}
			selch = base = 0;
			cmd = tmp = 0;
			sreq = tc = 0;
			mask = 0x0f;
		}
		b16 = data & 2;
		break;
	case 0x01:
		selch = data & 3;
		base = data & 4;
		break;
	case 0x02:
		dma[selch].bcreg = (dma[selch].bcreg & 0xff00) | data;
//		if(!base) {
			dma[selch].creg = (dma[selch].creg & 0xff00) | data;
//		}
		break;
	case 0x03:
		dma[selch].bcreg = (dma[selch].bcreg & 0x00ff) | (data << 8);
//		if(!base) {
			dma[selch].creg = (dma[selch].creg & 0x00ff) | (data << 8);
//		}
		break;
	case 0x04:
		dma[selch].bareg = (dma[selch].bareg & 0xffff00) | data;
//		if(!base) {
			dma[selch].areg = (dma[selch].areg & 0xffff00) | data;
//		}
		break;
	case 0x05:
		dma[selch].bareg = (dma[selch].bareg & 0xff00ff) | (data << 8);
//		if(!base) {
			dma[selch].areg = (dma[selch].areg & 0xff00ff) | (data << 8);
//		}
		break;
	case 0x06:
		dma[selch].bareg = (dma[selch].bareg & 0x00ffff) | (data << 16);
//		if(!base) {
			dma[selch].areg = (dma[selch].areg & 0x00ffff) | (data << 16);
//		}
		break;
	case 0x08:
		cmd = (cmd & 0xff00) | data;
		break;
	case 0x09:
		cmd = (cmd & 0xff) | (data << 8);
		break;
	case 0x0a:
		dma[selch].mode = data;
		break;
	case 0x0e:
		if((sreq = data) != 0) {
#ifndef SINGLE_MODE_DMA
			do_dma();
#endif
		}
		break;
	case 0x0f:
		mask = data;
		break;
	}
}

uint32_t UPD71071::read_io8(uint32_t addr)
{
	uint32_t val;
	
	switch(addr & 0x0f) {
	case 0x00:
		return b16;
	case 0x01:
		return (base << 2) | (1 << selch);
	case 0x02:
		if(base) {
			return dma[selch].bcreg & 0xff;
		} else {
			return dma[selch].creg & 0xff;
		}
	case 0x03:
		if(base) {
			return (dma[selch].bcreg >> 8) & 0xff;
		} else {
			return (dma[selch].creg >> 8) & 0xff;
		}
	case 0x04:
		if(base) {
			return dma[selch].bareg & 0xff;
		} else {
			return dma[selch].areg & 0xff;
		}
	case 0x05:
		if(base) {
			return (dma[selch].bareg >> 8) & 0xff;
		} else {
			return (dma[selch].areg >> 8) & 0xff;
		}
	case 0x06:
		if(base) {
			return (dma[selch].bareg >> 16) & 0xff;
		} else {
			return (dma[selch].areg >> 16) & 0xff;
		}
	case 0x08:
		return cmd & 0xff;
	case 0x09:
		return (cmd >> 8) & 0xff;
	case 0x0a:
		return dma[selch].mode;
	case 0x0b:
		val = (req << 4) | tc;
		tc = 0;
		return val;
	case 0x0c:
		return tmp & 0xff;
	case 0x0d:
		return (tmp >> 8) & 0xff;
	case 0x0e:
		return sreq;
	case 0x0f:
		return mask;
	}
	return 0xff;
}

void UPD71071::write_signal(int id, uint32_t data, uint32_t mask)
{
	uint8_t bit = 1 << (id & 3);
	
	if(data & mask) {
		if(!(req & bit)) {
			req |= bit;
#ifndef SINGLE_MODE_DMA
			do_dma();
#endif
		}
	} else {
		req &= ~bit;
	}
}

void UPD71071::write_via_debugger_data8w(uint32_t addr, uint32_t data, int *wait)
{
	d_mem->write_dma_data8w(addr, data, wait);
}

uint32_t UPD71071::read_via_debugger_data8w(uint32_t addr, int *wait)
{
	return d_mem->read_dma_data8w(addr, wait);
}

void UPD71071::write_via_debugger_data16w(uint32_t addr, uint32_t data, int *wait)
{
	d_mem->write_dma_data16w(addr, data, wait);
}

uint32_t UPD71071::read_via_debugger_data16w(uint32_t addr, int *wait)
{
	return d_mem->read_dma_data16w(addr, wait);
}

// note: if SINGLE_MODE_DMA is defined, do_dma() is called in every machine cycle

void UPD71071::do_dma()
{
	// check DDMA
	if(cmd & 4) {
		return;
	}
	
	// run dma
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
/*
					// ToDo: Will check WORD transfer mode for FM-Towns.(mode.bit0 = '1).
					// 16bit transfer mode
					if((dma[c].mode & 0x0c) == 0x00) {
						// verify
						uint32_t val = dma[c].dev->read_dma_io16w(0, &wait_r);
#ifdef USE_DEBUGGER
						if(d_debugger != NULL && d_debugger->now_device_debugging) {
							val = d_debugger->read_via_debugger_data16w(dma[c].areg, &wait_w);
						} else
#endif
						val = this->read_via_debugger_data16w(dma[c].areg, &wait_w);
						wait += compressed ? 5 : 7;
						if(cmd & 0x20) wait += wair_r + wait_w;
						// update temporary register
						tmp = val;
					} else if((dma[c].mode & 0x0c) == 0x04) {
						// io -> memory
						uint32_t val = dma[c].dev->read_dma_io16w(0, &wait_r);
#ifdef USE_DEBUGGER
						if(d_debugger != NULL && d_debugger->now_device_debugging) {
							d_debugger->write_via_debugger_data16w(dma[c].areg, val, &wait_w);
						} else
#endif
						this->write_via_debugger_data16w(dma[c].areg, val, &wait_w);
						wait += compressed ? 5 : 7;
						if(exptended) wait += wait_r + wait_w;
						// update temporary register
						tmp = val;
					} else if((dma[c].mode & 0x0c) == 0x08) {
						// memory -> io
						uint32_t val;
#ifdef USE_DEBUGGER
						if(d_debugger != NULL && d_debugger->now_device_debugging) {
							val = d_debugger->read_via_debugger_data16w(dma[c].areg, &wait_r);
						} else
#endif
						val = this->read_via_debugger_data16w(dma[c].areg, &wait_r);
						dma[c].dev->write_dma_io16w(0, val, &wait_w);
						wait += compressed ? 5 : 7;
						if(exptended) wait += wait_r + wait_w;
						// update temporary register
						tmp = val;
					}
					if(dma[c].mode & 0x20) {
						dma[c].areg = (dma[c].areg - 2) & 0xffffff;
					} else {
						dma[c].areg = (dma[c].areg + 2) & 0xffffff;
					}
*/
				} else {
					// 8bit transfer mode
					if((dma[c].mode & 0x0c) == 0x00) {
						// verify
						uint32_t val = dma[c].dev->read_dma_io8w(0, &wait_r);
#ifdef USE_DEBUGGER
						if(d_debugger != NULL && d_debugger->now_device_debugging) {
							val = d_debugger->read_via_debugger_data8w(dma[c].areg, &wait_w);
						} else
#endif
						val = this->read_via_debugger_data8w(dma[c].areg, &wait_w);
						wait += compressed ? 5 : 7;
						if(cmd & 0x20) wait += wait_r + wait_w;
						// update temporary register
						tmp = (tmp >> 8) | (val << 8);
					} else if((dma[c].mode & 0x0c) == 0x04) {
						// io -> memory
						uint32_t val = dma[c].dev->read_dma_io8w(0, &wait_r);
#ifdef USE_DEBUGGER
						if(d_debugger != NULL && d_debugger->now_device_debugging) {
							d_debugger->write_via_debugger_data8w(dma[c].areg, val, &wait_w);
						} else
#endif
						this->write_via_debugger_data8w(dma[c].areg, val, &wait_w);
						wait += compressed ? 5 : 7;
						if(exptended) wait += wait_r + wait_w;
						// update temporary register
						tmp = (tmp >> 8) | (val << 8);
					} else if((dma[c].mode & 0x0c) == 0x08) {
						// memory -> io
						uint32_t val;
#ifdef USE_DEBUGGER
						if(d_debugger != NULL && d_debugger->now_device_debugging) {
							val = d_debugger->read_via_debugger_data8w(dma[c].areg, &wait_r);
						} else
#endif
						val = this->read_via_debugger_data8w(dma[c].areg, &wait_r);
						dma[c].dev->write_dma_io8w(0, val, &wait_w);
						wait += compressed ? 5 : 7;
						if(exptended) wait += wait_r + wait_w;
						// update temporary register
						tmp = (tmp >> 8) | (val << 8);
					}
					if(dma[c].mode & 0x20) {
						dma[c].areg = (dma[c].areg - 1) & 0xffffff;
					} else {
						dma[c].areg = (dma[c].areg + 1) & 0xffffff;
					}
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
					write_signals(&outputs_tc, 0xffffffff);
				} else if((dma[c].mode & 0xc0) == 0x40) {
					// single mode
					running = false;
#ifdef SINGLE_MODE_DMA
					break;
#endif
				}
			}
		}
	}
#ifdef SINGLE_MODE_DMA
	if(d_dma) {
		d_dma->do_dma();
	}
#endif
}

#ifdef USE_DEBUGGER
bool UPD71071::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
/*
CH0 AREG=FFFF CREG=FFFF BAREG=FFFF BCREG=FFFF REQ=1 MASK=1 MODE=FF MEM->I/O
CH1 AREG=FFFF CREG=FFFF BAREG=FFFF BCREG=FFFF REQ=1 MASK=1 MODE=FF I/O->MEM
CH2 AREG=FFFF CREG=FFFF BAREG=FFFF BCREG=FFFF REQ=1 MASK=1 MODE=FF VERIFY
CH3 AREG=FFFF CREG=FFFF BAREG=FFFF BCREG=FFFF REQ=1 MASK=1 MODE=FF INVALID
*/
	static const _TCHAR *dir[4] = {
		_T("VERIFY"), _T("I/O->MEM"), _T("MEM->I/O"), _T("INVALID")
	};
	my_stprintf_s(buffer, buffer_len,
	_T("CH0 AREG=%04X CREG=%04X BAREG=%04X BCREG=%04X REQ=%d MASK=%d MODE=%02X %s\n")
	_T("CH1 AREG=%04X CREG=%04X BAREG=%04X BCREG=%04X REQ=%d MASK=%d MODE=%02X %s\n")
	_T("CH2 AREG=%04X CREG=%04X BAREG=%04X BCREG=%04X REQ=%d MASK=%d MODE=%02X %s\n")
	_T("CH3 AREG=%04X CREG=%04X BAREG=%04X BCREG=%04X REQ=%d MASK=%d MODE=%02X %s"),
	dma[0].areg, dma[0].creg, dma[0].bareg, dma[0].bcreg, ((req | sreq) >> 0) & 1, (mask >> 0) & 1, dma[0].mode, dir[(dma[0].mode >> 2) & 3],
	dma[1].areg, dma[1].creg, dma[1].bareg, dma[1].bcreg, ((req | sreq) >> 1) & 1, (mask >> 1) & 1, dma[1].mode, dir[(dma[1].mode >> 2) & 3],
	dma[2].areg, dma[2].creg, dma[2].bareg, dma[2].bcreg, ((req | sreq) >> 2) & 1, (mask >> 2) & 1, dma[2].mode, dir[(dma[2].mode >> 2) & 3],
	dma[3].areg, dma[3].creg, dma[3].bareg, dma[3].bcreg, ((req | sreq) >> 3) & 1, (mask >> 3) & 1, dma[3].mode, dir[(dma[3].mode >> 2) & 3]);
	return true;
}
#endif

#define STATE_VERSION	2

bool UPD71071::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < 4; i++) {
		state_fio->StateValue(dma[i].areg);
		state_fio->StateValue(dma[i].bareg);
		state_fio->StateValue(dma[i].creg);
		state_fio->StateValue(dma[i].bcreg);
		state_fio->StateValue(dma[i].mode);
	}
	state_fio->StateValue(b16);
	state_fio->StateValue(selch);
	state_fio->StateValue(base);
	state_fio->StateValue(cmd);
	state_fio->StateValue(tmp);
	state_fio->StateValue(req);
	state_fio->StateValue(sreq);
	state_fio->StateValue(mask);
	state_fio->StateValue(tc);
	state_fio->StateValue(running);
	return true;
}

