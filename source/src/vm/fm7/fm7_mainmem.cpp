/*
 * Main memory without MMR for FM-7 [FM7_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */
#include "vm.h"
#include "emu.h"
#include "fm7_mainmem.h"

void FM7_MAINMEM::reset()
{
   	waitfactor = 0;
	waitcount = 0;
	ioaccess_wait = false;
	//sub_halted = (display->read_signal(SIG_DISPLAY_HALT) == 0) ? false : true;
	sub_halted = false;
	memset(fm7_mainmem_bootrom_vector, 0x00, 0x10); // Clear without vector

#if defined(_FM77AV_VARIANTS)
	memset(fm7_bootram, 0x00, 0x1f0);
	initiator_enabled = true;
	boot_ram_write = true;
#elif defined(_FM77_VARIANTS)
	boot_ram_write = false;
#endif	
	bootmode = config.boot_mode & 3;
#if defined(HAS_MMR)
	if((config.dipswitch & FM7_DIPSW_EXTRAM) != 0) {
		extram_connected = true;
	} else {
		extram_connected = false;
	}
#endif
#if defined(_FM77AV_VARIANTS)
	if(dictrom_connected) {
		use_page2_extram = true;
	} else {
		use_page2_extram = ((config.dipswitch & FM7_DIPSW_EXTRAM_AV) != 0) ? true : false;
	}
#endif   
#ifdef HAS_MMR
	mmr_extend = false;
	mmr_segment = 0;
	window_offset = 0;
	mmr_enabled = false;
	mmr_fast = false;
	window_enabled = false;
	window_fast = false;
	refresh_fast = false;
#endif
	if((bootmode & 0x03) == 0) { // IF BASIC BOOT THEN ROM
		basicrom_fd0f = true;
	} else { // ELSE RAM
		basicrom_fd0f = false;
	}
	clockmode = (config.cpu_type == 0) ? true : false;
	is_basicrom = ((bootmode & 0x03) == 0) ? true : false;
	setclock(clockmode ? 0 : 1);
	setup_table_extram();
	maincpu->reset();
}

void FM7_MAINMEM::setclock(int mode)
{
	uint32_t clock = MAINCLOCK_SLOW;
	if(mode == 1) { // SLOW
		clock = MAINCLOCK_SLOW; // Temporally
#if defined(HAS_MMR)		
		if(!mmr_fast && !window_fast) {
			if(refresh_fast) {
				if(mmr_enabled || window_enabled) {
					clock = (uint32_t)((double)clock * 1.089);
				} else {
					clock = (uint32_t)((double)clock * 1.086);
				}					
			}
		}
#endif		
	} else {
#if defined(HAS_MMR)
		if(window_enabled) {
			if(window_fast) {
				clock = MAINCLOCK_FAST_MMR;
			} else {
				clock = MAINCLOCK_MMR;
			}
		} else if(mmr_enabled) {
			if(mmr_fast) {
				clock = MAINCLOCK_FAST_MMR;
			} else {
				clock = MAINCLOCK_MMR;
			}
		} else {
			clock = MAINCLOCK_NORMAL;
		}
		if(!mmr_fast && !window_fast) {
			if(refresh_fast) {
				if(mmr_enabled || window_enabled) {
					clock = (uint32_t)((double)clock * 1.089);
				} else {
					clock = (uint32_t)((double)clock * 1.086);
				}					
			}
		}
#else
		clock = MAINCLOCK_NORMAL;
#endif				
	}
	p_vm->set_cpu_clock(this->maincpu, clock);
}
		

