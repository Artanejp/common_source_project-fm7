/*
 * Main memory for FM-7 [FM7_MAINMEM/MAINMEM_UTIL]
 *  Author: K.Ohta
 *  Date  : 2017.04.01-
 *  License: GPLv2
 *
 */
#include "../vm.h"
#include "../../emu.h"
#include "fm7_mainmem.h"

void FM7_MAINMEM::initialize(void)
{
	int i;
	diag_load_basicrom = false;
	diag_load_bootrom_bas = false;
	diag_load_bootrom_dos = false;
	diag_load_bootrom_mmr = false;
	diag_load_bootrom_bubble = false;
	diag_load_bootrom_bubble_128k = false;
	diag_load_bootrom_sfd8 = false;
	diag_load_bootrom_2hd = false;

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
#if defined(_FM7) || defined(_FM77AV_VARIANTS)
	bootmode = config.boot_mode & 3;
#else
	bootmode = config.boot_mode & 7;
#endif
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
	extram_size = extram_pages * 0x10000;
	if(extram_pages > 0) {
		i = FM7_MAINMEM_EXTRAM;
		fm7_mainmem_extram = (uint8_t *)malloc(extram_size);
		if(fm7_mainmem_extram != NULL) {
			memset(fm7_mainmem_extram, 0x00, extram_size);
		}
	}
#endif	

#if defined(_FM77_VARIANTS)
	memset(fm77_shadowram, 0x00, 0x200);
#endif
#if defined(_FM77_VARIANTS) || defined(_FM8)
	for(i = 0; i < 8; i++) memset(fm7_bootroms[i], 0xff, 0x200);
#elif defined(_FM7) || defined(_FMNEW7)
	for(i = 0; i < 4; i++) memset(fm7_bootroms[i], 0xff, 0x200);
#endif	
#if defined(_FM8)

	// FM-8 HAS TWO TYPE of BOOTROM.
	// See http://www.mindspring.com/~thasegaw/rpcg/fm8_boot.html .
	{
		uint8_t *tmpp;
		bool tmpb = false;
		tmpp = (uint8_t *)malloc(0x800);
		// SM11-14
		diag_load_sm11_14 = false;
		diag_load_sm11_15 = false;
		if(tmpp != NULL) {
			tmpb = (read_bios(_T(ROM_FM8_SM11_14), tmpp, 0x800) >= 0x800);
		}
		if(tmpb) {
			memcpy(fm7_bootroms[0], &(tmpp[0x000]), 0x200); // BASIC
			memcpy(fm7_bootroms[2], &(tmpp[0x200]), 0x200); // BUBBLE
			memcpy(fm7_bootroms[1], &(tmpp[0x400]), 0x200); // DOS 320K
			memcpy(fm7_bootroms[3], &(tmpp[0x600]), 0x200); // DEBUG
			this->out_debug_log(_T("NOTE: LOADING BULK BOOTROM SM11-14 OK."));
			diag_load_sm11_14 = true;
		} else {	
			if(read_bios(_T(ROM_FM8_BOOT_BASIC), fm7_bootroms[0], 0x200) >= 0x1e0) {
				diag_load_bootrom_bas = true;
			}
			if(read_bios(_T(ROM_FM8_BOOT_DOS), fm7_bootroms[1], 0x200) >= 0x1e0) {
				diag_load_bootrom_dos = true;
			}
			if(read_bios(_T(ROM_FM8_BOOT_BUBBLE_128K), fm7_bootroms[2], 0x200) >= 0x1e0) {
				diag_load_bootrom_bubble_128k = true;
			} else if(read_bios(_T(ROM_FM8_BOOT_BUBBLE), fm7_bootroms[2], 0x200) >= 0x1e0) {
				diag_load_bootrom_bubble = true;
			}
			if(read_bios(_T(ROM_FM8_BOOT_DEBUG), fm7_bootroms[3], 0x200) >= 0x1e0) {
				//diag_load_bootrom_debug = true;
			}
		}
		tmpb = false;
		// SM11-15
		if(tmpp != NULL) {
			tmpb = (read_bios(_T(ROM_FM8_SM11_15), tmpp, 0x800) >= 0x800);
		}
		if(tmpb) {
			memcpy(fm7_bootroms[4], &(tmpp[0x000]), 0x200); // Basic
			memcpy(fm7_bootroms[6], &(tmpp[0x200]), 0x200); // BUBBLE
			memcpy(fm7_bootroms[5], &(tmpp[0x400]), 0x200); // DOS 320K
			memcpy(fm7_bootroms[7], &(tmpp[0x600]), 0x200); // DOS 8INCH
			this->out_debug_log(_T("NOTE: LOADING BULK BOOTROM SM11-15 OK."));
			diag_load_sm11_15 = true;
		} else {
			memcpy(fm7_bootroms[4], fm7_bootroms[0], 0x200); // Basic
			memcpy(fm7_bootroms[5], fm7_bootroms[1], 0x200); // DOS 5Inch
			memcpy(fm7_bootroms[6], fm7_bootroms[2], 0x200); // BUBBLE
			if(read_bios(_T(ROM_FM8_BOOT_DOS_FD8), fm7_bootroms[7], 0x200) >= 0x1e0) {
				diag_load_bootrom_sfd8 = true;
			}
		}
		if(tmpp != NULL) free(tmpp);
	}

#elif defined(_FM7) || defined(_FMNEW7)
	// FM-7 HAS TWO TYPE ROM.
	// See, http://www.mindspring.com/~thasegaw/rpcg/fm7rom.html .
	diag_load_tl11_11 = false;
	diag_load_tl11_12 = false;
	{
		uint8_t *tmpp;
		bool tmpb = false;
		tmpp = (uint8_t *)malloc(0x800);
		if(tmpp != NULL) memset(tmpp, 0xff, 0x800);
# if defined(_FMNEW7)
		// For FM-NEW7, If you have TL11-12, load first.
		if(tmpp != NULL) {
			diag_load_tl11_12 = (read_bios(_T(ROM_FM7_BOOT_TL11_12), tmpp, 0x800) >= 0x800);
		}
		if(diag_load_tl11_12) {
			memcpy(fm7_bootroms[0], &(tmpp[0x000]), 0x200);
			memcpy(fm7_bootroms[2], &(tmpp[0x200]), 0x200);
			memcpy(fm7_bootroms[1], &(tmpp[0x400]), 0x200);
			memcpy(fm7_bootroms[3], &(tmpp[0x600]), 0x200);
			diag_load_bootrom_bas = true;
			diag_load_bootrom_dos = true;
			diag_load_bootrom_bubble = true;
			this->out_debug_log(_T("NOTE: LOADING BULK BOOTROM TL11-12 OK"));
		}			
# endif
		if(!diag_load_tl11_12) {
			// TL11-11
			if(tmpp != NULL) {
				tmpb = (read_bios(_T(ROM_FM7_BOOT_TL11_11), tmpp, 0x800) >= 0x800);
			}
			if(tmpb) {
# if defined(_FMNEW7)
				this->out_debug_log(_T("NOTE: LOADING BULK BOOTROM TL11-11 (FALLBACK) OK"));
# else
				this->out_debug_log(_T("NOTE: LOADING BULK BOOTROM TL11-11 OK"));
# endif
				memcpy(fm7_bootroms[0], &(tmpp[0x000]), 0x200);
				memcpy(fm7_bootroms[2], &(tmpp[0x200]), 0x200);
				memcpy(fm7_bootroms[1], &(tmpp[0x400]), 0x200);
				memcpy(fm7_bootroms[3], &(tmpp[0x600]), 0x200);
				diag_load_bootrom_bas = true;
				diag_load_bootrom_dos = true;
				diag_load_bootrom_bubble = true;
				diag_load_tl11_11 = true;
			} else {			
				if(read_bios(_T(ROM_FM7_BOOT_BASIC), fm7_bootroms[0], 0x200) >= 0x1e0) {
					diag_load_bootrom_bas = true;
				}
				if(read_bios(_T(ROM_FM7_BOOT_DOS), fm7_bootroms[1], 0x200) >= 0x1e0) {
					diag_load_bootrom_dos = true;
				}
				if(read_bios(_T(ROM_FM7_BOOT_BUBBLE_7), fm7_bootroms[2], 0x200) >= 0x1e0) {
					diag_load_bootrom_bubble = true;
				}
			}
			if(tmpp != NULL) free(tmpp);
		}
	}
#elif defined(_FM77_VARIANTS)
	// FM-77 HAS ONE TYPE ROM.
	// See, http://www.mindspring.com/~thasegaw/rpcg/fm7rom.html .
	{
		uint8_t *tmpp;
		bool tmpb = false;
		tmpp = (uint8_t *)malloc(0x1000);
		// WB11-12
		diag_load_wb11_12 = false;
		if(tmpp != NULL) {
			tmpb = (read_bios(_T(ROM_FM77_BOOT_WB11_12), tmpp, 0x1000) >= 0x1000);
		}
		if(tmpb) {
			diag_load_wb11_12 = true;
			this->out_debug_log(_T("NOTE: LOADING BULK BOOTROM WB11-12 OK."));
			memcpy(fm7_bootroms[2], &(tmpp[0x000]), 0x200); // Basic (MMR)
			memcpy(fm7_bootroms[5], &(tmpp[0x200]), 0x200); // Bubble (128K)
			memcpy(fm7_bootroms[6], &(tmpp[0x400]), 0x200); // Bubble (32K)
			memcpy(fm7_bootroms[4], &(tmpp[0x600]), 0x200); // Reserve
			memcpy(fm7_bootroms[0], &(tmpp[0x800]), 0x200); // Basic
			memcpy(fm7_bootroms[1], &(tmpp[0xa00]), 0x200); // DOS (320K)
			memcpy(fm7_bootroms[3], &(tmpp[0xc00]), 0x200); // DOS (1M)
			memcpy(fm7_bootroms[7], &(tmpp[0xe00]), 0x200); // Reserve 2
			diag_load_bootrom_bas = true;
			diag_load_bootrom_dos = true;
			diag_load_bootrom_mmr = true;
			diag_load_bootrom_2hd = true;
		} else {
			if(read_bios(_T(ROM_FM7_BOOT_BASIC), fm7_bootroms[0], 0x200) >= 0x1e0) {
				diag_load_bootrom_bas = true;
			}
			
			if(read_bios(_T(ROM_FM7_BOOT_DOS), fm7_bootroms[1], 0x200) >= 0x1e0) {
				diag_load_bootrom_dos = true;
			}
			if(read_bios(_T(ROM_FM7_BOOT_MMR), fm7_bootroms[2], 0x200) >= 0x1e0) {
				diag_load_bootrom_mmr = true;
			} else {
				memcpy(fm7_bootroms[2], fm7_bootroms[0], 0x200); // Copy Fallback
				diag_load_bootrom_mmr = false;
			}				
			if(read_bios(_T(ROM_FM7_BOOT_2HD), fm7_bootroms[3], 0x200) >= 0x1e0) {
				diag_load_bootrom_2hd = true;
			}
			
			if(read_bios(_T(ROM_FM77_BOOT_BUBBLE_128K), fm7_bootroms[5], 0x200) >= 0x1e0) { // Bubble 128K
				//diag_load_bootrom_dos = true;
			}
			if(read_bios(_T(ROM_FM7_BOOT_BUBBLE_7), fm7_bootroms[6], 0x200) >= 0x1e0) { // Bubble 32K
				//diag_load_bootrom_dos = true;
			}
		}
		if(tmpp != NULL) free(tmpp);
	}
	i = FM7_MAINMEM_BOOTROM_RAM;
	memset(fm7_bootram, 0x00, 0x200 * sizeof(uint8_t)); // RAM
# elif defined(_FM77AV_VARIANTS)
	i = FM7_MAINMEM_AV_PAGE0;
	memset(fm7_mainmem_mmrbank_0, 0x00, 0x10000 * sizeof(uint8_t));
	
	i = FM7_MAINMEM_AV_PAGE2;
	memset(fm7_mainmem_mmrbank_2, 0x00, 0x10000 * sizeof(uint8_t));
	
	i = FM7_MAINMEM_INITROM;
	diag_load_initrom = false;
	memset(fm7_mainmem_initrom, 0xff, 0x2000 * sizeof(uint8_t));

	if(read_bios(_T(ROM_FM77AV_INITIATOR), fm7_mainmem_initrom, 0x2000) >= 0x2000) diag_load_initrom = true;
	this->out_debug_log(_T("77AV INITIATOR ROM READING : %s"), diag_load_initrom ? "OK" : "NG");

	if(read_bios(_T(ROM_FM7_BOOT_MMR), fm77av_hidden_bootmmr, 0x200) < 0x1e0) {
		memcpy(fm77av_hidden_bootmmr, &fm7_mainmem_initrom[0x1a00], 0x200);
		diag_load_bootrom_mmr = true;
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
	this->out_debug_log(_T("BOOT ROM (2HD   mode) READING : %s"), diag_load_bootrom_2hd ? "OK" : "NG");
#elif defined(_FM8)
	if(diag_load_bootrom_bubble_128k) {
		this->out_debug_log(_T("BOOT ROM (BUBBLE 128K) READING : %s"), "OK");
	} else if(diag_load_bootrom_bubble) {
		this->out_debug_log(_T("BOOT ROM (BUBBLE  32K) READING : %s"), "OK");
	} else {
		this->out_debug_log(_T("BOOT ROM (BUBBLE  32K) READING : %s"), "NG");
	}		
	this->out_debug_log(_T("BOOT ROM (2HD   mode) READING : %s"), diag_load_bootrom_2hd ? "OK" : "NG");
#elif defined(_FM7) || defined(_FM7)
	if(diag_load_bootrom_bubble) {
		this->out_debug_log(_T("BOOT ROM (BUBBLE mode) READING : %s"), "OK");
	} else {
		this->out_debug_log(_T("BOOT ROM (BUBBLE mode) READING : %s"), "NG");
	}
#else // FM77AV*
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
	if(read_bios(_T(ROM_FM7_FBASICV30L20), fm7_mainmem_basicrom, 0x7c00) == 0x7c00) {
		diag_load_basicrom = true;
	} else if(read_bios(_T(ROM_FM7_FBASICV30L10), fm7_mainmem_basicrom, 0x7c00) == 0x7c00) {
		diag_load_basicrom = true;
	} else if(read_bios(_T(ROM_FM7_FBASICV30L00), fm7_mainmem_basicrom, 0x7c00) == 0x7c00) {
		diag_load_basicrom = true;
	} else if(read_bios(_T(ROM_FM7_FBASICV30), fm7_mainmem_basicrom, 0x7c00) == 0x7c00) {
		diag_load_basicrom = true;
	}
   
#else // FM8
	if(read_bios(_T(ROM_FM8_FBASICV10), fm7_mainmem_basicrom, 0x7c00) == 0x7c00) diag_load_basicrom = true;
#endif	
	this->out_debug_log(_T("BASIC ROM READING : %s"), diag_load_basicrom ? "OK" : "NG");
   
	i = FM7_MAINMEM_BIOSWORK;
	memset(fm7_mainmem_bioswork, 0x00, 0x80 * sizeof(uint8_t));
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	config.dipswitch = config.dipswitch | FM7_DIPSW_DICTROM_AV;
#endif
#if defined(CAPABLE_DICTROM)
	diag_load_dictrom = false;
	i = FM7_MAINMEM_DICTROM;
	memset(fm7_mainmem_dictrom, 0xff, 0x40000 * sizeof(uint8_t));
	memset(fm7_mainmem_learndata, 0x00, 0x2000 * sizeof(uint8_t));
	if((config.dipswitch & FM7_DIPSW_DICTROM_AV) != 0) {
		if(read_bios(_T(ROM_FM77AV_DICTIONARY), fm7_mainmem_dictrom, 0x40000) == 0x40000) diag_load_dictrom = true;
		this->out_debug_log(_T("DICTIONARY ROM READING : %s"), diag_load_dictrom ? "OK" : "NG");
		dictrom_connected = diag_load_dictrom;
	
		i = FM7_MAINMEM_BACKUPED_RAM;
		diag_load_learndata = false;
		if(read_bios(_T(RAM_FM77AV_DIC_BACKUP), fm7_mainmem_learndata, 0x2000) == 0x2000) diag_load_learndata = true;
		this->out_debug_log(_T("DICTIONARY BACKUPED RAM READING : %s"), diag_load_learndata ? "OK" : "NG");
		if(!diag_load_learndata) write_bios(_T("USERDIC.DAT"), fm7_mainmem_learndata, 0x2000);
	} else {
		this->out_debug_log(_T("LOADING FROM DICTIONARY CARD IS CANCELLED."));
		dictrom_connected = false;
		diag_load_dictrom = false;
		diag_load_learndata = false;
	}		
#endif
	
 	i = FM7_MAINMEM_77AV40_EXTRAROM;
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	diag_load_extrarom = false;
	memset(fm7_mainmem_extrarom, 0xff, sizeof(fm7_mainmem_extrarom));
	if(read_bios(_T(ROM_FM77AV40EX_EXTSUB), fm7_mainmem_extrarom, 0xc000) == 0xc000) diag_load_extrarom = true;
	this->out_debug_log(_T("AV40SX/EX EXTRA ROM READING : %s"), diag_load_extrarom ? "OK" : "NG");
	//register_event(this, EVENT_FM7_MAINMEM_DRAM_REFRESH, 13.02, true, &event_refresh); // OK?
#endif
	init_data_table();
	update_all_mmr_jumptable();
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
			data_table[paddr].read_func  = NULL;
			data_table[paddr].write_func = NULL;
		}
		for(addr = 0x40000 + extram_pages * 0x10000; addr < 0x100000; addr += 0x80) {
			paddr = addr >> 7;
			data_table[paddr].read_data  = NULL;
			data_table[paddr].write_data = NULL;
			data_table[paddr].read_func  = NULL;
			data_table[paddr].write_func = NULL;
		}
	}
# endif
#elif defined(_FM77_VARIANTS)
	int pages = extram_pages;
	if((pages > 0) && (pages < 4) && (fm7_mainmem_extram != NULL)) {
		for(addr = 0x00000; addr < (extram_pages * 0x10000) ; addr += 0x80) { // Thanks to Ryu Takegami
			paddr = addr >> 7;
			data_table[paddr].read_data  = &fm7_mainmem_extram[addr];
			data_table[paddr].write_data = &fm7_mainmem_extram[addr];
			data_table[paddr].read_func  = NULL;
			data_table[paddr].write_func = NULL;
		}
		if(extram_pages < 3) {
			for(addr = extram_pages * 0x10000; addr < 0x30000; addr += 0x80) {
				paddr = addr >> 7;
				data_table[paddr].read_data  = NULL;
				data_table[paddr].write_data = NULL;
				data_table[paddr].read_func  = NULL;
				data_table[paddr].write_func = NULL;
			}
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

