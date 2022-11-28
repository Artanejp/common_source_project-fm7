/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date   : 2011.05.06-

	[ MC6809 ]
*/

#ifndef _MC6809_H_
#define _MC6809_H_

//#if defined(USE_SHARED_DLL)
//#if 0
//#include "libcpu_newdev/libcpu_mc6809/mc6809.h"
//#else
//#include "vm.h"
//#include "../emu.h"
#include "device.h"
#include "mc6809_consts.h"

enum {
	MC6809_PHASE_RUN = 0,
	MC6809_PHASE_PUSH_STACK,
	MC6809_PHASE_FETCH_VECTOR,
	MC6809_PHASE_DEAD_CYCLE,

	MC6809_PHASE_REQ_HALT,
	MC6809_PHASE_DO_HALT,
};

// Tables
/* increment */
const uint8_t flags8i[256] = {
	CC_Z, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	CC_N | CC_V, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N
};

/* decrement */
const uint8_t flags8d[256] = {
	CC_Z, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, CC_V,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N,
	CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N, CC_N
};

/* FIXME: Cycles differ slighly from hd6309 emulation */
const int index_cycle_em[256] = {	/* Index Loopup cycle counts */
	/*           0xX0, 0xX1, 0xX2, 0xX3, 0xX4, 0xX5, 0xX6, 0xX7, 0xX8, 0xX9, 0xXA, 0xXB, 0xXC, 0xXD, 0xXE, 0xXF */

	/* 0x0X */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 0x1X */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 0x2X */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 0x3X */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 0x4X */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 0x5X */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 0x6X */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 0x7X */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 0x8X */ 2, 3, 2, 3, 0, 1, 1, 1, 1, 4, 0, 4, 1, 5, 0, 2,
	/* 0x9X */ 5, 6, 5, 6, 3, 4, 4, 4, 4, 7, 3, 7, 4, 8, 3, 3,
	/* 0xAX */ 2, 3, 2, 3, 0, 1, 1, 1, 1, 4, 0, 4, 1, 5, 0, 2,
	/* 0xBX */ 5, 6, 5, 6, 3, 4, 4, 4, 4, 7, 3, 7, 4, 8, 3, 5,
	/* 0xCX */ 2, 3, 2, 3, 0, 1, 1, 1, 1, 4, 0, 4, 1, 5, 0, 2,
	/* 0xDX */ 5, 6, 5, 6, 3, 4, 4, 4, 4, 7, 3, 7, 4, 8, 3, 5,
	/* 0xEX */ 2, 3, 2, 3, 0, 1, 1, 1, 1, 4, 0, 4, 1, 5, 0, 2,
	/* 0xFX */ 4, 6, 5, 6, 3, 4, 4, 4, 4, 7, 3, 7, 4, 8, 3, 5
};

/* timings for 1-byte opcodes */
/* 20100731 Fix to XM7 */
const int cycles1[256] = {
	/*     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0 */ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 6,
	/*1 */ 0, 0, 2, 2, 0, 0, 5, 9, 3, 2, 3, 2, 3, 2, 8, 6,
	/*2 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	/*3 */ 4, 4, 4, 4, 5, 5, 5, 5, 4, 5, 3, 6, 20, 11, 1, 19,
	/*4 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	/*5 */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	/*6 */ 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 6,
	/*7 */ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 7,
	/*8 */ 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 4, 7, 3, 3,
	/*9 */ 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 7, 5, 5,
	/*A*/ 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 7, 5, 5,
	/*B*/ 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 5, 7, 8, 6, 6,
	/*C*/ 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 3, 0, 3, 3,
	/*D*/ 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	/*E*/ 4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	/*F*/ 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6
};

// Note: Below is ugly hack cause of CPU#0 cannot modify clock.
#define SIG_CPU_WAIT_FACTOR	0
//#define SIG_CPU_WAIT_FACTOR 0x8001 + SIG_CPU_BUSREQ

class VM;
class EMU;
class DEBUGGER;
class MC6809_BASE : public DEVICE
{
protected:
	// context
	DEVICE *d_mem;

	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored;
	int dasm_ptr;

	outputs_t outputs_bus_clr; // If clr() insn used, write "1" or "2".
	outputs_t outputs_bus_ba; // Bus available.
	outputs_t outputs_bus_bs; // Bus status.
	bool clr_used;

