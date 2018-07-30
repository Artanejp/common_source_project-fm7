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
	SYSTEM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("System I/O"));
	}
	~SYSTEM() {}
	
	// common functions
	uint32_t read_io8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	
	// unique function
	void set_context_key(DEVICE* device)
	{
		d_key = device;
	}
};
#endif
