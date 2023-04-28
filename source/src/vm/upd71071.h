/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2007.08.14 -

	[ uPD71071 ]
*/

#ifndef _UPD71071_H_
#define _UPD71071_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_UPD71071_CH0	0
#define SIG_UPD71071_CH1	1
#define SIG_UPD71071_CH2	2
#define SIG_UPD71071_CH3	3

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class UPD71071 : public DEVICE
{
private:
	DEVICE* d_cpu;
	DEVICE* d_mem;
#ifdef SINGLE_MODE_DMA
	DEVICE* d_dma;
#endif
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	outputs_t outputs_tc;
	
	struct {
		DEVICE* dev;
		uint32_t areg, bareg;
		uint16_t creg, bcreg;
		uint8_t mode;
	} dma[4];
	
	uint8_t b16, selch, base;
	uint16_t cmd, tmp;
	uint8_t req, sreq, mask, tc;
	bool running;
	
public:
	UPD71071(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 4; i++) {
			dma[i].dev = vm->dummy;
		}
		d_cpu = NULL;
#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
#endif
#ifdef USE_DEBUGGER
		d_debugger = NULL;
#endif
		initialize_output_signals(&outputs_tc);
		set_device_name(_T("uPD71071 DMAC"));
	}
	~UPD71071() {}
	
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
		dma[0].dev = device;
	}
	void set_context_ch1(DEVICE* device)
	{
		dma[1].dev = device;
	}
	void set_context_ch2(DEVICE* device)
	{
		dma[2].dev = device;
	}
	void set_context_ch3(DEVICE* device)
	{
		dma[3].dev = device;
	}
#ifdef SINGLE_MODE_DMA
	void set_context_child_dma(DEVICE* device)
	{
		d_dma = device;
	}
#endif
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
	void set_context_tc(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_tc, device, id, mask);
	}
};

#endif

