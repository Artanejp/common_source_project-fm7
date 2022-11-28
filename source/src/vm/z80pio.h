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
		uint32_t wreg;
		uint8_t rreg;
		uint8_t mode;
		uint8_t ctrl1;
		uint8_t ctrl2;
		uint8_t dir;
		uint8_t mask;
		uint8_t vector;
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
	uint32_t intr_bit;
	void update_intr();
	
public:
	Z80PIO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(port, 0, sizeof(port));
		for(int i = 0; i < 2; i++) {
			initialize_output_signals(&port[i].outputs_data);
			initialize_output_signals(&port[i].outputs_ready);
			port[i].wreg = 0xffffff00;
			port[i].rreg = 0;
		}
		d_cpu = d_child = NULL;
		set_device_name(_T("Z80 PIO"));
	}
	~Z80PIO() {}
	
	// common functions
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// interrupt common functions
	void set_context_intr(DEVICE* device, uint32_t bit)
	{
		d_cpu = device;
		intr_bit = bit;
	}
	void set_context_child(DEVICE* device)
	{
		d_child = device;
	}
	DEVICE *get_context_child()
	{
		return d_child;
	}
	void set_intr_iei(bool val);
	uint32_t get_intr_ack();
	void notify_intr_reti();
	
	// unique functions
	void set_context_port_a(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[0].outputs_data, device, id, mask, shift);
	}
	void set_context_port_b(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&port[1].outputs_data, device, id, mask, shift);
	}
	void set_context_ready_a(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&port[0].outputs_ready, device, id, mask);
	}
	void set_context_ready_b(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&port[1].outputs_ready, device, id, mask);
	}
	void set_hand_shake(int ch, bool value)
	{
		port[ch].hand_shake = value;
	}
};

#endif

