/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-

	[ SYSTEM rom & RAM area 0x000f8000 - 0x000fffff]
	* MEMORY :
	*   0x000d0000 - 0x000d7fff : RAM / DICTIONARY (BANKED)
	*   0x000d8000 - 0x000d9fff : RAM / CMOS
	*   0x000da000 - 0x000dffff : RAM / RESERVED
	*   0x000f8000 - 0x000fffff : RAM / SYSTEM
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
	uint8_t rom[0x40000]; // 256KB
	
public:
	SYSROM(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name("FM-Towns SYSTEM ROM");
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
	
};

}

// END
