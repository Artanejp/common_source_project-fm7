/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ system poty ]
*/

#ifndef _SYSPORT_H_
#define _SYSPORT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class SYSPORT : public DEVICE
{
private:
	DEVICE *d_pit, *d_sio;
	
public:
	SYSPORT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SYSPORT() {}
	
	// common functions
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	
	// unique function
	void set_context_pit(DEVICE* device)
	{
		d_pit = device;
	}
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
};

#endif

