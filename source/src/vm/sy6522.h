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

#ifndef _SY6522_H_
#define _SY6522_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_SY6522_PORT_A	0
#define SIG_SY6522_PORT_B	1
#define SIG_SY6522_PORT_CA1	2
#define SIG_SY6522_PORT_CA2	3
#define SIG_SY6522_PORT_CB1	4
#define SIG_SY6522_PORT_CB2	5

class SY6522 : public DEVICE
{
private:
	outputs_t outputs_a;
	outputs_t outputs_b;
	outputs_t outputs_ca2;
	outputs_t outputs_cb1;
	outputs_t outputs_cb2;
	outputs_t outputs_irq;

	uint32_t clock;

	uint16_t get_counter1_value();

	void set_int(int data);
	void clear_int(int data);
	void shift_out();
	void shift_in();

	uint8_t input_pa();
	void output_pa();
	uint8_t input_pb();
	void output_pb();
	void output_irq();

	uint8_t m_in_a;
	int m_in_ca1;
	int m_in_ca2;
	uint8_t m_out_a;
	int m_out_ca2;
	uint8_t m_ddr_a;
	uint8_t m_latch_a;

	uint8_t m_in_b;
	int m_in_cb1;
	int m_in_cb2;
	uint8_t m_out_b;
	int m_out_cb1;
	int m_out_cb2;
	uint8_t m_ddr_b;
	uint8_t m_latch_b;

	uint8_t m_t1cl;
	uint8_t m_t1ch;
	uint8_t m_t1ll;
	uint8_t m_t1lh;
	uint8_t m_t2cl;
	uint8_t m_t2ch;
	uint8_t m_t2ll;
	uint8_t m_t2lh;

	uint8_t m_sr;
	uint8_t m_pcr;
	uint8_t m_acr;
	uint8_t m_ier;
	uint8_t m_ifr;

	int m_t1;
	uint32_t m_time1;
	uint8_t m_t1_active;
	int m_t1_pb7;
	int m_t2;
	uint32_t m_time2;
	uint8_t m_t2_active;
	int m_ca2_timer;

	int m_shift_timer;
	uint8_t m_shift_counter;

public:
	SY6522(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_a);
		initialize_output_signals(&outputs_b);
		initialize_output_signals(&outputs_ca2);
		initialize_output_signals(&outputs_cb1);
		initialize_output_signals(&outputs_cb2);
		initialize_output_signals(&outputs_irq);
		clock = CPU_CLOCKS;
		set_device_name(_T("SY6522 VIA"));
	}
	~SY6522() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_port_a(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&outputs_a, device, id, mask, shift);
	}
	void set_context_port_b(DEVICE* device, int id, uint32_t mask, int shift)
	{
		register_output_signal(&outputs_b, device, id, mask, shift);
	}
	void set_context_ca2(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ca2, device, id, mask);
	}
	void set_context_cb1(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_cb1, device, id, mask);
	}
	void set_context_cb2(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_cb2, device, id, mask);
	}
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_constant_clock(uint32_t hz)
	{
		clock = hz;
	}
};

#endif
