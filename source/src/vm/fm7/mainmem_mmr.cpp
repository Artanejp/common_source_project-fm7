/*
 * Main memory for FM-7 [FM7_MAINMEM/MAINMEM_MMR]
 *  Author: K.Ohta
 *  Date  : 2017.04.01-
 *  License: GPLv2
 *
 */
#include "../vm.h"
#include "../../emu.h"
#include "fm7_mainmem.h"

int FM7_MAINMEM::window_convert(uint32_t addr, uint32_t *realaddr)
{
	uint32_t raddr = addr;
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

void FM7_MAINMEM::update_mmr_jumptable(uint32_t pos)
{
#if defined(HAS_MMR)
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)	
	if(pos >= 0x80) return;
# else
	if(pos >= 0x40) return;
# endif
	uint32_t n_pos = pos * (0x1000 / 0x80);
	uint32_t i;
	uint8_t  mmr_index = mmr_map_data[pos];
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)	
	uint32_t r_pos_ext = mmr_index * (0x1000 / 0x80);
# endif
	uint32_t r_pos_nor = (mmr_index & 0x3f) * (0x1000 / 0x80);
	uint32_t raddr_nor;
	uint32_t raddr_ext;
	raddr_ext = (mmr_index << 12);
	raddr_nor = ((mmr_index & 0x3f) << 12);
	
	for(i = 0; i < (0x1000 / 0x80); i++) {
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)	
		mmr_bank_table[n_pos] = mmr_index;
		mmr_baseaddr_table_ext[i + n_pos] = raddr_ext;
		if((mmr_index >= 0x40) && !(extram_connected)) {
			mmr_update_table_ext[i + n_pos].read_data = NULL;
			mmr_update_table_ext[i + n_pos].write_data = NULL;
			mmr_update_table_ext[i + n_pos].read_func = NULL;
			mmr_update_table_ext[i + n_pos].write_func = NULL;
		} else {			
			if(mmr_index != 0x3f) {
				mmr_update_table_ext[i + n_pos].read_data = data_table[r_pos_ext + i].read_data;
				mmr_update_table_ext[i + n_pos].write_data = data_table[r_pos_ext + i].write_data;
				mmr_update_table_ext[i + n_pos].read_func = data_table[r_pos_ext + i].read_func;
				mmr_update_table_ext[i + n_pos].write_func = data_table[r_pos_ext + i].write_func;
			} else {
				mmr_update_table_ext[i + n_pos].read_data = NULL;
				mmr_update_table_ext[i + n_pos].write_data = NULL;
				mmr_update_table_ext[i + n_pos].read_func = &FM7_MAINMEM::read_segment_3f;
				mmr_update_table_ext[i + n_pos].write_func = &FM7_MAINMEM::write_segment_3f;
			}
		}
# endif
		mmr_baseaddr_table_nor[i + n_pos] = raddr_nor;
		
# if defined(_FM77_VARIANTS)
		if(mmr_index < 0x30) {
			if(!extram_connected) {
				mmr_update_table_nor[i + n_pos].read_data = NULL;
				mmr_update_table_nor[i + n_pos].write_data = NULL;
				mmr_update_table_nor[i + n_pos].read_func = NULL;
				mmr_update_table_nor[i + n_pos].write_func = NULL;
				return;
			}
		}
# endif
		if(mmr_index != 0x3f) {
			mmr_update_table_nor[i + n_pos].read_data = data_table[r_pos_nor + i].read_data;
			mmr_update_table_nor[i + n_pos].write_data = data_table[r_pos_nor + i].write_data;
			mmr_update_table_nor[i + n_pos].read_func = data_table[r_pos_nor + i].read_func;
			mmr_update_table_nor[i + n_pos].write_func = data_table[r_pos_nor + i].write_func;
		} else {
			mmr_update_table_nor[i + n_pos].read_data = NULL;
			mmr_update_table_nor[i + n_pos].write_data = NULL;
			mmr_update_table_nor[i + n_pos].read_func = &FM7_MAINMEM::read_segment_3f;
			mmr_update_table_nor[i + n_pos].write_func = &FM7_MAINMEM::write_segment_3f;
		}
	}
