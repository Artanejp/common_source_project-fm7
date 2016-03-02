/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2007.02.11 -

	[ MZ-1E26 (Voice Communication I/F) ]
*/

#include "mz1e26.h"

void MZ1E26::initialize()
{
//	prev_data = 0;
}

void MZ1E26::write_data8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xca:
//		if((prev_data & 0x10) && !(data & 0x10)) {
			// power off
//		}
//		prev_data = data;
		break;
	}
}

uint32_t MZ1E26::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xca:
		return 0x30;
	}
	return 0xff;
}

