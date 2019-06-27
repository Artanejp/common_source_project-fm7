/*
	Skelton for retropc emulator

	Origin : MAME i286 core
	Author : Takeda.Toshiya
	Date  : 2012.10.18-

	[ i286 ]
*/
#pragma once
#ifndef _I86_H_ 
#define _I86_H_

//#include "fileio.h"
//#include "vm_template.h"
//#include "../emu.h"
#include "device.h"

#define SIG_I86_TEST	0

class DEBUGGER;

enum {
	N_CPU_TYPE_I8086 = 0,
	N_CPU_TYPE_I8088,
	N_CPU_TYPE_I80186,
	N_CPU_TYPE_V30,
	N_CPU_TYPE_I80286,
};	


class I8086 : public DEVICE
{
protected:
	DEVICE *d_mem, *d_io, *d_pic;
	DEVICE *d_bios;
	DEVICE *d_dma;
	DEBUGGER *d_debugger;
	void *opaque;
	int n_cpu_type;
	bool _HAS_i80286;
	bool _HAS_v30;
	void cpu_reset_generic();

public:
	I8086(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_bios = NULL;
		d_dma = NULL;
		d_debugger = NULL;;
	}
	~I8086() {}
	
	// common functions
	virtual void initialize();
	virtual void release();
	virtual void reset();
	virtual int run(int icount);
	virtual uint32_t __FASTCALL read_signal(int id);
	virtual void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	virtual void set_intr_line(bool line, bool pending, uint32_t bit);
	virtual void set_extra_clock(int icount);
	virtual int get_extra_clock();
	virtual uint32_t get_pc();
	virtual uint32_t get_next_pc();
	virtual uint32_t __FASTCALL translate_address(int segment, uint32_t offset);


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
	virtual uint32_t get_debug_prog_addr_mask()
	{
		return 0xfffff;
	}
	virtual uint32_t get_debug_data_addr_mask()
	{
		return 0xfffff;
	}
	virtual void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	virtual void __FASTCALL write_debug_data16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_data16(uint32_t addr);
	virtual void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_io8(uint32_t addr);
	virtual void __FASTCALL write_debug_io16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_io16(uint32_t addr);
	virtual bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	virtual uint32_t __FASTCALL read_debug_reg(const _TCHAR *reg);
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
	virtual bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_context_intr(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_bios(DEVICE* device)
	{
		d_bios = device;
	}
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}

};

#endif
