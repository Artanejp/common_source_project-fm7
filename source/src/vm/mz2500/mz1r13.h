/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.01 -

	[ MZ-1R13 (Kanji ROM) ]
*/

#ifndef _MZ1R13_H_
#define _MZ1R13_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MZ1R13 : public DEVICE
{
private:
	uint8 kanji[0x20000];
	uint8 dic[0x4000];
	
	uint16 address;
	bool select_kanji;
	
public:
	MZ1R13(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MZ1R13() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

