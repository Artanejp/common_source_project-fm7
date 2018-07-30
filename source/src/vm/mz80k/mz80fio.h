/*
	SHARP MZ-80K/C Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2015.09.04-

	[ MZ-80FIO ]
*/

#ifndef _MZ80FIO_H_
#define _MZ80FIO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MZ80FIO : public DEVICE
{
private:
	DEVICE* d_fdc;
	
public:
	MZ80FIO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("MZ-80FIO (FDC I/F)"));
	}
	~MZ80FIO() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	
	// unique function
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
};

#endif

