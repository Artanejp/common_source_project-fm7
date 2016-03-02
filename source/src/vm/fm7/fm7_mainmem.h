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
		uint8_t* memory;
		int wait;
	} bank_t;
	bank_t read_table[FM7_MAINMEM_END];
	bank_t write_table[FM7_MAINMEM_END];
	bool ioaccess_wait;
	int waitfactor;
	int waitcount;
	bool sub_halted;
	// V2
#ifdef HAS_MMR
	bool window_enabled;
	bool window_fast;
	bool mmr_enabled;
	bool mmr_fast;
	bool mmr_extend;
	bool refresh_fast;
	uint16_t window_offset;
	uint8_t mmr_segment;
	uint8_t mmr_map_data[0x80];
#endif
	bool is_basicrom;
	bool clockmode;
	bool basicrom_fd0f;
	uint32_t bootmode;
#ifdef _FM77AV_VARIANTS
	uint32_t extcard_bank;
	bool extrom_bank;
	bool initiator_enabled;
#endif
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	bool boot_ram_write;
#endif
 protected:
	EMU *p_emu;
	VM *p_vm;
   
	uint8_t fm7_mainmem_omote[0x8000];
	uint8_t fm7_mainmem_ura[0x7c00];
	uint8_t fm7_mainmem_basicrom[0x7c00];
  	uint8_t fm7_mainmem_bioswork[0x80];
#if !defined(_FM77AV_VARIANTS)
	uint8_t *fm7_bootroms[4];
#endif	
	uint8_t fm7_mainmem_bootrom_vector[0x1e]; // Without
	uint8_t fm7_mainmem_reset_vector[2]; // Without
	uint8_t fm7_mainmem_null[1];
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	uint8_t fm7_bootram[0x200]; // $00000-$0ffff
#endif
#ifdef HAS_MMR
	bool extram_connected;
# ifdef _FM77AV_VARIANTS
	bool diag_load_initrom;
	bool diag_load_dictrom;
	bool diag_load_learndata;
	bool dictrom_connected;
	bool dictrom_enabled;
	bool dictram_enabled;
	
	bool use_page2_extram;
	uint8_t fm7_mainmem_initrom[0x2000]; // $00000-$0ffff
	uint8_t fm77av_hidden_bootmmr[0x200];
	uint8_t fm7_mainmem_mmrbank_0[0x10000]; // $00000-$0ffff
	uint8_t fm7_mainmem_mmrbank_2[0x10000]; // $20000-$2ffff
#  if defined(CAPABLE_DICTROM)
	uint8_t fm7_mainmem_dictrom[0x40000]; // $00000-$3ffff, banked
	uint8_t fm7_mainmem_learndata[0x2000];
#  endif
#  if defined(_FM77AV40EX) || defined(_FM77AV40SX)
	bool diag_load_extrarom;
	uint8_t fm7_mainmem_extrarom[0x10000]; // $20000-$2bfff, banked
#  endif	
#  if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	int extram_pages;
	uint8_t *fm7_mainmem_extram; // $40000- : MAX 768KB ($c0000)
#  endif
# endif
# if defined(_FM77_VARIANTS)
	int extram_pages;
	uint8_t *fm7_mainmem_extram; // $00000-$2ffff
	uint8_t fm77_shadowram[0x200];
# endif
#endif
#if defined(CAPABLE_DICTROM)
	DEVICE *kanjiclass1;
	//KANJIROM *kanjiclass2;
#endif	
	MC6809 *maincpu;
	DEVICE *mainio;
	DEVICE *display;
	
	bool diag_load_basicrom;
	bool diag_load_bootrom_bas;
	bool diag_load_bootrom_dos;
	bool diag_load_bootrom_mmr;

	int getbank(uint32_t addr, uint32_t *realaddr, bool write_state, bool dmamode);
	int check_extrom(uint32_t raddr, uint32_t *realaddr);
	
	int window_convert(uint32_t addr, uint32_t *realaddr);
	int mmr_convert(uint32_t addr, uint32_t *realaddr, bool write_state, bool dmamode);
	int nonmmr_convert(uint32_t addr, uint32_t *realaddr);
	uint32_t read_bios(const _TCHAR *name, uint8_t *ptr, uint32_t size);
	uint32_t write_bios(const _TCHAR *name, uint8_t *ptr, uint32_t size);
	void setclock(int mode);

 public:
	FM7_MAINMEM(VM* parent_vm, EMU* parent_emu);
	~FM7_MAINMEM();
	uint32_t read_data8(uint32_t addr);
	uint32_t read_dma_data8(uint32_t addr);
	uint32_t read_dma_io8(uint32_t addr);
	uint32_t read_data8_main(uint32_t addr, bool dmamode);
   
	void write_data8(uint32_t addr, uint32_t data);
	void write_dma_data8(uint32_t addr, uint32_t data);
	void write_dma_io8(uint32_t addr, uint32_t data);
	void write_data8_main(uint32_t addr, uint32_t data, bool dmamode);
   
	virtual uint32_t read_data16(uint32_t addr);
	virtual void write_data16(uint32_t addr, uint32_t data);
   
	virtual uint32_t read_data32(uint32_t addr);
	virtual void write_data32(uint32_t addr, uint32_t data);
   
	void initialize(void);
	void wait(void);
	void reset(void);
	void release(void);

	bool get_loadstat_basicrom(void);
	bool get_loadstat_bootrom_bas(void);
	bool get_loadstat_bootrom_dos(void);
	void save_state(FILEIO *state_fio);
	void update_config();
	bool load_state(FILEIO *state_fio);
	const _TCHAR *get_device_name()
	{
		return _T("FM7_MAIN_MEM");
	}

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
#if defined(CAPABLE_DICTROM)
	void set_context_kanjirom_class1(DEVICE *p){
		kanjiclass1 = p;
	}
#endif	
	void write_signal(int sigid, uint32_t data, uint32_t mask);
	uint32_t read_signal(int sigid);
	uint32_t read_io8(uint32_t addr) {
		return mainio->read_io8(addr);
	}
};

#endif
