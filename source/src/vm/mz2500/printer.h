/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2015.12.28-

	[ printer ]
*/

#ifndef _PRINTER_H_
#define _PRINTER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace MZ2500 {

class PRINTER : public DEVICE
{
private:
	DEVICE* d_prn;
	
public:
	PRINTER(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Printer I/F"));
	}
	~PRINTER() {}
	
	// common functions
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	
	// unique function
	void set_context_prn(DEVICE* device)
	{
		d_prn = device;
	}
};
}
#endif

