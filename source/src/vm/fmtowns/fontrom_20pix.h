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
	uint8_t rom[0x80000];
public:
	FONT_ROM_20PIX(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name("Font Roms(20pix)");
	}
	~FONT_ROM_20PIX() {}

	void initialize() override;

	uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr) override;
	uint32_t __FASTCALL read_memory_mapped_io16(uint32_t addr) override;
	uint32_t __FASTCALL read_memory_mapped_io32(uint32_t addr) override;

	uint32_t __FASTCALL read_dma_data8w(uint32_t addr, int* wait) override;
	uint32_t __FASTCALL read_dma_data16w(uint32_t addr, int* wait) override;
	uint32_t __FASTCALL read_dma_data32w(uint32_t addr, int* wait) override;

};

}
