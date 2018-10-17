/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

	[ system ]
*/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class SYSTEM : public DEVICE
{
private:
	uint8_t arr;
	uint8_t nmistat, nmimask;
	
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
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif

