/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ uPD71071 ]
*/

#include "upd71071.h"
#include "debugger.h"

void UPD71071::initialize()
{
	DEVICE::initialize();
	_SINGLE_MODE_DMA = osd->check_feature(_T("SINGLE_MODE_DMA"));
	for(int i = 0; i < 4; i++) {
		dma[i].areg = dma[i].bareg = 0;
		dma[i].creg = dma[i].bcreg = 0;
	}
	if(__USE_DEBUGGER) {
		if(d_debugger != NULL) {
			d_debugger->set_device_name(_T("Debugger (uPD71071 DMAC)"));
			d_debugger->set_context_mem(this);
			d_debugger->set_context_io(vm->dummy);
		}
	}
	for(int i = 0; i < 4; i++) {
		inputs_ube[i] = false; // This is input, maybe not initialize at reset().
	}
}

void UPD71071::reset()
{
	for(int i = 0; i < 4; i++) {
		dma[i].mode = 0x04;
		dma[i].is_16bit = false;
		dma[i].end = false;
		dma[i].endreq = false;
		reset_ube(i);
	}
	b16 = selch = base = 0;
	cmd = tmp = 0;
	req = sreq = 0;
	mask = 0x0f;
//	mask = 0x00;
	running = false;
	reset_all_tc();
}
#if 0
void UPD71071::write_io16(uint32_t addr, uint32_t data)
{
	pair32_t _d, _bd;
//	if(b16 != 0) {
		switch(addr & 0x0e) {
		case 0x02:
			if(base == 0) {
				dma[selch].creg = data & 0xffff;
			}
			dma[selch].bcreg = data & 0xffff;
			return;
			break;
		case 0x04:
			if(base == 0) {
				_d.d = dma[selch].areg;
				_d.w.l = data;
				dma[selch].areg = _d.d;
			}
			_d.d = dma[selch].bareg;
			_d.w.l = data;
			dma[selch].bareg = _d.d;
			break;
		case 0x06:
			if(base == 0) {
				_d.d = dma[selch].areg;
				_d.b.h2 = data;
				dma[selch].areg = _d.d;
			}
			_d.d = dma[selch].bareg;
			_d.b.h2 = data;
			dma[selch].bareg = _d.d;
			break;
		case 0x08:
			cmd = data & 0xffff;
			break;
		default:
//			write_io8(addr & 0x0e, data);
			write_io8(addr, data);
			break;
		}
//	} else {
//		write_io8(addr, data);
////		write_io8((addr & 0x0e) + 0, data & 0xff);
////		write_io8((addr & 0x0e) + 1, (data >> 8) & 0xff);
//	}
}
#endif

