/*
 * Main memory for FM-7 [FM7_MAINMEM/MAINMEM_UTIL]
 *  Author: K.Ohta
 *  Date  : 2017.04.01-
 *  License: GPLv2
 *
 */
#include "vm.h"
#include "emu.h"
#include "fm7_mainmem.h"

void FM7_MAINMEM::initialize(void)
{
	int i;
	diag_load_basicrom = false;
	diag_load_bootrom_bas = false;
	diag_load_bootrom_dos = false;
	diag_load_bootrom_mmr = false;

#if defined(_FM77AV_VARIANTS)
	dictrom_connected = false;
#endif
#ifdef HAS_MMR	
	for(i = 0x00; i < 0x80; i++) {
		mmr_map_data[i] = 0;
	}
	mmr_segment = 0;
	window_offset = 0;
	mmr_enabled = false;
	mmr_fast = false;
	window_enabled = false;
#endif	
#ifdef _FM77AV_VARIANTS
	extcard_bank = 0;
	extrom_bank = false;
	dictrom_enabled = false;
	dictram_enabled = false;
	
	initiator_enabled = true;
	boot_ram_write = true;
#endif	
	bootmode = config.boot_mode & 3;
	basicrom_fd0f = false;
	is_basicrom = ((bootmode & 0x03) == 0) ? true : false;
   
	// $0000-$7FFF
	i = FM7_MAINMEM_OMOTE;
	memset(fm7_mainmem_omote, 0x00, 0x8000 * sizeof(uint8_t));

	// $8000-$FBFF
	i = FM7_MAINMEM_URA;
	memset(fm7_mainmem_ura, 0x00, 0x7c00 * sizeof(uint8_t));
	
	i = FM7_MAINMEM_VECTOR;
	memset(fm7_mainmem_bootrom_vector, 0x00, 0x1e);
	
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
    defined(_FM77_VARIANTS)
	extram_pages = FM77_EXRAM_BANKS;
#if defined(_FM77_VARIANTS)
	if(extram_pages > 3) extram_pages = 3;
#else
	if(extram_pages > 12) extram_pages = 12;
#endif 
	if(extram_pages > 0) {
		i = FM7_MAINMEM_EXTRAM;
		fm7_mainmem_extram = (uint8_t *)malloc(extram_pages * 0x10000);
		if(fm7_mainmem_extram != NULL) {
			memset(fm7_mainmem_extram, 0x00, extram_pages * 0x10000);
		}
	}
#endif	

#if defined(_FM77_VARIANTS)
	memset(fm77_shadowram, 0x00, 0x200);
#endif
#if !defined(_FM77AV_VARIANTS)	
	for(i = FM7_MAINMEM_BOOTROM_BAS; i <= FM7_MAINMEM_BOOTROM_EXTRA; i++) {
		 memset(fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS], 0xff, 0x200);
	}
#endif	
#if defined(_FM8)
	if(read_bios(_T("BOOT_BAS8.ROM"), fm7_bootroms[0], 0x200) >= 0x1e0) {
		diag_load_bootrom_bas = true;
	} else {
		diag_load_bootrom_bas = false;
	}
	if(read_bios(_T("BOOT_DOS8.ROM"), fm7_bootroms[1], 0x200) >= 0x1e0) {
		diag_load_bootrom_dos = true;
	} else {
		diag_load_bootrom_dos = false;
	}
	diag_load_bootrom_mmr = false;
# elif defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
	if(read_bios(_T("BOOT_BAS.ROM"), fm7_bootroms[0], 0x200) >= 0x1e0) {
		diag_load_bootrom_bas = true;
	} else {
		diag_load_bootrom_bas = false;
	}
	if(read_bios(_T("BOOT_DOS.ROM"), fm7_bootroms[1], 0x200) >= 0x1e0) {
		diag_load_bootrom_dos = true;
	} else {
		diag_load_bootrom_dos = false;
	}
#  if defined(_FM77_VARIANTS)
	if(read_bios(_T("BOOT_MMR.ROM"), fm7_bootroms[2], 0x200) >= 0x1e0) {
		diag_load_bootrom_mmr = true;
	} else {
		diag_load_bootrom_mmr = false;
	}
   
	i = FM7_MAINMEM_BOOTROM_RAM;
	memset(fm7_bootram, 0x00, 0x200 * sizeof(uint8_t)); // RAM
