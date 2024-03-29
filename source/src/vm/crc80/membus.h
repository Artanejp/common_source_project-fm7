/*
	Computer Research CRC-80 Emulator 'eCRC-80'

	Author : Takeda.Toshiya
	Date   : 2022.06.05-

	[ memory bus ]
*/

#ifndef _MEMBUS_H_
#define _MEMBUS_H_

#include "../memory.h"

namespace CRC80 {
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
	
	// common function
	uint32_t __FASTCALL fetch_op(uint32_t addr, int *wait) override;
	
	// unique function
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
};
}

#endif
