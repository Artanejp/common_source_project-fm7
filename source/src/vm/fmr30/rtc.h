/*
	FUJITSU FMR-30 Emulator 'eFMR-30'

	Author : Takeda.Toshiya
	Date   : 2008.12.30 -

	[ rtc ]
*/

#ifndef _RTC_H_
#define _RTC_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace FMR30 {

class RTC : public DEVICE
{
private:
	DEVICE* d_pic;

	dll_cur_time_t cur_time;
	int register_id;

	uint16_t rtcmr, rtdsr, rtadr, rtobr, rtibr;
	uint8_t regs[40];

	void  read_from_cur_time();
	void  write_to_cur_time();
	void  update_checksum();
public:
	RTC(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("RTC"));
	}
	~RTC() {}

	// common functions
	void initialize() override;
	void release() override;
	
	void  __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t  __FASTCALL read_io8(uint32_t addr) override;
	void  __FASTCALL write_io16(uint32_t addr, uint32_t data) override;
	uint32_t  __FASTCALL read_io16(uint32_t addr) override;
	void  __FASTCALL write_io16w(uint32_t addr, uint32_t data, int *wait) override;
	uint32_t  __FASTCALL read_io16w(uint32_t addr, int *wait) override;
	
	void __FASTCALL event_callback(int event_id, int err) override;
	
	void  update_intr() override;
	
	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique function
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
};

}
#endif
