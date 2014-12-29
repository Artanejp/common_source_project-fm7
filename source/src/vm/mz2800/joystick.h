/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ joystick ]
*/

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class JOYSTICK : public DEVICE
{
private:
	uint32 mode;
	int full_auto;
	uint32* joy_stat;
public:
	JOYSTICK(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~JOYSTICK() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_frame();
};

#endif
