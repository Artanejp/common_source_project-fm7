/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.11.24 -

	[ memory ]
*/

#include "memory.h"

#define PAGE_TYPE_NORMAL	0
#define PAGE_TYPE_VRAM		1
#define PAGE_TYPE_KANJI		2
#define PAGE_TYPE_DIC		3
#define PAGE_TYPE_MODIFY	4

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
	memset(tvram, 0, sizeof(tvram));
	memset(pcg, 0, sizeof(pcg));
	memset(ipl, 0xff, sizeof(ipl));
	memset(kanji, 0xff, sizeof(kanji));
	memset(dic, 0xff, sizeof(dic));
	memset(phone, 0xff, sizeof(phone));
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
	if(fio->Fopen(create_local_path(_T("PHONE.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(phone, sizeof(phone), 1);
		fio->Fclose();
	}
	delete fio;
}

// NOTE: IPL reset is done at system boot

void MEMORY::reset()
{
	// ipl reset
	bank = 0;
	set_map(0x34);
	set_map(0x35);
	set_map(0x36);
	set_map(0x37);
	set_map(0x04);
	set_map(0x05);
	set_map(0x06);
	set_map(0x07);
	
	// reset crtc signals
	blank = hblank = vblank = busreq = false;
	extra_wait = 0;
}

void MEMORY::special_reset()
{
	// reset
	bank = 0;
	set_map(0x00);
	set_map(0x01);
	set_map(0x02);
	set_map(0x03);
	set_map(0x04);
	set_map(0x05);
	set_map(0x06);
	set_map(0x07);
	
	// reset crtc signals
	blank = hblank = vblank = busreq = false;
	extra_wait = 0;
}

void MEMORY::write_data8_tmp(int b, uint32_t addr, uint32_t data)
{
	if(is_vram[b] && !blank) {
		// vram wait
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		busreq = true;
	}
	if(page_type[b] == PAGE_TYPE_MODIFY) {
		// read write modify
		if(page[b] == 0x30) {
			d_crtc->write_data8((addr & 0x1fff) + 0x0000, data);
		} else if(page[b] == 0x31) {
			d_crtc->write_data8((addr & 0x1fff) + 0x2000, data);
		} else if(page[b] == 0x32) {
			d_crtc->write_data8((addr & 0x1fff) + 0x4000, data);
		} else {
			d_crtc->write_data8((addr & 0x1fff) + 0x6000, data);
		}
		return;
	}
	wbank[addr >> 11][addr & 0x7ff] = data;
}

uint32_t MEMORY::read_data8_tmp(int b, uint32_t addr)
{
	if(is_vram[b] && !blank) {
		// vram wait
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		busreq = true;
	}
	if(page_type[b] == PAGE_TYPE_MODIFY) {
		// read write modify
		if(page[b] == 0x30) {
			return d_crtc->read_data8((addr & 0x1fff) + 0x0000);
		} else if(page[b] == 0x31) {
			return d_crtc->read_data8((addr & 0x1fff) + 0x2000);
		} else if(page[b] == 0x32) {
			return d_crtc->read_data8((addr & 0x1fff) + 0x4000);
		} else {
			return d_crtc->read_data8((addr & 0x1fff) + 0x6000);
		}
	}
	return rbank[addr >> 11][addr & 0x7ff];
}


void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xffff;
	int b = addr >> 13;
	write_data8_tmp(b, addr, data);
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	addr &= 0xffff;
	int b = addr >> 13;
	return read_data8_tmp(b, addr);
}

void MEMORY::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	addr &= 0xffff;
	int b = addr >> 13;
	write_data8_tmp(b, addr, data);
	
/*
	if(busreq) {
		*wait = 0;
		extra_wait += page_wait[b];
	} else {
		*wait = page_wait[b] + extra_wait;
		extra_wait = 0;
	}
*/
	*wait = page_wait[b];
}

uint32_t MEMORY::read_data8w(uint32_t addr, int* wait)
{
	addr &= 0xffff;
	int b = addr >> 13;
	uint32_t data = read_data8_tmp(b, addr);
	
/*
	if(busreq) {
		*wait = 0;
		extra_wait += page_wait[b];
	} else {
		*wait = page_wait[b] + extra_wait;
		extra_wait = 0;
	}
*/
	*wait = page_wait[b];
	return data;
}

uint32_t MEMORY::fetch_op(uint32_t addr, int* wait)
{
	*wait = 1;
	return read_data8(addr);
}

