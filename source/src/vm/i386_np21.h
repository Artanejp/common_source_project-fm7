/*
	Skelton for retropc emulator

	Origin : np21/w i386c core
	Author : Takeda.Toshiya
	Date  : 2020.01.25-

	[ i386/i486/Pentium ]
*/

#ifndef _I386_H_ 
#define _I386_H_

#include "vm_template.h"
//#include "../emu.h"
#include "device.h"

#define SIG_I386_A20	        1
#define SIG_I386_NOTIFY_RESET	2

//#ifdef USE_DEBUGGER
class DEBUGGER;
//#endif
class I8259;

namespace I386_NP21 {
	enum {
	N_CPU_TYPE_I386DX = 0,
	N_CPU_TYPE_I386SX,
	N_CPU_TYPE_I486DX,
	N_CPU_TYPE_I486SX,
	N_CPU_TYPE_PENTIUM,
	N_CPU_TYPE_PENTIUM_PRO,
	N_CPU_TYPE_PENTIUM_MMX,
	N_CPU_TYPE_PENTIUM2,
	N_CPU_TYPE_PENTIUM3,
	N_CPU_TYPE_PENTIUM4
};
}
class I386 : public DEVICE
{
private:
	I8259 *device_pic;
	outputs_t outputs_extreset;
	
//#ifdef USE_DEBUGGER
	DEBUGGER *device_debugger;
	DEVICE *device_mem_stored;
	DEVICE *device_io_stored;
	uint64_t total_cycles;
	uint64_t prev_total_cycles;
//#endif
	int remained_cycles, extra_cycles;
	bool busreq;
	uint32_t CPU_PREV_CS;
	uint32_t waitfactor;
	int64_t waitcount;
	
	bool _USE_DEBUGGER;
	bool _I386_PSEUDO_BIOS;
	bool _SINGLE_MODE_DMA;
	uint32_t n_cpu_type;
	uint32_t address_mask;
	
	int run_one_opecode();
	uint32_t __FASTCALL convert_address(uint32_t cs, uint32_t eip);
	void __FASTCALL cpu_wait(int clocks);

public:
	I386(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
//#if defined(HAS_I386)
		set_device_name(_T("80386 CPU"));
//#elif defined(HAS_I486SX)
//		set_device_name(_T("80486SX CPU"));
//#elif defined(HAS_I486DX)
//		set_device_name(_T("80486DX CPU"));
//#elif defined(HAS_PENTIUM)
//		set_device_name(_T("Pentium CPU"));
//#elif defined(HAS_PENTIUM_PRO)
//		set_device_name(_T("Pentium Pro CPU"));
//#elif defined(HAS_PENTIUM_MMX)
//		set_device_name(_T("Pentium MMX CPU"));
//#elif defined(HAS_PENTIUM2)
//		set_device_name(_T("Pentium2 CPU"));
//#elif defined(HAS_PENTIUM3)
//		set_device_name(_T("Pentium3 CPU"));
//#elif defined(HAS_PENTIUM4)
//		set_device_name(_T("Pentium4 CPU"));
//#endif
//#ifdef USE_DEBUGGER
		total_cycles = prev_total_cycles = 0;
//#endif
		busreq = false;
		initialize_output_signals(&outputs_extreset);
		_USE_DEBUGGER = false;
		_I386_PSEUDO_BIOS = false;
		_SINGLE_MODE_DMA = false;
		address_mask = 0x000fffff; // OK?
		n_cpu_type = I386_NP21::N_CPU_TYPE_I386DX;
	}
	~I386() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int __FASTCALL run(int cycles);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	void set_intr_line(bool line, bool pending, uint32_t bit);
	void set_extra_clock(int cycles);
	int get_extra_clock();
	uint32_t get_pc();
	uint32_t get_next_pc();
//#ifdef USE_DEBUGGER
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
		return device_debugger;
	}
	uint32_t get_debug_prog_addr_mask()
	{
		return 0xffffffff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xffffffff;
	}
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	void __FASTCALL write_debug_data16(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data16(uint32_t addr);
	void __FASTCALL write_debug_data32(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data32(uint32_t addr);
	void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_io8(uint32_t addr);
	void __FASTCALL write_debug_io16(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_io32(uint32_t addr);
	void __FASTCALL write_debug_io32(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_io16(uint32_t addr);
	bool __FASTCALL write_debug_reg(const _TCHAR *reg, uint32_t data);
	uint32_t __FASTCALL read_debug_reg(const _TCHAR *reg);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
//#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_mem(DEVICE* device);
//	{
//		device_mem = device;
//	}
	void set_context_io(DEVICE* device);
//	{
//		device_io = device;
//	}
	void set_context_intr(I8259* device)
	{
		device_pic = device;
	}
//#ifdef I386_PSEUDO_BIOS
	void set_context_bios(DEVICE* device);
//	{
//		device_bios = device;
//	}
//#endif
//#ifdef SINGLE_MODE_DMA
	void set_context_dma(DEVICE* device);
//	{
//		device_dma = device;
//	}
//#endif
//#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		device_debugger = device;
	}
//#endif
	void set_context_extreset(DEVICE *dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_extreset, dev, id, mask);
	}

	void set_address_mask(uint32_t mask);
	uint32_t get_address_mask();
	void set_shutdown_flag(int shutdown);
	int get_shutdown_flag();
};

#endif
