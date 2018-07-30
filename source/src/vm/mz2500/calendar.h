/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.01 -

	[ calendar ]
*/

#ifndef _CALENDAR_H_
#define _CALENDAR_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class CALENDAR : public DEVICE
{
private:
	DEVICE* d_rtc;
	
public:
	CALENDAR(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("RTC I/F"));
	}
	~CALENDAR() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	
	// unique function
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
};

#endif

