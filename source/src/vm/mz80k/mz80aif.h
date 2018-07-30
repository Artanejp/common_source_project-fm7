/*
	SHARP MZ-80A Emulator 'EmuZ-80A'

	Author : Takeda.Toshiya
	Date   : 2006.12.04 -

	Modify : Hideki Suga
	Date   : 2014.12.30 -

	[ MZ-80AIF ]
*/

#ifndef _MZ80AIF_H_
#define _MZ80AIF_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MZ80AIF : public DEVICE
{
private:
	DEVICE* d_fdc;
	
public:
	MZ80AIF(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("MZ-80AIF (FDC I/F)"));
	}
	~MZ80AIF() {}
	
	// common function
	void write_io8(uint32_t addr, uint32_t data);
	
	// unique function
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
};

#endif

