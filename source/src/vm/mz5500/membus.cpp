/*
	SHARP MZ-5500 Emulator 'EmuZ-5500'

	Author : Takeda.Toshiya
	Date   : 2008.04.10 -

	[ memory bus ]
*/

#include "membus.h"
#include "../i8237.h"

void MEMBUS::initialize()
{
	MEMORY::initialize();
	
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
	set_memory_rw(0x00000, 0x9ffff, ram);
#else
	set_memory_rw(0x00000, 0x7ffff, ram);
#endif
	set_memory_r (0xa0000, 0xbffff, kanji);
	set_memory_rw(0xc0000, 0xeffff, vram);
#ifdef _MZ6550
	set_memory_r (0xf8000, 0xfffff, ipl);
#else
	set_memory_r (0xfc000, 0xfffff, ipl);
#endif
}

void MEMBUS::reset()
{
	bank1 = 0xe0;
	bank2 = 0;
	update_bank();
}

/*
void MEMBUS::write_data8(uint32_t addr, uint32_t data)
{
	addr &= 0xfffff;
	if((0x80000 <= addr && addr < 0xa0000) || (0xf0000 <= addr && addr < 0xfc000)) {
		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
	}
	MEMORY::write_data8(addr, data);
}

uint32_t MEMBUS::read_data8(uint32_t addr)
{
	addr &= 0xfffff;
	if((0x80000 <= addr && addr < 0xa0000) || (0xf0000 <= addr && addr < 0xfc000)) {
		d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
	}
	return MEMORY::read_data8(addr);
}
*/

void MEMBUS::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x50:
		d_dma->write_signal(SIG_I8237_BANK0, data >> 4, 0x0f);
		d_dma->write_signal(SIG_I8237_BANK1, data >> 4, 0x0f);
		d_dma->write_signal(SIG_I8237_BANK2, data >> 4, 0x0f);
		d_dma->write_signal(SIG_I8237_BANK3, data >> 4, 0x0f);
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

uint32_t MEMBUS::read_io8(uint32_t addr)
{
	return 0xf0 | bank2;	// ???
}

void MEMBUS::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(bank1 != data) {
		bank1 = data;
		update_bank();
	}
}

void MEMBUS::update_bank()
{
	switch(bank1 & 0xe0) {
	case 0xe0:
		set_memory_r(0x0a0000, 0x0bffff, kanji);
		unset_memory_w(0x0a0000, 0x0bffff);
		break;
	case 0xc0:
		set_memory_r(0x0a0000, 0x0bffff, kanji + 0x20000);
		unset_memory_w(0x0a0000, 0x0bffff);
		break;
	case 0xa0:
		set_memory_r(0x0a0000, 0x0bffff, dic);
		unset_memory_w(0x0a0000, 0x0bffff);
		break;
	case 0x80:
		set_memory_r(0x0a0000, 0x0bffff, dic + 0x20000);
		unset_memory_w(0x0a0000, 0x0bffff);
		break;
#if defined(_MZ6500) || defined(_MZ6550)
	case 0x60:
		// MZ-1R32
		{
			int ofs = 0x20000 * ((bank2 >> 1) & 7);
			set_memory_rw(0x0a0000, 0x0bffff, mz1r32 + ofs);
		}
		break;
#endif
	default:
		unset_memory_rw(0x0a0000, 0x0bffff);
		break;
	}
}

#define STATE_VERSION	3

bool MEMBUS::process_state(FILEIO* state_fio, bool loading)
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
	
	// post process
	if(loading) {
		 update_bank();
	}
	return true;
}
