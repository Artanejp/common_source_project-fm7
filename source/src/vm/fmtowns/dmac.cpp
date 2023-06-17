
#include "./dmac.h"
#include "debugger.h"

namespace FMTOWNS {
#define EVENT_DMAC_CYCLE	1

void TOWNS_DMAC::initialize()
{
	UPD71071::initialize();
	for(int ch = 0; ch < 4; ch++) {
		is_started[ch] = false;
		address_aligns_16bit[ch] = true;
		is_16bit_transfer[ch] = false;
		is_16bit[ch] = false;
	}
	event_dmac_cycle = -1;
}

void TOWNS_DMAC::reset()
{
	UPD71071::reset();
  	dma_wrap = true;
	div_count = 0;
	for(int ch = 0; ch < 4; ch++) {
		end_req[ch] = false;
		is_started[ch] = false;
		calc_transfer_status(ch);
	}
	clear_event(this, event_dmac_cycle);

	for(int ch = 0; ch < 4; ch++) {
		//write_signals(&outputs_towns_tc[ch], ((tc & (1 << ch)) != 0) ? 0xffffffff : 0);
		write_signals(&outputs_ube[ch], (is_16bit[ch]) ? 0xffffffff : 0);
	}
}


void TOWNS_DMAC::call_dma(int ch)
{
	bool is_use_debugger = false;
	__LIKELY_IF(__USE_DEBUGGER) {
		__LIKELY_IF(d_debugger != NULL) {
			is_use_debugger = d_debugger->now_device_debugging;
		}
	}
	do_dma_per_channel(ch, is_use_debugger, false);
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
		if(data & 1) {
			clear_event(this, event_dmac_cycle);
		}
		for(int ch = 0; ch < 4; ch++) {
			if(data & 1) {
				end_req[ch] = false;
				is_started[ch] = false;
			}
			calc_transfer_status(ch);
		}
		check_running();
		for(int ch = 0; ch < 4; ch++) {
			write_signals(&outputs_ube[ch], (is_16bit[ch]) ? 0xffffffff : 0);
		}
		out_debug_log(_T("RESET from I/O; B16=%s"), ((b16 & 2) != 0) ? _T("16bit") : _T("8bit"));
		break;
	case 0x02:
	case 0x03:
		// Reset TC bit for towns, by Tsugaru commit ab067790479064efce693f7317af13696cb68d96 .
		tc &= ~(1 << selch);
		write_signals(&outputs_towns_tc[selch], 0);
		UPD71071::write_io8(addr, data);
		break;
	case 0x04:
		dma[selch].bareg = (dma[selch].bareg & 0xffffff00) | data;
//		if(!base) {
			dma[selch].areg = (dma[selch].areg & 0xffffff00) | data;
//		}
		#if 0
		if(is_started[selch]) {
			if(check_address_16bit_bus_changed(selch)) {
				set_ube_line(selch);
			}
		}
		#endif
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
		// CMD
	case 0x08:
	case 0x09:
		UPD71071::write_io8(addr, data);
	#if 1
		if(/*(cmd != cmd_bak) && */((cmd & 0x04) == 0)) {
			check_start_condition();
		} else {
			check_running();
		}
		#if 0
		if(cmd != cmd_bak) {
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
		}
		#endif
	#endif
		break;
		// MODE
	case 0x0a:
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

		#if 1
		/* DO NOTHING */
		#endif
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
		break;
		// MASK
	case 0x0e:
		{
			const uint8_t sreq_b = sreq;
			if(((sreq = data) & 0x0f) != 0) {
				if(((sreq_b ^ sreq) & 0x0f) != 0) {
					// SREQ CHANGED.
					check_start_condition();
				}
			}
		}
		break;
		// MASK
	case 0x0f:
		#if 1
		{
			const uint8_t _mask_b = mask;
			const uint8_t bit = 1 << selch;
			UPD71071::write_io8(addr, data);

			// Check when mask[selch] has changed,
			if(((mask ^ _mask_b) & bit) != 0) {
				check_start_condition();
				check_running();
			}
		}
		#endif
		// Add trigger of transfer by SREQ.
		//need_transfer = (!(_SINGLE_MODE_DMA) && ((cmd & 0x04) == 0));
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

inline uint32_t TOWNS_DMAC::read_16bit_from_device(DEVICE* dev, uint32_t addr, int* wait)
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

inline void TOWNS_DMAC::write_16bit_to_device(DEVICE* dev, uint32_t addr, uint32_t data, int* wait)
{
	__UNLIKELY_IF(dev == nullptr) {
		if(wait != nullptr) {
			*wait = 0; // ToDo
		}
		return;
	}
	dev->write_dma_io16w(addr, data, wait);
}

inline uint32_t TOWNS_DMAC::read_16bit_from_memory(uint32_t addr, int* wait, const bool is_use_debugger)
{
	__UNLIKELY_IF((is_use_debugger) && (d_debugger != NULL)) {
		return d_debugger->read_via_debugger_data16w(addr, wait);
	} else {
		return read_via_debugger_data16w(addr, wait);
	}
}

inline void TOWNS_DMAC::write_16bit_to_memory(uint32_t addr, uint32_t data, int* wait, const bool is_use_debugger)
{
	__UNLIKELY_IF((is_use_debugger) && (d_debugger != NULL)) {
		d_debugger->write_via_debugger_data16w(addr, data, wait);
	} else {
		write_via_debugger_data16w(addr, data, wait);
	}
}

inline uint32_t TOWNS_DMAC::read_8bit_from_device(DEVICE* dev, uint32_t addr, int* wait)
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

inline void TOWNS_DMAC::write_8bit_to_device(DEVICE* dev, uint32_t addr, uint32_t data, int* wait)
{
	__UNLIKELY_IF(dev == nullptr) {
		if(wait != nullptr) {
			*wait = 0; // ToDo
		}
		return;
	}
	dev->write_dma_io8w(addr, data, wait);
}

inline uint32_t TOWNS_DMAC::read_8bit_from_memory(uint32_t addr, int* wait, const bool is_use_debugger)
{
	__UNLIKELY_IF((is_use_debugger) && (d_debugger != NULL)) {
		return d_debugger->read_via_debugger_data8w(addr, wait);
	} else {
		return read_via_debugger_data8w(addr, wait);
	}
}

inline void TOWNS_DMAC::write_8bit_to_memory(uint32_t addr, uint32_t data, int* wait, const bool is_use_debugger)
{
	__UNLIKELY_IF((is_use_debugger) && (d_debugger != NULL)) {
		d_debugger->write_via_debugger_data8w(addr, data, wait);
	} else {
		write_via_debugger_data8w(addr, data, wait);
	}
}


void TOWNS_DMAC::check_start_condition()
{
	for(int ch = 0; ch < 4; ch++) {
		uint8_t bit = 1 << ch;
		if(((mask & bit) == 0) && (((req | sreq) & bit) != 0)) {
			if(!(is_started[ch])) {
				is_started[ch] = true;
				calc_transfer_status(selch);
				write_signals(&outputs_ube[selch], (is_16bit[selch]) ? 0xffffffff : 0x00000000);
				end_req[selch] = false;
				__UNLIKELY_IF((event_dmac_cycle < 0) && !(_SINGLE_MODE_DMA)) {
					register_event(this, EVENT_DMAC_CYCLE, dmac_cycle_us, true, &event_dmac_cycle);
				}
			}
		} else {
			if(is_started[ch]) {
				is_started[ch] = false;
				check_running();
			}
		}
	}
}

inline void TOWNS_DMAC::do_dma_16bit(DEVICE* dev, const uint8_t tr_mode, uint32_t& memory_address, constexpr bool compressed, constexpr bool extended, bool is_use_debugger, int& wait)
{
	uint16_t val;
	int wait_w = 0;
	int wait_r = 0;
	const int wait_extended = (compressed) ? 5 : 7;
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

inline void TOWNS_DMAC::do_dma_8bit(DEVICE* dev, const uint8_t tr_mode, uint32_t& memory_address, constexpr bool compressed, constexpr bool extended, bool is_use_debugger, int& wait)
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

bool TOWNS_DMAC::do_dma_per_channel(int ch, bool is_use_debugger, bool force_exit)
{
	int c = ch & 3;
	uint8_t bit = 1 << c;

	if(/*((req | sreq) & bit) && !(mask & bit) && */(is_started[c])) {
		// execute dma
		// This is workaround for FM-Towns's SCSI.

		bool __is_16bit = (is_16bit_transfer[c] || force_16bit_transfer[c]);
		if(__is_16bit != is_16bit[c]) {
			is_16bit[c] = __is_16bit;
			write_signals(&outputs_ube[c], (__is_16bit) ? 0xffffffff : 0x00000000); // Reset UBE
		}

		int wait = 0;
		bool compressed = ((cmd & 0x08) != 0);
		bool exptended = ((cmd & 0x20) != 0);
		//while(((req | sreq) & bit) && (dma[c].creg <= dma[c].bcreg)) {
		if(((req | sreq) & bit) && (dma[c].creg <= dma[c].bcreg)) {
			set_ack(c, false);

			__UNLIKELY_IF(!running) {
				wait += 2; // S0
				running = true;
			}

			if(__is_16bit) {
				// ToDo: Will check WORD transfer mode for FM-Towns.(mode.bit0 = '1).
				// 16bit transfer mode
				do_dma_16bit(dma[c].dev, dma[c].mode, dma[c].areg, compressed, extended, is_use_debugger, wait);
			} else {
				// 8bit transfer mode
				do_dma_8bit(dma[c].dev, dma[c].mode, dma[c].areg, compressed, extended, is_use_debugger, wait);
			}
			__UNLIKELY_IF(d_cpu != NULL) {
				d_cpu->set_extra_clock(wait);
				wait = 0;
			}
			// Note: At FM-Towns, SCSI's DMAC will be set after
			//       SCSI bus phase become DATA IN/DATA OUT.
			//       Before bus phase became DATA IN/DATA OUT,
			//       DMAC mode and state was unstable (and ASSERTED
			//       DRQ came from SCSI before this state change).
			// ToDo: Stop correctly before setting.
			// CHECK COUNT DOWN REACHED.
			bool is_tc = false;
			set_ack(c, true);
			dma[c].creg--; // OK?
			__UNLIKELY_IF(dma[c].creg > dma[c].bcreg) {
				is_tc = true;
			}
			uint8_t _mode = dma[c].mode & 0xc0;

			__UNLIKELY_IF((end_req[c]) && (_mode != 0x40)){ // END_REQ ASSERTED EXCEPTS SINGLE TRANSFER.
				do_end_sequence(c, false);
				__LIKELY_IF(_SINGLE_MODE_DMA) {
					//	running = false;
					return true;
				}
			} else __UNLIKELY_IF(is_tc) {
				// TC
				do_end_sequence(c, true);
				__LIKELY_IF((_SINGLE_MODE_DMA) || (_mode == 0x40)) {
					//	running = false;
					return true;
				}
				return false;
			} else {
				// IF NOT COUNTDOWN REACHED.
				switch(_mode) {
				case 0x00: // DEMAND
					// WHY STOP:
					// - COUNTDOWN REACHED.
					// - END_REQ[c] asserted.
					// - DMA REQ MADE INACTIVE.
					// -> CLEAR REQ and SREQ bit, ASSERT TC REGISTER.
					__UNLIKELY_IF((req & bit) == 0) {
						do_end_sequence(c, false);
						__UNLIKELY_IF(_SINGLE_MODE_DMA) {
							return false;
						}
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
					running = false;
					break;
				case 0x80: // BURST
					// WHY STOP:
					// - END_REQ[c] asserted.
					// - COUNTDOWN REACHED.
					// -> DO NOTHING.
					break;
				case 0xC0: // CASCADE
				default:
					break;
				}
				__LIKELY_IF(_SINGLE_MODE_DMA) {
					return true;
				}
			}
			__UNLIKELY_IF(force_exit) {
				return false;;
			}
		}
	}
	return false;
}

void TOWNS_DMAC::do_dma_internal()
{
	__LIKELY_IF((event_dmac_cycle < 0) || (_SINGLE_MODE_DMA)) {
		if(div_count < 2) {
			div_count++;
			return;
		}
	}
	div_count = 0;

	// run dma
	bool is_use_debugger = false;
	if(__USE_DEBUGGER) {
		__LIKELY_IF(d_debugger != NULL) {
			is_use_debugger = d_debugger->now_device_debugging;
		}
	}
	for(int c = 0; c < 4; c++) {
		if(do_dma_per_channel(c, is_use_debugger, false)) {
			break;
		}
	}
}

void TOWNS_DMAC::do_dma()
{
	// check DDMA
	do_dma_internal();
	__UNLIKELY_IF(_SINGLE_MODE_DMA) {
		__LIKELY_IF(d_dma) {
			d_dma->do_dma();
		}
	}
}

void TOWNS_DMAC::event_callback(int event_id, int err)
{
	__LIKELY_IF(event_id == EVENT_DMAC_CYCLE) {
		do_dma_internal();
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
		dma_wrap = ((data & _mask) != 0) ? true : false;
//		this->write_signal(SIG_TOWNS_DMAC_ADDR_MASK, data, mask);
	} else if((id >= SIG_TOWNS_DMAC_EOT_CH0) && (id <= SIG_TOWNS_DMAC_EOT_CH3)) {
		int ch = (id - SIG_TOWNS_DMAC_EOT_CH0) & 3;
		if(((data & _mask) != 0) && !(end_req[ch])){
			end_req[ch] = true;
			out_debug_log(_T("END#%d ASSERTED"), ch);
			if(!(_SINGLE_MODE_DMA)) {
				if((dma[ch].mode & 0xc0) != 0x40) { // WITHOUT SINGLE
					do_end_sequence(ch, false); // Check immediately.
				}
			}
		}
	} else {
		__LIKELY_IF((id >= SIG_UPD71071_CH0) && (id <= SIG_UPD71071_CH3)) {
	//		out_debug_log(_T("DRQ#%d %s"), ch, ((data & _mask) != 0) ? _T("ON ") : _T("OFF"));
			int ch = (id - SIG_UPD71071_CH0) & 3;
			uint8_t bit = 1 << ch;
			uint8_t _mode = dma[ch].mode & 0xc0;
			if(data & _mask) {
				if(!(req & bit)) {
					req |= bit;
					if(!(_SINGLE_MODE_DMA)) {
						check_start_condition();
						call_dma(ch); // OK?
					}
				}
			} else {
				uint8_t _b = req & bit;
				req &= ~bit;
				if(!(_SINGLE_MODE_DMA) && (_b)) { // ON -> OFF
					check_start_condition();
					check_running();
				}
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

#define STATE_VERSION	10

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
	state_fio->StateArray(is_started, sizeof(is_started), 1);
	state_fio->StateArray(end_req, sizeof(end_req), 1);
	state_fio->StateValue(div_count);

	state_fio->StateValue(event_dmac_cycle);

	if(loading) {
		for(int ch = 0; ch < 4; ch++) {
			calc_transfer_status(ch);
		}
	}
	return true;
}


}
