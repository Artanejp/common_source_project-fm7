/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2009.03.14-

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MB8877;
#ifdef _X1TURBO_FEATURE
class Z80DMA;
#endif

class FLOPPY : public DEVICE
{
private:
	MB8877 *d_fdc;
	int prev;
	bool motor_on;
	int register_id;
	
public:
	FLOPPY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		prev = 0;
		motor_on = false;
	}
	~FLOPPY() {}
	
	// common functions
	void reset();
	void write_io8(uint32 addr, uint32 data);
#ifdef _X1TURBO_FEATURE
	uint32 read_io8(uint32 addr);
#endif
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_fdc(MB8877* device)
	{
		d_fdc = device;
	}
};

#endif

