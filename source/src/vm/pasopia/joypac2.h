/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ joystick ]
*/

#ifndef _JOYPAC2_H_
#define _JOYPAC2_H_

#include "../vm.h"
#include "../../emu.h"
#include "pac2dev.h"

class JOYPAC2 : public PAC2DEV
{
private:
	uint32* joy;
public:
	JOYPAC2(VM* parent_vm, EMU* parent_emu) : PAC2DEV(parent_vm, parent_emu) {}
	~JOYPAC2() {}
	
	// common functions
	void initialize(int id);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
};

#endif

