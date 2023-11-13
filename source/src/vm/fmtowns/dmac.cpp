
#include "./dmac.h"
#include "debugger.h"

namespace FMTOWNS {
#define EVENT_DMAC_CYCLE	1

void TOWNS_DMAC::initialize()
{
	UPD71071::initialize();
	for(int ch = 0; ch < 4; ch++) {
		address_aligns_16bit[ch] = true;
		is_16bit_transfer[ch] = false;
		is_16bit[ch] = false;
	}
	event_dmac_cycle = -1;
	spent_clocks = 0;
	transfer_ch = 0;
}

void TOWNS_DMAC::reset_from_io()
{
	for(int ch = 0; ch < 4; ch++) {
		end_req[ch] = false;
		calc_transfer_status(ch);
	}

	for(int ch = 0; ch < 4; ch++) {
		//write_signals(&outputs_towns_tc[ch], ((tc & (1 << ch)) != 0) ? 0xffffffff : 0);
		write_signals(&outputs_ube[ch], (is_16bit[ch]) ? 0xffffffff : 0);
	}
	clear_event(this, event_dmac_cycle);
	spent_clocks = 0;
	transfer_ch = 0;
}

void TOWNS_DMAC::reset()
{
	UPD71071::reset();
	set_mask_reg(mask);
  	dma_wrap = true;
	reset_from_io();
}

void TOWNS_DMAC::set_dmac_clock(double clock_hz, int ratio)
{
	if(ratio > 0) {
		clock_multiply = ratio;
	} else {
		clock_multiply = 1;
	}
	double tmp_us = 1.0e6 / clock_hz;
	if((tmp_us != dmac_cycle_us) && (event_dmac_cycle >= 0)) {
		cancel_event(this, event_dmac_cycle);
		register_event(this, EVENT_DMAC_CYCLE, tmp_us, true, &event_dmac_cycle);
	}
	dmac_cycle_us = tmp_us;
}

void TOWNS_DMAC::check_mask_and_cmd()
{
	#if 1
	__UNLIKELY_IF(_SINGLE_MODE_DMA) {
		return;
	}
	bool req_burst = false;
	if(((cmd & 0x04) == 0) && ((mask & 0x0f) != 0x0f)) {
		// Mode Check
		#if 0
		req_burst = true;
		#else
		for(int ch = 0; ch < 4; ch++) {
			uint8_t _bit = 1 << ch;
			if((_bit & mask) == 0) { // Running
				__UNLIKELY_IF((primary_dmac) && (ch == 1)) {
					 // Workaround for SCSI HOST (Primary DMAC CH.1).
					req_burst = true;
					break;
				} else if((dma[ch].mode & 0xc0) != 0x40) {
					// At other channel, SINGLE MODE may make asyncronous.
					req_burst = true;
					break;
				}
			}
		}
		#endif
	}
	__UNLIKELY_IF(req_burst) {
		__UNLIKELY_IF(event_dmac_cycle < 0) {
			register_event(this, EVENT_DMAC_CYCLE, dmac_cycle_us, true, &event_dmac_cycle);
		}
	} else {
		spent_clocks = 0;
		__UNLIKELY_IF(event_dmac_cycle >= 0) {
			transfer_ch = 0;
			cancel_event(this, event_dmac_cycle);
		}
		event_dmac_cycle = -1;
	}
	#endif
}

void TOWNS_DMAC::write_io16(uint32_t addr, uint32_t data)
{
	switch(addr & 0x0f) {
	case 0x02:
		dma[selch].bcreg = data;
		dma[selch].creg = data;
		// Reset TC bit for towns, by Tsugaru commit ab067790479064efce693f7317af13696cb68d96 .
		tc &= ~(1 << selch);
		write_signals(&outputs_towns_tc[selch], 0);
		break;
	case 0x04: // ADDR LOW
		dma[selch].bareg = (dma[selch].bareg & 0xffff0000) | (data & 0xffff);
		dma[selch].areg  = (dma[selch].areg  & 0xffff0000) | (data & 0xffff);
		break;
	case 0x06: // ADDR HIGH
		dma[selch].bareg = (dma[selch].bareg & 0x0000ffff) | ((data & 0xffff) << 16);
		dma[selch].areg  = (dma[selch].areg  & 0x0000ffff) | ((data & 0xffff) << 16);
		break;
	case 0x08: // Control
		cmd = data;
		check_mask_and_cmd();
		break;
	default:
		write_io8((addr & 0x0e) + 0, data & 0x00ff);
		write_io8((addr & 0x0e) + 1, (data & 0xff00) >> 8); // OK?
		break;
	}
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
		set_mask_reg(mask);
		check_mask_and_cmd();
		if(data & 1) {
			reset_from_io();
			out_debug_log(_T("RESET from I/O; B16=%s"), ((b16 & 2) != 0) ? _T("16bit") : _T("8bit"));
		}
		break;
	case 0x02:
		// Reset TC bit for towns, by Tsugaru commit ab067790479064efce693f7317af13696cb68d96 .		tc &= ~(1 << selch);
		tc = tc & ~(1 << (selch & 3));
		write_signals(&outputs_towns_tc[selch], 0);
		UPD71071::write_io8(addr, data);
		break;
//	case 0x03:
		//tc = tc & ~(1 << (selch & 3));
		//write_signals(&outputs_towns_tc[selch], 0);
//		UPD71071::write_io8(addr, data);
//		break;
	case 0x04:
	case 0x05:
	case 0x06:
		{
			uint32_t __d_mask = ~(0x000000ff << (((addr & 0x0f) - 4) * 8));
			uint32_t __d = ((data & 0x000000ff) << (((addr & 0x0f) - 4) * 8));
			dma[selch].bareg = (dma[selch].bareg & __d_mask) | __d;
			dma[selch].areg  = (dma[selch].areg  & __d_mask) | __d;
		}
		break;
	case 0x07:
		{
			uint32_t __d_mask = 0x00ffffff;
			uint32_t __d = (data & 0x000000ff) << 24;
			dma[selch].bareg = (dma[selch].bareg & __d_mask) | __d;
			dma[selch].areg  = (dma[selch].areg  & __d_mask) | __d;
		}
		break;
	case 0x08:
		cmd = (cmd & 0xff00) | (data & 0xff);
		check_mask_and_cmd();
		break;
	case 0x09:
		cmd = (cmd & 0xff) | ((data & 0xff) << 8);
		break;
	case 0x0a:
		// MODE
		// BIT 7,6 : TRANSFER MODE
		//   DEMAND    = 00
		//   SINGLE    = 01
		//   BLOCK     = 10
		//   CASCADE   = 11
		// BIT 5    : ADIR
		//   INC       = 0
		//   DEC       = 1
		// BIT 4    : AUTI
		//   AUTO INIT = 1
		// BIT 3, 2 : TDIR
		//   VERIFY    = 00
		//   IO to MEM = 01
		//   MEM to IO = 10
		//   DONT      = 11
		// BIT 0    : W/B
		//   BYTE = 0
		//   WORD = 1
		UPD71071::write_io8(addr, data);
		//check_mask_and_cmd();

		#if 1
		/* DO NOTHING */
		#endif
		#if 0
		out_debug_log(_T("MODE CHANGED at CH.%d to 0x%02X Request 16bit=%s CMD=%02X"), selch, dma[selch].mode,
					  (is_16bit_transfer[selch]) ? _T("Yes") : _T("NO"),
					  cmd);
		static const _TCHAR *dir[4] = {
			_T("VERIFY"), _T("I/O->MEM"), _T("MEM->I/O"), _T("INVALID")
		};
		out_debug_log(_T("CH%d AREG=%08X CREG=%04X BAREG=%08X BCREG=%04X REQ=%d MASK=%d MODE=%02X %s"),
					  selch, dma[selch].areg, dma[selch].creg, dma[selch].bareg, dma[selch].bcreg,
					  ((req | sreq) >> selch) & 1,
					  (mask >> selch) & 1,
					  dma[selch].mode,
					  dir[(dma[selch].mode >> 2) & 3]);
		#endif
		break;
		// MASK
	case 0x0f:
		set_mask_reg(data);
		check_mask_and_cmd();
		break;
	default:
		UPD71071::write_io8(addr, data);
		break;
	}
//	if((need_transfer) && ((cmd & 4) == 0)) {
//		do_dma();
//	}
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
		//for(int ch = 0; ch < 4; ch++) {
		//	write_signals(&outputs_towns_tc[ch], 0);
		//}
		break;
	default:
		val = UPD71071::read_io8(addr);
		break;
	}
	return val;
}