#endif
}

void FM7_MAINMEM::update_all_mmr_jumptable(void)
{
#if defined(HAS_MMR)
	uint32_t i;
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)	
	for(i = 0; i < 0x80; i++) {
		update_mmr_jumptable(i);
	}
# else
	for(i = 0; i < 0x40; i++) {
		update_mmr_jumptable(i);
	}
# endif
#endif
}

uint8_t FM7_MAINMEM::read_segment_3f(uint32_t addr, bool dmamode)
{
#if defined(HAS_MMR)
	uint32_t raddr = addr & 0x0fff;
# ifdef _FM77AV_VARIANTS
	if((raddr >= 0xd80) && (raddr <= 0xd97)) { // MMR AREA
		return 0xff;
	} else {
		raddr = raddr | 0x3f000;
		return read_data_tbl(raddr, dmamode);
	}
# elif defined(_FM77_VARIANTS)
	if((raddr >= 0xc00) && (raddr < 0xe00)) {
		if(is_basicrom) {
			return 0x00;
		} else {
			raddr = raddr - 0xc00;
			return fm77_shadowram[raddr];
		}
	} else if(raddr >= 0xe00) {
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
# endif
#else
	return 0xff;
#endif
}

void FM7_MAINMEM::write_segment_3f(uint32_t addr, uint32_t data, bool dmamode)
{
#if defined(HAS_MMR)
	uint32_t raddr = addr & 0x0fff;
# ifdef _FM77AV_VARIANTS
	if((raddr >= 0xd80) && (raddr <= 0xd97)) { // MMR AREA
		return;
	} else {
		raddr = raddr | 0x3f000;
		write_data_tbl(raddr, data, dmamode);
		return;
	}
# elif defined(_FM77_VARIANTS)
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
		if(!is_basicrom) {
			fm7_bootram[raddr] = (uint8_t)data;
		}
	} else {
		raddr = raddr | 0x3f000;
		write_data_tbl(raddr, data, dmamode);
	} 
# endif
#else
	return;
#endif
}

uint8_t FM7_MAINMEM::read_with_mmr(uint32_t addr, uint32_t segment, uint32_t dmamode)
{
#if defined(HAS_MMR)
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint32_t raddr;
	uint32_t n_pos;
	if(!mmr_extend) {
		//n_pos = (segment & 0x03) * (0x10 * 0x1000 / 0x80) + ((addr & 0xffff) >> 7); 
		n_pos = ((segment & 0x03) << 9)  + ((addr & 0xffff) >> 7); 
		if(mmr_update_table_nor[n_pos].read_data != NULL) {
			return mmr_update_table_nor[n_pos].read_data[addr & 0x7f];
		} else if(mmr_update_table_nor[n_pos].read_func != NULL) {
			uint8_t (FM7_MAINMEM::*read_func)(uint32_t, bool);
			read_func = this->mmr_update_table_nor[n_pos].read_func;
			raddr = mmr_baseaddr_table_nor[n_pos] | (addr & 0xfff);
			return (this->*read_func)(raddr, dmamode);
		}
		return 0xff;
	} else {
		//n_pos = (segment & 0x0f) * (0x10 * 0x1000 / 0x80) + ((addr & 0xffff) >> 7); 
		n_pos = ((segment & 0x0f) << 9)  + ((addr & 0xffff) >> 7); 
		if(mmr_update_table_ext[n_pos].read_data != NULL) {
			return mmr_update_table_ext[n_pos].read_data[addr & 0x7f];
		} else if(mmr_update_table_ext[n_pos].read_func != NULL) {
			uint8_t (FM7_MAINMEM::*read_func)(uint32_t, bool);
			raddr = mmr_baseaddr_table_ext[n_pos] | (addr & 0xfff);
			read_func = this->mmr_update_table_ext[n_pos].read_func;
			return (this->*read_func)(raddr, dmamode);
		}
		return 0xff;
	}		
# else
	uint32_t n_pos;
	uint32_t raddr;
	//n_pos = (segment & 0x03) * (0x10 * 0x1000 / 0x80) + ((addr & 0xffff) >> 7); 
	//n_pos = (((segment & 0x03) * 0x10) | ((addr >> 12) & 0x0f)) * (0x1000 / 0x80) + ((addr & 0xfff) >> 7);  
	n_pos = ((segment & 0x03) << 9)  + ((addr & 0xffff) >> 7); 
	if(mmr_update_table_nor[n_pos].read_data != NULL) {
		return mmr_update_table_nor[n_pos].read_data[addr & 0x7f];
	} else if(mmr_update_table_nor[n_pos].read_func != NULL) {
		uint8_t (FM7_MAINMEM::*read_func)(uint32_t, bool);
		read_func = this->mmr_update_table_nor[n_pos].read_func;
		raddr = mmr_baseaddr_table_nor[n_pos] | (addr & 0xfff);
		return (this->*read_func)(raddr, dmamode);
	}
	return 0xff;
# endif
#else
	return 0xff;
#endif
}

