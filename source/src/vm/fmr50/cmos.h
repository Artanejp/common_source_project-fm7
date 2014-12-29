/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.05.01 -

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
#ifdef _FMRCARD
	uint8 cmos[4][0x800];
#else
	uint8 cmos[1][0x800];
#endif
	bool modified;
	uint8 bank;
	
public:
	CMOS(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CMOS() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	
	// unique function
	uint8* get_cmos()
	{
		return cmos[0];
	}
};

#endif

