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
	uint8_t* rbank[8192];	// 16MB / 2KB
	uint8_t* wbank[8192];
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	
	uint8_t ram[0xc0000];		// RAM 768KB
	uint8_t exram[0x100000];		// Ext RAM 1MB
	uint8_t vram[0x60000];		// VRAM 384KB ???
	uint8_t tvram[0x7800];		// TVRAM 32KB ???
	uint8_t backup[0x8200];		// Battery BackUp 32KB ???
	uint8_t ipl[0x10000];		// IPL 64KB
	
	bool protect;
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	
	// unique functions
	uint8_t* get_vram()
	{
		return vram;
	}
	uint8_t* get_tvram()
	{
		return tvram;
	}
};

#endif

