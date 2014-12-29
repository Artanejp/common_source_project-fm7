/*
	EPSON QC-10 Emulator 'eQC-10'

	Author : Takeda.Toshiya
	Date   : 2008.02.18 -

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FLOPPY : public DEVICE
{
private:
	DEVICE *d_fdc, *d_mem;
	
public:
	FLOPPY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~FLOPPY() {}
	
	// common functions
	void write_io8(uint32 addr, uint32 data);
	
	// unique function
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
};

#endif

