/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2017.04.13-

	[ memory mapper ]
*/

#ifndef _MAPPER_H_
#define _MAPPER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MEMORY;

class MAPPER : public DEVICE
{
private:
	MEMORY *d_memory;
	
	uint8_t ram[0x20000];
	uint8_t mapper_reg;
	uint8_t bank_reg[16];
	uint8_t cur_bank[15];
	
	void update_bank(int num);
	
public:
	MAPPER(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Mapper"));
	}
	~MAPPER() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_memory(MEMORY* device)
	{
		d_memory = device;
	}
};

#endif
