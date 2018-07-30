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
	BIOS(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Pseudo BIOS"));
	}
	~BIOS() {}
	
	// common function
	bool bios_int_i86(int intnum, uint16_t regs[], uint16_t sregs[], int32_t* ZeroFlag, int32_t* CarryFlag);
	
	// unique function
	void set_context_fdc(UPD765A* device)
	{
		d_fdc = device;
	}
};

#endif

