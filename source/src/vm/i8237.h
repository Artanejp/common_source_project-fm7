/*
	Skelton for retropc emulator

	Origin : MESS
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ i8237 ]
*/

#ifndef _I8237_H_
#define _I8237_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I8237_CH0	0
#define SIG_I8237_CH1	1
#define SIG_I8237_CH2	2
#define SIG_I8237_CH3	3
#define SIG_I8237_BANK0	4
#define SIG_I8237_BANK1	5
#define SIG_I8237_BANK2	6
#define SIG_I8237_BANK3	7
#define SIG_I8237_MASK0	8
#define SIG_I8237_MASK1	9
#define SIG_I8237_MASK2	10
#define SIG_I8237_MASK3	11

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class I8237 : public DEVICE
{
private:
	DEVICE* d_mem;
#ifdef SINGLE_MODE_DMA
	DEVICE* d_dma;
#endif
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	
	struct {
		DEVICE* dev;
		uint16_t areg;
		uint16_t creg;
		uint16_t bareg;
		uint16_t bcreg;
		uint8_t mode;
		// external bank
		uint16_t bankreg;
		uint16_t incmask;
		// output tc signals
		outputs_t outputs_tc;
	} dma[4];
	
	bool low_high;
	uint8_t cmd;
	uint8_t req;
	uint8_t mask;
	uint8_t tc;
	uint32_t tmp;
	bool mode_word;
	uint32_t addr_mask;
	
	void write_mem(uint32_t addr, uint32_t data);
	uint32_t read_mem(uint32_t addr);
	void write_io(int ch, uint32_t data);
	uint32_t read_io(int ch);
	
public:
	I8237(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 4; i++) {
			dma[i].dev = vm->dummy;
			dma[i].bankreg = dma[i].incmask = 0;
			initialize_output_signals(&dma[i].outputs_tc);
		}
#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
#endif
#ifdef USE_DEBUGGER
		d_debugger = NULL;
#endif
		mode_word = false;
		addr_mask = 0xffffffff;
		set_device_name(_T("8237 DMAC"));
	}
	~I8237() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);
	void do_dma();
	// for debug
	void write_via_debugger_data8(uint32_t addr, uint32_t data);
	uint32_t read_via_debugger_data8(uint32_t addr);
	void write_via_debugger_data16(uint32_t addr, uint32_t data);
	uint32_t read_via_debugger_data16(uint32_t addr);
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
	void set_context_tc0(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&dma[0].outputs_tc, device, id, mask);
	}
	void set_context_tc1(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&dma[1].outputs_tc, device, id, mask);
	}
	void set_context_tc2(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&dma[2].outputs_tc, device, id, mask);
	}
	void set_context_tc3(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&dma[3].outputs_tc, device, id, mask);
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
	void set_mode_word(bool val)
	{
		mode_word = val;
	}
	void set_address_mask(uint32_t val)
	{
		addr_mask = val;
	}
};

#endif