void FM7_MAINMEM::write_with_mmr(uint32_t addr, uint32_t segment, uint32_t data, uint32_t dmamode)
{
#if defined(HAS_MMR)
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	uint32_t n_pos;
	uint32_t raddr;
	if(!mmr_extend) {
		//n_pos = (segment & 0x03) * (0x10000 >> 7) + ((addr & 0xffff) >> 7); 
		n_pos = ((segment & 0x03) << 9)  + ((addr & 0xffff) >> 7); 
		if(mmr_update_table_nor[n_pos].write_data != NULL) {
			mmr_update_table_nor[n_pos].write_data[addr & 0x7f] = (uint8_t)data;
		} else if(mmr_update_table_nor[n_pos].write_func != NULL) {
			void (FM7_MAINMEM::*write_func)(uint32_t, uint32_t, bool);
			write_func = this->mmr_update_table_nor[n_pos].write_func;
			raddr = mmr_baseaddr_table_nor[n_pos] | (addr & 0xfff);
			(this->*write_func)(raddr, data, dmamode);
		}
		return;
	} else {
		//n_pos = (segment & 0x0f) * (0x10 * 0x1000 / 0x80) + ((addr & 0xffff) >> 7); 
		n_pos = ((segment & 0x0f) << 9)  + ((addr & 0xffff) >> 7); 
		if(mmr_update_table_ext[n_pos].write_data != NULL) {
			mmr_update_table_ext[n_pos].write_data[addr & 0x7f] = (uint8_t)data;
		} else if(mmr_update_table_ext[n_pos].write_func != NULL) {
			void (FM7_MAINMEM::*write_func)(uint32_t, uint32_t, bool);
			write_func = this->mmr_update_table_ext[n_pos].write_func;
			raddr = mmr_baseaddr_table_ext[n_pos] | (addr & 0xfff);
			(this->*write_func)(raddr, data, dmamode);
		}
		return;
	}		
# else
	uint32_t n_pos;
	uint32_t raddr;
	//n_pos = (segment & 0x03) * (0x10 * 0x1000 / 0x80) + ((addr & 0xffff) >> 7);
	//n_pos = (((segment & 0x03) * 0x10) | ((addr >> 12) & 0x0f)) * (0x1000 / 0x80) + ((addr & 0xfff) >> 7);  
	n_pos = ((segment & 0x03) << 9)  + ((addr & 0xffff) >> 7); 
	if(mmr_update_table_nor[n_pos].write_data != NULL) {
		mmr_update_table_nor[n_pos].write_data[addr & 0x7f] = (uint8_t)data;
	} else if(mmr_update_table_nor[n_pos].write_func != NULL) {
		void (FM7_MAINMEM::*write_func)(uint32_t, uint32_t, bool);
		write_func = this->mmr_update_table_nor[n_pos].write_func;
		raddr = mmr_baseaddr_table_nor[n_pos] | (addr & 0xfff);
		//printf("%08x %08x %08x\n", addr, raddr, n_pos);
		(this->*write_func)(raddr, data, dmamode);
	}
	return;
# endif
#else
	return;
#endif
}
