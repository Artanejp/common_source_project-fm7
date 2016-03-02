/*
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

	[ memory ]
*/

#include "slmemory.h"

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
	memset(vram, 0, sizeof(vram));
	memset(ems, 0, sizeof(ems));
	memset(kanji, 0xff, sizeof(kanji));
	memset(backup, 0, sizeof(backup));
	memset(ipl, 0xff, sizeof(ipl));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load images
	FILEIO* fio = new FILEIO();
//	if(fio->Fopen(create_local_path(_T("EMS.BIN")), FILEIO_READ_BINARY)) {
//		fio->Fread(ems, sizeof(ems), 1);
//		fio->Fclose();
//	}
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, sizeof(kanji), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("BACKUP.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(backup, sizeof(backup), 1);
		fio->Fclose();
	}
	if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	delete fio;
	
	// set memory map
	SET_BANK(0x00000, 0x9ffff, ram, ram);
	SET_BANK(0xa0000, 0xb7fff, wdmy, rdmy);
	SET_BANK(0xb8000, 0xbffff, vram, vram);
	SET_BANK(0xc0000, 0xeffff, wdmy, rdmy);
#ifdef _J3100SE
	SET_BANK(0xf0000, 0xf3fff, backup, backup);
	SET_BANK(0xf4000, 0xfffff, wdmy, ipl);
#else
	SET_BANK(0xf0000, 0xf07ff, backup, backup);
	SET_BANK(0xf0800, 0xf7fff, wdmy, rdmy);
	SET_BANK(0xf8000, 0xfffff, wdmy, ipl);
#endif
	
	kanji_bank = 0;
	ems_index = 0;
	ems_regs[0] = 0;
	ems_regs[1] = 2;
	memset(ems_page, 0, sizeof(ems_page));
	ems_bsl = 3;
	
	ems_crc32 = get_crc32(ems, sizeof(ems));
	backup_crc32 = get_crc32(backup, sizeof(backup));
}

