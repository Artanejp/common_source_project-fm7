/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2016.03.03-

	[ scsi ]
*/

#ifndef _SCSI_H_
#define _SCSI_H_

#include "../device.h"

#define SIG_SCSI_IRQ			0
#define SIG_SCSI_DRQ			1
#define SIG_SCSI_16BIT_TRANSFER	2
#define SIG_SCSI_EOT			3

namespace FMTOWNS {
class SCSI : public DEVICE
{
private:
	DEVICE *d_dma, *d_pic, *d_host;

	uint8_t ctrl_reg;
	bool irq_status;
	bool irq_status_bak;
	bool exirq_status;
	bool ex_int_enable;
	bool dma_enabled;
	bool transfer_16bit;

	uint16_t machine_id;
	uint8_t cpu_id;

public:
	SCSI(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu) {
		transfer_16bit = false;
		set_device_name(_T("FM Towns SCSI"));
	}
	~SCSI() {}

	// common functions
	void reset();
	void __FASTCALL write_io8w(uint32_t addr, uint32_t data, int *wait);
	uint32_t __FASTCALL read_io8w(uint32_t addr, int *wait);
	void __FASTCALL write_io16w(uint32_t addr, uint32_t data, int *wait);
	uint32_t __FASTCALL read_io16w(uint32_t addr, int *wait);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);

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
	void set_machine_id(uint16_t val)
	{
		machine_id = val & 0xfff8;
	}
	void set_cpu_id(uint16_t val)
	{
		cpu_id = val & 0x07;
	}
	bool process_state(FILEIO* state_fio, bool loading);
};
}

#endif
