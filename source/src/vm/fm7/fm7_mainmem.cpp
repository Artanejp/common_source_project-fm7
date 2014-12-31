/*
 * Main memory without MMR for FM-7 [FM7_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */

#include "fm7_mainmem.h"

int FM7_MAINMEM::getbank(uint32 addr, uint32 *realaddr)
{
   if(realaddr == NULL) return -1; // Not effect.
   
   addr = addr & 0xffff;
   if(window != NULL) {
      if(window->isenabled() && ((addr >= 0x7c00) && (addr < 0x8000)) { // Window open
	 *realaddr = (addr + window->getwindow() * 256) & 0xffff;
	 if(window->is77av()) return FM7_MAINMEM_AV_PAGE0; // 77AV
	 return FM7_MAINMEM_TVRAM; // FM77
      }
   }
   if(mmr != NULL) {
      int  mmr_page;
      uint32 mmr_offset;
      if(!mmr->isenabled()) goto _fm7_0;
      if(addr < 0xfc00) { // MMR Banked
	 mmr_page = mmr->getpage(addr);
	 mmr_offset = addr & 0x0fff;
	 
	 if(mmr->is77av() != true) { // FM77
	    if(mmr_page < 0x30) {
	       if(extram != NULL) {
		  if(extram->isenabled()) { // EXTEND RAM, 192KB.
		     *realaddr = mmr_offset + mmr_page * 0x1000;
		     return FM7_MAINMEM_77EXTRAM;
		  }
	       }
	    } else if((mmr_page & 0x30) == 0x30) {
	       if(mmr_page == 0x3f) { // $3f000 - $3ffff
		    if(mmr_offset < 0x0c00) {
		       addr = mmr_offset + 0xf000;
		       goto _fm7_0;
		    }
		    if((mmr_offset < 0x0e00)) {
		       *realaddr = mmr_offset - 0x0c00;
		       if(mainio->get_rommode_fd0f() == true) {
			  return FM7_MAINMEN_77_NULLRAM;
		       }
		       return FM7_MAINMEM_77_SHADOWRAM;
		    }
		    addr = mmr_offset + 0xf000;
		    goto _fm7_0; // 
	       }
	       // $30000 - $3efff
	       addr = mmr_offset + (mmr_page & 0x0f) * 0x1000;
	       goto _fm7_0;
	    }
	   *realaddr = mmr_offset;
	   return -1; // Illegal
	 } else { // FM77AV
	    if((mmr_page & 0x30) == 0x30) { // 77AV RAM/ROM Page1
	       uint32 a;
	       a = (mmr_page & 0x0f) * 0x1000 + mmr_offset;
	       if((a < 0xfd80) && (a > 0xfd97)) { // Okay.
		  addr = a;
		  goto _fm7_0;
	       }
	       // MMR Area is illegal.
	       *realaddr = 0;
	       return -1;
	    }
	    if(mmr_page < 0x10) { // 77AV RAM PAGE0
	       *realaddr = mmr_page * 0x1000 + mmr_offset;
	       return FM7_MAINMEM_AV_PAGE0;
	    }
	    if(mmr_page < 0x20) { // 77AV VRAM
	       *realaddr = ((mmr_page & 0x0f) * 0x1000) + mmr_offset;
	       return FM7_MAINMEM_AV_SUBMEM;
	    }
	    if(mmr_page < 0x30) { // 77AV Jcard
	       if(jcard != NULL) {
		    if(jcard->isenabled()) {
		       *realaddr = ((mmr_page & 0x0f) * 0x1000) + mmr_offset;
		       return FM7_MAINMEM_AV_JCARD;
		    }
	       }
	       *realaddr = mmr_offset;
	       return -1; // Illegal
	    }
	    if(av_extram != NULL) {
		 if(av_extram->isenabled()) {
		    *realaddr = ((mmr_page - 0x40) * 0x1000) + mmr_offset;
		    return FM7_MAINMEM_AV_EXTRAM768K;
		 }
	    }
	    // Illegal Page
	    *realaddr = offset;
	    return -1;
	 }
      }
   }

_fm7_0:	    
      addr = addr & 0x0ffff;
      if(mainio->is_initiaterom()) { // Initiate rom enabled, force overwrite vector.
	if(addr >= 0xfffe) {
	   *realaddr = addr - 0xe000;
	   return FM7_MAINMEM_AV_INITROM;
	}
	if((addr >= 0x6000) && (addr < 0x8000)){ // Real addr: $6000 - $7fff
	   *realaddr = addr - 0x6000;
	   return FM7_MAINMEM_AV_INITROM;
	}
      }
   
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
	*realaddr = addr - 0xfd00;
	return FM7_MAINMEM_MMIO;
      }
      if(addr < 0xfff0) {
	*realaddr = addr - 0xfe00;
	if(mainio->get_boot_romram() != true) return FM7_MAINMEM_BOOTROM_RAM;
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
   
     if(addr < 0xfffe) {
	*realaddr = addr - 0xfff0;
	return FM7_MAINMEM_VECTOR;
     }
     if(addr < 0x10000) {
	*realaddr = addr - 0xfffe;
	return FM7_MAINMEM_VECTOR_RESET;
     }
     // Over
     realaddr = addr;
     return -1;
}

uint32 FM7_MAINMEM::read_data8(uint32 addr)
{
   uint32 ret;
   uint32 realaddr;
   int bank;

   bank = getbank(addr, &realaddr);
   if(bank < 0) return 0xff; // Illegal
   
   if(read_table[bank].dev != NULL) {
	return read_table[bank].dev->read_memory_mapped_io8(realaddr);
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
   
   if(write_table[bank].dev != NULL) {
	write_table[bank].dev->write_memory_mapped_io8(realaddr, data);
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

