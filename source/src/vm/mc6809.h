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


// Note: Below is ugly hack cause of CPU#0 cannot modify clock.
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

	outputs_t outputs_bus_ba; // Bus available.
	outputs_t outputs_bus_bs; // Bus status.

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
	void __FASTCALL WM16(uint32_t Addr, pair32_t *p);
	void __FASTCALL cpu_irq_push(void);
	void __FASTCALL cpu_firq_push(void);
	void __FASTCALL cpu_nmi_push(void);
	void __FASTCALL cpu_irq_fetch_vector_address(void);
	void __FASTCALL cpu_firq_fetch_vector_address(void);
	void __FASTCALL cpu_nmi_fetch_vector_address(void);
	void __FASTCALL cpu_wait(int clocks = 1);
	// Tables
/* increment */
	const uint8_t flags8i[256] = {
		CC_Z,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		CC_N|CC_V,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N
	};
	
/* decrement */
	const uint8_t flags8d[256] = {
		CC_Z,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,CC_V,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
		CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N
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
	// opcodes
	virtual void __FASTCALL run_one_opecode();
	void __FASTCALL op(uint8_t ireg);
	void __FASTCALL fetch_effective_address();
	void __FASTCALL fetch_effective_address_IDX(uint8_t upper, uint8_t lower);
	// Useful routines.
	inline void __FASTCALL BRANCH(bool cond);
	inline void __FASTCALL LBRANCH(bool cond);
	
	inline pair32_t __FASTCALL RM16_PAIR(uint32_t addr);
	inline uint8_t __FASTCALL GET_INDEXED_DATA(void);
	inline pair32_t __FASTCALL GET_INDEXED_DATA16(void);
	
	inline void __FASTCALL  NEG_MEM(uint8_t a_neg);
	inline uint8_t __FASTCALL NEG_REG(uint8_t r_neg);
	inline void __FASTCALL  COM_MEM(uint8_t a_neg);
	inline uint8_t __FASTCALL COM_REG(uint8_t r_neg);
	inline void __FASTCALL  LSR_MEM(uint8_t a_neg);
	inline uint8_t __FASTCALL LSR_REG(uint8_t r_neg);
	inline void __FASTCALL  ROR_MEM(uint8_t a_neg);
	inline uint8_t __FASTCALL ROR_REG(uint8_t r_neg);
	inline void __FASTCALL  ASR_MEM(uint8_t a_neg);
	inline uint8_t __FASTCALL ASR_REG(uint8_t r_neg);
	inline void __FASTCALL  ASL_MEM(uint8_t a_neg);
	inline uint8_t __FASTCALL ASL_REG(uint8_t r_neg);
	inline void __FASTCALL  ROL_MEM(uint8_t a_neg);
	inline uint8_t __FASTCALL ROL_REG(uint8_t r_neg);
	inline void __FASTCALL  DEC_MEM(uint8_t a_neg);
	inline uint8_t __FASTCALL DEC_REG(uint8_t r_neg);
	inline void __FASTCALL  DCC_MEM(uint8_t a_neg);
	inline uint8_t __FASTCALL DCC_REG(uint8_t r_neg);
	inline void __FASTCALL  INC_MEM(uint8_t a_neg);
	inline uint8_t __FASTCALL INC_REG(uint8_t r_neg);
	inline void __FASTCALL  TST_MEM(uint8_t a_neg);
	inline uint8_t __FASTCALL TST_REG(uint8_t r_neg);
	inline uint8_t __FASTCALL CLC_REG(uint8_t r_neg);
	inline void __FASTCALL  CLR_MEM(uint8_t a_neg);
	inline uint8_t __FASTCALL CLR_REG(uint8_t r_neg);
	
	inline uint8_t __FASTCALL SUB8_REG(uint8_t reg, uint8_t data);
	inline uint8_t __FASTCALL CMP8_REG(uint8_t reg, uint8_t data);
	inline uint8_t __FASTCALL SBC8_REG(uint8_t reg, uint8_t data);
	inline uint8_t __FASTCALL AND8_REG(uint8_t reg, uint8_t data);
	inline uint8_t __FASTCALL BIT8_REG(uint8_t reg, uint8_t data);
	inline uint8_t __FASTCALL OR8_REG(uint8_t reg, uint8_t data);
	inline uint8_t __FASTCALL EOR8_REG(uint8_t reg, uint8_t data);
	inline uint8_t __FASTCALL ADD8_REG(uint8_t reg, uint8_t data);
	inline uint8_t __FASTCALL ADC8_REG(uint8_t reg, uint8_t data);
	inline void __FASTCALL  STORE8_REG(uint8_t reg);
	inline uint8_t __FASTCALL LOAD8_REG(uint8_t reg);

	inline uint16_t SUB16_REG(uint16_t reg, uint16_t data);
	inline uint16_t ADD16_REG(uint16_t reg, uint16_t data);
	inline uint16_t CMP16_REG(uint16_t reg, uint16_t data);
	inline uint16_t LOAD16_REG(uint16_t reg);
	inline void __FASTCALL STORE16_REG(pair32_t *p);
 public:
	void __FASTCALL abx();
	void __FASTCALL adca_di();
	void __FASTCALL adca_ex();
	inline void __FASTCALL adca_im();
	void __FASTCALL adca_ix();
	void __FASTCALL adcb_di();
	void __FASTCALL adcb_ex();
	void __FASTCALL adcb_im();
	void __FASTCALL adcb_ix();
	void __FASTCALL adda_di();
	void __FASTCALL adda_ex();
	void __FASTCALL adda_im();
	void __FASTCALL adda_ix();
	void __FASTCALL addb_di();
	void __FASTCALL addb_ex();
	void __FASTCALL addb_im();
	void __FASTCALL addb_ix();
	void __FASTCALL addd_di();
	void __FASTCALL addd_ex();
	void __FASTCALL addd_im();
	void __FASTCALL addd_ix();
	void __FASTCALL anda_di();
	void __FASTCALL anda_ex();
	void __FASTCALL anda_im();
	void __FASTCALL anda_ix();
	void __FASTCALL andb_di();
	void __FASTCALL andb_ex();
	void __FASTCALL andb_im();
	void __FASTCALL andb_ix();
	void __FASTCALL andcc();
	void __FASTCALL asla();
	void __FASTCALL aslb();
	void __FASTCALL aslcc_in();
	void __FASTCALL asl_di();
	void __FASTCALL asl_ex();
	void __FASTCALL asl_ix();
	void __FASTCALL asra();
	void __FASTCALL asrb();
	void __FASTCALL asr_di();
	void __FASTCALL asr_ex();
	void __FASTCALL asr_ix();
	void __FASTCALL bcc();
	void __FASTCALL bcs();
	void __FASTCALL beq();
	void __FASTCALL bge();
	void __FASTCALL bgt();
	void __FASTCALL bhi();
	void __FASTCALL bita_di();
	void __FASTCALL bita_ex();
	void __FASTCALL bita_im();
	void __FASTCALL bita_ix();
	void __FASTCALL bitb_di();
	void __FASTCALL bitb_ex();
	void __FASTCALL bitb_im();
	void __FASTCALL bitb_ix();
	void __FASTCALL ble();
	void __FASTCALL bls();
	void __FASTCALL blt();
	void __FASTCALL bmi();
	void __FASTCALL bne();
	void __FASTCALL bpl();
	void __FASTCALL bra();
	void __FASTCALL brn();
	void __FASTCALL bsr();
	void __FASTCALL bvc();
	void __FASTCALL bvs();
	void __FASTCALL clca();
	void __FASTCALL clcb();
	void __FASTCALL clra();
	void __FASTCALL clrb();
	void __FASTCALL clr_di();
	void __FASTCALL clr_ex();
	void __FASTCALL clr_ix();
	void __FASTCALL cmpa_di();
	void __FASTCALL cmpa_ex();
	void __FASTCALL cmpa_im();
	void __FASTCALL cmpa_ix();
	void __FASTCALL cmpb_di();
	void __FASTCALL cmpb_ex();
	void __FASTCALL cmpb_im();
	void __FASTCALL cmpb_ix();
	void __FASTCALL cmpd_di();
	void __FASTCALL cmpd_ex();
	void __FASTCALL cmpd_im();
	void __FASTCALL cmpd_ix();
	void __FASTCALL cmps_di();
	void __FASTCALL cmps_ex();
	void __FASTCALL cmps_im();
	void __FASTCALL cmps_ix();
	void __FASTCALL cmpu_di();
	void __FASTCALL cmpu_ex();
	void __FASTCALL cmpu_im();
	void __FASTCALL cmpu_ix();
	void __FASTCALL cmpx_di();
	void __FASTCALL cmpx_ex();
	void __FASTCALL cmpx_im();
	void __FASTCALL cmpx_ix();
	void __FASTCALL cmpy_di();
	void __FASTCALL cmpy_ex();
	void __FASTCALL cmpy_im();
	void __FASTCALL cmpy_ix();
	void __FASTCALL coma();
	void __FASTCALL comb();
	void __FASTCALL com_di();
	void __FASTCALL com_ex();
	void __FASTCALL com_ix();
	void __FASTCALL cwai();
	void __FASTCALL daa();
	void __FASTCALL dcca();
	void __FASTCALL dccb();
	void __FASTCALL dcc_di();
	void __FASTCALL dcc_ex();
	void __FASTCALL dcc_ix();
	void __FASTCALL deca();
	void __FASTCALL decb();
	void __FASTCALL dec_di();
	void __FASTCALL dec_ex();
	void __FASTCALL dec_ix();
	void __FASTCALL eora_di();
	void __FASTCALL eora_ex();
	void __FASTCALL eora_im();
	void __FASTCALL eora_ix();
	void __FASTCALL eorb_di();
	void __FASTCALL eorb_ex();
	void __FASTCALL eorb_im();
	void __FASTCALL eorb_ix();
	void __FASTCALL exg();
	void __FASTCALL flag8_im();
	void __FASTCALL flag16_im();
	void __FASTCALL illegal();
	void __FASTCALL inca();
	void __FASTCALL incb();
	void __FASTCALL inc_di();
	void __FASTCALL inc_ex();
	void __FASTCALL inc_ix();
	void __FASTCALL jmp_di();
	void __FASTCALL jmp_ex();
	void __FASTCALL jmp_ix();
	void __FASTCALL jsr_di();
	void __FASTCALL jsr_ex();
	void __FASTCALL jsr_ix();
	void __FASTCALL lbcc();
	void __FASTCALL lbcs();
	void __FASTCALL lbeq();
	void __FASTCALL lbge();
	void __FASTCALL lbgt();
	void __FASTCALL lbhi();
	void __FASTCALL lble();
	void __FASTCALL lbls();
	void __FASTCALL lblt();
	void __FASTCALL lbmi();
	void __FASTCALL lbne();
	void __FASTCALL lbpl();
	void __FASTCALL lbra();
	void __FASTCALL lbrn();
	void __FASTCALL lbsr();
	void __FASTCALL lbvc();
	void __FASTCALL lbvs();
	void __FASTCALL lda_di();
	void __FASTCALL lda_ex();
	void __FASTCALL lda_im();
	void __FASTCALL lda_ix();
	void __FASTCALL ldb_di();
	void __FASTCALL ldb_ex();
	void __FASTCALL ldb_im();
	void __FASTCALL ldb_ix();
	void __FASTCALL ldd_di();
	void __FASTCALL ldd_ex();
	void __FASTCALL ldd_im();
	void __FASTCALL ldd_ix();
	void __FASTCALL lds_di();
	void __FASTCALL lds_ex();
	void __FASTCALL lds_im();
	void __FASTCALL lds_ix();
	void __FASTCALL ldu_di();
	void __FASTCALL ldu_ex();
	void __FASTCALL ldu_im();
	void __FASTCALL ldu_ix();
	void __FASTCALL ldx_di();
	void __FASTCALL ldx_ex();
	void __FASTCALL ldx_im();
	void __FASTCALL ldx_ix();
	void __FASTCALL ldy_di();
	void __FASTCALL ldy_ex();
	void __FASTCALL ldy_im();
	void __FASTCALL ldy_ix();
	void __FASTCALL leas();
	void __FASTCALL leau();
	void __FASTCALL leax();
	void __FASTCALL leay();
	void __FASTCALL lsra();
	void __FASTCALL lsrb();
	void __FASTCALL lsr_di();
	void __FASTCALL lsr_ex();
	void __FASTCALL lsr_ix();
	void __FASTCALL mul();
	void __FASTCALL nega();
	void __FASTCALL negb();
	void __FASTCALL neg_di();
	void __FASTCALL neg_ex();
	void __FASTCALL neg_ix();
	void __FASTCALL ngca();
	void __FASTCALL ngcb();
	void __FASTCALL ngc_di();
	void __FASTCALL ngc_ex();
	void __FASTCALL ngc_ix();
	void __FASTCALL nop();
	void __FASTCALL ora_di();
	void __FASTCALL ora_ex();
	void __FASTCALL ora_im();
	void __FASTCALL ora_ix();
	void __FASTCALL orb_di();
	void __FASTCALL orb_ex();
	void __FASTCALL orb_im();
	void __FASTCALL orb_ix();
	void __FASTCALL orcc();
	void __FASTCALL pref10();
	void __FASTCALL pref11();
	void __FASTCALL pshs();
	void __FASTCALL pshu();
	void __FASTCALL puls();
	void __FASTCALL pulu();
	void __FASTCALL rola();
	void __FASTCALL rolb();
	void __FASTCALL rol_di();
	void __FASTCALL rol_ex();
	void __FASTCALL rol_ix();
	void __FASTCALL rora();
	void __FASTCALL rorb();
	void __FASTCALL ror_di();
	void __FASTCALL ror_ex();
	void __FASTCALL ror_ix();
	void __FASTCALL rst();
	void __FASTCALL rti();	
	void __FASTCALL rts();	
	void __FASTCALL sbca_di();
	void __FASTCALL sbca_ex();
	void __FASTCALL sbca_im();
	void __FASTCALL sbca_ix();
	void __FASTCALL sbcb_di();
	void __FASTCALL sbcb_ex();
	void __FASTCALL sbcb_im();
	void __FASTCALL sbcb_ix();
	void __FASTCALL sex();
	void __FASTCALL sta_di();
	void __FASTCALL sta_ex();
	void __FASTCALL sta_im();
	void __FASTCALL sta_ix();
	void __FASTCALL stb_di();
	void __FASTCALL stb_ex();
	void __FASTCALL stb_im();
	void __FASTCALL stb_ix();
	void __FASTCALL std_di();
	void __FASTCALL std_ex();
	void __FASTCALL std_im();
	void __FASTCALL std_ix();
	void __FASTCALL sts_di();
	void __FASTCALL sts_ex();
	void __FASTCALL sts_im();
	void __FASTCALL sts_ix();
	void __FASTCALL stu_di();
	void __FASTCALL stu_ex();
	void __FASTCALL stu_im();
	void __FASTCALL stu_ix();
	void __FASTCALL stx_di();
	void __FASTCALL stx_ex();
	void __FASTCALL stx_im();
	void __FASTCALL stx_ix();
	void __FASTCALL sty_di();
	void __FASTCALL sty_ex();
	void __FASTCALL sty_im();
	void __FASTCALL sty_ix();
	void __FASTCALL suba_di();
	void __FASTCALL suba_ex();
	void __FASTCALL suba_im();
	void __FASTCALL suba_ix();
	void __FASTCALL subb_di();
	void __FASTCALL subb_ex();
	void __FASTCALL subb_im();
	void __FASTCALL subb_ix();
	void __FASTCALL subd_di();
	void __FASTCALL subd_ex();
	void __FASTCALL subd_im();
	void __FASTCALL subd_ix();
	void __FASTCALL swi2();
	void __FASTCALL swi3();
	void __FASTCALL swi();
	void __FASTCALL sync_09();
	void __FASTCALL tfr();
	void __FASTCALL trap();
	void __FASTCALL tsta();
	void __FASTCALL tstb();
	void __FASTCALL tst_di();
	void __FASTCALL tst_ex();
	void __FASTCALL tst_ix();

	bool __USE_DEBUGGER;
	uint64_t cycles_tmp_count;
	uint32_t insns_count;
	uint32_t extra_tmp_count;
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
		d_debugger = NULL;
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
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	void __FASTCALL write_debug_data16(uint32_t addr, uint32_t data)
	{
		write_debug_data8(addr, (data >> 8) & 0xff);
		write_debug_data8(addr + 1, data & 0xff);
	}
	uint32_t __FASTCALL read_debug_data16(uint32_t addr)
	{
		uint32_t val = read_debug_data8(addr) << 8;
		val |= read_debug_data8(addr + 1);
		return val;
	}
	void __FASTCALL write_debug_data32(uint32_t addr, uint32_t data)
	{
		write_debug_data16(addr, (data >> 16) & 0xffff);
		write_debug_data16(addr + 2, data & 0xffff);
	}
	uint32_t __FASTCALL read_debug_data32(uint32_t addr)
	{
		uint32_t val = read_debug_data16(addr) << 16;
		val |= read_debug_data16(addr + 2);
		return val;
	}
	void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_io8(uint32_t addr);
	void __FASTCALL write_debug_io16(uint32_t addr, uint32_t data)
	{
		write_debug_io8(addr, (data >> 8) & 0xff);
		write_debug_io8(addr + 1, data & 0xff);
	}
	uint32_t __FASTCALL read_debug_io16(uint32_t addr)
	{
		uint32_t val = read_debug_io8(addr) << 8;
		val |= read_debug_io8(addr + 1);
		return val;
	}
	void __FASTCALL write_debug_io32(uint32_t addr, uint32_t data)
	{
		write_debug_io16(addr, (data >> 16) & 0xffff);
		write_debug_io16(addr + 2, data & 0xffff);
	}
	uint32_t __FASTCALL read_debug_io32(uint32_t addr)
	{
		uint32_t val = read_debug_io16(addr) << 16;
		val |= read_debug_io16(addr + 2);
		return val;
	}
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	virtual int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
	virtual uint32_t cpu_disassemble_m6809(_TCHAR *buffer, uint32_t pc, const uint8_t *oprom, const uint8_t *opram);
	virtual void __FASTCALL debugger_hook(void);
	// common functions
	void reset();
	virtual void initialize();
	int __FASTCALL run(int clock);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
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
	void __FASTCALL run_one_opecode();
	uint32_t cpu_disassemble_m6809(_TCHAR *buffer, uint32_t pc, const uint8_t *oprom, const uint8_t *opram);
	virtual int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
	void __FASTCALL debugger_hook(void);
};
#endif

