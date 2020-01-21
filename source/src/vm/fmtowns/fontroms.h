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
	uint8_t font_kanji16[0x40000];
public:
	FONT_ROMS(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name("Font Roms");
	}
	~FONT_ROMS() {}

	void initialize();
	uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr);
};

}

