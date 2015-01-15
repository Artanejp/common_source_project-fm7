/*
	CASIO PV-2000 Emulator 'EmuGaki'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ printer ]
*/

#ifndef _PRINTER_H_
#define _PRINTER_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class PRINTER : public DEVICE
{
private:
	uint8 out, ctrl0, ctrl1;
	bool busy;
	
public:
	PRINTER(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~PRINTER() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

