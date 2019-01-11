/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-

	[ SYSTEM rom & RAM area 0x000f0000 - 0x000fffff]
*/

#pragma once

#include "common.h"
#include "device.h"

#define SIG_FMTOWNS_SYSROMSEL 0x1000

namespace FMTOWNS {

class SYSROM : public DEVICE
{
protected:
	uint8_t rom[0x40000]; // 256KB
	uint8_t ram[0x10000];  // 64KB
	
	bool map_dos;

public:
	SYSROM(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name("FM-Towns SYSTEM ROM/RAM 0x000fxxxx");
	}
	~SYSROM() {}
	void initialize();
	void reset();

	uint32_t read_data8(uint32_t addr);

	void write_data8(uint32_t addr, uint32_t data);

	void write_signal(int ch, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch);

	bool process_state(FILEIO* state_fio, bool loading);

};

}

// END
