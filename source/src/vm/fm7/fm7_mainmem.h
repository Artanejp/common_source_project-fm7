/*
 * Main memory without MMR for FM-7 [FM7_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */

#ifndef _FM7_MAINMEM_H_
#define _FM7_MAINMEM_H_

#define MEMORY_ADDR_MAX 0x0fc00
#define MEMORY_BANK_SIZE 0x8000
#include "../memory.h"

enum 
{
   FM7_MAINMEM_OMOTE = 0, // $0000-$7FFF
   FM7_MAINMEM_URA,       // $8000-$FBFF #1
   FM7_MAINMEM_BASICROM,  // $8000-$FBFF #2
   FM7_MAINMEM_BIOSWORK,  // $FC00-$FC7F
   FM7_MAINMEM_SHAREDRAM, // $FC80-$FCFF
   FM7_MAINMEM_MMIO, // $FD00-$FDFF
   FM7_MAINMEM_BOOTROM_BAS,  // $FE00-$FFEF #1
   FM7_MAINMEM_BOOTROM_DOS,  // $FE00-$FFEF #2
   FM7_MAINMEM_BOOTROM_ROM3, // $FE00-$FFEF #3
   FM7_MAINMEM_BOOTROM_RAM,  // $FE00-$FFEF #4
   FM7_MAINMEM_VECTOR, // $FFF0-$FFFD
   FM7_MAINMEM_VECTOR_RESET, // $FFFE-$FFFF
   
   FM7_MAINMEM_TVRAM, // TVRAM. Enabled only 77L4.
   FM7_MAINMEM_77EXTRAM, // 77AVL4, 192KB EXTRAM.
   FM7_MAINMEM_77_NULLRAM, // 0x00 
   FM7_MAINMEM_77_SHADOWRAM, // 0x200
   
   FM7_MAINMEM_AV_PAGE0,
   FM7_MAINMEM_AV_SUBMEM,
   FM7_MAINMEM_AV_JCARD,
   FM7_MAINMEM_AV_EXTRAM786K,
   FM7_MAINMEN_AV_INITROM,
   FM7_MAINMEM_END
};

#define FM7_BOOTMODE_BASIC 0
#define FM7_BOOTMODE_DOS   1
#define FM7_BOOTMODE_ROM3  2
#define FM7_BOOTMODE_RAM   4

class DEVICE;
class MEMORY;
class FM7_SHAREDRAM;
class FM7_MAIN_IO;
class FM7_DIPSW;
class FM7_MAIN_MMR;
class FM7_MAIN_WINDOW;

class FM7_MAINMEM : public MEMORY
{
   
 private:
   uint8 fm7_mainmem_omote[0x8000];
   uint8 fm7_mainmem_ura[0x7c00];
   uint8 fm7_mainmem_basicrom[0x7c00];
   uint8 fm7_mainmem_bioswork[0x80];
   uint8 *fm7_bootroms[4];
   uint8 fm7_mainmem_bootrom_vector[0x0e]; // Without
   uint8 fm7_mainmem_resetvector[2] = {0xfe, 0x00}; // Reset vector. Addr = 0xfe00.
	
   FM7_SHAREDRAM *sharedram;
   FM7_MAIN_IO   *mainio;
   FM7_DIPSW *dipsw;
   FM7_MAIN_MMR *mmr = NULL;
   FM7_MAIN_WINDOW *window = NULL;
   bool diag_load_basicrom = false;
   bool diag_load_bootrom_bas = false;
   bool diag_load_bootrom_dos = false;
   bool have_
   int getbank(uint32 addr, uint32 *realaddr);
 public:
   FM7_MAINMEM(VM* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
     {
	int bank_num = FM7_MAINMEM_END;
	int i;

	read_table = (bank_t *)malloc(sizeof(bank_t) * bank_num);
	write_table = (bank_t *)malloc(sizeof(bank_t) * bank_num);
	
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
   
    uint32 read_data8(uint32 addr);
    void write_data8(uint32 addr, uint32 data);
    uint32 read_data16(uint32 addr);
    void write_data16(uint32 addr, uint32 data);
    uint32 read_data32(uint32 addr);
    void write_data32(uint32 addr, uint32 data);
    bool get_loadstat_basicrom(void);
    bool get_loadstat_bootrom_bas(void);
    bool get_loadstat_bootrom_dos(void);
}
