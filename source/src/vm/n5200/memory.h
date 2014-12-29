/*
	NEC N5200 Emulator 'eN5200'

	Author : Takeda.Toshiya
	Date   : 2009.06.03-

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
	
	uint8 ram[0xc0000];		// RAM 768KB
	uint8 exram[0x100000];		// Ext RAM 1MB
	uint8 vram[0x60000];		// VRAM 384KB ???
	uint8 tvram[0x7800];		// TVRAM 32KB ???
	uint8 backup[0x8200];		// Battery BackUp 32KB ???
	uint8 ipl[0x10000];		// IPL 64KB
	
	bool protect;
	
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
	uint8* get_tvram()
	{
		return tvram;
	}
};

#endif

