/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date   : 2011.05.06-

	[ MC6809 ]
*/

#ifndef _MC6809_H_
#define _MC6809_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#ifdef USE_DEBUGGER
class DEBUGGER;
#endif

class MC6809 : public DEVICE
{
private:
	// context
	DEVICE *d_mem;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored;
	int dasm_ptr;
#endif
	outputs_t outputs_bus_halt; // For sync

	outputs_t outputs_bus_clr; // If clr() insn used, write "1" or "2".
	bool clr_used;

	// registers
	pair pc; 	/* Program counter */
	pair ppc;	/* Previous program counter */
	pair acc;	/* Accumulator a and b */
	pair dp;	/* Direct Page register (page in MSB) */
	pair u, s;	/* Stack pointers */
	pair x, y;	/* Index registers */
	uint8 cc;
	pair ea;	/* effective address */
	
	uint32 int_state;
	bool busreq;
	int icount;
	int extra_icount;
	inline void WM16(uint32 Addr, pair *p);
	void cpu_irq(void);
	void cpu_firq(void);
	void cpu_nmi(void);
	
	// opcodes
	void run_one_opecode();
	void op(uint8 ireg);
	inline void fetch_effective_address();
	inline void fetch_effective_address_IDX(uint8 upper, uint8 lower);
	// Useful routines.
	inline void BRANCH(bool cond);
	inline void LBRANCH(bool cond);
	
	inline pair RM16_PAIR(uint32 addr);
	inline uint8 GET_INDEXED_DATA(void);
	inline pair GET_INDEXED_DATA16(void);
	
	inline void  NEG_MEM(uint8 a_neg);
	inline uint8 NEG_REG(uint8 r_neg);
	inline void  COM_MEM(uint8 a_neg);
	inline uint8 COM_REG(uint8 r_neg);
	inline void  LSR_MEM(uint8 a_neg);
	inline uint8 LSR_REG(uint8 r_neg);
	inline void  ROR_MEM(uint8 a_neg);
	inline uint8 ROR_REG(uint8 r_neg);
	inline void  ASR_MEM(uint8 a_neg);
	inline uint8 ASR_REG(uint8 r_neg);
	inline void  ASL_MEM(uint8 a_neg);
	inline uint8 ASL_REG(uint8 r_neg);
	inline void  ROL_MEM(uint8 a_neg);
	inline uint8 ROL_REG(uint8 r_neg);
	inline void  DEC_MEM(uint8 a_neg);
	inline uint8 DEC_REG(uint8 r_neg);
	inline void  DCC_MEM(uint8 a_neg);
	inline uint8 DCC_REG(uint8 r_neg);
	inline void  INC_MEM(uint8 a_neg);
	inline uint8 INC_REG(uint8 r_neg);
	inline void  TST_MEM(uint8 a_neg);
	inline uint8 TST_REG(uint8 r_neg);
	inline uint8 CLC_REG(uint8 r_neg);
	inline void  CLR_MEM(uint8 a_neg);
	inline uint8 CLR_REG(uint8 r_neg);
	
	inline uint8 SUB8_REG(uint8 reg, uint8 data);
	inline uint8 CMP8_REG(uint8 reg, uint8 data);
	inline uint8 SBC8_REG(uint8 reg, uint8 data);
	inline uint8 AND8_REG(uint8 reg, uint8 data);
	inline uint8 BIT8_REG(uint8 reg, uint8 data);
	inline uint8 OR8_REG(uint8 reg, uint8 data);
	inline uint8 EOR8_REG(uint8 reg, uint8 data);
	inline uint8 ADD8_REG(uint8 reg, uint8 data);
	inline uint8 ADC8_REG(uint8 reg, uint8 data);
	inline void  STORE8_REG(uint8 reg);
	inline uint8 LOAD8_REG(uint8 reg);

	inline uint16 SUB16_REG(uint16 reg, uint16 data);
	inline uint16 ADD16_REG(uint16 reg, uint16 data);
	inline uint16 CMP16_REG(uint16 reg, uint16 data);
	inline uint16 LOAD16_REG(uint16 reg);
	inline void STORE16_REG(pair *p);

