/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

	[ diskette i/f ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class UPD765A;

class FLOPPY : public DEVICE
{
private:
	UPD765A *d_fdc;
	
	uint8_t ctrl_reg;
	
public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	
	// unique function
	void set_context_fdc(UPD765A* device)
	{
		d_fdc = device;
	}
};

#endif

