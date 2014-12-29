/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.13 -

	[ memory ]
*/

#include "memory.h"
#include "../../fileio.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 11, eb = (e) >> 11; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x800 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x800 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// init memory
	memset(ram, 0, sizeof(ram));
	memset(ext, 0, sizeof(ext));
	memset(vram, 0, sizeof(vram));
	memset(tvram, 0, sizeof(tvram));
	memset(pcg, 0, sizeof(pcg));
	memset(ipl, 0xff, sizeof(ipl));
	memset(kanji, 0xff, sizeof(kanji));
	memset(dic, 0xff, sizeof(dic));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(dic, sizeof(dic), 1);
		fio->Fclose();
		
		// kanji rom is on even addr
		for(int i = 0; i < 0x40000; i++) {
			kanji[i << 1] = dic[i];
		}
		memset(dic, 0xff, sizeof(dic));
	}
	if(fio->Fopen(emu->bios_path(_T("DICT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(dic, sizeof(dic), 1);
		fio->Fclose();
	}
	delete fio;
}

void MEMORY::reset()
{
	SET_BANK(0x000000, 0x0bffff, ram, ram);
	SET_BANK(0x0c0000, 0x0dffff, vram, vram);
	SET_BANK(0x0e0000, 0x0effff, wdmy, dic);
	SET_BANK(0x0f0000, 0x0f3fff, pcg, ipl);
	SET_BANK(0x0f4000, 0x0f5fff, tvram, ipl + 0x4000);
	SET_BANK(0x0f6000, 0x0f7fff, tvram, ipl + 0x6000);
	SET_BANK(0x0f8000, 0x0fffff, wdmy, ipl + 0x8000);
	SET_BANK(0x100000, 0x1fffff, wdmy, rdmy);
	SET_BANK(0x200000, 0x7fffff, ext, ext);
	SET_BANK(0x800000, 0xfeffff, wdmy, rdmy);
	SET_BANK(0xff0000, 0xffffff, wdmy, ipl);
	
	mem_window = 2 << 18;
	vram_bank = dic_bank = kanji_bank = 0;
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	if((addr & 0xfc0000) == 0x80000) {
		write_dma_data8((addr & 0x3ffff) | mem_window, data);
	} else if((addr & 0xfe0000) == 0xc0000 && !vram_bank) {
		d_crtc->write_data8(addr, data);
	} else {
		write_dma_data8(addr, data);
	}
}

uint32 MEMORY::read_data8(uint32 addr)
{
	if((addr & 0xfc0000) == 0x80000) {
		return read_dma_data8((addr & 0x3ffff) | mem_window);
	} else if((addr & 0xfe0000) == 0xc0000 && !vram_bank) {
		return d_crtc->read_data8(addr);
	} else {
		return read_dma_data8(addr);
	}
}

void MEMORY::write_dma_data8(uint32 addr, uint32 data)
{
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32 MEMORY::read_dma_data8(uint32 addr)
{
	return rbank[addr >> 11][addr & 0x7ff];
}

void MEMORY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0x7fff) {
	case 0x8c:
		// memory window register
		mem_window = (data & 0x3f) << 18;
		break;
	case 0x8d:
		// vram bank
		if(data == 0) {
			// read modify write ???
			
			// temporary implement
			if(kanji_bank & 0x80) {
				SET_BANK(0x0f0000, 0x0f0fff, wdmy, kanji + 0x1000 * (kanji_bank & 0x7f));
			} else {
				SET_BANK(0x0f0000, 0x0f0fff, pcg, pcg);
			}
			SET_BANK(0x0f1000, 0x0f3fff, pcg + 0x1000, pcg + 0x1000);
			SET_BANK(0x0f4000, 0x0f5fff, tvram, tvram);
			SET_BANK(0x0f6000, 0x0f7fff, tvram, tvram);
		} else if(data == 4) {
			SET_BANK(0x0c0000, 0x0dffff, vram, vram);
		} else if(data == 5) {
			SET_BANK(0x0c0000, 0x0dffff, vram + 0x20000, vram + 0x20000);
		} else if(data == 6) {
			SET_BANK(0x0c0000, 0x0dffff, vram + 0x40000, vram + 0x40000);
		} else if(data == 7) {
			SET_BANK(0x0c0000, 0x0dffff, vram + 0x60000, vram + 0x60000);
		}
		vram_bank = data;
		break;
	case 0xce:
		// dictionary rom bank
		SET_BANK(0x0e0000, 0x0effff, wdmy, dic + ((data & 0x18) >> 3) * 0x10000);
		dic_bank = data;
		break;
	case 0x274:
		// kanji rom / pcg bank
		if(data & 0x80) {
			SET_BANK(0x0f0000, 0x0f0fff, wdmy, kanji + 0x1000 * (data & 0x7f));
		} else {
			SET_BANK(0x0f0000, 0x0f0fff, pcg, pcg);
		}
		kanji_bank = data;
		break;
	}
}

uint32 MEMORY::read_io8(uint32 addr)
{
	switch(addr & 0x7fff) {
	case 0x8c:
		return mem_window >> 18;
	case 0x8d:
		return vram_bank;
	case 0xce:
		return dic_bank;
	case 0x274:
		return kanji_bank;
	}
	return 0xff;
}

