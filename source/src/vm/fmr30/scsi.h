/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2016.03.04-

	[ scsi ]
*/

#ifndef _SCSI_H_
#define _SCSI_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SCSI_IRQ	0
#define SIG_SCSI_DRQ	1
#define SIG_SCSI_TC	2

namespace FMR30 {
	
class SCSI : public DEVICE
{
private:
	DEVICE *d_dma, *d_pic, *d_host;
	
	uint8_t ctrl_reg, intm_reg;
	bool phase_status, eop_status;
	
public:
	SCSI(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("SCSI I/F"));
	}
	~SCSI() {}
	
	// common functions
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
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

}
#endif

