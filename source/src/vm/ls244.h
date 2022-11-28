/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.19-

	[ 74LS244 / 74LS245 ]
*/

#ifndef _LS244_H_
#define _LS244_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_LS244_INPUT	0

class LS244 : public DEVICE
{
private:
	// output signals
	outputs_t outputs;
	
	uint8_t din;
	
public:
	LS244(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs);
		set_device_name(_T("74LS244 Octal 3-State Buffer"));
	}
	~LS244() {}
	
	// common functions
	void initialize();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_output(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&outputs, device, id, mask, shift);
	}
};

#endif

