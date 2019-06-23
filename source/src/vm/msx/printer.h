/*
	Common Source Code Project
	MSX Series (experimental)

	Origin : src/vm/mz2500/printer.cpp

	[ printer ]
*/

#ifndef _PRINTER_H_
#define _PRINTER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace MSX {
	
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
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	
	// unique function
	void set_context_prn(DEVICE* device)
	{
		d_prn = device;
	}
};

}
#endif

