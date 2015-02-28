/*
 * Main memory without MMR for FM-7 [FM7_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */

#include "fm7_mainmem.h"

int FM7_MAINMEM::window_convert(uint32 addr, uint32 *realaddr)
{
	uint32 raddr = addr;
#ifdef HAS_MMR
	if((addr < 0x8000) && (addr >= 0x7c00) && (window_enabled)) {
		addr &= 0x03ff;
		raddr = ((window_offset << 8) + addr) & 0xffff;
		*realaddr = raddr;
#ifdef _FM77AV_VARIANTS
		return FM7_MAINMEM_MMRBANK_0; // 0x20000 - 0x2ffff
#else // FM77(L4 or others)
		return FM7_MAINMEM_MMRBANK_2; // 0x20000 - 0x2ffff
#endif
	}
	// Window not hit.
#endif
	return -1;
}

int FM7_MAINMEM::mmr_convert(uint32 addr, uint32 *realaddr)
{
	uint32 raddr = 0;
	uint8  mmr_segment;
	uint8  mmr_bank;
	
	if(addr >= 0xfc00) return -1;
	mmr_segment = mainio->read_signal(SIG_FM7MAINIO_MMR_SEGMENT);
	mmr_bank = mainio->read_signal(SIG_FM7MAINIO_MMR_BANK + mmr_segment * 16 + ((addr >> 12) & 0x000f));
	// Out of EXTRAM : 77AV20/40.
	
#if !defined(_FM77AV_VARIANTS)
	mmr_bank &= 0x3f;
#endif
	// Reallocated by MMR
	raddr = addr & 0x0fff;
	// Bank 3x : Standard memories.
	if((mmr_bank < 0x3f) && (mmr_bank >= 0x30)) {
		raddr = (((uint32)mmr_bank & 0x0f) << 12) | raddr;
		return nonmmr_convert(raddr, realaddr);
	}
  
#ifdef _FM77AV_VARIANTS
	if(mmr_bank == 0x3f) {
		if((raddr >= 0xd80) && (raddr <= 0xd97)) { // MMR AREA
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		}
	}
#else
	if((mmr_bank == 0x3f) && (addr >= 0xc00) && (addr < 0xe00)) {
		if(mainio->read_signal(SIG_FM7MAINIO_IS_BASICROM) != 0) { // BASICROM enabled
			*realaddr = 0;
			return FM7_MAINMEM_ZERO;
		} else {
			*realaddr = addr & 0x1ff;
			return FM7_MAINMEM_SHADOWRAM;
		}
	}
#endif
	
#ifdef _FM77AV_VARIANTS
	if((mmr_bank & 0xf0) == 0) { // PAGE 0
		*realaddr = (((uint32)mmr_bank & 0x0f) << 12) | raddr;
		return FM7_MAINIO_77AV_PAGE0;
	}
	if((mmr_bank & 0xf0) == 1) { // PAGE 1
		if(mainio->read_signal(SIG_FM7MAINIO_SUB_RUN) != 0) { // Subsystem is not halted.
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		} else { // Subsystem is halted.
			*realaddr = (((uint32)mmr_bank & 0x0f) << 12) | raddr;
			return FM7_MAIMEM_SUBSYSTEM;
		}
	}
	if((mmr_bank & 0xf0) == 2) { // PAGE 1
		if(dict_connected) {
			*realaddr = (((uint32)mmr_bank & 0x0f) << 12) | raddr;
			return FM7_MAIMEM_77AVDICTROM;
		} else {
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		}
	}
	
	if(extram_connected) { // PAGE 4-
		if((mmr_bank >> 4) >= (extram_pages + 4)) {
			*realaddr = 0;
			return FM7_MAINMEM_NULL; // $FF
		} else {
			raddr = ((uint32)mmr_bank << 12) | raddr;
			*realaddr = raddr;
			return FM7_MAINMEM_EXTRAM;
		}
	} else {
		if(mmr_bank >= 0x40) {
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		}
	}
#else // 77
	// page 0 or 1 or 2.
	if(extram_connected) {
		if((mmr_bank >> 4) >= extram_pages) {
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		} else { // EXTRAM Exists.
			raddr = (((uint32)mmr_bank << 12) & 0x3ffff) | raddr;
			*realaddr = raddr;
			return FM7_MAINMEM_EXTRAM;
		}
	}
#endif
  return -1;
}

