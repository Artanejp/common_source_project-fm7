/*
	EPS TRN Junior Emulator 'eTRNJunior'

	Author : Takeda.Toshiya
	Date   : 2022.07.02-

	[ memory bus ]
*/

#ifndef _MEMBUS_H_
#define _MEMBUS_H_

#include "../memory.h"

namespace TRNJR {
class MEMBUS : public MEMORY
{
protected:
	DEVICE *d_cpudev;
	
public:
	MEMBUS(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMBUS() {}
	
	// common function
	uint32_t __FASTCALL fetch_op(uint32_t addr, int *wait) ovrride;
	
	// unique function
	void set_context_cpudev(DEVICE* device)
	{
		d_cpudev = device;
	}
};

}

#endif
