/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MB8877;

class FLOPPY : public DEVICE
{
private:
	MB8877* d_fdc;
	
public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	
	// unique function
	void set_context_fdc(MB8877* device)
	{
		d_fdc = device;
	}
};

#endif

