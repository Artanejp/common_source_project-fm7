/*
	Skelton for retropc emulator

	Origin : MAME i386 core
	Author : Takeda.Toshiya
	Date   : 2009.06.08-

	[ i386/i486/Pentium/MediaGX ]
*/

#ifndef _I386_H_
#define _I386_H_
//#if defined(USE_SHARED_DLL)
//#if 0
//#include "libcpu_newdev/i386.h"
//#else
//#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I386_A20			1
#define SIG_I386_NOTIFY_RESET	2

#define I386_TRACE_DATA_BIT_USERDATA_SET	0x80000000
#define I386_TRACE_DATA_BIT_OP32			0x00000001
//#ifdef USE_DEBUGGER
class DEBUGGER;
//#endif

class  DLL_PREFIX I386 : public DEVICE
{
protected:
	DEVICE *d_mem, *d_io, *d_pic;
//#ifdef I86_PSEUDO_BIOS
	DEVICE *d_bios;
//#endif
//#ifdef SINGLE_MODE_DMA
	DEVICE *d_dma;
//#endif
//#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
//#endif
	void *opaque;
	outputs_t outputs_extreset;
	void get_debug_sregs_info(_TCHAR *buffer, size_t buffer_len);
	void get_debug_gregs_info(_TCHAR *buffer, size_t buffer_len);
public:
	I386(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
//#ifdef I86_PSEUDO_BIOS
		d_bios = NULL;
//#endif
//#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
//#endif
//#if defined(HAS_I386)
//		set_device_name(_T("i80386 CPU"));
//#elif defined(HAS_I486)
//		set_device_name(_T("i80486 CPU"));
//#elif defined(HAS_PENTIUM)
//		set_device_name(_T("Pentium CPU"));
//#elif defined(HAS_MEDIAGX)
//		set_device_name(_T("Media GX CPU"));
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
		initialize_output_signals(&outputs_extreset);
		set_device_name(_T("Intel i80x86 CPU"));
	}
	~I386() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int __FASTCALL run(int cycles);
	uint32_t __FASTCALL read_signal(int id);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	void __FASTCALL set_intr_line(bool line, bool pending, uint32_t bit);
	void __FASTCALL set_extra_clock(int cycles);
	int get_extra_clock();
	uint32_t get_pc();
	uint32_t get_next_pc();

	uint32_t __FASTCALL translate_address(int segment, uint32_t offset);
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
		return d_debugger;
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
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	uint32_t __FASTCALL read_debug_reg(const _TCHAR *reg);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
//#endif
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_extreset(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_extreset, device, id, mask);
	}
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_context_intr(DEVICE* device, uint32_t bit = 0xffffffff)
	{
		d_pic = device;
	}
//#ifdef I86_PSEUDO_BIOS
	void set_context_bios(DEVICE* device)
	{
		d_bios = device;
	}
//#endif
//#ifdef SINGLE_MODE_DMA
	void set_context_dma(DEVICE* device)
	{
		d_dma = device;
	}
//#endif
//#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
//#endif
	void set_address_mask(uint32_t mask);
	uint32_t get_address_mask();
	void set_shutdown_flag(int shutdown);
	int get_shutdown_flag();
};
//#endif // USE_SHARED_DLL
#endif
