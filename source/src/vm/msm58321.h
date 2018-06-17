/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.05.02-

	[ MSM58321/MSM5832 ]
*/

#ifndef _MSM58321_H_
#define _MSM58321_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_MSM58321_DATA	0
#define SIG_MSM58321_CS		1
#define SIG_MSM58321_READ	2
#define SIG_MSM58321_WRITE	3
// for MSM58321 only
#define SIG_MSM58321_ADDR_WRITE	4
// for MSM5832 only
#define SIG_MSM5832_ADDR	5
#define SIG_MSM5832_HOLD	6

class VM;
class EMU;
class MSM58321_BASE : public DEVICE
{
protected:
	// output signals
	outputs_t outputs_data;
	
	dll_cur_time_t cur_time;
	int register_id;
	
	uint8_t regs[16];
	uint8_t wreg, regnum;
	bool cs, rd, wr, addr_wr, busy, hold;
	int count_1024hz, count_1s, count_1m, count_1h;
	
	virtual void read_from_cur_time();
	virtual void write_to_cur_time();
	void output_data();
	virtual void set_busy(bool val);
	int start_day;
	int start_year;
public:
	MSM58321_BASE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_data);
		set_device_name(_T("MSM5832 RTC"));
		start_day = 0;
		start_year = 0;
	}
	~MSM58321_BASE() {}
	
	// common functions
	virtual void initialize();
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int ch);
	void decl_state();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_data(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&outputs_data, device, id, mask, shift);
	}
};

class MSM58321: public MSM58321_BASE
{
protected:
	outputs_t outputs_busy;
	void read_from_cur_time();
	void write_to_cur_time();
	void set_busy(bool val);
public:
	MSM58321(VM* parent_vm, EMU* parent_emu);
	~MSM58321() {}
	
	void initialize();
	void set_context_busy(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_busy, device, id, mask);
	}
};

class MSM5832: public MSM58321_BASE
{
protected:
	void read_from_cur_time();
	void write_to_cur_time();
	void set_busy(bool val);

public:
	MSM5832(VM* parent_vm, EMU* parent_emu);
	~MSM5832() {}
	
	void initialize();
};
	
#endif

