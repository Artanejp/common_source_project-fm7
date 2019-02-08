/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ uPD71071 ]
*/

#include "upd71071.h"

void UPD71071::initialize()
{
	DEVICE::initialize();
	_SINGLE_MODE_DMA = osd->check_feature(_T("SINGLE_MODE_DMA"));
	
	for(int i = 0; i < 4; i++) {
		dma[i].areg = dma[i].bareg = 0;
		dma[i].creg = dma[i].bcreg = 0;
	}
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
		if(((sreq = data) != 0) && !(_SINGLE_MODE_DMA)) {
//#ifndef SINGLE_MODE_DMA
			do_dma();
//#endif
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
//#ifndef SINGLE_MODE_DMA
			if(!_SINGLE_MODE_DMA) do_dma();
//#endif
		}
	} else {
		req &= ~bit;
	}
}

// note: if SINGLE_MODE_DMA is defined, do_dma() is called in every machine cycle

uint32_t UPD71071::read_signal(int ch)
{
	if((ch >= (SIG_UPD71071_IS_TRANSFERING + 0)) && (ch < (SIG_UPD71071_IS_TRANSFERING + 4))) {
		bool _nch = ch - SIG_UPD71071_IS_TRANSFERING;
		if((cmd & 0x04) != 0) return 0x00; // Not transfering
		if((dma[_nch].creg == 0)) return 0x00; //
		return 0xffffffff;
	} else if((ch >= (SIG_UPD71071_IS_16BITS_TRANSFER + 0)) && (ch < (SIG_UPD71071_IS_16BITS_TRANSFER + 4))) {
		bool _nch = ch - SIG_UPD71071_IS_16BITS_TRANSFER;
		return ((dma[_nch].mode & 1) != 0) ? 0xffffffff : 0;
	}
	return 0;
}
			
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
				// Will check WORD transfer mode for FM-Towns.(mode.bit0 = '1).
				if((dma[c].mode & 0x01) == 1) {
					// 8bit transfer mode
					if((dma[c].mode & 0x0c) == 0x00) {
						// verify
						uint32_t val = dma[c].dev->read_dma_io16(0);
						// update temporary register
						tmp = val;
					} else if((dma[c].mode & 0x0c) == 0x04) {
						// io -> memory
						uint32_t val;
						val = dma[c].dev->read_dma_io16(0);
						d_mem->write_dma_data16(dma[c].areg, val);
						// update temporary register
						tmp = val;
					} else if((dma[c].mode & 0x0c) == 0x08) {
						// memory -> io
						uint32_t val = d_mem->read_dma_data16(dma[c].areg);
						dma[c].dev->write_dma_io16(0, val);
						// update temporary register
						tmp = val;
					}
					if(dma[c].mode & 0x20) {
						dma[c].areg = (dma[c].areg - 2) & 0xffffff;
					} else {
						dma[c].areg = (dma[c].areg + 2) & 0xffffff;
					}
					if(dma[c].creg-- == 0) {  // OK?
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
						
						write_signals(&outputs_tc, 0xffffffff);
					} else if(_SINGLE_MODE_DMA) {
						if((dma[c].mode & 0xc0) == 0x40) {
							// single mode
							break;
						}
					}
				} else {
					// 8bit transfer mode
					if((dma[c].mode & 0x0c) == 0x00) {
						// verify
						uint32_t val = dma[c].dev->read_dma_io8(0);
						// update temporary register
						tmp = (tmp >> 8) | (val << 8);
					} else if((dma[c].mode & 0x0c) == 0x04) {
						// io -> memory
						uint32_t val;
						val = dma[c].dev->read_dma_io8(0);
						d_mem->write_dma_data8(dma[c].areg, val);
						// update temporary register
						tmp = (tmp >> 8) | (val << 8);
					} else if((dma[c].mode & 0x0c) == 0x08) {
						// memory -> io
						uint32_t val = d_mem->read_dma_data8(dma[c].areg);
						dma[c].dev->write_dma_io8(0, val);
						// update temporary register
						tmp = (tmp >> 8) | (val << 8);
					}
					if(dma[c].mode & 0x20) {
						dma[c].areg = (dma[c].areg - 1) & 0xffffff;
					} else {
						dma[c].areg = (dma[c].areg + 1) & 0xffffff;
					}
				}
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
					
					write_signals(&outputs_tc, 0xffffffff);
//#ifdef SINGLE_MODE_DMA
				} else if(_SINGLE_MODE_DMA) {
					if((dma[c].mode & 0xc0) == 0x40) {
						// single mode
						break;
					}
//#endif
				}
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

#define STATE_VERSION	1

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
	return true;
}

