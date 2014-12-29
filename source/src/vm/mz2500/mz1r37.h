/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.01 -

	[ MZ-1R37 (640KB EMM) ]
*/

#ifndef _MZ1R37_H_
#define _MZ1R37_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MZ1R37 : public DEVICE
{
private:
	uint8* buffer;
	uint32 address;
public:
	MZ1R37(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MZ1R37() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

