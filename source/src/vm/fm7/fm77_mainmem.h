/*
 * Main memory with MMR for FM-77 [FM77_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */

#ifndef _FM77_MAINMEM_H_
#define _FM77_MAINMEM_H_

#include "fm7_mainmem.h"

class FM77_MAIN_MMR;
class FM77_MAIN_WINDOW;

class FM77_MAINMEM : public FM7_MAINMEM
{
 private:
   uint8 fm77_extram_192k[0x30000];
   uint8 fm77_shadowram[0x200];
   
   FM77_MAIN_MMR *mmr = NULL;
   FM77_MAIN_WINDOW *window = NULL;
   bool use_extram = false;

   int getbank(uint32 addr, uint32 *realaddr);
 public:
   FM77_MAINMEM(VM* parent_vm, EMU* parent_emu) : FM7_MAINMEM(parent_vm, parent_emu)
     {
       int bank_num;
	int i;

	i = FM77_MAINMEM_77EXTRAM;
	memset(fm77_extram_192k, 0x00, 0x30000 * sizeof(uint8));
        read_table[FM7_MAINMEM_77EXTRAM].memory = fm77_extram_192k;
        write_table[FM7_MAINMEM_77EXTRAM].memory = fm77_extram_192k;
	read_table[FM7_MAINMEM_77EXTRAM].dev = NULL;
	write_table[FM7_MAINMEM_77EXTRAM].dev = NULL;

	i = FM77_MAINMEM_77SHADOWRAM;
	memset(fm77_extram_192k, 0x00, 0x200 * sizeof(uint8));
        read_table[FM7_MAINMEM_77EXTRAM].memory = fm77_shadowram;
        write_table[FM7_MAINMEM_77EXTRAM].memory = fm77_shadowram;
	read_table[FM7_MAINMEM_77EXTRAM].dev = NULL;
	write_table[FM7_MAINMEM_77EXTRAM].dev = NULL;
	

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
