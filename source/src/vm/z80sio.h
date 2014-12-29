/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.12.31 -

	[ Z80SIO ]
*/

#ifndef _Z80SIO_H_
#define _Z80SIO_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_Z80SIO_RECV_CH0	0
#define SIG_Z80SIO_RECV_CH1	1
#define SIG_Z80SIO_BREAK_CH0	2
#define SIG_Z80SIO_BREAK_CH1	3
#define SIG_Z80SIO_DCD_CH0	4
#define SIG_Z80SIO_DCD_CH1	5
#define SIG_Z80SIO_CTS_CH0	6
#define SIG_Z80SIO_CTS_CH1	7
#define SIG_Z80SIO_SYNC_CH0	8
#define SIG_Z80SIO_SYNC_CH1	9
// hack: clear recv buffer
#define SIG_Z80SIO_CLEAR_CH0	10
#define SIG_Z80SIO_CLEAR_CH1	11

class FIFO;

class Z80SIO : public DEVICE
{
private:
	struct {
		int pointer;
		uint8 wr[8];
		uint8 vector;
		uint8 affect;
		bool nextrecv_intr;
		bool first_data;
		bool over_flow;
		bool under_run;
		bool abort;
		bool sync;
		uint8 sync_bit;
#ifdef UPD7201
		uint16 tx_count;
#endif
		// buffer
		FIFO* send;
		FIFO* recv;
		FIFO* rtmp;
		int send_id;
		int recv_id;
		// interrupt
		bool err_intr;
		int recv_intr;
		bool stat_intr;
		bool send_intr;
		bool req_intr;
		bool in_service;
		// input signals
		bool dcd;
		bool cts;
		// output signals
		outputs_t outputs_rts;
		outputs_t outputs_dtr;
		outputs_t outputs_send;
		outputs_t outputs_sync;
		outputs_t outputs_break;
		outputs_t outputs_txdone;
		outputs_t outputs_rxdone;
	} port[2];
	
	// daisy chain
	DEVICE *d_cpu, *d_child;
	bool iei, oei;
	uint32 intr_bit;
	void update_intr();
	
public:
	Z80SIO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(port, 0, sizeof(port));
		for(int i = 0; i < 2; i++) {
			init_output_signals(&port[i].outputs_rts);
			init_output_signals(&port[i].outputs_dtr);
			init_output_signals(&port[i].outputs_send);
			init_output_signals(&port[i].outputs_sync);
			init_output_signals(&port[i].outputs_break);
			init_output_signals(&port[i].outputs_txdone);
			init_output_signals(&port[i].outputs_rxdone);
		}
		d_cpu = d_child = NULL;
	}
	~Z80SIO() {}
	
	// common functions
	void initialize();
	void reset();
	void release();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
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
	void set_context_rts0(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[0].outputs_rts, device, id, mask);
	}
	void set_context_dtr0(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[0].outputs_dtr, device, id, mask);
	}
	void set_context_send0(DEVICE* device, int id)
	{
		register_output_signal(&port[0].outputs_send, device, id, 0xff);
	}
	void set_context_sync0(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[0].outputs_sync, device, id, mask);
	}
	void set_context_break0(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[0].outputs_break, device, id, mask);
	}
	void set_context_rxdone0(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[0].outputs_rxdone, device, id, mask);
	}
	void set_context_txdone0(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[0].outputs_txdone, device, id, mask);
	}
	void set_context_rts1(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[1].outputs_rts, device, id, mask);
	}
	void set_context_dtr1(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[1].outputs_dtr, device, id, mask);
	}
	void set_context_send1(DEVICE* device, int id)
	{
		register_output_signal(&port[1].outputs_send, device, id, 0xff);
	}
	void set_context_sync1(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[1].outputs_sync, device, id, mask);
	}
	void set_context_break1(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[1].outputs_break, device, id, mask);
	}
	void set_context_rxdone1(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[1].outputs_rxdone, device, id, mask);
	}
	void set_context_txdone1(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[1].outputs_txdone, device, id, mask);
	}
};

#endif

