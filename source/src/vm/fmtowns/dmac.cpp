
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
	} else if(id == SIG_UPD71071_CH1) {
		// Workaround for SCSI
		if(((dma[1].mode & 0x01) == 1) && ((data & _mask) != 0)) { // This extend feature maybe used for TOWNS, 16		reset_dma_ack();
			if(cmd & 4) {
				return;
			}
			reset_dma_ack(1);
			bool dev_to_mem = ((dma[1].mode & 0x04) != 0);
			if((dma[1].mode & 0x0c) != 0x0c) {
				uint32_t tmpdata;
				int wait = 0;
				if(dev_to_mem) {
					tmpdata = read_from_io_8bit(1, &wait);
					data_reg = ((tmpdata & 0xff) << 8) | ((data_reg >> 8) & 0xff);
				} else {
					tmpdata = read_from_io_8bit(1, &wait, __debugging);
				}

			}
			if((local_count & 1) != 0) {
				UPD71071::write_signal(id, data, _mask);
			} else {
				set_dma_ack(1);
			}
			local_count++;
		} else {
			UPD71071::write_signal(id, data, _mask);
		}
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
