/*
	SHARP MZ-2800 Emulator 'EmuZ-2800'

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ mouse ]
*/

#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MOUSE_SEL	0
#define SIG_MOUSE_DTR	1

class MOUSE : public DEVICE
{
private:
	DEVICE* d_sio;
	
	// mouse
	const int32_t* stat;
	bool select;
	
public:
	MOUSE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Mouse I/F"));
	}
	~MOUSE() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
};

#endif

