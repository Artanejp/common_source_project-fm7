/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2011.06.02-

	[ mc6820 ]
*/

#ifndef _MC6820_H_
#define _MC6820_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_MC6820_PORT_A	0
#define SIG_MC6820_PORT_B	1
#define SIG_MC6820_C1_A		2
#define SIG_MC6820_C1_B		3
#define SIG_MC6820_C2_A		4
#define SIG_MC6820_C2_B		5

class MC6820 : public DEVICE
{
private:
	typedef struct {
		uint8 wreg;
		uint8 rreg;
		uint8 ctrl;
		uint8 ddr;
		bool c1, c2;
		bool first;
		// output signals
		outputs_t outputs;
		outputs_t outputs_irq;
	} port_t;
	port_t port[2];
	
public:
	MC6820(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 2; i++) {
			init_output_signals(&port[i].outputs);
			init_output_signals(&port[i].outputs_irq);
			port[i].wreg = port[i].rreg = 0;//0xff;
		}
	}
	~MC6820() {}
	
	// common functions
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_port_a(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[0].outputs, device, id, mask, shift);
	}
	void set_context_port_b(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[1].outputs, device, id, mask, shift);
	}
	void set_context_irq_a(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[0].outputs_irq, device, id, mask);
	}
	void set_context_irq_b(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[1].outputs_irq, device, id, mask);
	}
};

#endif

