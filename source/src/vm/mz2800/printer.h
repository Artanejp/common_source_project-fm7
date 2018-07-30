/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2016.01.05-

	[ printer ]
*/

#ifndef _PRINTER_H_
#define _PRINTER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class PRINTER : public DEVICE
{
private:
	DEVICE* d_prn;
	
public:
	PRINTER(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Printer I/F"));
	}
	~PRINTER() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	
	// unique function
	void set_context_prn(DEVICE* device)
	{
		d_prn = device;
	}
};

#endif

