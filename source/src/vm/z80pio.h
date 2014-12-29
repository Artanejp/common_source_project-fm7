/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.17 -

	[ Z80PIO ]
*/

#ifndef _Z80PIO_H_
#define _Z80PIO_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_Z80PIO_PORT_A	0
#define SIG_Z80PIO_PORT_B	1
#define SIG_Z80PIO_STROBE_A	2
#define SIG_Z80PIO_STROBE_B	3

class Z80PIO : public DEVICE
{
private:
	struct {
		uint32 wreg;
		uint8 rreg;
		uint8 mode;
		uint8 ctrl1;
		uint8 ctrl2;
		uint8 dir;
		uint8 mask;
		uint8 vector;
		bool set_dir;
		bool set_mask;
		// ready signal
		bool hand_shake;
		int ready_signal;
		bool input_empty;
		bool output_ready;
		// interrupt
		bool enb_intr;
		bool enb_intr_tmp;
		bool req_intr;
		bool in_service;
		// output signals
		outputs_t outputs_data;
		outputs_t outputs_ready;
	} port[2];
	
	void update_ready();
	void check_mode3_intr(int ch);
	
	// daisy chain
	DEVICE *d_cpu, *d_child;
	bool iei, oei;
	uint32 intr_bit;
	void update_intr();
	
public:
	Z80PIO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(port, 0, sizeof(port));
		for(int i = 0; i < 2; i++) {
			init_output_signals(&port[i].outputs_data);
			init_output_signals(&port[i].outputs_ready);
			port[i].wreg = 0xffffff00;
			port[i].rreg = 0;
		}
		d_cpu = d_child = NULL;
	}
	~Z80PIO() {}
	
	// common functions
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
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
	void set_context_port_a(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[0].outputs_data, device, id, mask, shift);
	}
	void set_context_port_b(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&port[1].outputs_data, device, id, mask, shift);
	}
	void set_context_ready_a(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[0].outputs_ready, device, id, mask);
	}
	void set_context_ready_b(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&port[1].outputs_ready, device, id, mask);
	}
	void set_hand_shake(int ch, bool value)
	{
		port[ch].hand_shake = value;
	}
};

#endif

