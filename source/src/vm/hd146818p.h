/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.10.11 -

	[ HD146818P ]
*/

#ifndef _HD146818P_H_
#define _HD146818P_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class HD146818P : public DEVICE
{
private:
	// output signals
	outputs_t outputs_intr;
	outputs_t outputs_sqw;
	
	cur_time_t cur_time;
	int register_id_1sec;
	
	uint8 regs[0x40];
	int ch, period, register_id_sqw;
	bool intr, sqw, modified;
	
	void read_from_cur_time();
	void write_to_cur_time();
	void check_alarm();
	void update_intr();
	
public:
	HD146818P(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs_intr);
		init_output_signals(&outputs_sqw);
	}
	~HD146818P() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_callback(int event_id, int err);
	
	// unique functions
	void set_context_intr(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_intr, device, id, mask);
	}
	void set_context_sqw(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_sqw, device, id, mask);
	}
};

#endif

