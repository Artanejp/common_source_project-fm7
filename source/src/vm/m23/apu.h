/*
	SORD M23 Emulator 'Emu23'

	Author : Takeda.Toshiya
	Date   : 2022.05.21-

	[ apu control ]
*/

#ifndef _APU_H_
#define _APU_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace M23 {
class APU : public DEVICE
{
private:
	DEVICE *d_apu;
	
public:
	APU(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("APU"));
	}
	~APU() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	
	// unique function
	void set_context_apu(DEVICE* device)
	{
		d_apu = device;
	}
};
}
#endif
