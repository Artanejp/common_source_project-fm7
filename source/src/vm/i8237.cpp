/*
	Skelton for retropc emulator

	Origin : MESS
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ i8237 ]
*/

#include "i8237.h"
#include "../fileio.h"

void I8237::reset()
{
	low_high = false;
	cmd = req = tc = 0;
	mask = 0xff;
}

void I8237::write_io8(uint32 addr, uint32 data)
{
	int ch = (addr >> 1) & 3;
	uint8 bit = 1 << (data & 3);
	
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

uint32 I8237::read_io8(uint32 addr)
{
	int ch = (addr >> 1) & 3;
	uint32 val = 0xff;
	
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

void I8237::write_signal(int id, uint32 data, uint32 mask)
{
	if(SIG_I8237_CH0 <= id && id <= SIG_I8237_CH3) {
		uint8 bit = 1 << (id & 3);
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
	} else if(SIG_I8237_BANK0 <= id && id <= SIG_I8237_BANK3) {
		// external bank registers
		dma[id & 3].bankreg = data & mask;
	} else if(SIG_I8237_MASK0 <= id && id <= SIG_I8237_MASK3) {
		// external bank registers
		dma[id & 3].incmask = data & mask;
	}
}

// note: if SINGLE_MODE_DMA is defined, do_dma() is called in every machine cycle

void I8237::do_dma()
{
	for(int ch = 0; ch < 4; ch++) {
		uint8 bit = 1 << ch;
		if((req & bit) && !(mask & bit)) {
			// execute dma
			while(req & bit) {
				if((dma[ch].mode & 0x0c) == 0) {
					// verify
				} else if((dma[ch].mode & 0x0c) == 4) {
					// io -> memory
					tmp = read_io(ch);
					write_mem(dma[ch].areg | (dma[ch].bankreg << 16), tmp);
				} else if((dma[ch].mode & 0x0c) == 8) {
					// memory -> io
					tmp = read_mem(dma[ch].areg | (dma[ch].bankreg << 16));
					write_io(ch, tmp);
				}
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
#ifdef SINGLE_MODE_DMA
				} else if((dma[ch].mode & 0xc0) == 0x40) {
					// single mode
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

void I8237::write_mem(uint32 addr, uint32 data)
{
	if(mode_word) {
		d_mem->write_dma_data16(addr << 1, data);
	} else {
		d_mem->write_dma_data8(addr, data);
	}
}

uint32 I8237::read_mem(uint32 addr)
{
	if(mode_word) {
		return d_mem->read_dma_data16(addr << 1);
	} else {
		return d_mem->read_dma_data8(addr);
	}
}

void I8237::write_io(int ch, uint32 data)
{
	if(mode_word) {
		dma[ch].dev->write_dma_io16(0, data);
	} else {
		dma[ch].dev->write_dma_io8(0, data);
	}
}

uint32 I8237::read_io(int ch)
{
	if(mode_word) {
		return dma[ch].dev->read_dma_io16(0);
	} else {
		return dma[ch].dev->read_dma_io8(0);
	}
}

#define STATE_VERSION	1

void I8237::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	for(int i = 0; i < 4; i++) {
		state_fio->FputUint16(dma[i].areg);
		state_fio->FputUint16(dma[i].creg);
		state_fio->FputUint16(dma[i].bareg);
		state_fio->FputUint16(dma[i].bcreg);
		state_fio->FputUint8(dma[i].mode);
		state_fio->FputUint16(dma[i].bankreg);
		state_fio->FputUint16(dma[i].incmask);
	}
	state_fio->FputBool(low_high);
	state_fio->FputUint8(cmd);
	state_fio->FputUint8(req);
	state_fio->FputUint8(mask);
	state_fio->FputUint8(tc);
	state_fio->FputUint32(tmp);
	state_fio->FputBool(mode_word);
}

bool I8237::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	for(int i = 0; i < 4; i++) {
		dma[i].areg = state_fio->FgetUint16();
		dma[i].creg = state_fio->FgetUint16();
		dma[i].bareg = state_fio->FgetUint16();
		dma[i].bcreg = state_fio->FgetUint16();
		dma[i].mode = state_fio->FgetUint8();
		dma[i].bankreg = state_fio->FgetUint16();
		dma[i].incmask = state_fio->FgetUint16();
	}
	low_high = state_fio->FgetBool();
	cmd = state_fio->FgetUint8();
	req = state_fio->FgetUint8();
	mask = state_fio->FgetUint8();
	tc = state_fio->FgetUint8();
	tmp = state_fio->FgetUint32();
	mode_word = state_fio->FgetBool();
	return true;
}

