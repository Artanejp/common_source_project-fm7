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

class FLOPPY : public DEVICE
{
private:
	DEVICE* d_fdc;
	
public:
	FLOPPY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~FLOPPY() {}
	
	// common functions
	void write_io8(uint32 addr, uint32 data);
	
	// unique functions
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
};

#endif