void FM7_MAINMEM::wait()
{
	int waitfactor; // If MMR of TWR enabled, factor = 3.
			    // If memory, factor = 2?
	if(!clockmode) return; // SLOW
#ifdef HAS_MMR
    //if(!mmr_fast && !window_fast && (window_enabled || mmr_enabled)) waitfactor = 2;
	if(!ioaccess_wait) {
		waitfactor = 2;
		ioaccess_wait = true;
	} else { // Not MMR, TWR or enabled FAST MMR mode
		waitfactor = 3; // If(MMR or TWR) and NOT FAST MMR factor = 3, else factor = 2
		if(mmr_fast) waitfactor = 2;
		ioaccess_wait = false;
	} 
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

int FM7_MAINMEM::check_extrom(uint32_t raddr, uint32_t *realaddr)
{
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(extrom_bank) { // Extra ROM selected.
		uint32_t dbank = extcard_bank & 0x3f;
		if(dbank < 0x20) { // KANJI
			if((dbank == 0x07) || (dbank == 0x06)) {
				// NOT KANJI AS IS.Thanks Ryu.
				*realaddr = raddr & 0x01;
				return FM7_MAINMEM_KANJI_DUMMYADDR;
			}
			*realaddr = (dbank << 12) | raddr;
			return FM7_MAINMEM_KANJI_LEVEL1;
		} else if(dbank < 0x2c) {
			raddr = ((dbank << 12) - 0x20000) | raddr;
			*realaddr = raddr;
			return FM7_MAINMEM_77AV40_EXTRAROM;
		} else if(dbank < 0x30) {
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		} else {
			raddr = ((dbank << 12) - 0x30000) | raddr;
			if((raddr >= 0x8000)  && (raddr < 0xfc00)) {
				*realaddr = raddr - 0x8000;
				return FM7_MAINMEM_BASICROM;
			} else if((raddr >= 0xfe00) && (raddr < 0xffe0)) {
				*realaddr = raddr - 0xfe00;
				return FM7_MAINMEM_BOOTROM_MMR;
			} else if(raddr >= 0xfffe) {
				*realaddr = raddr - 0xfffe;
				return FM7_MAINMEM_RESET_VECTOR;
			}
			//*realaddr = raddr + 0x10000;
			//return FM7_MAINMEM_77AV40_EXTRAROM;
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		}
	}
#endif	
	return -1;
}
   
uint8_t FM7_MAINMEM::read_page2_extcard(uint32_t addr, bool dmamode)
{
#if defined(USE_MMR)
	uint32_t raddr = addr & 0x0fff;
	uint32_t saddr = addr & 0xffff;
	uint32_t realaddr;
	uint32_t mmr_bank;
	bool write_state = false;
	if(!mmr_extend) {
		mmr_bank = mmr_map_data[(addr >> 12) & 0x000f | ((segment & 0x03) << 4)] & 0x003f;
	} else {
		mmr_bank = mmr_map_data[(addr >> 12) & 0x000f | ((segment & 0x07) << 4)] & 0x007f;
	}		
	// PAGE 2
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if((mmr_bank == 0x2e) && (!write_state)) {
		if(extrom_bank) { // Extra ROM selected.
			uint32_t dbank = extcard_bank & 0x3f;
			if(dbank < 0x20) { // KANJI
				if((dbank == 0x07) || (dbank == 0x06)) {
					// NOT KANJI AS IS.Thanks Ryu.
					realaddr = raddr & 0x01;
					return (uint8_t)realaddr;
				}
				realaddr = (dbank << 12) | raddr;
				return (uint8_t)kanjiclass1->read_data8(KANJIROM_DIRECTADDR + realaddr);
			} else if(dbank < 0x2c) {
				raddr = ((dbank << 12) - 0x20000) | raddr;
				return fm7_mainmem_extrarom[raddr];
			} else if(dbank < 0x30) {
				return 0xff;
			} else {
				raddr = ((dbank << 12) - 0x30000) | raddr;
				if((raddr >= 0x8000)  && (raddr < 0xfc00)) {
					realaddr = raddr - 0x8000;
					return fm7_mainmem_basicrom[realaddr];
				} else if((raddr >= 0xfe00) && (raddr < 0xffe0)) {
					realaddr = raddr - 0xfe00;
					return fm77av_hidden_bootmmr[realaddr];
				} else if(raddr >= 0xfffe) {
					realaddr = raddr - 0xfffe;
					return fm7_mainmem_reset_vector[realaddr];
				}
				return 0xff;
			}
			return 0xff;
		}
	}
#endif	

#if defined(CAPABLE_DICTROM)
	if((mmr_bank == 0x2e) && (!write_state)) {
		if(dictrom_connected && dictrom_enabled) { // Dictionary ROM
			uint32_t dbank = extcard_bank & 0x3f;
			realaddr = raddr | (dbank << 12);
			return fm7_mainmem_dictrom[realaddr];
		}
	}
	uint32_t bank = (addr >> 12) & 0x7f;
	switch(mmr_bank) {
	case 0x28:
	case 0x29: // Backuped RAM
		if(dictrom_connected && dictram_enabled){ // Battery backuped RAM
			realaddr =  addr & 0x1fff;
			return fm7_mainmem_learndata[realaddr];
		} else {
			return fm7_mainmem_mmrbank_2[addr & 0xffff];
		}
		break;
	default:
		return fm7_mainmem_mmrbank_2[addr & 0xffff];
		break;
	}
#  else
	if(use_page2_extram) {
		return fm7_mainmem_mmrbank_2[addr & 0xffff];
	} else {
		return 0xff;
	}
#  endif
# endif
	return 0xff;
}

void FM7_MAINMEM::write_page2_extcard(uint32_t addr, uint32_t data, bool dmamode)
{
#if defined(USE_MMR)
	uint32_t raddr = addr & 0x0fff;
	uint32_t saddr = addr & 0xffff;
	uint32_t realaddr;
	uint32_t mmr_bank;
	bool write_state = false;
	if(!mmr_extend) {
		mmr_bank = mmr_map_data[(addr >> 12) & 0x000f | ((segment & 0x03) << 4)] & 0x003f;
	} else {
		mmr_bank = mmr_map_data[(addr >> 12) & 0x000f | ((segment & 0x07) << 4)] & 0x007f;
	}		
#if defined(CAPABLE_DICTROM)
	uint32_t bank = (addr >> 12) & 0x7f;
	switch(mmr_bank) {
	case 0x28:
	case 0x29: // Backuped RAM
		if(dictrom_connected && dictram_enabled){ // Battery backuped RAM
			realaddr =  addr & 0x1fff;
			fm7_mainmem_learndata[realaddr] = (uint8_t)data;
		} else {
			fm7_mainmem_mmrbank_2[addr & 0xffff] = (uint8_t)data;
		}
		return;
		break;
	default:
		fm7_mainmem_mmrbank_2[addr & 0xffff] = (uint8_t)data;
		return;
		break;
	}
#  else
	if(use_page2_extram) {
		fm7_mainmem_mmrbank_2[addr & 0xffff] = (uint8_t)data;
		return
	} else {
		return;
	}
#  endif
# endif
}


uint8_t FM7_MAINMEM::read_page3_basicrom_uraram(uint32_t addr, bool dmamode)
{
	uint32_t raddr = addr & 0x7fff;
	//if(raddr >= 0x7c00) return 0xff;
	if(basicrom_fd0f) {
		return fm7_mainmem_basicrom[raddr];
	}
	return fm7_mainmem_ura[raddr];
}

void FM7_MAINMEM::write_page3_basicrom_uraram(uint32_t addr, uint32_t data, bool dmamode)
{
	uint32_t raddr = addr & 0x7fff;
	//if(raddr >= 0x7c00) return;
	if(basicrom_fd0f) {
		return;
	}
	fm7_mainmem_ura[raddr] = (uint8_t)data;;
}

uint8_t FM7_MAINMEM::read_page3_bootrom(uint32_t addr, bool dmamode)
{
	uint32_t raddr;
	if (addr < 0xffe0){
		wait();
		raddr = addr - 0xfe00;
#if defined(_FM77AV_VARIANTS)
		return fm7_bootram[raddr];
#else
		switch(bootmode) {
		case 0:
		case 1:
		case 2:
		case 3:
			return fm7_bootroms[bootmode][raddr];
			break;
# if defined(_FM77_VARIANTS)
		case 4:
			return fm7_bootram[raddr];
			break;
# endif				
		default:
			return fm7_bootroms[0][raddr];
			break;
		}
#endif
	}else if (addr < 0xfffe) { // VECTOR
		raddr = addr - 0xffe0;
		return fm7_mainmem_bootrom_vector[raddr];
	} else if(addr < 0x10000) {
#if defined(_FM77AV_VARIANTS)
		{
			wait();
			raddr = addr - 0xfe00;
			return fm7_bootram[raddr];
		}
#else
		{
			raddr = addr - 0xfffe;
			return fm7_mainmem_reset_vector[raddr];
		}
#endif			
	}
	return 0xff;
}

void FM7_MAINMEM::write_page3_bootrom(uint32_t addr, uint32_t data, bool dmamode)
{
	uint32_t raddr;
	if (addr < 0xffe0){
		wait();
		raddr = addr - 0xfe00;
#if defined(_FM77AV_VARIANTS)
		if(boot_ram_write) {
			fm7_bootram[raddr] = (uint8_t)data;
		}
#else
		switch(bootmode) {
		case 0:
		case 1:
		case 2:
		case 3:
			break;
# if defined(_FM77_VARIANTS)
		case 4:
			if(boot_ram_write) {
				fm7_bootram[raddr] = (uint8_t)data;
			}
			break;
# endif				
		default:
			break;
		}
#endif
	}else if (addr < 0xfffe) { // VECTOR
		raddr = addr - 0xffe0;
		fm7_mainmem_bootrom_vector[raddr] = (uint8_t)data;;
	} else if(addr < 0x10000) {
#if defined(_FM77AV_VARIANTS)
		{
			wait();
			raddr = addr - 0xfe00;
			if(boot_ram_write) fm7_bootram[raddr] = (uint8_t)data;
		}
#endif			
	}
	return;
}

uint8_t FM7_MAINMEM::read_page3_mmio(uint32_t addr, bool dmamode)
{
	wait();
	if(mainio != NULL) {
		return (uint8_t)(mainio->read_data8(addr));
	} else {
		return 0xff;
	}
}

void FM7_MAINMEM::write_page3_mmio(uint32_t addr, uint32_t data, bool dmamode)
{
	wait();
	if(mainio != NULL) {
		mainio->write_data8(addr, data);
	}
}

int FM7_MAINMEM::window_convert(uint32_t addr, uint32_t *realaddr)
{
	uint32_t raddr = addr;
#ifdef HAS_MMR
	if((addr < 0x8000) && (addr >= 0x7c00)) {
		raddr = ((window_offset * 256) + addr) & 0x0ffff; 
		*realaddr = raddr;
#ifdef _FM77AV_VARIANTS
		//printf("TWR hit %04x -> %04x\n", addr, raddr);
		return 0;
#else // FM77(L4 or others)
		*realaddr |= 0x20000;
		return 0;
#endif
	}
	// Window not hit.
#endif
	return -1;
}

int FM7_MAINMEM::mmr_convert(uint32_t addr, uint32_t *realaddr, bool dmastate)
{
	uint32_t  raddr = addr & 0x0fff;
	uint32_t sraddr;
	uint32_t  mmr_bank;
	uint32_t  major_bank;
	uint8_t segment;
   
#ifdef HAS_MMR
	if(dmastate) {
		segment = 0x00;
	} else {
		segment = mmr_segment;
	}
	//if(addr >= 0xfc00) return -1;
	if(!mmr_extend) {
		mmr_bank = mmr_map_data[((addr >> 12) & 0x000f) | ((segment & 0x03) << 4)] & 0x003f;
	} else {
		mmr_bank = mmr_map_data[((addr >> 12) & 0x000f) | ((segment & 0x07) << 4)] & 0x007f;
	}		
	// Reallocated by MMR
	// Bank 3x : Standard memories.
	sraddr = (mmr_bank << 12) | raddr;
	
# ifdef _FM77AV_VARIANTS
	if(mmr_bank == 0x3f) {
		if((raddr >= 0xd80) && (raddr <= 0xd97)) { // MMR AREA
			*realaddr = 0;
			return -1;
		} else {
			*realaddr = raddr | 0x3f000;
			return 0;
		}
	}
# elif defined(_FM77_VARIANTS)
	if(mmr_bank == 0x3f) {
		if((raddr >= 0xc00) && (raddr < 0xe00)) {
			if(is_basicrom) {
				*realaddr = 0;
				return -1;
			} else {
				*realaddr = raddr - 0xc00;
				return FM7_MAINMEM_SHADOWRAM;
			}
		} else if(raddr >= 0xe00) {
			*realaddr = raddr - 0x0e00;
			if(is_basicrom) {
				if(diag_load_bootrom_mmr) {
					return FM7_MAINMEM_BOOTROM_MMR;
				} else {
					return FM7_MAINMEM_BOOTROM_BAS;
				}
			} else {
				return FM7_MAINMEM_BOOTROM_RAM;
			}
		} else {
			*realaddr = raddr | 0x3f000;
			return 0;
		} 
	}
# endif
	*realaddr = sraddr;
#endif
	return 0;
}

void FM7_MAINMEM::write_directaccess(uint32_t addr, uint32_t data, bool dmamode)
{
	if(!sub_halted) return; // Not halt
	if(dmamode) {
		display->write_dma_data8(addr & 0xffff, data); // Okay?
	} else {
		display->write_data8(addr & 0xffff, data); // Okay?
	}
	return;
}

uint8_t FM7_MAINMEM::read_directaccess(uint32_t addr, bool dmamode)
{
	uint32_t data;
	if(!sub_halted) return 0xff; // Not halt
	if(dmamode) {
		data = display->read_dma_data8(addr & 0xffff); // Okay?
	} else {
		data = display->read_data8(addr & 0xffff); // Okay?
	}
	return (uint8_t)data;
}

void FM7_MAINMEM::write_page3_sharedram(uint32_t addr, uint32_t data, bool dmamode)
{
	if(!sub_halted) return; // Not halt
	display->write_data8((addr & 0x7f) + 0xd380, data); // Okay?
	return;
}

uint8_t FM7_MAINMEM::read_page3_sharedram(uint32_t addr, bool dmamode)
{
	if(!sub_halted) return 0xff; // Not halt
	return (uint8_t)(display->read_data8((addr & 0x7f) + 0xd380)); // Okay?
}

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

void FM7_MAINMEM::write_pages(uint32_t addr, uint32_t data, bool dmamode)
{
	uint32_t realaddr = addr;
	uint32_t saddr;
	uint8_t *pdata;
	
#ifdef _FM77AV_VARIANTS
	if(initiator_enabled) {
		if(mmr_enabled) realaddr = addr - 0x30000;
		if((realaddr >= 0x6000) && (realaddr < 0x8000)) {
			//realaddr = realaddr - 0x6000;
			//fm7_mainmem_initrom[realaddr] = (uint8_t)data;
			return;
		}
		if((realaddr >= 0xfffe) && (realaddr < 0x10000)) {
			//realaddr = realaddr - 0xe000;
			//fm7_mainmem_initrom[realaddr] = (uint8_t)data;
			return;
		}
	}
#endif
#if defined(HAS_MMR)
	if(mmr_enabled) {
		realaddr = addr & 0xfffff;
	} else {
		realaddr = (addr & 0xffff) + 0x30000;
	}
#else
	realaddr = (addr & 0xffff) + 0x30000;
#endif
	saddr = realaddr >> 7;
	pdata = addr_w_table_page[saddr];
	if(pdata == NULL) {
		if(func_w_table_page[saddr] != NULL) {
			(this->*func_w_table_page[saddr])(realaddr & 0xffff, data, dmamode);
			return;
		}
	} else {
		pdata[realaddr & 0x7f] = (uint8_t)data;
		return;
	}
	return;
}	

uint8_t FM7_MAINMEM::read_pages(uint32_t addr, bool dmamode)
{
	uint32_t realaddr = addr;
	uint32_t saddr;
	uint8_t *pdata;
	uint8_t data;
#ifdef _FM77AV_VARIANTS
	if(initiator_enabled) {
		if(mmr_enabled) realaddr = addr - 0x30000;
		if((realaddr >= 0x6000) && (realaddr < 0x8000)) {
			realaddr = realaddr - 0x6000;
			data = fm7_mainmem_initrom[realaddr];
			return data;
		}
		if((realaddr >= 0xfffe) && (realaddr < 0x10000)) {
			realaddr = realaddr - 0xe000;
			data = fm7_mainmem_initrom[realaddr];
			return data;
		}
	}
#endif
#if defined(HAS_MMR)
	if(mmr_enabled) {
		realaddr = addr & 0xfffff;
	} else {
		realaddr = (addr & 0xffff) + 0x30000;
	}
#else
	realaddr = (addr & 0xffff) + 0x30000;
#endif
	saddr = realaddr >> 7;
	pdata = addr_r_table_page[saddr];
	if(pdata == NULL) {
		if(func_r_table_page[saddr] != NULL) {
			data = (this->*func_r_table_page[saddr])(realaddr & 0xffff, dmamode);
			return data;
		}
	} else {
		data = pdata[realaddr & 0x7f];
		return data;
	}
	return 0xff;
}	

uint32_t FM7_MAINMEM::read_signal(int sigid)
{
	uint32_t value = 0x00000000;
	switch(sigid) {
	case FM7_MAINIO_PUSH_FD0F:
		value = (basicrom_fd0f) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_IS_BASICROM:
		value = (is_basicrom) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_CLOCKMODE:
		value = (clockmode) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_BOOTMODE:
		value = (uint32_t)bootmode & 0x07;
		break;
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	case FM7_MAINIO_BOOTRAM_RW:
		value = (boot_ram_write) ? 0xffffffff : 0x00000000;
		break;
#endif			
#ifdef HAS_MMR			
	case FM7_MAINIO_WINDOW_ENABLED:
		value = (window_enabled) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_WINDOW_FAST:
		value = (window_fast) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_FASTMMR_ENABLED:
		value = (mmr_fast) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_MMR_ENABLED:
		value = (mmr_enabled) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINIO_MMR_EXTENDED:
		value = (mmr_extend) ? 0xffffffff : 0x00000000;
		break;
	case FM7_MAINMEM_REFRESH_FAST:
		value = (refresh_fast) ? 0xffffffff : 0x00000000;
		break;
#endif			
#if defined(_FM77AV_VARIANTS)
	case FM7_MAINIO_INITROM_ENABLED:
		value = (initiator_enabled) ? 0xffffffff: 0x00000000;
		break;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)	   
	case FM7_MAINIO_EXTROM:
		value = (extrom_bank) ? 0xffffffff: 0x00000000;
		break;
# endif	   
	case FM7_MAINIO_EXTBANK:
		value = extcard_bank & 0x3f;
		value |= (dictram_enabled) ? 0x80 : 0;
		value |= (dictrom_enabled) ? 0x40 : 0;
		break;
#endif
	}
	return value;
}


