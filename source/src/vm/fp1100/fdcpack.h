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
	FDCPACK(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~FDCPACK() {}
	
	// common functions
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	
	// unique functions
	void set_context_fdc(DEVICE *device)
	{
		d_fdc = device;
	}
};

#endif
