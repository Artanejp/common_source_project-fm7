/*
	Systems Formulate BUBCOM80 Emulator 'eBUBCOM80'

	Author : Takeda.Toshiya
	Date   : 2018.05.08-

	[ memory bus ]
*/

#ifndef _MEMBUS_H_
#define _MEMBUS_H_

#include "../memory.h"

class MEMBUS : public MEMORY
{
private:
	
	uint8_t boot[0x800];
	uint8_t basic[0x10000];
	uint8_t ram[0x10000];
	
	pair32_t basic_addr;
	bool ram_selected;
	
	void update_bank();
	
public:
	MEMBUS(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMBUS() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_dma_data8(uint32_t addr, uint32_t data);
	uint32_t read_dma_data8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif
