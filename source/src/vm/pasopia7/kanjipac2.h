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
	uint8_t rom[0x20000];
	uint32_t ptr;
public:
	KANJIPAC2(VM_TEMPLATE* parent_vm, EMU* parent_emu) : PAC2DEV(parent_vm, parent_emu)
	{
		set_device_name(_T("Kanji ROM PAC2"));
	}
	~KANJIPAC2() {}
	
	// common functions
	void initialize(int id);
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif

