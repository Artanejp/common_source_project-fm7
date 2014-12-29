/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.20-

	[ not gate ]
*/

#ifndef _NOT_H_
#define _NOT_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_NOT_INPUT	0

class NOT : public DEVICE
{
private:
	outputs_t outputs;
	bool prev, first;
	
public:
	NOT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs);
		prev = first = true;
	}
	~NOT() {}
	
	// common functions
	void write_signal(int id, uint32 data, uint32 mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_out(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs, device, id, mask);
	}
};

#endif