uint32_t TOWNS_DMAC::read_io16(uint32_t addr)
{
	switch(addr & 0x0f) {
	case 0x02:
		if(base != 0) {
			return dma[selch].bcreg;
		} else {
			return dma[selch].creg;
		}
		break;
	case 0x04:
		if(base != 0) {
			return dma[selch].bareg & 0x0000ffff;
		} else {
			return dma[selch].areg  & 0x0000ffff;
		}
		break;
	case 0x06:
		if(base != 0) {
			return (dma[selch].bareg & 0xffff0000) >> 16;
		} else {
			return (dma[selch].areg  & 0xffff0000) >> 16;
		}
		break;
	case 0x08:
		return cmd;
		break;
	default:
		break;
	}
	uint32_t val = 0;
	val = read_io8((addr & 0x0e) + 0) & 0x00ff;
	val = val | ((read_io8((addr & 0x0e) + 1) & 0xff) << 8); // OK?
	return val;
}

void TOWNS_DMAC::inc_dec_ptr_a_byte(uint32_t& addr, const bool inc)
{
	// Note: FM-Towns may extend to 32bit.
	// Note: By Tsugaru, commit e5920fdc1ba89ba10172f0954ecf1107bb592919,
	// ADIR bit (bit 5 of mode register)
	// has not supported by TOWNS DMAC.
	// Very tanks to YAMAKAWA-San. - 20230617 K.O
	//
	// Commiting message is below:
	//	Author:     CaptainYS <PEB01130@nifty.com>
	//	AuthorDate: Sat Feb 29 23:43:52 2020 -0500
	//	Commit:     CaptainYS <PEB01130@nifty.com>
	//	CommitDate: Sat Feb 29 23:43:52 2020 -0500
	//
	//	Parent:     9390f5be
	//					Found Device-DMACh correspondence in [2] pp. 56.
	//					Still trying to find the correct sequence of CD-ROM drive
	//					data transfer.
	//					Unit test still temporarily broken.

//	uint32_t incdec = (inc) ? 1 : UINT32_MAX;
	const uint32_t incdec = 1;
	__LIKELY_IF(dma_wrap) {
		uint32_t high_a = addr & 0xff000000;
		addr = (addr + incdec) & 0x00ffffff;
		addr = addr | high_a;
	} else {
		addr = (addr + incdec) & 0xffffffff;
	}
}

