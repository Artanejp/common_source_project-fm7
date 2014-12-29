/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.18-

	[ ram pack ]
*/

#ifndef _RAMPACK_H_
#define _RAMPACK_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class RAMPACK : public DEVICE
{
private:
	uint8 ram[0x4000];
	bool modified;
	
public:
	RAMPACK(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~RAMPACK() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	
	// unique variable
	int index;
};

#endif
