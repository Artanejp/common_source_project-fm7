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
	_USE_DEBUGGER    = osd->check_feature(_T("USE_DEBUGGER"));
	for(int i = 0; i < 4; i++) {
		dma[i].areg = dma[i].bareg = 0;
		dma[i].creg = dma[i].bcreg = 0;
	}
	if(_USE_DEBUGGER) {
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
		dma[selch].is_16bit = false;
		reset_ube(i);
	}
	b16 = selch = base = 0;
	cmd = tmp = 0;
	req = sreq = 0;
	mask = 0x0f;
//	mask = 0x00;
	reset_all_tc();
}

void UPD71071::reset_all_tc()
{
	for(int i = 0; i < 4; i++) {
		tc = 0;
		write_signals(&(outputs_tc[i]), 0);
	}
}

void UPD71071::reset_tc(int ch)
{
	if((ch < 0) || (ch > 3)) return;
	uint8_t bit = (1 << ch);
	uint8_t tc_bak = tc;
	tc &= ~bit;
	/*if(tc != tc_bak) */ write_signals(&(outputs_tc[ch]), 0);
}

void UPD71071::set_tc(int ch)
{
	if((ch < 0) || (ch > 3)) return;
	uint8_t bit = (1 << ch);
	uint8_t tc_bak = tc;
	tc |= bit;
	/*if(tc != tc_bak) */write_signals(&(outputs_tc[ch]), 0xffffffff);
}

void UPD71071::write_io8(uint32_t addr, uint32_t data)
{
	pair32_t _d;
	pair32_t _bd;
	_d.d = 0;
	_bd.d = 0;
	switch(addr & 0x0f) {
	case 0x00:
		if(data & 1) {
			// dma reset
			for(int i = 0; i < 4; i++) {
				dma[i].mode = 0x04;
				dma[i].is_16bit = false;
				reset_ube(i);
			}
			selch = base = 0;
			cmd = tmp = 0;
			sreq = 0;
			mask = 0x0f;
//			mask = 0x00;
			reset_all_tc();
		}
		b16 = data & 2;
		break;
	case 0x01:
		selch = data & 3;
		base = data & 4;
		break;
	case 0x02:
	case 0x03:
		_d.w.l  = dma[selch].creg;
		_bd.w.l = dma[selch].bcreg;
		switch(addr & 0x0f) {
		case 0x02:
			_d.b.l  = data;
			_bd.b.l = data;
			break;
		case 0x03:
			_d.b.h  = data;
			_bd.b.h = data;
			break;
		}			
		if(base == 0) {
			dma[selch].creg = _d.w.l;
		}
		dma[selch].bcreg = _bd.w.l;
		reset_tc(selch);
		break;
	case 0x04:
	case 0x05:
	case 0x06:
		_d.d  = dma[selch].areg;
		_bd.d = dma[selch].bareg;
		switch(addr & 0x0f) {
		case 0x04:
			_d.b.l   = data;
			_bd.b.l  = data;
			break;
		case 0x05:
			_d.b.h   = data;
			_bd.b.h  = data;
			break;
		case 0x06:
			_d.b.h2  = data;
			_bd.b.h2 = data;
			break;
		}
		if(base == 0) {
			dma[selch].areg = _d.d;
		}
		dma[selch].bareg = _bd.d;
		break;
	case 0x08:
		cmd = (cmd & 0xff00) | data;
		break;
	case 0x09:
		cmd = (cmd & 0xff) | (data << 8);
		break;
	case 0x0a:
		dma[selch].mode = data;
		dma[selch].is_16bit = ((data & 1) != 0) ? true : false;
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

uint32_t UPD71071::read_io8(uint32_t addr)
{
	uint32_t val;
//	pair32_t _d;
//	pair32_t _bd;
//	_d.d = 0;
//	_bd.d = 0;
	switch(addr & 0x0f) {
	case 0x00:
		return b16;
	case 0x01:
		// Q: Should be right BIT shift of BASE bit? 20200315 K.O
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
#if 1				
				if((mask & (1 << ch)) == 0) { // MASK register MASKS DRQ.20200918 K.O
					// Without #define SINGLE_MODE_DMA ,
					// DMA trasfer is triggerd by SIGNAL or writing I/O 0Eh.
					do_dma_per_channel(ch);
//					req &= ~bit;
				}
#endif
			}
		} else {
			req &= ~bit;
		}
	} else if((id >= SIG_UPD71071_UBE_CH0) && (id <= SIG_UPD71071_UBE_CH3)) {
		inputs_ube[ch] = ((data & _mask) != 0) ? true : false;
	} else if((id >= SIG_UPD71071_EOT_CH0) && (id <= SIG_UPD71071_EOT_CH3)) {
		set_tc(ch);
	}
}