	// registers
	pair32_t pc; 	/* Program counter */
	pair32_t ppc;	/* Previous program counter */
	pair32_t acc;	/* Accumulator a and b */
	pair32_t dp;	/* Direct Page register (page in MSB) */
	pair32_t u, s;	/* Stack pointers */
	pair32_t x, y;	/* Index registers */
	uint8_t cc;
	pair32_t ea;	/* effective address */
	
	uint32_t int_state;
	/* In Motorola's datasheet, status has some valiants. 20171207 K.O */
	
	bool req_halt_on;
	bool req_halt_off;
	bool busreq;

	uint32_t waitfactor;
	uint32_t waitcount;
	uint64_t total_icount;
	uint64_t prev_total_icount;

	int icount;
	int extra_icount;
	void WM16(uint32_t Addr, pair32_t *p);
	void cpu_irq_push(void);
	void cpu_firq_push(void);
	void cpu_nmi_push(void);
	void cpu_irq_fetch_vector_address(void);
	void cpu_firq_fetch_vector_address(void);
	void cpu_nmi_fetch_vector_address(void);
	void cpu_wait(int clocks = 1);
	// opcodes
	virtual void run_one_opecode();
	void op(uint8_t ireg);
	void fetch_effective_address();
	void fetch_effective_address_IDX(uint8_t upper, uint8_t lower);
	// Useful routines.
	inline void BRANCH(bool cond);
	inline void LBRANCH(bool cond);
	
	inline pair32_t RM16_PAIR(uint32_t addr);
	inline uint8_t GET_INDEXED_DATA(void);
	inline pair32_t GET_INDEXED_DATA16(void);
	
	inline void  NEG_MEM(uint8_t a_neg);
	inline uint8_t NEG_REG(uint8_t r_neg);
	inline void  COM_MEM(uint8_t a_neg);
	inline uint8_t COM_REG(uint8_t r_neg);
	inline void  LSR_MEM(uint8_t a_neg);
	inline uint8_t LSR_REG(uint8_t r_neg);
	inline void  ROR_MEM(uint8_t a_neg);
	inline uint8_t ROR_REG(uint8_t r_neg);
	inline void  ASR_MEM(uint8_t a_neg);
	inline uint8_t ASR_REG(uint8_t r_neg);
	inline void  ASL_MEM(uint8_t a_neg);
	inline uint8_t ASL_REG(uint8_t r_neg);
	inline void  ROL_MEM(uint8_t a_neg);
	inline uint8_t ROL_REG(uint8_t r_neg);
	inline void  DEC_MEM(uint8_t a_neg);
	inline uint8_t DEC_REG(uint8_t r_neg);
	inline void  DCC_MEM(uint8_t a_neg);
	inline uint8_t DCC_REG(uint8_t r_neg);
	inline void  INC_MEM(uint8_t a_neg);
	inline uint8_t INC_REG(uint8_t r_neg);
	inline void  TST_MEM(uint8_t a_neg);
	inline uint8_t TST_REG(uint8_t r_neg);
	inline uint8_t CLC_REG(uint8_t r_neg);
	inline void  CLR_MEM(uint8_t a_neg);
	inline uint8_t CLR_REG(uint8_t r_neg);
	
	inline uint8_t SUB8_REG(uint8_t reg, uint8_t data);
	inline uint8_t CMP8_REG(uint8_t reg, uint8_t data);
	inline uint8_t SBC8_REG(uint8_t reg, uint8_t data);
	inline uint8_t AND8_REG(uint8_t reg, uint8_t data);
	inline uint8_t BIT8_REG(uint8_t reg, uint8_t data);
	inline uint8_t OR8_REG(uint8_t reg, uint8_t data);
	inline uint8_t EOR8_REG(uint8_t reg, uint8_t data);
	inline uint8_t ADD8_REG(uint8_t reg, uint8_t data);
	inline uint8_t ADC8_REG(uint8_t reg, uint8_t data);
	inline void  STORE8_REG(uint8_t reg);
	inline uint8_t LOAD8_REG(uint8_t reg);

	inline uint16_t SUB16_REG(uint16_t reg, uint16_t data);
	inline uint16_t ADD16_REG(uint16_t reg, uint16_t data);
	inline uint16_t CMP16_REG(uint16_t reg, uint16_t data);
	inline uint16_t LOAD16_REG(uint16_t reg);
	inline void STORE16_REG(pair32_t *p);
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

	bool __USE_DEBUGGER;
	uint64_t cycles_tmp_count;
	uint64_t insns_count;
	uint64_t extra_tmp_count;
	uint32_t nmi_count;
	uint32_t firq_count;
	uint32_t irq_count;
	int frames_count;

public:
	MC6809_BASE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) 
	{

		total_icount = prev_total_icount = 0;
		cycles_tmp_count = 0;
		insns_count = 0;
		__USE_DEBUGGER = false;
		initialize_output_signals(&outputs_bus_clr);
		initialize_output_signals(&outputs_bus_ba);
		initialize_output_signals(&outputs_bus_bs);
		set_device_name(_T("MC6809 MPU"));
	}
	~MC6809_BASE() {}
	
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
	void write_debug_data16(uint32_t addr, uint32_t data)
	{
		write_debug_data8(addr, (data >> 8) & 0xff);
		write_debug_data8(addr + 1, data & 0xff);
	}
	uint32_t read_debug_data16(uint32_t addr)
	{
		uint32_t val = read_debug_data8(addr) << 8;
		val |= read_debug_data8(addr + 1);
		return val;
	}
	void write_debug_data32(uint32_t addr, uint32_t data)
	{
		write_debug_data16(addr, (data >> 16) & 0xffff);
		write_debug_data16(addr + 2, data & 0xffff);
	}
	uint32_t read_debug_data32(uint32_t addr)
	{
		uint32_t val = read_debug_data16(addr) << 16;
		val |= read_debug_data16(addr + 2);
		return val;
	}
	void write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t read_debug_io8(uint32_t addr);
	void write_debug_io16(uint32_t addr, uint32_t data)
	{
		write_debug_io8(addr, (data >> 8) & 0xff);
		write_debug_io8(addr + 1, data & 0xff);
	}
	uint32_t read_debug_io16(uint32_t addr)
	{
		uint32_t val = read_debug_io8(addr) << 8;
		val |= read_debug_io8(addr + 1);
		return val;
	}
	void write_debug_io32(uint32_t addr, uint32_t data)
	{
		write_debug_io16(addr, (data >> 16) & 0xffff);
		write_debug_io16(addr + 2, data & 0xffff);
	}
	uint32_t read_debug_io32(uint32_t addr)
	{
		uint32_t val = read_debug_io16(addr) << 16;
		val |= read_debug_io16(addr + 2);
		return val;
	}
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
	virtual uint32_t cpu_disassemble_m6809(_TCHAR *buffer, uint32_t pc, const uint8_t *oprom, const uint8_t *opram);
	virtual void debugger_hook(void);
	// common functions
	void reset();
	virtual void initialize();
	int run(int clock);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
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
		return ppc.w.l;
	}
	uint32_t get_next_pc()
	{
		return pc.w.l;
	}
	// For debug
	uint32_t get_ix()
	{
		return x.w.l;
	}
	uint32_t get_iy()
	{
		return y.w.l;
	}
	uint32_t get_ustack()
	{
		return u.w.l;
	}
	uint32_t get_sstack()
	{
		return s.w.l;
	}
	uint32_t get_acca()
	{
		return acc.b.h;
	}
	uint32_t get_accb()
	{
		return acc.b.l;
	}
	uint32_t get_cc()
	{
		return cc;
	}
	uint32_t get_dp()
	{
		return dp.b.h;
	}

	// unique function
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_bus_clr(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_bus_clr, device, id, mask);
	}
	void set_context_bus_ba(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_bus_ba, device, id, mask);
	}
	void set_context_bus_bs(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_bus_bs, device, id, mask);
	}

	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
	void event_frame();
};

class MC6809 : public MC6809_BASE
{

 public:
	MC6809(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MC6809_BASE(parent_vm, parent_emu) 
	{
	}
	~MC6809() {}
	void initialize();
	void run_one_opecode();
	uint32_t cpu_disassemble_m6809(_TCHAR *buffer, uint32_t pc, const uint8_t *oprom, const uint8_t *opram);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);
	void debugger_hook(void);
};
#endif