#  else
       // FM-7/8
	diag_load_bootrom_mmr = false;
#  endif
# elif defined(_FM77AV_VARIANTS)
	i = FM7_MAINMEM_AV_PAGE0;
	memset(fm7_mainmem_mmrbank_0, 0x00, 0x10000 * sizeof(uint8_t));
	
	i = FM7_MAINMEM_AV_PAGE2;
	memset(fm7_mainmem_mmrbank_2, 0x00, 0x10000 * sizeof(uint8_t));
	
	i = FM7_MAINMEM_INITROM;
	diag_load_initrom = false;
	memset(fm7_mainmem_initrom, 0xff, 0x2000 * sizeof(uint8_t));

	if(read_bios(_T("INITIATE.ROM"), fm7_mainmem_initrom, 0x2000) >= 0x2000) diag_load_initrom = true;
	this->out_debug_log(_T("77AV INITIATOR ROM READING : %s"), diag_load_initrom ? "OK" : "NG");

	if(read_bios(_T("BOOT_MMR.ROM"), fm77av_hidden_bootmmr, 0x200) < 0x1e0) {
		memcpy(fm77av_hidden_bootmmr, &fm7_mainmem_initrom[0x1a00], 0x200);
	}
	fm77av_hidden_bootmmr[0x1fe] = 0xfe;
	fm77av_hidden_bootmmr[0x1fe] = 0x00;
	
	i = FM7_MAINMEM_BOOTROM_RAM;
	memset(fm7_bootram, 0x00, 0x200 * sizeof(uint8_t)); // RAM
	
	if(diag_load_initrom) diag_load_bootrom_bas = true;
	if(diag_load_initrom) diag_load_bootrom_dos = true;
	
	if((config.boot_mode & 0x03) == 0) {
		memcpy(fm7_bootram, &fm7_mainmem_initrom[0x1800], 0x1e0 * sizeof(uint8_t));
	} else {
		memcpy(fm7_bootram, &fm7_mainmem_initrom[0x1a00], 0x1e0 * sizeof(uint8_t));
	}
	fm7_bootram[0x1fe] = 0xfe; // Set reset vector.
	fm7_bootram[0x1ff] = 0x00; //
	// FM-7
#endif
	this->out_debug_log(_T("BOOT ROM (basic mode) READING : %s"), diag_load_bootrom_bas ? "OK" : "NG");
	this->out_debug_log(_T("BOOT ROM (DOS   mode) READING : %s"), diag_load_bootrom_dos ? "OK" : "NG");
#if defined(_FM77_VARIANTS)
	this->out_debug_log(_T("BOOT ROM (MMR   mode) READING : %s"), diag_load_bootrom_mmr ? "OK" : "NG");
#endif


#if !defined(_FM77AV_VARIANTS)
	for(i = 0; i <= 3; i++) {
		uint8_t *p = fm7_bootroms[i];
		p[0x1fe] = 0xfe; // Set reset vector.
		p[0x1ff] = 0x00; //
	}
	
#endif	
	i = FM7_MAINMEM_RESET_VECTOR;
	fm7_mainmem_reset_vector[0] = 0xfe;
	fm7_mainmem_reset_vector[1] = 0x00;
   
	i = FM7_MAINMEM_BASICROM;
	memset(fm7_mainmem_basicrom, 0xff, 0x7c00 * sizeof(uint8_t));

#if !defined(_FM8)
	if(read_bios(_T("FBASIC302.ROM"), fm7_mainmem_basicrom, 0x7c00) == 0x7c00) {
		diag_load_basicrom = true;
	} else if(read_bios(_T("FBASIC300.ROM"), fm7_mainmem_basicrom, 0x7c00) == 0x7c00) {
		diag_load_basicrom = true;
	} else if(read_bios(_T("FBASIC30.ROM"), fm7_mainmem_basicrom, 0x7c00) == 0x7c00) {
		diag_load_basicrom = true;
	}
   
#else // FM8
	if(read_bios(_T("FBASIC10.ROM"), fm7_mainmem_basicrom, 0x7c00) == 0x7c00) diag_load_basicrom = true;
#endif	
	this->out_debug_log(_T("BASIC ROM READING : %s"), diag_load_basicrom ? "OK" : "NG");
   
	i = FM7_MAINMEM_BIOSWORK;
	memset(fm7_mainmem_bioswork, 0x00, 0x80 * sizeof(uint8_t));
#if defined(CAPABLE_DICTROM)
	diag_load_dictrom = false;
	i = FM7_MAINMEM_DICTROM;
	memset(fm7_mainmem_dictrom, 0xff, 0x40000 * sizeof(uint8_t));
	if(read_bios(_T("DICROM.ROM"), fm7_mainmem_dictrom, 0x40000) == 0x40000) diag_load_dictrom = true;
	this->out_debug_log(_T("DICTIONARY ROM READING : %s"), diag_load_dictrom ? "OK" : "NG");
	dictrom_connected = diag_load_dictrom;
	
	i = FM7_MAINMEM_BACKUPED_RAM;
	diag_load_learndata = false;
	memset(fm7_mainmem_learndata, 0x00, 0x2000 * sizeof(uint8_t));
	
	if(read_bios(_T("USERDIC.DAT"), fm7_mainmem_learndata, 0x2000) == 0x2000) diag_load_learndata = true;
	this->out_debug_log(_T("DICTIONARY BACKUPED RAM READING : %s"), diag_load_learndata ? "OK" : "NG");
	if(!diag_load_learndata) write_bios(_T("USERDIC.DAT"), fm7_mainmem_learndata, 0x2000);
#endif
	
 	i = FM7_MAINMEM_77AV40_EXTRAROM;
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	diag_load_extrarom = false;
	memset(fm7_mainmem_extrarom, 0xff, sizeof(fm7_mainmem_extrarom));
	if(read_bios(_T("EXTSUB.ROM"), fm7_mainmem_extrarom, 0xc000) == 0xc000) diag_load_extrarom = true;
	this->out_debug_log(_T("AV40SX/EX EXTRA ROM READING : %s"), diag_load_extrarom ? "OK" : "NG");
#endif
	init_data_table();
}

void FM7_MAINMEM::release()
{
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
 	 defined(_FM77_VARIANTS)
	if(fm7_mainmem_extram != NULL) free(fm7_mainmem_extram);
#endif  
#if !defined(_FM77AV_VARIANTS)
	int i;
	for(i = 0; i < 4; i++) {
		if(fm7_bootroms[i] != NULL) free(fm7_bootroms[i]);
		fm7_bootroms[i] = NULL;
	}
#endif
#if defined(CAPABLE_DICTROM)
	write_bios(_T("USERDIC.DAT"), fm7_mainmem_learndata, 0x2000);
#endif
//	MEMORY::release();
}

