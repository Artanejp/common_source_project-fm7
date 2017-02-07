/*
	Skelton for retropc emulator

	Origin : MAME i386 core
	Author : Takeda.Toshiya
	Date  : 2009.06.08-

	[ i386/i486/Pentium/MediaGX ]
*/

#ifndef _NEWDEV_I386_H_ 
#define _NEWDEV_I386_H_

#include "vm.h"
#include "../../emu.h"
#include "./i386_base.h"
//#include "./libcpu_i386/i386_real.h"

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif
 
class I386 : public I386_BASE
{
protected:
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif

public:
	I386(VM* parent_vm, EMU* parent_emu) : I386_BASE(parent_vm, parent_emu)
	{
#ifdef USE_DEBUGGER
		d_debugger = NULL;
#endif
#if defined(HAS_I386)
		set_device_name(_T("i80386 CPU"));
#elif defined(HAS_I486)
		set_device_name(_T("i80486 CPU"));
#elif defined(HAS_PENTIUM)
		set_device_name(_T("Pentium CPU"));
#elif defined(HAS_MEDIAGX)
		set_device_name(_T("Media GX CPU"));
#elif defined(HAS_PENTIUM_PRO)
		set_device_name(_T("Pentium Pro CPU"));
#elif defined(HAS_PENTIUM_MMX)
		set_device_name(_T("Pentium MMX CPU"));
#elif defined(HAS_PENTIUM2)
		set_device_name(_T("Pentium2 CPU"));
#elif defined(HAS_PENTIUM3)
		set_device_name(_T("Pentium3 CPU"));
#elif defined(HAS_PENTIUM4)
		set_device_name(_T("Pentium4 CPU"));
#endif
	}
	~I386() {}
	// common functions
	void initialize();
	void reset();
	int run(int cycles);
	//int cpu_execute(void *p, int cycles);	
#ifdef USE_DEBUGGER
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
	void write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t read_debug_data8(uint32_t addr);
	void write_debug_data16(uint32_t addr, uint32_t data);
	uint32_t read_debug_data16(uint32_t addr);
	void write_debug_data32(uint32_t addr, uint32_t data);
	uint32_t read_debug_data32(uint32_t addr);
	void write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t read_debug_io8(uint32_t addr);
	void write_debug_io16(uint32_t addr, uint32_t data);
	uint32_t read_debug_io32(uint32_t addr);
	void write_debug_io32(uint32_t addr, uint32_t data);
	uint32_t read_debug_io16(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	void get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
#endif

	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_bios(DEVICE* device);
	void set_context_dma(DEVICE* device);

#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device);
#endif

};

#endif
