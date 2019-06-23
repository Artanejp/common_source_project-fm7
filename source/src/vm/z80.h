/*
	Skelton for retropc emulator

	Origin : MAME 0.145
	Author : Takeda.Toshiya
	Date   : 2012.02.15-

	[ Z80 ]
*/

#ifndef _Z80_H_ 
#define _Z80_H_

#include "device.h"

//#ifdef HAS_NSC800
#define SIG_NSC800_INT	0
#define SIG_NSC800_RSTA	1
#define SIG_NSC800_RSTB	2
#define SIG_NSC800_RSTC	3
//#endif
//#if defined(USE_SHARED_DLL) || defined(USE_QT)
#define Z80_INLINE
//#else
//#define Z80_INLINE inline
//#endif
//#ifdef USE_DEBUGGER
class DEBUGGER;
//#endif
class Z80_BASE : public DEVICE
{
protected:
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */
	
	DEVICE *d_mem, *d_io, *d_pic;
//#ifdef Z80_PSEUDO_BIOS
	DEVICE *d_bios;
//#endif
//#ifdef SINGLE_MODE_DMA
	DEVICE *d_dma;
//#endif
//#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored, *d_io_stored;
//#endif
	outputs_t outputs_busack;

	bool has_nsc800;
	bool has_memory_wait;
	bool has_io_wait;
	bool has_pseudo_bios;
	bool has_ldair_quirk;
	bool has_single_mode_dma;
	bool flags_initialized;
	
	bool is_primary;
	
	/* ---------------------------------------------------------------------------
	registers
	--------------------------------------------------------------------------- */
	
	uint64_t total_icount;
	uint64_t prev_total_icount;
	int icount;
	int extra_icount;
	int busreq_icount;
	uint16_t prevpc;
	pair32_t pc, sp, af, bc, de, hl, ix, iy, wz;
	pair32_t af2, bc2, de2, hl2;
	uint8_t I, R, R2;
	uint32_t ea;
	
	bool busreq, after_halt;
	uint8_t im, iff1, iff2, icr;
	bool after_ei, after_ldair;
	uint32_t intr_req_bit, intr_pend_bit;
	
	Z80_INLINE uint8_t __FASTCALL RM8(uint32_t addr);
	Z80_INLINE void __FASTCALL WM8(uint32_t addr, uint8_t val);
	Z80_INLINE void __FASTCALL RM16(uint32_t addr, pair32_t *r);
	Z80_INLINE void __FASTCALL WM16(uint32_t addr, pair32_t *r);
	Z80_INLINE uint8_t __FASTCALL FETCHOP();
	Z80_INLINE uint8_t __FASTCALL FETCH8();
	Z80_INLINE uint32_t __FASTCALL FETCH16();
	Z80_INLINE uint8_t __FASTCALL IN8(uint32_t addr);
	Z80_INLINE void __FASTCALL OUT8(uint32_t addr, uint8_t val);
	
	Z80_INLINE uint8_t __FASTCALL INC(uint8_t value);
	Z80_INLINE uint8_t __FASTCALL DEC(uint8_t value);
	
	Z80_INLINE uint8_t __FASTCALL RLC(uint8_t value);
	Z80_INLINE uint8_t __FASTCALL RRC(uint8_t value);
	Z80_INLINE uint8_t __FASTCALL RL(uint8_t value);
	Z80_INLINE uint8_t __FASTCALL RR(uint8_t value);
	Z80_INLINE uint8_t __FASTCALL SLA(uint8_t value);
	Z80_INLINE uint8_t __FASTCALL SRA(uint8_t value);
	Z80_INLINE uint8_t __FASTCALL SLL(uint8_t value);
	Z80_INLINE uint8_t __FASTCALL SRL(uint8_t value);
	
	Z80_INLINE uint8_t __FASTCALL RES(uint8_t bit, uint8_t value);
	Z80_INLINE uint8_t __FASTCALL SET(uint8_t bit, uint8_t value);
	
	void OP_CB(uint8_t code);
	void OP_XY(uint8_t code);
	void OP_DD(uint8_t code);
	void OP_FD(uint8_t code);
	void OP_ED(uint8_t code);
	void OP(uint8_t code);
	virtual void __FASTCALL run_one_opecode();
	virtual void __FASTCALL debugger_hook(void);
	
	uint8_t SZ[256];		/* zero and sign flags */
	uint8_t SZ_BIT[256];	/* zero, sign and parity/overflow (=zero) flags for BIT opcode */
	uint8_t SZP[256];		/* zero, sign and parity flags */
	uint8_t SZHV_inc[256];	/* zero, sign, half carry and overflow flags INC r8 */
	uint8_t SZHV_dec[256];	/* zero, sign, half carry and overflow flags DEC r8 */

	uint8_t SZHVC_add[2 * 256 * 256];
	uint8_t SZHVC_sub[2 * 256 * 256];