int FM7_MAINMEM::nonmmr_convert(uint32 addr, uint32 *realaddr)
{
	if(addr < 0x8000) {
		*realaddr = addr - 0;
 		return FM7_MAINMEM_OMOTE;
	}
	if(addr < 0xfc00) {
		*realaddr = addr - 0x8000;
		if(mainio->get_rommode_fd0f() == true) return FM7_MAINMEM_BASICROM;
		return FM7_MAINMEM_URA;
	}
	if(addr < 0xfc80) {
		*realaddr = addr - 0xfc00;
		return FM7_MAINMEM_BIOSWORK;
	}
	if(addr < 0xfd00) {
		*realaddr = addr - 0xfc80;
		return FM7_MAINMEM_SHAREDRAM;
	}
	if(addr < 0xfe00) {
		mainio->wait();
		*realaddr = addr - 0xfd00;
		return FM7_MAINMEM_MMIO;
	}
	if(addr < 0xffe0) {
		if(addr < 0xffe0) mainio->wait();
		*realaddr = addr - 0xfe00;
		//if(mainio->get_boot_romram() != true) return FM7_MAINMEM_BOOTROM_RAM;
		switch(dipsw->get_boot_mode()) {
			case FM7_BOOTMODE_BASIC:
				return FM7_MAIMEM_BOOTROM_BAS;
				break;
			case FM7_BOOTMODE_DOS:
				return FM7_MAIMEM_BOOTROM_DOS;
				break;
			case FM7_BOOTMODE_ROM3:
				return FM7_MAIMEM_BOOTROM_ROM3;
				break;
			default:
				return FM7_MAINMEM_BOOTROM_BAS; // Really?
				break;
		}
	}
	if(addr < 0xfffe) { // VECTOR
		*realaddr = addr - 0xffe0;
		return FM7_MAINMEM_VECTOR;
	}
	if(addr < 0x10000) {
		mainio->wait();
		*realaddr = addr - 0xfffe;
		return FM7_MAINMEM_VECTOR_RESET;
	}
	realaddr = addr;
	return FM7_MAINMEM_NULL;
}
     
int FM7_MAINMEM::getbank(uint32 addr, uint32 *realaddr)
{
	if(realaddr == NULL) return FM7_MAINMEM_NULL; // Not effect.
	addr = addr & 0xffff;
#ifdef HAS_MMR
	if(window_enabled) {
		int stat;
		uint32 raddr;
		stat = window_convert(addr, &raddr);
		if(stat >= 0) {
			*realaddr = raddr;
			return stat;
		}
	}
	if(mmr_enabled) {
		int stat;
		uint32 raddr;
		stat = mmr_convert(addr, &raddr);
		if(stat >= 0) {
			*realaddr = raddr;
			return stat;
		}
	}
#endif
	// NOT MMR.
	return nonmmr_convert(addr, realaddr);
}

uint32 FM7_MAINMEM::read_data8(uint32 addr)
{
	uint32 ret;
	uint32 realaddr;
	int bank;

	bank = getbank(addr, &realaddr);
	if(bank < 0) return 0xff; // Illegal

        if(bank == FM7_MAINMEM_SHAREDRAM) {
	   	if(submem->read_signal(SIG_SUBCPU_HALT) != 0) return 0xff; // Not halt
		return submem->read_data8(realaddr + 0xd000); // Okay?
	}
	if(read_table[bank].dev != NULL) {
		return read_table[bank].dev->read_data8(realaddr);
	} else {
        	if(read_table[bank].memory != NULL) {
	   		return read_table[bank].memory[realaddr];
		}
		return 0xff; // Dummy
	}
}

void FM7_MAINMEM::write_data8(uint32 addr, uint32 data)
{
	uint32 ret;
	uint32 realaddr;
	int bank;
   
	bank = getbank(addr, &realaddr);
	if(bank < 0) return; // Illegal
   
        if(bank == FM7_MAINMEM_SHAREDRAM) {
       		if(submem->read_signal(SIG_SUBCPU_HALT) != 0) return; // Not halt
		submem->write_data8(realaddr + 0xd000, data); // Okay?
		return;
	}
	if(write_table[bank].dev != NULL) {
		write_table[bank].dev->write_data8(realaddr, data);
	} else {
        	if(write_table[bank].memory != NULL) {
		  write_table[bank].memory[realaddr] = (uint8)(data & 0x000000ff);
		}
	}
}

// Read / Write data(s) as big endian.
uint32 FM7_MAINMEM::read_data16(uint32 addr)
{
	uint32 hi, lo;
	uint32 val;
   
	hi = read_data8(addr) & 0xff;
	lo = read_data8(addr + 1) & 0xff;
   
	val = hi * 256 + lo;
	return val;
}

