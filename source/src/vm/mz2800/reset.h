/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ reset ]
*/

#ifndef _RESET_H_
#define _RESET_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_RESET_CONTROL	0

class RESET : public DEVICE
{
private:
	uint8_t prev;
	
public:
	RESET(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Reset I/O"));
	}
	~RESET() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif

