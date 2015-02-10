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
class FM7_MAINIO;

class FM7_MAINMEM : public MEMORY
{
 protected:
	EMU *p_emu;
	VM *p_vm;
   
	uint8 fm7_mainmem_omote[0x8000];
	uint8 fm7_mainmem_ura[0x7c00];
	uint8 fm7_mainmem_basicrom[0x7c00];
  	uint8 fm7_mainmem_bioswork[0x80];
	uint8 *fm7_bootroms[4];
	uint8 fm7_mainmem_bootrom_vector[0x1e]; // Without
	uint8 fm7_mainmem_resetvector[2] = {0xfe, 0x00}; // Reset vector. Addr = 0xfe00.
	
	bool diag_load_basicrom = false;
	bool diag_load_bootrom_bas = false;
	bool diag_load_bootrom_dos = false;
	
	virtual int getbank(uint32 addr, uint32 *realaddr);
	FM7_MAINIO *mainio;
	DEVICE *submem;
 public:
	FM7_MAINMEM(VM* parent_vm, EMU* parent_emu);
	~FM7_MAINMEM();
	virtual uint32 read_data8(uint32 addr);
	virtual void write_data8(uint32 addr, uint32 data);
	virtual uint32 read_data16(uint32 addr);
	virtual void write_data16(uint32 addr, uint32 data);
	virtual uint32 read_data32(uint32 addr);
	virtual void write_data32(uint32 addr, uint32 data);
	void initialize(void);

	bool get_loadstat_basicrom(void){
		return diag_load_basicrom;
	}
	bool get_loadstat_bootrom_bas(void){
		return diag_load_bootrom_bas;
	}
	bool get_loadstat_bootrom_dos(void){
		return diag_load_bootrom_dos;
	}
	void set_context_submem(DEVICE *p){
		submem = p;
	}
};

#endif
