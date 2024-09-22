/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.06.11 -

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_FLOPPY_IRQ	0
#define SIG_FLOPPY_DRQ	1

class UPD765A;

namespace PC98HA {

class FLOPPY : public DEVICE
{
private:
	UPD765A *d_fdc;
	DEVICE *d_dma, *d_pic;
	
	uint8_t ctrlreg, modereg;
	int timer_id;
	
public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		timer_id = -1;
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}
	
	// common functions
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;
	void __FASTCALL event_callback(int event_id, int err) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique functions
	void set_context_fdc(UPD765A* device)
	{
		d_fdc = device;
	}
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

}
#endif

