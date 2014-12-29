/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.14 -

	[ i8255 ]
*/

#ifndef _I8255_H_
#define _I8255_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I8255_PORT_A	0
#define SIG_I8255_PORT_B	1
#define SIG_I8255_PORT_C	2

class I8255 : public DEVICE
{
private:
	struct {
		uint8 wreg;
		uint8 rreg;
		uint8 rmask;
		uint8 mode;
		bool first;
		// output signals
		outputs_t outputs;
	} port[3];
	
public:
	I8255(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 3; i++) {
			init_output_signals(&port[i].outputs);
			port[i].wreg = port[i].rreg = 0;//0xff;
		}
		clear_ports_by_cmdreg = false;
	}
	~I8255() {}
	
	// common functions
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_signal(int id);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_port_a(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[0].outputs, device, id, mask, shift);
	}
	void set_context_port_b(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[1].outputs, device, id, mask, shift);
	}
	void set_context_port_c(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[2].outputs, device, id, mask, shift);
	}
	bool clear_ports_by_cmdreg;
};

#endif

