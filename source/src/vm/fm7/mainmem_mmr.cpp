/*
 * Main memory for FM-7 [FM7_MAINMEM/MAINMEM_MMR]
 *  Author: K.Ohta
 *  Date  : 2017.04.01-
 *  License: GPLv2
 *
 */
#include "vm.h"
#include "emu.h"
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