void UPD71071::write_io8(uint32_t addr, uint32_t data)
{
	pair32_t _d;
	uint8_t ad[4];
	switch(addr & 0x0f) {
	case 0x00:
		b16 = data & 2;
		if(data & 1) {
			// dma reset
//			b16 = 0;
			for(int i = 0; i < 4; i++) {
				dma[i].mode = 0x04;
				dma[i].is_16bit = false;
				dma[i].end = false;
				dma[i].endreq = false;
				reset_ube(i);
			}
			selch = base = 0;
			cmd = tmp = 0;
			sreq = 0;
			mask = 0x0f;
//			mask = 0x00;
			reset_all_tc();
		}
		break;
	case 0x01:
		selch = data & 3;
		base = data & 4;
		break;
	case 0x02:
	case 0x03:
		dma[selch].bcreg = manipulate_a_byte_from_word_le(dma[selch].bcreg, (addr & 0x0f) - 2, data);
		if(base == 0) {
			dma[selch].creg = manipulate_a_byte_from_word_le(dma[selch].creg, (addr & 0x0f) - 2, data);
		}
		dma[selch].end = false; // OK?
		dma[selch].endreq = false; // OK?
		reset_tc(selch);
		break;
	case 0x04:
	case 0x05:
	case 0x06:
		dma[selch].bareg = manipulate_a_byte_from_dword_le(dma[selch].bareg, (addr & 0x0f) - 4, data);
		if(base == 0) {
			dma[selch].areg = manipulate_a_byte_from_dword_le(dma[selch].areg, (addr & 0x0f) - 4, data);
		}
		break;
	case 0x08:
		cmd = (cmd & 0xff00) | (data & 0x00ff);
		break;
	case 0x09:
		cmd = (cmd & 0xff) | ((data & 0x00ff) << 8);
		break;
	case 0x0a:
		dma[selch].mode = data;
		//dma[selch].is_16bit = ((data & 1) != 0) ? true : false;
		if((data & 0x04) == 0) {
			dma[selch].end = false;
			dma[selch].endreq = false;
		}
		set_ube(selch);
		break;
	case 0x0e:
		sreq = data;
		for(int _ch = 0; _ch < 4; _ch++) {
			if((sreq & (1 << _ch)) != 0) {
				//if((dma[_ch].mode & 0xc0) == 0x40) { // Single
				do_dma_per_channel(_ch);
				//}
			}
		}
		break;
	case 0x0f:
		mask = data;
		for(int _ch = 0; _ch < 4; _ch++) {
			if(((sreq | req) & (1 << _ch)) != 0) {
				if((mask & (1 << _ch)) == 0) {
					do_dma_per_channel(_ch);
				}
			}
		}
		set_ube(selch);
		break;
	}
}
#if 0
uint32_t UPD71071::read_io16(uint32_t addr)
{
//	if(b16 != 0) {
	switch(addr & 0x0e) {
	case 0x02:
		if(base == 0) {
			return (dma[selch].creg & 0xffff);
		} else {
			return (dma[selch].bcreg & 0xffff);
		}
		break;
	case 0x04:
		if(base == 0) {
			return (dma[selch].areg & 0xffff);
		} else {
			return (dma[selch].bareg & 0xffff);
		}
		break;
	case 0x06:
		if(base == 0) {
			return ((dma[selch].areg >> 16) & 0xff);
		} else {
			return ((dma[selch].bareg >> 16) & 0xff);
		}
		break;
	case 0x08:
		return (uint32_t)(cmd & 0xffff);
		break;
	default:
//			return read_io8(addr & 0x0e);
		break;
	}
	return read_io8(addr);
}
#endif
uint32_t UPD71071::read_io8(uint32_t addr)
{
	uint32_t val;
	pair32_t _d;
//	_d.d = 0;
//	_bd.d = 0;
	switch(addr & 0x0f) {
	case 0x00:
		return b16;
	case 0x01:
		// Q: Should be right BIT shift of BASE bit? 20200315 K.O
		return ((base != 0) ? 0x10 : 0x00) | (1 << selch);
	case 0x02:
		if(base != 0) {
			return dma[selch].bcreg & 0xff;
		} else {
			return dma[selch].creg & 0xff;
		}
	case 0x03:
		if(base != 0) {
			return (dma[selch].bcreg >> 8) & 0xff;
		} else {
			return (dma[selch].creg >> 8) & 0xff;
		}
	case 0x04:
	case 0x05:
	case 0x06:
		if(base == 0) {
			_d.d = dma[selch].areg;
		} else {
			_d.d = dma[selch].bareg;
		}
		switch(addr & 0x0f) {
		case 4:
			return _d.b.l;
			break;
		case 5:
			return _d.b.h;
			break;
		case 6:
			return _d.b.h2;
			break;
		}
		return 0x00;
		break;
	case 0x08:
		return cmd & 0xff;
	case 0x09:
		return (cmd >> 8) & 0xff;
	case 0x0a:
		return dma[selch].mode;
	case 0x0b:
		val = (req << 4) | tc;
		reset_all_tc();
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

void UPD71071::write_signal(int id, uint32_t data, uint32_t _mask)
{
	int ch = id & 3;
	uint8_t bit = 1 << ch;
	if((id >= SIG_UPD71071_CH0) && (id <= SIG_UPD71071_CH3)) {
//		out_debug_log(_T("DRQ#%d %s"), ch, ((data & _mask) != 0) ? _T("ON ") : _T("OFF"));
		if(data & _mask) {
			if(!(req & bit)) {
				req |= bit;
				if(!(_SINGLE_MODE_DMA)) {
					if((mask & (1 << ch)) == 0) { // MASK register MASKS DRQ.20200918 K.O
						// Without #define SINGLE_MODE_DMA ,
						// DMA trasfer is triggerd by SIGNAL or writing I/O 0Eh.
						do_dma_per_channel(ch);
						req &= ~bit;
					}
				}
			}
		} else {
			req &= ~bit;
		}
	} else if((id >= SIG_UPD71071_UBE_CH0) && (id <= SIG_UPD71071_UBE_CH3)) {
		inputs_ube[ch] = ((data & _mask) != 0) ? true : false;
	} else if((id >= SIG_UPD71071_EOT_CH0) && (id <= SIG_UPD71071_EOT_CH3)) {
		if((cmd & 0x04) == 0) {
			switch(dma[ch].mode & 0xc0) {
			case 0x00: // Demand
				dma[ch].endreq = true;
				break;
			case 0x40: // Single -> Noop
				break;
			case 0x80: // Demand
				dma[ch].endreq = true;
				break;
			default:
				break;
			}
		}
	}
}

uint32_t UPD71071::read_signal(int ch)
{
	if((ch >= (SIG_UPD71071_IS_TRANSFERING + 0)) && (ch < (SIG_UPD71071_IS_TRANSFERING + 4))) {
		int _nch = ch - SIG_UPD71071_IS_TRANSFERING;
		if((cmd & 0x04) != 0) return 0x00; // Not transfering
		if(dma[_nch].creg == 0xffffffff) return 0x00; //
		return 0xffffffff;
	} else if((ch >= (SIG_UPD71071_IS_16BITS_TRANSFER + 0)) && (ch < (SIG_UPD71071_IS_16BITS_TRANSFER + 4))) {
		int _nch = ch - SIG_UPD71071_IS_16BITS_TRANSFER;
		bool stat = stats_ube[_nch];
		return (stat) ? 0xffffffff : 0;
	} else if((ch >= SIG_UPD71071_UBE_CH0) && (ch <= SIG_UPD71071_UBE_CH3)) {
		return (inputs_ube[ch - SIG_UPD71071_UBE_CH0]) ? 0xffffffff : 0x00000000;
	} else if((ch >= (SIG_UPD71071_CREG + 0)) && (ch < (SIG_UPD71071_CREG + 4))) {
		return dma[ch - SIG_UPD71071_CREG].creg;
	} else if((ch >= (SIG_UPD71071_BCREG + 0)) && (ch < (SIG_UPD71071_BCREG + 4))) {
		return dma[ch - SIG_UPD71071_BCREG].creg;
	} else if((ch >= (SIG_UPD71071_AREG + 0)) && (ch < (SIG_UPD71071_AREG + 4))) {
		return dma[ch - SIG_UPD71071_AREG].creg;
	} else if((ch >= (SIG_UPD71071_BAREG + 0)) && (ch < (SIG_UPD71071_BAREG + 4))) {
		return dma[ch - SIG_UPD71071_BAREG].creg;
	}
	return 0;
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


void UPD71071::do_dma_verify_8bit(int c, bool extended, bool compressed, int& wait)
{
	bool __debugging = false;
	int wait_1 = 0, wait_2 = 0;
	__LIKELY_IF((__USE_DEBUGGER) && (d_debugger != NULL)) {
		__debugging = d_debugger->now_device_debugging;
	}
	// verify
	reset_ube(c);

	uint32_t val = dma[c].dev->read_dma_io8w(0, &wait_1);
	uint32_t val2;
	__UNLIKELY_IF(__debugging) {
		val2 = d_debugger->read_via_debugger_data8w(dma[c].areg, &wait_2);
	} else {
		val2 = read_via_debugger_data8w(dma[c].areg,  &wait_2);
	}
	// ToDo: Compare val1 and val2.
	// update temporary register
	tmp = (tmp >> 8) | (val << 8);

	wait += compressed ? 5 : 7;
	if(extended) {
		wait += wait_1 + wait_2;
	}
}

void UPD71071::do_dma_dev_to_mem_8bit(int c, bool extended, bool compressed, int& wait)
{
	reset_ube(c);
	// io -> memory
	bool __debugging = false;
	int wait_r = 0, wait_w = 0;
	__LIKELY_IF((__USE_DEBUGGER) && (d_debugger != NULL)) {
		__debugging = d_debugger->now_device_debugging;
	}
	uint32_t val;
//	reset_ube(c);

	val = dma[c].dev->read_dma_io8w(0, &wait_r);
	// update temporary register
	tmp = (tmp >> 8) | (val << 8);
	__UNLIKELY_IF(__debugging) {
		d_debugger->write_via_debugger_data8w(dma[c].areg, val, &wait_w);
	} else {
		write_via_debugger_data8w(dma[c].areg,  val, &wait_w);
	}

	wait += compressed ? 5 : 7;
	if(extended) {
		wait += wait_r + wait_w;
	}
}

void UPD71071::do_dma_mem_to_dev_8bit(int c, bool extended, bool compressed, int& wait)
{
	// memory -> io
	bool __debugging = false;
	int wait_r = 0, wait_w = 0;
	__LIKELY_IF((__USE_DEBUGGER) && (d_debugger != NULL)) {
		__debugging = d_debugger->now_device_debugging;
	}
	uint32_t val;
	reset_ube(c);
	__UNLIKELY_IF(__debugging) {
		val = d_debugger->read_via_debugger_data8w(dma[c].areg, &wait_r);
	} else {
		val = read_via_debugger_data8w(dma[c].areg, &wait_r);
	}
	// update temporary register
	tmp = (tmp >> 8) | (val << 8);

	dma[c].dev->write_dma_io8w(0, val, &wait_w);

	wait += compressed ? 5 : 7;
	if(extended) {
		wait += wait_r + wait_w;
	}
}

void UPD71071::do_dma_inc_dec_ptr_8bit(int c)
{
	// Note: FM-Towns may extend to 32bit.
	if(dma[c].mode & 0x20) {
		dma[c].areg = (dma[c].areg - 1) & 0xffffff;
	} else {
		dma[c].areg = (dma[c].areg + 1) & 0xffffff;
	}
}

void UPD71071::do_dma_verify_16bit(int c, bool extended, bool compressed, int& wait)
{
	// verify
	bool __debugging = false;
	int wait_1 = 0, wait_2 = 0;

	__LIKELY_IF((__USE_DEBUGGER) && (d_debugger != NULL)) {
		__debugging = d_debugger->now_device_debugging;
	}
	// verify
	set_ube(c);
	uint32_t val = dma[c].dev->read_dma_io16w(0, &wait_1);
	uint32_t val2;
	__UNLIKELY_IF(__debugging) {
		val2 = d_debugger->read_via_debugger_data16w(dma[c].areg, &wait_2);
	} else {
		val2 = read_via_debugger_data16w(dma[c].areg,  &wait_2);
	}

	// ToDo: Compare val1 and val2.
	// update temporary register
	tmp = val;

	wait += compressed ? 5 : 7;
	if(extended) {
		wait += wait_1 + wait_2;
	}
}

void UPD71071::do_dma_dev_to_mem_16bit(int c, bool extended, bool compressed, int& wait)
{
	// io -> memory
	bool __debugging = false;
	int wait_r = 0, wait_w = 0;
	__LIKELY_IF((__USE_DEBUGGER) && (d_debugger != NULL)) {
		__debugging = d_debugger->now_device_debugging;
	}
	// ToDo: For odd address.
	uint32_t val;
	set_ube(c);
	val = dma[c].dev->read_dma_io16w(0, &wait_r);
	// update temporary register
	tmp = val;

	__UNLIKELY_IF(__debugging) {
		d_debugger->write_via_debugger_data16w(dma[c].areg, val, &wait_w);
	} else {
		write_via_debugger_data16w(dma[c].areg,  val, &wait_w);
	}

	wait += compressed ? 5 : 7;
	if(extended) {
		wait += wait_r + wait_w;
	}
}

void UPD71071::do_dma_mem_to_dev_16bit(int c, bool extended, bool compressed, int& wait)
{
	// memory -> io
	bool __debugging = false;
	int wait_r = 0, wait_w = 0;
	__LIKELY_IF((__USE_DEBUGGER) && (d_debugger != NULL)) {
		__debugging = d_debugger->now_device_debugging;
	}
	// ToDo: For odd address.
	uint32_t val;
	set_ube(c);
	__UNLIKELY_IF(__debugging) {
		val = d_debugger->read_via_debugger_data16w(dma[c].areg, &wait_r);
	} else {
		val = read_via_debugger_data16w(dma[c].areg, &wait_r);
	}
	// update temporary register
	tmp = val;

	dma[c].dev->write_dma_io16w(0, val, &wait_w);

	wait += compressed ? 5 : 7;
	if(extended) {
		wait += wait_r + wait_w;
	}
}

void UPD71071::do_dma_inc_dec_ptr_16bit(int c)
{
	// Note: FM-Towns may extend to 32bit.
	if(dma[c].mode & 0x20) {
		dma[c].areg = (dma[c].areg - 2) & 0xffffff;
	} else {
		dma[c].areg = (dma[c].areg + 2) & 0xffffff;
	}
}

bool UPD71071::do_dma_epilogue(int c)
{
	c = c & 3;
	uint8_t bit = 1 << c;

#if 0 /* SYNC TO UPSTREAM */
	if(dma[c].creg-- == 0) {
		//if(dma[c].endreq) dma[c].end = true;
		if(dma[c].mode & 0x10) {
			// auto initialize
			dma[c].areg = dma[c].bareg;
			dma[c].creg = dma[c].bcreg;
		} else {
			mask |= bit;
		}
		req &= ~bit;
		sreq &= ~bit;
		running = false;
		set_tc(c);
		//write_signals(&outputs_tc, 0xffffffff);
		if((dma[c].mode & 0xc0) == 0x40) {
			// Single mode
			return true;
		} else {
			return false;
		}
	} else if((dma[c].mode & 0xc0) == 0x40) {
		// Single mode
		return true;
	}
	return false;

	// Note: At FM-Towns, SCSI's DMAC will be set after
	//       SCSI bus phase become DATA IN/DATA OUT.
	//       Before bus phase became DATA IN/DATA OUT,
	//       DMAC mode and state was unstable (and ASSERTED
	//       DRQ came from SCSI before this state change).
	// ToDo: Stop correctly before setting.
	//       -- 20200316 K.O
#else
//	if(dma[c].end) return true; // OK?
	if((dma[c].creg == 0) || ((dma[c].endreq) && !(dma[c].end) && ((dma[c].mode & 0xc0) != 0x40))) {  // OK?
		if(dma[c].endreq) dma[c].end = true;
		bool is_tc = false;
		dma[c].creg--;
		if(dma[c].end) is_tc = true;
		// TC
		if(dma[c].bcreg < dma[c].creg) {
			is_tc = true;
		}
		if(dma[c].mode & 0x10) {
			// auto initialize
			dma[c].areg = dma[c].bareg;
			dma[c].creg = dma[c].bcreg;
		} else {
			mask |= bit;
		}
		req &= ~bit;
		sreq &= ~bit;
		running = false;
		if(is_tc) {
			set_tc(c);
		}
		if((dma[c].mode & 0xc0) == 0x40) {
			// Single mode
			return true;
		} else {
			return false;
		}
	}
	dma[c].creg--;
	// Note: At FM-Towns, SCSI's DMAC will be set after
	//       SCSI bus phase become DATA IN/DATA OUT.
	//       Before bus phase became DATA IN/DATA OUT,
	//       DMAC mode and state was unstable (and ASSERTED
	//       DRQ came from SCSI before this state change).
	// ToDo: Stop correctly before setting.
	//       -- 20200316 K.O
	if((dma[c].mode & 0xc0) == 0x40){
		// single mode
		//req &= ~bit;
		//sreq &= ~bit;
		running = false;
		return true;
	}
#endif
	return false;
}

bool UPD71071::do_dma_per_channel(int c)
{
	reset_dma_ack(c);
	if(cmd & 4) {
		return true;
	}
	if(dma[c].end) {
		if((dma[c].mode & 0xc0) != 0x40) { // Without Single
			return true;
		}
	}
	uint8_t bit = 1 << c;

	if(((req | sreq) & bit) /*&& !(mask & bit)*/) {
		// execute dma
		//while((req | sreq) & bit) { // Q: Will use burst transfer? 20230319 K.O
			int wait = 0;
			bool compressed = ((cmd & 0x08) != 0);
			bool extended = ((cmd & 0x20) != 0);
			//reset_dma_ack(c);
			if(!running) {
				wait += 2; // S0
				running = true;
			}
			// SINGLE
			// Will check WORD transfer mode for FM-Towns.(mode.bit0 = '1).
			// Note: At FM-Towns, may set bit0 of mode register (B/W),
			//       but transferring per 8bit from/to SCSI HOST...
			///      I wonder this...
			// 2020-03-16 K.O
			if((dma[c].is_16bit) && (inputs_ube[c]) /*&& (b16 != 0)*/) {
				// This channel transferr makes 16bit.
				if((dma[c].mode & 0x0c) == 0x00) {
					do_dma_verify_16bit(c, extended, compressed, wait);
				} else if((dma[c].mode & 0x0c) == 0x04) {
					do_dma_dev_to_mem_16bit(c, extended, compressed, wait);
				} else if((dma[c].mode & 0x0c) == 0x08) {
					do_dma_mem_to_dev_16bit(c, extended, compressed, wait);
				}
				do_dma_inc_dec_ptr_16bit(c);
			} else {
				// 8bit transfer mode
				if((dma[c].mode & 0x0c) == 0x00) {
					do_dma_verify_8bit(c, extended, compressed, wait);
				} else if((dma[c].mode & 0x0c) == 0x04) {
					do_dma_dev_to_mem_8bit(c, extended, compressed, wait);
				} else if((dma[c].mode & 0x0c) == 0x08) {
					do_dma_mem_to_dev_8bit(c, extended, compressed, wait);
				}
				do_dma_inc_dec_ptr_8bit(c);
			}
			if(d_cpu != NULL) d_cpu->set_extra_clock(wait);

			set_dma_ack(c);
			if(do_dma_epilogue(c)) {
//				//break;
//				if(_SINGLE_MODE_DMA) {
					return true;
//				}
//			set_dma_ack(c);
			}
		//}
	}
	return false;
}

void UPD71071::do_dma()
{
	// check DDMA
	if(cmd & 4) {
		return;
	}

	// run dma
	for(int c = 0; c < 4; c++) {
		if((mask & (1 << c)) == 0) { // MASK
			if((dma[c].mode & 0xc0) == 0x00) { // Demand
				if(!(dma[c].end)) {
					do_dma_per_channel(c);
				}
			} else if((dma[c].mode & 0xc0) == 0x40) { // Single
				if(do_dma_per_channel(c)) break;
			} else if((dma[c].mode & 0xc0) == 0xc0) { // Block (ToDo)
				if(do_dma_per_channel(c)) break;
			}
		}
	}
//#ifdef SINGLE_MODE_DMA
	if(_SINGLE_MODE_DMA) {
		if(d_dma) {
			d_dma->do_dma();
		}
	}
//#endif
}

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
	_T("16Bit=%s\n")
	_T("SELECT CH=%d BASE=%02X REQ=%02X SREQ=%02X MASK=%02X TC=%02X ")
	_T("CMD=%04X TMP=%04X\n")
	_T("CH0 AREG=%04X CREG=%04X BAREG=%04X BCREG=%04X REQ=%d MASK=%d MODE=%02X %s\n")
	_T("CH1 AREG=%04X CREG=%04X BAREG=%04X BCREG=%04X REQ=%d MASK=%d MODE=%02X %s\n")
	_T("CH2 AREG=%04X CREG=%04X BAREG=%04X BCREG=%04X REQ=%d MASK=%d MODE=%02X %s\n")
	_T("CH3 AREG=%04X CREG=%04X BAREG=%04X BCREG=%04X REQ=%d MASK=%d MODE=%02X %s"),
	(b16 != 0) ? _T("YES") : _T("NO"),
	selch, base, req, sreq, mask, tc,
	cmd, tmp,
	dma[0].areg, dma[0].creg, dma[0].bareg, dma[0].bcreg, ((req | sreq) >> 0) & 1, (mask >> 0) & 1, dma[0].mode, dir[(dma[0].mode >> 2) & 3],
	dma[1].areg, dma[1].creg, dma[1].bareg, dma[1].bcreg, ((req | sreq) >> 1) & 1, (mask >> 1) & 1, dma[1].mode, dir[(dma[1].mode >> 2) & 3],
	dma[2].areg, dma[2].creg, dma[2].bareg, dma[2].bcreg, ((req | sreq) >> 2) & 1, (mask >> 2) & 1, dma[2].mode, dir[(dma[2].mode >> 2) & 3],
	dma[3].areg, dma[3].creg, dma[3].bareg, dma[3].bcreg, ((req | sreq) >> 3) & 1, (mask >> 3) & 1, dma[3].mode, dir[(dma[3].mode >> 2) & 3]);
	return true;
}

#define STATE_VERSION	6

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
		state_fio->StateValue(dma[i].is_16bit);
		state_fio->StateValue(dma[i].endreq);
		state_fio->StateValue(dma[i].end);
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

	state_fio->StateArray(inputs_ube, sizeof(inputs_ube), 1);
	state_fio->StateArray(stats_ube, sizeof(stats_ube), 1);

	return true;
}
