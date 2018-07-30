/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2007.02.11 -

	[ MZ-1E26 (Voice Communication I/F) ]
*/

#ifndef _MZ1E26_H_
#define _MZ1E26_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MZ1E26 : public DEVICE
{
private:
//	uint8_t prev_data;
	
public:
	MZ1E26(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("MZ-1E26 (Voice Communication I/F)"));
	}
	~MZ1E26() {}
	
	// common functions
	void initialize();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
};

#endif

