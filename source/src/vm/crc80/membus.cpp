/*
	Computer Research CRC-80 Emulator 'eCRC-80'

	Author : Takeda.Toshiya
	Date   : 2022.06.05-

	[ memory bus ]
*/

#include "membus.h"

uint32_t MEMBUS::fetch_op(uint32_t addr, int *wait)
{
	if((config.dipswitch & 1) && (addr & 0x8c00)) {
		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
	}
	*wait = 0;
	return MEMORY::read_data8(addr);
}
