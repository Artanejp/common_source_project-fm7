/*
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.03 -

	[ timer ]
*/

#ifndef _TIMER_H_
#define _TIMER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class TIMER : public DEVICE
{
private:
	DEVICE* d_pit;
	
public:
	TIMER(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~TIMER() {}
	
	// common functions
	void write_io8(uint32 addr, uint32 data);
	
	// unique functions
	void set_context_pit(DEVICE* device)
	{
		d_pit = device;
	}
};

#endif

