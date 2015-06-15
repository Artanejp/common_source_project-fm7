/*
 * Main memory without MMR for FM-7 [FM7_MAINMEM]
 *  Author: K.Ohta
 *  Date  : 2015.01.01-
 *
 */

#ifndef _FM7_MAINMEM_H_
#define _FM7_MAINMEM_H_

#include "fm7_common.h"

#include "../device.h"
#include "../mc6809.h"

class DEVICE;
class MEMORY;
class FM7_MAINIO;

class FM7_MAINMEM : public DEVICE
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
 protected:
	EMU *p_emu;
	VM *p_vm;
   
	uint8 fm7_mainmem_omote[0x8000];
	uint8 fm7_mainmem_ura[0x7c00];
	uint8 fm7_mainmem_basicrom[0x7c00];
  	uint8 fm7_mainmem_bioswork[0x80];
#if !defined(_FM77AV_VARIANTS)
	uint8 *fm7_bootroms[4];
#endif	
	uint8 fm7_mainmem_bootrom_vector[0x1e]; // Without
	uint8 fm7_mainmem_reset_vector[2]; // Without
	uint8 fm7_mainmem_null[1];
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	uint8 fm7_bootram[0x200]; // $00000-$0ffff
#endif
#ifdef HAS_MMR
	bool extram_connected;
# ifdef _FM77AV_VARIANTS
	bool diag_load_initrom;
	bool diag_load_dictrom;
	bool diag_load_learndata;
	bool dictrom_connected;

	bool use_page2_extram;
	uint8 fm7_mainmem_initrom[0x2000]; // $00000-$0ffff
	uint8 fm7_mainmem_mmrbank_0[0x10000]; // $00000-$0ffff
	uint8 fm7_mainmem_mmrbank_2[0x10000]; // $20000-$2ffff
#  if defined(CAPABLE_DICTROM)
	bool diag_load_extrarom;
	uint8 fm7_mainmem_extrarom[0x20000]; // $20000-$2ffff, banked
	uint8 fm7_mainmem_dictrom[0x40000]; // $20000-$2ffff, banked
	uint8 fm7_mainmem_learndata[0x2000];
#  endif	
#  if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV20)
	int extram_pages;
	uint8 *fm7_mainmem_extram; // $40000- : MAX 768KB ($c0000)
#  endif
# endif
# if defined(_FM77_VARIANTS)
	int extram_pages;
	uint8 *fm7_mainmem_extram; // $00000-$2ffff
	uint8 fm77_shadowram[0x200];
# endif
#endif
	KANJIROM *kanjiclass1;
	KANJIROM *kanjiclass2;
	MC6809 *maincpu;
	DEVICE *mainio;
	DEVICE *display;
	
	bool diag_load_basicrom;
	bool diag_load_bootrom_bas;
	bool diag_load_bootrom_dos;
	bool diag_load_bootrom_mmr;

	int getbank(uint32 addr, uint32 *realaddr);
	int window_convert(uint32 addr, uint32 *realaddr);
	int mmr_convert(uint32 addr, uint32 *realaddr);
	int nonmmr_convert(uint32 addr, uint32 *realaddr);
	uint32 read_bios(const char *name, uint8 *ptr, uint32 size);
	uint32 write_bios(const char *name, uint8 *ptr, uint32 size);

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
	void release(void);

	bool get_loadstat_basicrom(void);
	bool get_loadstat_bootrom_bas(void);
	bool get_loadstat_bootrom_dos(void);
	void save_state(FILEIO *state_fio);
	bool load_state(FILEIO *state_fio);

	void set_context_display(DEVICE *p){
		int i;  
		display = p;
		i = FM7_MAINMEM_SHAREDRAM;
		read_table[i].dev = display;
		write_table[i].dev = display;
	
#if defined(_FM77AV_VARIANTS)
		i = FM7_MAINMEM_AV_DIRECTACCESS;
		read_table[i].dev = display;
		write_table[i].dev = display;
#endif
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
	uint32 read_io8(uint32 addr) {
		return mainio->read_io8(addr);
	}
};

#endif
