/*
	Skelton for retropc emulator

	Origin : MAME 0.148
	Author : Takeda.Toshiya
	Date   : 2013.05.01-

	[ MCS48 ]
*/

#ifndef _MCS84_H_ 
#define _MCS48_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define MCS48_PORT_P0	0x100	/* Not used */
#define MCS48_PORT_P1	0x101	/* P10-P17 */
#define MCS48_PORT_P2	0x102	/* P20-P28 */
#define MCS48_PORT_T0	0x110
#define MCS48_PORT_T1	0x111
#define MCS48_PORT_BUS	0x120	/* DB0-DB7 */
#define MCS48_PORT_PROG	0x121	/* PROG line to 8243 expander */

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class MCS48MEM : public DEVICE
{
private:
	uint8_t ram[0x100];
public:
	MCS48MEM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(ram, 0, sizeof(ram));
		set_device_name(_T("MCS48 Memory Bus"));
	}
	~MCS48MEM() {}
	
	uint32_t read_data8(uint32_t addr)
	{
		return ram[addr & 0xff];
	}
	void write_data8(uint32_t addr, uint32_t data)
	{
		ram[addr & 0xff] = data;
	}
	bool process_state(FILEIO* state_fio, bool loading);
};

class MCS48 : public DEVICE
{
private:
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */
	
	DEVICE *d_mem, *d_io, *d_intr;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored, *d_io_stored;
#endif
	void *opaque;
	
	/* ---------------------------------------------------------------------------
	registers
	--------------------------------------------------------------------------- */
	
#ifdef USE_DEBUGGER
	uint64_t total_icount;
	uint64_t prev_total_icount;
#endif
	
public:
	MCS48(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_mem = d_io = d_intr = NULL;
#ifdef USE_DEBUGGER
		total_icount = prev_total_icount = 0;
#endif
		set_device_name(_T("MCS48 MCU"));
	}
	~MCS48() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int run(int icount);
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
		return 0xfff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xff;
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
	
	// unique functions
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
		d_intr = device;
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
	void load_rom_image(const _TCHAR *file_path);
	uint8_t *get_rom_ptr();
};

#endif

