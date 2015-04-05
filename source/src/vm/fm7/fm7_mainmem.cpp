/*
 * Main memory without MMR for FM-7 [FM7_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */

#include "fm7_mainmem.h"


void FM7_MAINMEM::reset()
{
   	waitfactor = 0;
	waitcount = 0;
	ioaccess_wait = false;
	sub_halted = false;
	first_pass = true;
	flag_debug = false;
#if defined(_FM77AV_VARIANTS)	
	if((config.boot_mode & 3) == 0) {
		memcpy(fm7_bootram, &fm7_mainmem_initrom[0x1800], 0x200 * sizeof(uint8));
	} else {
		memcpy(fm7_bootram, &fm7_mainmem_initrom[0x1a00], 0x200 * sizeof(uint8));
	}
	fm7_bootram[0x1fe] = 0xfe; // Set reset vector.
	fm7_bootram[0x1ff] = 0x00; //
#endif	
}



void FM7_MAINMEM::wait()
{
	int waitfactor; // If MMR of TWR enabled, factor = 3.
			    // If memory, factor = 2?
	if(mainio->read_data8(FM7_MAINIO_CLOCKMODE) == FM7_MAINCLOCK_SLOW) return;
#ifdef HAS_MMR
	if(!ioaccess_wait) {
		waitfactor = 2;
		ioaccess_wait = true;
	} else { // Not MMR, TWR or enabled FAST MMR mode
		waitfactor = 3; // If(MMR or TWR) and NOT FAST MMR factor = 3, else factor = 2
		if((mainio->read_data8(FM7_MAINIO_FASTMMR_ENABLED) != 0)) waitfactor = 2;
		ioaccess_wait = false;
	} 
	if((mainio->read_data8(FM7_MAINIO_WINDOW_ENABLED) == 0) &&
	   (mainio->read_data8(FM7_MAINIO_MMR_ENABLED) == 0)) waitfactor = 2;
#else
	waitfactor = 2;
#endif	  
	if(waitfactor <= 0) return;
	waitcount++;
	if(waitcount >= waitfactor) {
		if(maincpu != NULL) maincpu->set_extra_clock(1);
		waitcount = 0;
	}
}


int FM7_MAINMEM::window_convert(uint32 addr, uint32 *realaddr)
{
	uint32 raddr = addr;
#ifdef HAS_MMR
	if((addr < 0x8000) && (addr >= 0x7c00)) {
		raddr = ((mainio->read_data8(FM7_MAINIO_WINDOW_OFFSET) * 256) + addr) & 0xffff;
		*realaddr = raddr;
#ifdef _FM77AV_VARIANTS
		//printf("TWR hit %04x -> %04x\n", addr, raddr);
		return FM7_MAINMEM_AV_PAGE0; // 0x00000 - 0x0ffff
#else // FM77(L4 or others)
		*realaddr |= 0x20000;
		return FM7_MAINMEM_EXTRAM; // 0x20000 - 0x2ffff
#endif
	}
	// Window not hit.
#endif
	return -1;
}

