/*
	SHARP MZ-80A Emulator 'EmuZ-80A'

	Author : Takeda.Toshiya
	Date   : 2006.12.04 -

	Modify : Hideki Suga
	Date   : 2014.12.30 -

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_FLOPPY_REVERSE	0

class FLOPPY : public DEVICE
{
private:
	DEVICE* d_fdc;
	
public:
	FLOPPY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~FLOPPY() {}
	
	// common function
	void write_io8(uint32 addr, uint32 data);
	
	// unique function
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
};

#endif

