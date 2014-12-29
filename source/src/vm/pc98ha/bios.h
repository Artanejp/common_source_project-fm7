/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2010.05.12 -

	[ bios ]
*/

#ifndef _BIOS_H_
#define _BIOS_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class UPD765A;

class BIOS : public DEVICE
{
private:
	UPD765A *d_fdc;
	
public:
	BIOS(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~BIOS() {}
	
	// common functions
	bool bios_int(int intnum, uint16 regs[], uint16 sregs[], int32* ZeroFlag, int32* CarryFlag);
	
	// unique functions
	void set_context_fdc(UPD765A* device)
	{
		d_fdc = device;
	}
};

#endif

