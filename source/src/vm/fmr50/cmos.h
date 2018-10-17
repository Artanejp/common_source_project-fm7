/*
	FUJITSU FMR-50 Emulator 'eFMR-50'
	FUJITSU FMR-60 Emulator 'eFMR-60'

	Author : Takeda.Toshiya
	Date   : 2008.05.01 -

	[ cmos ]
*/

#ifndef _CMOS_H_
#define _CMOS_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class CMOS : public DEVICE
{
private:
#ifdef _FMRCARD
	uint8_t cmos[4][0x800];
#else
	uint8_t cmos[1][0x800];
#endif
	bool modified;
	uint8_t bank;
	
public:
	CMOS(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("CMOS RAM"));
	}
	~CMOS() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	uint8_t* get_cmos()
	{
		return cmos[0];
	}
};

#endif

