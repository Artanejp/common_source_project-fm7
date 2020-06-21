/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2010.08.10-

	[ M6502 ]
*/

#ifndef _M6502_H_ 
#define _M6502_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define SIG_M6502_OVERFLOW	0

//#ifdef USE_DEBUGGER
class DEBUGGER;
//#endif

class M6502_BASE : public DEVICE
{
protected:
	DEVICE *d_mem, *d_pic;
//#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored;
//#endif
	
	pair32_t pc, sp, zp, ea;
	uint16_t prev_pc;
	uint8_t a, x, y, p;
	bool pending_irq, after_cli;
	bool nmi_state, irq_state, so_state;

	uint64_t total_icount;
	uint64_t prev_total_icount;

	int icount;
	bool busreq;
	
	void __FASTCALL run_one_opecode();
	virtual void __FASTCALL OP(uint8_t code);
	void __FASTCALL update_irq();

public:
	M6502_BASE(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		total_icount = prev_total_icount = 0;
		busreq = false;
		d_debugger = NULL;
		d_mem = NULL;
		d_mem_stored = NULL;
		d_pic = NULL;
		set_device_name(_T("M6502 CPU"));
	}
	~M6502_BASE() {}
	
	// common functions
	virtual void initialize();
	virtual void reset();
	virtual int __FASTCALL run(int clock);
	
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
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
//#ifdef USE_DEBUGGER
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
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
//#endif
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_intr(DEVICE* device)
	{
		d_pic = device;
	}
//#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
//#endif
};

class M6502 : public M6502_BASE
{
protected:
	void __FASTCALL OP(uint8_t code);
public:
	M6502(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : M6502_BASE(parent_vm, parent_emu)
	{
	}
	~M6502() {}
	void initialize();
	void reset();
	int __FASTCALL run(int clock);
	int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
	bool process_state(FILEIO* state_fio, bool loading);
};	

class N2A03 : public M6502_BASE
{
protected:
	void __FASTCALL OP(uint8_t code);
public:
	N2A03(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : M6502_BASE(parent_vm, parent_emu)
	{
	}
	~N2A03() {}
	void initialize();
	void reset();
	int __FASTCALL run(int clock);
	int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
	bool process_state(FILEIO* state_fio, bool loading);
};	

#endif

