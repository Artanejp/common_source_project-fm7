/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.04 -

	[ floppy ]
*/

#ifndef _FLOPPY_H_
#define _FLOPPY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_FLOPPY_REVERSE	0

class FLOPPY : public DEVICE
{
private:
	DEVICE* d_fdc;
#ifdef _MZ2500
	bool reversed;
#endif
	
public:
	FLOPPY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Floppy I/F"));
	}
	~FLOPPY() {}
	
	// common functions
#ifdef _MZ2500
	void initialize();
#endif
	void write_io8(uint32_t addr, uint32_t data);
#ifdef _MZ2500
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
#endif
	
	// unique functions
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
};

#endif

