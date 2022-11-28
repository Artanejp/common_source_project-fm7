/*
	Skelton for retropc emulator

	Origin : MESS 0.147
	Author : Takeda.Toshiya
	Date   : 2012.10.23-

	[ HuC6280 ]
*/

#ifndef _HUC6280_H_ 
#define _HUC6280_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class HUC6280 : public DEVICE
{
private:
	DEVICE *d_mem, *d_io;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	void *opaque;
#ifdef USE_DEBUGGER
	uint64_t total_icount;
	uint64_t prev_total_icount;
#endif
	int icount;
	bool busreq;
	
	int run_one_opecode();
	
public:
	HUC6280(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef USE_DEBUGGER
		total_icount = prev_total_icount = 0;
#endif
		set_device_name(_T("HuC6280 CPU"));
	}
	~HUC6280() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t get_pc();
	uint32_t get_next_pc();
#ifdef USE_DEBUGGER
	bool is_cpu()
	{
		return true;
	}
	bool is_debugger_available()
	{
		return true;
	}
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32_t get_debug_prog_addr_mask()
	{
		return 0xffff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0x1fffff;
	}
	void write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t read_debug_data8(uint32_t addr);
	void write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t read_debug_io8(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_mem(DEVICE* device)
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
	uint8_t irq_status_r(uint16_t offset);
	void irq_status_w(uint16_t offset, uint8_t data);
	uint8_t timer_r(uint16_t offset);
	void timer_w(uint16_t offset, uint8_t data);
};

#endif

