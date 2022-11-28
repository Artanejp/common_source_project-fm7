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

# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
#define ADDRESS_SPACE 0x100000
#elif defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
#define ADDRESS_SPACE 0x40000
#else
#define ADDRESS_SPACE 0x10000
#endif

class DEVICE;
class MEMORY;
class FM7_MAINIO;
class KANJIROM;
class DISPLAY;

class FM7_MAINMEM : public DEVICE
{
 private:
	typedef struct {
		uint8_t *read_data;
		uint8_t (FM7_MAINMEM::*read_func)(uint32_t, bool);
		uint8_t *write_data;
		void (FM7_MAINMEM::*write_func)(uint32_t, uint32_t, bool);
	} data_func_table_t;
	
	data_func_table_t data_table[ADDRESS_SPACE / 0x80];
#if defined(HAS_MMR)
# if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX)	
	data_func_table_t mmr_update_table_ext[(0x80 * 0x1000) / 0x80];
	uint32_t mmr_baseaddr_table_ext[(0x80 * 0x1000) / 0x80];
# endif
	data_func_table_t mmr_update_table_nor[(0x80 * 0x1000) / 0x80];
	uint32_t mmr_bank_table[(0x80 * 0x1000) / 0x80];
	uint32_t mmr_baseaddr_table_nor[(0x80 * 0x1000) / 0x80];
#endif
	bool ioaccess_wait;
	int waitfactor;
	int waitcount;
	int cpu_clocks;
	
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
	uint8_t fm7_mainmem_omote[0x8000];
	uint8_t fm7_mainmem_ura[0x7c00];
	uint8_t fm7_mainmem_basicrom[0x7c00];
  	uint8_t fm7_mainmem_bioswork[0x80];
#if !defined(_FM77AV)
	uint8_t *fm7_bootroms[8];
#endif	
	uint8_t fm7_mainmem_bootrom_vector[0x1e]; // Without
	uint8_t fm7_mainmem_reset_vector[2]; // Without
	uint8_t fm7_mainmem_null[1];
#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	uint8_t fm7_bootram[0x200]; // $00000-$0ffff
#endif
# if defined(_FM8)
	bool diag_load_sm11_14;
	bool diag_load_sm11_15;
# elif defined(_FM77_VARIANTS)
	bool diag_load_wb11_12;
# elif defined(_FM7) || defined(_FMNEW7)
	bool diag_load_tl11_11;
	bool diag_load_tl11_12;
#endif
#ifdef HAS_MMR
	bool extram_connected;

# if defined(_FM77AV_VARIANTS)
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
#  if defined(_FM77AV40) || defined(_FM77AV40EX) || defined(_FM77AV40SX) 
	bool diag_load_extrarom;
	uint8_t fm7_mainmem_extrarom[0x10000]; // $20000-$2bfff, banked
	int extram_pages;
	uint32_t extram_size;
	uint8_t *fm7_mainmem_extram; // $40000- : MAX 768KB ($c0000)
#  endif
# endif
# if defined(_FM77_VARIANTS)
	int extram_pages;
	uint32_t extram_size;
	uint8_t *fm7_mainmem_extram; // $00000-$2ffff
	uint8_t fm77_shadowram[0x200];
# endif
#endif
#if defined(CAPABLE_DICTROM)
	KANJIROM *kanjiclass1;
	//KANJIROM *kanjiclass2;
#endif	
	MC6809 *maincpu;
	FM7_MAINIO *mainio;
	DISPLAY *display;
	
	bool diag_load_basicrom;
	bool diag_load_bootrom_bas;
	bool diag_load_bootrom_dos;
	bool diag_load_bootrom_mmr;
	bool diag_load_bootrom_bubble;
	bool diag_load_bootrom_bubble_128k;
	bool diag_load_bootrom_sfd8;
	bool diag_load_bootrom_2hd;

	uint32_t mem_waitfactor;
	uint32_t mem_waitcount;

	int check_extrom(uint32_t raddr, uint32_t *realaddr);
	
	int window_convert(uint32_t addr, uint32_t *realaddr);
	uint32_t read_bios(const _TCHAR *name, uint8_t *ptr, uint32_t size);
	uint32_t write_bios(const _TCHAR *name, uint8_t *ptr, uint32_t size);
	void setclock(int mode);
	
