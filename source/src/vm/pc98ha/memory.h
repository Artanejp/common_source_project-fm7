/*
	NEC PC-98LT Emulator 'ePC-98LT'
	NEC PC-98HA Emulator 'eHANDY98'

	Author : Takeda.Toshiya
	Date   : 2008.06.10 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_IR2	0

class MEMORY : public DEVICE
{
private:
	uint8* rbank[64];	// 1MB / 16KB
	uint8* wbank[64];
	uint8 wdmy[0x4000];
	uint8 rdmy[0x10000];
	
	uint8 ram[0xa0000];		// RAM 640KB
	uint8 vram[0x8000];		// VRAM 32KB
	
	uint8 ipl[0x10000];		// IPL 64KB
	uint8 kanji[0x40000];		// Kanji ROM 256KB
#ifdef _PC98HA
	uint8 learn[0x40000];		// Learn RAM 256KB
	uint8 dic[0xc0000];		// Dictionary ROM 768KB
	uint8 romdrv[0x100000];		// ROM Drive 1024KB
	uint8 ramdrv[0x160000];		// RAM Drive 1408KB
	uint8 ems[0x400000];		// EMS 4096KB
	uint8 memcard[0x400000];	// Memory Card 4096KB
#else
	uint8 learn[0x10000];		// Learn RAM 64KB
	uint8 dic[0x80000];		// Dictionary ROM 512KB
	uint8 romdrv[0x80000];		// ROM Drive 512KB
#endif
	
	uint32 learn_crc32;
#ifdef _PC98HA
	uint32 ramdrv_crc32;
	uint32 memcard_crc32;
#endif
	
	void update_bank();
	uint8 learn_bank, dic_bank, kanji_bank, romdrv_bank;
#ifdef _PC98HA
	uint8 ramdrv_bank, ramdrv_sel;
	uint8 ems_bank[4];
#endif
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	
	// unique function
	uint8* get_vram()
	{
		return vram;
	}
	void draw_screen();
};

#endif

