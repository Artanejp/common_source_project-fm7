/*
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2004.09.05 -

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
	MOUSE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MOUSE() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
};

#endif