	uint8_t read_shared_ram(uint32_t realaddr, bool dmamode);
	void write_shared_ram(uint32_t realaddr, uint32_t data, bool dmamode);
	uint8_t read_direct_access(uint32_t realaddr, bool dmamode);
	void write_direct_access(uint32_t realaddr, uint32_t data, bool dmamode);
	uint8_t read_kanjirom_level1(uint32_t realaddr, bool dmamode);
	uint8_t read_kanji_dummyaddr(uint32_t realaddr, bool dmamode);
	uint8_t read_ura_basicrom(uint32_t addr, bool dmamode);
	void write_ura_basicrom(uint32_t addr, uint32_t data, bool dmamode);
	uint8_t read_mmio(uint32_t addr, bool dmamode);
	void write_mmio(uint32_t addr, uint32_t data, bool dmamode);
	uint8_t read_bootrom(uint32_t addr, bool dmamode);
	void write_bootrom(uint32_t addr, uint32_t data, bool dmamode);
	uint8_t read_page2(uint32_t addr, bool dmamode);
	void write_page2(uint32_t addr, uint32_t data, bool dmamode);
	int check_page2(uint32_t addr, uint32_t *realaddr, bool write_state, bool dmamode);
	
	void init_data_table(void);
	uint8_t read_data(uint32_t addr, bool dmamode);
	void write_data(uint32_t addr, uint32_t data, bool dmamode);
	uint8_t read_data_tbl(uint32_t addr, bool dmamode);
	void write_data_tbl(uint32_t addr, uint32_t data, bool dmamode);

	void update_mmr_jumptable(uint32_t pos);
	void update_all_mmr_jumptable(void);
	uint8_t read_segment_3f(uint32_t addr, bool dmamode);
	void write_segment_3f(uint32_t addr, uint32_t data, bool dmamode);
	uint8_t read_with_mmr(uint32_t addr, uint32_t segment, uint32_t dmamode);
	void write_with_mmr(uint32_t addr, uint32_t segment, uint32_t data, uint32_t dmamode);

	template <class T>
	void call_write_signal(T *np, int id, uint32_t data, uint32_t mask)
	{
		//T *nnp = static_cast<T *>(np);
		static_cast<T *>(np)->write_signal(id, data, mask);
	}
	template <class T>
		void call_write_data8(T *np, uint32_t addr, uint32_t data)
	{
		//T *nnp = static_cast<T *>(np);
		static_cast<T *>(np)->write_data8(addr, data);
	}
	template <class T>
		uint32_t call_read_data8(T *np, uint32_t addr)
	{
		//T *nnp = static_cast<T *>(np);
		return static_cast<T *>(np)->read_data8(addr);
	}
	template <class T>
		void call_write_dma_data8(T *np, uint32_t addr, uint32_t data)
	{
		//T *nnp = static_cast<T *>(np);
		static_cast<T *>(np)->write_dma_data8(addr, data);
	}
	template <class T>
		uint32_t call_read_dma_data8(T *np, uint32_t addr)
	{
		//T *nnp = static_cast<T *>(np);
		return static_cast<T *>(np)->read_dma_data8(addr);
	}
 public:
	FM7_MAINMEM(VM_TEMPLATE* parent_vm, EMU* parent_emu);
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
	void iowait(void);
	void dram_refresh(void);
	void reset(void);
	void release(void);

	bool get_loadstat_basicrom(void);
	bool get_loadstat_bootrom_bas(void);
	bool get_loadstat_bootrom_dos(void);
	bool decl_state(FILEIO *state_fio, bool loading);
	void save_state(FILEIO *state_fio);
	void update_config();
	bool load_state(FILEIO *state_fio);

	void set_context_display(DEVICE *p){
		display = (DISPLAY *)p;
	}
	void set_context_maincpu(MC6809 *p){
		maincpu = (MC6809 *)p;
	}
	void set_context_mainio(DEVICE *p){
		mainio = (FM7_MAINIO *)p;
	}
#if defined(CAPABLE_DICTROM)
	void set_context_kanjirom_class1(DEVICE *p){
		kanjiclass1 = (KANJIROM *)p;
	}
#endif	
	void write_signal(int sigid, uint32_t data, uint32_t mask);
	uint32_t read_signal(int sigid);
	uint32_t read_io8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
};

#endif
