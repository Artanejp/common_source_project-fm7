/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date   : 2011.05.06-

	[ MC6809 ]
        Notes from K.Ohta <whatisthis.sowhat _at_ gmail.com> at Jan 16, 2015:
              All of undocumented instructions (i.e. ngc, flag16) of MC6809(not HD6309) are written by me.
              These behaviors of undocumented insns are refered from "vm/cpu_x86.asm" (ia32 assembly codefor nasm) within XM7
              written by Ryu Takegami , and older article wrote in magazine, "I/O" at 1985.
              But, these C implements are written from scratch by me , and I tested many years at XM7/SDL.
              Perhaps, these insns. are not implement MAME/MESS yet.
*/

// Fixed IRQ/FIRQ by Mr.Sasaji at 2011.06.17

#include "mc6809.h"
#include "mc6809_consts.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#define OP_HANDLER(_name) void MC6809::_name (void)

/* macros for branch instructions */
inline void MC6809::BRANCH(bool cond)
{
	uint8 t;
	IMMBYTE(t);
	if(!cond) return;
	PC = PC + SIGNED(t);
	PC = PC & 0xffff;
}

inline void MC6809::LBRANCH(bool cond)
{
	pair t;
	IMMWORD(t);
	if(!cond) return;
	icount -= 1;
	PC += t.w.l;
	PC = PC & 0xffff;
}

/* macros for setting/getting registers in TFR/EXG instructions */

inline pair MC6809::RM16_PAIR(uint32 addr)
{
	pair b;
	b.d = 0;
	b.b.h = RM(addr);
	b.b.l = RM(addr + 1);
	return b;
}

inline void MC6809::WM16(uint32 Addr, pair *p)
{
	WM(Addr , p->b.h);
	WM(Addr + 1, p->b.l);
}

/* increment */
static const uint8 flags8i[256] = {
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
static const uint8 flags8d[256] = {
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
static const int index_cycle_em[256] = {	/* Index Loopup cycle counts */
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
static const int cycles1[] = {
/*   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
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


static void (MC6809::*m6809_main[0x100]) (void) = {
/*          0xX0,   0xX1,     0xX2,    0xX3,    0xX4,    0xX5,    0xX6,    0xX7,
            0xX8,   0xX9,     0xXA,    0xXB,    0xXC,    0xXD,    0xXE,    0xXF   */

/* 0x0X */
	&MC6809::neg_di, &MC6809::neg_di, &MC6809::ngc_di, &MC6809::com_di,
	&MC6809::lsr_di, &MC6809::lsr_di, &MC6809::ror_di, &MC6809::asr_di,
	&MC6809::asl_di, &MC6809::rol_di, &MC6809::dec_di, &MC6809::dcc_di,
	&MC6809::inc_di, &MC6809::tst_di, &MC6809::jmp_di, &MC6809::clr_di,
/* 0x1X */
	&MC6809::pref10, &MC6809::pref11, &MC6809::nop, &MC6809::sync_09,
	&MC6809::trap, &MC6809::trap, &MC6809::lbra, &MC6809::lbsr,
	&MC6809::aslcc_in, &MC6809::daa, &MC6809::orcc, &MC6809::nop,
	&MC6809::andcc, &MC6809::sex, &MC6809::exg, &MC6809::tfr,
/* 0x2X */
	&MC6809::bra, &MC6809::brn, &MC6809::bhi, &MC6809::bls,
	&MC6809::bcc, &MC6809::bcs, &MC6809::bne, &MC6809::beq,
	&MC6809::bvc, &MC6809::bvs, &MC6809::bpl, &MC6809::bmi,
	&MC6809::bge, &MC6809::blt, &MC6809::bgt, &MC6809::ble,
/* 0x3X */
	&MC6809::leax, &MC6809::leay, &MC6809::leas, &MC6809::leau,
	&MC6809::pshs, &MC6809::puls, &MC6809::pshu, &MC6809::pulu,
	&MC6809::andcc, &MC6809::rts, &MC6809::abx, &MC6809::rti,
	&MC6809::cwai, &MC6809::mul, &MC6809::rst, &MC6809::swi,
/* 0x4X */
	&MC6809::nega, &MC6809::nega, &MC6809::ngca, &MC6809::coma,
	&MC6809::lsra, &MC6809::lsra, &MC6809::rora, &MC6809::asra,
	&MC6809::asla, &MC6809::rola, &MC6809::deca, &MC6809::dcca,
	&MC6809::inca, &MC6809::tsta, &MC6809::clca, &MC6809::clra,
/* 0x5X */
	&MC6809::negb, &MC6809::negb, &MC6809::ngcb, &MC6809::comb,
	&MC6809::lsrb, &MC6809::lsrb, &MC6809::rorb, &MC6809::asrb,
	&MC6809::aslb, &MC6809::rolb, &MC6809::decb, &MC6809::dccb,
	&MC6809::incb, &MC6809::tstb, &MC6809::clcb, &MC6809::clrb,
/* 0x6X */
	&MC6809::neg_ix, &MC6809::neg_ix, &MC6809::ngc_ix, &MC6809::com_ix,
	&MC6809::lsr_ix, &MC6809::lsr_ix, &MC6809::ror_ix, &MC6809::asr_ix,
	&MC6809::asl_ix, &MC6809::rol_ix, &MC6809::dec_ix, &MC6809::dcc_ix,
	&MC6809::inc_ix, &MC6809::tst_ix, &MC6809::jmp_ix, &MC6809::clr_ix,
/* 0x7X */
	&MC6809::neg_ex, &MC6809::neg_ex, &MC6809::ngc_ex, &MC6809::com_ex,
	&MC6809::lsr_ex, &MC6809::lsr_ex, &MC6809::ror_ex, &MC6809::asr_ex,
	&MC6809::asl_ex, &MC6809::rol_ex, &MC6809::dec_ex, &MC6809::dcc_ex,
	&MC6809::inc_ex, &MC6809::tst_ex, &MC6809::jmp_ex, &MC6809::clr_ex,
/* 0x8X */
	&MC6809::suba_im, &MC6809::cmpa_im, &MC6809::sbca_im, &MC6809::subd_im,
	&MC6809::anda_im, &MC6809::bita_im, &MC6809::lda_im, &MC6809::flag8_im,
	&MC6809::eora_im, &MC6809::adca_im, &MC6809::ora_im, &MC6809::adda_im,
	&MC6809::cmpx_im, &MC6809::bsr, &MC6809::ldx_im, &MC6809::flag16_im,
/* 0x9X */
	&MC6809::suba_di, &MC6809::cmpa_di, &MC6809::sbca_di, &MC6809::subd_di,
	&MC6809::anda_di, &MC6809::bita_di, &MC6809::lda_di, &MC6809::sta_di,
	&MC6809::eora_di, &MC6809::adca_di, &MC6809::ora_di, &MC6809::adda_di,
	&MC6809::cmpx_di, &MC6809::jsr_di, &MC6809::ldx_di, &MC6809::stx_di,
/* 0xAX */
	&MC6809::suba_ix, &MC6809::cmpa_ix, &MC6809::sbca_ix, &MC6809::subd_ix,
	&MC6809::anda_ix, &MC6809::bita_ix, &MC6809::lda_ix, &MC6809::sta_ix,
	&MC6809::eora_ix, &MC6809::adca_ix, &MC6809::ora_ix, &MC6809::adda_ix,
	&MC6809::cmpx_ix, &MC6809::jsr_ix, &MC6809::ldx_ix, &MC6809::stx_ix,
/* 0xBX */
	&MC6809::suba_ex, &MC6809::cmpa_ex, &MC6809::sbca_ex, &MC6809::subd_ex,
	&MC6809::anda_ex, &MC6809::bita_ex, &MC6809::lda_ex, &MC6809::sta_ex,
	&MC6809::eora_ex, &MC6809::adca_ex, &MC6809::ora_ex, &MC6809::adda_ex,
	&MC6809::cmpx_ex, &MC6809::jsr_ex, &MC6809::ldx_ex, &MC6809::stx_ex,
/* 0xCX */
	&MC6809::subb_im, &MC6809::cmpb_im, &MC6809::sbcb_im, &MC6809::addd_im,
	&MC6809::andb_im, &MC6809::bitb_im, &MC6809::ldb_im, &MC6809::flag8_im,
	&MC6809::eorb_im, &MC6809::adcb_im, &MC6809::orb_im, &MC6809::addb_im,
	&MC6809::ldd_im, &MC6809::trap, &MC6809::ldu_im, &MC6809::flag16_im,
/* 0xDX */
	&MC6809::subb_di, &MC6809::cmpb_di, &MC6809::sbcb_di, &MC6809::addd_di,
	&MC6809::andb_di, &MC6809::bitb_di, &MC6809::ldb_di, &MC6809::stb_di,
	&MC6809::eorb_di, &MC6809::adcb_di, &MC6809::orb_di, &MC6809::addb_di,
	&MC6809::ldd_di, &MC6809::std_di, &MC6809::ldu_di, &MC6809::stu_di,
/* 0xEX */
	&MC6809::subb_ix, &MC6809::cmpb_ix, &MC6809::sbcb_ix, &MC6809::addd_ix,
	&MC6809::andb_ix, &MC6809::bitb_ix, &MC6809::ldb_ix, &MC6809::stb_ix,
	&MC6809::eorb_ix, &MC6809::adcb_ix, &MC6809::orb_ix, &MC6809::addb_ix,
	&MC6809::ldd_ix, &MC6809::std_ix, &MC6809::ldu_ix, &MC6809::stu_ix,
/* 0xFX */
	&MC6809::subb_ex, &MC6809::cmpb_ex, &MC6809::sbcb_ex, &MC6809::addd_ex,
	&MC6809::andb_ex, &MC6809::bitb_ex, &MC6809::ldb_ex, &MC6809::stb_ex,
	&MC6809::eorb_ex, &MC6809::adcb_ex, &MC6809::orb_ex, &MC6809::addb_ex,
	&MC6809::ldd_ex, &MC6809::std_ex, &MC6809::ldu_ex, &MC6809::stu_ex
};

void MC6809::reset()
{
	icount = 0;
	int_state &= MC6809_HALT_BIT;
	extra_icount = 0;
	//busreq = false;
   
	DPD = 0;	/* Reset direct page register */
	CC = 0;
	D = 0;
	X = 0;
	Y = 0;
	U = 0;
	S = 0;
	EA = 0;
#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	clr_used = false;
	write_signals(&outputs_bus_clr, 0x00000000);
#endif
	write_signals(&outputs_bus_halt, ((int_state & MC6809_HALT_BIT) != 0) ? 0xffffffff : 0x00000000);
   
	CC |= CC_II;	/* IRQ disabled */
	CC |= CC_IF;	/* FIRQ disabled */
	
	pPC = RM16_PAIR(0xfffe);
}


void MC6809::initialize()
{
	int_state = 0;
	busreq = false;
#ifdef USE_DEBUGGER
	d_mem_stored = d_mem;
	d_debugger->set_context_mem(d_mem);
#endif
}

void MC6809::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_CPU_IRQ) {
		if(data & mask) {
			int_state |= MC6809_IRQ_BIT;
		} else {
			int_state &= ~MC6809_IRQ_BIT;
		}
	} else if(id == SIG_CPU_FIRQ) {
		if(data & mask) {
			int_state |= MC6809_FIRQ_BIT;
		} else {
			int_state &= ~MC6809_FIRQ_BIT;
		}
	} else if(id == SIG_CPU_NMI) {
		if(data & mask) {
			int_state |= MC6809_NMI_BIT;
		} else {
			int_state &= ~MC6809_NMI_BIT;
		}
	} else if(id == SIG_CPU_BUSREQ) {
		if(data & mask) {
			int_state |= MC6809_HALT_BIT;
		} else {
			int_state &= ~MC6809_HALT_BIT;
		}
	}
}

void MC6809::cpu_nmi(void)
{
	pair rpc = pPC;
	if ((int_state & MC6809_CWAI_IN) == 0) {
		CC |= CC_E;
		PUSHWORD(pPC);
		PUSHWORD(pU);
		PUSHWORD(pY);
		PUSHWORD(pX);
		PUSHBYTE(DP);
		PUSHBYTE(B);
		PUSHBYTE(A);
		PUSHBYTE(CC);
	}
	CC = CC | CC_II | CC_IF;	// 0x50
	pPC = RM16_PAIR(0xfffc);
//	printf("NMI occured PC=0x%04x VECTOR=%04x SP=%04x \n",rpc.w.l,pPC.w.l,S);
	int_state |= MC6809_CWAI_OUT;
	int_state &= ~(MC6809_NMI_BIT | MC6809_SYNC_IN | MC6809_SYNC_OUT | MC6809_CWAI_IN);	// $FF1E
}


// Refine from cpu_x86.asm of V3.52a.
void MC6809::cpu_firq(void)
{
	pair rpc = pPC;
	if ((int_state & MC6809_CWAI_IN) == 0) {
		/* NORMAL */
		CC &= ~CC_E;
		PUSHWORD(pPC);
		PUSHBYTE(CC);
	}
	CC = CC | CC_IF | CC_II;
	pPC = RM16_PAIR(0xfff6);
	int_state |= MC6809_CWAI_OUT;
	int_state &= ~(MC6809_SYNC_IN | MC6809_SYNC_OUT | MC6809_CWAI_IN);
//	printf("Firq occured PC=0x%04x VECTOR=%04x SP=%04x \n",rpc.w.l,pPC.w.l,S);
}

// Refine from cpu_x86.asm of V3.52a.
void MC6809::cpu_irq(void)
{
	pair rpc = pPC;
	if ((int_state & MC6809_CWAI_IN) == 0) {
		CC |= CC_E;
		PUSHWORD(pPC);
		PUSHWORD(pU);
		PUSHWORD(pY);
		PUSHWORD(pX);
		PUSHBYTE(DP);
		PUSHBYTE(B);
		PUSHBYTE(A);
		PUSHBYTE(CC);
	}
	CC |= CC_II;
	pPC = RM16_PAIR(0xfff8);
	int_state |= MC6809_CWAI_OUT;
	int_state &= ~(MC6809_SYNC_IN | MC6809_SYNC_OUT | MC6809_CWAI_IN);

//	printf("IRQ occured PC=0x%04x VECTOR=%04x SP=%04x \n",rpc.w.l,pPC.w.l,S);
}

int MC6809::run(int clock)
{
	int cycle = 0;
	if ((int_state & MC6809_HALT_BIT) != 0) {	// 0x80
		if(icount > 0) icount = 0;  // OK?
	   	if(!busreq) write_signals(&outputs_bus_halt, 0xffffffff);
		busreq = true;
		return icount;
	}
	if(busreq) write_signals(&outputs_bus_halt, 0x00000000);
	busreq = false;
	if((int_state & MC6809_INSN_HALT) != 0) {	// 0x80
		uint8 dmy = RM(PCD);
		icount -= 2;
	        icount -= extra_icount;
	        extra_icount = 0;
		PC++;
		return icount;
	}
 	/*
	 * Check Interrupt
	 */
check_nmi:
	if ((int_state & (MC6809_NMI_BIT | MC6809_FIRQ_BIT | MC6809_IRQ_BIT)) != 0) {	// 0x0007
		if ((int_state & MC6809_NMI_BIT) == 0)
			goto check_firq;
		//if ((int_state & MC6809_LDS) == 0)
		//	goto check_firq;
		cpu_nmi();
		run_one_opecode();
		int_state &= ~MC6809_SYNC_IN;
		cycle = 19;
		goto int_cycle;
	} else {
		goto check_ok;
	}

check_firq:
	if ((int_state & MC6809_FIRQ_BIT) != 0) {
		if ((CC & CC_IF) != 0)
			goto check_irq;
		cpu_firq();
		run_one_opecode();
		int_state &= ~MC6809_SYNC_IN;
		cycle = 10;
		goto int_cycle;
	}

check_irq:
	if ((int_state & MC6809_IRQ_BIT) != 0) {
		if ((CC & CC_II) != 0)
			goto check_ok;
		cpu_irq();
		run_one_opecode();
		int_state &= ~MC6809_SYNC_IN;
		cycle = 19;
		goto int_cycle;
	}
	/*
	 * NO INTERRUPT
	 */
	goto check_ok;
	/*
	 * INTERRUPT
	 */
int_cycle:
	if((int_state & MC6809_CWAI_IN) == 0) {
		icount -= cycle;
	} else {
		int_state &= ~MC6809_CWAI_IN;
	}
	return icount;

	// run cpu
check_ok:
	if((int_state & MC6809_SYNC_IN) != 0) {
		icount = 0;
		return icount;
	}
	if((int_state & MC6809_CWAI_IN) == 0) {
		if(clock == -1) {
		// run only one opcode
		  //icount = 0;
			run_one_opecode();
			return icount;
		} else {
			// run cpu while given clocks
			icount += clock;
			int first_icount = icount;
			
			while(icount > 0) {
				run_one_opecode();
			}
			return first_icount - icount;
		}
	} else {
		icount = 0;
		return icount;
	}

}

void MC6809::run_one_opecode()
{
#ifdef USE_DEBUGGER
	bool now_debugging = d_debugger->now_debugging;
	if(now_debugging) {
		d_debugger->check_break_points(PC);
		if(d_debugger->now_suspended) {
			emu->mute_sound();
			while(d_debugger->now_debugging && d_debugger->now_suspended) {
			  Sleep(10);
			}
		}
		if(d_debugger->now_debugging) {
			d_mem = d_debugger;
		} else {
			now_debugging = false;
		}
		
		pPPC = pPC;
		uint8 ireg = ROP(PCD);
		PC++;
		icount -= cycles1[ireg];
		icount -= extra_icount;
		extra_icount = 0;
		op(ireg);
		
		if(now_debugging) {
			if(!d_debugger->now_going) {
				d_debugger->now_suspended = true;
			}
			d_mem = d_mem_stored;
		}
	} else {
		pPPC = pPC;
		uint8 ireg = ROP(PCD);
		PC++;
		icount -= cycles1[ireg];
		icount -= extra_icount;
		extra_icount = 0;
		op(ireg);
	}
#else
	pPPC = pPC;
	uint8 ireg = ROP(PCD);
	PC++;
	icount -= cycles1[ireg];
	icount -= extra_icount;
	extra_icount = 0;
	op(ireg);
#endif
}

void MC6809::op(uint8 ireg)
{
#if defined(_FM7) || defined(_FM8) || defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	if(ireg == 0x0f) { // clr_di()
		write_signals(&outputs_bus_clr, 0x00000001);
		clr_used = true;
	} else if((ireg == 0x6f) || (ireg == 0x7f)){ //clr_ex() clr_ix()
		write_signals(&outputs_bus_clr, 0x00000002);
		clr_used = true;
	} else {
		if(clr_used) write_signals(&outputs_bus_clr, 0x00000000);
		clr_used = false;
	}
   
#endif
	//printf("CPU(%08x) PC=%04x OP=%02x %02x %02x %02x %02x\n", (void *)this, PC, ireg, RM(PC), RM(PC + 1), RM(PC + 2), RM(PC + 3));

	(this->*m6809_main[ireg])();
}

