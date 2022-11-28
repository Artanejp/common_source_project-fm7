/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2016.03.03-

	[ scsi ]
*/

#ifndef _SCSI_H_
#define _SCSI_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SCSI_IRQ	0
#define SIG_SCSI_DRQ	1

class SCSI : public DEVICE
{
private:
	DEVICE *d_dma, *d_pic, *d_host;
	
	uint8_t ctrl_reg;
	bool irq_status;
	
public:
	SCSI(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("SCSI I/F"));
	}
	~SCSI() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_host(DEVICE* device)
	{
		d_host = device;
	}
};

#endif