int FM7_MAINMEM::mmr_convert(uint32 addr, uint32 *realaddr)
{
	uint32 raddr = 0;
	uint32  mmr_segment;
	uint32  mmr_bank;
	
	//#ifdef _FM77AV_VARIANTS   
#ifdef HAS_MMR
	//mmr_segment = mainio->read_data8(FM7_MAINIO_MMR_SEGMENT);
	mmr_bank = mainio->read_data8(FM7_MAINIO_MMR_BANK + ((addr >> 12) & 0x000f));
	// Out of EXTRAM : 77AV20/40.
	
#if !defined(_FM77AV_VARIANTS)
	if(addr >= 0xfc00) return -1;
	mmr_bank = mmr_bank & 0x3f;
#else
	if(addr >= 0xfc00) return -1;
	mmr_bank = mmr_bank & 0x3f;
	//	mmr_bank &= 0xff;
#endif
	// Reallocated by MMR
	raddr = addr & 0x0fff;
	// Bank 3x : Standard memories.
	if((mmr_bank < 0x3f) && (mmr_bank >= 0x30)) {
		raddr = ((mmr_bank << 12) | raddr) & 0xffff;
		return nonmmr_convert(raddr, realaddr);
	}
  
#ifdef _FM77AV_VARIANTS
	if(mmr_bank == 0x3f) {
		if((raddr >= 0xd80) && (raddr <= 0xd97)) { // MMR AREA
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		} else {
			raddr = raddr | 0xf000;
			//printf("HIT: %04x\n", raddr);
			return nonmmr_convert(raddr, realaddr); // Access I/O, Bootrom, even via MMR.
		}
	}
#else
	if((mmr_bank == 0x3f) && (addr >= 0xc00) && (addr < 0xe00)) {
		if(mainio->read_data8(FM7_MAINIO_IS_BASICROM) != 0) { // BASICROM enabled
			*realaddr = 0;
			return FM7_MAINMEM_ZERO;
		} else {
			*realaddr = addr & 0x1ff;
			return FM7_MAINMEM_SHADOWRAM;
		}
	}
#endif
	
#ifdef _FM77AV_VARIANTS
	if((mmr_bank & 0xf0) == 0x00) { // PAGE 0
		*realaddr = ((mmr_bank << 12) | raddr) & 0xffff;
		return FM7_MAINMEM_AV_PAGE0;
	}
	if((mmr_bank & 0xf0) == 0x10) { // PAGE 1
		*realaddr = ((mmr_bank << 12) | raddr) & 0xffff;
		return FM7_MAINMEM_AV_DIRECTACCESS;
	}
	if((mmr_bank & 0xf0) == 0x20) { // PAGE 2
		if(use_page2_extram) {
			*realaddr = ((mmr_bank << 12) | raddr) & 0xffff;
			return FM7_MAINMEM_AV_PAGE2;
		}
       
#if 0
	        uint32 dbank = mainio->read_data8(FM7_MAINIO_EXTBANK);
		switch(mmr_bank) {
	  		case 0x28:
	  		case 0x29: // Backuped RAM
				if(((dbank & 0x80) != 0) && (dictrom_connected)){ // Battery backuped RAM
					raddr =  raddr & 0x1ff;
					*realaddr = raddr;
					return FM7_MAINMEM_BACKUPED_RAM;
				}
				break;
			case 0x2e:
				if(((dbank & 0x40) != 0) && (dictrom_connected)) { // Dictionary ROM
					dbank = dbank & 0x3f;
					uint32 extrom = mainio->read_data8(FM7_MAINIO_EXTROM) & 0x80;
					if(extrom == 0) { // Dictionary selected.
						dbank = dbank << 12;
						*realaddr = raddr | dbank;
						return FM7_MAINMEM_DICTROM;
					} else if(dbank <= 0x1f) { // KANJI
						*realaddr = (dbank << 12) | raddr;
						return FM7_MAINMEM_KANJI_LEVEL1;
					} else if(dbank <= 0x37) { 
						dbank = dbank << 12;
						*realaddr = (dbank - 0x20000) | raddr;
						return FM7_MAINMEM_77AV40_EXTRAROM;
					} else if(dbank <= 0x3f) {
					  	raddr = ((dbank << 12) - 0x30000) | raddr;
						if((raddr >= 0xffe0) || (raddr < 0xfd00)) { 
							return nonmmr_convert(raddr, realaddr);
						} else if((raddr >= 0xfe00) || (raddr < 0xffe0)) {
							*realaddr = raddr - 0xfe00;
							return FM7_MAINMEM_BOOTROM_DOS;
						}
						*realaddr = raddr + 0x10000;
						return FM7_MAINMEM_77AV40_EXTRAROM;
					}
				}
				break;
		}
		*realaddr = (raddr | (mmr_bank << 12)) & 0x0ffff;
		return FM7_MAINMEM_AV_PAGE2;
#else
		*realaddr = (raddr | (mmr_bank << 12)) & 0x0ffff;
		return FM7_MAINMEM_NULL;

#endif
	  	// RAM
	}
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV20)
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
#endif
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
#endif
	return -1;
}

