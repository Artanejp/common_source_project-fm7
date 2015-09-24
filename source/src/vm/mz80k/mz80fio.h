/*
	SHARP MZ-80K/C Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2015.09.04-

	[ MZ-80FIO ]
*/

#ifndef _MZ80FIO_H_
#define _MZ80FIO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MZ80FIO : public DEVICE
{
private:
	DEVICE* d_fdc;
	
public:
	MZ80FIO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MZ80FIO() {}
	
	// common functions
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	
	// unique function
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
};

#endif