void TOWNS_DMAC::inc_dec_ptr_two_bytes(uint32_t& addr, const bool inc)
{
	// Note: FM-Towns may extend to 32bit.

	//uint32_t incdec = (inc) ? 2 : (UINT32_MAX - 1);
	const uint32_t incdec = 2;
	__LIKELY_IF(dma_wrap) {
		uint32_t high_a = addr & 0xff000000;
		addr = (addr + incdec) & 0x00ffffff;
		addr = addr | high_a;
	} else {
		addr = (addr + incdec) & 0xffffffff;
	}
}

uint32_t TOWNS_DMAC::read_8bit_from_device(DEVICE* dev, uint32_t addr, int* wait)
{
	__UNLIKELY_IF(dev == nullptr) {
		if(wait != nullptr) {
			*wait = 0; // ToDo
		}
		return 0xff;
	}
	uint32_t val;
	val = dev->read_dma_io8w(addr, wait);
	return val;
}

void TOWNS_DMAC::write_8bit_to_device(DEVICE* dev, uint32_t addr, uint32_t data, int* wait)
{
	__UNLIKELY_IF(dev == nullptr) {
		if(wait != nullptr) {
			*wait = 0; // ToDo
		}
		return;
	}
	dev->write_dma_io8w(addr, data, wait);
}


uint32_t TOWNS_DMAC::read_8bit_from_memory(uint32_t addr, int* wait, const bool is_use_debugger)
{
	__UNLIKELY_IF((is_use_debugger) && (d_debugger != NULL)) {
		return d_debugger->read_via_debugger_data8w(addr, wait);
	} else {
		return read_via_debugger_data8w(addr, wait);
	}
}

void TOWNS_DMAC::write_8bit_to_memory(uint32_t addr, uint32_t data, int* wait, const bool is_use_debugger)
{
	__UNLIKELY_IF((is_use_debugger) && (d_debugger != NULL)) {
		d_debugger->write_via_debugger_data8w(addr, data, wait);
	} else {
		write_via_debugger_data8w(addr, data, wait);
	}
}

