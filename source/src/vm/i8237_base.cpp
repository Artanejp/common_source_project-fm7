/*
	Skelton for retropc emulator

	Origin : MESS
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ i8237 ]
*/

#include "debugger.h"
#include "i8237.h"

void I8237_BASE::initialize()
{
	DEVICE::initialize();
}

void I8237_BASE::reset()
{
	low_high = false;
	cmd = req = tc = 0;
	mask = 0xff;
}

void I8237_BASE::write_io8(uint32_t addr, uint32_t data)
{
	// Dummy function
}

uint32_t I8237_BASE::read_io8(uint32_t addr)
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

void I8237_BASE::write_signal(int id, uint32_t data, uint32_t mask)
{
	// Dummy function
}

// note: if SINGLE_MODE_DMA is defined, do_dma() is called in every machine cycle

void I8237_BASE::do_dma()
{
	// Dummy function
}

void I8237_BASE::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
	d_mem->write_dma_data8(addr, data);
}

uint32_t I8237_BASE::read_via_debugger_data8(uint32_t addr)
{
	return d_mem->read_dma_data8(addr);
}

void I8237_BASE::write_via_debugger_data16(uint32_t addr, uint32_t data)
{
	d_mem->write_dma_data16(addr, data);
}

uint32_t I8237_BASE::read_via_debugger_data16(uint32_t addr)
{
	return d_mem->read_dma_data16(addr);
}

void I8237_BASE::write_mem(uint32_t addr, uint32_t data)
{
	if(mode_word) {
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			d_debugger->write_via_debugger_data16((addr << 1) & addr_mask, data);
		} else {
			this->write_via_debugger_data16((addr << 1) & addr_mask, data);
		}
	} else {
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			d_debugger->write_via_debugger_data8(addr & addr_mask, data);
		} else {
			this->write_via_debugger_data8(addr & addr_mask, data);
		}
	}
}

uint32_t I8237_BASE::read_mem(uint32_t addr)
{
	if(mode_word) {
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			return d_debugger->read_via_debugger_data16((addr << 1) & addr_mask);
		} else {
			return this->read_via_debugger_data16((addr << 1) & addr_mask);
		}
	} else {
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			return d_debugger->read_via_debugger_data8(addr & addr_mask);
		} else {
			return this->read_via_debugger_data8(addr & addr_mask);
		}
	}
}

void I8237_BASE::write_io(int ch, uint32_t data)
{
	if(mode_word) {
		dma[ch].dev->write_dma_io16(0, data);
	} else {
		dma[ch].dev->write_dma_io8(0, data);
	}
}

bool I8237_BASE::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
/*
CH0 AREG=FFFF CREG=FFFF BAREG=FFFF BCREG=FFFF BANK=FFFF REQ=1 MASK=1 MODE=FF MEM->I/O
CH1 AREG=FFFF CREG=FFFF BAREG=FFFF BCREG=FFFF BANK=FFFF REQ=1 MASK=1 MODE=FF I/O->MEM
CH2 AREG=FFFF CREG=FFFF BAREG=FFFF BCREG=FFFF BANK=FFFF REQ=1 MASK=1 MODE=FF VERIFY
CH3 AREG=FFFF CREG=FFFF BAREG=FFFF BCREG=FFFF BANK=FFFF REQ=1 MASK=1 MODE=FF INVALID
*/
	static const _TCHAR *dir[4] = {
		_T("VERIFY"), _T("I/O->MEM"), _T("MEM->I/O"), _T("INVALID")
	};
	my_stprintf_s(buffer, buffer_len,
	_T("CH0 AREG=%04X CREG=%04X BAREG=%04X BCREG=%04X BANK=%04X REQ=%d MASK=%d MODE=%02X %s\n")
	_T("CH1 AREG=%04X CREG=%04X BAREG=%04X BCREG=%04X BANK=%04X REQ=%d MASK=%d MODE=%02X %s\n")
	_T("CH2 AREG=%04X CREG=%04X BAREG=%04X BCREG=%04X BANK=%04X REQ=%d MASK=%d MODE=%02X %s\n")
	_T("CH3 AREG=%04X CREG=%04X BAREG=%04X BCREG=%04X BANK=%04X REQ=%d MASK=%d MODE=%02X %s"),
	dma[0].areg, dma[0].creg, dma[0].bareg, dma[0].bcreg, dma[0].bankreg, (req >> 0) & 1, (mask >> 0) & 1, dma[0].mode, dir[(dma[0].mode >> 2) & 3],
	dma[1].areg, dma[1].creg, dma[1].bareg, dma[1].bcreg, dma[1].bankreg, (req >> 1) & 1, (mask >> 1) & 1, dma[1].mode, dir[(dma[1].mode >> 2) & 3],
	dma[2].areg, dma[2].creg, dma[2].bareg, dma[2].bcreg, dma[2].bankreg, (req >> 2) & 1, (mask >> 2) & 1, dma[2].mode, dir[(dma[2].mode >> 2) & 3],
	dma[3].areg, dma[3].creg, dma[3].bareg, dma[3].bcreg, dma[3].bankreg, (req >> 3) & 1, (mask >> 3) & 1, dma[3].mode, dir[(dma[3].mode >> 2) & 3]);
	return true;
}

uint32_t I8237_BASE::read_io(int ch)
{
	if(mode_word) {
		return dma[ch].dev->read_dma_io16(0);
	} else {
		return dma[ch].dev->read_dma_io8(0);
	}
}

