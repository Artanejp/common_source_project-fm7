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
      if(addr < 0xfff0) {
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
   
     if(addr < 0xfffe) {
	*realaddr = addr - 0xfff0;
	return FM7_MAINMEM_VECTOR;
     }
     if(addr < 0x10000) {
        mainio->wait();
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

