/*
 * Main memory without MMR for FM-7 [FM7_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */

#ifndef _FM7_MAINMEM_H_
#define _FM7_MAINMEM_H_


#include "fm7_common.h"

#define MEMORY_BANK_SIZE 0x8000
#define MEMORY_ADDR_MAX (FM7_MAINMEM_END * MEMORY_BANK_SIZE)
#include "../memory.h"

class DEVICE;
class MEMORY;
class FM7_SHAREDRAM;
class FM7_MAIN_IO;
class FM7_DIPSW;


class FM7_MAINMEM : public MEMORY
{
   
 protected:
   uint8 fm7_mainmem_omote[0x8000];
   uint8 fm7_mainmem_ura[0x7c00];
   uint8 fm7_mainmem_basicrom[0x7c00];
   uint8 fm7_mainmem_bioswork[0x80];
   uint8 *fm7_bootroms[4];
   uint8 fm7_mainmem_bootrom_vector[0x0e]; // Without
   uint8 fm7_mainmem_resetvector[2] = {0xfe, 0x00}; // Reset vector. Addr = 0xfe00.
	
   bool diag_load_basicrom = false;
   bool diag_load_bootrom_bas = false;
   bool diag_load_bootrom_dos = false;

   virtual int getbank(uint32 addr, uint32 *realaddr);
 public:
   FM7_MAINMEM(VM* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
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
	read_table[i].dev = fm7_io_main;
	read_table[i].memory = NULL;
	write_table[i].dev = fm7_io_main;
	write_table[i].memory = NULL;

	for(i = FM7_MAINMEM_BOOTROM_BAS; i <= FM7_MAINMEM_BOOTROM_RAM; i++) {
	    fm7_bootroms[i - FM7_MAINMEM_BOOTROM_BAS] = malloc(0x200);
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
	memset(fm7_mainmem_bootrom_vector, 0x00, 0x0e);
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

     ~FM7_MAINMEM(void) {
	int i;
	for(i = 0; i < 4; i++) {
	   if(fm7_bootroms[i] != NULL) free(fm7_bootroms[i]);
	   fm7_bootroms[i] = NULL;
	}
	if(read_table != NULL) free(read_table);
	if(write_table != NULL) free(write_table);
	write_table = NULL;
	read_table = NULL;
     }
   
    virtual uint32 read_data8(uint32 addr);
    virtual void write_data8(uint32 addr, uint32 data);
    virtual uint32 read_data16(uint32 addr);
    virtual void write_data16(uint32 addr, uint32 data);
    virtual uint32 read_data32(uint32 addr);
    virtual void write_data32(uint32 addr, uint32 data);
    bool get_loadstat_basicrom(void){
         return diag_load_basicrom;
    }

   bool get_loadstat_bootrom_bas(void){
     return diag_load_bootrom_bas;
   }
   bool get_loadstat_bootrom_dos(void){
     return diag_load_bootrom_dos;
   }
}