uint32_t TOWNS_DMAC::read_16bit_from_device(DEVICE* dev, uint32_t addr, int* wait)
{
	__UNLIKELY_IF(dev == nullptr) {
		if(wait != nullptr) {
			*wait = 0; // ToDo
		}
		return 0xffff;
	}
	uint32_t val;
	val = dev->read_dma_io16w(addr, wait);
	return val;
}

void TOWNS_DMAC::write_16bit_to_device(DEVICE* dev, uint32_t addr, uint32_t data, int* wait)
{
	__UNLIKELY_IF(dev == nullptr) {
		if(wait != nullptr) {
			*wait = 0; // ToDo
		}
		return;
	}
	dev->write_dma_io16w(addr, data, wait);
}

uint32_t TOWNS_DMAC::read_16bit_from_memory(uint32_t addr, int* wait, const bool is_use_debugger)
{
	__UNLIKELY_IF((is_use_debugger) && (d_debugger != NULL)) {
		return d_debugger->read_via_debugger_data16w(addr, wait);
	} else {
		return read_via_debugger_data16w(addr, wait);
	}
}

void TOWNS_DMAC::write_16bit_to_memory(uint32_t addr, uint32_t data, int* wait, const bool is_use_debugger)
{
	__UNLIKELY_IF((is_use_debugger) && (d_debugger != NULL)) {
		d_debugger->write_via_debugger_data16w(addr, data, wait);
	} else {
		write_via_debugger_data16w(addr, data, wait);
	}
}

void TOWNS_DMAC::do_dma_16bit(DEVICE* dev, const uint8_t tr_mode, uint32_t& memory_address, const bool compressed, const bool extended, bool is_use_debugger, int& wait)
{
	uint16_t val;
	int wait_w = 0;
	int wait_r = 0;
	const int wait_compressed = (compressed) ? 5 : 7;
	switch(tr_mode & 0x0c) {
	case 0x00: // VERIFY
		val = read_16bit_from_device(dev, 0, &wait_r);
		tmp = val;
		read_16bit_from_memory(memory_address, &wait_w, is_use_debugger);
		break;
	case 0x04: // DEVICE TO MEMORY
		val = read_16bit_from_device(dev, 0, &wait_r);
		tmp = val;
		write_16bit_to_memory(memory_address, val, &wait_w, is_use_debugger);
		break;
	case 0x08: // MEMORY TO DEVICE
		val = read_16bit_from_memory(memory_address, &wait_r, is_use_debugger);
		tmp = val;
		write_16bit_to_device(dev, 0, val, &wait_w);
		break;
	case 0x0c: // MEMORY TO MEMORY : still unimplemented
		break;
	default:
		break;
	}
	wait += wait_compressed;
	if(extended) {
		wait = wait + wait_r + wait_w;
	}
//				inc_dec_ptr_two_bytes(memory_adderss, !(tr_mode & 0x20));
	inc_dec_ptr_two_bytes(memory_address, true);
}

void TOWNS_DMAC::do_dma_8bit(DEVICE* dev, const uint8_t tr_mode, uint32_t& memory_address, const bool compressed, const bool extended, bool is_use_debugger, int& wait)
{
	uint32_t val;
	int wait_w = 0;
	int wait_r = 0;
	const int wait_compressed = (compressed) ? 5 : 7;
	switch(tr_mode & 0x0c) {
	case 0x00: // VERIFY
		val = read_8bit_from_device(dev, 0, &wait_r);
		tmp = ((tmp & 0xff00) >> 8) | ((val & 0xff) << 8);
		read_8bit_from_memory(memory_address, &wait_w, is_use_debugger);
		break;
	case 0x04: // DEVICE TO MEMORY
		val = read_8bit_from_device(dev, 0, &wait_r);
		tmp = ((tmp & 0xff00) >> 8) | ((val & 0xff) << 8);
		write_8bit_to_memory(memory_address, val, &wait_w, is_use_debugger);
		break;
	case 0x08: // MEMORY TO DEVICE
		val = read_8bit_from_memory(memory_address, &wait_r, is_use_debugger);
		tmp = ((tmp & 0xff00) >> 8) | ((val & 0xff) << 8);
		write_8bit_to_device(dev, 0, val, &wait_w);
		break;
	case 0x0c: // MEMORY TO MEMORY : still unimplemented
		break;
	default:
		break;
	}
	wait += wait_compressed;
	if(extended) {
		wait = wait + wait_r + wait_w;
	}
//				inc_dec_ptr_a_byte(memory_address, !(tr_mode & 0x20));
	inc_dec_ptr_a_byte(memory_address, true);
}

