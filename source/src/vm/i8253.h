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

enum {
	INTEL_8253 = 0,
	INTEL_8254,
};

class I8253 : public DEVICE
{
private:
	struct {
		bool prev_out;
		bool prev_in;
		bool gate;
		int32_t count;
		uint16_t latch;
		uint16_t count_reg;
		uint8_t ctrl_reg;
		bool count_latched;
		bool low_read, high_read;
		bool low_write, high_write;
		int mode;
		bool delay;
		bool start;
//#ifdef HAS_I8254
		bool null_count;
		bool status_latched;
		uint8_t status;
//#endif
		// constant clock
		uint64_t freq;
		int register_id;
		uint32_t input_clk;
		int period;
		uint32_t prev_clk;
		// output signals
		outputs_t outputs;
	} counter[3];
	uint64_t cpu_clocks;
	
	void input_clock(int ch, int clock);
	void input_gate(int ch, bool signal);
	void start_count(int ch);
	void stop_count(int ch);
	void latch_count(int ch);
	void set_signal(int ch, bool signal);
	int get_next_count(int ch);
	
public:
	I8253(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 3; i++) {
			initialize_output_signals(&counter[i].outputs);
			counter[i].freq = 0;
		}
		device_model = INTEL_8253;
		set_device_name(_T("8253 PIT"));
	}
	~I8253() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
	{
		cpu_clocks = new_clocks;
	}
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_ch0(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&counter[0].outputs, device, id, mask);
	}
	void set_context_ch1(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&counter[1].outputs, device, id, mask);
	}
	void set_context_ch2(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&counter[2].outputs, device, id, mask);
	}
	void set_constant_clock(int ch, uint32_t hz)
	{
		counter[ch].freq = hz;
	}
	int device_model;
};

#endif

