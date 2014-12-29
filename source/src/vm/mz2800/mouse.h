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
	int* stat;
	bool select;
	
public:
	MOUSE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MOUSE() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique function
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
};

#endif

