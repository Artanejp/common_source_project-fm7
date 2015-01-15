/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2014.12.26-

	[ kanji/dictionary rom ]
*/

#ifndef _KANJI_H_
#define _KANJI_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class KANJI : public DEVICE
{
private:
	uint8 kanji[0x20000];
	uint8 dic[0x40000];
	uint32 control_reg;
	uint32 kanji_addr, dic_addr;
	
public:
	KANJI(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~KANJI() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

