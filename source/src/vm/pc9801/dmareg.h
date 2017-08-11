/*
	NEC PC-9801VX Emulator 'ePC-9801VX'

	Author : Takeda.Toshiya
	Date   : 2017.06.25-

	[ dma regs ]
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
	
public:
	DMAREG(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("DMA I/O"));
	}
	~DMAREG() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data);
//	void save_state(FILEIO* state_fio);
//	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
};

#endif

