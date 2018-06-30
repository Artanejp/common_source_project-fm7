/*
	CASIO FP-1100 Emulator 'eFP-1100'

	Author : Takeda.Toshiya
	Date   : 2010.06.18-

	[ ram pack ]
*/

#ifndef _RAMPACK_H_
#define _RAMPACK_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class RAMPACK : public DEVICE
{
private:
	uint8_t ram[0x4000];
	bool modified;
	
public:
	RAMPACK(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("RAM Pack"));
	}
	~RAMPACK() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void decl_state();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique variable
	int index;
};

#endif
