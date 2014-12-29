/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ reset ]
*/

#ifndef _RESET_H_
#define _RESET_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_RESET_CONTROL	0

class RESET : public DEVICE
{
private:
	uint8 prev;
	
public:
	RESET(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~RESET() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32 data, uint32 mask);
};

#endif

