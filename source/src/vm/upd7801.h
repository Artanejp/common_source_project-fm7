/*
	Skelton for retropc emulator

	Origin : MESS UPD7810 Core
	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ uPD7801 ]
*/

#ifndef _UPD7801_H_
#define _UPD7801_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_UPD7801_INTF0	0
#define SIG_UPD7801_INTF1	1
#define SIG_UPD7801_INTF2	2
#define SIG_UPD7801_WAIT	3

// virtual i/o port address
#define P_A	0
#define P_B	1
#define P_C	2
#define P_SI	3
#define P_SO	4

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class UPD7801 : public DEVICE
{
private:
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */
	
	DEVICE *d_mem, *d_io;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored, *d_io_stored;
#endif
	
	/* ---------------------------------------------------------------------------
	registers
	--------------------------------------------------------------------------- */
	
	int count, period, scount, tcount;
	bool wait;
	
	pair regs[8];
	uint16 SP, PC, prevPC;
	uint8 PSW, IRR, IFF, SIRQ, HALT, MK, MB, MC, TM0, TM1, SR;
	// for port c
	uint8 SAK, TO, PORTC;
	
	/* ---------------------------------------------------------------------------
	virtual machine interface
	--------------------------------------------------------------------------- */
	
	// memory
	inline uint8 RM8(uint16 addr);
	inline void WM8(uint16 addr, uint8 val);
	inline uint16 RM16(uint16 addr);
	inline void WM16(uint16 addr, uint16 val);
	inline uint8 FETCH8();
	inline uint16 FETCH16();
	inline uint16 FETCHWA();
	inline uint8 POP8();
	inline void PUSH8(uint8 val);
	inline uint16 POP16();
	inline void PUSH16(uint16 val);
	
	// i/o
	inline uint8 IN8(int port);
	inline void OUT8(int port, uint8 val);
	inline void UPDATE_PORTC(uint8 IOM);
	
	/* ---------------------------------------------------------------------------
	opecode
	--------------------------------------------------------------------------- */
	
	void run_one_opecode();
#ifdef USE_DEBUGGER
	void run_one_opecode_debugger();
#endif
	void OP();
	void OP48();
	void OP4C();
	void OP4D();
	void OP60();
	void OP64();
	void OP70();
	void OP74();
	
public:
	UPD7801(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~UPD7801() {}
	
	// common function
	void initialize();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 get_pc()
	{
		return prevPC;
	}
	uint32 get_next_pc()
	{
		return PC;
	}
#ifdef USE_DEBUGGER
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32 debug_prog_addr_mask()
	{
		return 0xffff;
	}
	uint32 debug_data_addr_mask()
	{
		return 0xffff;
	}
	void debug_write_data8(uint32 addr, uint32 data);
	uint32 debug_read_data8(uint32 addr);
	void debug_write_io8(uint32 addr, uint32 data);
	uint32 debug_read_io8(uint32 addr);
	bool debug_write_reg(_TCHAR *reg, uint32 data);
	void debug_regs_info(_TCHAR *buffer);
	int debug_dasm(uint32 pc, _TCHAR *buffer);
#endif
	
	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif
