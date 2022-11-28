/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2010.08.10-

	[ M6502 ]
*/

#ifndef _M6502_H_ 
#define _M6502_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_M6502_OVERFLOW	0

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class M6502 : public DEVICE
{
private:
	DEVICE *d_mem, *d_pic;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored;
#endif
	
	pair32_t pc, sp, zp, ea;
	uint16_t prev_pc;
	uint8_t a, x, y, p;
	bool pending_irq, after_cli;
	bool nmi_state, irq_state, so_state;
#ifdef USE_DEBUGGER
	uint64_t total_icount;
	uint64_t prev_total_icount;
#endif
	int icount;
	bool busreq;
	
	void run_one_opecode();
	void OP(uint8_t code);
	void update_irq();
	
public:
	M6502(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef USE_DEBUGGER
		total_icount = prev_total_icount = 0;
#endif
		busreq = false;
#ifdef HAS_N2A03
		set_device_name(_T("N2A03 CPU"));
#else
		set_device_name(_T("M6502 CPU"));
#endif
	}
	~M6502() {}
	
	// common functions
	void initialize();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void set_intr_line(bool line, bool pending, uint32_t bit)
	{
		write_signal(SIG_CPU_IRQ, line ? 1 : 0, 1);
	}
	uint32_t get_pc()
	{
		return prev_pc;
	}
	uint32_t get_next_pc()
	{
		return pc.w.l;
	}
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
		return 0xffff;
	}
	void write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t read_debug_data8(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_intr(DEVICE* device)
	{
		d_pic = device;
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif

