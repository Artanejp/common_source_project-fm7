/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2023.06.09-

	[ i8257 ]
*/

#include "i8257.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

void I8257::initialize()
{
#ifdef USE_DEBUGGER
	if(d_debugger != NULL) {
		d_debugger->set_device_name(_T("Debugger (8257 DMAC)"));
		d_debugger->set_context_mem(this);
		d_debugger->set_context_io(vm->dummy);
	}
#endif
}

void I8257::reset()
{
	for(int c = 0; c < 4; c++) {
		ch[c].addr.d = ch[c].count.d = 0;
		ch[c].mode = 0;
		ch[c].running = 0;
	}
	mode = status = 0;
	high_low = 0;
}

void I8257::write_io8(uint32_t addr, uint32_t data)
{
	int c = (addr >> 1) & 3;
	
	switch(addr & 0x0f) {
	case 0x00:
	case 0x02: case 0x0a:
	case 0x04: case 0x0c:
	case 0x06: case 0x0e:
		if(!high_low) {
			if((mode & 0x80) && c == 2) {
				ch[3].addr.b.l = data;
			}
			ch[c].addr.b.l = data;
		} else {
			if((mode & 0x80) && c == 2) {
				ch[3].addr.b.h = data;
				ch[3].addr.b.h2 = ch[3].addr.b.h3 = 0;
			}
			ch[c].addr.b.h = data;
			ch[c].addr.b.h2 = ch[c].addr.b.h3 = 0;
		}
		high_low = !high_low;
		break;
	case 0x01: case 0x09:
	case 0x03: case 0x0b:
	case 0x05: case 0x0d:
	case 0x07: case 0x0f:
		if(!high_low) {
			if((mode & 0x80) && c == 2) {
				ch[3].count.b.l = data;
			}
			ch[c].count.b.l = data;
		} else {
			if((mode & 0x80) && c == 2) {
				ch[3].count.b.h = data & 0x3f;
				ch[3].count.b.h2 = ch[3].count.b.h3 = 0;
//				ch[3].mode = data & 0xc0;
			}
			ch[c].count.b.h = data & 0x3f;
			ch[c].count.b.h2 = ch[c].count.b.h3 = 0;
			ch[c].mode = data & 0xc0;
		}
		high_low = !high_low;
		break;
	case 0x08:
		if(!(data & 0x80)) {
			status &= ~0x10;
		}
		mode = data;
		high_low = false;
		break;
	}
}

uint32_t I8257::read_io8(uint32_t addr)
{
	uint32_t val = 0xff;
	int c = (addr >> 1) & 3;
	
	switch(addr & 0x0f) {
	case 0x00:
	case 0x02: case 0x0a:
	case 0x04: case 0x0c:
	case 0x06: case 0x0e:
		if(!high_low) {
			val = ch[c].addr.b.l;
		} else {
			val = ch[c].addr.b.h;
		}
		high_low = !high_low;
		break;
	case 0x01: case 0x09:
	case 0x03: case 0x0b:
	case 0x05: case 0x0d:
	case 0x07: case 0x0f:
		if(!high_low) {
			val = ch[c].count.b.l;
		} else {
			val = (ch[c].count.b.h & 0x3f) | ch[c].mode;
		}
		high_low = !high_low;
		break;
	case 0x08:
		val = status;
		status &= 0xf0;
//		high_low = false;
		break;
	}
	return val;
}

void I8257::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(SIG_I8257_CH0 <= id && id <= SIG_I8257_CH3) {
		int c = id - SIG_I8257_CH0;
		if(data & mask) {
			if(mode & (1 << c)) {
				status &= ~(1 << c);
				ch[c].running = true;
			} else {
				ch[c].running = false;
			}
#ifndef SINGLE_MODE_DMA
			if(ch[c].running) {
				do_dma(c);
			}
#endif
		} else {
			ch[c].running = false;
		}
	}
}

// note: if SINGLE_MODE_DMA is defined, do_dma() is called in every machine cycle

void I8257::do_dma()
{
	for(int c = 0; c < 4; c++) {
		do_dma(c);
	}
}

