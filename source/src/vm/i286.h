/*
	Skelton for retropc emulator

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
#define SIG_I86_A20	1

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class I286 : public DEVICE
{
private:
	DEVICE *d_mem, *d_io, *d_pic;
#ifdef I86_BIOS_CALL
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
	I286(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef I86_BIOS_CALL
		d_bios = NULL;
#endif
#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
#endif
	}
	~I286() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int run(int icount);
	void write_signal(int id, uint32 data, uint32 mask);
	void set_intr_line(bool line, bool pending, uint32 bit);
	void set_extra_clock(int icount);
	int get_extra_clock();
	uint32 get_pc();
	uint32 get_next_pc();
#ifdef USE_DEBUGGER
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32 debug_prog_addr_mask()
	{
#ifdef HAS_I286
		return 0xffffff;
#else
		return 0xfffff;
#endif
	}
	uint32 debug_data_addr_mask()
	{
#ifdef HAS_I286
		return 0xffffff;
#else
		return 0xfffff;
#endif
	}
	void debug_write_data8(uint32 addr, uint32 data);
	uint32 debug_read_data8(uint32 addr);
	void debug_write_data16(uint32 addr, uint32 data);
	uint32 debug_read_data16(uint32 addr);
	void debug_write_io8(uint32 addr, uint32 data);
	uint32 debug_read_io8(uint32 addr);
	void debug_write_io16(uint32 addr, uint32 data);
	uint32 debug_read_io16(uint32 addr);
	bool debug_write_reg(_TCHAR *reg, uint32 data);
	void debug_regs_info(_TCHAR *buffer);
	int debug_dasm(uint32 pc, _TCHAR *buffer);
#endif
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
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
#ifdef I86_BIOS_CALL
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
	void set_address_mask(uint32 mask);
	uint32 get_address_mask();
	void set_shutdown_flag(int shutdown);
	int get_shutdown_flag();
#endif
};

#endif
