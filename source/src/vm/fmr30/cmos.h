/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

	[ cmos ]
*/

#ifndef _CMOS_H_
#define _CMOS_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class CMOS : public DEVICE
{
private:
	uint8 cmos[0x2000];
	bool modified;
	
public:
	CMOS(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CMOS() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	
	// unique function
	uint8* get_cmos()
	{
		return cmos;
	}
};

#endif

