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
	const uint32_t* joy;
public:
	JOYPAC2(VM_TEMPLATE* parent_vm, EMU* parent_emu) : PAC2DEV(parent_vm, parent_emu)
	{
		set_device_name(_T("Joystick PAC2"));
	}
	~JOYPAC2() {}
	
	// common functions
	void initialize(int id);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
};

#endif