#ifdef USE_DEBUGGER
void MC6809::debug_write_data8(uint32 addr, uint32 data)
{
	d_mem_stored->write_data8(addr, data);
}

uint32 MC6809::debug_read_data8(uint32 addr)
{
	return d_mem_stored->read_data8(addr);
}

void MC6809::debug_write_io8(uint32 addr, uint32 data)
{
		
}

uint32 MC6809::debug_read_io8(uint32 addr)
{
	uint8 val = d_mem_stored->read_io8(addr);
	return val;
}

bool MC6809::debug_write_reg(_TCHAR *reg, uint32 data)
{
	if(_tcsicmp(reg, _T("PC")) == 0) {
		PC = data;
	} else if(_tcsicmp(reg, _T("DP")) == 0) {
		DP = data;
	} else if(_tcsicmp(reg, _T("A")) == 0) {
		A = data;
	} else if(_tcsicmp(reg, _T("B")) == 0) {
		B = data;
	} else if(_tcsicmp(reg, _T("D")) == 0) {
		D = data;
	} else if(_tcsicmp(reg, _T("U")) == 0) {
		U = data;
	} else if(_tcsicmp(reg, _T("X")) == 0) {
		X = data;
	} else if(_tcsicmp(reg, _T("Y")) == 0) {
		Y = data;
	} else if(_tcsicmp(reg, _T("S")) == 0) {
		S = data;
	} else if(_tcsicmp(reg, _T("CC")) == 0) {
		CC = data;
	} else {
		return false;
	}
	return true;
}

#ifdef _MSC_VER
#define snprintf _snprintf
#endif
void MC6809::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	snprintf(buffer, buffer_len,
		 _T("PC = %04x PPC = %04x INTR=[%s %s %s %s][%s %s %s %s %s] CC = [%c%c%c%c%c%c%c%c]\nA = %02x B = %02x DP = %02x X = %04x Y = %04x U = %04x S = %04x EA = %04x"),
		 PC,
		 PPC,
		 ((int_state & MC6809_IRQ_BIT) == 0)   ? "----" : " IRQ",
		 ((int_state & MC6809_FIRQ_BIT) == 0)  ? "----" : "FIRQ",
		 ((int_state & MC6809_NMI_BIT) == 0)   ? "----" : " NMI",
		 ((int_state & MC6809_HALT_BIT) == 0)  ? "----" : "HALT",
		 ((int_state & MC6809_CWAI_IN) == 0)   ? "--" : "CI",
		 ((int_state & MC6809_CWAI_OUT) == 0)  ? "--" : "CO",
		 ((int_state & MC6809_SYNC_IN) == 0)   ? "--" : "SI",
		 ((int_state & MC6809_SYNC_OUT) == 0)  ? "--" : "SO",
		 ((int_state & MC6809_INSN_HALT) == 0) ? "----" : "TRAP",
		 ((CC & CC_E) == 0)  ? '-' : 'E', 
		 ((CC & CC_IF) == 0) ? '-' : 'F', 
		 ((CC & CC_H) == 0)  ? '-' : 'H', 
		 ((CC & CC_II) == 0) ? '-' : 'I', 
		 ((CC & CC_N) == 0)  ? '-' : 'N', 
		 ((CC & CC_Z) == 0)  ? '-' : 'Z', 
		 ((CC & CC_V) == 0)  ? '-' : 'V', 
		 ((CC & CC_C) == 0)  ? '-' : 'C',
		 A, B, DP,
		 X, Y, U, S,
		 EAD
	 );
}  
#ifdef _MSC_VER
#undef snprintf
#endif

// from MAME 0.160

/*****************************************************************************

    6809dasm.c - a 6809 opcode disassembler
    Version 1.4 1-MAR-95
    Copyright Sean Riddle

    Thanks to Franklin Bowen for bug fixes, ideas

    Freely distributable on any medium given all copyrights are retained
    by the author and no charge greater than $7.00 is made for obtaining
    this software

    Please send all bug reports, update ideas and data files to:
    sriddle@ionet.net

*****************************************************************************/

// Opcode structure
struct opcodeinfo
{
	uint8   opcode;     // 8-bit opcode value
	uint8   length;     // Opcode length in bytes
	_TCHAR  name[6];    // Opcode name
	uint8   mode;       // Addressing mode
//	unsigned flags;     // Disassembly flags
};

enum m6809_addressing_modes
{
	INH,                // Inherent
	DIR,                // Direct
	IND,                // Indexed
	REL,                // Relative (8 bit)
	LREL,               // Long relative (16 bit)
	EXT,                // Extended
	IMM,                // Immediate
	IMM_RR,             // Register-to-register
	PG1,                // Switch to page 1 opcodes
	PG2                 // Switch to page 2 opcodes
};

// Page 0 opcodes (single byte)
static const opcodeinfo m6809_pg0opcodes[] =
{
	{ 0x00, 2, _T("NEG"),   DIR    },
	{ 0x01, 2, _T("NEG"),   DIR    },
	{ 0x02, 2, _T("NGC"),   DIR    },
	{ 0x03, 2, _T("COM"),   DIR    },
	{ 0x04, 2, _T("LSR"),   DIR    },
	{ 0x05, 2, _T("LSR"),   DIR    },
	{ 0x06, 2, _T("ROR"),   DIR    },
	{ 0x07, 2, _T("ASR"),   DIR    },
	{ 0x08, 2, _T("ASL"),   DIR    },
	{ 0x09, 2, _T("ROL"),   DIR    },
	{ 0x0A, 2, _T("DEC"),   DIR    },
	{ 0x0B, 2, _T("DCC"),   DIR    },
	{ 0x0C, 2, _T("INC"),   DIR    },
	{ 0x0D, 2, _T("TST"),   DIR    },
	{ 0x0E, 2, _T("JMP"),   DIR    },
	{ 0x0F, 2, _T("CLR"),   DIR    },

	{ 0x10, 1, _T("page1"), PG1    },
	{ 0x11, 1, _T("page2"), PG2    },
	{ 0x12, 1, _T("NOP"),   INH    },
	{ 0x13, 1, _T("SYNC"),  INH    },
	{ 0x14, 1, _T("HALT"),  INH    },
	{ 0x15, 1, _T("HALT"),  INH    },
	{ 0x16, 3, _T("LBRA"),  LREL   },
	{ 0x17, 3, _T("LBSR"),  LREL   },
	{ 0x18, 1, _T("ASLCC"), INH    },
	{ 0x19, 1, _T("DAA"),   INH    },
	{ 0x1A, 2, _T("ORCC"),  IMM    },
	{ 0x1B, 1, _T("NOP"),   INH    },
	{ 0x1C, 2, _T("ANDCC"), IMM    },
	{ 0x1D, 1, _T("SEX"),   INH    },
	{ 0x1E, 2, _T("EXG"),   IMM_RR },
	{ 0x1F, 2, _T("TFR"),   IMM_RR },

	{ 0x20, 2, _T("BRA"),   REL    },
	{ 0x21, 2, _T("BRN"),   REL    },
	{ 0x22, 2, _T("BHI"),   REL    },
	{ 0x23, 2, _T("BLS"),   REL    },
	{ 0x24, 2, _T("BCC"),   REL    },
	{ 0x25, 2, _T("BCS"),   REL    },
	{ 0x26, 2, _T("BNE"),   REL    },
	{ 0x27, 2, _T("BEQ"),   REL    },
	{ 0x28, 2, _T("BVC"),   REL    },
	{ 0x29, 2, _T("BVS"),   REL    },
	{ 0x2A, 2, _T("BPL"),   REL    },
	{ 0x2B, 2, _T("BMI"),   REL    },
	{ 0x2C, 2, _T("BGE"),   REL    },
	{ 0x2D, 2, _T("BLT"),   REL    },
	{ 0x2E, 2, _T("BGT"),   REL    },
	{ 0x2F, 2, _T("BLE"),   REL    },

	{ 0x30, 2, _T("LEAX"),  IND    },
	{ 0x31, 2, _T("LEAY"),  IND    },
	{ 0x32, 2, _T("LEAS"),  IND    },
	{ 0x33, 2, _T("LEAU"),  IND    },
	{ 0x34, 2, _T("PSHS"),  INH    },
	{ 0x35, 2, _T("PULS"),  INH    },
	{ 0x36, 2, _T("PSHU"),  INH    },
	{ 0x37, 2, _T("PULU"),  INH    },
	{ 0x38, 2, _T("ANDCC"), IMM    },
	{ 0x39, 1, _T("RTS"),   INH    },
	{ 0x3A, 1, _T("ABX"),   INH    },
	{ 0x3B, 1, _T("RTI"),   INH    },
	{ 0x3C, 2, _T("CWAI"),  IMM    },
	{ 0x3D, 1, _T("MUL"),   INH    },
	{ 0x3F, 1, _T("SWI"),   INH    },

	{ 0x40, 1, _T("NEGA"),  INH    },
	{ 0x41, 1, _T("NEGA"),  INH    },
	{ 0x42, 1, _T("NGGA"),  INH    },
	{ 0x43, 1, _T("COMA"),  INH    },
	{ 0x44, 1, _T("LSRA"),  INH    },
	{ 0x45, 1, _T("LSRA"),  INH    },
	{ 0x46, 1, _T("RORA"),  INH    },
	{ 0x47, 1, _T("ASRA"),  INH    },
	{ 0x48, 1, _T("ASLA"),  INH    },
	{ 0x49, 1, _T("ROLA"),  INH    },
	{ 0x4A, 1, _T("DECA"),  INH    },
	{ 0x4B, 1, _T("DCCA"),  INH    },
	{ 0x4C, 1, _T("INCA"),  INH    },
	{ 0x4D, 1, _T("TSTA"),  INH    },
	{ 0x4E, 1, _T("CLCA"),  INH    },
	{ 0x4F, 1, _T("CLRA"),  INH    },

	{ 0x50, 1, _T("NEGB"),  INH    },
	{ 0x51, 1, _T("NEGB"),  INH    },
	{ 0x52, 1, _T("NGGB"),  INH    },
	{ 0x53, 1, _T("COMB"),  INH    },
	{ 0x54, 1, _T("LSRB"),  INH    },
	{ 0x55, 1, _T("LSRB"),  INH    },
	{ 0x56, 1, _T("RORB"),  INH    },
	{ 0x57, 1, _T("ASRB"),  INH    },
	{ 0x58, 1, _T("ASLB"),  INH    },
	{ 0x59, 1, _T("ROLB"),  INH    },
	{ 0x5A, 1, _T("DECB"),  INH    },
	{ 0x5B, 1, _T("DCCB"),  INH    },
	{ 0x5C, 1, _T("INCB"),  INH    },
	{ 0x5D, 1, _T("TSTB"),  INH    },
	{ 0x5E, 1, _T("CLCB"),  INH    },
	{ 0x5F, 1, _T("CLRB"),  INH    },

	{ 0x60, 2, _T("NEG"),   IND    },
	{ 0x61, 2, _T("NEG"),   IND    },
	{ 0x62, 2, _T("NGC"),   IND    },
	{ 0x63, 2, _T("COM"),   IND    },
	{ 0x64, 2, _T("LSR"),   IND    },
	{ 0x65, 2, _T("LSR"),   IND    },
	{ 0x66, 2, _T("ROR"),   IND    },
	{ 0x67, 2, _T("ASR"),   IND    },
	{ 0x68, 2, _T("ASL"),   IND    },
	{ 0x69, 2, _T("ROL"),   IND    },
	{ 0x6A, 2, _T("DEC"),   IND    },
	{ 0x6B, 2, _T("DCC"),   IND    },
	{ 0x6C, 2, _T("INC"),   IND    },
	{ 0x6D, 2, _T("TST"),   IND    },
	{ 0x6E, 2, _T("JMP"),   IND    },
	{ 0x6F, 2, _T("CLR"),   IND    },

	{ 0x70, 3, _T("NEG"),   EXT    },
	{ 0x71, 3, _T("NEG"),   EXT    },
	{ 0x72, 3, _T("NGC"),   EXT    },
	{ 0x73, 3, _T("COM"),   EXT    },
	{ 0x74, 3, _T("LSR"),   EXT    },
	{ 0x75, 3, _T("LSR"),   EXT    },
	{ 0x76, 3, _T("ROR"),   EXT    },
	{ 0x77, 3, _T("ASR"),   EXT    },
	{ 0x78, 3, _T("ASL"),   EXT    },
	{ 0x79, 3, _T("ROL"),   EXT    },
	{ 0x7A, 3, _T("DEC"),   EXT    },
	{ 0x7B, 3, _T("DCC"),   EXT    },
	{ 0x7C, 3, _T("INC"),   EXT    },
	{ 0x7D, 3, _T("TST"),   EXT    },
	{ 0x7E, 3, _T("JMP"),   EXT    },
	{ 0x7F, 3, _T("CLR"),   EXT    },

	{ 0x80, 2, _T("SUBA"),  IMM    },
	{ 0x81, 2, _T("CMPA"),  IMM    },
	{ 0x82, 2, _T("SBCA"),  IMM    },
	{ 0x83, 3, _T("SUBD"),  IMM    },
	{ 0x84, 2, _T("ANDA"),  IMM    },
	{ 0x85, 2, _T("BITA"),  IMM    },
	{ 0x86, 2, _T("LDA"),   IMM    },
	{ 0x87, 2, _T("FLAG"),  IMM    },
	{ 0x88, 2, _T("EORA"),  IMM    },
	{ 0x89, 2, _T("ADCA"),  IMM    },
	{ 0x8A, 2, _T("ORA"),   IMM    },
	{ 0x8B, 2, _T("ADDA"),  IMM    },
	{ 0x8C, 3, _T("CMPX"),  IMM    },
	{ 0x8D, 2, _T("BSR"),   REL    },
	{ 0x8E, 3, _T("LDX"),   IMM    },
	{ 0x8F, 3, _T("FLAG"),  IMM    },

	{ 0x90, 2, _T("SUBA"),  DIR    },
	{ 0x91, 2, _T("CMPA"),  DIR    },
	{ 0x92, 2, _T("SBCA"),  DIR    },
	{ 0x93, 2, _T("SUBD"),  DIR    },
	{ 0x94, 2, _T("ANDA"),  DIR    },
	{ 0x95, 2, _T("BITA"),  DIR    },
	{ 0x96, 2, _T("LDA"),   DIR    },
	{ 0x97, 2, _T("STA"),   DIR    },
	{ 0x98, 2, _T("EORA"),  DIR    },
	{ 0x99, 2, _T("ADCA"),  DIR    },
	{ 0x9A, 2, _T("ORA"),   DIR    },
	{ 0x9B, 2, _T("ADDA"),  DIR    },
	{ 0x9C, 2, _T("CMPX"),  DIR    },
	{ 0x9D, 2, _T("JSR"),   DIR    },
	{ 0x9E, 2, _T("LDX"),   DIR    },
	{ 0x9F, 2, _T("STX"),   DIR    },

	{ 0xA0, 2, _T("SUBA"),  IND    },
	{ 0xA1, 2, _T("CMPA"),  IND    },
	{ 0xA2, 2, _T("SBCA"),  IND    },
	{ 0xA3, 2, _T("SUBD"),  IND    },
	{ 0xA4, 2, _T("ANDA"),  IND    },
	{ 0xA5, 2, _T("BITA"),  IND    },
	{ 0xA6, 2, _T("LDA"),   IND    },
	{ 0xA7, 2, _T("STA"),   IND    },
	{ 0xA8, 2, _T("EORA"),  IND    },
	{ 0xA9, 2, _T("ADCA"),  IND    },
	{ 0xAA, 2, _T("ORA"),   IND    },
	{ 0xAB, 2, _T("ADDA"),  IND    },
	{ 0xAC, 2, _T("CMPX"),  IND    },
	{ 0xAD, 2, _T("JSR"),   IND    },
	{ 0xAE, 2, _T("LDX"),   IND    },
	{ 0xAF, 2, _T("STX"),   IND    },

	{ 0xB0, 3, _T("SUBA"),  EXT    },
	{ 0xB1, 3, _T("CMPA"),  EXT    },
	{ 0xB2, 3, _T("SBCA"),  EXT    },
	{ 0xB3, 3, _T("SUBD"),  EXT    },
	{ 0xB4, 3, _T("ANDA"),  EXT    },
	{ 0xB5, 3, _T("BITA"),  EXT    },
	{ 0xB6, 3, _T("LDA"),   EXT    },
	{ 0xB7, 3, _T("STA"),   EXT    },
	{ 0xB8, 3, _T("EORA"),  EXT    },
	{ 0xB9, 3, _T("ADCA"),  EXT    },
	{ 0xBA, 3, _T("ORA"),   EXT    },
	{ 0xBB, 3, _T("ADDA"),  EXT    },
	{ 0xBC, 3, _T("CMPX"),  EXT    },
	{ 0xBD, 3, _T("JSR"),   EXT    },
	{ 0xBE, 3, _T("LDX"),   EXT    },
	{ 0xBF, 3, _T("STX"),   EXT    },

	{ 0xC0, 2, _T("SUBB"),  IMM    },
	{ 0xC1, 2, _T("CMPB"),  IMM    },
	{ 0xC2, 2, _T("SBCB"),  IMM    },
	{ 0xC3, 3, _T("ADDD"),  IMM    },
	{ 0xC4, 2, _T("ANDB"),  IMM    },
	{ 0xC5, 2, _T("BITB"),  IMM    },
	{ 0xC6, 2, _T("LDB"),   IMM    },
	{ 0xC7, 2, _T("FLAG"),  IMM    },
	{ 0xC8, 2, _T("EORB"),  IMM    },
	{ 0xC9, 2, _T("ADCB"),  IMM    },
	{ 0xCA, 2, _T("ORB"),   IMM    },
	{ 0xCB, 2, _T("ADDB"),  IMM    },
	{ 0xCC, 3, _T("LDD"),   IMM    },
	{ 0xCD, 1, _T("HALT"),  INH    },
	{ 0xCE, 3, _T("LDU"),   IMM    },
	{ 0xCF, 3, _T("FLAG"),  IMM    },

	{ 0xD0, 2, _T("SUBB"),  DIR    },
	{ 0xD1, 2, _T("CMPB"),  DIR    },
	{ 0xD2, 2, _T("SBCB"),  DIR    },
	{ 0xD3, 2, _T("ADDD"),  DIR    },
	{ 0xD4, 2, _T("ANDB"),  DIR    },
	{ 0xD5, 2, _T("BITB"),  DIR    },
	{ 0xD6, 2, _T("LDB"),   DIR    },
	{ 0xD7, 2, _T("STB"),   DIR    },
	{ 0xD8, 2, _T("EORB"),  DIR    },
	{ 0xD9, 2, _T("ADCB"),  DIR    },
	{ 0xDA, 2, _T("ORB"),   DIR    },
	{ 0xDB, 2, _T("ADDB"),  DIR    },
	{ 0xDC, 2, _T("LDD"),   DIR    },
	{ 0xDD, 2, _T("STD"),   DIR    },
	{ 0xDE, 2, _T("LDU"),   DIR    },
	{ 0xDF, 2, _T("STU"),   DIR    },

	{ 0xE0, 2, _T("SUBB"),  IND    },
	{ 0xE1, 2, _T("CMPB"),  IND    },
	{ 0xE2, 2, _T("SBCB"),  IND    },
	{ 0xE3, 2, _T("ADDD"),  IND    },
	{ 0xE4, 2, _T("ANDB"),  IND    },
	{ 0xE5, 2, _T("BITB"),  IND    },
	{ 0xE6, 2, _T("LDB"),   IND    },
	{ 0xE7, 2, _T("STB"),   IND    },
	{ 0xE8, 2, _T("EORB"),  IND    },
	{ 0xE9, 2, _T("ADCB"),  IND    },
	{ 0xEA, 2, _T("ORB"),   IND    },
	{ 0xEB, 2, _T("ADDB"),  IND    },
	{ 0xEC, 2, _T("LDD"),   IND    },
	{ 0xED, 2, _T("STD"),   IND    },
	{ 0xEE, 2, _T("LDU"),   IND    },
	{ 0xEF, 2, _T("STU"),   IND    },

	{ 0xF0, 3, _T("SUBB"),  EXT    },
	{ 0xF1, 3, _T("CMPB"),  EXT    },
	{ 0xF2, 3, _T("SBCB"),  EXT    },
	{ 0xF3, 3, _T("ADDD"),  EXT    },
	{ 0xF4, 3, _T("ANDB"),  EXT    },
	{ 0xF5, 3, _T("BITB"),  EXT    },
	{ 0xF6, 3, _T("LDB"),   EXT    },
	{ 0xF7, 3, _T("STB"),   EXT    },
	{ 0xF8, 3, _T("EORB"),  EXT    },
	{ 0xF9, 3, _T("ADCB"),  EXT    },
	{ 0xFA, 3, _T("ORB"),   EXT    },
	{ 0xFB, 3, _T("ADDB"),  EXT    },
	{ 0xFC, 3, _T("LDD"),   EXT    },
	{ 0xFD, 3, _T("STD"),   EXT    },
	{ 0xFE, 3, _T("LDU"),   EXT    },
	{ 0xFF, 3, _T("STU"),   EXT    }
};

