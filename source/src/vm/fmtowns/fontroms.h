/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[fonts]
*/

#pragma once

#include "../vm.h"
#include "../device.h"

namespace FMTOWNS {

class FONT_ROMS : public DEVICE
{
protected:
	int wait_val;
	uint8_t font_kanji16[0x40000];
#if defined(HAVE_20PIXS_FONT)	
	uint8_t font_kanji20[0x80000];
#endif
public:
	FONT_ROMS(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name("Font Roms");
	}
	~FONT_ROMS() {}

	void initialize();
	
	uint32_t read_data8(uint32_t addr);
	uint32_t read_data8w(uint32_t addr, int* wait);
	void write_data8w(uint32_t addr, uint32_t data, int* wait);
	void write_signal(int ch, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
};

}

