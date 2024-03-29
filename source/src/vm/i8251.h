/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.14 -

	[ i8251 ]
*/

#ifndef _I8251_H_
#define _I8251_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_I8251_RECV		0
#define SIG_I8251_BREAK		1
#define SIG_I8251_DSR		2
#define SIG_I8251_CLEAR		3
#define SIG_I8251_LOOPBACK	4

class FIFO;
class  DLL_PREFIX I8251 : public DEVICE
{
private:
	// i8251
	uint8_t recv, status, mode;
	bool txen, rxen, loopback;
	
	// output signals
	outputs_t outputs_out;
	outputs_t outputs_rxrdy;
	outputs_t outputs_syndet;
	outputs_t outputs_txrdy;
	outputs_t outputs_txe;
	outputs_t outputs_dtr;
	outputs_t outputs_brk;
	outputs_t outputs_rts;
	
	// buffer
	FIFO *recv_buffer;
	FIFO *send_buffer;
	int recv_id, send_id;
	
public:
	I8251(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_out);
		initialize_output_signals(&outputs_rxrdy);
		initialize_output_signals(&outputs_syndet);
		initialize_output_signals(&outputs_txrdy);
		initialize_output_signals(&outputs_txe);
		initialize_output_signals(&outputs_dtr);
		initialize_output_signals(&outputs_brk);
		initialize_output_signals(&outputs_rts);
		set_device_name(_T("i8251 SIO"));
	}
	~I8251() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	void __FASTCALL event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);

	// unique functions
	void set_context_out(DEVICE* device, int id)
	{
		register_output_signal(&outputs_out, device, id, 0xff);
	}
	void set_context_rxrdy(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_rxrdy, device, id, mask);
	}
	void set_context_syndet(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_syndet, device, id, mask);
	}
	void set_context_txrdy(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_txrdy, device, id, mask);
	}
	void set_context_txempty(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_txe, device, id, mask);
	}
	void set_context_dtr(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_dtr, device, id, mask);
	}
	void set_context_brk(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_brk, device, id, mask);
	}
	void set_context_rts(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_rts, device, id, mask);
	}
};

#endif

