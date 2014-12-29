/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.06.10-

	[ nor gate ]
*/

#ifndef _NOR_H_
#define _NOR_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_NOR_BIT_0	0x01
#define SIG_NOR_BIT_1	0x02
#define SIG_NOR_BIT_2	0x04
#define SIG_NOR_BIT_3	0x08
#define SIG_NOR_BIT_4	0x10
#define SIG_NOR_BIT_5	0x20
#define SIG_NOR_BIT_6	0x40
#define SIG_NOR_BIT_7	0x80

class NOR : public DEVICE
{
private:
	outputs_t outputs;
	uint32 bits_in;
	bool prev, first;
	
public:
	NOR(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs);
		bits_in = 0;
		prev = first = true;
	}
	~NOR() {}
	
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

