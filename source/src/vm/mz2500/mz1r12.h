/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2013.03.17-

	[ MZ-1R12 (32KB SRAM) ]
*/

#ifndef _MZ1R12_H_
#define _MZ1R12_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MZ1R12 : public DEVICE
{
private:
	uint8 sram[0x8000];
	bool read_only;
	uint16 address;
	uint32 crc32;
	
public:
	MZ1R12(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MZ1R12() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