int FM7_MAINMEM::nonmmr_convert(uint32 addr, uint32 *realaddr)
{
	addr &= 0x0ffff;
#ifdef _FM77AV_VARIANTS
	if(mainio->read_data8(FM7_MAINIO_INITROM_ENABLED) != 0) {
		if((addr >= 0x6000) && (addr < 0x8000)) {
			*realaddr = addr - 0x6000;
			return FM7_MAINMEM_INITROM;
		}
		if(addr >= 0xfffe) {
		  //printf("HIT %02x\n", read_table[FM7_MAINMEM_INITROM].memory[addr - 0xe000]);
			*realaddr = addr - 0xe000;
			return FM7_MAINMEM_INITROM;
		}
	}
#endif	

	if(addr < 0x8000) {
		*realaddr = addr;
 		return FM7_MAINMEM_OMOTE;
	} else if(addr < 0xfc00) {
		*realaddr = addr - 0x8000;
		if(mainio->read_data8(FM7_MAINIO_READ_FD0F) != 0) {
			return FM7_MAINMEM_BASICROM;
		}
		return FM7_MAINMEM_URA;
	} else 	if(addr < 0xfc80) {
		*realaddr = addr - 0xfc00;
		return FM7_MAINMEM_BIOSWORK;
	}else if(addr < 0xfd00) {
		*realaddr = addr - 0xfc80;
		return FM7_MAINMEM_SHAREDRAM;
	} else if(addr < 0xfe00) {
		wait();
		*realaddr = addr - 0xfd00;
		return FM7_MAINMEM_MMIO;
	}else if(addr < 0xffe0){
		wait();
		*realaddr = addr - 0xfe00;
#if defined(_FM77AV_VARIANTS)
		return FM7_MAINMEM_BOOTROM_RAM;
#else
		switch(mainio->read_data8(FM7_MAINIO_BOOTMODE)) {
			case 0:
				return FM7_MAINMEM_BOOTROM_BAS;
				break;
			case 1:
			  //printf("BOOT_DOS ADDR=%04x\n", addr);
				return FM7_MAINMEM_BOOTROM_DOS;
				break;
			case 2:
				return FM7_MAINMEM_BOOTROM_MMR;
				break;
			case 4:
				return FM7_MAINMEM_BOOTROM_RAM;
				break;
			default:
				return FM7_MAINMEM_BOOTROM_BAS; // Really?
				break;
		}
#endif
	} else if(addr < 0xfffe) { // VECTOR
		*realaddr = addr - 0xffe0;
		return FM7_MAINMEM_VECTOR;
	} else if(addr < 0x10000) {
		*realaddr = addr - 0xfffe;
		return FM7_MAINMEM_RESET_VECTOR;
	}
   
	emu->out_debug_log("Main: Over run ADDR = %08x\n", addr);
	*realaddr = addr;
	return FM7_MAINMEM_NULL;
}
     
int FM7_MAINMEM::getbank(uint32 addr, uint32 *realaddr)
{
	if(realaddr == NULL) return FM7_MAINMEM_NULL; // Not effect.
	addr = addr & 0xffff;
#ifdef HAS_MMR
	if(mainio->read_data8(FM7_MAINIO_WINDOW_ENABLED) != 0) {
		int stat;
		uint32 raddr;
		stat = window_convert(addr, &raddr);
		//if(stat >= 0) printf("WINDOW CONVERT: %04x to %04x, bank = %02x\n", addr, raddr, stat);
		if(stat >= 0) {
			*realaddr = raddr;
			return stat;
		}
	}
	if(mainio->read_data8(FM7_MAINIO_MMR_ENABLED) != 0) {
		int stat;
		uint32 raddr;
		stat = mmr_convert(addr, &raddr);
		if(stat >= 0) {
		  //printf("MMR CONVERT: %04x to %05x, bank = %02x\n", addr, raddr, stat);
			*realaddr = raddr;
			return stat;
		}
	}
#endif
	// NOT MMR.
	return nonmmr_convert(addr, realaddr);
}

