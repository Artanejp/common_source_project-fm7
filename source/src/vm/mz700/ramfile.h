/*
	SHARP MZ-700 Emulator 'EmuZ-700'
	SHARP MZ-800 Emulator 'EmuZ-800'
	SHARP MZ-1500 Emulator 'EmuZ-1500'

	Author : Takeda.Toshiya
	Date   : 2010.09.02 -

	[ ram file ]
*/

#ifndef _RAMFILE_H_
#define _RAMFILE_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class RAMFILE : public DEVICE
{
private:
	uint8_t *data_buffer;
	uint32_t data_addr;
	
public:
	RAMFILE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("RAM File"));
	}
	~RAMFILE() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif

