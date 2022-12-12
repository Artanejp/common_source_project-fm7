/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.06.10 -

	[ memory ]
*/

#ifndef _MEMBUS_H_
#define _MEMBUS_H_

#include "../memory.h"

#define SIG_MEMORY_IR2	0

class MEMBUS : public MEMORY
{
private:
	uint8_t rdmy[0x10000];
	
	uint8_t ram[0xa0000];		// RAM 640KB
	uint8_t vram[0x8000];		// VRAM 32KB
	
	uint8_t ipl[0x10000];		// IPL 64KB
	uint8_t kanji[0x40000];		// Kanji ROM 256KB
#ifdef _PC98HA
	uint8_t learn[0x40000];		// Learn RAM 256KB
	uint8_t dic[0xc0000];		// Dictionary ROM 768KB
	uint8_t romdrv[0x100000];		// ROM Drive 1024KB
	uint8_t ramdrv[0x160000];		// RAM Drive 1408KB
	uint8_t ems[0x400000];		// EMS 4096KB
	uint8_t memcard[0x400000];	// Memory Card 4096KB
#else
	uint8_t learn[0x10000];		// Learn RAM 64KB
	uint8_t dic[0x80000];		// Dictionary ROM 512KB
	uint8_t romdrv[0x80000];		// ROM Drive 512KB
#endif
	
	uint32_t learn_crc32;
#ifdef _PC98HA
	uint32_t ramdrv_crc32;
	uint32_t memcard_crc32;
#endif
	
	void update_bank();
	uint8_t learn_bank, dic_bank, kanji_bank, romdrv_bank;
#ifdef _PC98HA
	uint8_t ramdrv_bank, ramdrv_sel;
	uint8_t ems_bank[4];
#endif
	
public:
	MEMBUS(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MEMORY(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMBUS() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
#ifdef _PC98HA
	void write_data8w(uint32_t addr, uint32_t data, int *wait);
#endif
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	uint8_t* get_vram()
	{
		return vram;
	}
	void draw_screen();
};

#endif

