/*
 * Main memory for FM-7/8/77/AV/AV40
 * around tables.
 * [MAINMEM_TABLES]
 *  Author: K.Ohta
 *  Date  : 2017-04-01 Split from fm7_mainmem.cpp.
 *  License: GPLv2
 */

#include "vm.h"
#include "emu.h"
#include "fm7_mainmem.h"

void FM7_MAINMEM::setup_table_extram(void)
{
	uint32_t addr, n;
#if defined(_FM77_VARIANTS)
	for(addr = 0x00000; addr < 0x30000; addr += 0x80) {
		n = addr >> 7;
		addr_w_table_page[n] = NULL;
		addr_r_table_page[n] = NULL;
		func_r_table_page[n] = NULL;
		func_w_table_page[n] = NULL;
	}
	if(extram_connected) {
		for(addr = 0x00000; addr < (0x10000 * extram_pages); addr += 0x80) {
			n = addr >> 7;
			addr_w_table_page[n] = &fm7_mainmem_extram[addr];
			addr_r_table_page[n] = &fm7_mainmem_extram[addr];
			func_r_table_page[n] = NULL;
			func_w_table_page[n] = NULL;
		}
	}		
#elif defined(_FM77AV_VARIANTS)
	for(addr = 0x00000; addr < 0x10000; addr += 0x80) {
		n = addr >> 7;
		addr_w_table_page[n] = &fm7_mainmem_mmrbank_0[addr & 0xffff];
		addr_r_table_page[n] = &fm7_mainmem_mmrbank_0[addr & 0xffff];
		func_r_table_page[n] = NULL;
		func_w_table_page[n] = NULL;
	}
	
	for(addr = 0x10000; addr < 0x20000; addr += 0x80) {
		n = addr >> 7;
		addr_w_table_page[n] = NULL;
		addr_r_table_page[n] = NULL;
		func_r_table_page[n] = &FM7_MAINMEM::read_directaccess;
		func_w_table_page[n] = &FM7_MAINMEM::write_directaccess;
	}

	for(addr = 0x20000; addr < 0x30000; addr += 0x80) {
		n = addr >> 7;
		addr_w_table_page[n] = NULL;
		addr_r_table_page[n] = NULL;
		func_r_table_page[n] = &FM7_MAINMEM::read_page2_extcard;
		func_w_table_page[n] = &FM7_MAINMEM::write_page2_extcard;
	}
#  if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	for(addr = 0x40000; addr < 0x100000; addr += 0x80) {
		n = addr >> 7;
		addr_w_table_page[n] = NULL;
		addr_r_table_page[n] = NULL;
		func_r_table_page[n] = NULL;
		func_w_table_page[n] = NULL;
	}
	if(extram_connected) {
		for(addr = 0x40000; addr < (0x10000 * (extram_pages + 4)); addr += 0x80) {
			n = addr >> 7;
			addr_w_table_page[n] = &fm7_mainmem_extram[addr - 0x40000];
			addr_r_table_page[n] = &fm7_mainmem_extram[addr - 0x40000];
			func_r_table_page[n] = NULL;
			func_w_table_page[n] = NULL;
		}
	}
# endif		
#endif	
}

void FM7_MAINMEM::setup_table_page3(void)
{
	uint32_t addr;
	uint32_t n;
	memset(addr_r_table_page, 0x00, sizeof(addr_r_table_page));
	memset(addr_w_table_page, 0x00, sizeof(addr_w_table_page));
	memset(func_r_table_page, 0x00, sizeof(func_r_table_page));
	memset(func_w_table_page, 0x00, sizeof(func_w_table_page));
	
	for(addr = 0x30000; addr < 0x38000; addr += 0x80) {
		n = addr >> 7;
		addr_w_table_page[n] = &fm7_mainmem_omote[addr & 0xffff];
		addr_r_table_page[n] = &fm7_mainmem_omote[addr & 0xffff];
		func_r_table_page[n] = NULL;
		func_w_table_page[n] = NULL;
	}
	for(addr = 0x38000; addr < 0x3fc00; addr += 0x80) {
		n = addr >> 7;
		addr_w_table_page[n] = NULL;
		addr_r_table_page[n] = NULL;
		func_r_table_page[n] = &FM7_MAINMEM::read_page3_basicrom_uraram;
		func_w_table_page[n] = &FM7_MAINMEM::write_page3_basicrom_uraram;
	}
	for(addr = 0x3fc00; addr < 0x3fc80; addr += 0x80) {
		n = addr >> 7;
		addr_w_table_page[n] = &fm7_mainmem_bioswork[0];
		addr_r_table_page[n] = &fm7_mainmem_bioswork[0];
		func_r_table_page[n] = NULL;
		func_w_table_page[n] = NULL;
	}
	for(addr = 0x3fc80; addr < 0x3fd00; addr += 0x80) {
		n = addr >> 7;
		addr_w_table_page[n] = NULL;
		addr_r_table_page[n] = NULL;
		func_r_table_page[n] = &FM7_MAINMEM::read_page3_sharedram;
		func_w_table_page[n] = &FM7_MAINMEM::write_page3_sharedram;
	}
	for(addr = 0x3fd00; addr < 0x3fe00; addr += 0x80) {
		n = addr >> 7;
		addr_w_table_page[n] = NULL;
		addr_r_table_page[n] = NULL;
		func_r_table_page[n] = &FM7_MAINMEM::read_page3_mmio;
		func_w_table_page[n] = &FM7_MAINMEM::write_page3_mmio;
	}
	for(addr = 0x3fe00; addr < 0x40000; addr += 0x80) {
		n = addr >> 7;
		addr_w_table_page[n] = NULL;
		addr_r_table_page[n] = NULL;
		func_r_table_page[n] = &FM7_MAINMEM::read_page3_bootrom;
		func_w_table_page[n] = &FM7_MAINMEM::write_page3_bootrom;
	}
}	

void FM7_MAINMEM::update_mmr_reg_to_realmap(uint8_t seg)
{
#if defined(HAS_MMR)
	uint32_t addr1 = 0;
	uint32_t addr2 = 0;
	uint32_t s_base;
	uint32_t t_base;
	uint32_t m_base;
	uint32_t mmr_bank;
	
	if(!mmr_extend) {
		s_base = (seg & 0x03) * 0x200;
	} else {
		s_base = (seg & 0x07) * 0x200;
	}
		
	for(addr1 = 0; addr1 < 0x10; addr1 += 0x1) {
		if(!mmr_extend) {
			mmr_bank = mmr_map_data[(addr1 & 0x000f) | ((seg & 0x03) << 4)] & 0x003f;
		} else {
			mmr_bank = mmr_map_data[(addr1 & 0x000f) | ((seg & 0x07) << 4)] & 0x007f;
		}
		t_base = s_base + (addr1 * 0x20);
		m_base = (mmr_bank << 12) / 0x80;
		for(addr2 = 0; addr2 < (0x1000 / 0x80); addr2 += 0x1) {
			mmr_addr_r_table[t_base + addr2] = addr_r_table_page[m_base + addr2];
			mmr_addr_w_table[t_base + addr2] = addr_w_table_page[m_base + addr2];
			mmr_func_r_table[t_base + addr2] = func_r_table_page[m_base + addr2];
			mmr_func_w_table[t_base + addr2] = func_w_table_page[m_base + addr2];
		}
	}
#endif
}