void FM7_MAINMEM::write_signal(int sigid, uint32_t data, uint32_t mask)
{
	bool flag = ((data & mask) != 0);
	switch(sigid) {
		case SIG_FM7_SUB_HALT:
			sub_halted = flag;
			break;
		case FM7_MAINIO_IS_BASICROM:
			is_basicrom = flag;
			break;
		case FM7_MAINIO_PUSH_FD0F:
			basicrom_fd0f = flag;
			break;
		case FM7_MAINIO_CLOCKMODE:
			clockmode = flag;
			setclock(clockmode ? 0 : 1);
			break;
		case FM7_MAINIO_BOOTMODE:
			bootmode = data & 0x07;
			break;
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
		case FM7_MAINIO_BOOTRAM_RW:
			boot_ram_write = flag;
			break;
#endif			
#ifdef _FM77AV_VARIANTS
		case FM7_MAINIO_INITROM_ENABLED:
			initiator_enabled = flag;
			break;
		case FM7_MAINIO_EXTBANK:
			extcard_bank = data & 0x3f;
			dictram_enabled = ((data & 0x80) != 0) ? true : false;
			dictrom_enabled = ((data & 0x40) != 0) ? true : false;
			break;
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)	   
		case FM7_MAINIO_EXTROM:
			extrom_bank = flag;
	   		break;
# endif	   
#endif			
#ifdef HAS_MMR			
		case FM7_MAINIO_WINDOW_ENABLED:
			window_enabled = flag;
			setclock(config.cpu_type);
			break;
		case FM7_MAINIO_WINDOW_FAST:
			window_fast = flag;
			setclock(config.cpu_type);
			break;
		case FM7_MAINIO_FASTMMR_ENABLED:
			mmr_fast = flag;
			setclock(config.cpu_type);
			break;
		case FM7_MAINIO_MMR_ENABLED:
			mmr_enabled = flag;
			setclock(config.cpu_type);
			break;
		case FM7_MAINIO_MMR_EXTENDED:
			mmr_extend = flag;
			break;
		case FM7_MAINMEM_REFRESH_FAST:
			refresh_fast = flag;
			setclock(config.cpu_type);
			break;
#endif			
	}
}


