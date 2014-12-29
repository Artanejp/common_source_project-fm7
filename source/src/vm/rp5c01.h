/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.11-

	[ RP-5C01 / RP-5C15 ]
*/

#ifndef _RP5C01_H_
#define _RP5C01_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class RP5C01 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_alarm;
	outputs_t outputs_pulse;
	
	cur_time_t cur_time;
	int register_id;
	
	uint8 regs[16];
	uint8 time[13];
#ifndef HAS_RP5C15
	uint8 ram[26];
	bool modified;
#endif
	bool alarm, pulse_1hz, pulse_16hz;
	int count_16hz;
	
	void update_pulse();
	void read_from_cur_time();
	void write_to_cur_time();
	
public:
	RP5C01(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs_alarm);
		init_output_signals(&outputs_pulse);
	}
	~RP5C01() {}
	
	// common functions
	void initialize();
	void release();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_alarm(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_alarm, device, id, mask);
	}
	void set_context_pulse(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_pulse, device, id, mask);
	}
};

#endif