void FM7_MAINMEM::write_signal(int sigid, uint32 data, uint32 mask)
{
	bool flag = ((data & mask) != 0);
	switch(sigid) {
		case SIG_FM7_SUB_HALT:
			sub_halted = flag;
			break;
	}
}


uint32 FM7_MAINMEM::read_data8(uint32 addr)
{
	uint32 ret;
	uint32 realaddr;
	int bank;

	bank = getbank(addr, &realaddr);
	if(bank < 0) {
		emu->out_debug_log("Illegal BANK: ADDR = %04x\n", addr);
		return 0xff; // Illegal
	}
   
        if(bank == FM7_MAINMEM_SHAREDRAM) {
	   	if(!sub_halted) return 0xff; // Not halt
		return display->read_data8((realaddr & 0x7f) + 0xd380); // Okay?
	} else if(bank == FM7_MAINMEM_MMIO) {
		return mainio->read_data8(realaddr);
//		return mainio->read_data8(addr);
	}
#if defined(_FM77AV_VARIANTS)
	else if(bank == FM7_MAINMEM_AV_DIRECTACCESS) {
	  if(display->read_signal(SIG_DISPLAY_HALT) != 0) return 0xff; // Not halt
		//printf("READ MMR : %04x to %05x\n", addr, realaddr);
		return display->read_data8(realaddr); // Okay?
	}
#endif
	else if(read_table[bank].memory != NULL) {
	  //if(bank == FM7_MAINMEM_BOOTROM_RAM) printf("Boot RAM: ADDR=%04x data=%02x\n",
	  //					   addr, read_table[bank].memory[realaddr]);
		return read_table[bank].memory[realaddr];
	}
	return 0xff; // Dummy
}