uint32_t FM7_MAINMEM::read_dma_data8(uint32_t addr)
{
#if defined(HAS_MMR)	
	uint32_t val;
	val = this->read_data8_main(addr & 0xffff, true);
	return val;
#else
	return this->read_data8(addr & 0xffff);
#endif	
}

uint32_t FM7_MAINMEM::read_dma_io8(uint32_t addr)
{
#if defined(HAS_MMR)	
	uint32_t val;
	val = this->read_data8_main(addr & 0xffff, true);
	return val;
#else
	return this->read_data8(addr & 0xffff);
#endif	
}


uint32_t FM7_MAINMEM::read_data8_main(uint32_t addr, bool dmamode)
{
	uint32_t realaddr;
	int bank;
	
#ifdef HAS_MMR
	if(window_enabled) {
		int stat;
		stat = window_convert(addr, &realaddr);
		//if(stat >= 0) printf("WINDOW CONVERT: %04x to %04x, bank = %02x\n", addr, raddr, stat);
		if(stat >= 0) {
# ifdef _FM77AV_VARIANTS
			return (uint32_t)(fm7_mainmem_mmrbank_0[realaddr & 0xffff]);
# else
			if((extram_pages >= 3) && (extram_connected)) {
				return (uint32_t)(fm7_mainmem_extram[realaddr]);
			} else {
				return 0xff;
			}
# endif							 
		}
	}
	if(mmr_enabled) {
		int stat;
		uint32_t raddr;
		if(addr >= 0xfc00) {
			raddr = (addr & 0xffff) | 0x30000;
			return (uint32_t)read_pages(raddr, dmamode);
		} else {
			stat = mmr_convert(addr, &raddr, dmamode);
		}
		if(stat == 0) {
			return (uint32_t)read_pages(raddr, dmamode);
		}
# if defined(_FM77_VARIANTS)
		else {
			switch(stat) {
			case FM7_MAINMEM_SHADOWRAM:
				return fm77_shadowram[raddr & 0x1ff];
				break;
			case FM7_MAINMEM_BOOTROM_MMR:
				return fm7_bootroms[2][raddr & 0x1ff];
				break;
			case FM7_MAINMEM_BOOTROM_BAS:
				return fm7_bootroms[0][raddr & 0x1ff];
				break;
			case FM7_MAINMEM_BOOTROM_RAM:
				return fm7_bootram[raddr & 0x1ff];
				break;
			default:
				return 0xff;
				break;
			}
		}		
# endif
	}
#endif
	return (uint32_t)read_pages(addr, dmamode);
}	