// Page 1 opcodes (0x10 0x..)
static const opcodeinfo m6809_pg1opcodes[] =
{
	{ 0x20, 4, _T("LBRA"),  LREL   },
	{ 0x21, 4, _T("LBRN"),  LREL   },
	{ 0x22, 4, _T("LBHI"),  LREL   },
	{ 0x23, 4, _T("LBLS"),  LREL   },
	{ 0x24, 4, _T("LBCC"),  LREL   },
	{ 0x25, 4, _T("LBCS"),  LREL   },
	{ 0x26, 4, _T("LBNE"),  LREL   },
	{ 0x27, 4, _T("LBEQ"),  LREL   },
	{ 0x28, 4, _T("LBVC"),  LREL   },
	{ 0x29, 4, _T("LBVS"),  LREL   },
	{ 0x2A, 4, _T("LBPL"),  LREL   },
	{ 0x2B, 4, _T("LBMI"),  LREL   },
	{ 0x2C, 4, _T("LBGE"),  LREL   },
	{ 0x2D, 4, _T("LBLT"),  LREL   },
	{ 0x2E, 4, _T("LBGT"),  LREL   },
	{ 0x2F, 4, _T("LBLE"),  LREL   },
	{ 0x3F, 2, _T("SWI2"),  INH    },
	{ 0x83, 4, _T("CMPD"),  IMM    },
	{ 0x8C, 4, _T("CMPY"),  IMM    },
	{ 0x8D, 4, _T("LBSR"),  LREL   },
	{ 0x8E, 4, _T("LDY"),   IMM    },
	{ 0x93, 3, _T("CMPD"),  DIR    },
	{ 0x9C, 3, _T("CMPY"),  DIR    },
	{ 0x9E, 3, _T("LDY"),   DIR    },
	{ 0x9F, 3, _T("STY"),   DIR    },
	{ 0xA3, 3, _T("CMPD"),  IND    },
	{ 0xAC, 3, _T("CMPY"),  IND    },
	{ 0xAE, 3, _T("LDY"),   IND    },
	{ 0xAF, 3, _T("STY"),   IND    },
	{ 0xB3, 4, _T("CMPD"),  EXT    },
	{ 0xBC, 4, _T("CMPY"),  EXT    },
	{ 0xBE, 4, _T("LDY"),   EXT    },
	{ 0xBF, 4, _T("STY"),   EXT    },
	{ 0xCE, 4, _T("LDS"),   IMM    },
	{ 0xDE, 3, _T("LDS"),   DIR    },
	{ 0xDF, 3, _T("STS"),   DIR    },
	{ 0xEE, 3, _T("LDS"),   IND    },
	{ 0xEF, 3, _T("STS"),   IND    },
	{ 0xFE, 4, _T("LDS"),   EXT    },
	{ 0xFF, 4, _T("STS"),   EXT    }
};

// Page 2 opcodes (0x11 0x..)
static const opcodeinfo m6809_pg2opcodes[] =
{
	{ 0x3F, 2, _T("SWI3"),  INH    },
	{ 0x83, 4, _T("CMPU"),  IMM    },
	{ 0x8C, 4, _T("CMPS"),  IMM    },
	{ 0x93, 3, _T("CMPU"),  DIR    },
	{ 0x9C, 3, _T("CMPS"),  DIR    },
	{ 0xA3, 3, _T("CMPU"),  IND    },
	{ 0xAC, 3, _T("CMPS"),  IND    },
	{ 0xB3, 4, _T("CMPU"),  EXT    },
	{ 0xBC, 4, _T("CMPS"),  EXT    }
};

static const opcodeinfo *const m6809_pgpointers[3] =
{
	m6809_pg0opcodes, m6809_pg1opcodes, m6809_pg2opcodes
};

static const int m6809_numops[3] =
{
	array_length(m6809_pg0opcodes),
	array_length(m6809_pg1opcodes),
	array_length(m6809_pg2opcodes)
};

static const _TCHAR *const m6809_regs[5] = { _T("X"), _T("Y"), _T("U"), _T("S"), _T("PC") };

static const _TCHAR *const m6809_regs_te[16] =
{
	_T("D"), _T("X"),  _T("Y"),  _T("U"),   _T("S"),  _T("PC"), _T("inv"), _T("inv"),
	_T("A"), _T("B"), _T("CC"), _T("DP"), _T("inv"), _T("inv"), _T("inv"), _T("inv")
};

