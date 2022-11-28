/*
 * Main memory for FM-7 [FM7_MAINMEM/MAINMEM_READSEQ]
 *  Author: K.Ohta
 *  Date  : 2017.04.01-
 *  License: GPLv2
 *
 */
#include "../vm.h"
#include "../../emu.h"
#include "fm7_mainmem.h"
#include "fm7_mainio.h"
#include "fm7_display.h"
#include "kanjirom.h"

uint8_t FM7_MAINMEM::read_data_tbl(uint32_t addr, bool dmamode)
{
	uint32_t paddr = addr >> 7;
	if(data_table[paddr].read_data != NULL) {
		return data_table[paddr].read_data[addr & 0x7f];
	} else if(data_table[paddr].read_func != NULL) {
		uint8_t (FM7_MAINMEM::*read_func)(uint32_t, bool);
		read_func = this->data_table[paddr].read_func;
		return (this->*read_func)(addr, dmamode);
	}
	return 0xff;
}		

uint8_t FM7_MAINMEM::read_data(uint32_t addr, bool dmamode)
{
#ifdef HAS_MMR
	int stat;
	if(window_enabled) {
		uint32_t raddr;
		stat = window_convert(addr, &raddr);
#if defined(_FM77AV_VARIANTS)
		if(stat >= 0) {
			return fm7_mainmem_mmrbank_0[raddr & 0xffff];
		}
#elif defined(_FM77_VARIANTS)
		if(stat >= 0) {
			if((extram_pages >= 3) && (fm7_mainmem_extram != NULL) && (extram_connected)) {
				return fm7_mainmem_extram[raddr];
			} else {
				return 0xff;
			}
		}
#endif
	}
	if(mmr_enabled) {
		uint32_t segment = 0x00;
		uint32_t raddr = (addr & 0x0fff);
		uint32_t mmr_bank;
		if(addr < 0xfc00) {
			if(!dmamode) segment = mmr_segment;
#if 1
			return read_with_mmr(addr, segment, dmamode);
#else
			if(!mmr_extend) {
				mmr_bank = mmr_map_data[(addr >> 12) & 0x000f | ((segment & 0x03) << 4)] & 0x003f;
			} else {
				mmr_bank = mmr_map_data[(addr >> 12) & 0x000f | ((segment & 0x07) << 4)];
			}		
			// Reallocated by MMR
			// Bank 3x : Standard memories.
			if(mmr_bank != 0x3f){
				raddr = (mmr_bank << 12) | raddr;
				return read_data_tbl(raddr, dmamode);
			}
# ifdef _FM77AV_VARIANTS
			else if(mmr_bank == 0x3f) {
				if((raddr >= 0xd80) && (raddr <= 0xd97)) { // MMR AREA
					iowait(); // OK?
					return 0xff;
				} else {
					raddr = raddr | 0x3f000;
					return read_data_tbl(raddr, dmamode);
				}
			}
# elif defined(_FM77_VARIANTS)
			else if(mmr_bank == 0x3f) {
				if((raddr >= 0xc00) && (raddr < 0xe00)) {
					if(is_basicrom) {
						return 0x00;
					} else {
						raddr = raddr - 0xc00;
						return fm77_shadowram[raddr];
					}
				} else if(raddr >= 0xe00) { // Note: Is this right sequence?
					raddr = raddr - 0x0e00;
					if(is_basicrom) {
						if(diag_load_bootrom_mmr) {
							return fm7_bootroms[2][raddr];
						} else {
							return fm7_bootroms[0][raddr];
						}
					} else {
						return fm7_bootram[raddr];
					}
				} else {
					raddr = raddr | 0x3f000;
					return read_data_tbl(raddr, dmamode);
				} 
			}
# endif
#endif
		} else {
			raddr = 0x30000 | (addr & 0xffff);
			return read_data_tbl(raddr, dmamode);
		}
	}
#endif
	
#if !defined(_FM77AV_VARIANTS) && !defined(_FM77_VARIANTS)
	uint32_t raddr = (addr & 0xffff);
	return read_data_tbl(raddr, dmamode);
#else
	uint32_t raddr = (addr & 0xffff) | 0x30000;
	return read_data_tbl(raddr, dmamode);
#endif
}

