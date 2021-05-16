/*
	CANON BX-1 Emulator 'eBX-1'

	Author : Takeda.Toshiya
	Date   : 2021.02.14-

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu_template.h"
#include "../device.h"

class MC6843;

namespace BX1 {
class FLOPPY : public DEVICE
{
private:
	MC6843* d_fdc;
	
public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}
	
	// common functions
	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);
	
	// unique function
	void set_context_fdc(MC6843* device)
	{
		d_fdc = device;
	}
};
}

#endif

