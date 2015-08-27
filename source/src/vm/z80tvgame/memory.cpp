/*
	Homebrew Z80 TV GAME SYSTEM Emulator 'eZ80TVGAME'

	Author : Takeda.Toshiya
	Date   : 2015.04.28-

	[ memory ]
*/

// http://w01.tp1.jp/~a571632211/z80tvgame/index.html

#include "memory.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		} \
		if((r) == rdmy) { \
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
}

void MEMORY::reset()
{
	memset(ram, 0, sizeof(ram));
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	wbank[addr >> 12][addr & 0xfff] = data;
}

uint32 MEMORY::read_data8(uint32 addr)
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
	scrntype col_w = RGB_COLOR(255, 255, 255);
	scrntype col_b = 0;
	
	for(int y = 0, offset = 0x403c; y < 210; y++, offset += 30) {
		scrntype* dest = emu->screen_buffer(y) - 30;
		// (30 * 8 - 176) / 2 = 32
		for(int x = 32; x < 240 - 32; x++) {
			uint8 val = ram[offset + (x >> 3)];
			uint8 bit = 1 << (x & 7);
			dest[x] = (val & bit) ? col_w : col_b;
		}
	}
}

#define STATE_VERSION	1

void MEMORY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
	state_fio->FputBool(inserted);
}

bool MEMORY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(ram, sizeof(ram), 1);
	inserted = state_fio->FgetBool();
	return true;
}

