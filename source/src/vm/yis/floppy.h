/*
	YAMAHA YIS Emulator 'eYIS'

	Author : Takeda.Toshiya
	Date   : 2017.04.13-

	[ floppy i/f ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FLOPPY : public DEVICE
{
private:
	DEVICE* d_fdc;
	
public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}
	
	// common function
	void write_io8(uint32_t addr, uint32_t data);
	
	// unique function
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
};

#endif

