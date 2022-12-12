/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2022.11.26-

	[ memory bus ]
*/

#include "membus.h"

uint32_t MEMBUS::read_dma_data8(uint32_t addr)
{
	int bank = get_bank(addr);
	
	if(rd_table[bank].device != NULL) {
//		return rd_table[bank].device->read_memory_mapped_io8(addr);
		return 0xff;
	} else {
		return MEMORY::read_data8(addr);
	}
}

void MEMBUS::write_dma_data8(uint32_t addr, uint32_t data)
{
	int bank = get_bank(addr);
	
	if(wr_table[bank].device != NULL) {
//		wr_table[bank].device->write_memory_mapped_io8(addr, data);
	} else {
		MEMORY::write_data8(addr, data);
	}
}
