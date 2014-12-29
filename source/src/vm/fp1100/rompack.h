/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.18-

	[ rom pack ]
*/

#ifndef _ROMPACK_H_
#define _ROMPACK_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class ROMPACK : public DEVICE
{
private:
	uint8 rom[0x8000];
	
public:
	ROMPACK(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~ROMPACK() {}
	
	// common functions
	void initialize();
	uint32 read_io8(uint32 addr);
};

#endif