void MEMORY::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xb4:
		// map bank
		bank = data & 7;
		break;
	case 0xb5:
		// map reg
		set_map(data & 0x3f);
		break;
	case 0xce:
		// dictionary bank
		dic_bank = data & 0x1f;
		for(int i = 0; i < 8; i++) {
			if(page_type[i] == PAGE_TYPE_DIC) {
				SET_BANK(i * 0x2000,  i * 0x2000 + 0x1fff, wdmy, dic + dic_bank * 0x2000);
			}
		}
		break;
	case 0xcf:
		// kanji bank
		kanji_bank = data;
		for(int i = 0; i < 8; i++) {
			if(page_type[i] == PAGE_TYPE_KANJI) {
				if(kanji_bank & 0x80) {
					SET_BANK(i * 0x2000,  i * 0x2000 + 0x7ff, wdmy, kanji + (kanji_bank & 0x7f) * 0x800);
				} else {
					SET_BANK(i * 0x2000,  i * 0x2000 + 0x7ff, pcg, pcg);
				}
			}
		}
		break;
	}
}

uint32_t MEMORY::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xb4:
		// map bank
		return bank;
	case 0xb5:
		// map reg
		uint32_t val = page[bank];
		bank = (bank + 1) & 7;
		return val;
	}
	return 0xff;
}

void MEMORY::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MEMORY_HBLANK) {
		hblank = ((data & mask) != 0);
	} else if(id == SIG_MEMORY_VBLANK) {
		vblank = ((data & mask) != 0);
	}
	
	// if blank, disable busreq
	bool next = hblank || vblank;
	if(!blank && next && busreq) {
		d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 1);
		busreq = false;
	}
	blank = next;
}

void MEMORY::set_map(uint8_t data)
{
	int base = bank * 0x2000;
	
	page_wait[bank] = 0;
	is_vram[bank] = false;
	if(data <= 0x1f) {
		// main ram
		SET_BANK(base,  base + 0x1fff, ram + data * 0x2000, ram + data * 0x2000);
		page_type[bank] = PAGE_TYPE_NORMAL;
	} else if(0x20 <= data && data <= 0x2f) {
		// vram
		static const int ofs_table[] = {0x00, 0x01, 0x04, 0x05, 0x08, 0x09, 0x0c, 0x0d, 0x02, 0x03, 0x06, 0x07, 0x0a, 0x0b, 0x0e, 0x0f};
		int ofs = ofs_table[data - 0x20] * 0x2000;
		SET_BANK(base,  base + 0x1fff, vram + ofs, vram + ofs);
		page_type[bank] = PAGE_TYPE_VRAM;
		page_wait[bank] = 1;
		is_vram[bank] = true;
	} else if(0x30 <= data && data <= 0x33) {
		// read modify write
		SET_BANK(base,  base + 0x1fff, wdmy, rdmy);
		page_type[bank] = PAGE_TYPE_MODIFY;
		page_wait[bank] = 2;
		is_vram[bank] = true;
	} else if(0x34 <= data && data <= 0x37) {
		// ipl rom
		SET_BANK(base,  base + 0x1fff, wdmy, ipl + (data - 0x34) * 0x2000);
		page_type[bank] = PAGE_TYPE_NORMAL;
	} else if(data == 0x38) {
		// text vram
		SET_BANK(base         ,  base + 0x17ff, tvram, tvram);
		SET_BANK(base + 0x1800,  base + 0x1fff, wdmy, rdmy);
		page_type[bank] = PAGE_TYPE_VRAM;
		page_wait[bank] = 1;
		is_vram[bank] = true;
	} else if(data == 0x39) {
		// kanji rom, pcg
		SET_BANK(base,  base + 0x1fff, pcg, pcg);
		if(kanji_bank & 0x80) {
			SET_BANK(base,  base + 0x7ff, wdmy, kanji + (kanji_bank & 0x7f) * 0x800);
		}
		page_type[bank] = PAGE_TYPE_KANJI;
		page_wait[bank] = 2;
		is_vram[bank] = true;
	} else if(data == 0x3a) {
		// dictionary rom
		SET_BANK(base,  base + 0x1fff, wdmy, dic + dic_bank * 0x2000);
		page_type[bank] = PAGE_TYPE_DIC;
	} else if(0x3c <= data && data <= 0x3f) {
		// phone rom
		SET_BANK(base,  base + 0x1fff, wdmy, phone + (data - 0x3c) * 0x2000);
		page_type[bank] = PAGE_TYPE_NORMAL;
	} else {
		// n.c
		SET_BANK(base,  base + 0x1fff, wdmy, rdmy);
		page_type[bank] = PAGE_TYPE_NORMAL;
	}
	page[bank] = data;
	bank = (bank + 1) & 7;
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
	state_fio->StateArray(tvram, sizeof(tvram), 1);
	state_fio->StateArray(pcg, sizeof(pcg), 1);
	state_fio->StateValue(bank);
	state_fio->StateArray(page, sizeof(page), 1);
	state_fio->StateValue(dic_bank);
	state_fio->StateValue(kanji_bank);
	state_fio->StateValue(blank);
	state_fio->StateValue(hblank);
	state_fio->StateValue(vblank);
	state_fio->StateValue(busreq);
	
	// post process
	if(loading) {
		uint8_t bank_tmp = bank;
		bank = 0;
		for(int i = 0; i < 8; i++) {
			set_map(page[i]);
		}
		bank = bank_tmp;
	}
	return true;
}

