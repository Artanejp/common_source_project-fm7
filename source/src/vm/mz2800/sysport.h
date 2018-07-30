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
	SYSPORT(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("System I/O"));
	}
	~SYSPORT() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	
	// unique functions
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

