/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

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
	FLOPPY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		prev = 0;
		motor_on = false;
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
#ifdef _X1TURBO_FEATURE
	uint32_t read_io8(uint32_t addr);
#endif
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_fdc(MB8877* device)
	{
		d_fdc = device;
	}
};

#endif