	const uint8_t cc_op[0x100] = {
		4,10, 7, 6, 4, 4, 7, 4, 4,11, 7, 6, 4, 4, 7, 4,
		8,10, 7, 6, 4, 4, 7, 4,12,11, 7, 6, 4, 4, 7, 4,
		7,10,16, 6, 4, 4, 7, 4, 7,11,16, 6, 4, 4, 7, 4,
		7,10,13, 6,11,11,10, 4, 7,11,13, 6, 4, 4, 7, 4,
		4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
		4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
		4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
		7, 7, 7, 7, 7, 7, 4, 7, 4, 4, 4, 4, 4, 4, 7, 4,
		4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
		4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
		4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
		4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
		5,10,10,10,10,11, 7,11, 5,10,10, 0,10,17, 7,11,
		5,10,10,11,10,11, 7,11, 5, 4,10,11,10, 0, 7,11,
		5,10,10,19,10,11, 7,11, 5, 4,10, 4,10, 0, 7,11,
		5,10,10, 4,10,11, 7,11, 5, 6,10, 4,10, 0, 7,11
	};

	 const uint8_t cc_cb[0x100] = {
		8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
		8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
		8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
		8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
		8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
		8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
		8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
		8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
		8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
		8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
		8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
		8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
		8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
		8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
		8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
		8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8
	};
	
	 const uint8_t cc_ed[0x100] = {
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		12,12,15,20, 8,14, 8, 9,12,12,15,20, 8,14, 8, 9,
		12,12,15,20, 8,14, 8, 9,12,12,15,20, 8,14, 8, 9,
		12,12,15,20, 8,14, 8,18,12,12,15,20, 8,14, 8,18,
		12,12,15,20, 8,14, 8, 8,12,12,15,20, 8,14, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,
		16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
	};
	
	 const uint8_t cc_xy[0x100] = {
		4, 4, 4, 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4,
		4,14,20,10, 9, 9,11, 4, 4,15,20,10, 9, 9,11, 4,
		4, 4, 4, 4,23,23,19, 4, 4,15, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
		4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
		9, 9, 9, 9, 9, 9,19, 9, 9, 9, 9, 9, 9, 9,19, 9,
		19,19,19,19,19,19, 4,19, 4, 4, 4, 4, 9, 9,19, 4,
		4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
		4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
		4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
		4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		4,14, 4,23, 4,15, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4,10, 4, 4, 4, 4, 4, 4
	};

	 const uint8_t cc_xycb[0x100] = {
		23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
		23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
		23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
		23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
		20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
		20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
		20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
		20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
		23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
		23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
		23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
		23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
		23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
		23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
		23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
		23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23
	};
	
	const uint8_t cc_ex[0x100] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* DJNZ */
		5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,	/* JR NZ/JR Z */
		5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,	/* JR NC/JR C */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0,	/* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
		6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
		6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
		6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
		6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2
	};

	/* ---------------------------------------------------------------------------
	debug
	--------------------------------------------------------------------------- */
	// Collect counters
	uint64_t cycles_tmp_count;
	uint64_t extra_tmp_count;
	uint32_t insns_count;
	int frames_count;
	int nmi_count;
	int	irq_count;
	int	nsc800_int_count;
	int	nsc800_rsta_count;
	int	nsc800_rstb_count;
	int	nsc800_rstc_count;

public:
	Z80_BASE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		flags_initialized = false;
		busreq = false;
//#ifdef Z80_PSEUDO_BIOS
		d_bios = NULL;
//#endif
//#ifdef SINGLE_MODE_DMA
		d_dma = NULL;
//#endif
		d_debugger = NULL;
		d_mem_stored = NULL;
		d_io_stored = NULL;
		d_pic = NULL;
		has_nsc800 = false;
		has_io_wait = false;
		has_memory_wait = false;
		has_pseudo_bios = false;
		has_ldair_quirk = false;
		has_single_mode_dma = false;
		total_icount = prev_total_icount = 0;
		initialize_output_signals(&outputs_busack);
		is_primary = false;
		set_device_name(_T("Z80 CPU"));

	}
	~Z80_BASE() {}
	
	// common functions
	virtual void initialize();
	virtual void reset();
	void event_frame();
	int run(int clock);

	virtual bool process_state(FILEIO* state_fio, bool loading);
	
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t __FASTCALL read_signal(int id);
	void set_intr_line(bool line, bool pending, uint32_t bit)
	{
		uint32_t mask = 1 << bit;
		intr_req_bit = line ? (intr_req_bit | mask) : (intr_req_bit & ~mask);
		intr_pend_bit = pending ? (intr_pend_bit | mask) : (intr_pend_bit & ~mask);
		if(line) irq_count++;
	}
	void set_extra_clock(int clock)
	{
		extra_icount += clock;
	}
	int get_extra_clock()
	{
		return extra_icount;
	}
	uint32_t get_pc()
	{
		return prevpc;
	}
	uint32_t get_next_pc()
	{
		return pc.w.l;
	}
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
		return 0xffff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xffff;
	}
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
//#endif
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	DEVICE *get_context_child()
	{
		return d_pic;
	}
	void set_context_intr(DEVICE* device)
	{
		d_pic = device;
	}
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
};

class Z80 : public Z80_BASE 
{
protected:
	void check_interrupt();
	void __FASTCALL run_one_opecode() override;
	void __FASTCALL debugger_hook(void) override;
public:
	Z80(VM_TEMPLATE* parent_vm, EMU* parent_emu);
	~Z80();
	void initialize();
	void reset();
	int run(int clock) override;

	int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
#ifdef USE_DEBUGGER
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_io8(uint32_t addr);
#endif
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
};


#endif

