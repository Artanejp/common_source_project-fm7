/*
	SORD M23 Emulator 'Emu23'
	SORD M68 Emulator 'Emu68'

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
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;

	// unique function
	void set_context_apu(DEVICE* device)
	{
		d_apu = device;
	}
};
}
#endif
