/*
	Skelton for retropc emulator

	Origin : MAME 0.164 Rockwell 6522 VIA
	Author : Takeda.Toshiya
	Date   : 2015.08.27-

	[ SY6522 ]
*/

// license:BSD-3-Clause
// copyright-holders:Peter Trauner, Mathis Rosenhauer
/**********************************************************************

    Rockwell 6522 VIA interface and emulation

    This function emulates all the functionality of 6522
    versatile interface adapters.

    This is based on the pre-existing 6821 emulation.

    Written by Mathis Rosenhauer

**********************************************************************/

#ifndef _SY6552_H_
#define _SY6552_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_SY6552_PORT_A	0
#define SIG_SY6552_PORT_B	1
#define SIG_SY6552_PORT_CA1	2
#define SIG_SY6552_PORT_CA2	3
#define SIG_SY6552_PORT_CB1	4
#define SIG_SY6552_PORT_CB2	5

class SY6552 : public DEVICE
{
private:
	outputs_t outputs_a;
	outputs_t outputs_b;
	outputs_t outputs_ca2;
	outputs_t outputs_cb1;
	outputs_t outputs_cb2;
	outputs_t outputs_irq;

	uint32 clock;

	uint16 get_counter1_value();

	void set_int(int data);
	void clear_int(int data);
	void shift_out();
	void shift_in();

	uint8 input_pa();
	void output_pa();
	uint8 input_pb();
	void output_pb();
	void output_irq();

	uint8 m_in_a;
	int m_in_ca1;
	int m_in_ca2;
	uint8 m_out_a;
	int m_out_ca2;
	uint8 m_ddr_a;
	uint8 m_latch_a;

	uint8 m_in_b;
	int m_in_cb1;
	int m_in_cb2;
	uint8 m_out_b;
	int m_out_cb1;
	int m_out_cb2;
	uint8 m_ddr_b;
	uint8 m_latch_b;

	uint8 m_t1cl;
	uint8 m_t1ch;
	uint8 m_t1ll;
	uint8 m_t1lh;
	uint8 m_t2cl;
	uint8 m_t2ch;
	uint8 m_t2ll;
	uint8 m_t2lh;

	uint8 m_sr;
	uint8 m_pcr;
	uint8 m_acr;
	uint8 m_ier;
	uint8 m_ifr;

	int m_t1;
	uint32 m_time1;
	uint8 m_t1_active;
	int m_t1_pb7;
	int m_t2;
	uint32 m_time2;
	uint8 m_t2_active;
	int m_ca2_timer;

	int m_shift_timer;
	uint8 m_shift_counter;

public:
	SY6552(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_a);
		initialize_output_signals(&outputs_b);
		initialize_output_signals(&outputs_ca2);
		initialize_output_signals(&outputs_cb1);
		initialize_output_signals(&outputs_cb2);
		initialize_output_signals(&outputs_irq);
		clock = CPU_CLOCKS;
	}
	~SY6552() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32 data, uint32 mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_port_a(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&outputs_a, device, id, mask, shift);
	}
	void set_context_port_b(DEVICE* device, int id, uint32 mask, int shift)
	{
		register_output_signal(&outputs_b, device, id, mask, shift);
	}
	void set_context_ca2(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_ca2, device, id, mask);
	}
	void set_context_cb1(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_cb1, device, id, mask);
	}
	void set_context_cb2(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_cb2, device, id, mask);
	}
	void set_context_irq(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_constant_clock(uint32 hz)
	{
		clock = hz;
	}
};

#endif