uint32_t FM7_MAINMEM::read_data8(uint32_t addr)
{
#if defined(HAS_MMR)   
	if(addr >= FM7_MAINIO_WINDOW_OFFSET) {
		switch(addr) {
		case FM7_MAINIO_WINDOW_OFFSET:
			return (uint32_t)window_offset;
			break;
		case FM7_MAINIO_MMR_SEGMENT:
			return (uint32_t)mmr_segment;
			break;
		default:
			if((addr >= FM7_MAINIO_MMR_BANK) && (addr < (FM7_MAINIO_MMR_BANK + 0x80))){
				return mmr_map_data[addr - FM7_MAINIO_MMR_BANK];
			}
			break;
		}
		return 0xff;
	}
#endif   
	return this->read_data8_main(addr, false);
}

void FM7_MAINMEM::write_dma_data8(uint32_t addr, uint32_t data)
{
#if defined(HAS_MMR)
	this->write_data8_main(addr & 0xffff, data, true);
#else
	this->write_data8(addr & 0xffff, data);
#endif	
}

void FM7_MAINMEM::write_dma_io8(uint32_t addr, uint32_t data)
{
#if defined(HAS_MMR)
	this->write_data8_main(addr & 0xffff, data, true);
#else
	this->write_data8(addr & 0xffff, data);
#endif	
}

