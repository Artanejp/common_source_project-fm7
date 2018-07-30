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
	uint8_t* rbank[8192];	// 16MB / 2KB
	uint8_t* wbank[8192];
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	uint8_t ram[0xa0000];	// RAM 640KB
	uint8_t vram[0x8000];	// VRAM 32KB
	uint8_t ems[0x4000*512];	// EMS 16KB * 512
	uint8_t kanji[0x100000];	// KANJI ROM 1MB
	uint8_t ipl[0x10000];	// IPL 64KB
	
	int kanji_bank;
	int ems_page[4][4];
	
	void update_ems(int page);
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	
	// unique function
	uint8_t* get_vram()
	{
		return vram;
	}
};

#endif

