/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[fonts]
*/

#pragma once

#include "../device.h"

namespace FMTOWNS {

class FONT_ROM_20PIX : public DEVICE
{
protected:
	uint8_t font_kanji20[0x40000];
public:
	FONT_ROM_20PIX(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name("Font Roms(20pix)");
	}
	~FONT_ROM_20PIX() {}

	void initialize();
	uint32_t read_data8(uint32_t addr);
};

}

