/*
	Systems Formulate BUBCOM80 Emulator 'eBUBCOM80'

	Author : Takeda.Toshiya
	Date   : 2018.05.11-

	[ rtc ]
*/

#ifndef _RTC_H_
#define _RTC_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

namespace BUBCOM80 {
class RTC : public DEVICE
{
private:
	dll_cur_time_t cur_time;
	dll_cur_time_t tmp_time;
	uint8_t ctrl;
	
public:
	RTC(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("RTC"));
	}
	~RTC() {}
	
	// common functions
	void initialize() override;
	void reset() override;
	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL event_callback(int event_id, int err) override;
	bool process_state(FILEIO* state_fio, bool loading) override;
};
}

#endif