uint32 MC6809::cpu_disassemble_m6809(_TCHAR *buffer, uint32 pc, const uint8 *oprom, const uint8 *opram)
{
	uint8 opcode, mode, pb, pbm, reg;
	const uint8 *operandarray;
	unsigned int ea;//, flags;
	int numoperands, offset;
	int i, p = 0, page = 0;
	bool opcode_found = false;
	bool indirect;

	do {
		opcode = oprom[p++];

		for (i = 0; i < m6809_numops[page]; i++)
			if (m6809_pgpointers[page][i].opcode == opcode)
				break;

		if (i < m6809_numops[page])
			opcode_found = true;
		else
		{
			_stprintf(buffer, _T("Illegal Opcode %02X"), opcode);
			return p;
		}

		if (m6809_pgpointers[page][i].mode >= PG1)
		{
			page = m6809_pgpointers[page][i].mode - PG1 + 1;
			opcode_found = false;
		}
	} while (!opcode_found);

	if (page == 0)
		numoperands = m6809_pgpointers[page][i].length - 1;
	else
		numoperands = m6809_pgpointers[page][i].length - 2;

	operandarray = &opram[p];
	p += numoperands;
	pc += p;
	mode = m6809_pgpointers[page][i].mode;
//	flags = m6809_pgpointers[page][i].flags;

	buffer += _stprintf(buffer, _T("%-6s"), m6809_pgpointers[page][i].name);

	switch (mode)
	{
	case INH:
		switch (opcode)
		{
		case 0x34:  // PSHS
		case 0x36:  // PSHU
			pb = operandarray[0];
			if (pb & 0x80)
				buffer += _stprintf(buffer, _T("PC"));
			if (pb & 0x40)
				buffer += _stprintf(buffer, _T("%s%s"), (pb&0x80)?_T(","):_T(""), (opcode==0x34)?"U":"S");
			if (pb & 0x20)
				buffer += _stprintf(buffer, _T("%sY"),  (pb&0xc0)?_T(","):_T(""));
			if (pb & 0x10)
				buffer += _stprintf(buffer, _T("%sX"),  (pb&0xe0)?_T(","):_T(""));
			if (pb & 0x08)
				buffer += _stprintf(buffer, _T("%sDP"), (pb&0xf0)?_T(","):_T(""));
			if (pb & 0x04)
				buffer += _stprintf(buffer, _T("%sB"),  (pb&0xf8)?_T(","):_T(""));
			if (pb & 0x02)
				buffer += _stprintf(buffer, _T("%sA"),  (pb&0xfc)?_T(","):_T(""));
			if (pb & 0x01)
				buffer += _stprintf(buffer, _T("%sCC"), (pb&0xfe)?_T(","):_T(""));
			break;
		case 0x35:  // PULS
		case 0x37:  // PULU
			pb = operandarray[0];
			if (pb & 0x01)
				buffer += _stprintf(buffer, _T("CC"));
			if (pb & 0x02)
				buffer += _stprintf(buffer, _T("%sA"),  (pb&0x01)?_T(","):_T(""));
			if (pb & 0x04)
				buffer += _stprintf(buffer, _T("%sB"),  (pb&0x03)?_T(","):_T(""));
			if (pb & 0x08)
				buffer += _stprintf(buffer, _T("%sDP"), (pb&0x07)?_T(","):_T(""));
			if (pb & 0x10)
				buffer += _stprintf(buffer, _T("%sX"),  (pb&0x0f)?_T(","):_T(""));
			if (pb & 0x20)
				buffer += _stprintf(buffer, _T("%sY"),  (pb&0x1f)?_T(","):_T(""));
			if (pb & 0x40)
				buffer += _stprintf(buffer, _T("%s%s"), (pb&0x3f)?_T(","):_T(""), (opcode==0x35)?_T("U"):_T("S"));
			if (pb & 0x80)
				buffer += _stprintf(buffer, _T("%sPC ; (PUL? PC=RTS)"), (pb&0x7f)?_T(","):_T(""));
			break;
		default:
			// No operands
			break;
		}
		break;

	case DIR:
		ea = operandarray[0];
		buffer += _stprintf(buffer, _T("$%02X"), ea);
		break;

	case REL:
		offset = (INT8)operandarray[0];
		buffer += _stprintf(buffer, _T("$%04X"), (pc + offset) & 0xffff);
		break;

	case LREL:
		offset = (INT16)((operandarray[0] << 8) + operandarray[1]);
		buffer += _stprintf(buffer, _T("$%04X"), (pc + offset) & 0xffff);
		break;

	case EXT:
		ea = (operandarray[0] << 8) + operandarray[1];
		buffer += _stprintf(buffer, _T("$%04X"), ea);
		break;

	case IND:
		pb = operandarray[0];
		reg = (pb >> 5) & 3;
		pbm = pb & 0x8f;
		indirect = ((pb & 0x90) == 0x90 )? true : false;

		// open brackets if indirect
		if (indirect && pbm != 0x80 && pbm != 0x82)
			buffer += _stprintf(buffer, _T("["));

		switch (pbm)
		{
		case 0x80:  // ,R+
			if (indirect)
				_tcscpy(buffer, _T("Illegal Postbyte"));
			else
				buffer += _stprintf(buffer, _T(",%s+"), m6809_regs[reg]);
			break;

		case 0x81:  // ,R++
			buffer += _stprintf(buffer, _T(",%s++"), m6809_regs[reg]);
			break;

		case 0x82:  // ,-R
		  //if (indirect)
		  //	_tcscpy(buffer, _T("Illegal Postbyte"));
		  //	else
				buffer += _stprintf(buffer, _T(",-%s"), m6809_regs[reg]);
			break;

		case 0x83:  // ,--R
			buffer += _stprintf(buffer, _T(",--%s"), m6809_regs[reg]);
			break;

		case 0x84:  // ,R
			buffer += _stprintf(buffer, _T(",%s"), m6809_regs[reg]);
			break;

		case 0x85:  // (+/- B),R
			buffer += _stprintf(buffer, _T("B,%s"), m6809_regs[reg]);
			break;

		case 0x86:  // (+/- A),R
			buffer += _stprintf(buffer, _T("A,%s"), m6809_regs[reg]);
			break;

		case 0x87:  // (+/- A),R // Also 0x*6.
			buffer += _stprintf(buffer, _T("A,%s"), m6809_regs[reg]);
			break;
			//case 0x87:
			//_tcscpy(buffer, _T("Illegal Postbyte"));
			//break;

		case 0x88:  // (+/- 7 bit offset),R
			offset = (INT8)opram[p++];
			buffer += _stprintf(buffer, _T("%s"), (offset < 0) ? "-" : "");
			buffer += _stprintf(buffer, _T("$%02X,"), (offset < 0) ? -offset : offset);
			buffer += _stprintf(buffer, _T("%s"), m6809_regs[reg]);
			break;

		case 0x89:  // (+/- 15 bit offset),R
			offset = (INT16)((opram[p+0] << 8) + opram[p+1]);
			p += 2;
			buffer += _stprintf(buffer, _T("%s"), (offset < 0) ? "-" : "");
			buffer += _stprintf(buffer, _T("$%04X,"), (offset < 0) ? -offset : offset);
			buffer += _stprintf(buffer, _T("%s"), m6809_regs[reg]);
			break;

		case 0x8a:
			_tcscpy(buffer, _T("Illegal Postbyte"));
			break;

		case 0x8b:  // (+/- D),R
			buffer += _stprintf(buffer, _T("D,%s"), m6809_regs[reg]);
			break;

		case 0x8c:  // (+/- 7 bit offset),PC
			offset = (INT8)opram[p++];
			buffer += _stprintf(buffer, _T("%s"), (offset < 0) ? "-" : "");
			buffer += _stprintf(buffer, _T("$%02X,PC"), (offset < 0) ? -offset : offset);
			break;

		case 0x8d:  // (+/- 15 bit offset),PC
			offset = (INT16)((opram[p+0] << 8) + opram[p+1]);
			p += 2;
			buffer += _stprintf(buffer, _T("%s"), (offset < 0) ? "-" : "");
			buffer += _stprintf(buffer, _T("$%04X,PC"), (offset < 0) ? -offset : offset);
			break;

		case 0x8e: // $FFFFF
		  //_tcscpy(buffer, _T("Illegal Postbyte"));
			offset = (INT16)0xffff;
			//p += 2;
			buffer += _stprintf(buffer, _T("$%04X"), offset);
			break;

		case 0x8f:  // address
			ea = (uint16)((opram[p+0] << 8) + opram[p+1]);
			p += 2;
			buffer += _stprintf(buffer, _T("$%04X"), ea);
			break;

		default:    // (+/- 4 bit offset),R
			offset = pb & 0x1f;
			if (offset > 15)
				offset = offset - 32;
			buffer += _stprintf(buffer, _T("%s"), (offset < 0) ? "-" : "");
			buffer += _stprintf(buffer, _T("$%X,"), (offset < 0) ? -offset : offset);
			buffer += _stprintf(buffer, _T("%s"), m6809_regs[reg]);
			break;
		}

		// close brackets if indirect
		if (indirect && pbm != 0x80 && pbm != 0x82)
			buffer += _stprintf(buffer, _T("]"));
		break;

	case IMM:
		if (numoperands == 2)
		{
			ea = (operandarray[0] << 8) + operandarray[1];
			buffer += _stprintf(buffer, _T("#$%04X"), ea);
		}
		else
		if (numoperands == 1)
		{
			ea = operandarray[0];
			buffer += _stprintf(buffer, _T("#$%02X"), ea);
		}
		break;

	case IMM_RR:
		pb = operandarray[0];
		buffer += _stprintf(buffer, _T("%s,%s"), m6809_regs_te[(pb >> 4) & 0xf], m6809_regs_te[pb & 0xf]);
		break;
	}

	return p;
}

int MC6809::debug_dasm(uint32 pc, _TCHAR *buffer, size_t buffer_len)
{
	_TCHAR buffer_tmp[1024]; // enough ???
	uint8 ops[4];
	for(int i = 0; i < 4; i++) {
		ops[i] = d_mem_stored->read_data8(pc + i);
	}
	int length = cpu_disassemble_m6809(buffer_tmp, pc, ops, ops);
	_strcpy_s(buffer, buffer_len, buffer_tmp);
	return length;
}
#endif




inline void MC6809::fetch_effective_address()
{
	uint8 postbyte;
	uint8 upper, lower;

	IMMBYTE(postbyte);
	
	upper = (postbyte >> 4) & 0x0f;
	lower = postbyte & 0x0f;
	switch (upper) {
		case 0x00:
			EA = X + lower;
			break;
		case 0x01:
			EA = X - 16 + lower;
			break;
		case 0x02:
			EA = Y + lower;
			break;
		case 0x03:
			EA = Y - 16 + lower;
			break;
		case 0x04:
			EA = U + lower;
			break;
		case 0x05:
			EA = U - 16 + lower;
			break;
		case 0x06:
			EA = S + lower;
			break;
		case 0x07:
			EA = S - 16 + lower;
			break;
		default:
			fetch_effective_address_IDX(upper, lower);
			break;
	}
	EAD &= 0xffff;
	icount -= index_cycle_em[postbyte];
}

inline void MC6809::fetch_effective_address_IDX(uint8 upper, uint8 lower)
{
	bool indirect = false;
	uint16 *reg;
	uint8 bx_p;
	pair pp;
	
	indirect = ((upper & 0x01) != 0) ? true : false;

	switch ((upper >> 1) & 0x03) {	// $8-$f >> 1 = $4 - $7 : delete bit2 
		case 0:	// $8x,$9x
			reg = &X;
			break;
		case 1:	// $ax,$bx
			reg = &Y;
			break;
		case 2:	// $cx,$dx
			reg = &U;
			break;
		case 3:	// $ex,$fx
			reg = &S;
			break;
	}

	switch (lower) {
		case 0:	// ,r+ 
			EA = *reg;
			*reg = *reg + 1;
			*reg = *reg & 0xffff;
			break;
		case 1:	// ,r++
			EA = *reg;
			*reg = *reg + 2;
			*reg = *reg & 0xffff;
			break;
		case 2:	// ,-r
			*reg = *reg - 1;
			*reg = *reg & 0xffff;
			EA = *reg;
			break;
		case 3:	// ,--r
			*reg = *reg - 2;
			*reg = *reg & 0xffff;
			EA = *reg;
			break;
		case 4:	// ,r
			EA = *reg;
			break;
		case 5:	// b,r
			EA = *reg + SIGNED(B);
			break;
		case 6:	// a,r
		case 7:
			EA = *reg + SIGNED(A);
			break;
		case 8:	// $xx,r
			IMMBYTE(bx_p);
			EA = *reg + SIGNED(bx_p);
			break;
		case 9:	// $xxxx, r
			IMMWORD(EAP);
			EA = EA + *reg;
			break;
		case 0x0a:	// Undocumented
			EA = PC;
			EA++;
			EAP.w.l |= 0x00ff;
			break;
		case 0x0b:	// D,r
			EA = *reg + D;
			break;
		case 0x0c:	// xx,pc
			IMMBYTE(bx_p);
			EA = PC + SIGNED(bx_p);
			break;
		case 0x0d:	// xxxx,pc
			IMMWORD(EAP);
			EA = EA + PC;
			break;
		case 0x0e:	// Undocumented
			EA = 0xffff;
			break;
		case 0x0f:
			IMMWORD(EAP);
			break;
	}
	EAP.w.h = 0x0000;
	// $9x,$bx,$dx,$fx = INDIRECT
	if (indirect) {
		pp = EAP;
		EAP = RM16_PAIR(pp.d);
	}
}

#define IIError() illegal()

OP_HANDLER(illegal)
{
	//logerror("M6809: illegal opcode at %04x\n",PC);
	//printf("M6809: illegal opcode at %04x %02x %02x %02x %02x %02x \n",
	//	 PC - 2, RM(PC - 2), RM(PC - 1), RM(PC), RM(PC + 1), RM(PC + 2));
//        PC-=1;
}

inline uint8 MC6809::GET_INDEXED_DATA(void)
{
	uint8 t;
	fetch_effective_address();
	t = RM(EAD);
	return t;
}

inline pair MC6809::GET_INDEXED_DATA16(void)
{
	pair t;
	fetch_effective_address();
	t = RM16_PAIR(EAD);
	return t;
}

// $x0, $x1
inline void MC6809::NEG_MEM(uint8 a_neg)
{							
	uint16 r_neg;					
	r_neg = 0 - (uint16)a_neg;
	CLR_NZVC;
	SET_FLAGS8(0, a_neg, r_neg);
	WM(EAD, r_neg);					
}

inline uint8 MC6809::NEG_REG(uint8 a_neg)
{
	uint16 r_neg;
	r_neg = 0 - (uint16)a_neg;
	CLR_NZVC;
	SET_FLAGS8(0, a_neg, r_neg);
	return (uint8)r_neg;
}


// $x2
inline void MC6809::COM_MEM(uint8 a_com)
{			 
	uint8 t_com;		 
	t_com = ~a_com;	 
	CLR_NZVC;		 
	SET_NZ8(t_com);	 
	SEC;
	WM(EAD, t_com);
}

inline uint8 MC6809::COM_REG(uint8 r_com)
{
	r_com = ~r_com;
	CLR_NZVC;
	SET_NZ8(r_com);
	SEC;
	return r_com;
}

inline void MC6809::LSR_MEM(uint8 t)
{
	CLR_NZC;
	CC = CC | (t & CC_C);
	t >>= 1;
	SET_NZ8(t);
	WM(EAD, t);
}

inline uint8 MC6809::LSR_REG(uint8 r)
{
	CLR_NZC;
	CC |= (r & CC_C);
	r >>= 1;
	SET_NZ8(r);
	return r;
}

inline void MC6809::ROR_MEM(uint8 t)
{
	uint8 r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	t >>= 1;
	r |= t;
	SET_NZ8(r); //NZ8?
	WM(EAD, r);
}

inline uint8 MC6809::ROR_REG(uint8 t)
{
	uint8 r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	t >>= 1;
	r |= t;
	SET_NZ8(r); //NZ8?
	return r;
}


inline void MC6809::ASR_MEM(uint8 t)
{
	uint8 r;
	CLR_NZC;
	CC = CC | (t & CC_C);
	r = (t & 0x80) | (t >> 1);
	// H is undefined
	SET_NZ8(r);
	//SET_H(t, t, r);
	WM(EAD, r);
}

inline uint8 MC6809::ASR_REG(uint8 t)
{
	uint8 r;
	CLR_NZC;
	CC = CC | (t & CC_C);
	r = (t & 0x80) | (t >> 1);
	// H is undefined
	SET_NZ8(r);
	//SET_H(t, t, r);
	return r;
}

inline void MC6809::ASL_MEM(uint8 t)
{
	uint16 r, tt;
	tt = (uint16)t & 0x00ff;
	r = tt << 1;
	CLR_NZVC;
	SET_FLAGS8(tt, tt, r);
	//SET_H(tt, tt, r);
	WM(EAD, (uint8)r);
}

inline uint8 MC6809::ASL_REG(uint8 t)
{
	uint16 r, tt;
	tt = (uint16)t & 0x00ff;
	r = tt << 1;
	CLR_NZVC;
	SET_FLAGS8(tt, tt, r);
	//SET_H(tt, tt, r);
	return (uint8)r;
}

inline void MC6809::ROL_MEM(uint8 t)
{
	uint16 r, tt;
	tt = (uint16)t & 0x00ff;
	r = (CC & CC_C) | (tt << 1);
	CLR_NZVC;
	//SET_NZ8(r);
	//if(t & 0x80) {
	//	SEC;
	//	if((r & 0x80) == 0)SEV;
	//} else {
	//	if((r & 0x80) != 0) SEV;
	//}	  
	SET_FLAGS8(tt, tt, r);
	WM(EAD, (uint8)r);
}

inline uint8 MC6809::ROL_REG(uint8 t)
{
	uint16 r, tt;
	tt = (uint16)t & 0x00ff;
	r = (CC & CC_C) | (tt << 1);
	CLR_NZVC;
	//SET_NZ8(r);
	//if(t & 0x80) {
	//	SEC;
	//	if((r & 0x80) == 0) SEV;
	//} else {
	//	if((r & 0x80) != 0) SEV;
	//}	  
	SET_FLAGS8(tt, tt, r);
	return (uint8)r;
}

inline void MC6809::DEC_MEM(uint8 t)
{
	uint16 tt;
	tt = t - 1;
	CLR_NZV;
	SET_FLAGS8D(tt);
	WM(EAD, tt);
}

inline uint8 MC6809::DEC_REG(uint8 t)
{
	uint8 tt;
	tt = t - 1;
	CLR_NZV;
	SET_FLAGS8D(tt);
	return tt;
}

inline void MC6809::DCC_MEM(uint8 t)
{
	uint16 tt, ss;
	tt = t - 1;
	CLR_NZVC;
	SET_FLAGS8D(tt);
	ss = CC;
	ss >>= 2;
	ss = ~ss;
	ss = ss & CC_C;
	CC = ss | CC;
	WM(EAD, tt);
}

inline uint8 MC6809::DCC_REG(uint8 t)
{
	uint16 tt, ss;
	tt = t - 1;
	CLR_NZVC;
	SET_FLAGS8D(tt);
	ss = CC;
	ss >>= 2;
	ss = ~ss;
	ss = ss & CC_C;
	CC = ss | CC;
	return (uint8)tt;
}

inline void MC6809::INC_MEM(uint8 t)
{
	uint16 tt = t + 1;
	CLR_NZV;
	SET_FLAGS8I(tt);
	WM(EAD, tt);
}

inline uint8 MC6809::INC_REG(uint8 t)
{
	uint16 tt = t + 1;
	CLR_NZV;
	SET_FLAGS8I(tt);
	return (uint8)tt;
}

inline void MC6809::TST_MEM(uint8 t)
{
	CLR_NZV;
	SET_NZ8(t);
}

inline uint8 MC6809::TST_REG(uint8 t)
{
	CLR_NZV;
	SET_NZ8(t);
	return t;
}

inline uint8 MC6809::CLC_REG(uint8 t)
{
	uint8 r;
	r = 0;
	CLR_NZV;
	SEZ;
	return r;
}
  

inline void MC6809::CLR_MEM(uint8 t)
{
	WM(EAD, 0);
	CLR_NZVC;
	SEZ;
}

inline uint8 MC6809::CLR_REG(uint8 t)
{
	CLR_NZVC;
	SEZ;
	return 0;
}

inline uint8 MC6809::SUB8_REG(uint8 reg, uint8 data)
{
	uint16 r;
	r = (uint16)reg - (uint16)data;
	CLR_HNZVC;
	// H is undefined
	SET_FLAGS8(reg, data, r);
	return (uint8)r;
}

