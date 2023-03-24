/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2022.11.26-

	[ memory bus ]
*/

#ifndef _MEMBUS_H_
#define _MEMBUS_H_

#include "../memory.h"

namespace YIS {
class MEMBUS : public MEMORY
{
private:
	DEVICE *d_cpu;

public:
	MEMBUS(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMBUS() {}

	// common functions
	uint32_t __FASTCALL read_dma_data8(uint32_t addr) override;
	void __FASTCALL write_dma_data8(uint32_t addr, uint32_t data) override;
};

}
#endif
