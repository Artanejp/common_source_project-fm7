/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.11-

	[ 74LS393 ]
*/

#ifndef _LS393_H_
#define _LS393_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_LS393_CLK	0

class LS393 : public DEVICE
{
private:
	// output signals
	outputs_t outputs[8];
	
	uint32 count;
	bool prev_in;
	
public:
	LS393(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 8; i++) {
			init_output_signals(&outputs[i]);
		}
		count = 0;
		prev_in = false;
	}
	~LS393() {}
	
	// common functions
	void write_signal(int id, uint32 data, uint32 mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_1qa(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs[0], device, id, mask);
	}
	void set_context_1qb(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs[1], device, id, mask);
	}
	void set_context_1qc(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs[2], device, id, mask);
	}
	void set_context_1qd(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs[3], device, id, mask);
	}
	void set_context_2qa(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs[4], device, id, mask);
	}
	void set_context_2qb(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs[5], device, id, mask);
	}
	void set_context_2qc(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs[6], device, id, mask);
	}
	void set_context_2qd(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs[7], device, id, mask);
	}
};

#endif

