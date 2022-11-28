/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2017.05.09-

	[ MC6850 ]
*/

#ifndef _MC6850_H_
#define _MC6850_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_MC6850_RECV		0
#define SIG_MC6850_DCD		1
#define SIG_MC6850_CTS		2
#define SIG_MC6850_CLEAR	3

class FIFO;

class MC6850 : public DEVICE
{
private:
	uint8_t recv, status, ctrl;
	
	outputs_t outputs_out;
	outputs_t outputs_rts;
	outputs_t outputs_irq;
	
	FIFO *recv_buffer;
	FIFO *send_buffer;
	int recv_id, send_id;
	
	void update_irq();
	
public:
	MC6850(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_out);
		initialize_output_signals(&outputs_rts);
		initialize_output_signals(&outputs_irq);
		set_device_name(_T("MC6850 ACIA"));
	}
	~MC6850() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_out(DEVICE* device, int id)
	{
		register_output_signal(&outputs_out, device, id, 0xff);
	}
	void set_context_rts(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_rts, device, id, mask);
	}
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}
};

#endif

