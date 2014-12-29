/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.05.02 -

	[ scsi ]
*/

#ifndef _SCSI_H_
#define _SCSI_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class SCSI : public DEVICE
{
private:
	DEVICE *d_dma, *d_pic;
	
	int phase;
	uint8 ctrlreg, datareg, statreg;
	
public:
	SCSI(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SCSI() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_dma_io8(uint32 addr, uint32 data);
	uint32 read_dma_io8(uint32 addr);
	
	// unique function
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

#endif

