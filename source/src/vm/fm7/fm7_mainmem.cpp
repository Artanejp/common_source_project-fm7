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
	sub_halted = (display->read_signal(SIG_DISPLAY_HALT) == 0) ? false : true;
	//sub_halted = false;
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
	maincpu->reset();
}

void FM7_MAINMEM::setclock(int mode)
{
	uint32 clock = MAINCLOCK_SLOW;
	if(mode == 1) { // SLOW
		clock = MAINCLOCK_SLOW; // Temporally
#if defined(HAS_MMR)		
		if(!mmr_fast && !window_fast) {
			if(refresh_fast) {
				if(mmr_enabled || window_enabled) {
					clock = (uint32)((double)clock * 1.089);
				} else {
					clock = (uint32)((double)clock * 1.086);
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
					clock = (uint32)((double)clock * 1.089);
				} else {
					clock = (uint32)((double)clock * 1.086);
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


int FM7_MAINMEM::window_convert(uint32 addr, uint32 *realaddr)
{
	uint32 raddr = addr;
#ifdef HAS_MMR
	if((addr < 0x8000) && (addr >= 0x7c00)) {
		raddr = ((window_offset * 256) + addr) & 0x0ffff; 
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


int FM7_MAINMEM::check_extrom(uint32 raddr, uint32 *realaddr)
{
#if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	if(extrom_bank) { // Extra ROM selected.
		uint32 dbank = extcard_bank & 0x3f;
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
   

int FM7_MAINMEM::mmr_convert(uint32 addr, uint32 *realaddr, bool write_state, bool dmamode)
{
	uint32  raddr = addr & 0x0fff;
	uint32  mmr_bank;
	uint32  major_bank;
	uint8 segment;
   
#ifdef HAS_MMR
	if(dmamode) {
		segment = 0x00;
	} else {
		segment = mmr_segment;
	}
	if(addr >= 0xfc00) return -1;
	if(!mmr_extend) {
		mmr_bank = mmr_map_data[(addr >> 12) & 0x000f | ((segment & 0x03) << 4)] & 0x003f;
	} else {
		mmr_bank = mmr_map_data[(addr >> 12) & 0x000f | ((segment & 0x07) << 4)] & 0x007f;
	}		
	// Reallocated by MMR
	// Bank 3x : Standard memories.
	if((mmr_bank < 0x3f) && (mmr_bank >= 0x30)) {
		raddr = ((mmr_bank << 12) | raddr) & 0xffff;
		return nonmmr_convert(raddr, realaddr);
	}
# ifdef _FM77AV_VARIANTS
	else if(mmr_bank == 0x3f) {
		if((raddr >= 0xd80) && (raddr <= 0xd97)) { // MMR AREA
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		} else {
			raddr = raddr | 0xf000;
			return nonmmr_convert(raddr, realaddr); // Access I/O, Bootrom, even via MMR.
		}
	}
# elif defined(_FM77_VARIANTS)
	else if(mmr_bank == 0x3f) {
		if((raddr >= 0xc00) && (raddr < 0xe00)) {
			if(is_basicrom) {
				*realaddr = 0;
				return FM7_MAINMEM_ZERO;
			} else {
				*realaddr = raddr - 0xc00;
				return FM7_MAINMEM_SHADOWRAM;
			}
		} else if(raddr >= 0xe00) {
			*realaddr = addr - 0x0e00;
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
			raddr = raddr | 0xf000;
			return nonmmr_convert(raddr, realaddr); // Access I/O, Bootrom, even via MMR.
		} 
	}
# endif
 	major_bank = (mmr_bank >> 4) & 0x0f;
    
# ifdef _FM77AV_VARIANTS
	if(major_bank == 0x0) { // PAGE 0
 		*realaddr = ((mmr_bank << 12) | raddr) & 0x0ffff;
 		return FM7_MAINMEM_AV_PAGE0;
	} else if(major_bank == 0x1) { // PAGE 1		
 		*realaddr = ((mmr_bank << 12) | raddr) & 0x0ffff;
 		return FM7_MAINMEM_AV_DIRECTACCESS;
	} else 	if(major_bank == 0x2) { // PAGE 2
		if((mmr_bank == 0x2e) && (!write_state)) {
			int banknum = check_extrom(raddr, realaddr);
			if(banknum >= 0) {
				return banknum;
			} else {
#  if defined(CAPABLE_DICTROM)
				if(dictrom_connected && dictrom_enabled) { // Dictionary ROM
					uint32 dbank = extcard_bank & 0x3f;
					*realaddr = raddr | (dbank << 12);
					return FM7_MAINMEM_DICTROM;
				}
#  else
					*realaddr = 0;
					return FM7_MAINMEM_NULL;
#  endif			   
			}
		}
#  if defined(CAPABLE_DICTROM)
		switch(mmr_bank) {
		case 0x28:
		case 0x29: // Backuped RAM
			if(dictrom_connected && dictram_enabled){ // Battery backuped RAM
				raddr = (((uint32)mmr_bank & 0x01) << 12) | raddr;
				raddr =  raddr & 0x1fff;
				*realaddr = raddr;
				return FM7_MAINMEM_BACKUPED_RAM;
			}
			break;
		}
		*realaddr = (raddr | (mmr_bank << 12)) & 0x0ffff;
		return FM7_MAINMEM_AV_PAGE2;
#  else
 		if(use_page2_extram) {
 			*realaddr = ((mmr_bank << 12) | raddr) & 0x0ffff;
 			return FM7_MAINMEM_AV_PAGE2;
 		} else {
			*realaddr = 0;
			return FM7_MAINMEM_NULL;
		}
#  endif
	}
# endif
	
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)
	else if(extram_connected && mmr_extend) { // PAGE 4-
		if((major_bank >= (extram_pages + 4)) || (fm7_mainmem_extram == NULL) ) {
			*realaddr = 0;
			return FM7_MAINMEM_NULL; // $FF
		} else {
			raddr = ((uint32)(mmr_bank - 0x40) << 12) | raddr;
			*realaddr = raddr;
			return FM7_MAINMEM_EXTRAM;
		}
	} else {
		*realaddr = 0;
		return FM7_MAINMEM_NULL;
	}
# elif defined(_FM77_VARIANTS)
	if(extram_connected) { // PAGE 4-
		if((major_bank > (extram_pages - 1)) || (major_bank >= 3)) {
			*realaddr = 0;
			return FM7_MAINMEM_NULL; // $FF
		} else {
			raddr = ((uint32)mmr_bank << 12) | raddr;
			*realaddr = raddr;
			return FM7_MAINMEM_EXTRAM;
		}
	} else {
		*realaddr = 0;
		return FM7_MAINMEM_NULL; // $FF
	}
# else // _FM77AV_VARIANTS
	if(major_bank > 3) {
		*realaddr = 0;
		return FM7_MAINMEM_NULL; // $FF
	}
# endif
#endif // HAS_MMR
	*realaddr = 0;
	return -1;
}

int FM7_MAINMEM::nonmmr_convert(uint32 addr, uint32 *realaddr)
{
	addr &= 0x0ffff;
#ifdef _FM77AV_VARIANTS
	if(initiator_enabled) {
		if((addr >= 0x6000) && (addr < 0x8000)) {
		    //printf("HIT %02x\n", read_table[FM7_MAINMEM_INITROM].memory[addr - 0x6000]);
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

	uint32 addr_major, addr_minor;
	addr_major = (addr >> 12) & 0x0f;

	switch (addr_major) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		*realaddr = addr;
		return FM7_MAINMEM_OMOTE;
		break;
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
		*realaddr = addr - 0x8000;
		if (basicrom_fd0f) {
			return FM7_MAINMEM_BASICROM;
		}
		return FM7_MAINMEM_URA;
		break;
	case 0x0f:
		addr_minor = (addr >> 8) & 0x0f;
		switch (addr_minor){
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
			*realaddr = addr - 0x8000;
			if (basicrom_fd0f) {
				return FM7_MAINMEM_BASICROM;
			}
			return FM7_MAINMEM_URA;
			break;
		case 0x0c:
			if (addr < 0xfc80) {
				*realaddr = addr - 0xfc00;
				return FM7_MAINMEM_BIOSWORK;
			}
			else {
				*realaddr = addr - 0xfc80;
				return FM7_MAINMEM_SHAREDRAM;
			}
			break;
		case 0x0d:
			wait();
			*realaddr = addr - 0xfd00;
			return FM7_MAINMEM_MMIO;
			break;
		default:
			if (addr < 0xffe0){
				wait();
				*realaddr = addr - 0xfe00;
#if defined(_FM77AV_VARIANTS)
				return FM7_MAINMEM_BOOTROM_RAM;
#else
				switch(bootmode) {
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
				case 3:
					return FM7_MAINMEM_BOOTROM_EXTRA;
					break;
# if defined(_FM77_VARIANTS)
				case 4:
					return FM7_MAINMEM_BOOTROM_RAM;
					break;
# endif				
				default:
					return FM7_MAINMEM_BOOTROM_BAS; // Really?
					break;
				}
#endif
			}
			else if (addr < 0xfffe) { // VECTOR
				*realaddr = addr - 0xffe0;
				return FM7_MAINMEM_VECTOR;
			}
#if defined(_FM77AV_VARIANTS)
			else if(addr < 0x10000) {
				wait();
				*realaddr = addr - 0xfe00;
				return FM7_MAINMEM_BOOTROM_RAM;
			}
#else
			else if (addr < 0x10000) {
				*realaddr = addr - 0xfffe;
				return FM7_MAINMEM_RESET_VECTOR;
			}
#endif			
			break;
		}
		break;
	}
	emu->out_debug_log("Main: Over run ADDR = %08x", addr);
	*realaddr = addr;
	return FM7_MAINMEM_NULL;
}
     
int FM7_MAINMEM::getbank(uint32 addr, uint32 *realaddr, bool write_state, bool dmamode)
{
	if(realaddr == NULL) return FM7_MAINMEM_NULL; // Not effect.
#ifdef HAS_MMR
	if(window_enabled) {
		int stat;
		uint32 raddr;
		stat = window_convert(addr, &raddr);
		//if(stat >= 0) printf("WINDOW CONVERT: %04x to %04x, bank = %02x\n", addr, raddr, stat);
		if(stat >= 0) {
			*realaddr = raddr;
			return stat;
		}
	}
	if(mmr_enabled) {
		int stat;
		uint32 raddr;
		stat = mmr_convert(addr, &raddr, write_state, dmamode);
		if(stat >= 0) {
		  //printf("MMR CONVERT: %04x to %05x, bank = %02x\n", addr, raddr, stat);
			*realaddr = raddr;
			return stat;
		}
	}
#endif
	addr = addr & 0xffff;
	// NOT MMR.
	return nonmmr_convert(addr, realaddr);
}

uint32 FM7_MAINMEM::read_signal(int sigid)
{
	uint32 value = 0x00000000;
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
		value = (uint32)bootmode & 0x07;
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


void FM7_MAINMEM::write_signal(int sigid, uint32 data, uint32 mask)
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


uint32 FM7_MAINMEM::read_dma_data8(uint32 addr)
{
#if defined(HAS_MMR)	
	uint32 val;
	val = this->read_data8_main(addr & 0xffff, true);
	return val;
#else
	return this->read_data8(addr & 0xffff);
#endif	
}

uint32 FM7_MAINMEM::read_dma_io8(uint32 addr)
{
#if defined(HAS_MMR)	
	uint32 val;
	val = this->read_data8_main(addr & 0xffff, true);
	return val;
#else
	return this->read_data8(addr & 0xffff);
#endif	
}


uint32 FM7_MAINMEM::read_data8_main(uint32 addr, bool dmamode)
{
	uint32 realaddr;
	int bank;
	
	bank = getbank(addr, &realaddr, false, dmamode);
	if(bank < 0) {
		emu->out_debug_log("Illegal BANK: ADDR = %04x", addr);
		return 0xff; // Illegal
	}
	if(bank == FM7_MAINMEM_SHAREDRAM) {
		if(!sub_halted) return 0xff; // Not halt
		return display->read_data8(realaddr  + 0xd380); // Okay?
	} else if(bank == FM7_MAINMEM_NULL) {
		return 0xff;
	}
#if defined(_FM77AV_VARIANTS)
	else if(bank == FM7_MAINMEM_AV_DIRECTACCESS) {
	   	if(!sub_halted) return 0xff; // Not halt
		if(dmamode) {
			return display->read_dma_data8(realaddr); // Okay?
		} else {
			return display->read_data8(realaddr); // Okay?
		}
	} else if(bank == FM7_MAINMEM_KANJI_DUMMYADDR) {
		return (realaddr & 0x01);
	}
#endif	
#if defined(CAPABLE_DICTROM)
	else if(bank == FM7_MAINMEM_KANJI_LEVEL1) {
		return kanjiclass1->read_data8(KANJIROM_DIRECTADDR + realaddr);
	}
#endif
	else if(read_table[bank].dev != NULL) {
		return read_table[bank].dev->read_data8(realaddr);
	} else if(read_table[bank].memory != NULL) {
		return read_table[bank].memory[realaddr];
	}
	return 0xff; // Dummy
}	

uint32 FM7_MAINMEM::read_data8(uint32 addr)
{
	uint32 realaddr;
	int bank;

#if defined(HAS_MMR)   
	if(addr >= FM7_MAINIO_WINDOW_OFFSET) {
		switch(addr) {
		case FM7_MAINIO_WINDOW_OFFSET:
			return (uint32)window_offset;
			break;
		case FM7_MAINIO_MMR_SEGMENT:
			return (uint32)mmr_segment;
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

void FM7_MAINMEM::write_dma_data8(uint32 addr, uint32 data)
{
#if defined(HAS_MMR)
	this->write_data8_main(addr & 0xffff, data, true);
#else
	this->write_data8(addr & 0xffff, data);
#endif	
}

void FM7_MAINMEM::write_dma_io8(uint32 addr, uint32 data)
{
#if defined(HAS_MMR)
	this->write_data8_main(addr & 0xffff, data, true);
#else
	this->write_data8(addr & 0xffff, data);
#endif	
}

void FM7_MAINMEM::write_data8_main(uint32 addr, uint32 data, bool dmamode)
{
	uint32 realaddr;
	int bank;
	bank = getbank(addr, &realaddr, true, dmamode);
	if(bank < 0) {
		emu->out_debug_log("Illegal BANK: ADDR = %04x", addr);
		return; // Illegal
	}
	if(bank == FM7_MAINMEM_SHAREDRAM) {
		if(!sub_halted) return; // Not halt
		display->write_data8(realaddr + 0xd380, data); // Okay?
		return;
	} else if(bank == FM7_MAINMEM_NULL) {
		return;
	}
#if defined(_FM7) || defined(_FMNEW7)
        else if(bank == FM7_MAINMEM_BASICROM) {
		bank = FM7_MAINMEM_URA; // FM-7/NEW7 write to ura-ram even enabled basic-rom. 
	}
#endif   
   
#if defined(_FM77AV_VARIANTS)
	else if(bank == FM7_MAINMEM_AV_DIRECTACCESS) {
		if(!sub_halted) return; // Not halt
		if(dmamode) {
			display->write_dma_data8(realaddr, data); // Okay?
		} else {
			display->write_data8(realaddr, data); // Okay?
		}
		return;
	}
#endif
#if defined(HAS_MMR)	
	else if(bank == FM7_MAINMEM_BOOTROM_RAM) {
		if(!boot_ram_write) return;
	}
#endif
	if(write_table[bank].dev != NULL) {
		write_table[bank].dev->write_data8(realaddr, data);
	} else if(write_table[bank].memory != NULL) {
		write_table[bank].memory[realaddr] = (uint8)data;
	}
}

void FM7_MAINMEM::write_data8(uint32 addr, uint32 data)
{
	uint32 realaddr;
	int bank;
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
				mmr_map_data[addr - FM7_MAINIO_MMR_BANK] = (uint8)data;
			}
			break;
		}
		return;
	}
#endif
	write_data8_main(addr, data, false);
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
	s = emu->bios_path((const _TCHAR *)name);
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
	s = emu->bios_path((const _TCHAR *)name);
	if(s == NULL) return 0;
  
	fio.Fopen(s, FILEIO_WRITE_BINARY);
	blocks = fio.Fwrite(ptr, size, 1);
	fio.Fclose();

	return blocks * size;
}

void FM7_MAINMEM::update_config()
{
	setclock(config.cpu_type);
}

FM7_MAINMEM::FM7_MAINMEM(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
{
	p_vm = parent_vm;
	p_emu = parent_emu;
#if !defined(_FM77AV_VARIANTS)
	for(int i = 0; i < 4; i++) fm7_bootroms[i] = (uint8 *)malloc(0x200);
#endif	
	mainio = NULL;
	display = NULL;
	maincpu = NULL;
#if defined(CAPABLE_DICTROM)
	kanjiclass1 = NULL;
#endif	
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX)  || \
	defined(_FM77_VARIANTS)
	fm7_mainmem_extram = NULL;
#endif
	// Initialize table
	memset(read_table, 0x00, sizeof(read_table));
	memset(write_table, 0x00, sizeof(write_table));
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
	
	
#if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || \
	defined(_FM77_VARIANTS)
	extram_pages = FM77_EXRAM_BANKS;
#if defined(_FM77_VARIANTS)
	if(extram_pages > 3) extram_pages = 3;
#else
	if(extram_pages > 12) extram_pages = 12;
#endif 
	if(extram_pages > 0) {
		i = FM7_MAINMEM_EXTRAM;
		fm7_mainmem_extram = (uint8 *)malloc(extram_pages * 0x10000);
		if(fm7_mainmem_extram != NULL) {
			memset(fm7_mainmem_extram, 0x00, extram_pages * 0x10000);
			read_table[i].memory = fm7_mainmem_extram;
			write_table[i].memory = fm7_mainmem_extram;
		}
	}
#endif	

#if defined(_FM77_VARIANTS)
	memset(fm77_shadowram, 0x00, 0x200);
	read_table[FM7_MAINMEM_SHADOWRAM].memory = fm77_shadowram;
	write_table[FM7_MAINMEM_SHADOWRAM].memory = fm77_shadowram;
#endif
#if !defined(_FM77AV_VARIANTS)	
	for(i = FM7_MAINMEM_BOOTROM_BAS; i <= FM7_MAINMEM_BOOTROM_EXTRA; i++) {
		 memset(fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS], 0xff, 0x200);
		 read_table[i].memory = fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS];
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
# elif defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
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
#  if defined(_FM77_VARIANTS)
	if(read_bios("BOOT_MMR.ROM", fm7_bootroms[2], 0x200) >= 0x1e0) {
		diag_load_bootrom_mmr = true;
	} else {
		diag_load_bootrom_mmr = false;
	}
   
	i = FM7_MAINMEM_BOOTROM_RAM;
	memset(fm7_bootram, 0x00, 0x200 * sizeof(uint8)); // RAM
	read_table[i].memory = fm7_bootram;
	write_table[i].memory = fm7_bootram;
#  else
       // FM-7/8
	diag_load_bootrom_mmr = false;
#  endif
# elif defined(_FM77AV_VARIANTS)
	i = FM7_MAINMEM_AV_PAGE0;
	memset(fm7_mainmem_mmrbank_0, 0x00, 0x10000 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_mmrbank_0;
	write_table[i].memory = fm7_mainmem_mmrbank_0;
	
	i = FM7_MAINMEM_AV_PAGE2;
	memset(fm7_mainmem_mmrbank_2, 0x00, 0x10000 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_mmrbank_2;
	write_table[i].memory = fm7_mainmem_mmrbank_2;
	
	i = FM7_MAINMEM_INITROM;
	diag_load_initrom = false;
	memset(fm7_mainmem_initrom, 0xff, 0x2000 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_initrom;

	if(read_bios("INITIATE.ROM", read_table[i].memory, 0x2000) >= 0x2000) diag_load_initrom = true;
	emu->out_debug_log("77AV INITIATOR ROM READING : %s", diag_load_initrom ? "OK" : "NG");

	read_table[FM7_MAINMEM_BOOTROM_BAS].memory = NULL; // Not connected.
	read_table[FM7_MAINMEM_BOOTROM_DOS].memory = NULL; // Not connected.
	read_table[FM7_MAINMEM_BOOTROM_MMR].memory = NULL; // Not connected.
	
	if(read_bios("BOOT_MMR.ROM", fm77av_hidden_bootmmr, 0x200) < 0x1e0) {
		memcpy(fm77av_hidden_bootmmr, &fm7_mainmem_initrom[0x1a00], 0x200);
	}
	read_table[FM7_MAINMEM_BOOTROM_MMR].memory = fm77av_hidden_bootmmr; // Not connected.
	fm77av_hidden_bootmmr[0x1fe] = 0xfe;
	fm77av_hidden_bootmmr[0x1fe] = 0x00;
	
	i = FM7_MAINMEM_BOOTROM_RAM;
	memset(fm7_bootram, 0x00, 0x200 * sizeof(uint8)); // RAM
	read_table[i].memory = fm7_bootram;
	write_table[i].memory = fm7_bootram;
	
	if(diag_load_initrom) diag_load_bootrom_bas = true;
	if(diag_load_initrom) diag_load_bootrom_dos = true;
	
	if((config.boot_mode & 0x03) == 0) {
		memcpy(fm7_bootram, &fm7_mainmem_initrom[0x1800], 0x1e0 * sizeof(uint8));
	} else {
		memcpy(fm7_bootram, &fm7_mainmem_initrom[0x1a00], 0x1e0 * sizeof(uint8));
	}
	fm7_bootram[0x1fe] = 0xfe; // Set reset vector.
	fm7_bootram[0x1ff] = 0x00; //
	// FM-7
#endif
	emu->out_debug_log("BOOT ROM (basic mode) READING : %s", diag_load_bootrom_bas ? "OK" : "NG");
	emu->out_debug_log("BOOT ROM (DOS   mode) READING : %s", diag_load_bootrom_dos ? "OK" : "NG");
#if defined(_FM77_VARIANTS)
	emu->out_debug_log("BOOT ROM (MMR   mode) READING : %s", diag_load_bootrom_mmr ? "OK" : "NG");
#endif


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
   
	i = FM7_MAINMEM_BASICROM;
	memset(fm7_mainmem_basicrom, 0xff, 0x7c00 * sizeof(uint8));

	read_table[i].memory = fm7_mainmem_basicrom;
#if !defined(_FM8)
	if(read_bios("FBASIC302.ROM", fm7_mainmem_basicrom, 0x7c00) == 0x7c00) {
		diag_load_basicrom = true;
	} else if(read_bios("FBASIC300.ROM", fm7_mainmem_basicrom, 0x7c00) == 0x7c00) {
		diag_load_basicrom = true;
	} else if(read_bios("FBASIC30.ROM", fm7_mainmem_basicrom, 0x7c00) == 0x7c00) {
		diag_load_basicrom = true;
	}
   
#else // FM8
	if(read_bios("FBASIC10.ROM", fm7_mainmem_basicrom, 0x7c00) == 0x7c00) diag_load_basicrom = true;
#endif	
	emu->out_debug_log("BASIC ROM READING : %s", diag_load_basicrom ? "OK" : "NG");
   
	i = FM7_MAINMEM_BIOSWORK;
	memset(fm7_mainmem_bioswork, 0x00, 0x80 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_bioswork;
	write_table[i].memory = fm7_mainmem_bioswork;
#if defined(CAPABLE_DICTROM)
	diag_load_dictrom = false;
	i = FM7_MAINMEM_DICTROM;
	memset(fm7_mainmem_dictrom, 0xff, 0x40000 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_dictrom;
	if(read_bios("DICROM.ROM", fm7_mainmem_dictrom, 0x40000) == 0x40000) diag_load_dictrom = true;
	emu->out_debug_log("DICTIONARY ROM READING : %s", diag_load_dictrom ? "OK" : "NG");
	dictrom_connected = diag_load_dictrom;
	
	i = FM7_MAINMEM_BACKUPED_RAM;
	diag_load_learndata = false;
	memset(fm7_mainmem_learndata, 0x00, 0x2000 * sizeof(uint8));
	read_table[i].memory = fm7_mainmem_learndata;
	write_table[i].memory = fm7_mainmem_learndata;
	if(read_bios("USERDIC.DAT", read_table[i].memory, 0x2000) == 0x2000) diag_load_learndata = true;
	emu->out_debug_log("DICTIONARY BACKUPED RAM READING : %s", diag_load_learndata ? "OK" : "NG");
	if(!diag_load_learndata) write_bios("USERDIC.DAT", fm7_mainmem_learndata, 0x2000);
#endif
	
 	i = FM7_MAINMEM_77AV40_EXTRAROM;
#if defined(_FM77AV40SX) || defined(_FM77AV40EX)
	diag_load_extrarom = false;
	memset(fm7_mainmem_extrarom, 0xff, sizeof(fm7_mainmem_extrarom));
	read_table[i].memory = fm7_mainmem_extrarom;
	if(read_bios("EXTSUB.ROM", read_table[i].memory, 0xc000) == 0xc000) diag_load_extrarom = true;
	emu->out_debug_log("AV40SX/EX EXTRA ROM READING : %s", diag_load_extrarom ? "OK" : "NG");
#endif
}

void FM7_MAINMEM::release()
{
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || \
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
	write_bios("USERDIC.DAT", fm7_mainmem_learndata, 0x2000);
#endif
//	MEMORY::release();
}

#define STATE_VERSION 2
void FM7_MAINMEM::save_state(FILEIO *state_fio)
{
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);

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
	
# if defined(_FM77AV40SX) || defined(_FM77AV40EX)
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
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || \
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
	uint32 version;
	version = state_fio->FgetUint32_BE();
	if(this_device_id != state_fio->FgetInt32_BE()) return false;
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
	
# if defined(_FM77AV40SX) || defined(_FM77AV40EX)
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
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || \
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
	if(version >= 2) { // V2;
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
	return true;
}