void FM7_MAINMEM::init_data_table(void)
{
	uint32_t addr;
	uint32_t paddr;
	uint32_t main_begin;
	data_func_table_t *p;
	memset(data_table, 0x00, sizeof(data_table));
#if defined(HAS_MMR)
	main_begin = 0x30000;
#else
	main_begin = 0x00000;
#endif
	for(addr = main_begin; addr < (main_begin + 0x8000); addr += 0x80) {
		paddr = addr >> 7;
		data_table[paddr].read_data  = &fm7_mainmem_omote[addr & 0x7fff];
		data_table[paddr].write_data = &fm7_mainmem_omote[addr & 0x7fff];
	}
	for(addr = main_begin + 0x8000; addr < (main_begin + 0xfc00); addr += 0x80) {
		paddr = addr >> 7;
		data_table[paddr].read_func  = &FM7_MAINMEM::read_ura_basicrom;
		data_table[paddr].write_func = &FM7_MAINMEM::write_ura_basicrom;
	}
	{
		addr = main_begin + 0xfc00;
		paddr = addr >> 7;
		data_table[paddr].read_data  = fm7_mainmem_bioswork;
		data_table[paddr].write_data = fm7_mainmem_bioswork;
	}
	{
		addr = main_begin + 0xfc80;
		paddr = addr >> 7;
		data_table[paddr].read_func  = &FM7_MAINMEM::read_shared_ram;
		data_table[paddr].write_func = &FM7_MAINMEM::write_shared_ram;
	}
	for(addr = main_begin + 0xfd00; addr < (main_begin + 0xfe00); addr += 0x80) {
		paddr = addr >> 7;
		data_table[paddr].read_func  = &FM7_MAINMEM::read_mmio;
		data_table[paddr].write_func = &FM7_MAINMEM::write_mmio;
	}
	for(addr = main_begin + 0xfe00; addr < (main_begin + 0x10000); addr += 0x80) {
		paddr = addr >> 7;
		data_table[paddr].read_func  = &FM7_MAINMEM::read_bootrom;
		data_table[paddr].write_func = &FM7_MAINMEM::write_bootrom;
	}
	
#if defined(_FM77AV_VARIANTS)
	for(addr = 0x00000; addr < 0x10000; addr += 0x80) {
		paddr = addr >> 7;
		data_table[paddr].read_data  = &fm7_mainmem_mmrbank_0[addr & 0xffff];
		data_table[paddr].write_data = &fm7_mainmem_mmrbank_0[addr & 0xffff];
	}
	for(addr = 0x10000; addr < 0x20000; addr += 0x80) {
		paddr = addr >> 7;
		data_table[paddr].read_func = &FM7_MAINMEM::read_direct_access;
		data_table[paddr].write_func = &FM7_MAINMEM::write_direct_access;
	}
# if defined(CAPABLE_DICTROM)
	for(addr = 0x20000; addr < 0x30000; addr += 0x80) {
		paddr = addr >> 7;
		data_table[paddr].read_func = &FM7_MAINMEM::read_page2;
		data_table[paddr].write_func = &FM7_MAINMEM::write_page2;
	}
# else
	for(addr = 0x20000; addr < 0x30000; addr += 0x80) {
		paddr = addr >> 7;
		data_table[paddr].read_data  = &fm7_mainmem_mmrbank_2[addr & 0xffff];
		data_table[paddr].write_data = &fm7_mainmem_mmrbank_2[addr & 0xffff];
	}
# endif
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	int pages = extram_pages;
	if((pages > 0) && (pages < 12) && (fm7_mainmem_extram != NULL)) {
		for(addr = 0x40000; addr < (0x40000 + extram_pages * 0x10000) ; addr += 0x80) {
			paddr = addr >> 7;
			data_table[paddr].read_data  = &fm7_mainmem_extram[addr - 0x40000];
			data_table[paddr].write_data = &fm7_mainmem_extram[addr - 0x40000];
		}
	}
# endif
#elif defined(_FM77_VARIANTS)
	int pages = extram_pages;
	if((pages > 0) && (pages < 4) && (fm7_mainmem_extram != NULL)) {
		for(addr = 0x10000; addr < (extram_pages * 0x10000) ; addr += 0x80) {
			paddr = addr >> 7;
			data_table[paddr].read_data  = &fm7_mainmem_extram[addr];
			data_table[paddr].write_data = &fm7_mainmem_extram[addr];
		}
	}
#endif	
}	

bool FM7_MAINMEM::get_loadstat_basicrom(void)
{
	return diag_load_basicrom;
}

bool FM7_MAINMEM::get_loadstat_bootrom_bas(void)
{
	return diag_load_bootrom_bas;
}

bool FM7_MAINMEM::get_loadstat_bootrom_dos(void)
{
	return diag_load_bootrom_dos;
}

uint32_t FM7_MAINMEM::read_bios(const _TCHAR *name, uint8_t *ptr, uint32_t size)
{
	FILEIO fio;
	uint32_t blocks;
	const _TCHAR *s;
  
	if((name == NULL) || (ptr == NULL))  return 0;
	s = create_local_path(name);
	if(s == NULL) return 0;
  
	if(!fio.Fopen(s, FILEIO_READ_BINARY)) return 0;
	blocks = fio.Fread(ptr, size, 1);
	fio.Fclose();

	return blocks * size;
}

uint32_t FM7_MAINMEM::write_bios(const _TCHAR *name, uint8_t *ptr, uint32_t size)
{
	FILEIO fio;
	uint32_t blocks;
	const _TCHAR *s;
  
	if((name == NULL) || (ptr == NULL))  return 0;
	s = create_local_path(name);
	if(s == NULL) return 0;
  
	fio.Fopen(s, FILEIO_WRITE_BINARY);
	blocks = fio.Fwrite(ptr, size, 1);
	fio.Fclose();

	return blocks * size;
}

