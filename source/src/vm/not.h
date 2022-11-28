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
	NOT(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs);
		prev = first = true;
		set_device_name(_T("NOT Gate"));
	}
	~NOT() {}
	
	// common functions
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_out(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs, device, id, mask);
	}
};

#endif

