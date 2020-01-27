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
	memset(ram, 0x00, sizeof(ram));
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_FNT.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(font_kanji16, sizeof(font_kanji16), 1);
		fio->Fclose();
	}

	delete fio;
}

void FONT_ROMS::reset()
{
	ankcg_enabled = false;
	dma_is_vram = true;
	kanji_code.w = 0;
	kanji_address = 0;
}
	
uint32_t FONT_ROMS::read_memory_mapped_io8(uint32_t addr)
{
	if((addr >= 0xc2100000) && (addr < 0xc2140000)) {
		return (uint32_t)(font_kanji16[addr & 0x3ffff]);
	} else if((addr >= 0x000ca000) && (addr < 0x000ca800)) {
		return (uint32_t)(font_kanji16[0x1e800 + (addr & 0x7ff)]);
	} else if((addr >= 0x000cb000) && (addr < 0x000cc000)) {
		if(ankcg_enabled) {
			return (uint32_t)(font_kanji16[0x1d800 + (addr & 0x7ff)]);
		} else {
			return (uint32_t)ram[addr & 0x0fff];
		}
	}
	return 0xff;
}

void FONT_ROMS::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	if((addr >= 0x000cb000) && (addr < 0x000cc000)) {
		ram[addr & 0x0fff] = data;
	}
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
		kanji_address |= 0x38000;
		break;
	}
}		
	
	
void FONT_ROMS::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_TOWNS_FONT_ANKCG) {  // write CFF19
		ankcg_enabled = ((data & mask) != 0);
		out_debug_log(_T("ANKCG %s"), (ankcg_enabled) ? _T("ON") : _T("OFF"));
	} else if(ch == SIG_TOWNS_FONT_KANJI_LOW) { // write CFF15
		kanji_code.b.l = data & 0x7f;
		calc_kanji_offset();
	} else if(ch == SIG_TOWNS_FONT_KANJI_HIGH) { // write CFF14
		kanji_code.b.h = data & 0x7f;
		calc_kanji_offset();
	} else 	if(ch == SIG_TOWNS_FONT_DMA_IS_VRAM) { 
		dma_is_vram = ((data & mask) != 0);
	}
}

uint32_t FONT_ROMS::read_signal(int ch)
{
	if(ch == SIG_TOWNS_FONT_ANKCG) {  // read CFF99
		return (ankcg_enabled) ? 0xffffffff : 0x00000000;
	} else if(ch == SIG_TOWNS_FONT_DMA_IS_VRAM) { 
		return (dma_is_vram) ? 0xffffffff : 0x00000000;
	} else if(ch == SIG_TOWNS_FONT_KANJI_DATA_HIGH) { // read CFF97
		uint8_t val = font_kanji16[(kanji_address << 1) + 1];
		kanji_address++;
		return val;
	} else if(ch == SIG_TOWNS_FONT_KANJI_DATA_LOW) {  // read CFF96
		uint8_t val = font_kanji16[(kanji_address << 1) + 0];
		return val;
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
	state_fio->StateValue(dma_is_vram);
	state_fio->StateValue(ankcg_enabled);
	state_fio->StateValue(kanji_code);
	state_fio->StateValue(kanji_address);
	
	state_fio->StateArray(ram, sizeof(ram), 1);
	return true;
}
}
