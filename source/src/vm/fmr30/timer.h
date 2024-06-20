/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.31 -

	[ timer ]
*/

#ifndef _TIMER_H_
#define _TIMER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_TIMER_CH0	0
#define SIG_TIMER_CH1	1

namespace FMR30 {
	
class TIMER : public DEVICE
{
private:
	DEVICE *d_pic;
	
	uint8_t ctrl, status;
public:
	TIMER(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Timer I/F"));
	}
	~TIMER() {}
	
	// common functions
	void initialize() override;
	
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask) override;

	void update_intr() override;
	
	bool process_state(FILEIO* state_fio, bool loading) override;
	
	// unique function
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

}
#endif