inline uint8 MC6809::CMP8_REG(uint8 reg, uint8 data)
{
	uint16 r;
	r = (uint16)reg - (uint16)data;
	CLR_NZVC;
	// H is undefined
	SET_FLAGS8(reg, data, r);
	return reg;
}

inline uint8 MC6809::SBC8_REG(uint8 reg, uint8 data)
{
	uint16 r;
	uint8 cc_c = CC & CC_C;
	r = (uint16)reg - (uint16)data - (uint16)cc_c;
	CLR_HNZVC;
	SET_FLAGS8(reg, data + cc_c , r);
	return (uint8)r;
}

inline uint8 MC6809::AND8_REG(uint8 reg, uint8 data)
{
	uint8 r = reg;
	r &= data;
	CLR_NZV;
	SET_NZ8(r);
	return r;
}

inline uint8 MC6809::BIT8_REG(uint8 reg, uint8 data)
{
	uint16 r;
	r = reg & data;
	CLR_NZV;
	SET_NZ8(r);
	SET_V8(reg, data, r);
	return reg;
}

inline uint8 MC6809::EOR8_REG(uint8 reg, uint8 data)
{
	uint8 r = reg;
	r ^= data;
	CLR_NZV;
	SET_NZ8(r);
	return r;
}

inline uint8 MC6809::OR8_REG(uint8 reg, uint8 data)
{
	uint8 r = reg;
	r |= data;
	CLR_NZV;
	SET_NZ8(r);
	return r;
}

inline uint8 MC6809::ADD8_REG(uint8 reg, uint8 data)
{
	uint16 t, r;
	t = (uint16) data;
	t &= 0x00ff;
	r = reg + t;
	CLR_HNZVC;
	SET_HNZVC8(reg, t, r);
	return (uint8)r;
}

inline uint8 MC6809::ADC8_REG(uint8 reg, uint8 data)
{
	uint16 t, r;
	uint8 c_cc = CC & CC_C;
	t = (uint16) data;
	t &= 0x00ff;
	r = reg + t + c_cc;
	CLR_HNZVC;
	SET_HNZVC8(reg, t + c_cc, r);
	return (uint8)r;
}	

inline uint8 MC6809::LOAD8_REG(uint8 reg)
{
	CLR_NZV;
	SET_NZ8(reg);
	return reg;
}

inline void MC6809::STORE8_REG(uint8 reg)
{
	CLR_NZV;
	SET_NZ8(reg);
	WM(EAD, reg);
}

inline uint16 MC6809::LOAD16_REG(uint16 reg)
{
	CLR_NZV;
	SET_NZ16(reg);
	return reg;
}
  

inline uint16 MC6809::SUB16_REG(uint16 reg, uint16 data)
{
	uint32 r, d;
	d = reg;
	r = d - data;
	CLR_NZVC;
	SET_FLAGS16(d, data, r);
	return (uint16)r;
}

inline uint16 MC6809::ADD16_REG(uint16 reg, uint16 data)
{
	uint32 r, d;
	d = reg;
	r = d + (uint32)data;
	CLR_HNZVC;
	SET_HNZVC16(d, data, r);
	return (uint16)r;
}

inline uint16 MC6809::CMP16_REG(uint16 reg, uint16 data)
{
	uint32 r, d;
	d = reg;
	r = d - data;
	CLR_NZVC;
	SET_FLAGS16(d, data, r);
	return reg;
}

inline void MC6809::STORE16_REG(pair *p)
{
	CLR_NZV;
	SET_NZ16(p->w.l);
	WM16(EAD, p);
}


/* $00 NEG direct ?**** */
OP_HANDLER(neg_di) {
	uint8 t;
	DIRBYTE(t);
	NEG_MEM(t);
}

/* $01 Undefined Neg */
/* $03 COM direct -**01 */
OP_HANDLER(com_di) {
	uint8 t;
	DIRBYTE(t);
	COM_MEM(t);
}

/* $02 NGC Direct (Undefined) */
OP_HANDLER(ngc_di) {
	if ((CC & CC_C) == 0) {
		neg_di();
	}
	else {
		com_di();
	}
}


/* $04 LSR direct -0*-* */
OP_HANDLER(lsr_di) {
	uint8 t;
	DIRBYTE(t);
	LSR_MEM(t);
}

/* $05 ILLEGAL */

/* $06 ROR direct -**-* */
OP_HANDLER(ror_di) {
	uint8 t;
	DIRBYTE(t);
	ROR_MEM(t);
}

/* $07 ASR direct ?**-* */
OP_HANDLER(asr_di) {
	uint8 t;
	DIRBYTE(t);
	ASR_MEM(t);
}

/* $08 ASL direct ?**** */
OP_HANDLER(asl_di) {
	uint8 t;
	DIRBYTE(t);
	ASL_MEM(t);
}

/* $09 ROL direct -**** */
OP_HANDLER(rol_di) {
	uint8 t;
	DIRBYTE(t);
	ROL_MEM(t);
}

/* $0A DEC direct -***- */
OP_HANDLER(dec_di) {
	uint8 t;
	DIRBYTE(t);
	DEC_MEM(t);
}

/* $0B DCC direct */
OP_HANDLER(dcc_di) {
	uint8 t;
	DIRBYTE(t);
	DCC_MEM(t);
}


/* $OC INC direct -***- */
OP_HANDLER(inc_di) {
	uint8 t;
	DIRBYTE(t);
	INC_MEM(t);
}

/* $OD TST direct -**0- */
OP_HANDLER(tst_di) {
	uint8 t;
	DIRBYTE(t);
	t = RM(EAD);
	TST_MEM(t);
}

/* $0E JMP direct ----- */
OP_HANDLER(jmp_di) {
	DIRECT;
	PC = EA;
}

/* $0F CLR direct -0100 */
OP_HANDLER(clr_di) {
	uint8 dummy;
	DIRECT;
	dummy = RM(EAD);	// Dummy Read(Alpha etc...)
	CLR_MEM(dummy);
}

/* $10 FLAG */

/* $11 FLAG */

/* $12 NOP inherent ----- */
OP_HANDLER(nop) {
	;
}

/* $13 SYNC inherent ----- */
OP_HANDLER(sync_09)	// Rename 20101110
{
	int_state |= MC6809_SYNC_IN;
}



/* $14 trap(HALT) */
OP_HANDLER(trap) {
	int_state |= MC6809_INSN_HALT;	// HALTフラグ
	// Debug: トラップ要因
	emu->out_debug_log("MC6809 : TRAP(HALT) @%04x %02x %02x\n", PC - 1, RM(PC - 1), RM(PC));
}

/* $15 trap */

/* $16 LBRA relative ----- */
OP_HANDLER(lbra) {
	LBRANCH(true);
}

/* $17 LBSR relative ----- */
OP_HANDLER(lbsr) {
	IMMWORD(EAP);
	PUSHWORD(pPC);
	PC += EAD;
}

/* $18 ASLCC */

OP_HANDLER(aslcc_in) {
	uint8 cc_r = CC;
	if ((cc_r & CC_Z) != 0x00) { //20100824 Fix
		cc_r |= CC_C;
	}
	cc_r <<= 1;
	cc_r &= 0x3e;
	CC = cc_r;
}

/* $19 DAA inherent (A) -**0* */
OP_HANDLER(daa) {
	uint8 msn, lsn;
	uint16 t, cf = 0;
	msn = A & 0xf0;
	lsn = A & 0x0f;
	if (lsn > 0x09 || CC & CC_H)
		cf |= 0x06;
	if (msn > 0x80 && lsn > 0x09)
		cf |= 0x60;
	if (msn > 0x90 || CC & CC_C)
		cf |= 0x60;
	t = cf + A;
	CLR_NZV;	/* keep carry from previous operation */
	SET_NZ8((uint8) t);
	SET_C8(t);
	A = (uint8)t;
}


/* $1A ORCC immediate ##### */
OP_HANDLER(orcc) {
	uint8 t;
	IMMBYTE(t);
	CC |= t;
}

/* $1B ILLEGAL */


/* $1C ANDCC immediate ##### */
OP_HANDLER(andcc) {
	uint8 t;
	IMMBYTE(t);
	CC &= t;
//  check_irq_lines(); /* HJB 990116 */
}

/* $1D SEX inherent -**-- */
OP_HANDLER(sex) {
	uint16 t;
	t = SIGNED(B);
	D = t; // Endian OK?
	//  CLR_NZV;    Tim Lindner 20020905: verified that V flag is not affected
	CLR_NZ;
	SET_NZ16(t);
}

	/* $1E EXG inherent ----- */// 20100825
OP_HANDLER(exg) {
	pair t1, t2;
	uint8 tb;
	IMMBYTE(tb);
	t1.d = 0;
	t2.d = 0;
	/*
	 * 20111011: 16bit vs 16Bitの演算にする(XM7/ cpu_x86.asmより
	 */
	{
		switch ((tb >> 4) & 15) {
			case 0:
				t1.w.l = D;
				break;
			case 1:
				t1.w.l = X;
				break;
			case 2:
				t1.w.l = Y;
				break;
			case 3:
				t1.w.l = U;
				break;
			case 4:
				t1.w.l = S;
				break;
			case 5:
				t1.w.l = PC;
				break;
			case 8:
				t1.b.l = A;
				t1.b.h = 0xff;
				break;
			case 9:
				t1.b.l = B;
				t1.b.h = 0xff;
				break;
			case 10:
				t1.b.l = CC;
				t1.b.h = 0xff;
				break;
			case 11:
				t1.b.l = DP;
				t1.b.h = 0xff;
				break;
			default:
				t1.w.l = 0xffff;
				break;
		}
		switch (tb & 15) {
			case 0:
				t2.w.l = D;
				break;
			case 1:
				t2.w.l = X;
				break;
			case 2:
				t2.w.l = Y;
				break;
			case 3:
				t2.w.l = U;
				break;
			case 4:
				t2.w.l = S;
				break;
			case 5:
				t2.w.l = PC;
				break;
			case 8:
				t2.b.l = A;
				t2.b.h = 0xff;
				break;
			case 9:
				t2.b.l = B;
				t2.b.h = 0xff;
				break;
			case 10:
				t2.b.l = CC;
				t2.b.h = 0xff;
				break;
			case 11:
				t2.b.l = DP;
				t2.b.h = 0xff;
				break;
			default:
				t2.w.l = 0xffff;
				break;
		}
	}
	switch ((tb >> 4) & 15) {
		case 0:
			D = t2.w.l;
			break;
		case 1:
			X = t2.w.l;
			break;
		case 2:
			Y = t2.w.l;
			break;
		case 3:
			U = t2.w.l;
			break;
		case 4:
			S = t2.w.l;
			int_state |= MC6809_LDS;
			break;
		case 5:
			PC = t2.w.l;
			break;
		case 8:
			A = t2.b.l;
			break;
		case 9:
			B = t2.b.l;
			break;
		case 10:
			CC = t2.b.l;
			break;
		case 11:
			DP = t2.b.l;
			break;
	}
	switch (tb & 15) {
		case 0:
			D = t1.w.l;
			break;
		case 1:
			X = t1.w.l;
			break;
		case 2:
			Y = t1.w.l;
			break;
		case 3:
			U = t1.w.l;
			break;
		case 4:
			S = t1.w.l;
			int_state |= MC6809_LDS;
			break;
		case 5:
			PC = t1.w.l;
			break;
		case 8:
			A = t1.b.l;
			break;
		case 9:
			B = t1.b.l;
			break;
		case 10:
			CC = t1.b.l;
			break;
		case 11:
			DP = t1.b.l;
			break;
	}
}

/* $1F TFR inherent ----- */
OP_HANDLER(tfr) {
	uint8 tb;
	pair t;
	IMMBYTE(tb);
	t.d = 0;
	/*
	 * 20111011: 16bit vs 16Bitの演算にする(XM7/ cpu_x86.asmより)
	 */
	{
		switch ((tb >> 4) & 15) {
			case 0:
				t.w.l = D;
				break;
			case 1:
				t.w.l = X;
				break;
			case 2:
				t.w.l = Y;
				break;
			case 3:
				t.w.l = U;
				break;
			case 4:
				t.w.l = S;
				break;
			case 5:
				t.w.l = PC;
				break;
			case 8:
				t.b.l = A;
				t.b.h = 0xff;
				break;
			case 9:
				t.b.l = B;
				t.b.h = 0xff;
				break;
			case 10:
				t.b.l = CC;
				t.b.h = 0xff;
				break;
			case 11:
				t.b.l = DP;
				t.b.h = 0xff;
				break;
			default:
				t.w.l = 0xffff;
				break;
		}
	}
	switch (tb & 15) {
		case 0:
			D = t.w.l;
			break;
		case 1:
			X = t.w.l;
			break;
		case 2:
			Y = t.w.l;
			break;
		case 3:
			U = t.w.l;
			break;
		case 4:
			S = t.w.l;
			int_state |= MC6809_LDS;
			break;
		case 5:
			PC = t.w.l;
			break;
		case 8:
			A = t.b.l;
			break;
		case 9:
			B = t.b.l;
			break;
		case 10:
			CC = t.b.l;
			break;
		case 11:
			DP = t.b.l;
			break;
	}
}

/* $20 BRA relative ----- */
OP_HANDLER(bra) {
	BRANCH(true);
}

/* $21 BRN relative ----- */
OP_HANDLER(brn) {
	BRANCH(false);
}

/* $1021 LBRN relative ----- */
OP_HANDLER(lbrn) {
	LBRANCH(false);
}

/* $22 BHI relative ----- */
OP_HANDLER(bhi) {
	BRANCH(((CC & (CC_Z | CC_C)) == 0));
}

/* $1022 LBHI relative ----- */
OP_HANDLER(lbhi) {
	LBRANCH(((CC & (CC_Z | CC_C)) == 0));
}

/* $23 BLS relative ----- */
OP_HANDLER(bls) {
	BRANCH(((CC & (CC_Z | CC_C)) != 0));
}

/* $1023 LBLS relative ----- */
OP_HANDLER(lbls) {
	LBRANCH(((CC & (CC_Z | CC_C)) != 0));
	//LBRANCH((CC & (CC_Z | CC_C)));
}

/* $24 BCC relative ----- */
OP_HANDLER(bcc) {
	BRANCH((CC & CC_C) == 0);
}

/* $1024 LBCC relative ----- */
OP_HANDLER(lbcc) {
	LBRANCH((CC & CC_C) == 0);
}

/* $25 BCS relative ----- */
OP_HANDLER(bcs) {
	BRANCH((CC & CC_C) != 0);
}

/* $1025 LBCS relative ----- */
OP_HANDLER(lbcs) {
	LBRANCH((CC & CC_C) != 0);
}

/* $26 BNE relative ----- */
OP_HANDLER(bne) {
	BRANCH((CC & CC_Z) == 0);
}

/* $1026 LBNE relative ----- */
OP_HANDLER(lbne) {
	LBRANCH((CC & CC_Z) == 0);
}

/* $27 BEQ relative ----- */
OP_HANDLER(beq) {
	BRANCH((CC & CC_Z) != 0);
}

/* $1027 LBEQ relative ----- */
OP_HANDLER(lbeq) {
	LBRANCH((CC & CC_Z) != 0);
}

/* $28 BVC relative ----- */
OP_HANDLER(bvc) {
	BRANCH((CC & CC_V) == 0);
}

/* $1028 LBVC relative ----- */
OP_HANDLER(lbvc) {
	LBRANCH((CC & CC_V) == 0);
}

/* $29 BVS relative ----- */
OP_HANDLER(bvs) {
	BRANCH((CC & CC_V) != 0);
}

/* $1029 LBVS relative ----- */
OP_HANDLER(lbvs) {
	LBRANCH((CC & CC_V) != 0);
}

/* $2A BPL relative ----- */
OP_HANDLER(bpl) {
	BRANCH((CC & CC_N) == 0);
}

/* $102A LBPL relative ----- */
OP_HANDLER(lbpl) {
	LBRANCH((CC & CC_N) == 0);
}

/* $2B BMI relative ----- */
OP_HANDLER(bmi) {
	BRANCH((CC & CC_N) != 0);
}

/* $102B LBMI relative ----- */
OP_HANDLER(lbmi) {
	LBRANCH((CC & CC_N) != 0);
}

/* $2C BGE relative ----- */
OP_HANDLER(bge) {
	BRANCH(!NXORV);
}

/* $102C LBGE relative ----- */
OP_HANDLER(lbge) {
	LBRANCH(!NXORV);
}

