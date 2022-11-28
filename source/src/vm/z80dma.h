/*
	Skelton for retropc emulator

	Origin : MAME Z80DMA / Xmillenium
	Author : Takeda.Toshiya
	Date   : 2011.04.96-

	[ Z80DMA ]
*/

#ifndef _Z80DMA_H_
#define _Z80DMA_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_Z80DMA_READY	0

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class Z80DMA : public DEVICE
{
private:
	DEVICE *d_mem, *d_io;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	
	typedef union {
		uint16_t m[7][8];
		uint16_t t[6*8+1+1];
	} regs_t;
	regs_t regs;
	uint8_t status;
	
	uint16_t wr_tmp[4];
	int wr_num, wr_ptr;
	uint16_t rr_tmp[7];
	int rr_num, rr_ptr;
	
	bool enabled;
	uint32_t ready;
	bool force_ready;
	bool enalbe_after_reti;
	
	uint16_t addr_a;
	uint16_t addr_b;
	int upcount;
	int blocklen;
	bool dma_stop;
	bool bus_master;
	
	void write_memory(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_memory(uint32_t addr, int* wait);
	void write_ioport(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_ioport(uint32_t addr, int* wait);
	
	// interrupt
	bool req_intr;
	bool in_service;
	uint8_t vector;
	
	bool now_ready();
	void request_bus();
	void release_bus();
	void update_read_buffer();
	void request_intr(int level);
	
	// daisy chain
	DEVICE *d_cpu, *d_child;
	bool iei, oei;
	uint32_t intr_bit;
	void update_intr();
	
public:
	Z80DMA(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		for(int i = 0; i < 6 * 8 + 1 + 1; i++) {
			regs.t[i] = 0;
		}
		d_cpu = d_child = NULL;
#ifdef USE_DEBUGGER
		d_debugger = NULL;
#endif
		set_device_name(_T("Z80 DMA"));
	}
	~Z80DMA() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void do_dma();
	// for debug
	void write_via_debugger_data8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_via_debugger_data8w(uint32_t addr, int* wait);
	void write_via_debugger_io8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_via_debugger_io8w(uint32_t addr, int* wait);
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
	
	// unique function
	void set_context_memory(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif

