/*
 * Main memory without MMR for FM-7 [FM7_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */

#ifndef _FM7_MAINMEM_H_
#define _FM7_MAINMEM_H_

#include "fm7_common.h"

#include "../memory.h"
#include "../mc6809.h"
#define MEMORY_ADDR_MAX  0x20000
#define MEMORY_BANK_SIZE 0x20000
#define MEMORY_BANK_SIZE 0x10
#define MEMORY_ADDR_MAX (FM7_MAINMEM_END * MEMORY_BANK_SIZE)

class DEVICE;
class MEMORY;
class FM7_MAINIO;

class FM7_MAINMEM : public MEMORY
{
 private:
	typedef struct {
		DEVICE* dev;
		uint8* memory;
		int wait;
	} bank_t;
	bank_t read_table[FM7_MAINMEM_END];
	bank_t write_table[FM7_MAINMEM_END];
	bool ioaccess_wait;
	int waitfactor;
	int waitcount;
	bool sub_halted;
	bool first_pass;
	bool flag_debug;
 protected:
	EMU *p_emu;
	VM *p_vm;
   
	uint8 fm7_mainmem_omote[0x8000];
	uint8 fm7_mainmem_ura[0x7c00];
	uint8 fm7_mainmem_basicrom[0x7c00];
  	uint8 fm7_mainmem_bioswork[0x80];
	uint8 *fm7_bootroms[4];
	uint8 fm7_mainmem_bootrom_vector[0x1e]; // Without
	uint8 fm7_mainmem_reset_vector[2]; // Without
	uint8 fm7_mainmem_null[1];

#ifdef HAS_MMR
#ifdef _FM77AV_VARIANTS
	bool diag_load_initrom;
	bool diag_load_dictrom;
	
	uint8 fm7_mainmem_initrom[0x2000]; // $00000-$0ffff
	uint8 fm7_mainmem_mmrbank_0[0x10000]; // $00000-$0ffff
	uint8 fm7_mainmem_mmrbank_2[0x10000]; // $20000-$2ffff 
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV20)
	bool diag_load_extrarom;
	uint8 *fm7_mainmem_extram; // $40000- : MAX 768KB ($c0000)
	uint8 fm7_mainmem_extrarom[0x20000]; // $20000-$2ffff, banked
	uint8 fm7_mainmem_dictrom[0x20000]; // $20000-$2ffff, banked
# endif
#else
	uint8 *fm7_mainmem_extram; // $00000-$2ffff
#endif
#endif
	KANJIROM *kanjiclass1;
	KANJIROM *kanjiclass2;
	MC6809 *maincpu;
	
	bool diag_load_basicrom;
	bool diag_load_bootrom_bas;
	bool diag_load_bootrom_dos;
	bool diag_load_bootrom_mmr;

	bool extram_connected;
	uint8 extram_pages; // Per 64KB, MAX=3(77) or 12(77AV40)
	int getbank(uint32 addr, uint32 *realaddr);
	int window_convert(uint32 addr, uint32 *realaddr);
	int mmr_convert(uint32 addr, uint32 *realaddr);
	int nonmmr_convert(uint32 addr, uint32 *realaddr);
	uint32 read_bios(const char *name, uint8 *ptr, uint32 size);

	DEVICE *mainio;
	DEVICE *display;
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
	void wait(void);
	void reset(void);

	bool get_loadstat_basicrom(void);
	bool get_loadstat_bootrom_bas(void);
	bool get_loadstat_bootrom_dos(void);

	void set_context_display(DEVICE *p){
		display = p;
	}
	void set_context_maincpu(MC6809 *p){
		maincpu = p;
	}
	void set_context_mainio(DEVICE *p){
		int i;
		mainio = p;
		i = FM7_MAINMEM_MMIO;
		read_table[i].dev = mainio;
		read_table[i].memory = NULL;
		write_table[i].dev = mainio;
		write_table[i].memory = NULL;
		
	}
	void write_signal(int sigid, uint32 data, uint32 mask);
};

#endif
