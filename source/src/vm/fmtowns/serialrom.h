/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[serial rom]
*/

#pragma once

#include "../device.h"

#define SIG_SERIALROM_CLK	1
#define SIG_SERIALROM_CS	2
#define SIG_SERIALROM_RESET	3
#define SIG_SERIALROM_DATA	4

namespace FMTOWNS {

class SERIAL_ROM : public DEVICE
{
protected:
	DEVICE* d_mem;

	bool cs;
	bool clk;
	bool reset_reg;
	int reset_state;
	uint8_t rom_addr;
	uint8_t rom[32];

public:
	SERIAL_ROM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
		set_device_name(_T("FMTOWNS_SERIAL_ROM"));
		d_mem = NULL;
	}
	~SERIAL_ROM() {}

	void initialize();
	void reset();

	void write_signal(int ch, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch);

	bool process_state(FILEIO* state_fio, bool loading);

	void set_context_mem(DEVICE* dev)
	{
		d_mem = dev;
	}
};

}
	
