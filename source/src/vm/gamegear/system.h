/*
	SEGA GAME GEAR Emulator 'yaGAME GEAR'

	Author : tanam
	Date   : 2013.08.24-

	[ system port ]
*/

#ifndef _GG_SYSTEM_H_
#define _GG_SYSTEM_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace GAMEGEAR {

class SYSTEM : public DEVICE
{
private:
	DEVICE *d_key;
	bool tenkey;

public:
	SYSTEM(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("System I/O"));
	}
	~SYSTEM() {}
	
	// common functions
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	
	// unique function
	void set_context_key(DEVICE* device)
	{
		d_key = device;
	}
};

}
#endif
