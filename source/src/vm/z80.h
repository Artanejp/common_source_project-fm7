/*
	Skelton for retropc emulator

	Origin : MAME 0.145
	Author : Takeda.Toshiya
	Date   : 2012.02.15-

	[ Z80 ]
*/

#ifndef _Z80_H_ 
#define _Z80_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#ifdef HAS_NSC800
#define SIG_NSC800_INT	0
#define SIG_NSC800_RSTA	1
#define SIG_NSC800_RSTB	2
#define SIG_NSC800_RSTC	3
#endif

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class Z80 : public DEVICE
{
private:
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */
	
	DEVICE *d_mem, *d_io, *d_pic;
#ifdef SINGLE_MODE_DMA
	DEVICE *d_dma;
#endif
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored, *d_io_stored;
#endif
	outputs_t outputs_busack;
	
	/* ---------------------------------------------------------------------------
	registers
	--------------------------------------------------------------------------- */
	
	int icount;
	int extra_icount;
	uint16 prevpc;
	pair pc, sp, af, bc, de, hl, ix, iy, wz;
	pair af2, bc2, de2, hl2;
	uint8 I, R, R2;
	uint32 ea;
	
	bool busreq, halt;
	uint8 im, iff1, iff2, icr;
	bool after_ei, after_ldair;
	uint32 intr_req_bit, intr_pend_bit;
	
	inline uint8 RM8(uint32 addr);
	inline void WM8(uint32 addr, uint8 val);
	inline void RM16(uint32 addr, pair *r);
	inline void WM16(uint32 addr, pair *r);
	inline uint8 FETCHOP();
	inline uint8 FETCH8();
	inline uint32 FETCH16();
	inline uint8 IN8(uint32 addr);
	inline void OUT8(uint32 addr, uint8 val);
	
	inline uint8 INC(uint8 value);
	inline uint8 DEC(uint8 value);
	
	inline uint8 RLC(uint8 value);
	inline uint8 RRC(uint8 value);
	inline uint8 RL(uint8 value);
	inline uint8 RR(uint8 value);
	inline uint8 SLA(uint8 value);
	inline uint8 SRA(uint8 value);
	inline uint8 SLL(uint8 value);
	inline uint8 SRL(uint8 value);
	
	inline uint8 RES(uint8 bit, uint8 value);
	inline uint8 SET(uint8 bit, uint8 value);
	
	void OP_CB(uint8 code);
	void OP_XY(uint8 code);
	void OP_DD(uint8 code);
	void OP_FD(uint8 code);
	void OP_ED(uint8 code);
	void OP(uint8 code);
	void run_one_opecode();
	
	/* ---------------------------------------------------------------------------
	debug
	--------------------------------------------------------------------------- */
	
public:
	Z80(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		busreq = false;
#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
#endif
		init_output_signals(&outputs_busack);
	}
	~Z80() {}
	
	// common functions
	void initialize();
	void reset();
	int run(int clock);
	void write_signal(int id, uint32 data, uint32 mask);
	void set_intr_line(bool line, bool pending, uint32 bit)
	{
		uint32 mask = 1 << bit;
		intr_req_bit = line ? (intr_req_bit | mask) : (intr_req_bit & ~mask);
		intr_pend_bit = pending ? (intr_pend_bit | mask) : (intr_pend_bit & ~mask);
	}
	void set_extra_clock(int clock)
	{
		extra_icount += clock;
	}
	int get_extra_clock()
	{
		return extra_icount;
	}
	uint32 get_pc()
	{
		return prevpc;
	}
	uint32 get_next_pc()
	{
		return pc.w.l;
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
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
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
		d_pic = device;
	}
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
	void set_context_busack(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_busack, device, id, mask);
	}
	void set_pc(uint16 value)
	{
		pc.w.l = value;
	}
	void set_sp(uint16 value)
	{
		sp.w.l = value;
	}
};

#endif

