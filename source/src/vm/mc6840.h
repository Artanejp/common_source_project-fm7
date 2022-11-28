/*
	Skelton for retropc emulator

	Origin : MAME 0.168 Motorola 6840 (PTM)
	Author : Takeda.Toshiya
	Date   : 2016.02.24-

	[ MC6840 ]
*/

#ifndef _MC6840_H_
#define _MC6840_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_MC6840_CLOCK_0	0
#define SIG_MC6840_CLOCK_1	1
#define SIG_MC6840_CLOCK_2	2
#define SIG_MC6840_GATE_0	3
#define SIG_MC6840_GATE_1	4
#define SIG_MC6840_GATE_2	5

class MC6840 : public DEVICE
{
private:
	enum
	{
		PTM_6840_CTRL1   = 0,
		PTM_6840_CTRL2   = 1,
		PTM_6840_STATUS  = 1,
		PTM_6840_MSBBUF1 = 2,
		PTM_6840_LSB1    = 3,
		PTM_6840_MSBBUF2 = 4,
		PTM_6840_LSB2    = 5,
		PTM_6840_MSBBUF3 = 6,
		PTM_6840_LSB3    = 7
	};
	
	double m_internal_clock;
	double m_external_clock[3];
	
	outputs_t outputs_ch0;
	outputs_t outputs_ch1;
	outputs_t outputs_ch2;
	outputs_t outputs_irq;
	
	UINT8 m_control_reg[3];
	UINT8 m_output[3]; // Output states
	UINT8 m_gate[3];   // Input gate states
	UINT8 m_clk[3];  // Clock states
	UINT8 m_enabled[3];
	UINT8 m_mode[3];
	UINT8 m_fired[3];
	UINT8 m_t3_divisor;
	UINT8 m_t3_scaler;
	UINT8 m_IRQ;
	UINT8 m_status_reg;
	UINT8 m_status_read_since_int;
	UINT8 m_lsb_buffer;
	UINT8 m_msb_buffer;
	
	// Each PTM has 3 timers
	int m_timer[3];
	
	UINT16 m_latch[3];
	UINT16 m_counter[3];
	
	void subtract_from_counter(int counter, int count);
	void tick(int counter, int count);
	void update_interrupts();
	UINT16 compute_counter( int counter );
	void reload_count(int idx);
	void timeout(int idx);
	void set_gate(int idx, int state);
	void set_clock(int idx, int state);
	
public:
	MC6840(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_ch0);
		initialize_output_signals(&outputs_ch1);
		initialize_output_signals(&outputs_ch2);
		initialize_output_signals(&outputs_irq);
		m_internal_clock = 0.0;
		m_external_clock[0] = m_external_clock[1] = m_external_clock[2] = 0.0;
		set_device_name(_T("MC6840 Counter/Timer"));
	}
	~MC6840() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t offset, uint32_t data);
	uint32_t read_io8(uint32_t offset);
	void event_callback(int id, int err);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_ch0(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ch0, device, id, mask);
	}
	void set_context_ch1(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ch1, device, id, mask);
	}
	void set_context_ch2(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_ch2, device, id, mask);
	}
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_internal_clock(double clock)
	{
		m_internal_clock = clock;
	}
	void set_external_clock(int ch, double clock)
	{
		m_external_clock[ch] = clock;
	}
};

#endif