void I8257::do_dma(int c)
{
	while(ch[c].running) {
		if(ch[c].count.sd >= 0) {
			int wait_r = 0, wait_w = 0;
			uint32_t tmp;
			switch(ch[c].mode) {
			case 0x00:
				// verify
				tmp = read_io(c, &wait_r);
				break;
			case 0x40:
				// io -> memory
				tmp = read_io(c, &wait_r);
				write_mem(ch[c].addr.w.l, tmp, &wait_w);
				break;
			case 0x80:
				// memory -> io
				tmp = read_mem(ch[c].addr.w.l, &wait_r);
				write_io(c, tmp, &wait_w);
				break;
			}
			if(d_cpu != NULL) d_cpu->set_extra_clock(4 + wait_r + wait_w);
			
			ch[c].addr.sd++;
			ch[c].count.sd--;
			status &= ~0x10;
		}
		if(ch[c].count.sd < 0) {
			if((mode & 0x80) && c == 2) {
				ch[2].addr.sd = ch[3].addr.sd;
				ch[2].count.sd = ch[3].count.sd;
//				ch[2].mode = ch[3].mode;
				status |= 0x10;
			} else if(mode & 0x40) {
				mode &= ~(1 << c);
			}
			status |= (1 << c);
			ch[c].running = false;
			write_signals(&outputs_tc, 0xffffffff);
		}
#ifdef SINGLE_MODE_DMA
		break;
#endif
	}
}

void I8257::write_via_debugger_data8w(uint32_t addr, uint32_t data, int *wait)
{
	d_mem->write_dma_data8w(addr, data, wait);
}

uint32_t I8257::read_via_debugger_data8w(uint32_t addr, int *wait)
{
	return d_mem->read_dma_data8w(addr, wait);
}

void I8257::write_via_debugger_data16w(uint32_t addr, uint32_t data, int *wait)
{
	d_mem->write_dma_data16w(addr, data, wait);
}

uint32_t I8257::read_via_debugger_data16w(uint32_t addr, int *wait)
{
	return d_mem->read_dma_data16w(addr, wait);
}

void I8257::write_mem(uint32_t addr, uint32_t data, int *wait)
{
#ifdef USE_DEBUGGER
	if(d_debugger != NULL && d_debugger->now_device_debugging) {
		d_debugger->write_via_debugger_data8w(addr, data, wait);
	} else
#endif
	this->write_via_debugger_data8w(addr, data, wait);
}

uint32_t I8257::read_mem(uint32_t addr, int *wait)
{
#ifdef USE_DEBUGGER
	if(d_debugger != NULL && d_debugger->now_device_debugging) {
		return d_debugger->read_via_debugger_data8w(addr, wait);
	} else
#endif
	return this->read_via_debugger_data8w(addr, wait);
}

void I8257::write_io(int c, uint32_t data, int *wait)
{
	ch[c].io->write_dma_io8w(0, data, wait);
}

uint32_t I8257::read_io(int c, int *wait)
{
	return ch[c].io->read_dma_io8w(0, wait);
}

#ifdef USE_DEBUGGER
bool I8257::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
/*
CH0 ADDR=FFFF COUNT=FFFF REQ=1 MODE=FF MEM->I/O
CH1 ADDR=FFFF COUNT=FFFF REQ=1 MODE=FF I/O->MEM
CH2 ADDR=FFFF COUNT=FFFF REQ=1 MODE=FF VERIFY
CH3 ADDR=FFFF COUNT=FFFF REQ=1 MODE=FF INVALID
*/
	static const _TCHAR *dir[4] = {
		_T("VERIFY"), _T("I/O->MEM"), _T("MEM->I/O"), _T("INVALID")
	};
	my_stprintf_s(buffer, buffer_len,
	_T("CH0 ADDR=%04X COUNT=%04X REQ=%d MODE=%02X %s\n")
	_T("CH1 ADDR=%04X COUNT=%04X REQ=%d MODE=%02X %s\n")
	_T("CH2 ADDR=%04X COUNT=%04X REQ=%d MODE=%02X %s\n")
	_T("CH3 ADDR=%04X COUNT=%04X REQ=%d MODE=%02X %s"),
	ch[0].addr.w.l, ch[0].count.w.l, ch[0].running, ch[0].mode, dir[(ch[0].mode >> 6) & 3],
	ch[1].addr.w.l, ch[1].count.w.l, ch[1].running, ch[1].mode, dir[(ch[1].mode >> 6) & 3],
	ch[2].addr.w.l, ch[2].count.w.l, ch[2].running, ch[2].mode, dir[(ch[2].mode >> 6) & 3],
	ch[3].addr.w.l, ch[3].count.w.l, ch[3].running, ch[3].mode, dir[(ch[3].mode >> 6) & 3]);
	return true;
}
#endif

#define STATE_VERSION	1

bool I8257::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int c = 0; c < 4; c++) {
		state_fio->StateValue(ch[c].addr.d);
		state_fio->StateValue(ch[c].count.d);
		state_fio->StateValue(ch[c].mode);
		state_fio->StateValue(ch[c].running);
	}
	state_fio->StateValue(mode);
	state_fio->StateValue(status);
	state_fio->StateValue(high_low);
	return true;
}