/* $2D BLT relative ----- */
OP_HANDLER(blt) {
	BRANCH(NXORV);
}

/* $102D LBLT relative ----- */
OP_HANDLER(lblt) {
	LBRANCH(NXORV);
}

/* $2E BGT relative ----- */
OP_HANDLER(bgt) {
	BRANCH(!(NXORV || (CC & CC_Z)));
}

/* $102E LBGT relative ----- */
OP_HANDLER(lbgt) {
	LBRANCH(!(NXORV || (CC & CC_Z)));
}

/* $2F BLE relative ----- */
OP_HANDLER(ble) {
	BRANCH((NXORV || (CC & CC_Z)));
}

/* $102F LBLE relative ----- */
OP_HANDLER(lble) {
	LBRANCH((NXORV || (CC & CC_Z)));
}

/* $30 LEAX indexed --*-- */
OP_HANDLER(leax) {
	fetch_effective_address();
	X = EA;
	CLR_Z;
	SET_Z16(X);
}

/* $31 LEAY indexed --*-- */
OP_HANDLER(leay) {
	fetch_effective_address();
	Y = EA;
	CLR_Z;
	SET_Z16(Y);
}

/* $32 LEAS indexed ----- */
OP_HANDLER(leas) {
	fetch_effective_address();
	S = EA;
	int_state |= MC6809_LDS;
}

/* $33 LEAU indexed ----- */
OP_HANDLER(leau) {
	fetch_effective_address();
	U = EA;
}

/* $34 PSHS inherent ----- */
OP_HANDLER(pshs) {
		uint8 t, dmy;
		IMMBYTE(t);
		dmy = RM(S);	// Add 20100825
		if (t & 0x80) {
			PUSHWORD(pPC);
			icount -= 2;
		}
		if (t & 0x40) {
			PUSHWORD(pU);
			icount -= 2;
		}
		if (t & 0x20) {
			PUSHWORD(pY);
			icount -= 2;
		}
		if (t & 0x10) {
			PUSHWORD(pX);
			icount -= 2;
		}
		if (t & 0x08) {
			PUSHBYTE(DP);
			icount -= 1;
		}
		if (t & 0x04) {
			PUSHBYTE(B);
			icount -= 1;
		}
		if (t & 0x02) {
			PUSHBYTE(A);
			icount -= 1;
		}
		if (t & 0x01) {
			PUSHBYTE(CC);
			icount -= 1;
		}
	}

/* 35 PULS inherent ----- */
OP_HANDLER(puls) {
		uint8 t, dmy;
		IMMBYTE(t);
		if (t & 0x01) {
			PULLBYTE(CC);
			icount -= 1;
		}
		if (t & 0x02) {
			PULLBYTE(A);
			icount -= 1;
		}
		if (t & 0x04) {
			PULLBYTE(B);
			icount -= 1;
		}
		if (t & 0x08) {
			PULLBYTE(DP);
			icount -= 1;
		}
		if (t & 0x10) {
			PULLWORD(pX);
			icount -= 2;
		}
		if (t & 0x20) {
			PULLWORD(pY);
			icount -= 2;
		}
		if (t & 0x40) {
			PULLWORD(pU);
			icount -= 2;
		}
		if (t & 0x80) {
			PULLWORD(pPC);
			icount -= 2;
		}
		dmy = RM(S);	// Add 20100825

		/* HJB 990225: moved check after all PULLs */
//  if( t&0x01 ) { check_irq_lines(); }
	}

/* $36 PSHU inherent ----- */
OP_HANDLER(pshu) {
		uint8 t, dmy;
		IMMBYTE(t);
		dmy = RM(U);	// Add 20100825
		if (t & 0x80) {
			PSHUWORD(pPC);
			icount -= 2;
		}
		if (t & 0x40) {
			PSHUWORD(pS);
			icount -= 2;
		}
		if (t & 0x20) {
			PSHUWORD(pY);
			icount -= 2;
		}
		if (t & 0x10) {
			PSHUWORD(pX);
			icount -= 2;
		}
		if (t & 0x08) {
			PSHUBYTE(DP);
			icount -= 1;
		}
		if (t & 0x04) {
			PSHUBYTE(B);
			icount -= 1;
		}
		if (t & 0x02) {
			PSHUBYTE(A);
			icount -= 1;
		}
		if (t & 0x01) {
			PSHUBYTE(CC);
			icount -= 1;
		}
	}

/* 37 PULU inherent ----- */
OP_HANDLER(pulu) {
		uint8 t, dmy;
		IMMBYTE(t);
		if (t & 0x01) {
			PULUBYTE(CC);
			icount -= 1;
		}
		if (t & 0x02) {
			PULUBYTE(A);
			icount -= 1;
		}
		if (t & 0x04) {
			PULUBYTE(B);
			icount -= 1;
		}
		if (t & 0x08) {
			PULUBYTE(DP);
			icount -= 1;
		}
		if (t & 0x10) {
			PULUWORD(pX);
			icount -= 2;
		}
		if (t & 0x20) {
			PULUWORD(pY);
			icount -= 2;
		}
		if (t & 0x40) {
			PULUWORD(pS);
			icount -= 2;
		}
		if (t & 0x80) {
			PULUWORD(pPC);
			icount -= 2;
		}
		dmy = RM(U);	// Add 20100825

		/* HJB 990225: moved check after all PULLs */
		//if( t&0x01 ) { check_irq_lines(); }
}

/* $38 ILLEGAL */

/* $39 RTS inherent ----- */
OP_HANDLER(rts) {
	//printf("RTS: Before PC=%04x", pPC.w.l);
	PULLWORD(pPC);
	//printf(" After PC=%04x\n", pPC.w.l);
}

/* $3A ABX inherent ----- */
OP_HANDLER(abx) {
	pair bt;
	bt.d = 0;
	bt.b.l = B;
	X = X + bt.w.l;
}

/* $3B RTI inherent ##### */
OP_HANDLER(rti) {
		PULLBYTE(CC);
//  t = CC & CC_E;    /* HJB 990225: entire state saved? */
	if ((CC & CC_E) != 0) {	// NMIIRQ
		icount -= 9;
		PULLBYTE(A);
		PULLBYTE(B);
		PULLBYTE(DP);
		PULLWORD(pX);
		PULLWORD(pY);
		PULLWORD(pU);
	}
	PULLWORD(pPC);
//  check_irq_lines(); /* HJB 990116 */
}