void MEMORY::release()
{
	// save images
	FILEIO* fio = new FILEIO();
//	if(ems_crc32 != get_crc32(ems, sizeof(ems)))
//		if(fio->Fopen(create_local_path(_T("EMS.BIN")), FILEIO_WRITE_BINARY)) {
//			fio->Fwrite(ems, sizeof(ems), 1);
//			fio->Fclose();
//		}
//	}
	if(backup_crc32 != get_crc32(backup, sizeof(backup))) {
		if(fio->Fopen(create_local_path(_T("BACKUP.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(backup, sizeof(backup), 1);
			fio->Fclose();
		}
	}
	delete fio;
}

void MEMORY::reset()
{
	
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	if((addr & 0xf0000) == 0xe0000) {
		if(kanji_bank != (data & 0x8f)) {
			if(data & 0x80) {
				SET_BANK(0xe0000, 0xeffff, wdmy, kanji + 0x10000 * (data & 0x0f));
			} else {
				SET_BANK(0xe0000, 0xeffff, wdmy, rdmy);
			}
			kanji_bank = data & 0x8f;
		}
		return;
	}
	addr &= 0xfffff;
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xfffff;
	return rbank[addr >> 11][addr & 0x7ff];
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xffff) {
	case 0xee:
		ems_index = data;
		break;
	case 0xef:
		if(ems_index == 50 || ems_index == 51) {
			ems_regs[ems_index - 50] = data;
		}
		break;
	case 0x208: case 0x4208: case 0x8208: case 0xc208:
	case 0x218: case 0x4218: case 0x8218: case 0xc218:
	case 0x258: case 0x4258: case 0x8258: case 0xc258:
	case 0x268: case 0x4268: case 0x8268: case 0xc268:
	case 0x2a8: case 0x42a8: case 0x82a8: case 0xc2a8:
	case 0x2b8: case 0x42b8: case 0x82b8: case 0xc2b8:
	case 0x2e8: case 0x42e8: case 0x82e8: case 0xc2e8:
		if(((addr >> 4) & 0x0f) == (ems_regs[0] & 0x0f)) {
			int page = (addr >> 14) & 3;
			if(ems_page[page] != data) {
				ems_page[page] = data;
				update_ems(page);
			}
		}
		break;
	case 0x209: case 0x4209: case 0x8209: case 0xc209:
	case 0x219: case 0x4219: case 0x8219: case 0xc219:
	case 0x259: case 0x4259: case 0x8259: case 0xc259:
	case 0x269: case 0x4269: case 0x8269: case 0xc269:
	case 0x2a9: case 0x42a9: case 0x82a9: case 0xc2a9:
	case 0x2b9: case 0x42b9: case 0x82b9: case 0xc2b9:
	case 0x2e9: case 0x42e9: case 0x82e9: case 0xc2e9:
		if(((addr >> 4) & 0x0f) == (ems_regs[0] & 0x0f)) {
			int bit = 1 << ((addr >> 14) & 3);
			int bsl = (data & 0x80) ? (ems_bsl | bit) : (ems_bsl & ~bit);
			if(ems_bsl != bsl) {
				ems_bsl = bsl;
				update_ems(1);
				update_ems(2);
				update_ems(3);
			}
		}
		break;
	}
}

uint32_t MEMORY::read_io8(uint32_t addr)
{
	switch(addr & 0xffff) {
	case 0xef:
		if(ems_index == 50) {
			return (ems_regs[0] & 0x0f) | 0x80;
		} else if(ems_index == 51) {
			return (ems_regs[1] & 0x1f) | (ems_bsl << 5);
		}
		break;
	case 0x208: case 0x4208: case 0x8208: case 0xc208:
	case 0x218: case 0x4218: case 0x8218: case 0xc218:
	case 0x258: case 0x4258: case 0x8258: case 0xc258:
	case 0x268: case 0x4268: case 0x8268: case 0xc268:
	case 0x2a8: case 0x42a8: case 0x82a8: case 0xc2a8:
	case 0x2b8: case 0x42b8: case 0x82b8: case 0xc2b8:
	case 0x2e8: case 0x42e8: case 0x82e8: case 0xc2e8:
		if(((addr >> 4) & 0x0f) == (ems_regs[0] & 0x0f)) {
			int page = (addr >> 14) & 3;
			return ems_page[page];
		}
		break;
	}
	return 0xff;
}

void MEMORY::update_ems(int page)
{
	int pe = ems_page[page] & 0x80;
	int pa = ems_page[page] & 0x7f;
	
	switch(page) {
	case 0:
		if(pe && pa < 56) {
			SET_BANK(0xd0000, 0xd3fff, ems + 0x4000 * pa, ems + 0x4000 * pa);
		} else {
			SET_BANK(0xd0000, 0xd3fff, wdmy, rdmy);
		}
		break;
	case 1:
		if(pe && pa < 56) {
			if(ems_bsl == 0) {
				SET_BANK(0xc4000, 0xc7fff, ems + 0x4000 * pa, ems + 0x4000 * pa);
				SET_BANK(0xd4000, 0xd7fff, wdmy, rdmy);
			} else {
				SET_BANK(0xc4000, 0xc7fff, wdmy, rdmy);
				SET_BANK(0xd4000, 0xd7fff, ems + 0x4000 * pa, ems + 0x4000 * pa);
			}
		} else {
			SET_BANK(0xc4000, 0xc7fff, wdmy, rdmy);
			SET_BANK(0xd4000, 0xd7fff, wdmy, rdmy);
		}
		break;
	case 2:
		if(pe && pa < 56) {
			if(ems_bsl <= 1) {
				SET_BANK(0xc8000, 0xcbfff, ems + 0x4000 * pa, ems + 0x4000 * pa);
				SET_BANK(0xd8000, 0xdbfff, wdmy, rdmy);
			} else {
				SET_BANK(0xc8000, 0xcbfff, wdmy, rdmy);
				SET_BANK(0xd8000, 0xdbfff, ems + 0x4000 * pa, ems + 0x4000 * pa);
			}
		} else {
			SET_BANK(0xc8000, 0xcbfff, wdmy, rdmy);
			SET_BANK(0xd8000, 0xdbfff, wdmy, rdmy);
		}
		break;
	case 3:
		if(pe && pa < 56) {
			if(ems_bsl <= 2) {
				SET_BANK(0xcc000, 0xcffff, ems + 0x4000 * pa, ems + 0x4000 * pa);
				SET_BANK(0xdc000, 0xdffff, wdmy, rdmy);
			} else {
				SET_BANK(0xcc000, 0xcffff, wdmy, rdmy);
				SET_BANK(0xdc000, 0xdffff, ems + 0x4000 * pa, ems + 0x4000 * pa);
			}
		} else {
			SET_BANK(0xcc000, 0xcffff, wdmy, rdmy);
			SET_BANK(0xdc000, 0xdffff, wdmy, rdmy);
		}
		break;
	}
}

