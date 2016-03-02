/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'

	Author : Takeda.Toshiya
	Date   : 2011.08.28-

	[ system ]
*/

#include "system.h"
#include "../i8253.h"
#include "../pcm1bit.h"

void SYSTEM::initialize()
{
	status = 0x0f;
}

void SYSTEM::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0x61:
		if((status & 1) != (data & 1)) {
			d_pit->write_signal(SIG_I8253_GATE_2, data, 1);
		}
		if((status & 2) != (data & 2)) {
			d_pcm->write_signal(SIG_PCM1BIT_ON, data, 2);
		}
		status = (status & 0xf0) | (data & 0x0f);
		break;
	}
}

uint32_t SYSTEM::read_io8(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0x61:
		return status;
	}
	return 0xff;
}

void SYSTEM::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_SYSTEM_TC2O) {
		if(data & mask) {
			status |= 0x20;
		} else {
			status &= ~0x20;
		}
	}
}