uint32 FM7_MAINMEM::read_data32(uint32 addr)
{
	uint32 ah, a2, a3, al;
	uint32 val;
   
	ah = read_data8(addr) & 0xff;
	a2 = read_data8(addr + 1) & 0xff;
	a3 = read_data8(addr + 2) & 0xff;
	al = read_data8(addr + 3) & 0xff;
   
	val = ah * (65536 * 256) + a2 * 65536 + a3 * 256 + al;
	return val;
}

void FM7_MAINMEM::write_data16(uint32 addr, uint32 data)
{
	uint32 d = data;
   
	write_data8(addr + 1, d & 0xff);
	d = d / 256;
	write_data8(addr + 0, d & 0xff);
}

void FM7_MAINMEM::write_data32(uint32 addr, uint32 data)
{
	uint32 d = data;
   
	write_data8(addr + 3, d & 0xff);
	d = d / 256;
	write_data8(addr + 2, d & 0xff);
	d = d / 256;
	write_data8(addr + 1, d & 0xff);
	d = d / 256;
	write_data8(addr + 0, d & 0xff);
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

uint32 FM7_MAINMEM::read_bios(const char *name, uint8 *ptr, uint32 size)
{
	FILEIO fio;
	uint32 blocks;
	_TCHAR *s;
  
	if((name == NULL) || (ptr == NULL))  return 0;
	s = bios_path(name);
	if(s == NULL) return 0;
  
	if(!fio.Fopen(s, FILEIO_READ_BINARY)) return 0;
	blocks = fio.Fread(s, size, 1);
	fio.Fclose();

	return blocks * size;
}

FM7_MAINMEM::FM7_MAINMEM(VM* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
{
	int i;
	p_vm = parent_vm;
	p_emu = parent_emu;
	for(i = 0; i < 4; i++) fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS] = malloc(0x200);
}

FM7_MAINMEM::~FM7_MAINMEM()
{
	int i;
	for(i = 0; i < 4; i++) {
		if(fm7_bootroms[i] != NULL) free(fm7_bootroms[i]);
		fm7_bootroms[i] = NULL;
	}
	delete mainio;
}

void FM7_MAINMEM::initialize(void)
{
	int i;
	// Initialize table
	// $0000-$7FFF
	i = FM7_MAINMEM_OMOTE;
	memset(fm7_maimem_omote, 0x00, 0x8000 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_omote;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_omote;
	

	// $8000-$FBFF
	i = FM7_MAINMEM_URA
	memset(fm7_maimem_ura, 0x00, 0x7c00 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_ura;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_ura;
	
	
	i = FM7_MAINMEM_BASICROM;
	memset(fm7_maimem_basicrom, 0xff, 0x7c00 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_basicrom;
	write_table[i].dev = NULL;
	write_table[i].memory = write_dummy;
	if(read_bios("FBASIC30.ROM", read_table[i].memory, 0x7c00) == 0x7c00) diag_load_basicrom = true;
	
	i = FM7_MAINMEM_BIOSWORK;
	memset(fm7_maimem_bioswork, 0x00, 0x80 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_bioswork;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_bioswork;
	
	i = FM7_MAINMEM_SHAREDRAM;
	read_table[i].dev = sharedram;
	read_table[i].memory = NULL;
	write_table[i].dev = sharedram;
	write_table[i].memory = NULL;
	
	i = FM7_MAINMEM_MMIO;
	read_table[i].dev = mainio;
	read_table[i].memory = NULL;
	write_table[i].dev = mainio;
	write_table[i].memory = NULL;

	for(i = FM7_MAINMEM_BOOTROM_BAS; i <= FM7_MAINMEM_BOOTROM_RAM; i++) {
	    memset(fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS], 0x00, 0x200);
	    read_table[i].dev = NULL;
	    read_table[i].memory = fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS];
	    write_table[i].dev = NULL;
	    write_table[i].memory = write_dummy;
	}
	if(read_bios("BOOT_BAS.ROM", read_table[FM7_MAINMEM_BOOTROM_BAS].memory, 0x200) == 0x200) diag_load_bootrom_bas = true;
	if(read_bios("BOOT_DOS.ROM", read_table[FM7_MAINMEM_BOOTROM_DOS].memory, 0x200) == 0x200) diag_load_bootrom_dos = true;
	write_table[FM7_BAINMEM_BOOTROM_RAM].memory = read_table[FM7_BAINMEM_BOOTROM_RAM].memory; // Write enabled on BOOTRAM.
	
	i = FM7_MAINMEM_VECTOR;
	memset(fm7_mainmem_bootrom_vector, 0x00, 0x1e);
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_bootrom_vector;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_bootrom_vector;
 
	i = FM7_MAINMEM_VECTOR_RESET;
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_resetvector;
	write_table[i].dev = NULL;
	write_table[i].memory = write_dummy;
}
