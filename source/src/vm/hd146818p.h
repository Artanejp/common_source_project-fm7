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
	
	dll_cur_time_t cur_time;
	int register_id_1sec;
	
	uint8_t regs[0x40];
	int ch, period, register_id_sqw;
	bool intr, sqw, modified;
	
	void read_from_cur_time();
	void write_to_cur_time();
	void check_alarm();
	void update_intr();
	
public:
	HD146818P(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_intr);
		initialize_output_signals(&outputs_sqw);
		set_device_name(_T("HD146818P RTC"));
	}
	~HD146818P() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_intr(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_intr, device, id, mask);
	}
	void set_context_sqw(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_sqw, device, id, mask);
	}
};

#endif

