/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ pac slot 2 base class ]
*/

#ifndef _PAC2DEV_H_
#define _PAC2DEV_H_

#include "../vm.h"
#include "../../emu.h"

class PAC2DEV
{
protected:
	VM* vm;
	EMU* emu;
public:
	PAC2DEV(VM* parent_vm, EMU* parent_emu) : vm(parent_vm), emu(parent_emu) {}
	~PAC2DEV(void) {}
	
	virtual void initialize(int id) {}
	virtual void release() {}
	virtual void reset() {}
	virtual void write_io8(uint32 addr, uint32 data) {}
	virtual uint32 read_io8(uint32 addr) { return 0xff; }
};

#endif

