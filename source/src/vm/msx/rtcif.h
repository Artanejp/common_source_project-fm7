/*
	ASCII MSX2 Emulator 'yaMSX2'

	Author : umaiboux
	Date   : 2014.12.xx-

	modified by Takeda.Toshiya

	[ rtc i/f ]
*/

#ifndef _RTCIF_H_
#define _RTCIF_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class RTCIF : public DEVICE
{
private:
	DEVICE *d_rtc;
	uint8_t adrs;
	
public:
	RTCIF(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("RTC I/F"));
	}
	~RTCIF() {}
	
	// common functions
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
};

#endif

