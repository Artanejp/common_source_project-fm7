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

class RTC : public DEVICE
{
private:
	dll_cur_time_t cur_time;
	dll_cur_time_t tmp_time;
	uint8_t ctrl;
	
public:
	RTC(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("RTC"));
	}
	~RTC() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif

