/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2009.01.05-

	[ i8155 ]
*/

#ifndef _I8155_H_
#define _I8155_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I8155_PORT_A	0
#define SIG_I8155_PORT_B	1
#define SIG_I8155_PORT_C	2
#define SIG_I8155_CLOCK		3

class I8155 : public DEVICE
{
private:
	uint16_t count, countreg;
	bool now_count, stop_tc, half;
	bool prev_out, prev_in;
	
	// constant clock
	uint64_t freq;
	int register_id;
	uint32_t input_clk, prev_clk;
	int period;
	uint64_t cpu_clocks;
	
	struct {
		uint8_t wreg;
		uint8_t rreg;
		uint8_t rmask;
		uint8_t mode;
		bool first;
		// output signals
		outputs_t outputs;
	} pio[3];
	outputs_t outputs_timer;
	uint8_t cmdreg, statreg;
	
	uint8_t ram[256];
	
	void input_clock(int clock);
	void start_count();
	void stop_count();
	void update_count();
	int get_next_clock();
	void set_signal(bool signal);
	void set_pio(int ch, uint8_t data);
	
public:
	I8155(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 3; i++) {
			initialize_output_signals(&pio[i].outputs);
			pio[i].wreg = pio[i].rreg = 0;//0xff;
		}
		initialize_output_signals(&outputs_timer);
		freq = 0;
		set_device_name(_T("8155 PIO/PIT/RAM"));
	}
	~I8155() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame)
	{
		cpu_clocks = new_clocks;
	}
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_port_a(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&pio[0].outputs, device, id, mask, shift);
	}
	void set_context_port_b(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&pio[1].outputs, device, id, mask, shift);
	}
	void set_context_port_c(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&pio[2].outputs, device, id, mask, shift);
	}
	void set_context_timer(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_timer, device, id, mask);
	}
	void set_constant_clock(uint32_t hz)
	{
		freq = hz;
	}
};

#endif

