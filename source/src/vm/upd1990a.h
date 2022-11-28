/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.04.19-

	[ uPD1990A / uPD4990A ]
*/

#ifndef _UPD1990A_H_
#define _UPD1990A_H_

#define SIG_UPD1990A_CLK	0
#define SIG_UPD1990A_STB	1
#define SIG_UPD1990A_CMD	2
#define SIG_UPD1990A_C0		3
#define SIG_UPD1990A_C1		4
#define SIG_UPD1990A_C2		5
#define SIG_UPD1990A_DIN	6

#include "vm.h"
#include "../emu.h"
#include "device.h"

class UPD1990A : public DEVICE
{
private:
	// output signals
	outputs_t outputs_dout;
	outputs_t outputs_tp;
	
	dll_cur_time_t cur_time;
	int register_id_1sec;
	
	uint8_t cmd, mode, tpmode;
	uint64_t shift_data;
	bool clk, stb, din, hold, tp;
	uint32_t dout;
	bool dout_changed;
	int register_id_tp;
	
#ifdef HAS_UPD4990A
	uint8_t shift_cmd;
#endif
	
public:
	UPD1990A(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_dout);
		initialize_output_signals(&outputs_tp);
		cmd = mode = tpmode = 0;
		shift_data = 0;
		clk = stb = din = tp = true;
		hold = false;
		dout = 0;
		dout_changed = false;
#ifdef HAS_UPD4990A
		shift_cmd = 0;
		set_device_name(_T("uPD4990A RTC"));
#else
		set_device_name(_T("uPD1990A RTC"));
#endif
	}
	~UPD1990A() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch)
	{
		return dout;
	}
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_dout(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_dout, device, id, mask);
	}
	void set_context_tp(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_tp, device, id, mask);
	}
};

#endif

