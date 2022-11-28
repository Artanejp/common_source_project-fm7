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
	uint8_t kanji[0x20000];
	uint8_t dic[0x4000];
	
	uint16_t address;
	bool select_kanji;
	
public:
	MZ1R13(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("MZ-1R13 (Kanji ROM)"));
	}
	~MZ1R13() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif

