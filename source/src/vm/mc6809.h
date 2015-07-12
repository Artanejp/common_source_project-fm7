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
	void WM16(uint32 Addr, pair *p);
	void cpu_irq(void);
	void cpu_firq(void);
	void cpu_nmi(void);
	
	// opcodes
	void run_one_opecode();
	void op(uint8 ireg);
	void fetch_effective_address();
	void fetch_effective_address_IDX(uint8 upper, uint8 lower);
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
	void abx();
	void adca_di();
	void adca_ex();
	inline void adca_im();
	void adca_ix();
	void adcb_di();
	 void adcb_ex();
	 void adcb_im();
	 void adcb_ix();
	 void adda_di();
	 void adda_ex();
	 void adda_im();
	 void adda_ix();
	 void addb_di();
	 void addb_ex();
	 void addb_im();
	 void addb_ix();
	 void addd_di();
	 void addd_ex();
	 void addd_im();
	 void addd_ix();
	 void anda_di();
	 void anda_ex();
	 void anda_im();
	 void anda_ix();
	 void andb_di();
	 void andb_ex();
	 void andb_im();
	 void andb_ix();
	 void andcc();
	 void asla();
	 void aslb();
	 void aslcc_in();
	 void asl_di();
	 void asl_ex();
	 void asl_ix();
	 void asra();
	 void asrb();
	 void asr_di();
	 void asr_ex();
	 void asr_ix();
	 void bcc();
	 void bcs();
	 void beq();
	 void bge();
	 void bgt();
	 void bhi();
	 void bita_di();
	 void bita_ex();
	 void bita_im();
	 void bita_ix();
	 void bitb_di();
	 void bitb_ex();
	 void bitb_im();
	 void bitb_ix();
	 void ble();
	 void bls();
	 void blt();
	 void bmi();
	 void bne();
	 void bpl();
	 void bra();
	 void brn();
	 void bsr();
	 void bvc();
	 void bvs();
	 void clca();
	 void clcb();
	 void clra();
	 void clrb();
	 void clr_di();
	 void clr_ex();
	 void clr_ix();
	 void cmpa_di();
	 void cmpa_ex();
	 void cmpa_im();
	 void cmpa_ix();
	 void cmpb_di();
	 void cmpb_ex();
	 void cmpb_im();
	 void cmpb_ix();
	 void cmpd_di();
	 void cmpd_ex();
	 void cmpd_im();
	 void cmpd_ix();
	 void cmps_di();
	 void cmps_ex();
	 void cmps_im();
	 void cmps_ix();
	 void cmpu_di();
	 void cmpu_ex();
	 void cmpu_im();
	 void cmpu_ix();
	 void cmpx_di();
	 void cmpx_ex();
	 void cmpx_im();
	 void cmpx_ix();
	 void cmpy_di();
	 void cmpy_ex();
	 void cmpy_im();
	 void cmpy_ix();
	 void coma();
	 void comb();
	 void com_di();
	 void com_ex();
	 void com_ix();
	 void cwai();
	 void daa();
	 void dcca();
	 void dccb();
	 void dcc_di();
	 void dcc_ex();
	 void dcc_ix();
	 void deca();
	 void decb();
	 void dec_di();
	 void dec_ex();
	 void dec_ix();
	 void eora_di();
	 void eora_ex();
	 void eora_im();
	 void eora_ix();
	 void eorb_di();
	 void eorb_ex();
	 void eorb_im();
	 void eorb_ix();
	 void exg();
	 void flag8_im();
	 void flag16_im();
	 void illegal();
	 void inca();
	 void incb();
	 void inc_di();
	 void inc_ex();
	 void inc_ix();
	 void jmp_di();
	 void jmp_ex();
	 void jmp_ix();
	 void jsr_di();
	 void jsr_ex();
	 void jsr_ix();
	 void lbcc();
	 void lbcs();
	 void lbeq();
	 void lbge();
	 void lbgt();
	 void lbhi();
	 void lble();
	 void lbls();
	 void lblt();
	 void lbmi();
	 void lbne();
	 void lbpl();
	 void lbra();
	 void lbrn();
	 void lbsr();
	 void lbvc();
	 void lbvs();
	 void lda_di();
	 void lda_ex();
	 void lda_im();
	 void lda_ix();
	 void ldb_di();
	 void ldb_ex();
	 void ldb_im();
	 void ldb_ix();
	 void ldd_di();
	 void ldd_ex();
	 void ldd_im();
	 void ldd_ix();
	 void lds_di();
	 void lds_ex();
	 void lds_im();
	 void lds_ix();
	 void ldu_di();
	 void ldu_ex();
	 void ldu_im();
	 void ldu_ix();
	 void ldx_di();
	 void ldx_ex();
	 void ldx_im();
	 void ldx_ix();
	 void ldy_di();
	 void ldy_ex();
	 void ldy_im();
	 void ldy_ix();
	 void leas();
	 void leau();
	 void leax();
	 void leay();
	 void lsra();
	 void lsrb();
	 void lsr_di();
	 void lsr_ex();
	 void lsr_ix();
	 void mul();
	 void nega();
	 void negb();
	 void neg_di();
	 void neg_ex();
	 void neg_ix();
	 void ngca();
	 void ngcb();
	 void ngc_di();
	 void ngc_ex();
	 void ngc_ix();
	 void nop();
	 void ora_di();
	 void ora_ex();
	 void ora_im();
	 void ora_ix();
	 void orb_di();
	 void orb_ex();
	 void orb_im();
	 void orb_ix();
	 void orcc();
	 void pref10();
	 void pref11();
	 void pshs();
	 void pshu();
	 void puls();
	 void pulu();
	 void rola();
	 void rolb();
	 void rol_di();
	 void rol_ex();
	 void rol_ix();
	 void rora();
	 void rorb();
	 void ror_di();
	 void ror_ex();
	 void ror_ix();
	 void rst();
	 void rti();	
	 void rts();	
	 void sbca_di();
	 void sbca_ex();
	 void sbca_im();
	 void sbca_ix();
	 void sbcb_di();
	 void sbcb_ex();
	 void sbcb_im();
	 void sbcb_ix();
	 void sex();
	 void sta_di();
	 void sta_ex();
	 void sta_im();
	 void sta_ix();
	 void stb_di();
	 void stb_ex();
	 void stb_im();
	 void stb_ix();
	 void std_di();
	 void std_ex();
	 void std_im();
	 void std_ix();
	 void sts_di();
	 void sts_ex();
	 void sts_im();
	 void sts_ix();
	 void stu_di();
	 void stu_ex();
	 void stu_im();
	 void stu_ix();
	 void stx_di();
	 void stx_ex();
	 void stx_im();
	 void stx_ix();
	 void sty_di();
	 void sty_ex();
	 void sty_im();
	 void sty_ix();
	 void suba_di();
	 void suba_ex();
	 void suba_im();
	 void suba_ix();
	 void subb_di();
	 void subb_ex();
	 void subb_im();
	 void subb_ix();
	 void subd_di();
	 void subd_ex();
	 void subd_im();
	 void subd_ix();
	 void swi2();
	 void swi3();
	 void swi();
	 void sync_09();
	 void tfr();
	 void trap();
	 void tsta();
	 void tstb();
	 void tst_di();
	 void tst_ex();
	 void tst_ix();

	
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
	uint32 cpu_disassemble_m6809(_TCHAR *buffer, uint32 pc, const uint8 *oprom, const uint8 *opram);
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

