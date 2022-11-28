/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2011.06.02-

	[ mc6820 ]
*/

#include "mc6820.h"

/* note: ca2/cb2 output signals are not implemented */

void MC6820::reset()
{
	for(int i = 0; i < 2; i++) {
		port[i].ctrl = 0x00;
		port[i].ddr = 0x00;
		port[i].c1 = port[i].c2 = false;
		port[i].first = true;
	}
}

void MC6820::write_io8(uint32_t addr, uint32_t data)
{
	int ch = (addr & 2) >> 1;
	
	switch(addr & 3) {
	case 0:
	case 2:
		if(port[ch].ctrl & 4) {
			if(port[ch].wreg != data || port[ch].first) {
				write_signals(&port[ch].outputs, data);
				port[ch].wreg = data;
				port[ch].first = false;
			}
		} else {
			port[ch].ddr = data;
		}
		break;
	case 1:
	case 3:
		if(data & 0x20) {
			port[ch].ctrl &= ~0x40;
		}
		port[ch].ctrl = (port[ch].ctrl & 0xc0) | (data & 0x3f);
		break;
	}
}

uint32_t MC6820::read_io8(uint32_t addr)
{
	int ch = (addr & 2) >> 1;
	
	switch(addr & 3) {
	case 0:
	case 2:
		if(port[ch].ctrl & 4) {
			write_signals(&port[ch].outputs_irq, 0);
			port[ch].ctrl &= ~0xc0;
			return (port[ch].rreg & ~port[ch].ddr) | (port[ch].wreg & port[ch].ddr);
		} else {
			return port[ch].ddr;
		}
	case 1:
	case 3:
		return port[ch].ctrl;
	}
	return 0xff;
}

void MC6820::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool signal = ((data & mask) != 0);
	int ch = id & 1;
	
	switch(id) {
	case SIG_MC6820_PORT_A:
	case SIG_MC6820_PORT_B:
		port[ch].rreg = (port[ch].rreg & ~mask) | (data & mask);
		break;
	case SIG_MC6820_C1_A:
	case SIG_MC6820_C1_B:
		if(((port[ch].ctrl & 2) && !port[ch].c1 && signal) || (!(port[ch].ctrl & 2) && port[ch].c1 && !signal)) {
			if(port[ch].ctrl & 1) {
				write_signals(&port[ch].outputs_irq, 0xffffffff);
			}
			port[ch].ctrl |= 0x80;
		}
		port[ch].c1 = signal;
		break;
	case SIG_MC6820_C2_A:
	case SIG_MC6820_C2_B:
		if(port[ch].ctrl & 0x20) {
			// c2 is output
			break;
		}
		if(((port[ch].ctrl & 0x10) && !port[ch].c2 && signal) || (!(port[ch].ctrl & 0x10) && port[ch].c2 && !signal)) {
			if(port[ch].ctrl & 8) {
				write_signals(&port[ch].outputs_irq, 0xffffffff);
			}
			port[ch].ctrl |= 0x40;
		}
		port[ch].c2 = signal;
		break;
	}
}

#define STATE_VERSION	1

bool MC6820::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(int i = 0; i < 2; i++) {
		state_fio->StateValue(port[i].wreg);
		state_fio->StateValue(port[i].rreg);
		state_fio->StateValue(port[i].ctrl);
		state_fio->StateValue(port[i].ddr);
		state_fio->StateValue(port[i].c1);
		state_fio->StateValue(port[i].c2);
		state_fio->StateValue(port[i].first);
	}
	return true;
}

