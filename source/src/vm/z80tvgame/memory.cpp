/*
	Homebrew Z80 TV GAME SYSTEM Emulator 'eZ80TVGAME'

	Author : Takeda.Toshiya
	Date   : 2015.04.28-

	[ memory ]
*/

// http://w01.tp1.jp/~a571632211/z80tvgame/index.html

#include "memory.h"

namespace Z80TVGAME {

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if(((uintptr_t)w) == ((uintptr_t)wdmy)) {	\
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		} \
		if(((uintptr_t)r) == ((uintptr_t)rdmy)) {	\
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x1000 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// set memory map
	SET_BANK(0x0000, 0x7fff, wdmy, rom);
	SET_BANK(0x8000, 0xdfff, ram, ram);
	SET_BANK(0xe000, 0xffff, wdmy, rdmy);
	
	inserted = false;

	PrepareReverseBitTransTableScrnType(&pixel_trans_table, RGB_COLOR(255, 255, 255), RGB_COLOR(0,0,0) );
}

void MEMORY::reset()
{
	memset(ram, 0, sizeof(ram));
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	return rbank[addr >> 12][addr & 0xfff];
}

void MEMORY::open_cart(const _TCHAR* file_path)
{
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		memset(rom, 0xff, sizeof(rom));
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
		inserted = true;
	}
	delete fio;
}

void MEMORY::close_cart()
{
	memset(rom, 0xff, sizeof(rom));
	inserted = false;
}

void MEMORY::draw_screen()
{
	// draw screen
	emu->set_vm_screen_lines(210);
	
	scrntype_t col_w = RGB_COLOR(255, 255, 255);
	scrntype_t col_b = RGB_COLOR(0, 0, 0);
	
	emu->set_vm_screen_lines(210);
	for(int y = 0, offset = 0x403c; y < 210; y++, offset += 30) {
		scrntype_t* dest = emu->get_screen_buffer(y) - 30;
		// (30 * 8 - 176) / 2 = 32
#if 0		
		for(int x = 32; x < 240 - 32; x++) {
			uint8_t val = ram[offset + (x >> 3)];
			uint8_t bit = 1 << (x & 7);
			dest[x] = (val & bit) ? col_w : col_b;
		}
#else
		__DECL_ALIGNED(32) scrntype_vec8_t d;
		for(int xx = 32; xx < (240 - 32); xx += 8) {
			uint8_t val = ram[offset + (xx >> 3)];
			d = ConvertByteToPackedPixel_PixelTbl(val, &pixel_trans_table);
			for(int i = 0; i < 8; i++) {
				dest[xx + i] = d.w[i];
			}
		}
#endif
	}
}

#define STATE_VERSION	1

bool MEMORY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	state_fio->StateValue(inserted);
	return true;
}

}
