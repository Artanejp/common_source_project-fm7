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
	uint8 adrs;
	
public:
	RTCIF(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~RTCIF() {}
	
	// common functions
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
};

#endif