void FM7_MAINMEM::write_data8(uint32 addr, uint32 data)
{
	uint32 ret;
	uint32 realaddr;
	int bank;
   
	bank = getbank(addr, &realaddr);
	if(bank < 0) {
		emu->out_debug_log("Illegal BANK: ADDR = %04x\n", addr);
		return; // Illegal
	}
   
        if(bank == FM7_MAINMEM_SHAREDRAM) {
       		if(!sub_halted) return; // Not halt
		display->write_data8((realaddr & 0x7f) + 0xd380, data); // Okay?
		return;
	} else if(bank == FM7_MAINMEM_MMIO) {
		mainio->write_data8(realaddr & 0x00ff, (uint8)data);
//		mainio->write_data8(addr, (uint8)data);
		return;
	}
#if defined(_FM77AV_VARIANTS)
	else if(bank == FM7_MAINMEM_AV_DIRECTACCESS) {
       		if(display->read_signal(SIG_DISPLAY_HALT) != 0) return; // Not halt
		//printf("WRITE MMR : %04x to %05x\n", addr, realaddr);
		display->write_data8(realaddr, data); // Okay?
		return;
	}
#endif
#if defined(HAS_MMR)	
	else if(bank == FM7_MAINMEM_BOOTROM_RAM) {
	  //printf("BOOTRAM : %d\n", mainio->read_data8(FM7_MAINIO_BOOTMODE));
		if(mainio->read_data8(FM7_MAINIO_BOOTMODE) != 4) return;
		write_table[bank].memory[realaddr] = (uint8)(data & 0x000000ff);
	}
#endif
       	else if(write_table[bank].memory != NULL) {
			write_table[bank].memory[realaddr] = (uint8)(data & 0x000000ff);
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
	s = emu->bios_path((_TCHAR *)name);
	if(s == NULL) return 0;
  
	if(!fio.Fopen(s, FILEIO_READ_BINARY)) return 0;
	blocks = fio.Fread(ptr, size, 1);
	fio.Fclose();

	return blocks * size;
}

uint32 FM7_MAINMEM::write_bios(const char *name, uint8 *ptr, uint32 size)
{
	FILEIO fio;
	uint32 blocks;
	_TCHAR *s;
  
	if((name == NULL) || (ptr == NULL))  return 0;
	s = emu->bios_path((_TCHAR *)name);
	if(s == NULL) return 0;
  
	fio.Fopen(s, FILEIO_WRITE_BINARY);
	blocks = fio.Fwrite(ptr, size, 1);
	fio.Fclose();

	return blocks * size;
}

FM7_MAINMEM::FM7_MAINMEM(VM* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
{
	int i;
	p_vm = parent_vm;
	p_emu = parent_emu;
#if !defined(_FM77AV_VARIANTS)
	for(i = 0; i < 4; i++) fm7_bootroms[i] = (uint8 *)malloc(0x200);
#endif	
	mainio = NULL;
	display = NULL;
	maincpu = NULL;
	kanjiclass1 = NULL;
	kanjiclass2 = NULL;
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV20)
	fm7_mainmem_extram = NULL;
	extram_pages = config.extram_pages;
	if(extram_pages > 12) extram_pages = 12;
#endif
#if defined(_FM77_VARIANTS)
	extram_pages = config.extram_pages;
	if(extram_pages > 3) extram_pages = 3;
#endif	
}

FM7_MAINMEM::~FM7_MAINMEM()
{
}

void FM7_MAINMEM::initialize(void)
{
	int i;
	diag_load_basicrom = false;
	diag_load_bootrom_bas = false;
	diag_load_bootrom_dos = false;
	diag_load_bootrom_mmr = false;
#if defined(_FM77AV_VARIANTS)
	dictrom_connected = false;
	use_page2_extram = ((config.dipswitch & FM7_DIPSW_EXTRAM_AV) != 0) ? true : false;
#endif
	// Initialize table
	// $0000-$7FFF
	memset(read_table, 0x00, sizeof(read_table));
	memset(write_table, 0x00, sizeof(write_table));
	i = FM7_MAINMEM_OMOTE;
	memset(fm7_mainmem_omote, 0x00, 0x8000 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_omote;
	write_table[i].memory = fm7_mainmem_omote;

	// $8000-$FBFF
	i = FM7_MAINMEM_URA;
	memset(fm7_mainmem_ura, 0x00, 0x7c00 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_ura;
	write_table[i].memory = fm7_mainmem_ura;

	
	i = FM7_MAINMEM_VECTOR;
	memset(fm7_mainmem_bootrom_vector, 0x00, 0x1e);
	read_table[i].memory = fm7_mainmem_bootrom_vector;
	write_table[i].memory = fm7_mainmem_bootrom_vector;
	
#if defined(CAPABLE_DICTROM)
	diag_load_dictrom = false;
	i = FM7_MAINMEM_DICTROM;
	memset(fm7_mainmem_extrarom, 0xff, 0x40000 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_dictrom;
	write_table[i].memory = NULL;
	if(read_bios("DICROM.ROM", fm7_mainmem_dictrom, 0x40000) == 0x40000) diag_load_dictrom = true;
	emu->out_debug_log("DICTIONARY ROM READING : %s\n", diag_load_dictrom ? "OK" : "NG");
	dictrom_connected = diag_load_dictrom;
	
	i = FM7_MAINMEM_BACKUPED_RAM;
	diag_load_learndata = false;
	memset(fm7_mainmem_learndata, 0x00, 0x2000 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_learndata;
	write_table[i].memory = fm7_mainmem_learndata;
	if(read_bios("USERDIC.DAT", read_table[i].memory, 0x2000) == 0x2000) diag_load_learndata = true;
	emu->out_debug_log("DICTIONARY ROM READING : %s\n", diag_load_learndata ? "OK" : "NG");
	if(!diag_load_learndata) write_bios("USERDIC.DAT", fm7_mainmem_learndata, 0x2000);
#endif
	
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV20)
	i = FM7_MAINMEM_77AV40_EXTRAROM;
	diag_load_extrarom = false;
	memset(fm7_mainmem_extrarom, 0xff, 0x20000 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_extrarom;
	write_table[i].memory = NULL;
	if(read_bios("EXTSUB.ROM", read_table[i].memory, 0xc000) >= 0xc000) diag_load_extrarom = true;
	emu->out_debug_log("AV40 EXTRA ROM READING : %s\n", diag_load_extrarom ? "OK" : "NG");
#endif
	
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV20) || (_FM77_VARIANTS)
	if(extram_pages > 0) {
		i = FM7_MAINMEM_EXTRAM;
		fm7_mainmem_extram = malloc(extram_pages * 0x10000);
		if(fm7_mainmem_extram != NULL) {
			memset(fm7_mainmem_extram, 0x00, extram_pages * 0x10000);
			read_table[i].memory = fm7_mainmem_extram;
			write_table[i].memory = fm7_mainmem_extram;
		}
	}
#endif	
#if !defined(_FM77AV_VARIANTS)	
	for(i = FM7_MAINMEM_BOOTROM_BAS; i <= FM7_MAINMEM_BOOTROM_EXTRA; i++) {
		 memset(fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS], 0xff, 0x200);
		 read_table[i].memory = fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS];
		 write_table[i].memory = NULL;
	}
#endif	
#if defined(_FM8)
	if(read_bios("BOOT_BAS8.ROM", fm7_bootroms[0], 0x200) >= 0x1e0) {
		diag_load_bootrom_bas = true;
	} else {
		diag_load_bootrom_bas = false;
	}
	if(read_bios("BOOT_DOS8.ROM", fm7_bootroms[1], 0x200) >= 0x1e0) {
		diag_load_bootrom_dos = true;
	} else {
		diag_load_bootrom_dos = false;
	}
	diag_load_bootrom_mmr = false;
# elif defined(_FM7) || defined(_FMNEW7)
	if(read_bios("BOOT_BAS.ROM", fm7_bootroms[0], 0x200) >= 0x1e0) {
		diag_load_bootrom_bas = true;
	} else {
		diag_load_bootrom_bas = false;
	}
	if(read_bios("BOOT_DOS.ROM", fm7_bootroms[1], 0x200) >= 0x1e0) {
		diag_load_bootrom_dos = true;
	} else {
		diag_load_bootrom_dos = false;
	}
	// FM-7/8
	diag_load_bootrom_mmr = false;
# elif defined(_FM77AV_VARIANTS)
	i = FM7_MAINMEM_AV_PAGE0;
	memset(fm7_mainmem_mmrbank_0, 0x00, 0x10000 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_mmrbank_0;
	write_table[i].memory = fm7_mainmem_mmrbank_0;
	
	i = FM7_MAINMEM_AV_PAGE2;
	memset(fm7_mainmem_mmrbank_0, 0x00, 0x10000 * sizeof(uint8));
	if(use_page2_extram) { 
		read_table[i].memory = fm7_mainmem_mmrbank_2;
		write_table[i].memory = fm7_mainmem_mmrbank_2;
	} else {
		read_table[i].memory = NULL;
		write_table[i].memory = NULL;
	}
	
	i = FM7_MAINMEM_INITROM;
	diag_load_initrom = false;
	memset(fm7_mainmem_initrom, 0xff, 0x2000 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_initrom;
	write_table[i].memory = NULL;
	if(read_bios("INITIATE.ROM", read_table[i].memory, 0x2000) >= 0x2000) diag_load_initrom = true;
	emu->out_debug_log("77AV INITIATOR ROM READING : %s\n", diag_load_initrom ? "OK" : "NG");

	read_table[FM7_MAINMEM_BOOTROM_BAS].memory = NULL; // Not connected.
	read_table[FM7_MAINMEM_BOOTROM_DOS].memory = NULL; // Not connected.
	read_table[FM7_MAINMEM_BOOTROM_MMR].memory = NULL; // Not connected.

	i = FM7_MAINMEM_BOOTROM_RAM;
	memset(fm7_bootram, 0x00, 0x200 * sizeof(uint8)); // RAM
	read_table[i].memory = fm7_bootram;
	write_table[i].memory = fm7_bootram;
	if(diag_load_initrom) diag_load_bootrom_bas = true;
	if(diag_load_initrom) diag_load_bootrom_dos = true;
	if((config.boot_mode & 0x03) == 0) {
		memcpy(fm7_bootram, &fm7_mainmem_initrom[0x1800], 0x200 * sizeof(uint8));
	} else {
		memcpy(fm7_bootram, &fm7_mainmem_initrom[0x1a00], 0x200 * sizeof(uint8));
	}
	fm7_bootram[0x1fe] = 0xfe; // Set reset vector.
	fm7_bootram[0x1ff] = 0x00; //
	// FM-7
#endif
	emu->out_debug_log("BOOT ROM (basic mode) READING : %s\n", diag_load_bootrom_bas ? "OK" : "NG");
	emu->out_debug_log("BOOT ROM (DOS   mode) READING : %s\n", diag_load_bootrom_dos ? "OK" : "NG");
#if defined(_FM77_VARIANTS)
	emu->out_debug_log("BOOT ROM (MMR   mode) READING : %s\n", diag_load_bootrom_mmr ? "OK" : "NG");
#endif

	i = FM7_MAINMEM_VECTOR;
	memset(fm7_mainmem_bootrom_vector, 0x00, 0x1e);
	read_table[i].memory = fm7_mainmem_bootrom_vector;
	write_table[i].memory = fm7_mainmem_bootrom_vector;

#if !defined(_FM77AV_VARIANTS)
	for(i = 0; i <= 3; i++) {
		uint8 *p = fm7_bootroms[i];
		p[0x1fe] = 0xfe; // Set reset vector.
		p[0x1ff] = 0x00; //
	}
#endif	
	i = FM7_MAINMEM_RESET_VECTOR;
	fm7_mainmem_reset_vector[0] = 0xfe;
	fm7_mainmem_reset_vector[1] = 0x00;
   
	read_table[i].memory = fm7_mainmem_reset_vector;
	write_table[i].memory = NULL;
   
	i = FM7_MAINMEM_BASICROM;
	memset(fm7_mainmem_basicrom, 0xff, 0x7c00 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_basicrom;
	write_table[i].dev = NULL;
	write_table[i].memory = NULL;
#if !defined(_FM8)
	if(read_bios("FBASIC30.ROM", fm7_mainmem_basicrom, 0x7c00) == 0x7c00) diag_load_basicrom = true;
#else // FM8
	if(read_bios("FBASIC10.ROM", fm7_mainmem_basicrom, 0x7c00) == 0x7c00) diag_load_basicrom = true;
#endif	
	emu->out_debug_log("BASIC ROM READING : %s\n", diag_load_basicrom ? "OK" : "NG");
   
	i = FM7_MAINMEM_BIOSWORK;
	memset(fm7_mainmem_bioswork, 0x00, 0x80 * sizeof(uint8));
	read_table[i].dev = NULL;
	read_table[i].memory = fm7_mainmem_bioswork;
	write_table[i].dev = NULL;
	write_table[i].memory = fm7_mainmem_bioswork;
}

void FM7_MAINMEM::release()
{
	int i;
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV20) || defined(_FM77_VARIANTS)
	if(fm7_mainmem_extram != NULL) free(fm7_mainmem_extram);
#endif  
#if !defined(_FM77AV_VARIANTS)
	for(i = 0; i < 4; i++) {
		if(fm7_bootroms[i] != NULL) free(fm7_bootroms[i]);
		fm7_bootroms[i] = NULL;
	}
#endif
	MEMORY::release();
}
