/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2013.11.08-

	[ mc6840 ]
*/

#ifndef _MC6840_H_
#define _MC6840_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_MC6840_CLOCK_0	0
#define SIG_MC6840_CLOCK_1	1
#define SIG_MC6840_CLOCK_2	2

class MC6840 : public DEVICE
{
private:
	typedef struct {
		uint16 counter, latch;
		uint8 counter_lo, latch_hi;
		uint8 control;
		bool in_pin, out_pin;
		bool signal, once;
		int clocks, prescaler;
		// constant clock
		int freq;
		// output signals
		outputs_t outputs;
	} timer_t;
	timer_t timer[3];
	uint8 status, status_read;
	outputs_t outputs_irq;
	
	void set_control(int ch, uint32 data);
	void set_counter(int ch);
	void input_clocks(int ch, int clocks);
	void set_irq(int ch, bool signal);
	void set_signal(int ch, bool signal);
	
public:
	MC6840(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 3; i++) {
			init_output_signals(&timer[i].outputs);
			counter[i].freq = 0;
		}
	}
	~MC6840() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_irq(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_ch0(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&timer[0].outputs, device, id, mask);
	}
	void set_context_ch1(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&timer[1].outputs, device, id, mask);
	}
	void set_context_ch2(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&timer[2].outputs, device, id, mask);
	}
	void set_constant_clock(int ch, uint32 hz)
	{
		timer[ch].freq = hz;
	}
};

#endif