bool TOWNS_DMAC::check_is_16bit(int ch)
{
	const bool __is_16bit = (is_16bit_transfer[ch] || force_16bit_transfer[ch]);
	if(__is_16bit != is_16bit[ch]) {
		is_16bit[ch] = __is_16bit;
		write_signals(&outputs_ube[ch], (__is_16bit) ? 0xffffffff : 0x00000000); // Reset UBE
	}
	return __is_16bit;
}

bool TOWNS_DMAC::decrement_counter(const int ch, uint8_t mode, uint16_t& counter, bool& is_single)
{
	bool is_terminated = false;
	uint8_t c = ch & 3;
	uint16_t counter_bak = counter;
	uint8_t bit = 1 << c;
	__LIKELY_IF((_SINGLE_MODE_DMA) || ((mode & 0xc0) == 0x40)) {
		is_single = true;
	} else {
		is_single = false;
	}
	counter--; // OK?

	__UNLIKELY_IF((counter == 0xffff) && (counter_bak == 0x0000)) {
		do_end_sequence(c, true);
		set_ack(c, true);
		is_terminated = true;
		return is_terminated;
	} else __UNLIKELY_IF(!(is_single) && (end_req[c])) {
		do_end_sequence(c, false);
		set_ack(c, true);
		is_terminated = true;
		return is_terminated;
	} else {
		// Continue
		switch(mode & 0xc0) {
		case 0x00: // DEMAND
			// WHY STOP:
			// - COUNTDOWN REACHED.
			// - END_REQ[c] asserted.
			// - DMA REQ MADE INACTIVE.
			// -> CLEAR REQ and SREQ bit, ASSERT TC REGISTER.
			__UNLIKELY_IF((req & bit) == 0) {
				do_end_sequence(c, false);
				is_terminated = true;
				return is_terminated;
			}
			break;
		case 0x40: // SINGLE
			// WHY STOP:
			// - COUNTDOWN REACHED.
			// - STOP PER BYTE/WORD TRANSFER.
			//   SOMETHING DON'T ASSERT TC, EXCEPTS COUNT DOWN REACHED TO 0.
			// -> CLEAR REQ and SREQ bit.
			req &= ~bit;
			sreq &= ~bit;
			is_terminated = true;
			break;
		case 0x80: // BURST
			// WHY STOP:
			// - END_REQ[c] asserted.
			// - COUNTDOWN REACHED.
			// -> DO NOTHING.
			break;
		case 0xC0: // CASCADE
			// ToDo.
			break;
		}
	}
	set_ack(c, true);
	return is_terminated;
}

int TOWNS_DMAC::do_dma_single(const int ch, const bool is_use_debugger, bool compressed, bool extended, bool& is_terminated, bool& is_single)
{
	int _clocks = 0;
	int c = ch & 3;
	uint8_t bit = 1 << c;
	if(((req | sreq) & bit) && !(mask & bit)) {
		set_ack(c, false);
		__UNLIKELY_IF(!running) {
			_clocks += 2; // S0
			running = true;
		}
		if(check_is_16bit(c)) {
			do_dma_16bit(dma[c].dev, dma[c].mode, dma[c].areg, compressed, extended, is_use_debugger, _clocks);
		} else {
			do_dma_8bit(dma[ch].dev, dma[c].mode, dma[c].areg, compressed, extended, is_use_debugger, _clocks);
		}
		// Note: At FM-Towns, SCSI's DMAC will be set after
		//       SCSI bus phase become DATA IN/DATA OUT.
		//       Before bus phase became DATA IN/DATA OUT,
		//       DMAC mode and state was unstable (and ASSERTED
		//       DRQ came from SCSI before this state change).
		// ToDo: Stop correctly before setting.
		// CHECK COUNT DOWN REACHED.
		is_terminated = decrement_counter(ch, dma[c].mode,  dma[c].creg, is_single);
	}
	return _clocks;
}


