/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-

	[ dictionary rom/ram & cmos & RAM area 0x000d0000 - 0x000effff]
*/

#pragma once

#include "../../common.h"
#include "../device.h"

#define SIG_FMTOWNS_DICTSEL 0x1000
#define SIG_FMTOWNS_DICTBANK 0x1001

namespace FMTOWNS {

class DICTIONARY : public DEVICE
{
protected:
	DEVICE *d_sysrom;
	
	uint8_t dict_rom[0x80000]; // 512KB
	uint8_t dict_ram[0x2000];  // 2 + 6KB
	uint8_t ram_0d0[0x20000]; // 128KB

	bool bankd0_dict;
	uint8_t dict_bank;

	bool cmos_dirty;

public:
	DICTIONARY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name("FM-Towns Dictionary ROM/RAM 0x000d0000 - 0x000effff with CMOS RAM");
	}
	~DICTIONARY() {}
	void initialize();
	void release();
	void reset();

	uint32_t read_data8(uint32_t addr);
	uint32_t read_data16(uint32_t addr);
	uint32_t read_data32(uint32_t addr);

	void write_data8(uint32_t addr, uint32_t data);
	void write_data16(uint32_t addr, uint32_t data);
	void write_data32(uint32_t addr, uint32_t data);

	void write_io8(uint32_t addr, uint32_t data);
	void read_io8(uint32_t addr, uint32_t data);

	void write_signal(int ch, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch);

	bool process_state(FILEIO* state_fio, bool loading);

	void set_context_sysrom(DEVICE* dev)
	{
		d_sysrom = dev;
	}
};

}

// END
