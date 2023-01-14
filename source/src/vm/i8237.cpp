/*
	Skelton for retropc emulator

	Origin : MESS
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ i8237 ]
*/

#include "i8237.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

void I8237::initialize()
{
#ifdef USE_DEBUGGER
	if(d_debugger != NULL) {
		d_debugger->set_device_name(_T("Debugger (8237 DMAC)"));
		d_debugger->set_context_mem(this);
		d_debugger->set_context_io(vm->dummy);
	}
#endif
}

void I8237::reset()
{
	low_high = false;
	cmd = req = tc = 0;
	mask = 0xff;
	running = false;
}

void I8237::write_io8(uint32_t addr, uint32_t data)
{
	int ch = (addr >> 1) & 3;
	uint8_t bit = 1 << (data & 3);
	
	switch(addr & 0x0f) {
	case 0x00: case 0x02: case 0x04: case 0x06:
		if(low_high) {
			dma[ch].bareg = (dma[ch].bareg & 0xff) | (data << 8);
		} else {
			dma[ch].bareg = (dma[ch].bareg & 0xff00) | data;
		}
		dma[ch].areg = dma[ch].bareg;
		low_high = !low_high;
		break;
	case 0x01: case 0x03: case 0x05: case 0x07:
		if(low_high) {
			dma[ch].bcreg = (dma[ch].bcreg & 0xff) | (data << 8);
		} else {
			dma[ch].bcreg = (dma[ch].bcreg & 0xff00) | data;
		}
		dma[ch].creg = dma[ch].bcreg;
		low_high = !low_high;
		break;
	case 0x08:
		// command register
		cmd = data;
		break;
	case 0x09:
		// request register
		if(data & 4) {
			if(!(req & bit)) {
				req |= bit;
#ifndef SINGLE_MODE_DMA
				do_dma();
#endif
			}
		} else {
			req &= ~bit;
		}
		break;
	case 0x0a:
		// single mask register
		if(data & 4) {
			mask |= bit;
		} else {
			mask &= ~bit;
		}
		break;
	case 0x0b:
		// mode register
		dma[data & 3].mode = data;
		break;
	case 0x0c:
		low_high = false;
		break;
	case 0x0d:
		// clear master
		reset();
		break;
	case 0x0e:
		// clear mask register
		mask = 0;
		break;
	case 0x0f:
		// all mask register
		mask = data & 0x0f;
		break;
	}
}

uint32_t I8237::read_io8(uint32_t addr)
{
	int ch = (addr >> 1) & 3;
	uint32_t val = 0xff;
	
	switch(addr & 0x0f) {
	case 0x00: case 0x02: case 0x04: case 0x06:
		if(low_high) {
			val = dma[ch].areg >> 8;
		} else {
			val = dma[ch].areg & 0xff;
		}
		low_high = !low_high;
		return val;
	case 0x01: case 0x03: case 0x05: case 0x07:
		if(low_high) {
			val = dma[ch].creg >> 8;
		} else {
			val = dma[ch].creg & 0xff;
		}
		low_high = !low_high;
		return val;
	case 0x08:
		// status register
		val = (req << 4) | tc;
		tc = 0;
		return val;
	case 0x0d:
		// temporary register
		return tmp & 0xff;
	}
	return 0xff;
}

void I8237::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(SIG_I8237_CH0 <= id && id <= SIG_I8237_CH3) {
		int ch = id - SIG_I8237_CH0;
		uint8_t bit = 1 << ch;
		if(data & mask) {
			if(!(req & bit)) {
				write_signals(&dma[ch].outputs_tc, 0);
				req |= bit;
#ifndef SINGLE_MODE_DMA
				do_dma();
#endif
			}
		} else {
			req &= ~bit;
		}
	} else if(SIG_I8237_BANK0 <= id && id <= SIG_I8237_BANK3) {
		// external bank registers
		int ch = id - SIG_I8237_BANK0;
		dma[ch].bankreg &= ~mask;
		dma[ch].bankreg |= data & mask;
	} else if(SIG_I8237_MASK0 <= id && id <= SIG_I8237_MASK3) {
		// external bank registers
		int ch = id - SIG_I8237_MASK0;
		dma[ch].incmask &= ~mask;
		dma[ch].incmask |= data & mask;
	}
}

uint32_t I8237::read_signal(int id)
{
	if(SIG_I8237_BANK0 <= id && id <= SIG_I8237_BANK3) {
		// external bank registers
		int ch = id - SIG_I8237_BANK0;
		return dma[ch].bankreg;
	} else if(SIG_I8237_MASK0 <= id && id <= SIG_I8237_MASK3) {
		// external bank registers
		int ch = id - SIG_I8237_MASK0;
		return dma[ch].incmask;
	}
	return 0;
}

// note: if SINGLE_MODE_DMA is defined, do_dma() is called in every machine cycle

