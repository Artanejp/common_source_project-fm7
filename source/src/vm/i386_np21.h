/*
	Skelton for retropc emulator

	Origin : np21/w i386c core
	Author : Takeda.Toshiya
	Date   : 2020.01.25-

	[ i386/i486/Pentium ]
*/

#ifndef _I386_NP21_H_
#define _I386_NP21_H_

#include "vm_template.h"
//#include "../emu.h"
#include "device.h"

#define SIG_I386_A20	        1
#define SIG_I386_NOTIFY_RESET	2

#define I386_TRACE_DATA_BIT_USERDATA_SET	0x80000000
#define I386_TRACE_DATA_BIT_OP32			0x00000001
#define I386_TRACE_DATA_BIT_RET				0x00000040
#define I386_TRACE_DATA_BIT_RETF			0x00000050
#define I386_TRACE_DATA_BIT_IRET			0x00000060
#define I386_TRACE_DATA_BIT_JMP				0x00000080
#define I386_TRACE_DATA_BIT_JMP_COND		0x00000090
#define I386_TRACE_DATA_BIT_CALL			0x00000100
#define I386_TRACE_DATA_BIT_INT				0x10000000
#define I386_TRACE_DATA_BIT_IRQ				0x20000000
#define I386_TRACE_DATA_BIT_EXCEPTION		0x40000000

enum {
	DEFAULT = -1,
	INTEL_80386 = 0,
	INTEL_I486SX,
	INTEL_I486DX,
	INTEL_PENTIUM,
	INTEL_MMX_PENTIUM,
	INTEL_PENTIUM_PRO,
	INTEL_PENTIUM_II,
	INTEL_PENTIUM_III,
	INTEL_PENTIUM_M,
	INTEL_PENTIUM_4,
	AMD_K6_2,
	AMD_K6_III,
	AMD_K7_ATHLON,
	AMD_K7_ATHLON_XP,
};

//#ifdef USE_DEBUGGER
class DEBUGGER;
//#endif
class I386 : public DEVICE
{
private:
	DEVICE *device_pic;
	outputs_t outputs_extreset;
	
//#ifdef USE_DEBUGGER
//	DEBUGGER *device_debugger;
	DEVICE *device_mem_stored;
	DEVICE *device_io_stored;
	uint64_t total_cycles;
	uint64_t prev_total_cycles;
//#endif
	int remained_cycles, extra_cycles;
	bool busreq;
	bool nmi_pending, irq_pending;
	uint32_t PREV_CS_BASE;
	uint32_t waitfactor;
	int64_t waitcount;
	
	bool _USE_DEBUGGER;
	bool _I86_PSEUDO_BIOS;
	bool _SINGLE_MODE_DMA;
	uint32_t address_mask;
	
	int run_one_opecode();
	uint32_t __FASTCALL convert_address(uint32_t cs, uint32_t eip);
	void __FASTCALL cpu_wait(int clocks);

public:
	I386(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
//#ifdef USE_DEBUGGER
		total_cycles = prev_total_cycles = 0;
//#endif
		busreq = false;
		initialize_output_signals(&outputs_extreset);
		_USE_DEBUGGER = false;
		_I86_PSEUDO_BIOS = false;
		_SINGLE_MODE_DMA = false;
		address_mask = 0x000fffff; // OK?
		device_model = DEFAULT;
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
	void *get_debugger();
//	{
//		return device_debugger;
//	}
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
	virtual bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	uint32_t __FASTCALL read_debug_reg(const _TCHAR *reg);
	virtual bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual bool get_debug_regs_description(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
	virtual bool debug_rewind_call_trace(uint32_t pc, int &size, _TCHAR* buffer, size_t buffer_len, uint64_t userdata = 0);
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
	void set_context_intr(DEVICE* device);
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
	void set_context_debugger(DEBUGGER* device);
//#endif
	void set_context_extreset(DEVICE *dev, int id, uint32_t mask)
	{
		register_output_signal(&outputs_extreset, dev, id, mask);
	}

	void set_address_mask(uint32_t mask);
	uint32_t get_address_mask();
	void set_shutdown_flag(int shutdown);
	int get_shutdown_flag();
	int device_model;
};

#endif
