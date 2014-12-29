/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.08.14 -

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
	DEVICE *d_rtc;
#ifdef _PC98HA
	uint8 ch;
#endif
	
public:
	CALENDAR(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CALENDAR() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	
	// unique functions
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
};

#endif

