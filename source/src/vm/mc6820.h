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
	struct {
		uint8_t wreg;
		uint8_t rreg;
		uint8_t ctrl;
		uint8_t ddr;
		bool c1, c2;
		bool first;
		// output signals
		outputs_t outputs;
		outputs_t outputs_irq;
	} port[2];
	
public:
	MC6820(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 2; i++) {
			initialize_output_signals(&port[i].outputs);
			initialize_output_signals(&port[i].outputs_irq);
			port[i].wreg = port[i].rreg = 0;//0xff;
		}
		set_device_name(_T("MC6820 PIA"));
	}
	~MC6820() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_port_a(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[0].outputs, device, id, mask, shift);
	}
	void set_context_port_b(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[1].outputs, device, id, mask, shift);
	}
	void set_context_irq_a(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&port[0].outputs_irq, device, id, mask);
	}
	void set_context_irq_b(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&port[1].outputs_irq, device, id, mask);
	}
};

#endif

