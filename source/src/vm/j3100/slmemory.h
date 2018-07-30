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
	uint8_t* rbank[512];	// 1MB / 2KB
	uint8_t* wbank[512];
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	uint8_t ram[0xa0000];	// RAM 640KB
	uint8_t vram[0x8000];	// VRAM 32KB
	uint8_t ems[0x4000*56];	// EMS 16KB * 56
	uint8_t kanji[0x100000];	// KANJI ROM 1MB
#ifdef _J3100SE
	uint8_t backup[0x4000];	// Backuped RAM 16KB
	uint8_t ipl[0xc000];	// IPL 48KB
#else
	uint8_t backup[0x800];	// Backuped RAM 2KB
	uint8_t ipl[0x8000];	// IPL 32KB
#endif
	
	int kanji_bank;
	
	int ems_index;
	int ems_regs[2];
	int ems_page[4];
	int ems_bsl;
	
	uint32_t ems_crc32;
	uint32_t backup_crc32;
	
	void update_ems(int page);
	
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
	
	// unique function
	uint8_t* get_vram()
	{
		return vram;
	}
};

#endif

