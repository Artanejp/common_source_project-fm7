/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[fonts]
*/

#include "../../fileio.h"
#include "./fontroms.h"

namespace FMTOWNS {

void FONT_ROMS::initialize()
{
	memset(font_kanji16, 0xff, sizeof(font_kanji16));
//	memset(ram, 0x00, sizeof(ram));
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_FNT.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(font_kanji16, sizeof(font_kanji16), 1);
		fio->Fclose();
	}

	delete fio;
}

void FONT_ROMS::reset()
{
//	dma_is_vram = true;
	kanji_code.w = 0;
	kanji_address = 0;
}

uint32_t FONT_ROMS::read_memory_mapped_io8(uint32_t addr)
{
	__LIKELY_IF((addr & 0xfffc0000) == 0xc2100000) { // 0xc2100000 - c213ffff
		return font_kanji16[addr & 0x3ffff];
	}
	__LIKELY_IF((addr & 0xffffc000) == 0x000c8000) {
		if((addr & 0xfffff800) == 0x000ca000) { // 000ca000 - 000ca7ff
			return font_kanji16[0x3d000 + (addr & 0x7ff)];
		} else if((addr & 0xfffff000) == 0x000cb000) { // 000cb000 - 000cbfff
			return font_kanji16[0x3d800 + (addr & 0xfff)];
		}
	}
	return 0xff;
}
// From MAME 0.216
void FONT_ROMS::calc_kanji_offset()
{
	uint8_t kanji_bank = (kanji_code.b.h & 0xf0) >> 4;
	uint32_t low_addr = (uint32_t)(kanji_code.b.l & 0x1f);
	uint32_t mid_addr = (uint32_t)(kanji_code.b.l - 0x20);
	uint32_t high_addr = (uint32_t)(kanji_code.b.h);

	switch(kanji_bank) {
	case 0:
	case 1:
	case 2:
		kanji_address =
			(low_addr << 4) | ((mid_addr & 0x20) << 8) |
			((mid_addr & 0x40) << 6) |
			((high_addr & 0x07) << 9);
//		kanji_address >>= 1;
		break;
	case 3:
	case 4:
	case 5:
	case 6:
		kanji_address =
			(low_addr << 5) +
			((mid_addr & 0x60) << 9) +
			((high_addr & 0x0f) << 10) +
			(((high_addr - 0x30) & 0x70) * 0xc00) + 0x8000;
		kanji_address >>= 1;
		break;
	default:
		kanji_address =
			(low_addr << 4) | ((mid_addr & 0x20) << 8) |
			((mid_addr & 0x40) << 6) |
			((high_addr & 0x07) << 9);
		kanji_address |= (0x38000 >> 1);
//		kanji_address >>= 1;
		break;
	}
}

void FONT_ROMS::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0xff94: // hidden register
		kanji_code.b.h = data;
		calc_kanji_offset();
		break;
	case 0xff95: // hidden register
		kanji_code.b.l = data;
		calc_kanji_offset();
		break;
	case 0xff9e: // Kanji ROW (After UG)
		kanji_address = (kanji_address & 0xfffffff0) | (data & 0x0f);
		break;
	}
}

uint32_t FONT_ROMS::read_io8(uint32_t addr)
{
	uint32_t val = 0x00;
	switch(addr) {
	case 0xff94: // ADDR LOW
		val = 0x80;
		break;
	case 0xff95: // ADDR HIGH
		val = 0xFF;
		break;
	case 0xff96: // LOW
	case 0xff9d: // LOW
		val = font_kanji16[(kanji_address << 1) + 0];
		break;
	case 0xff97: // High
	case 0xff9c: // High
		val = font_kanji16[(kanji_address << 1) + 1];
		kanji_address++;
		break;
	case 0xff9e: // Kanji ROW (After UG)
		val = kanji_address & 0x0f;
		break;
	}
	return val;
}


void FONT_ROMS::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_TOWNS_FONT_KANJI_LOW) { // write CFF15
		kanji_code.b.l = data & 0x7f;
		calc_kanji_offset();
	} else if(ch == SIG_TOWNS_FONT_KANJI_HIGH) { // write CFF14
		kanji_code.b.h = data & 0x7f;
		calc_kanji_offset();
	} /*else 	if(ch == SIG_TOWNS_FONT_DMA_IS_VRAM) {
		dma_is_vram = ((data & mask) != 0);
	} */else if(ch == SIG_TOWNS_FONT_KANJI_ROW) { // write CFF9E
		kanji_address = (kanji_address & 0xfffffff0) | (data & 0x0f);
	}
}

uint32_t FONT_ROMS::read_signal(int ch)
{
	/*if(ch == SIG_TOWNS_FONT_DMA_IS_VRAM) {
		return (dma_is_vram) ? 0xffffffff : 0x00000000;
	} else */
	if(ch == SIG_TOWNS_FONT_KANJI_DATA_HIGH) { // read CFF97
		uint8_t val = font_kanji16[(kanji_address << 1) + 1];
		kanji_address++;
		return val;
	} else if(ch == SIG_TOWNS_FONT_KANJI_DATA_LOW) {  // read CFF96
		uint8_t val = font_kanji16[(kanji_address << 1) + 0];
		return val;
	} else if(ch == SIG_TOWNS_FONT_KANJI_LOW) { // write CFF94
		return kanji_code.b.l;
	} else if(ch == SIG_TOWNS_FONT_KANJI_HIGH) { // write CFF95
		return kanji_code.b.h;
	} else if(ch == SIG_TOWNS_FONT_KANJI_ROW) { // write CFF9E
		return (kanji_address & 0x0f);
	} else if(ch >= SIG_TOWNS_FONT_PEEK_DATA) {
		int offset = ch - SIG_TOWNS_FONT_PEEK_DATA;
		if((offset >= 0) && (offset < 0x40000)) {
			return font_kanji16[offset];
		}
	}
	return 0;
}

#define STATE_VERSION	1

bool FONT_ROMS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
//	state_fio->StateValue(dma_is_vram);
	state_fio->StateValue(kanji_code);
	state_fio->StateValue(kanji_address);

//	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}
}