void FM7_MAINMEM::write_data8_main(uint32_t addr, uint32_t data, bool dmamode)
{
	
	uint32_t realaddr;
	int bank;
	
# ifdef HAS_MMR
	if(window_enabled) {
		int stat;
		uint32_t raddr;
		stat = window_convert(addr, &realaddr);
		//if(stat >= 0) printf("WINDOW CONVERT: %04x to %04x, bank = %02x\n", addr, raddr, stat);
		if(stat >= 0) {
# ifdef _FM77AV_VARIANTS
			fm7_mainmem_mmrbank_0[realaddr & 0xffff] = (uint8_t)data;
			return;
# else
			if((extram_pages >= 3) && (extram_connected)) {
				fm7_mainmem_extram[realaddr] = (uint8_t)data;
			}
			return;
# endif							 
		}
	}
	if(mmr_enabled) {
		int stat;
		uint32_t raddr;
		if(addr >= 0xfc00) {
			raddr = (addr & 0xffff) | 0x30000;
			write_pages(raddr, data, dmamode);
			return;
		} else {
			stat = mmr_convert(addr, &raddr, dmamode);
		}
		if(stat == 0) {
			write_pages(raddr, data, dmamode);
			return;
		}
# if defined(_FM77_VARIANTS)
		else {
			switch(stat) {
			case FM7_MAINMEM_SHADOWRAM:
				fm77_shadowram[raddr & 0x1ff] = (uint8_t)data;
				break;
			case FM7_MAINMEM_BOOTROM_RAM:
				if(boot_ram_write) {
					fm7_bootram[raddr & 0x1ff] = (uint8_t)data;
				}
				break;
			default:
				break;
			}
			return;
		}
# endif
	}
#endif
	write_pages(addr, data, dmamode);
	return;
}

void FM7_MAINMEM::write_data8(uint32_t addr, uint32_t data)
{
#if defined(HAS_MMR)   
	if(addr >= FM7_MAINIO_WINDOW_OFFSET) {
		switch(addr) {
		case FM7_MAINIO_WINDOW_OFFSET:
			window_offset = data;
			break;
		case FM7_MAINIO_MMR_SEGMENT:
			if(mmr_extend) {
				mmr_segment = data & 0x07;
			} else {
				mmr_segment = data & 0x03;
			}
			break;
		default:
			if((addr >= FM7_MAINIO_MMR_BANK) && (addr < (FM7_MAINIO_MMR_BANK + 0x80))){
				mmr_map_data[addr - FM7_MAINIO_MMR_BANK] = (uint8_t)data;
			}
			break;
		}
		return;
	}
#endif
	write_data8_main(addr, data, false);
}

// Read / Write data(s) as big endian.
uint32_t FM7_MAINMEM::read_data16(uint32_t addr)
{
	uint32_t hi, lo;
	uint32_t val;
   
	hi = read_data8(addr) & 0xff;
	lo = read_data8(addr + 1) & 0xff;
   
	val = hi * 256 + lo;
	return val;
}

uint32_t FM7_MAINMEM::read_data32(uint32_t addr)
{
	uint32_t ah, a2, a3, al;
	uint32_t val;
   
	ah = read_data8(addr) & 0xff;
	a2 = read_data8(addr + 1) & 0xff;
	a3 = read_data8(addr + 2) & 0xff;
	al = read_data8(addr + 3) & 0xff;
   
	val = ah * (65536 * 256) + a2 * 65536 + a3 * 256 + al;
	return val;
}

void FM7_MAINMEM::write_data16(uint32_t addr, uint32_t data)
{
	uint32_t d = data;
   
	write_data8(addr + 1, d & 0xff);
	d = d / 256;
	write_data8(addr + 0, d & 0xff);
}

