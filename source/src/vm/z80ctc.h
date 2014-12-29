/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ Z80CTC ]
*/

#ifndef _Z80CTC_H_
#define _Z80CTC_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_Z80CTC_TRIG_0	0
#define SIG_Z80CTC_TRIG_1	1
#define SIG_Z80CTC_TRIG_2	2
#define SIG_Z80CTC_TRIG_3	3

class Z80CTC : public DEVICE
{
private:
	struct {
		uint8 control;
		bool slope;
		uint16 count;
		uint16 constant;
		uint8 vector;
		int clocks;
		int prescaler;
		bool freeze;
		bool start;
		bool latch;
		bool prev_in;
		bool first_constant;
		// constant clock
		uint64 freq;
		int clock_id;
		int sysclock_id;
		uint32 input;
		uint32 period;
		uint32 prev;
		// interrupt
		bool req_intr;
		bool in_service;
		// output signals
		outputs_t outputs;
	} counter[4];
	uint64 cpu_clocks;
	
	void input_clock(int ch, int clock);
	void input_sysclock(int ch, int clock);
	void update_event(int ch, int err);
	
	// daisy chain
	DEVICE *d_cpu, *d_child;
	bool iei, oei;
	uint32 intr_bit;
	void update_intr();
	
public:
	Z80CTC(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(counter, 0, sizeof(counter));
		for(int i = 0; i < 4; i++) {
			init_output_signals(&counter[i].outputs);
			counter[i].freq = 0;
			counter[i].prev_in = false;
		}
		d_cpu = d_child = NULL;
	}
	~Z80CTC() {}
	
	// common functions
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
	{
		cpu_clocks = new_clocks;
	}
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// interrupt common functions
	void set_context_intr(DEVICE* device, uint32 bit)
	{
		d_cpu = device;
		intr_bit = bit;
	}
	void set_context_child(DEVICE* device)
	{
		d_child = device;
	}
	void set_intr_iei(bool val);
	uint32 intr_ack();
	void intr_reti();
	
	// unique functions
	void set_context_zc0(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&counter[0].outputs, device, id, mask);
	}
	void set_context_zc1(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&counter[1].outputs, device, id, mask);
	}
	void set_context_zc2(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&counter[2].outputs, device, id, mask);
	}
	void set_constant_clock(int ch, uint32 hz)
	{
		counter[ch].freq = hz;
	}
};

#endif