void UPD71071::set_ube(int ch)
{
	bool stat = inputs_ube[ch & 3];
	stat &= dma[ch & 3].is_16bit; 
	if(stats_ube[ch & 3] != stat) {
		write_signals(&outputs_ube[ch & 3], (stat) ? 0xffffffff : 0x00000000);
		stats_ube[ch & 3] = stat;
	}
}
void UPD71071::reset_ube(int ch)
{
	if(stats_ube[ch &3]) {
		write_signals(&outputs_ube[ch & 3], 0x00000000);
		stats_ube[ch & 3] = false;
	}
}

void UPD71071::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
	d_mem->write_dma_data8(addr, data);
}

uint32_t UPD71071::read_via_debugger_data8(uint32_t addr)
{
	return d_mem->read_dma_data8(addr);
}

void UPD71071::write_via_debugger_data16(uint32_t addr, uint32_t data)
{
	d_mem->write_dma_data16(addr, data);
}

uint32_t UPD71071::read_via_debugger_data16(uint32_t addr)
{
	return d_mem->read_dma_data16(addr);
}

// note: if SINGLE_MODE_DMA is defined, do_dma() is called in every machine cycle

uint32_t UPD71071::read_signal(int ch)
{
	if((ch >= (SIG_UPD71071_IS_TRANSFERING + 0)) && (ch < (SIG_UPD71071_IS_TRANSFERING + 4))) {
		int _nch = ch - SIG_UPD71071_IS_TRANSFERING;
		if((cmd & 0x04) != 0) return 0x00; // Not transfering
		if((dma[_nch].creg == 0xffffffff)) return 0x00; //
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

void UPD71071::do_dma_verify_8bit(int c)
{
	// verify
	uint32_t val = dma[c].dev->read_dma_io8(0);
	// update temporary register
	reset_ube(c);
	tmp = (tmp >> 8) | (val << 8);

}
void UPD71071::do_dma_dev_to_mem_8bit(int c)
{
	// io -> memory
	uint32_t val;
	reset_ube(c);
	val = dma[c].dev->read_dma_io8(0);
	if(_USE_DEBUGGER) {
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			d_debugger->write_via_debugger_data8(dma[c].areg, val);
		} else {
			this->write_via_debugger_data8(dma[c].areg, val);
		}
	} else {
		this->write_via_debugger_data8(dma[c].areg, val);
	}							
	// update temporary register
	tmp = (tmp >> 8) | (val << 8);

}

void UPD71071::do_dma_mem_to_dev_8bit(int c)
{
	// memory -> io
	uint32_t val;
	reset_ube(c);
	if(_USE_DEBUGGER) {
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			val = d_debugger->read_via_debugger_data8(dma[c].areg);
		} else {
			val = this->read_via_debugger_data8(dma[c].areg);
		}
	} else {
		val = this->read_via_debugger_data8(dma[c].areg);
	}
	dma[c].dev->write_dma_io8(0, val);
	// update temporary register
	tmp = (tmp >> 8) | (val << 8);
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

void UPD71071::do_dma_verify_16bit(int c)
{
	// verify
	uint32_t val = dma[c].dev->read_dma_io16(0);
	set_ube(c);
	// update temporary register
	tmp = val;

}
void UPD71071::do_dma_dev_to_mem_16bit(int c)
{
	// io -> memory
	uint32_t val;
	set_ube(c);
	val = dma[c].dev->read_dma_io16(0);
	if((dma[c].areg & 1) != 0) {
		// If odd address, write a byte.
		uint32_t tval = (val >> 8) & 0xff;
		if(_USE_DEBUGGER) {
			if(d_debugger != NULL && d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data8(dma[c].areg, tval);
			} else {
				this->write_via_debugger_data8(dma[c].areg, tval);
			}
		} else {
			this->write_via_debugger_data8(dma[c].areg, tval);
		}
	} else {
		// 16bit
		if(_USE_DEBUGGER) {
			if(d_debugger != NULL && d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data16(dma[c].areg, val);
			} else {
				this->write_via_debugger_data16(dma[c].areg, val);
			}
		} else {
			this->write_via_debugger_data16(dma[c].areg, val);
		}
	}
	// update temporary register
	tmp = val;
}

void UPD71071::do_dma_mem_to_dev_16bit(int c)
{
	// memory -> io
	uint32_t val;
	set_ube(c);
	if(_USE_DEBUGGER) {
		if(d_debugger != NULL && d_debugger->now_device_debugging) {
			val = d_debugger->read_via_debugger_data16(dma[c].areg);
		} else {
			val = this->read_via_debugger_data16(dma[c].areg);
		}
	} else {
		val = this->read_via_debugger_data16(dma[c].areg);
	}
	if((dma[c].areg & 1) != 0) {
		// If odd address, read a high byte.
		val = (val >> 8) & 0xff;
	}
	dma[c].dev->write_dma_io16(0, val);
	// update temporary register
	tmp = val;
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
	uint8_t bit = 1 << c;
	if(dma[c].creg == 0) {  // OK?
		dma[c].creg--;
		// TC
		bool is_tc = false;
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
		req &= ~bit;
		sreq &= ~bit;
		return true;
	} else if((dma[c].mode & 0xc0) == 0x00){
		// demand mode
		req &= ~bit;
		sreq &= ~bit;
		return false;
	}
	return false;
}

bool UPD71071::do_dma_per_channel(int c)
{
	if(cmd & 4) {
		return true;
	}
	uint8_t bit = 1 << c;
	if(((req | sreq) & bit) /*&& !(mask & bit)*/) {
		// execute dma
		{ // SINGLE
			// Will check WORD transfer mode for FM-Towns.(mode.bit0 = '1).
			// Note: At FM-Towns, may set bit0 of mode register (B/W),
			//       but transferring per 8bit from/to SCSI HOST...
			///      I wonder this...
			// 2020-03-16 K.O
			if((dma[c].is_16bit) && (inputs_ube[c]) && (b16)) {
				// This channel transferr makes 16bit.
				if((dma[c].mode & 0x0c) == 0x00) {
					do_dma_verify_16bit(c);
				} else if((dma[c].mode & 0x0c) == 0x04) {
					do_dma_dev_to_mem_16bit(c);
				} else if((dma[c].mode & 0x0c) == 0x08) {
					do_dma_mem_to_dev_16bit(c);
				}
				if((dma[c].areg & 1) != 0) {
					// If odd address, align next word to 2n.
					do_dma_inc_dec_ptr_8bit(c);
				} else {
					do_dma_inc_dec_ptr_16bit(c);
				}
			} else {
				// 8bit transfer mode
				if((dma[c].mode & 0x0c) == 0x00) {
					do_dma_verify_8bit(c);
				} else if((dma[c].mode & 0x0c) == 0x04) {
					do_dma_dev_to_mem_8bit(c);
				} else if((dma[c].mode & 0x0c) == 0x08) {
					do_dma_mem_to_dev_8bit(c);
				}
				do_dma_inc_dec_ptr_8bit(c);
			}
			if(do_dma_epilogue(c)) {
//				//break;
				return true;
			}
		}
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
		if(((dma[c].mode & 0xc0) == 0x40) && ((mask & (1 << c)) == 0)) { // Single
			if(do_dma_per_channel(c)) break;
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
	(b16) ? _T("YES") : _T("NO"),
	selch, base, req, sreq, mask, tc,
	cmd, tmp,
	dma[0].areg, dma[0].creg, dma[0].bareg, dma[0].bcreg, ((req | sreq) >> 0) & 1, (mask >> 0) & 1, dma[0].mode, dir[(dma[0].mode >> 2) & 3],
	dma[1].areg, dma[1].creg, dma[1].bareg, dma[1].bcreg, ((req | sreq) >> 1) & 1, (mask >> 1) & 1, dma[1].mode, dir[(dma[1].mode >> 2) & 3],
	dma[2].areg, dma[2].creg, dma[2].bareg, dma[2].bcreg, ((req | sreq) >> 2) & 1, (mask >> 2) & 1, dma[2].mode, dir[(dma[2].mode >> 2) & 3],
	dma[3].areg, dma[3].creg, dma[3].bareg, dma[3].bcreg, ((req | sreq) >> 3) & 1, (mask >> 3) & 1, dma[3].mode, dir[(dma[3].mode >> 2) & 3]);
	return true;
}

#define STATE_VERSION	3

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
	state_fio->StateArray(inputs_ube, sizeof(inputs_ube), 1);
	state_fio->StateArray(stats_ube, sizeof(stats_ube), 1);

	return true;
}

