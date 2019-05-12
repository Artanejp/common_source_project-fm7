/*
	Skelton for retropc emulator

	Origin : MAME i286 core
	Author : Takeda.Toshiya
	Date  : 2012.10.18-

	[ i286 ]
*/

#ifndef _I286_H_ 
#define _I286_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_I86_TEST	0
#define SIG_I286_A20	1

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class I286 : public DEVICE
{
private:
	DEVICE *d_mem, *d_io, *d_pic;
#ifdef I86_PSEUDO_BIOS
	DEVICE *d_bios;
#endif
#ifdef SINGLE_MODE_DMA
	DEVICE *d_dma;
#endif
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif
	void *opaque;
	
public:
	I286(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef I86_PSEUDO_BIOS
		d_bios = NULL;
#endif
#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
#endif
#ifdef USE_DEBUGGER
		d_debugger = NULL;;
#endif
#if defined(HAS_I86)
		set_device_name(_T("8086 CPU"));
#elif defined(HAS_I88)
		set_device_name(_T("8088 CPU"));
#elif defined(HAS_I186)
		set_device_name(_T("80186 CPU"));
#elif defined(HAS_V30)
		set_device_name(_T("V30 CPU"));
#elif defined(HAS_I286)
		set_device_name(_T("80286 CPU"));
#endif
	}
	~I286() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int run(int icount);
	uint32_t read_signal(int id);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void set_intr_line(bool line, bool pending, uint32_t bit);
	void set_extra_clock(int icount);
	int get_extra_clock();
	uint32_t get_pc();
	uint32_t get_next_pc();
	uint32_t translate_address(int segment, uint32_t offset);


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
#ifdef HAS_I286
		return 0xffffff;
#else
		return 0xfffff;
#endif
	}
	uint32_t get_debug_data_addr_mask()
	{
#ifdef HAS_I286
		return 0xffffff;
#else
		return 0xfffff;
#endif
	}
	void write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t read_debug_data8(uint32_t addr);
	void write_debug_data16(uint32_t addr, uint32_t data);
	uint32_t read_debug_data16(uint32_t addr);
	void write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t read_debug_io8(uint32_t addr);
	void write_debug_io16(uint32_t addr, uint32_t data);
	uint32_t read_debug_io16(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	uint32_t read_debug_reg(const _TCHAR *reg);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
#endif
	void save_state_cpustate(FILEIO* state_fio);
	void load_state_cpustate(FILEIO* state_fio);
	
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
	void set_context_intr(DEVICE* device)
	{
		d_pic = device;
	}
#ifdef I86_PSEUDO_BIOS
	void set_context_bios(DEVICE* device)
	{
		d_bios = device;
	}
#endif
#ifdef SINGLE_MODE_DMA
	void set_context_dma(DEVICE* device)
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
#ifdef HAS_I286
	void set_address_mask(uint32_t mask);
	uint32_t get_address_mask();
	void set_shutdown_flag(int shutdown);
	int get_shutdown_flag();
#endif
};

#endif
