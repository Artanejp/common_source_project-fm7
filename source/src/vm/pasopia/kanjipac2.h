/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ kanji rom pac 2 ]
*/

#ifndef _KANJIPAC2_H_
#define _KANJIPAC2_H_

#include "../vm.h"
#include "../../emu.h"
#include "pac2dev.h"

class KANJIPAC2 : public PAC2DEV
{
private:
	uint8 rom[0x20000];
	uint32 ptr;
public:
	KANJIPAC2(VM* parent_vm, EMU* parent_emu) : PAC2DEV(parent_vm, parent_emu) {}
	~KANJIPAC2() {}
	
	// common functions
	void initialize(int id);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

