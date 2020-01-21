/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-

	[ SYSTEM rom & RAM area 0x000f8000 - 0x000fffff]
	* MEMORY :
	*   0x000f8000 - 0x000fffff : RAM / DICTIONARY (BANKED)
	*   0xfffc0000 - 0xffffffff : SYSTEM ROM
	* I/O : 
	*   0x0480                         : F8 BANK
*/

#pragma once

#include "common.h"
#include "device.h"

#define SIG_FMTOWNS_SYSROMSEL     1
#define SIG_FMTOWNS_F8_DICTRAM    2

namespace FMTOWNS {
class DICTIONARY;
class SYSROM : public DEVICE
{
protected:
	DEVICE *d_dict;

	uint8_t rom[0x40000]; // 256KB
	uint8_t ram[0x8000];  // 32KB
	bool select_f8_rom;
	bool select_f8_dictram;
	
public:
	SYSROM(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_dict = NULL;
		set_device_name("FM-Towns SYSTEM ROM and RAM 0x000f8000 - 0x00fffff");
	}
	~SYSROM() {}
	void initialize();
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

	// Unique functions
	void set_context_dictionary(DEVICE *dev)
	{
		d_dict = dev;
	}
};

}

// END
