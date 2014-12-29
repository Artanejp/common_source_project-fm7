/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.14 -

	[ i8253/i8254 ]
*/

#ifndef _I8253_H_
#define _I8253_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I8253_CLOCK_0	0
#define SIG_I8253_CLOCK_1	1
#define SIG_I8253_CLOCK_2	2
#define SIG_I8253_GATE_0	3
#define SIG_I8253_GATE_1	4
#define SIG_I8253_GATE_2	5

class I8253 : public DEVICE
{
private:
	struct {
		bool prev_out;
		bool prev_in;
		bool gate;
		int32 count;
		uint16 latch;
		uint16 count_reg;
		uint8 ctrl_reg;
		bool count_latched;
		bool low_read, high_read;
		bool low_write, high_write;
		int mode;
		bool delay;
		bool start;
#ifdef HAS_I8254
		bool null_count;
		bool status_latched;
		uint8 status;
#endif
		// constant clock
		uint64 freq;
		int register_id;
		uint32 input_clk;
		int period;
		uint32 prev_clk;
		// output signals
		outputs_t outputs;
	} counter[3];
	uint64 cpu_clocks;
	
	void input_clock(int ch, int clock);
	void input_gate(int ch, bool signal);
	void start_count(int ch);
	void stop_count(int ch);
	void latch_count(int ch);
	void set_signal(int ch, bool signal);
	int get_next_count(int ch);
	
public:
	I8253(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 3; i++) {
			init_output_signals(&counter[i].outputs);
			counter[i].freq = 0;
		}
	}
	~I8253() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32 data, uint32 mask);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
	{
		cpu_clocks = new_clocks;
	}
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_ch0(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&counter[0].outputs, device, id, mask);
	}
	void set_context_ch1(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&counter[1].outputs, device, id, mask);
	}
	void set_context_ch2(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&counter[2].outputs, device, id, mask);
	}
	void set_constant_clock(int ch, uint32 hz)
	{
		counter[ch].freq = hz;
	}
};

#endif

