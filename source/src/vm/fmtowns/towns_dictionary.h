/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-

	[ dictionary rom/ram & cmos & RAM area 0x000d0000 - 0x000dffff]
	* MEMORY :
	*   0x000d0000 - 0x000d7fff : DICTIONARY ROM (BANKED)
	*   0x000d8000 - 0x000d9fff : DICTIONARY RAM / GAIJI RAM
	*   0x000da000 - 0x000dffff : RESERVED
	*   0xc2080000 - 0xc20fffff : DICTIONARY ROM (NOT BANKED)
	*   0xc2140000 - 0xc2141fff : DICTIONARY RAM
	* I/O : 
	*   0x0484                         : DICTIONARY BANK (for 0xd0000 - 0xd7ffff)
	*   0x3000 - 0x3ffe (even address) : DICTIONARY RAM
*/

#pragma once

#include "../../common.h"
#include "../device.h"

#define SIG_FMTOWNS_DICT_BANK 1

namespace FMTOWNS {

class DICTIONARY : public DEVICE
{
protected:
	uint8_t dict_rom[0x80000]; // 512KB
	uint8_t dict_ram[0x2000];  // 2 + 6KB

	uint8_t dict_bank;
	bool cmos_dirty;

public:
	DICTIONARY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		dict_bank = 0x00;
		cmos_dirty = false;
		set_device_name("FM-Towns Dictionary ROM/RAM 0x000d0000 - 0x000dffff with CMOS RAM");
	}
	~DICTIONARY() {}
	void initialize();
	void release();
	void reset();

	uint32_t __FASTCALL read_memory_mapped_io8(uint32_t addr);
	uint32_t __FASTCALL read_memory_mapped_io16(uint32_t addr);
	uint32_t __FASTCALL read_memory_mapped_io32(uint32_t addr);
	
	void __FASTCALL write_memory_mapped_io8(uint32_t addr, uint32_t data);
	void __FASTCALL write_memory_mapped_io16(uint32_t addr, uint32_t data);
	void __FASTCALL write_memory_mapped_io32(uint32_t addr, uint32_t data);

	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);

	void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);
	uint32_t __FASTCALL read_signal(int ch);

	bool process_state(FILEIO* state_fio, bool loading);
};

}

// END
