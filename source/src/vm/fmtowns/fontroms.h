/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[fonts]
*/

#pragma once

#include "../device.h"

#define SIG_TOWNS_FONT_DMA_IS_VRAM      2
#define SIG_TOWNS_FONT_KANJI_LOW        4
#define SIG_TOWNS_FONT_KANJI_HIGH       5
#define SIG_TOWNS_FONT_KANJI_DATA_LOW   8
#define SIG_TOWNS_FONT_KANJI_DATA_HIGH  9
#define SIG_TOWNS_FONT_KANJI_ROW        10
#define SIG_TOWNS_FONT_PEEK_DATA        0x10000
namespace FMTOWNS {

class FONT_ROMS : public DEVICE
{
protected:
	uint8_t font_kanji16[0x40000];
//	uint8_t ram[0x1000];

//	bool dma_is_vram;
	pair16_t kanji_code;
	uint32_t kanji_address;

	void calc_kanji_offset();
public:
	FONT_ROMS(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name("Font Roms");
	}
	~FONT_ROMS() {}

	void initialize();
	void reset();
	
	uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr);
	void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data);

	virtual void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_io8(uint32_t addr);

	void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);
	uint32_t __FASTCALL read_signal(int ch);
	
	bool process_state(FILEIO *state_fio, bool loading);

	/* Unique function(s) */
	uint8_t __FASTCALL read_direct_data8(uint32_t addr);
};

}

