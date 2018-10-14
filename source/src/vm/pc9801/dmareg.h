/*
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'

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
	DMAREG(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("DMA I/O"));
	}
	~DMAREG() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
//	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
};

#endif

