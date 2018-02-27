/*
	Skelton for retropc emulator

	Origin : MESS
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ i8237 ]
*/

#include "vm.h"
#include "../emu.h"
#include "i8237.h"


I8237::I8237(VM* parent_vm, EMU* parent_emu) : I8237_BASE(parent_vm, parent_emu)
{
	for(int i = 0; i < 4; i++) {
		dma[i].dev = vm->dummy;
	}
#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
#endif
}

I8237::~I8237()
{
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

// note: if SINGLE_MODE_DMA is defined, do_dma() is called in every machine cycle

void I8237::do_dma()
{
	for(int ch = 0; ch < 4; ch++) {
		uint8_t bit = 1 << ch;
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
					write_signals(&dma[ch].outputs_tc, 0xffffffff);
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

#define STATE_VERSION	2

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
	state_fio->FputUint32(addr_mask);
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
	addr_mask = state_fio->FgetUint32();
	return true;
}

