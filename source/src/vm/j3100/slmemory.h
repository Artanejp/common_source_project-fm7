/*
	TOSHIBA J-3100SL Emulator 'eJ-3100SL'

	Author : Takeda.Toshiya
	Date   : 2011.08.16-

	[ memory ]
*/

#ifndef _SL_MEMORY_H_
#define _SL_MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MEMORY : public DEVICE
{
private:
	uint8* rbank[512];	// 1MB / 2KB
	uint8* wbank[512];
	uint8 wdmy[0x800];
	uint8 rdmy[0x800];
	uint8 ram[0xa0000];	// RAM 640KB
	uint8 vram[0x8000];	// VRAM 32KB
	uint8 ems[0x4000*56];	// EMS 16KB * 56
	uint8 kanji[0x100000];	// KANJI ROM 1MB
#ifdef _J3100SE
	uint8 backup[0x4000];	// Backuped RAM 16KB
	uint8 ipl[0xc000];	// IPL 48KB
#else
	uint8 backup[0x800];	// Backuped RAM 2KB
	uint8 ipl[0x8000];	// IPL 32KB
#endif
	
	int kanji_bank;
	
	int ems_index;
	int ems_regs[2];
	int ems_page[4];
	int ems_bsl;
	
	uint32 ems_crc32;
	uint32 backup_crc32;
	
	void update_ems(int page);
	
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
	
	// unitque function
	uint8* get_vram()
	{
		return vram;
	}
};

#endif