void TOWNS_DMAC::do_dma_internal()
{
	__UNLIKELY_IF((cmd & 0x04) != 0) return;
	bool is_hold = ((cmd & 0x0100) != 0) ? true : false;
	bool compressed = ((cmd & 0x08) != 0);
	bool extended = ((cmd & 0x20) != 0);
	// run dma
	bool is_use_debugger = false;
	if(__USE_DEBUGGER) {
		__LIKELY_IF(d_debugger != NULL) {
			is_use_debugger = d_debugger->now_device_debugging;
		}
	}
	bool is_rot = ((cmd & 0x10) != 0) ? true : false;
	if(!(is_rot)) {
		transfer_ch = 0;
	}
	for(int i = 0; i < 4; i++) {
		bool is_single = false;
		bool is_terminated = false;
		int clocks = do_dma_single((transfer_ch + i) & 3, is_use_debugger, compressed, extended, is_terminated, is_single);
		if(clocks > 0) {
			spent_clocks += clocks;
			if(!(is_hold)) {
				break;
			}
		} else if((is_terminated) && !(is_hold)) {
			break;
		}
	}
	if(is_rot) {
		transfer_ch = (transfer_ch + 1) & 3;
	}
	__UNLIKELY_IF((spent_clocks > 0) && (d_cpu != NULL)) {
		d_cpu->set_extra_clock(spent_clocks);
		spent_clocks = 0;
	}
	return;
}

void TOWNS_DMAC::do_dma()
{
	// check DDMA
	if((cmd & 0x04) == 0) {
		spent_clocks = 0;
		do_dma_internal();
	}
	__UNLIKELY_IF(_SINGLE_MODE_DMA) {
		__LIKELY_IF(d_dma) {
			d_dma->do_dma();
		}
	}
}

void TOWNS_DMAC::event_callback(int event_id, int err)
{
	__LIKELY_IF(event_id == EVENT_DMAC_CYCLE) {
		__LIKELY_IF(spent_clocks > 0) {
			spent_clocks -= clock_multiply;
		} else if((cmd & 0x04) == 0) {
			spent_clocks = (clock_multiply > 0) ? (clock_multiply - 1) : 0;
			do_dma_internal();
		}
	}
}
uint32_t TOWNS_DMAC::read_signal(int id)
{
	if((id >= SIG_TOWNS_DMAC_MASK_CH0) && (id <= SIG_TOWNS_DMAC_MASK_CH3)) {
		int ch = id - SIG_TOWNS_DMAC_MASK_CH0;
		uint8_t _bit = 1 << ch;
		return ((_bit & mask) == 0) ? 0xffffffff : 0x00000000;
	}
	if(id == SIG_TOWNS_DMAC_WRAP) {
		return (dma_wrap) ? 0xffffffff : 0;
	}
	return UPD71071::read_signal(id);
}

void TOWNS_DMAC::write_signal(int id, uint32_t data, uint32_t _mask)
{
	if(id == SIG_TOWNS_DMAC_WRAP) {
		dma_wrap = ((data & _mask) != 0) ? true : false;
	} else if((id >= SIG_TOWNS_DMAC_EOT_CH0) && (id <= SIG_TOWNS_DMAC_EOT_CH3)) {
		int ch = (id - SIG_TOWNS_DMAC_EOT_CH0) & 3;
		end_req[ch] = ((data & _mask) != 0) ? true : false;
	} else {
		__LIKELY_IF((id >= SIG_UPD71071_CH0) && (id <= SIG_UPD71071_CH3)) {
			int ch = (id - SIG_UPD71071_CH0) & 3;
			uint8_t bit = 1 << ch;
			// ToDo: Per Channel.
			if(data & _mask) {
				__UNLIKELY_IF(!(req & bit)) {
					req |= bit;
					__LIKELY_IF(!(_SINGLE_MODE_DMA)) {
						if(event_dmac_cycle < 0) {
							do_dma_internal();
						}
					}
				}
			} else {
				req &= ~bit;
			}
		} else {
			// Fallthrough.
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

#define STATE_VERSION	11

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
	state_fio->StateArray(end_req, sizeof(end_req), 1);
	state_fio->StateValue(spent_clocks);
	state_fio->StateValue(transfer_ch);

	state_fio->StateValue(event_dmac_cycle);

	if(loading) {
		for(int ch = 0; ch < 4; ch++) {
			calc_transfer_status(ch);
		}
	}
	return true;
}


}