uint32_t FM7_MAINMEM::read_data8_main(uint32_t addr, bool dmamode)
{
	uint32_t realaddr;
	int bank;
#ifdef _FM77AV_VARIANTS
	if(initiator_enabled) {
		if((addr >= 0x6000) && (addr < 0x8000)) {
			uint32_t raddr = addr - 0x6000;
			iowait();
			return fm7_mainmem_initrom[raddr];
		}
		if((addr >= 0xfffe) && (addr < 0x10000)) {
			uint32_t raddr = addr - 0xe000;
			//printf("%04x %02x\n", raddr, fm7_mainmem_initrom[raddr]);
			iowait();
			return fm7_mainmem_initrom[raddr];
		}
	}
#endif
	return read_data(addr, dmamode);
}	


uint8_t FM7_MAINMEM::read_shared_ram(uint32_t realaddr, bool dmamode)
{
	realaddr = realaddr & 0x7f;
	if(!sub_halted) return 0xff; // Not halt
	return call_read_data8(display, realaddr  + 0xd380); // Okay?
}

uint8_t FM7_MAINMEM::read_direct_access(uint32_t realaddr, bool dmamode)
{
#if defined(_FM77AV_VARIANTS)
	if(!sub_halted) return 0xff; // Not halt
	if(dmamode) {
		return call_read_dma_data8(display, realaddr & 0xffff); // Okay?
	} else {
		return call_read_data8(display, realaddr & 0xffff); // Okay?
	}
#else
	return 0xff;
#endif	
}

uint8_t FM7_MAINMEM::read_ura_basicrom(uint32_t addr, bool dmamode)
{
	addr = addr & 0x7fff;
	if (basicrom_fd0f) {
		return fm7_mainmem_basicrom[addr];
	}
	return fm7_mainmem_ura[addr];
}

uint8_t FM7_MAINMEM::read_mmio(uint32_t addr, bool dmamode)
{
	addr &= 0xff;
	iowait();
	if(mainio != NULL) {
		return call_read_data8(mainio, addr);
	}
	return 0xff;
}

uint8_t FM7_MAINMEM::read_bootrom(uint32_t addr, bool dmamode)
{
	addr = addr & 0x1ff;
	if(addr <  0x1e0) {

#if defined(_FM77AV_VARIANTS)
		if(initiator_enabled) iowait();
		return fm7_bootram[addr];
#elif defined(_FM77_VARIANTS)
		if(boot_ram_write) {
			return fm7_bootram[addr];
		}
		switch(bootmode) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			return fm7_bootroms[bootmode][addr];
			break;
		default:
			return fm7_bootroms[0][addr];
			break;
		}
#elif defined(_FM8)
		switch(bootmode) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			return fm7_bootroms[bootmode][addr];
			break;
		default:
			return fm7_bootroms[0][addr];
			break;
		}
#else
		switch(bootmode & 3) {
		case 0:
		case 1:
		case 2:
		case 3:
			return fm7_bootroms[bootmode][addr];
			break;
		default:
			return fm7_bootroms[0][addr];
			break;
		}
#endif
	} else if (addr < 0x1fe) { // VECTOR
		return fm7_mainmem_bootrom_vector[addr - 0x1e0];
	} else { // RESET VECTOR
#if defined(_FM77AV_VARIANTS)
		if(initiator_enabled) iowait();
		return fm7_bootram[addr];
#elif defined(_FM77_VARIANTS)
		iowait();
		if(boot_ram_write) {
			return fm7_bootram[addr];
		}
		return fm7_mainmem_reset_vector[addr & 1];
#else
		return fm7_mainmem_reset_vector[addr & 1];
#endif
	}
	return 0xff;
}
