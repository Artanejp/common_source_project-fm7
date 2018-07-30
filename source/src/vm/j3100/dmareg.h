/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.28-

	[ dma bank register ]
*/

#ifndef _DMAREG_H_
#define _DMAREG_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class DMAREG : public DEVICE
{
private:
	DEVICE *d_dma;
#ifndef TYPE_SL
	DEVICE *d_dma2;
#endif
	
public:
	DMAREG(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("DMA Register"));
	}
	~DMAREG() {}
	
	// common function
	void write_io8(uint32_t addr, uint32_t data);
	
	// unique functions
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
#ifndef TYPE_SL
	void set_context_dma2(DEVICE* device)
	{
		d_dma2 = device;
	}
#endif
};

#endif

