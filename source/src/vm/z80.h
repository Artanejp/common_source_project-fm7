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
#ifdef Z80_PSEUDO_BIOS
	DEVICE *d_bios;
#endif
#ifdef SINGLE_MODE_DMA
	DEVICE *d_dma;
#endif
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored, *d_io_stored;
#endif
	outputs_t outputs_busack;
	
	bool is_primary;
	
	/* ---------------------------------------------------------------------------
	registers
	--------------------------------------------------------------------------- */
	
#ifdef USE_DEBUGGER
	uint64_t total_icount;
	uint64_t prev_total_icount;
#endif
	int icount;
	int dma_icount;
	int wait_icount, event_icount, in_op_icount;
	uint16_t prevpc;
	pair32_t pc, sp, af, bc, de, hl, ix, iy, wz;
	pair32_t af2, bc2, de2, hl2;
	uint8_t I, R, R2;
	uint32_t ea;
	
	bool busreq, wait, after_halt;
	uint8_t im, iff1, iff2, icr;
	bool after_di, after_ei, prev_after_ei;
	bool after_ldair;
	uint32_t intr_req_bit, intr_pend_bit;
	bool intr_enb;
	
	inline uint8_t RM8(uint32_t addr);
	inline void WM8(uint32_t addr, uint8_t val);
	inline void RM16(uint32_t addr, pair32_t *r);
	inline void WM16(uint32_t addr, pair32_t *r);
	inline uint8_t FETCHOP();
	inline uint8_t FETCH8();
	inline uint32_t FETCH16();
	inline uint8_t IN8(uint32_t addr);
	inline void OUT8(uint32_t addr, uint8_t val);
	
	inline uint8_t INC(uint8_t value);
	inline uint8_t DEC(uint8_t value);
	
	inline uint8_t RLC(uint8_t value);
	inline uint8_t RRC(uint8_t value);
	inline uint8_t RL(uint8_t value);
	inline uint8_t RR(uint8_t value);
	inline uint8_t SLA(uint8_t value);
	inline uint8_t SRA(uint8_t value);
	inline uint8_t SLL(uint8_t value);
	inline uint8_t SRL(uint8_t value);
	
	inline uint8_t RES(uint8_t bit, uint8_t value);
	inline uint8_t SET(uint8_t bit, uint8_t value);
	
	void OP_CB(uint8_t code);
	void OP_XY(uint8_t code);
	void OP_DD(uint8_t code);
	void OP_FD(uint8_t code);
	void OP_ED(uint8_t code);
	void OP(uint8_t code);
	void run_one_opecode();
	void check_interrupt();
	
	/* ---------------------------------------------------------------------------
	debug
	--------------------------------------------------------------------------- */
	
public:
	Z80(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef USE_DEBUGGER
		total_icount = prev_total_icount = 0;
#endif
		busreq = wait = false;
#ifdef Z80_PSEUDO_BIOS
		d_bios = NULL;
#endif
#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
#endif
		initialize_output_signals(&outputs_busack);
		is_primary = false;
		set_device_name(_T("Z80 CPU"));
	}
	~Z80() {}
	
	// common functions
	void initialize();
	void reset();
	void special_reset();
	int run(int clock);
	void write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t read_signal(int id);
	void set_intr_line(bool line, bool pending, uint32_t bit)
	{
		uint32_t mask = 1 << bit;
		intr_req_bit = line ? (intr_req_bit | mask) : (intr_req_bit & ~mask);
		intr_pend_bit = pending ? (intr_pend_bit | mask) : (intr_pend_bit & ~mask);
	}
	void set_extra_clock(int clock)
	{
		dma_icount += clock;
	}
	int get_extra_clock()
	{
		return dma_icount;
	}
	uint32_t get_pc()
	{
		return prevpc;
	}
	uint32_t get_next_pc()
	{
		return pc.w.l;
	}
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
		return 0xffff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xffff;
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
		d_pic = device;
	}
	DEVICE *get_context_child()
	{
		return d_pic;
	}
#ifdef Z80_PSEUDO_BIOS
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
	void set_context_busack(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_busack, device, id, mask);
	}
	void set_pc(uint16_t value)
	{
		pc.w.l = value;
	}
	void set_sp(uint16_t value)
	{
		sp.w.l = value;
	}
	void set_intr_enb(bool value)
	{
		intr_enb = value;
	}
};

#endif

