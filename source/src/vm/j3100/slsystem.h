/*
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

	[ system ]
*/

#ifndef _SL_SYSTEM_H_
#define _SL_SYSTEM_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class SYSTEM : public DEVICE
{
private:
	int nmi_mask;
	
public:
	SYSTEM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("System I/O"));
	}
	~SYSTEM() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
};

#endif

