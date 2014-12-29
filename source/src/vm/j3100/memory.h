/*
	TOSHIBA J-3100GT Emulator 'eJ-3100GT'

	Author : Takeda.Toshiya
	Date   : 2011.08.28-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MEMORY : public DEVICE
{
private:
	uint8* rbank[8192];	// 16MB / 2KB
	uint8* wbank[8192];
	uint8 wdmy[0x800];
	uint8 rdmy[0x800];
	uint8 ram[0xa0000];	// RAM 640KB
	uint8 vram[0x8000];	// VRAM 32KB
	uint8 ems[0x4000*512];	// EMS 16KB * 512
	uint8 kanji[0x100000];	// KANJI ROM 1MB
	uint8 ipl[0x10000];	// IPL 64KB
	
	int kanji_bank;
	int ems_page[4][4];
	
	void update_ems(int page);
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
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

