/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2023.06.09-

	[ i8257 ]
*/

#ifndef _I8257_H_
#define _I8257_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I8257_CH0	0
#define SIG_I8257_CH1	1
#define SIG_I8257_CH2	2
#define SIG_I8257_CH3	3

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class I8257 : public DEVICE
{
private:
	DEVICE *d_cpu;
	DEVICE *d_mem;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	
	struct {
		pair32_t addr, count;
		uint8_t mode;
		DEVICE* io;
		bool running;
	} ch[4];
	uint8_t mode, status;
	bool high_low;
	outputs_t outputs_tc;
	
	void do_dma(int c);
	void write_mem(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_mem(uint32_t addr, int *wait);
	void write_io(int c, uint32_t data, int *wait);
	uint32_t read_io(int c, int *wait);
	
public:
	I8257(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int c = 0; c < 4; c++) {
			ch[c].io = vm->dummy;
		}
		initialize_output_signals(&outputs_tc);
		d_cpu = NULL;
#ifdef USE_DEBUGGER
		d_debugger = NULL;
#endif
		set_device_name(_T("8257 DMAC"));
	}
	~I8257() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void do_dma();
	// for debug
	void write_via_debugger_data8w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_via_debugger_data8w(uint32_t addr, int *wait);
	void write_via_debugger_data16w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_via_debugger_data16w(uint32_t addr, int *wait);
#ifdef USE_DEBUGGER
	bool is_debugger_available()
	{
		return true;
	}
	void *get_debugger()
	{
		return d_debugger;
	}
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_memory(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_ch0(DEVICE* device)
	{
		ch[0].io = device;
	}
	void set_context_ch1(DEVICE* device)
	{
		ch[1].io = device;
	}
	void set_context_ch2(DEVICE* device)
	{
		ch[2].io = device;
	}
	void set_context_ch3(DEVICE* device)
	{
		ch[3].io = device;
	}
	void set_context_tc(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_tc, device, id, mask);
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif

