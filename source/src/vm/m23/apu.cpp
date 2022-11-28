/*
	SORD M23 Emulator 'Emu23'
	SORD M68 Emulator 'Emu68'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ apu ]
*/

#include "apu.h"

void APU::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xd4:
		d_apu->reset();
		break;
	case 0xdc:
	case 0xdd:
		d_apu->write_io8(addr, data);
		break;
	}
}

uint32_t APU::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xdc:
	case 0xdd:
		return d_apu->read_io8(addr);
	}
	return 0xff;
}