/* $3C CWAI inherent ----1 */
OP_HANDLER(cwai) {
	uint8 t;
	IMMBYTE(t);
	CC = CC & t;
	CC |= CC_E;	/* HJB 990225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);

	int_state = int_state | MC6809_CWAI_IN;
	int_state &= ~MC6809_CWAI_OUT;	// 0xfeff
	return;
}

/* $3D MUL inherent --*-@ */
OP_HANDLER(mul) {
	pair t, r;
	t.d = 0;
	r.d = 0;
	t.b.l = A;
	r.b.l = B;
	t.d = t.d * r.d;
	CLR_ZC;
	SET_Z16(t.w.l);
	if (t.b.l & 0x80) SEC;
	A = t.b.h;
	B = t.b.l;
}

/* $3E RST */
OP_HANDLER(rst) {
	this->reset();
}


/* $3F SWI (SWI2 SWI3) absolute indirect ----- */
OP_HANDLER(swi) {
		CC |= CC_E;	/* HJB 980225: save entire state */
		PUSHWORD(pPC);
		PUSHWORD(pU);
		PUSHWORD(pY);
		PUSHWORD(pX);
		PUSHBYTE(DP);
		PUSHBYTE(B);
		PUSHBYTE(A);
		PUSHBYTE(CC);
		CC |= CC_IF | CC_II;	/* inhibit FIRQ and IRQ */
		pPC = RM16_PAIR(0xfffa);
}

/* $103F SWI2 absolute indirect ----- */
OP_HANDLER(swi2) {
		CC |= CC_E;	/* HJB 980225: save entire state */
		PUSHWORD(pPC);
		PUSHWORD(pU);
		PUSHWORD(pY);
		PUSHWORD(pX);
		PUSHBYTE(DP);
		PUSHBYTE(B);
		PUSHBYTE(A);
		PUSHBYTE(CC);
		pPC = RM16_PAIR(0xfff4);
}

/* $113F SWI3 absolute indirect ----- */
OP_HANDLER(swi3) {
		CC |= CC_E;	/* HJB 980225: save entire state */
		PUSHWORD(pPC);
		PUSHWORD(pU);
		PUSHWORD(pY);
		PUSHWORD(pX);
		PUSHBYTE(DP);
		PUSHBYTE(B);
		PUSHBYTE(A);
		PUSHBYTE(CC);
		pPC = RM16_PAIR(0xfff2);
}

/* $40 NEGA inherent ?**** */
OP_HANDLER(nega) {
	A = NEG_REG(A);
}

/* $41 NEGA */


/* $43 COMA inherent -**01 */
OP_HANDLER(coma) {
	A = COM_REG(A);
}

/* $42 NGCA */
OP_HANDLER(ngca) {
	if ((CC & CC_C) == 0) {
		nega();
	} else {
		coma();
	}
}

/* $44 LSRA inherent -0*-* */
OP_HANDLER(lsra) {
	A = LSR_REG(A);
}

/* $45 LSRA */

/* $46 RORA inherent -**-* */
OP_HANDLER(rora) {
	A = ROR_REG(A);
}

/* $47 ASRA inherent ?**-* */
OP_HANDLER(asra) {
	A = ASR_REG(A);
}

/* $48 ASLA inherent ?**** */
OP_HANDLER(asla) {
	A = ASL_REG(A);
}

/* $49 ROLA inherent -**** */
OP_HANDLER(rola) {
	A = ROL_REG(A);
}

/* $4A DECA inherent -***- */
OP_HANDLER(deca) {
	A = DEC_REG(A);
}


/* $4B DCCA */
OP_HANDLER(dcca) {
	A = DCC_REG(A);
}

/* $4C INCA inherent -***- */
OP_HANDLER(inca) {
	A = INC_REG(A);
}

/* $4D TSTA inherent -**0- */
OP_HANDLER(tsta) {
	A = TST_REG(A);
}

/* $4E ILLEGAL */
OP_HANDLER(clca) {
	A = CLC_REG(A);
}

/* $4F CLRA inherent -0100 */
OP_HANDLER(clra) {
	A = CLR_REG(A);
}

/* $50 NEGB inherent ?**** */
OP_HANDLER(negb) {
	B = NEG_REG(B);
}

/* $51 NEGB */

/* $52 NGCB */

/* $53 COMB inherent -**01 */
OP_HANDLER(comb) {
	B = COM_REG(B);
}

/* $52 NGCB */
OP_HANDLER(ngcb) {
	if ((CC & CC_C) == 0) {
		negb();
	} else {
		comb();
	}
}

/* $54 LSRB inherent -0*-* */
OP_HANDLER(lsrb) {
	B = LSR_REG(B);
}

/* $55 LSRB */

/* $56 RORB inherent -**-* */
OP_HANDLER(rorb) {
	B = ROR_REG(B);
}

/* $57 ASRB inherent ?**-* */
OP_HANDLER(asrb) {
	B = ASR_REG(B);
}

/* $58 ASLB inherent ?**** */
OP_HANDLER(aslb) {
	B = ASL_REG(B);
}

/* $59 ROLB inherent -**** */
OP_HANDLER(rolb) {
	B = ROL_REG(B);
}

/* $5A DECB inherent -***- */
OP_HANDLER(decb) {
	B = DEC_REG(B);
}

/* $5B DCCB */
OP_HANDLER(dccb) {
	B = DCC_REG(B);
}

/* $5C INCB inherent -***- */
OP_HANDLER(incb) {
	B = INC_REG(B);
}

/* $5D TSTB inherent -**0- */
OP_HANDLER(tstb) {
	B = TST_REG(B);
}

/* $5E ILLEGAL */
OP_HANDLER(clcb) {
	B = CLC_REG(B);
}

/* $5F CLRB inherent -0100 */
OP_HANDLER(clrb) {
	B = CLR_REG(B);
}

/* $60 NEG indexed ?**** */
OP_HANDLER(neg_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	NEG_MEM(t);
}

/* $61 ILLEGAL */


/* $63 COM indexed -**01 */
OP_HANDLER(com_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	COM_MEM(t);
}

/* $62 ILLEGAL */
OP_HANDLER(ngc_ix) {
	if ((CC & CC_C) == 0) {
		neg_ix();
	} else {
		com_ix();
	}
}

/* $64 LSR indexed -0*-* */
OP_HANDLER(lsr_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	LSR_MEM(t);
}

/* $65 ILLEGAL */

/* $66 ROR indexed -**-* */
OP_HANDLER(ror_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	ROR_MEM(t);
}

/* $67 ASR indexed ?**-* */
OP_HANDLER(asr_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	ASR_MEM(t);
}

/* $68 ASL indexed ?**** */
OP_HANDLER(asl_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	ASL_MEM(t);
}

/* $69 ROL indexed -**** */
OP_HANDLER(rol_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	ROL_MEM(t);
}

/* $6A DEC indexed -***- */
OP_HANDLER(dec_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	DEC_MEM(t);
}

/* $6B DCC index */
OP_HANDLER(dcc_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	DCC_MEM(t);
}

/* $6C INC indexed -***- */
OP_HANDLER(inc_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	INC_MEM(t);
}

/* $6D TST indexed -**0- */
OP_HANDLER(tst_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	TST_MEM(t);
}

/* $6E JMP indexed ----- */
OP_HANDLER(jmp_ix) {
	fetch_effective_address();
	PCD = EAD;
}

/* $6F CLR indexed -0100 */
OP_HANDLER(clr_ix) {
	uint8 t, dummy;
	t = GET_INDEXED_DATA();
	dummy = RM(EAD);	// Dummy Read(Alpha etc...)
	CLR_MEM(t);
}

/* $70 NEG extended ?**** */
OP_HANDLER(neg_ex) {
	uint8 t;
	EXTBYTE(t);
	NEG_MEM(t);
}


/* $73 COM extended -**01 */
OP_HANDLER(com_ex) {
	uint8 t;
	EXTBYTE(t);
	COM_MEM(t);
}

/* $72 NGC extended */
OP_HANDLER(ngc_ex) {
	if ((CC & CC_C) == 0) {
		neg_ex();
	} else {
		com_ex();
	}
}

/* $74 LSR extended -0*-* */
OP_HANDLER(lsr_ex) {
	uint8 t;
	EXTBYTE(t);
	LSR_MEM(t);
}

/* $75 ILLEGAL */

/* $76 ROR extended -**-* */
OP_HANDLER(ror_ex) {
	uint8 t;
	EXTBYTE(t);
	ROR_MEM(t);
}

/* $77 ASR extended ?**-* */
OP_HANDLER(asr_ex) {
	uint8 t;
	EXTBYTE(t);
	ASR_MEM(t);
}

/* $78 ASL extended ?**** */
OP_HANDLER(asl_ex) {
	uint8 t;
	EXTBYTE(t);
	ASL_MEM(t);
}

/* $79 ROL extended -**** */
OP_HANDLER(rol_ex) {
	uint8 t;
	EXTBYTE(t);
	ROL_MEM(t);
}

/* $7A DEC extended -***- */
OP_HANDLER(dec_ex) {
	uint8 t;
	EXTBYTE(t);
	DEC_MEM(t);
}

/* $7B ILLEGAL */
/* $6B DCC index */
OP_HANDLER(dcc_ex) {
	uint8 t;
	EXTBYTE(t);
	DCC_MEM(t);
}

/* $7C INC extended -***- */
OP_HANDLER(inc_ex) {
	uint8 t;
	EXTBYTE(t);
	INC_MEM(t);
}

/* $7D TST extended -**0- */
OP_HANDLER(tst_ex) {
	uint8 t;
	EXTBYTE(t);
	TST_MEM(t);
}

/* $7E JMP extended ----- */
OP_HANDLER(jmp_ex) {
	EXTENDED;
	PCD = EAD;
}

/* $7F CLR extended -0100 */
OP_HANDLER(clr_ex) {
	uint8 dummy;
	EXTENDED;
	dummy = RM(EAD);
	CLR_MEM(dummy);
}

/* $80 SUBA immediate ?**** */
OP_HANDLER(suba_im) {
	uint8 t;
	IMMBYTE(t);
	A = SUB8_REG(A, t);
}

/* $81 CMPA immediate ?**** */
OP_HANDLER(cmpa_im) {
	uint8 t;
	IMMBYTE(t);
	A = CMP8_REG(A, t);
}

/* $82 SBCA immediate ?**** */
OP_HANDLER(sbca_im) {
	uint8 t;
	IMMBYTE(t);
	A = SBC8_REG(A, t);
}

/* $83 SUBD (CMPD CMPU) immediate -**** */
OP_HANDLER(subd_im) {
	pair b;
	IMMWORD(b);
	D = SUB16_REG(D, b.w.l);
}

/* $1083 CMPD immediate -**** */
OP_HANDLER(cmpd_im) {
	pair b;
	IMMWORD(b);
	D = CMP16_REG(D, b.w.l);
}

/* $1183 CMPU immediate -**** */
OP_HANDLER(cmpu_im) {
	pair b;
	IMMWORD(b);
	U = CMP16_REG(U, b.w.l);
}

/* $84 ANDA immediate -**0- */
OP_HANDLER(anda_im) {
	uint8 t;
	IMMBYTE(t);
	A = AND8_REG(A, t);
}

/* $85 BITA immediate -**0- */
OP_HANDLER(bita_im) {
	uint8 t;
	IMMBYTE(t);
	A = BIT8_REG(A, t);
}

/* $86 LDA immediate -**0- */
OP_HANDLER(lda_im) {
	IMMBYTE(A);
	A = LOAD8_REG(A);
}

/* is this a legal instruction? */
/* $87 STA immediate -**0- */
OP_HANDLER(sta_im) {
		CLR_NZV;
		SET_NZ8(A);
		IMM8;
		WM(EAD, A);
	}

/*
 * $87 , $C7: FLAG8
 */
OP_HANDLER(flag8_im) {
		// 20111117
		uint8 t;
		IMMBYTE(t);
		CLR_NZV;
		CC |= CC_N;
	}


/* $88 EORA immediate -**0- */
OP_HANDLER(eora_im) {
		uint8 t;
		IMMBYTE(t);
		A = EOR8_REG(A, t);
}

/* $89 ADCA immediate ***** */
OP_HANDLER(adca_im) {
	uint8 t;
	IMMBYTE(t);
	A = ADC8_REG(A, t);
}

/* $8A ORA immediate -**0- */
OP_HANDLER(ora_im) {
	uint8 t;
	IMMBYTE(t);
	A = OR8_REG(A, t);
}

/* $8B ADDA immediate ***** */
OP_HANDLER(adda_im) {
	uint8 t;
	IMMBYTE(t);
	A = ADD8_REG(A, t);
}

/* $8C CMPX (CMPY CMPS) immediate -**** */
OP_HANDLER(cmpx_im) {
	pair b;
	IMMWORD(b);
	X = CMP16_REG(X, b.w.l);
}

/* $108C CMPY immediate -**** */
OP_HANDLER(cmpy_im) {
	pair b;
	IMMWORD(b);
	Y = CMP16_REG(Y, b.w.l);
}

/* $118C CMPS immediate -**** */
OP_HANDLER(cmps_im) {
	pair b;
	IMMWORD(b);
	S = CMP16_REG(S, b.w.l);
}

/* $8D BSR ----- */
OP_HANDLER(bsr) {
		uint8 t;
		IMMBYTE(t);
		PUSHWORD(pPC);
		PC += SIGNED(t);
	}

/* $8E LDX (LDY) immediate -**0- */
OP_HANDLER(ldx_im) {
	IMMWORD(pX);
	X = LOAD16_REG(X);
}

/* $108E LDY immediate -**0- */
OP_HANDLER(ldy_im) {
	IMMWORD(pY);
	Y = LOAD16_REG(Y);
}

/* is this a legal instruction? */
/* $8F STX (STY) immediate -**0- */
OP_HANDLER(stx_im) {
		CLR_NZV;
		SET_NZ16(X);
		IMM16;
		WM16(EAD, &pX);
	}

/*
 * $8F , $CF: FLAG16
 */
OP_HANDLER(flag16_im) {
		pair t;
		IMMWORD(t);
		CLR_NZV;
		CC |= CC_N;
}


/* is this a legal instruction? */
/* $108F STY immediate -**0- */
OP_HANDLER(sty_im) {
		CLR_NZV;
		SET_NZ16(Y);
		IMM16;
		WM16(EAD, &pY);
	}

/* $90 SUBA direct ?**** */
OP_HANDLER(suba_di) {
	uint8 t;
	DIRBYTE(t);
	A = SUB8_REG(A, t);
}

/* $91 CMPA direct ?**** */
OP_HANDLER(cmpa_di) {
	uint8 t;
	DIRBYTE(t);
	A = CMP8_REG(A, t); 
}

/* $92 SBCA direct ?**** */
OP_HANDLER(sbca_di) {
	uint8 t;
	DIRBYTE(t);
	A = SBC8_REG(A, t);
}

/* $93 SUBD (CMPD CMPU) direct -**** */
OP_HANDLER(subd_di) {
	pair b;
	DIRWORD(b);
	D = SUB16_REG(D, b.w.l);
}

/* $1093 CMPD direct -**** */
OP_HANDLER(cmpd_di) {
	pair b;
	DIRWORD(b);
	D = CMP16_REG(D, b.w.l);
}

/* $1193 CMPU direct -**** */
OP_HANDLER(cmpu_di) {
	pair b;
	DIRWORD(b);
	U = CMP16_REG(U, b.w.l);
}

/* $94 ANDA direct -**0- */
OP_HANDLER(anda_di) {
	uint8 t;
	DIRBYTE(t);
	A = AND8_REG(A, t);
}

/* $95 BITA direct -**0- */
OP_HANDLER(bita_di) {
	uint8 t;
	DIRBYTE(t);
	A = BIT8_REG(A, t);
}

/* $96 LDA direct -**0- */
OP_HANDLER(lda_di) {
	DIRBYTE(A);
	A = LOAD8_REG(A);
}

/* $97 STA direct -**0- */
OP_HANDLER(sta_di) {
	DIRECT;
	STORE8_REG(A);
}

/* $98 EORA direct -**0- */
OP_HANDLER(eora_di) {
	uint8 t;
	DIRBYTE(t);
	A = EOR8_REG(A, t);
}

/* $99 ADCA direct ***** */
OP_HANDLER(adca_di) {
	uint8 t;
	DIRBYTE(t);
	A = ADC8_REG(A, t);
}

/* $9A ORA direct -**0- */
OP_HANDLER(ora_di) {
	uint8 t;
	DIRBYTE(t);
	A = OR8_REG(A, t);
}

/* $9B ADDA direct ***** */
OP_HANDLER(adda_di) {
	uint8 t;
	DIRBYTE(t);
	A = ADD8_REG(A, t);
}

/* $9C CMPX (CMPY CMPS) direct -**** */
OP_HANDLER(cmpx_di) {
	pair b;
	DIRWORD(b);
	X = CMP16_REG(X, b.w.l);
}

/* $109C CMPY direct -**** */
OP_HANDLER(cmpy_di) {
	pair b;
	DIRWORD(b);
	Y = CMP16_REG(Y, b.w.l);
}

/* $119C CMPS direct -**** */
OP_HANDLER(cmps_di) {
	pair b;
	DIRWORD(b);
	S = CMP16_REG(S, b.w.l);
}

/* $9D JSR direct ----- */
OP_HANDLER(jsr_di) {
		DIRECT;
		PUSHWORD(pPC);
		PCD = EAD;
	}

/* $9E LDX (LDY) direct -**0- */
OP_HANDLER(ldx_di) {
	DIRWORD(pX);
	X = LOAD16_REG(X);
}

/* $109E LDY direct -**0- */
OP_HANDLER(ldy_di) {
	DIRWORD(pY);
	Y = LOAD16_REG(Y);
}

/* $9F STX (STY) direct -**0- */
OP_HANDLER(stx_di) {
	DIRECT;
	STORE16_REG(&pX);
}

/* $109F STY direct -**0- */
OP_HANDLER(sty_di) {
	DIRECT;
	STORE16_REG(&pY);
}

/* $a0 SUBA indexed ?**** */
OP_HANDLER(suba_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	A = SUB8_REG(A, t); 
}

/* $a1 CMPA indexed ?**** */
OP_HANDLER(cmpa_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	A = CMP8_REG(A, t); 
}

/* $a2 SBCA indexed ?**** */
OP_HANDLER(sbca_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	A = SBC8_REG(A, t); 
}

/* $a3 SUBD (CMPD CMPU) indexed -**** */
OP_HANDLER(subd_ix) {
	pair b;
	b = GET_INDEXED_DATA16();
	D = SUB16_REG(D, b.w.l);
}

/* $10a3 CMPD indexed -**** */
OP_HANDLER(cmpd_ix) {
	pair b;
	b = GET_INDEXED_DATA16();
	D = CMP16_REG(D, b.w.l);
}

/* $11a3 CMPU indexed -**** */
OP_HANDLER(cmpu_ix) {
	pair b;
	b = GET_INDEXED_DATA16();
	U = CMP16_REG(U, b.w.l);
}

/* $a4 ANDA indexed -**0- */
OP_HANDLER(anda_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	A = AND8_REG(A, t);
}

/* $a5 BITA indexed -**0- */
OP_HANDLER(bita_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	A = BIT8_REG(A, t);
}

/* $a6 LDA indexed -**0- */
OP_HANDLER(lda_ix) {
	A = GET_INDEXED_DATA();
	A = LOAD8_REG(A);
}

/* $a7 STA indexed -**0- */
OP_HANDLER(sta_ix) {
	fetch_effective_address();
	STORE8_REG(A);
}

/* $a8 EORA indexed -**0- */
OP_HANDLER(eora_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	A = EOR8_REG(A, t);
}

/* $a9 ADCA indexed ***** */
OP_HANDLER(adca_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	A = ADC8_REG(A, t);
}

/* $aA ORA indexed -**0- */
OP_HANDLER(ora_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	A = OR8_REG(A, t);
}

/* $aB ADDA indexed ***** */
OP_HANDLER(adda_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	A = ADD8_REG(A, t);
}

/* $aC CMPX (CMPY CMPS) indexed -**** */
OP_HANDLER(cmpx_ix) {
	pair b;
	b = GET_INDEXED_DATA16();
	X = CMP16_REG(X, b.w.l);
}

/* $10aC CMPY indexed -**** */
OP_HANDLER(cmpy_ix) {
	pair b;
	b = GET_INDEXED_DATA16();
	Y = CMP16_REG(Y, b.w.l);
}

/* $11aC CMPS indexed -**** */
OP_HANDLER(cmps_ix) {
	pair b;
	b = GET_INDEXED_DATA16();
	S = CMP16_REG(S, b.w.l);
}

/* $aD JSR indexed ----- */
OP_HANDLER(jsr_ix) {
	fetch_effective_address();
	PUSHWORD(pPC);
	PCD = EAD;
}

/* $aE LDX (LDY) indexed -**0- */
OP_HANDLER(ldx_ix) {
	pair t;
	t = GET_INDEXED_DATA16();
	X = t.w.l;
	X = LOAD16_REG(X);
}

/* $10aE LDY indexed -**0- */
OP_HANDLER(ldy_ix) {
	pair t;
	t = GET_INDEXED_DATA16();
	Y = t.w.l;
	Y = LOAD16_REG(Y);
}

/* $aF STX (STY) indexed -**0- */
OP_HANDLER(stx_ix) {
	fetch_effective_address();
	STORE16_REG(&pX);
}

/* $10aF STY indexed -**0- */
OP_HANDLER(sty_ix) {
	fetch_effective_address();
	STORE16_REG(&pY);
}

/* $b0 SUBA extended ?**** */
OP_HANDLER(suba_ex) {
	uint8 t;
	EXTBYTE(t);
	A = SUB8_REG(A, t);
}

/* $b1 CMPA extended ?**** */
OP_HANDLER(cmpa_ex) {
	uint8 t;
	EXTBYTE(t);
	A = CMP8_REG(A, t);
}

/* $b2 SBCA extended ?**** */
OP_HANDLER(sbca_ex) {
	uint8 t;
	EXTBYTE(t);
	A = SBC8_REG(A, t);
}

/* $b3 SUBD (CMPD CMPU) extended -**** */
OP_HANDLER(subd_ex) {
	pair b;
	EXTWORD(b);
	D = SUB16_REG(D, b.w.l);
}

/* $10b3 CMPD extended -**** */
OP_HANDLER(cmpd_ex) {
	pair b;
	EXTWORD(b);
	D = CMP16_REG(D, b.w.l);
}

/* $11b3 CMPU extended -**** */
OP_HANDLER(cmpu_ex) {
	pair b;
	EXTWORD(b);
	U = CMP16_REG(U, b.w.l);
}

/* $b4 ANDA extended -**0- */
OP_HANDLER(anda_ex) {
	uint8 t;
	EXTBYTE(t);
	A = AND8_REG(A, t);
}

/* $b5 BITA extended -**0- */
OP_HANDLER(bita_ex) {
	uint8 t;
	EXTBYTE(t);
	A = BIT8_REG(A, t);
}

/* $b6 LDA extended -**0- */
OP_HANDLER(lda_ex) {
	EXTBYTE(A);
	A = LOAD8_REG(A);
}

/* $b7 STA extended -**0- */
OP_HANDLER(sta_ex) {
	EXTENDED;
	STORE8_REG(A);
}

/* $b8 EORA extended -**0- */
OP_HANDLER(eora_ex) {
	uint8 t;
	EXTBYTE(t);
	A = EOR8_REG(A, t);
}

/* $b9 ADCA extended ***** */
OP_HANDLER(adca_ex) {
	uint8 t;
	EXTBYTE(t);
	A = ADC8_REG(A, t);
}

/* $bA ORA extended -**0- */
OP_HANDLER(ora_ex) {
	uint8 t;
	EXTBYTE(t);
	A = OR8_REG(A, t);
}

/* $bB ADDA extended ***** */
OP_HANDLER(adda_ex) {
	uint8 t;
	EXTBYTE(t);
	A = ADD8_REG(A, t);
}

/* $bC CMPX (CMPY CMPS) extended -**** */
OP_HANDLER(cmpx_ex) {
	pair b;
	EXTWORD(b);
	X = CMP16_REG(X, b.w.l);
}

/* $10bC CMPY extended -**** */
OP_HANDLER(cmpy_ex) {
	pair b;
	EXTWORD(b);
	Y = CMP16_REG(Y, b.w.l);
}

/* $11bC CMPS extended -**** */
OP_HANDLER(cmps_ex) {
	pair b;
	EXTWORD(b);
	S = CMP16_REG(S, b.w.l);
}

/* $bD JSR extended ----- */
OP_HANDLER(jsr_ex) {
		EXTENDED;
		PUSHWORD(pPC);
		PCD = EAD;
}

/* $bE LDX (LDY) extended -**0- */
OP_HANDLER(ldx_ex) {
	EXTWORD(pX);
	X = LOAD16_REG(X);
}

/* $10bE LDY extended -**0- */
OP_HANDLER(ldy_ex) {
	EXTWORD(pY);
	Y = LOAD16_REG(Y);
}

/* $bF STX (STY) extended -**0- */
OP_HANDLER(stx_ex) {
	EXTENDED;
	STORE16_REG(&pX);
}

/* $10bF STY extended -**0- */
OP_HANDLER(sty_ex) {
	EXTENDED;
	STORE16_REG(&pY);
}

/* $c0 SUBB immediate ?**** */
OP_HANDLER(subb_im) {
	uint8 t;
	IMMBYTE(t);
	B = SUB8_REG(B, t);
}

/* $c1 CMPB immediate ?**** */
OP_HANDLER(cmpb_im) {
	uint8 t;
	IMMBYTE(t);
	B = CMP8_REG(B, t);
}

/* $c2 SBCB immediate ?**** */
OP_HANDLER(sbcb_im) {
	uint8 t;
	IMMBYTE(t);
	B = SBC8_REG(B, t);
}

/* $c3 ADDD immediate -**** */
OP_HANDLER(addd_im) {
	pair b;
	IMMWORD(b);
	D = ADD16_REG(D, b.w.l);
}

/* $c4 ANDB immediate -**0- */
OP_HANDLER(andb_im) {
	uint8 t;
	IMMBYTE(t);
	B = AND8_REG(B, t);
}

/* $c5 BITB immediate -**0- */
OP_HANDLER(bitb_im) {
	uint8 t;
	IMMBYTE(t);
	B = BIT8_REG(B, t);
}

/* $c6 LDB immediate -**0- */
OP_HANDLER(ldb_im) {
	IMMBYTE(B);
	B = LOAD8_REG(B);
}

/* is this a legal instruction? */
/* $c7 STB immediate -**0- */
OP_HANDLER(stb_im) {
	CLR_NZV;
	SET_NZ8(B);
	IMM8;
	WM(EAD, B);
}

/* $c8 EORB immediate -**0- */
OP_HANDLER(eorb_im) {
	uint8 t;
	IMMBYTE(t);
	B = EOR8_REG(B, t);
}

/* $c9 ADCB immediate ***** */
OP_HANDLER(adcb_im) {
	uint8 t;
	IMMBYTE(t);
	B = ADC8_REG(B, t);
}

/* $cA ORB immediate -**0- */
OP_HANDLER(orb_im) {
	uint8 t;
	IMMBYTE(t);
	B = OR8_REG(B, t);
}

/* $cB ADDB immediate ***** */
OP_HANDLER(addb_im) {
	uint8 t;
	IMMBYTE(t);
	B = ADD8_REG(B, t);
}

/* $cC LDD immediate -**0- */
OP_HANDLER(ldd_im) {
	IMMWORD(pD);
	D = LOAD16_REG(D);
}

/* is this a legal instruction? */
/* $cD STD immediate -**0- */
OP_HANDLER(std_im) {
		CLR_NZV;
		SET_NZ16(D);
		IMM16;
		WM(EAD, D);
}

/* $cE LDU (LDS) immediate -**0- */
OP_HANDLER(ldu_im) {
	IMMWORD(pU);
	U = LOAD16_REG(U);
}

/* $10cE LDS immediate -**0- */
OP_HANDLER(lds_im) {
	IMMWORD(pS);
	S = LOAD16_REG(S);
	int_state |= MC6809_LDS;
}

/* is this a legal instruction? */
/* $cF STU (STS) immediate -**0- */
OP_HANDLER(stu_im) {
		CLR_NZV;
		SET_NZ16(U);
		IMM16;
		WM16(EAD, &pU);
	}

/* is this a legal instruction? */
/* $10cF STS immediate -**0- */
OP_HANDLER(sts_im) {
		CLR_NZV;
		SET_NZ16(S);
		IMM16;
		WM16(EAD, &pS);
	}

/* $d0 SUBB direct ?**** */
OP_HANDLER(subb_di) {
	uint8 t;
	DIRBYTE(t);
	B = SUB8_REG(B, t);
}
/* $d1 CMPB direct ?**** */
OP_HANDLER(cmpb_di) {
	uint8 t;
	DIRBYTE(t);
	B = CMP8_REG(B, t);
}

/* $d2 SBCB direct ?**** */
OP_HANDLER(sbcb_di) {
	uint8 t;
	DIRBYTE(t);
	B = SBC8_REG(B, t);
}

/* $d3 ADDD direct -**** */
OP_HANDLER(addd_di) {
	pair b;
	DIRWORD(b);
	D = ADD16_REG(D, b.w.l);
}

/* $d4 ANDB direct -**0- */
OP_HANDLER(andb_di) {
	uint8 t;
	DIRBYTE(t);
	B = AND8_REG(B, t);
}

/* $d5 BITB direct -**0- */
OP_HANDLER(bitb_di) {
	uint8 t;
	DIRBYTE(t);
	B = BIT8_REG(B, t);
}

/* $d6 LDB direct -**0- */
OP_HANDLER(ldb_di) {
	DIRBYTE(B);
	B = LOAD8_REG(B);
}

/* $d7 STB direct -**0- */
OP_HANDLER(stb_di) {
	DIRECT;
	STORE8_REG(B);
}

/* $d8 EORB direct -**0- */
OP_HANDLER(eorb_di) {
	uint8 t;
	DIRBYTE(t);
	B = EOR8_REG(B, t);
}

/* $d9 ADCB direct ***** */
OP_HANDLER(adcb_di) {
	uint8 t;
	DIRBYTE(t);
	B = ADC8_REG(B, t);
}

/* $dA ORB direct -**0- */
OP_HANDLER(orb_di) {
	uint8 t;
	DIRBYTE(t);
	B = OR8_REG(B, t);
}

/* $dB ADDB direct ***** */
OP_HANDLER(addb_di) {
	uint8 t;
	DIRBYTE(t);
	B = ADD8_REG(B, t);
}

/* $dC LDD direct -**0- */
OP_HANDLER(ldd_di) {
	DIRWORD(pD);
	D = LOAD16_REG(D);
}

/* $dD STD direct -**0- */
OP_HANDLER(std_di) {
	DIRECT;
	STORE16_REG(&pD);
}

/* $dE LDU (LDS) direct -**0- */
OP_HANDLER(ldu_di) {
	DIRWORD(pU);
	U = LOAD16_REG(U);
}

/* $10dE LDS direct -**0- */
OP_HANDLER(lds_di) {
	DIRWORD(pS);
	S = LOAD16_REG(S);
	int_state |= MC6809_LDS;
}

/* $dF STU (STS) direct -**0- */
OP_HANDLER(stu_di) {
	DIRECT;
	STORE16_REG(&pU);
}

/* $10dF STS direct -**0- */
OP_HANDLER(sts_di) {
	DIRECT;
	STORE16_REG(&pS);
}

/* $e0 SUBB indexed ?**** */
OP_HANDLER(subb_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	B = SUB8_REG(B, t);
}

/* $e1 CMPB indexed ?**** */
OP_HANDLER(cmpb_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	B = CMP8_REG(B, t);
}

/* $e2 SBCB indexed ?**** */
OP_HANDLER(sbcb_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	B = SBC8_REG(B, t);
}

/* $e3 ADDD indexed -**** */
OP_HANDLER(addd_ix) {
	pair b;
	b = GET_INDEXED_DATA16();
	D = ADD16_REG(D, b.w.l);
}

/* $e4 ANDB indexed -**0- */
OP_HANDLER(andb_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	B = AND8_REG(B, t);
}

/* $e5 BITB indexed -**0- */
OP_HANDLER(bitb_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	B = BIT8_REG(B, t);
}

/* $e6 LDB indexed -**0- */
OP_HANDLER(ldb_ix) {
	B = GET_INDEXED_DATA();
	B = LOAD8_REG(B);
}

/* $e7 STB indexed -**0- */
OP_HANDLER(stb_ix) {
	fetch_effective_address();
	STORE8_REG(B);
}

/* $e8 EORB indexed -**0- */
OP_HANDLER(eorb_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	B = EOR8_REG(B, t);
}

/* $e9 ADCB indexed ***** */
OP_HANDLER(adcb_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	B = ADC8_REG(B, t);
}

/* $eA ORB indexed -**0- */
OP_HANDLER(orb_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	B = OR8_REG(B, t);
}

/* $eB ADDB indexed ***** */
OP_HANDLER(addb_ix) {
	uint8 t;
	t = GET_INDEXED_DATA();
	B = ADD8_REG(B, t);
}

/* $eC LDD indexed -**0- */
OP_HANDLER(ldd_ix) {
	pair t;
	t = GET_INDEXED_DATA16();
	D = t.w.l;
	D = LOAD16_REG(D);
}

/* $eD STD indexed -**0- */
OP_HANDLER(std_ix) {
	fetch_effective_address();
	STORE16_REG(&pD);
}

/* $eE LDU (LDS) indexed -**0- */
OP_HANDLER(ldu_ix) {
	pair t;
	t = GET_INDEXED_DATA16();
	U = t.w.l;
	U = LOAD16_REG(U);
}

/* $10eE LDS indexed -**0- */
OP_HANDLER(lds_ix) {
	pair t;
	t = GET_INDEXED_DATA16();
	S = t.w.l;
	S = LOAD16_REG(S);
	int_state |= MC6809_LDS;
}

/* $eF STU (STS) indexed -**0- */
OP_HANDLER(stu_ix) {
	fetch_effective_address();
	STORE16_REG(&pU);
}

/* $10eF STS indexed -**0- */
OP_HANDLER(sts_ix) {
	fetch_effective_address();
	STORE16_REG(&pS);
}

/* $f0 SUBB extended ?**** */
OP_HANDLER(subb_ex) {
	uint8 t;
	EXTBYTE(t);
	B = SUB8_REG(B, t);
}

/* $f1 CMPB extended ?**** */
OP_HANDLER(cmpb_ex) {
	uint8 t;
	EXTBYTE(t);
	B = CMP8_REG(B, t);
}

/* $f2 SBCB extended ?**** */
OP_HANDLER(sbcb_ex) {
	uint8 t;
	EXTBYTE(t);
	B = SBC8_REG(B, t);
}

/* $f3 ADDD extended -**** */
OP_HANDLER(addd_ex) {
	pair b;
	EXTWORD(b);
	D = ADD16_REG(D, b.w.l);
}

/* $f4 ANDB extended -**0- */
OP_HANDLER(andb_ex) {
	uint8 t;
	EXTBYTE(t);
	B = AND8_REG(B, t);
}

/* $f5 BITB extended -**0- */
OP_HANDLER(bitb_ex) {
	uint8 t;
	EXTBYTE(t);
	B = BIT8_REG(B, t);
}

/* $f6 LDB extended -**0- */
OP_HANDLER(ldb_ex) {
	EXTBYTE(B);
	B = LOAD8_REG(B);
}

/* $f7 STB extended -**0- */
OP_HANDLER(stb_ex) {
	EXTENDED;
	STORE8_REG(B);
}

/* $f8 EORB extended -**0- */
OP_HANDLER(eorb_ex) {
	uint8 t;
	EXTBYTE(t);
	B = EOR8_REG(B, t);
}

/* $f9 ADCB extended ***** */
OP_HANDLER(adcb_ex) {
	uint8 t;
	EXTBYTE(t);
	B = ADC8_REG(B, t);
}

/* $fA ORB extended -**0- */
OP_HANDLER(orb_ex) {
	uint8 t;
	EXTBYTE(t);
	B = OR8_REG(B, t);
}

/* $fB ADDB extended ***** */
OP_HANDLER(addb_ex) {
	uint8 t;
	EXTBYTE(t);
	B = ADD8_REG(B, t);
}

/* $fC LDD extended -**0- */
OP_HANDLER(ldd_ex) {
	EXTWORD(pD);
	D = LOAD16_REG(D);
}

/* $fD STD extended -**0- */
OP_HANDLER(std_ex) {
	EXTENDED;
	STORE16_REG(&pD);
}

/* $fE LDU (LDS) extended -**0- */
OP_HANDLER(ldu_ex) {
	EXTWORD(pU);
	U = LOAD16_REG(U);
}

/* $10fE LDS extended -**0- */
OP_HANDLER(lds_ex) {
	EXTWORD(pS);
	S = LOAD16_REG(S);
	int_state |= MC6809_LDS;
}

/* $fF STU (STS) extended -**0- */
OP_HANDLER(stu_ex) {
	EXTENDED;
	STORE16_REG(&pU);
}

/* $10fF STS extended -**0- */
OP_HANDLER(sts_ex) {
	EXTENDED;
	STORE16_REG(&pS);
}


/* $10xx opcodes */
OP_HANDLER(pref10) {
	uint8 ireg2 = ROP_ARG(PCD);
	PC++;
	switch (ireg2) {
		case 0x20:
			lbra();
			icount -= 5;
			break;	// 20111217
		case 0x21:
			lbrn();
			icount -= 5;
			break;
		case 0x22:
			lbhi();
			icount -= 5;
			break;
		case 0x23:
			lbls();
			icount -= 5;
			break;
		case 0x24:
			lbcc();
			icount -= 5;
			break;
		case 0x25:
			lbcs();
			icount -= 5;
			break;
		case 0x26:
			lbne();
			icount -= 5;
			break;
		case 0x27:
			lbeq();
			icount -= 5;
			break;
		case 0x28:
			lbvc();
			icount -= 5;
			break;
		case 0x29:
			lbvs();
			icount -= 5;
			break;
		case 0x2a:
			lbpl();
			icount -= 5;
			break;
		case 0x2b:
			lbmi();
			icount -= 5;
			break;
		case 0x2c:
			lbge();
			icount -= 5;
			break;
		case 0x2d:
			lblt();
			icount -= 5;
			break;
		case 0x2e:
			lbgt();
			icount -= 5;
			break;
		case 0x2f:
			lble();
			icount -= 5;
			break;
		case 0x3f:
			swi2();
			icount -= 20;
			break;
		case 0x83:
			cmpd_im();
			icount -= 5;
			break;
		case 0x8c:
			cmpy_im();
			icount -= 5;
			break;
		case 0x8d:
			lbsr();
			icount -= 9;
			break;
		case 0x8e:
			ldy_im();
			icount -= 4;
			break;
//    case 0x8f: flag16_im();->cycle=4; break; // 20130417
		case 0x93:
			cmpd_di();
			icount -= 7;
			break;
		case 0x9c:
			cmpy_di();
			icount -= 7;
			break;
		case 0x9e:
			ldy_di();
			icount -= 6;
			break;
		case 0x9f:
			sty_di();
			icount -= 6;
			break;
		case 0xa3:
			cmpd_ix();
			icount -= 7;
			break;
		case 0xac:
			cmpy_ix();
			icount -= 7;
			break;
		case 0xae:
			ldy_ix();
			icount -= 6;
			break;
		case 0xaf:
			sty_ix();
			icount -= 6;
			break;
		case 0xb3:
			cmpd_ex();
			icount -= 8;
			break;
		case 0xbc:
			cmpy_ex();
			icount -= 8;
			break;
		case 0xbe:
			ldy_ex();
			icount -= 7;
			break;
		case 0xbf:
			sty_ex();
			icount -= 7;
			break;
		case 0xce:
			lds_im();
			icount -= 4;
			break;
//    case 0xcf: flag16_im();->cycle=4; break;
		case 0xde:
			lds_di();
			icount -= 6;
			break;
		case 0xdf:
			sts_di();
			icount -= 6;
			break;
		case 0xee:
			lds_ix();
			icount -= 6;
			break;
		case 0xef:
			sts_ix();
			icount -= 6;
			break;
		case 0xfe:
			lds_ex();
			icount -= 7;
			break;
		case 0xff:
			sts_ex();
			icount -= 7;
			break;
		default:
			PC--;
			IIError();
			break;
	}
}

/* $11xx opcodes */
OP_HANDLER(pref11) {
		uint8 ireg2 = ROP_ARG(PCD);
		PC++;
		switch (ireg2) {
			case 0x3f:
				swi3();
				icount -= 20;
				break;

			case 0x83:
				cmpu_im();
				icount -= 5;
				break;
			case 0x8c:
				cmps_im();
				icount -= 5;
				break;

			case 0x93:
				cmpu_di();
				icount -= 7;
				break;
			case 0x9c:
				cmps_di();
				icount -= 7;
				break;

			case 0xa3:
				cmpu_ix();
				icount -= 7;
				break;
			case 0xac:
				cmps_ix();
				icount -= 7;
				break;

			case 0xb3:
				cmpu_ex();
				icount -= 8;
				break;
			case 0xbc:
				cmps_ex();
				icount -= 8;
				break;

			default:
				PC--;
				IIError();
				break;
		}
	}

#define STATE_VERSION	1

void MC6809::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputInt32(icount);
	state_fio->FputInt32(extra_icount);
	state_fio->FputUint32(int_state);

	state_fio->FputUint32(pc.d);
	state_fio->FputUint32(ppc.d);
	state_fio->FputUint32(acc.d);
	state_fio->FputUint32(dp.d);
	state_fio->FputUint32(u.d);
	state_fio->FputUint32(s.d);
	state_fio->FputUint32(x.d);
	state_fio->FputUint32(y.d);
	state_fio->FputUint8(cc);
	state_fio->FputUint32(ea.d);
}

bool MC6809::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	
	icount = state_fio->FgetInt32();
	extra_icount = state_fio->FgetInt32();
	int_state = state_fio->FgetUint32();
	pc.d = state_fio->FgetUint32();
	ppc.d = state_fio->FgetUint32();
	acc.d = state_fio->FgetUint32();
	dp.d = state_fio->FgetUint32();
	u.d = state_fio->FgetUint32();
	s.d = state_fio->FgetUint32();
	x.d = state_fio->FgetUint32();
	y.d = state_fio->FgetUint32();
	cc = state_fio->FgetUint8();
	ea.d = state_fio->FgetUint32();

	return true;
}


