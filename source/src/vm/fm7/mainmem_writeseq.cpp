/*
 * Main memory for FM-7 [FM7_MAINMEM/MAINMEM_WRITESEQ]
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

void FM7_MAINMEM::write_data_tbl(uint32_t addr, uint32_t data, bool dmamode)
{
	uint32_t paddr = addr >> 7;
	if(data_table[paddr].write_data != NULL) {
		data_table[paddr].write_data[addr & 0x7f] = (uint8_t)data;
		return;
	} else if(data_table[paddr].write_func != NULL) {
		void (FM7_MAINMEM::*write_func)(uint32_t, uint32_t, bool);
		write_func = this->data_table[paddr].write_func;
		(this->*write_func)(addr, data, dmamode);
	}
	return;
}		

void FM7_MAINMEM::write_shared_ram(uint32_t realaddr, uint32_t data, bool dmamode)
{
	realaddr = realaddr & 0x7f;
	if(!sub_halted) return; // Not halt
	return call_write_data8(display, realaddr  + 0xd380, data); // Okay?
}

void FM7_MAINMEM::write_direct_access(uint32_t realaddr, uint32_t data, bool dmamode)
{
#if defined(_FM77AV_VARIANTS)
	if(!sub_halted) return; // Not halt
	if(dmamode) {
		call_write_dma_data8(display, realaddr & 0xffff, data); // Okay?
	} else {
		call_write_data8(display, realaddr & 0xffff, data); // Okay?
	}
#else
	return;
#endif	
}

void FM7_MAINMEM::write_ura_basicrom(uint32_t addr, uint32_t data, bool dmamode)
{
	addr = addr & 0x7fff;
	if (basicrom_fd0f) {
		return;
	}
	fm7_mainmem_ura[addr] = (uint8_t)data;
	return;
}

void FM7_MAINMEM::write_mmio(uint32_t addr, uint32_t data, bool dmamode)
{
	addr &= 0xff;
	iowait();
	call_write_data8(mainio, addr, data);
	return;
}

void FM7_MAINMEM::write_bootrom(uint32_t addr, uint32_t data, bool dmamode)
{
	addr = addr & 0x1ff;
	if(addr <  0x1e0) {
		iowait();
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
		if(boot_ram_write) {
			fm7_bootram[addr] = (uint8_t)data;
		}
#endif
		return;
	} else if (addr < 0x1fe) { // VECTOR
		fm7_mainmem_bootrom_vector[addr - 0x1e0] = (uint8_t)data;
		return;
	}
	else {  // RESET VECTOR
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
		iowait();
		if(boot_ram_write) {
			fm7_bootram[addr] = (uint8_t)data;
		}
#endif
		return;
	}
	return;
}

void FM7_MAINMEM::write_data(uint32_t addr, uint32_t data, bool dmamode)
{
#ifdef HAS_MMR
	int stat;
	if(window_enabled) {
		uint32_t raddr;
		stat = window_convert(addr, &raddr);
#if defined(_FM77AV_VARIANTS)
		if(stat >= 0) {
			fm7_mainmem_mmrbank_0[raddr & 0xffff] = (uint8_t)data;
			return;
		}
#elif defined(_FM77_VARIANTS)
		if(stat >= 0) {
			if((extram_pages >= 3) && (fm7_mainmem_extram != NULL)) {
				fm7_mainmem_extram[raddr] = (uint8_t)data;;
			}
			return;
		}
#endif
	}
	if(mmr_enabled) {
		uint32_t segment = 0x00;
		uint32_t raddr = (addr & 0xfff);
		uint32_t mmr_bank;
		if(addr < 0xfc00) {
			if(!dmamode) segment = mmr_segment;
#if 1
			write_with_mmr(addr, segment, data, dmamode);
			return;
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
				write_data_tbl(raddr, data, dmamode);
				return;
			}
# ifdef _FM77AV_VARIANTS
			else if(mmr_bank == 0x3f) {
				if((raddr >= 0xd80) && (raddr <= 0xd97)) { // MMR AREA
					iowait(); // OK?
					return;
				} else {
					raddr = raddr | 0x3f000;
					write_data_tbl(raddr, data, dmamode);
					return;
				}
			}
# elif defined(_FM77_VARIANTS)
			else if(mmr_bank == 0x3f) {
				if((raddr >= 0xc00) && (raddr < 0xe00)) {
					if(is_basicrom) {
						return;
					} else {
						raddr = raddr - 0xc00;
						fm77_shadowram[raddr] = (uint8_t)data;
						return;
					}
				} else if(raddr >= 0xe00) {
					raddr = raddr - 0x0e00;
					if(is_basicrom) {
						return;
					} else {
						fm7_bootram[raddr] = (uint8_t)data;
						return;
					}
				} else {
					raddr = raddr | 0x3f000;
					write_data_tbl(raddr, data, dmamode);
					return;
				} 
			}
# endif
#endif
		} else {
			raddr = 0x30000 | (addr & 0xffff);
			write_data_tbl(raddr, data, dmamode);
		}
	}
# endif
#if !defined(_FM77AV_VARIANTS) && !defined(_FM77_VARIANTS)
	uint32_t raddr = (addr & 0xffff);
	write_data_tbl(raddr, data, dmamode);
#else
	uint32_t raddr = (addr & 0xffff) | 0x30000;
	write_data_tbl(raddr, data, dmamode);
#endif
}

void FM7_MAINMEM::write_data8_main(uint32_t addr, uint32_t data, bool dmamode)
{
#ifdef _FM77AV_VARIANTS
	if(initiator_enabled) {
		if((addr >= 0x6000) && (addr < 0x8000)) {
			iowait();
			//uint32_t raddr = addr - 0x6000;
			//return fm7_mainmen_initrom[raddr];
			return;
		}
		if((addr >= 0xfffe) && (addr < 0x10000)) {
			iowait();
			//uint32_t raddr = addr - 0xe000;
			//return fm7_mainmen_initrom[raddr];
			return;
		}
	}
#endif
	write_data(addr, data, dmamode);
}
