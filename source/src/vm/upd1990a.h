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
	
	cur_time_t cur_time;
	int register_id_1sec;
	
	uint8 cmd, mode, tpmode;
	uint64 shift_data;
	bool clk, stb, din, hold, tp;
	uint32 dout;
	bool dout_changed;
	int register_id_tp;
	
#ifdef HAS_UPD4990A
	uint8 shift_cmd;
#endif
	
public:
	UPD1990A(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs_dout);
		init_output_signals(&outputs_tp);
		cmd = mode = tpmode = 0;
		shift_data = 0;
		clk = stb = din = tp = true;
		hold = false;
		dout = 0;
		dout_changed = false;
#ifdef HAS_UPD4990A
		shift_cmd = 0;
#endif
	}
	~UPD1990A() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_signal(int ch)
	{
		return dout;
	}
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_dout(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_dout, device, id, mask);
	}
	void set_context_tp(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_tp, device, id, mask);
	}
};

#endif

