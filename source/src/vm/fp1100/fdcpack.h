/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.18-

	[ ram pack ]
*/

#ifndef _FDCPACK_H_
#define _FDCPACK_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FDCPACK : public DEVICE
{
private:
	// to fdc
	DEVICE *d_fdc;
	
public:
	FDCPACK(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("FDC Pack"));
	}
	~FDCPACK() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	
	// unique function
	void set_context_fdc(DEVICE *device)
	{
		d_fdc = device;
	}
};

#endif