void I8237::do_dma()
{
	for(int ch = 0; ch < 4; ch++) {
		uint8_t bit = 1 << ch;
		if((req & bit) && !(mask & bit)) {
			// execute dma
			while(req & bit) {
				int wait = 0, wait_r = 0, wait_w = 0;
				bool compressed = ((cmd & 0x09) == 0x08);
				bool exptended = ((cmd & 0x20) == 0x20) && !compressed;
				
				if(!running) {
					wait += 2; // S0
					running = true;
				}
				switch(dma[ch].mode & 0x0c) {
				case 0x00:
					// verify
					tmp = read_io(ch, &wait_r);
					wait += compressed ? 5 : 7;
					break;
				case 0x04:
					// io -> memory
					tmp = read_io(ch, &wait_r);
					write_mem(dma[ch].areg | (dma[ch].bankreg << 16), tmp, &wait_w);
					wait += compressed ? 5 : 7;
					if(exptended) wait += wait_r + wait_w;
					break;
				case 0x08:
					// memory -> io
					tmp = read_mem(dma[ch].areg | (dma[ch].bankreg << 16), &wait_r);
					write_io(ch, tmp, &wait_w);
					wait += compressed ? 5 : 7;
					if(exptended) wait += wait_r + wait_w;
					break;
				}
				if(d_cpu != NULL) d_cpu->set_extra_clock(wait);
				
				if(dma[ch].mode & 0x20) {
					dma[ch].areg--;
					if(dma[ch].areg == 0xffff) {
						dma[ch].bankreg = (dma[ch].bankreg & ~dma[ch].incmask) | ((dma[ch].bankreg - 1) & dma[ch].incmask);
					}
				} else {
					dma[ch].areg++;
					if(dma[ch].areg == 0) {
						dma[ch].bankreg = (dma[ch].bankreg & ~dma[ch].incmask) | ((dma[ch].bankreg + 1) & dma[ch].incmask);
					}
				}
				
				// check dma condition
				if(dma[ch].creg-- == 0) {
					// TC
					if(dma[ch].mode & 0x10) {
						// self initialize
						dma[ch].areg = dma[ch].bareg;
						dma[ch].creg = dma[ch].bcreg;
					} else {
						mask |= bit;
					}
					req &= ~bit;
					tc |= bit;
					running = false;
					write_signals(&dma[ch].outputs_tc, 0xffffffff);
				} else if((dma[ch].mode & 0xc0) == 0x40) {
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

void I8237::write_via_debugger_data8w(uint32_t addr, uint32_t data, int *wait)
{
	d_mem->write_dma_data8w(addr, data, wait);
}

uint32_t I8237::read_via_debugger_data8w(uint32_t addr, int *wait)
{
	return d_mem->read_dma_data8w(addr, wait);
}

void I8237::write_via_debugger_data16w(uint32_t addr, uint32_t data, int *wait)
{
	d_mem->write_dma_data16w(addr, data, wait);
}

uint32_t I8237::read_via_debugger_data16w(uint32_t addr, int *wait)
{
	return d_mem->read_dma_data16w(addr, wait);
}

void I8237::write_mem(uint32_t addr, uint32_t data, int *wait)
{
	if(mode_word) {
#ifdef USE_DEBUGGER
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			d_debugger->write_via_debugger_data16w((addr << 1) & addr_mask, data, wait);
		} else
#endif
		this->write_via_debugger_data16w((addr << 1) & addr_mask, data, wait);
	} else {
#ifdef USE_DEBUGGER
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			d_debugger->write_via_debugger_data8w(addr & addr_mask, data, wait);
		} else
#endif
		this->write_via_debugger_data8w(addr & addr_mask, data, wait);
	}
}

uint32_t I8237::read_mem(uint32_t addr, int *wait)
{
	if(mode_word) {
#ifdef USE_DEBUGGER
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			return d_debugger->read_via_debugger_data16w((addr << 1) & addr_mask, wait);
		} else
#endif
		return this->read_via_debugger_data16w((addr << 1) & addr_mask, wait);
	} else {
#ifdef USE_DEBUGGER
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			return d_debugger->read_via_debugger_data8w(addr & addr_mask, wait);
		} else
#endif
		return this->read_via_debugger_data8w(addr & addr_mask, wait);
	}
}

void I8237::write_io(int ch, uint32_t data, int *wait)
{
	if(mode_word) {
		dma[ch].dev->write_dma_io16w(0, data, wait);
	} else {
		dma[ch].dev->write_dma_io8w(0, data, wait);
	}
}

uint32_t I8237::read_io(int ch, int *wait)
{
	if(mode_word) {
		return dma[ch].dev->read_dma_io16w(0, wait);
	} else {
		return dma[ch].dev->read_dma_io8w(0, wait);
	}
}

#ifdef USE_DEBUGGER
bool I8237::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
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
	dma[0].areg, dma[0].creg, dma[0].bareg, dma[0].bcreg, (req >> 0) & 1, (mask >> 0) & 1, dma[0].mode, dir[(dma[0].mode >> 2) & 3],
	dma[1].areg, dma[1].creg, dma[1].bareg, dma[1].bcreg, (req >> 1) & 1, (mask >> 1) & 1, dma[1].mode, dir[(dma[1].mode >> 2) & 3],
	dma[2].areg, dma[2].creg, dma[2].bareg, dma[2].bcreg, (req >> 2) & 1, (mask >> 2) & 1, dma[2].mode, dir[(dma[2].mode >> 2) & 3],
	dma[3].areg, dma[3].creg, dma[3].bareg, dma[3].bcreg, (req >> 3) & 1, (mask >> 3) & 1, dma[3].mode, dir[(dma[3].mode >> 2) & 3]);
	return true;
}
#endif

#define STATE_VERSION	3

bool I8237::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < 4; i++) {
		state_fio->StateValue(dma[i].areg);
		state_fio->StateValue(dma[i].creg);
		state_fio->StateValue(dma[i].bareg);
		state_fio->StateValue(dma[i].bcreg);
		state_fio->StateValue(dma[i].mode);
		state_fio->StateValue(dma[i].bankreg);
		state_fio->StateValue(dma[i].incmask);
	}
	state_fio->StateValue(low_high);
	state_fio->StateValue(cmd);
	state_fio->StateValue(req);
	state_fio->StateValue(mask);
	state_fio->StateValue(tc);
	state_fio->StateValue(tmp);
	state_fio->StateValue(mode_word);
	state_fio->StateValue(addr_mask);
	state_fio->StateValue(running);
	return true;
}

