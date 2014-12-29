/*
	SEGA GAME GEAR Emulator 'yaGAME GEAR'

	Author : tanam
	Date   : 2013.08.24-

	[ system port ]
*/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class SYSTEM : public DEVICE
{
private:
	DEVICE *d_key;
	bool tenkey;

public:
	SYSTEM(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SYSTEM() {}
	
	// common functions
	uint32 read_io8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	
	// unique functions
	void set_context_key(DEVICE* device)
	{
		d_key = device;
	}
};
#endif