 public:
	inline void abx();
	inline void adca_di();
	inline void adca_ex();
	inline void adca_im();
	inline void adca_ix();
	inline void adcb_di();
	 inline void adcb_ex();
	 inline void adcb_im();
	 inline void adcb_ix();
	 inline void adda_di();
	 inline void adda_ex();
	 inline void adda_im();
	 inline void adda_ix();
	 inline void addb_di();
	 inline void addb_ex();
	 inline void addb_im();
	 inline void addb_ix();
	 inline void addd_di();
	 inline void addd_ex();
	 inline void addd_im();
	 inline void addd_ix();
	 inline void anda_di();
	 inline void anda_ex();
	 inline void anda_im();
	 inline void anda_ix();
	 inline void andb_di();
	 inline void andb_ex();
	 inline void andb_im();
	 inline void andb_ix();
	 inline void andcc();
	 inline void asla();
	 inline void aslb();
	 inline void aslcc_in();
	 inline void asl_di();
	 inline void asl_ex();
	 inline void asl_ix();
	 inline void asra();
	 inline void asrb();
	 inline void asr_di();
	 inline void asr_ex();
	 inline void asr_ix();
	 inline void bcc();
	 inline void bcs();
	 inline void beq();
	 inline void bge();
	 inline void bgt();
	 inline void bhi();
	 inline void bita_di();
	 inline void bita_ex();
	 inline void bita_im();
	 inline void bita_ix();
	 inline void bitb_di();
	 inline void bitb_ex();
	 inline void bitb_im();
	 inline void bitb_ix();
	 inline void ble();
	 inline void bls();
	 inline void blt();
	 inline void bmi();
	 inline void bne();
	 inline void bpl();
	 inline void bra();
	 inline void brn();
	 inline void bsr();
	 inline void bvc();
	 inline void bvs();
	 inline void clca();
	 inline void clcb();
	 inline void clra();
	 inline void clrb();
	 inline void clr_di();
	 inline void clr_ex();
	 inline void clr_ix();
	 inline void cmpa_di();
	 inline void cmpa_ex();
	 inline void cmpa_im();
	 inline void cmpa_ix();
	 inline void cmpb_di();
	 inline void cmpb_ex();
	 inline void cmpb_im();
	 inline void cmpb_ix();
	 inline void cmpd_di();
	 inline void cmpd_ex();
	 inline void cmpd_im();
	 inline void cmpd_ix();
	 inline void cmps_di();
	 inline void cmps_ex();
	 inline void cmps_im();
	 inline void cmps_ix();
	 inline void cmpu_di();
	 inline void cmpu_ex();
	 inline void cmpu_im();
	 inline void cmpu_ix();
	 inline void cmpx_di();
	 inline void cmpx_ex();
	 inline void cmpx_im();
	 inline void cmpx_ix();
	 inline void cmpy_di();
	 inline void cmpy_ex();
	 inline void cmpy_im();
	 inline void cmpy_ix();
	 inline void coma();
	 inline void comb();
	 inline void com_di();
	 inline void com_ex();
	 inline void com_ix();
	 inline void cwai();
	 inline void daa();
	 inline void dcca();
	 inline void dccb();
	 inline void dcc_di();
	 inline void dcc_ex();
	 inline void dcc_ix();
	 inline void deca();
	 inline void decb();
	 inline void dec_di();
	 inline void dec_ex();
	 inline void dec_ix();
	 inline void eora_di();
	 inline void eora_ex();
	 inline void eora_im();
	 inline void eora_ix();
	 inline void eorb_di();
	 inline void eorb_ex();
	 inline void eorb_im();
	 inline void eorb_ix();
	 inline void exg();
	 inline void flag8_im();
	 inline void flag16_im();
	 inline void illegal();
	 inline void inca();
	 inline void incb();
	 inline void inc_di();
	 inline void inc_ex();
	 inline void inc_ix();
	 inline void jmp_di();
	 inline void jmp_ex();
	 inline void jmp_ix();
	 inline void jsr_di();
	 inline void jsr_ex();
	 inline void jsr_ix();
	 inline void lbcc();
	 inline void lbcs();
	 inline void lbeq();
	 inline void lbge();
	 inline void lbgt();
	 inline void lbhi();
	 inline void lble();
	 inline void lbls();
	 inline void lblt();
	 inline void lbmi();
	 inline void lbne();
	 inline void lbpl();
	 inline void lbra();
	 inline void lbrn();
	 inline void lbsr();
	 inline void lbvc();
	 inline void lbvs();
	 inline void lda_di();
	 inline void lda_ex();
	 inline void lda_im();
	 inline void lda_ix();
	 inline void ldb_di();
	 inline void ldb_ex();
	 inline void ldb_im();
	 inline void ldb_ix();
	 inline void ldd_di();
	 inline void ldd_ex();
	 inline void ldd_im();
	 inline void ldd_ix();
	 inline void lds_di();
	 inline void lds_ex();
	 inline void lds_im();
	 inline void lds_ix();
	 inline void ldu_di();
	 inline void ldu_ex();
	 inline void ldu_im();
	 inline void ldu_ix();
	 inline void ldx_di();
	 inline void ldx_ex();
	 inline void ldx_im();
	 inline void ldx_ix();
	 inline void ldy_di();
	 inline void ldy_ex();
	 inline void ldy_im();
	 inline void ldy_ix();
	 inline void leas();
	 inline void leau();
	 inline void leax();
	 inline void leay();
	 inline void lsra();
	 inline void lsrb();
	 inline void lsr_di();
	 inline void lsr_ex();
	 inline void lsr_ix();
	 inline void mul();
	 inline void nega();
	 inline void negb();
	 inline void neg_di();
	 inline void neg_ex();
	 inline void neg_ix();
	 inline void ngca();
	 inline void ngcb();
	 inline void ngc_di();
	 inline void ngc_ex();
	 inline void ngc_ix();
	 inline void nop();
	 inline void ora_di();
	 inline void ora_ex();
	 inline void ora_im();
	 inline void ora_ix();
	 inline void orb_di();
	 inline void orb_ex();
	 inline void orb_im();
	 inline void orb_ix();
	 inline void orcc();
	 inline void pref10();
	 inline void pref11();
	 inline void pshs();
	 inline void pshu();
	 inline void puls();
	 inline void pulu();
	 inline void rola();
	 inline void rolb();
	 inline void rol_di();
	 inline void rol_ex();
	 inline void rol_ix();
	 inline void rora();
	 inline void rorb();
	 inline void ror_di();
	 inline void ror_ex();
	 inline void ror_ix();
	 inline void rst();
	 inline void rti();	
	 inline void rts();	
	 inline void sbca_di();
	 inline void sbca_ex();
	 inline void sbca_im();
	 inline void sbca_ix();
	 inline void sbcb_di();
	 inline void sbcb_ex();
	 inline void sbcb_im();
	 inline void sbcb_ix();
	 inline void sex();
	 inline void sta_di();
	 inline void sta_ex();
	 inline void sta_im();
	 inline void sta_ix();
	 inline void stb_di();
	 inline void stb_ex();
	 inline void stb_im();
	 inline void stb_ix();
	 inline void std_di();
	 inline void std_ex();
	 inline void std_im();
	 inline void std_ix();
	 inline void sts_di();
	 inline void sts_ex();
	 inline void sts_im();
	 inline void sts_ix();
	 inline void stu_di();
	 inline void stu_ex();
	 inline void stu_im();
	 inline void stu_ix();
	 inline void stx_di();
	 inline void stx_ex();
	 inline void stx_im();
	 inline void stx_ix();
	 inline void sty_di();
	 inline void sty_ex();
	 inline void sty_im();
	 inline void sty_ix();
	 inline void suba_di();
	 inline void suba_ex();
	 inline void suba_im();
	 inline void suba_ix();
	 inline void subb_di();
	 inline void subb_ex();
	 inline void subb_im();
	 inline void subb_ix();
	 inline void subd_di();
	 inline void subd_ex();
	 inline void subd_im();
	 inline void subd_ix();
	 inline void swi2();
	 inline void swi3();
	 inline void swi();
	 inline void sync_09();
	 inline void tfr();
	 inline void trap();
	 inline void tsta();
	 inline void tstb();
	 inline void tst_di();
	 inline void tst_ex();
	 inline void tst_ix();

	
public:
	MC6809(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) 
	{
		init_output_signals(&outputs_bus_clr);
		init_output_signals(&outputs_bus_halt);
	}
	~MC6809() {}
	
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
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32 pc, _TCHAR *buffer, size_t buffer_len);
#endif
	// common functions
	void reset();
	void initialize();
	int run(int clock);
	void write_signal(int id, uint32 data, uint32 mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
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
		return ppc.w.l;
	}
	uint32 get_next_pc()
	{
		return pc.w.l;
	}
	// For debug
	uint32 get_ix()
	{
		return x.w.l;
	}
	uint32 get_iy()
	{
		return y.w.l;
	}
	uint32 get_ustack()
	{
		return u.w.l;
	}
	uint32 get_sstack()
	{
		return s.w.l;
	}
	uint32 get_acca()
	{
		return acc.b.h;
	}
	uint32 get_accb()
	{
		return acc.b.l;
	}
	uint32 get_cc()
	{
		return cc;
	}
	uint32 get_dp()
	{
		return dp.b.h;
	}

	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_bus_halt(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_bus_halt, device, id, mask);
	}
	void set_context_bus_clr(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_bus_clr, device, id, mask);
	}
#ifdef USE_DEBUGGER
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};

#endif

