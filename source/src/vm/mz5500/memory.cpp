/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ memory ]
*/

#include "memory.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 14, eb = (e) >> 14; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x4000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x4000 * (i - sb); \
		} \
	} \
}

void MEMORY::initialize()
{
	// init memory
	memset(ram, 0, sizeof(ram));
	memset(vram, 0, sizeof(vram));
	memset(ipl, 0xff, sizeof(ipl));
	memset(kanji, 0xff, sizeof(kanji));
	memset(dic, 0xff, sizeof(dic));
#ifdef _MZ6550
	memset(dic2, 0xff, sizeof(dic2));
#endif
#if defined(_MZ6500) || defined(_MZ6550)
	memset(mz1r32, 0, sizeof(mz1r32));
#endif
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, sizeof(kanji), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("DICT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(dic, sizeof(dic), 1);
		fio->Fclose();
	}
#ifdef _MZ6550
	if(fio->Fopen(create_local_path(_T("DICT2.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(dic2, sizeof(dic2), 1);
		fio->Fclose();
	}
#endif
	delete fio;
	
	// set memory bank
#if defined(_MZ6500) || defined(_MZ6550)
	SET_BANK(0x00000, 0x9ffff, ram, ram);
#else
	SET_BANK(0x00000, 0x7ffff, ram, ram);
	SET_BANK(0x80000, 0x9ffff, wdmy, rdmy);	// aux
#endif
	SET_BANK(0xa0000, 0xbffff, wdmy, kanji);
	SET_BANK(0xc0000, 0xeffff, vram, vram);
#ifdef _MZ6550
	SET_BANK(0xf0000, 0xf7fff, wdmy, rdmy);	// aux
	SET_BANK(0xf8000, 0xfffff, wdmy, ipl);
#else
	SET_BANK(0xf0000, 0xfbfff, wdmy, rdmy);	// aux
	SET_BANK(0xfc000, 0xfffff, wdmy, ipl);
#endif
	
	// init dmac
	haddr = 0;
}

void MEMORY::reset()
{
	bank1 = 0xe0;
	bank2 = 0;
	update_bank();
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xfffff;
//	if((0x80000 <= addr && addr < 0xa0000) || (0xf0000 <= addr && addr < 0xfc000)) {
//		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
//	}
	wbank[addr >> 14][addr & 0x3fff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xfffff;
//	if((0x80000 <= addr && addr < 0xa0000) || (0xf0000 <= addr && addr < 0xfc000)) {
//		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
//	}
	return rbank[addr >> 14][addr & 0x3fff];
}

void MEMORY::write_dma_data8(uint32_t addr, uint32_t data)
{
	addr = (addr & 0xffff) | haddr;
//	if((0x80000 <= addr && addr < 0xa0000) || (0xf0000 <= addr && addr < 0xfc000)) {
//		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
//	}
	wbank[addr >> 14][addr & 0x3fff] = data;
}

uint32_t MEMORY::read_dma_data8(uint32_t addr)
{
	addr = (addr & 0xffff) | haddr;
//	if((0x80000 <= addr && addr < 0xa0000) || (0xf0000 <= addr && addr < 0xfc000)) {
//		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
//	}
	return rbank[addr >> 14][addr & 0x3fff];
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x50:
		haddr = (data & 0xf0) << 12;
		break;
#if defined(_MZ6500) || defined(_MZ6550)
	case 0xcd:
		// MZ-1R32
		if(bank2 != (data & 0x0f)) {
			bank2 = data & 0x0f;
			update_bank();
		}
		break;
#endif
	}
}

uint32_t MEMORY::read_io8(uint32_t addr)
{
	return 0xf0 | bank2;	// ???
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(bank1 != data) {
		bank1 = data;
		update_bank();
	}
}

void MEMORY::update_bank()
{
	switch(bank1 & 0xe0) {
	case 0xe0:
		SET_BANK(0x0a0000, 0x0bffff, wdmy, kanji);
		break;
	case 0xc0:
		SET_BANK(0x0a0000, 0x0bffff, wdmy, kanji + 0x20000);
		break;
	case 0xa0:
		SET_BANK(0x0a0000, 0x0bffff, wdmy, dic);
		break;
	case 0x80:
		SET_BANK(0x0a0000, 0x0bffff, wdmy, dic + 0x20000);
		break;
#if defined(_MZ6500) || defined(_MZ6550)
	case 0x60:
		// MZ-1R32
		{
			int ofs = 0x20000 * ((bank2 >> 1) & 7);
			SET_BANK(0x0a0000, 0x0bffff, mz1r32 + ofs, mz1r32 + ofs);
		}
		break;
#endif
	default:
		SET_BANK(0x0a0000, 0x0bffff, wdmy, rdmy);
		break;
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
	state_fio->StateArray(vram, sizeof(vram), 1);
#if defined(_MZ6500) || defined(_MZ6550)
	state_fio->StateArray(mz1r32, sizeof(mz1r32), 1);
#endif
	state_fio->StateValue(bank1);
	state_fio->StateValue(bank2);
	state_fio->StateValue(haddr);
	
	// post process
	if(loading) {
		 update_bank();
		}
	return true;
}

