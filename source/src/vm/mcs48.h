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
	uint8 ram[0x100];
public:
	MCS48MEM(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(ram, 0, sizeof(ram));
	}
	~MCS48MEM() {}
	
	uint32 read_data8(uint32 addr)
	{
		return ram[addr & 0xff];
	}
	void write_data8(uint32 addr, uint32 data)
	{
		ram[addr & 0xff] = data;
	}
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
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
	
public:
	MCS48(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_mem = d_io = d_intr = NULL;
	}
	~MCS48() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	int run(int icount);
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 get_pc();
	uint32 get_next_pc();
#ifdef USE_DEBUGGER
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32 debug_prog_addr_mask()
	{
		return 0xfff;
	}
	uint32 debug_data_addr_mask()
	{
		return 0xff;
	}
	void debug_write_data8(uint32 addr, uint32 data);
	uint32 debug_read_data8(uint32 addr);
	void debug_write_io8(uint32 addr, uint32 data);
	uint32 debug_read_io8(uint32 addr);
	bool debug_write_reg(_TCHAR *reg, uint32 data);
	void debug_regs_info(_TCHAR *buffer);
	int debug_dasm(uint32 pc, _TCHAR *buffer);
#endif
	void save_state(FILEIO* state_state_fio);
	bool load_state(FILEIO* state_state_fio);
	
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
	void load_rom_image(_TCHAR *file_path);
	uint8 *get_rom_ptr();
};

#endif