void FM7_MAINMEM::write_data32(uint32_t addr, uint32_t data)
{
	uint32_t d = data;
   
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

void FM7_MAINMEM::update_config()
{
	//setclock(config.cpu_type);
}

FM7_MAINMEM::FM7_MAINMEM(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
	p_vm = parent_vm;
	p_emu = parent_emu;
#if !defined(_FM77AV_VARIANTS)
	for(int i = 0; i < 4; i++) fm7_bootroms[i] = (uint8_t *)malloc(0x200);
#endif	
	mainio = NULL;
	display = NULL;
	maincpu = NULL;
#if defined(CAPABLE_DICTROM)
	kanjiclass1 = NULL;
#endif	
#if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)  || \
    defined(_FM77_VARIANTS)
	fm7_mainmem_extram = NULL;
#endif
	set_device_name(_T("MAIN MEMORY"));
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
	memset(fm7_mainmem_omote, 0x00, 0x8000 * sizeof(uint8_t));

	// $8000-$FBFF
	memset(fm7_mainmem_ura, 0x00, 0x7c00 * sizeof(uint8_t));
	
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
   
	memset(fm7_bootram, 0x00, 0x200 * sizeof(uint8_t)); // RAM
#  else
       // FM-7/8
	diag_load_bootrom_mmr = false;
#  endif
# elif defined(_FM77AV_VARIANTS)
	memset(fm7_mainmem_mmrbank_0, 0x00, 0x10000 * sizeof(uint8_t));
	
	memset(fm7_mainmem_mmrbank_2, 0x00, 0x10000 * sizeof(uint8_t));
	
	diag_load_initrom = false;
	memset(fm7_mainmem_initrom, 0xff, 0x2000 * sizeof(uint8_t));

	if(read_bios(_T("INITIATE.ROM"), fm7_mainmem_initrom, 0x2000) >= 0x2000) diag_load_initrom = true;
	this->out_debug_log(_T("77AV INITIATOR ROM READING : %s"), diag_load_initrom ? "OK" : "NG");

	if(read_bios(_T("BOOT_MMR.ROM"), fm77av_hidden_bootmmr, 0x200) < 0x1e0) {
		memcpy(fm77av_hidden_bootmmr, &fm7_mainmem_initrom[0x1a00], 0x200);
	}
	fm77av_hidden_bootmmr[0x1fe] = 0xfe;
	fm77av_hidden_bootmmr[0x1fe] = 0x00;
	
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
	fm7_mainmem_reset_vector[0] = 0xfe;
	fm7_mainmem_reset_vector[1] = 0x00;
   
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
   
	memset(fm7_mainmem_bioswork, 0x00, 0x80 * sizeof(uint8_t));
#if defined(CAPABLE_DICTROM)
	diag_load_dictrom = false;
	memset(fm7_mainmem_dictrom, 0xff, 0x40000 * sizeof(uint8_t));
	if(read_bios(_T("DICROM.ROM"), fm7_mainmem_dictrom, 0x40000) == 0x40000) diag_load_dictrom = true;
	this->out_debug_log(_T("DICTIONARY ROM READING : %s"), diag_load_dictrom ? "OK" : "NG");
	dictrom_connected = diag_load_dictrom;
	
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

	setup_table_page3();
	setup_table_extram();
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

#define STATE_VERSION 2
void FM7_MAINMEM::save_state(FILEIO *state_fio)
{
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);
	this->out_debug_log("Save State: MAINMEM: id=%d ver=%d\n", this_device_id, STATE_VERSION);

	// V1
	state_fio->FputBool(ioaccess_wait);
	state_fio->FputInt32_BE(waitfactor);
	state_fio->FputInt32_BE(waitcount);

	state_fio->FputBool(sub_halted);
	
	state_fio->FputBool(diag_load_basicrom);
	state_fio->FputBool(diag_load_bootrom_bas);
	state_fio->FputBool(diag_load_bootrom_dos);
	state_fio->FputBool(diag_load_bootrom_mmr);
	state_fio->Fwrite(fm7_mainmem_omote, sizeof(fm7_mainmem_omote), 1);
	state_fio->Fwrite(fm7_mainmem_ura, sizeof(fm7_mainmem_ura), 1);
	state_fio->Fwrite(fm7_mainmem_basicrom, sizeof(fm7_mainmem_basicrom), 1);
	state_fio->Fwrite(fm7_mainmem_bioswork, sizeof(fm7_mainmem_bioswork), 1);
	state_fio->Fwrite(fm7_mainmem_bootrom_vector, sizeof(fm7_mainmem_bootrom_vector), 1);
	state_fio->Fwrite(fm7_mainmem_reset_vector, sizeof(fm7_mainmem_reset_vector), 1);
	
	state_fio->Fwrite(fm7_mainmem_null, sizeof(fm7_mainmem_null), 1);
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	state_fio->Fwrite(fm7_bootram, sizeof(fm7_bootram), 1);
#endif	
#if !defined(_FM77AV_VARIANTS)
	int addr;
	for(addr = 0; addr < 4; addr++) state_fio->Fwrite(fm7_bootroms[addr], sizeof(0x200), 1);
#endif	
#ifdef _FM77AV_VARIANTS
	state_fio->FputBool(dictrom_connected);
	state_fio->FputBool(use_page2_extram);
	
	state_fio->FputBool(diag_load_initrom);
	state_fio->FputBool(diag_load_dictrom);
	state_fio->FputBool(diag_load_learndata);
	state_fio->Fwrite(fm7_mainmem_initrom, sizeof(fm7_mainmem_initrom), 1);
	state_fio->Fwrite(fm77av_hidden_bootmmr, sizeof(fm77av_hidden_bootmmr), 1);
	
	state_fio->Fwrite(fm7_mainmem_mmrbank_0, sizeof(fm7_mainmem_mmrbank_0), 1);
	state_fio->Fwrite(fm7_mainmem_mmrbank_2, sizeof(fm7_mainmem_mmrbank_2), 1);
	
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	state_fio->FputBool(diag_load_extrarom);
	state_fio->Fwrite(fm7_mainmem_extrarom, sizeof(fm7_mainmem_extrarom), 1);
# endif
# if defined(CAPABLE_DICTROM)
	state_fio->Fwrite(fm7_mainmem_dictrom, sizeof(fm7_mainmem_dictrom), 1);
	state_fio->Fwrite(fm7_mainmem_learndata, sizeof(fm7_mainmem_learndata), 1);
# endif
#endif
	
#ifdef HAS_MMR
	state_fio->FputBool(extram_connected);
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	 defined(_FM77_VARIANTS)
	int pages;
	state_fio->FputInt32_BE(extram_pages);
	pages = extram_pages;
#  if defined(_FM77_VARIANTS)
	if(pages > 3) pages = 3;
#  else
	if(pages > 12) pages = 12;
#  endif	
	if(pages > 0) state_fio->Fwrite(fm7_mainmem_extram, pages * 0x10000, 1);
#  if defined(_FM77_VARIANTS)
	state_fio->Fwrite(fm77_shadowram, sizeof(fm77_shadowram), 1);
#  endif
# endif
#endif
	
	{ // V2;
		state_fio->FputBool(is_basicrom);
		state_fio->FputBool(clockmode);
		state_fio->FputBool(basicrom_fd0f);
		state_fio->FputUint32_BE(bootmode);
#if defined(_FM77AV_VARIANTS)
		state_fio->FputUint32_BE(extcard_bank);
		state_fio->FputBool(extrom_bank);
		state_fio->FputBool(initiator_enabled);
		state_fio->FputBool(dictrom_enabled);
		state_fio->FputBool(dictram_enabled);
#endif
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
		state_fio->FputBool(boot_ram_write);
#endif		
#if defined(HAS_MMR)
		state_fio->FputBool(window_enabled);
		state_fio->FputBool(mmr_enabled);
		state_fio->FputBool(mmr_fast);
		state_fio->FputBool(mmr_extend);
		
		state_fio->FputUint16_BE(window_offset);
		state_fio->FputBool(window_fast);
		state_fio->FputBool(refresh_fast);
		state_fio->FputUint8(mmr_segment);
		state_fio->Fwrite(mmr_map_data, sizeof(mmr_map_data), 1);
#endif
	}
}

bool FM7_MAINMEM::load_state(FILEIO *state_fio)
{
	uint32_t version;
	version = state_fio->FgetUint32_BE();
	if(this_device_id != state_fio->FgetInt32_BE()) return false;
	this->out_debug_log("Load State: MAINMEM: id=%d ver=%d\n", this_device_id, version);
	if(version >= 1) {
		// V1
		ioaccess_wait = state_fio->FgetBool();
		waitfactor = state_fio->FgetInt32_BE();
		waitcount = state_fio->FgetInt32_BE();

		sub_halted = state_fio->FgetBool();
	
		diag_load_basicrom = state_fio->FgetBool();
		diag_load_bootrom_bas = state_fio->FgetBool();
		diag_load_bootrom_dos = state_fio->FgetBool();
		diag_load_bootrom_mmr = state_fio->FgetBool();
		
		state_fio->Fread(fm7_mainmem_omote, sizeof(fm7_mainmem_omote), 1);
		state_fio->Fread(fm7_mainmem_ura, sizeof(fm7_mainmem_ura), 1);
		state_fio->Fread(fm7_mainmem_basicrom, sizeof(fm7_mainmem_basicrom), 1);
		state_fio->Fread(fm7_mainmem_bioswork, sizeof(fm7_mainmem_bioswork), 1);
		state_fio->Fread(fm7_mainmem_bootrom_vector, sizeof(fm7_mainmem_bootrom_vector), 1);
		state_fio->Fread(fm7_mainmem_reset_vector, sizeof(fm7_mainmem_reset_vector), 1);
	
		state_fio->Fread(fm7_mainmem_null, sizeof(fm7_mainmem_null), 1);
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
		state_fio->Fread(fm7_bootram, sizeof(fm7_bootram), 1);
#endif	
#if !defined(_FM77AV_VARIANTS)
		int addr;
		for(addr = 0; addr < 4; addr++) state_fio->Fread(fm7_bootroms[addr], sizeof(0x200), 1);
#endif	
#ifdef _FM77AV_VARIANTS
		dictrom_connected = state_fio->FgetBool();
		use_page2_extram = state_fio->FgetBool();
	
		diag_load_initrom = state_fio->FgetBool();
		diag_load_dictrom = state_fio->FgetBool();
		diag_load_learndata = state_fio->FgetBool();
		state_fio->Fread(fm7_mainmem_initrom, sizeof(fm7_mainmem_initrom), 1);
		state_fio->Fread(fm77av_hidden_bootmmr, sizeof(fm77av_hidden_bootmmr), 1);
		
		state_fio->Fread(fm7_mainmem_mmrbank_0, sizeof(fm7_mainmem_mmrbank_0), 1);
		state_fio->Fread(fm7_mainmem_mmrbank_2, sizeof(fm7_mainmem_mmrbank_2), 1);
	
# if defined(_FM77AV40EX) || defined(_FM77AV40SX)
		diag_load_extrarom = state_fio->FgetBool();
		state_fio->Fread(fm7_mainmem_extrarom, sizeof(fm7_mainmem_extrarom), 1);
# endif		
# if defined(CAPABLE_DICTROM)
		state_fio->Fread(fm7_mainmem_dictrom, sizeof(fm7_mainmem_dictrom), 1);
		state_fio->Fread(fm7_mainmem_learndata, sizeof(fm7_mainmem_learndata), 1);
# endif
#endif
	
#ifdef HAS_MMR
		extram_connected = state_fio->FgetBool();
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) || \
	 defined(_FM77_VARIANTS)
		int pages;
		extram_pages = state_fio->FgetInt32_BE();
		pages = extram_pages;
#  if defined(_FM77_VARIANTS)
		if(pages > 3) pages = 3;
#  else
		if(pages > 12) pages = 12;
#  endif	
		if(pages > 0) state_fio->Fread(fm7_mainmem_extram, pages * 0x10000, 1);
#  if defined(_FM77_VARIANTS)
		state_fio->Fread(fm77_shadowram, sizeof(fm77_shadowram), 1);
#  endif
# endif
#endif
		if(version == 1) return true;
	}
	{ // V2;
		is_basicrom = state_fio->FgetBool();
		clockmode = state_fio->FgetBool();
		basicrom_fd0f = state_fio->FgetBool();
		bootmode = state_fio->FgetUint32_BE();
#if defined(_FM77AV_VARIANTS)
		extcard_bank = state_fio->FgetUint32_BE();
		extrom_bank = state_fio->FgetBool();
		initiator_enabled = state_fio->FgetBool();
		dictrom_enabled = state_fio->FgetBool();
		dictram_enabled = state_fio->FgetBool();
#endif
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
		boot_ram_write = state_fio->FgetBool();
#endif		
#if defined(HAS_MMR)
		window_enabled = state_fio->FgetBool();
		mmr_enabled = state_fio->FgetBool();
		mmr_fast = state_fio->FgetBool();
		mmr_extend = state_fio->FgetBool();
		
		window_offset = state_fio->FgetUint16_BE();
		window_fast = state_fio->FgetBool();
		refresh_fast = state_fio->FgetBool();
		mmr_segment = state_fio->FgetUint8();
		state_fio->Fread(mmr_map_data, sizeof(mmr_map_data), 1);
#endif
	}
	if(version != STATE_VERSION) return false;
	setup_table_page3();
	setup_table_extram();
	return true;
}
