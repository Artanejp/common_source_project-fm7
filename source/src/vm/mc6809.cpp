/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date   : 2011.05.06-

	[ MC6809 ]
*/

// Fixed IRQ/FIRQ by Mr.Sasaji at 2011.06.17

#include "mc6809.h"

#define MC6809_IRQ_BIT	1	/* IRQ line number  */
#define MC6809_FIRQ_BIT	2	/* FIRQ line number */
#define MC6809_NMI_BIT	4	/* NMI line number  */

/* flag bits in the cc register */
#define MC6809_CWAI	0x08	/* set when CWAI is waiting for an interrupt */
#define MC6809_SYNC	0x10	/* set when SYNC is waiting for an interrupt */
#define MC6809_LDS	0x20	/* set when LDS occured at least once */

#define CC_C	0x01		/* Carry */
#define CC_V	0x02		/* Overflow */
#define CC_Z	0x04		/* Zero */
#define CC_N	0x08		/* Negative */
#define CC_II	0x10		/* Inhibit IRQ */
#define CC_H	0x20		/* Half (auxiliary) carry */
#define CC_IF	0x40		/* Inhibit FIRQ */
#define CC_E	0x80		/* entire state pushed */

#define pPPC    ppc
#define pPC 	pc
#define pU	u
#define pS	s
#define pX	x
#define pY	y
#define pD	acc

#define PPC	ppc.w.l
#define PC	pc.w.l
#define PCD	pc.d
#define U	u.w.l
#define UD	u.d
#define S	s.w.l
#define SD	s.d
#define X	x.w.l
#define XD	x.d
#define Y	y.w.l
#define YD	y.d
#define D	acc.w.l
#define A	acc.b.h
#define B	acc.b.l
#define DP	dp.b.h
#define DPD	dp.d
#define CC	cc

#define EA	ea.w.l
#define EAD	ea.d
#define EAP	ea

/****************************************************************************/
/* memory                                                                   */
/****************************************************************************/

#define RM(Addr)	d_mem->read_data8(Addr)
#define WM(Addr,Value)	d_mem->write_data8(Addr, Value)

#define ROP(Addr)	d_mem->read_data8(Addr)
#define ROP_ARG(Addr)	d_mem->read_data8(Addr)

/* macros to access memory */
#define IMMBYTE(b)	b = ROP_ARG(PCD); PC++
#define IMMWORD(w)	w.d = (ROP_ARG(PCD) << 8) | ROP_ARG((PCD + 1) & 0xffff); PC += 2

#define PUSHBYTE(b)	--S; WM(SD,b)
#define PUSHWORD(w)	--S; WM(SD, w.b.l); --S; WM(SD, w.b.h)
#define PULLBYTE(b)	b = RM(SD); S++
#define PULLWORD(w)	w = RM(SD) << 8; S++; w |= RM(SD); S++

#define PSHUBYTE(b)	--U; WM(UD, b);
#define PSHUWORD(w)	--U; WM(UD, w.b.l); --U; WM(UD, w.b.h)
#define PULUBYTE(b)	b = RM(UD); U++
#define PULUWORD(w)	w = RM(UD) << 8; U++; w |= RM(UD); U++

#define CLR_HNZVC	CC &= ~(CC_H | CC_N | CC_Z | CC_V | CC_C)
#define CLR_NZV 	CC &= ~(CC_N | CC_Z | CC_V)
#define CLR_NZ		CC &= ~(CC_N | CC_Z)
#define CLR_HNZC	CC &= ~(CC_H | CC_N | CC_Z | CC_C)
#define CLR_NZVC	CC &= ~(CC_N | CC_Z | CC_V | CC_C)
#define CLR_Z		CC &= ~(CC_Z)
#define CLR_NZC 	CC &= ~(CC_N | CC_Z | CC_C)
#define CLR_ZC		CC &= ~(CC_Z | CC_C)

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)		if(!a) SEZ
#define SET_Z8(a)		SET_Z((uint8)a)
#define SET_Z16(a)		SET_Z((uint16)a)
#define SET_N8(a)		CC |= ((a & 0x80) >> 4)
#define SET_N16(a)		CC |= ((a & 0x8000) >> 12)
#define SET_H(a,b,r)		CC |= (((a ^ b ^ r) & 0x10) << 1)
#define SET_C8(a)		CC |= ((a & 0x100) >> 8)
#define SET_C16(a)		CC |= ((a & 0x10000) >> 16)
#define SET_V8(a,b,r)		CC |= (((a ^ b ^ r ^ (r >> 1)) & 0x80) >> 6)
#define SET_V16(a,b,r)		CC |= (((a ^ b ^ r ^ (r >> 1)) & 0x8000) >> 14)

#define SET_FLAGS8I(a)		{CC |= flags8i[(a) & 0xff];}
#define SET_FLAGS8D(a)		{CC |= flags8d[(a) & 0xff];}

/* combos */
#define SET_NZ8(a)		{SET_N8(a); SET_Z(a);}
#define SET_NZ16(a)		{SET_N16(a); SET_Z(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r); SET_Z8(r); SET_V8(a, b, r); SET_C8(r);}
#define SET_FLAGS16(a,b,r)	{SET_N16(r); SET_Z16(r); SET_V16(a, b, r); SET_C16(r);}

#define NXORV		((CC & CC_N) ^ ((CC & CC_V) << 2))

/* for treating an unsigned byte as a signed word */
#define SIGNED(b)	((uint16)((b & 0x80) ? (b | 0xff00) : b))

/* macros for addressing modes (postbytes have their own code) */
#define DIRECT		EAD = DPD; IMMBYTE(ea.b.l)
#define IMM8		EAD = PCD; PC++
#define IMM16		EAD = PCD; PC += 2
#define EXTENDED	IMMWORD(EAP)

/* macros to set status flags */
#define SEC		CC |= CC_C
#define CLC		CC &= ~CC_C
#define SEZ		CC |= CC_Z
#define CLZ		CC &= ~CC_Z
#define SEN		CC |= CC_N
#define CLN		CC &= ~CC_N
#define SEV		CC |= CC_V
#define CLV		CC &= ~CC_V
#define SEH		CC |= CC_H
#define CLH		CC &= ~CC_H

/* macros for convenience */
#define DIRBYTE(b)	{DIRECT;   b   = RM(EAD);  }
#define DIRWORD(w)	{DIRECT;   w.d = RM16(EAD);}
#define EXTBYTE(b)	{EXTENDED; b   = RM(EAD);  }
#define EXTWORD(w)	{EXTENDED; w.d = RM16(EAD);}

/* macros for branch instructions */
#define BRANCH(f) { \
	uint8 t; \
	IMMBYTE(t); \
	if(f) { \
		PC += SIGNED(t); \
	} \
}

#define LBRANCH(f) { \
	pair t; \
	IMMWORD(t); \
	if(f) { \
		icount -= 1; \
		PC += t.w.l; \
	} \
}

/* macros for setting/getting registers in TFR/EXG instructions */

inline uint32 MC6809::RM16(uint32 Addr)
{
	uint32 result = RM(Addr) << 8;
	return result | RM((Addr + 1) & 0xffff);
}

inline void MC6809::WM16(uint32 Addr, pair *p)
{
	WM(Addr, p->b.h);
	WM((Addr + 1) & 0xffff, p->b.l);
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
static const uint8 index_cycle_em[256] = {
	/* Index Loopup cycle counts */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 3, 2, 3, 0, 1, 1, 0, 1, 4, 0, 4, 1, 5, 0, 5,
	5, 6, 5, 6, 3, 4, 4, 0, 4, 7, 0, 7, 4, 8, 0, 8,
	2, 3, 2, 3, 0, 1, 1, 0, 1, 4, 0, 4, 1, 5, 0, 5,
	5, 6, 5, 6, 3, 4, 4, 0, 4, 7, 0, 7, 4, 8, 0, 8,
	2, 3, 2, 3, 0, 1, 1, 0, 1, 4, 0, 4, 1, 5, 0, 3,
	5, 6, 5, 6, 3, 4, 4, 0, 4, 7, 0, 7, 4, 8, 0, 8,
	2, 3, 2, 3, 0, 1, 1, 0, 1, 4, 0, 4, 1, 5, 0, 5,
	4, 6, 5, 6, 3, 4, 4, 0, 4, 7, 0, 7, 4, 8, 0, 8
};

/* timings for 1-byte opcodes */
static const uint8 cycles1[] =
{
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 6,
	0, 0, 2, 4, 2, 2, 5, 9, 2, 2, 3, 2, 3, 2, 8, 6,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 5, 5, 5, 5, 2, 5, 3, 6,20,11, 2,19,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 6,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4, 7,
	2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 4, 7, 3, 2,
	4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 7, 5, 5,
	4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 7, 5, 5,
	5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 5, 7, 8, 6, 6,
	2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 3,
	4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
	5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6
};

void MC6809::reset()
{
	icount = 0;
	int_state = 0;
	
	DPD = 0;	/* Reset direct page register */
	
	CC |= CC_II;	/* IRQ disabled */
	CC |= CC_IF;	/* FIRQ disabled */
	
	PCD = RM16(0xfffe);
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
	}
}

int MC6809::run(int clock)
{
	// run cpu
	if(clock == -1) {
		// run only one opcode
		icount = 0;
		run_one_opecode();
		return -icount;
	} else {
		// run cpu while given clocks
		icount += clock;
		int first_icount = icount;
		
		while(icount > 0) {
			run_one_opecode();
		}
		return first_icount - icount;
	}
}

void MC6809::run_one_opecode()
{
	if(int_state & MC6809_NMI_BIT) {
		int_state &= ~MC6809_NMI_BIT;
		int_state &= ~MC6809_SYNC; /* clear SYNC flag */
		if(int_state & MC6809_CWAI) {
			int_state &= ~MC6809_CWAI;
			icount -= 7; /* subtract +7 cycles next time */
		} else {
			CC |= CC_E; /* save entire state */
			PUSHWORD(pPC);
			PUSHWORD(pU);
			PUSHWORD(pY);
			PUSHWORD(pX);
			PUSHBYTE(DP);
			PUSHBYTE(B);
			PUSHBYTE(A);
			PUSHBYTE(CC);
			icount -= 19; /* subtract +19 cycles next time */
		}
		CC |= CC_IF | CC_II; /* inhibit FIRQ and IRQ */
		PCD = RM16(0xfffc);
	} else if(int_state & (MC6809_FIRQ_BIT | MC6809_IRQ_BIT)) {
		int_state &= ~MC6809_SYNC; /* clear SYNC flag */
		if((int_state & MC6809_FIRQ_BIT) && !(CC & CC_IF)) {
			/* fast IRQ */
			int_state &= ~MC6809_FIRQ_BIT;
			if(int_state & MC6809_CWAI) {
				int_state &= ~MC6809_CWAI; /* clear CWAI */
				icount -= 7; /* subtract +7 cycles */
			} else {
				CC &= ~CC_E; /* save 'short' state */
				PUSHWORD(pPC);
				PUSHBYTE(CC);
				icount -= 10; /* subtract +10 cycles */
			}
			CC |= CC_IF | CC_II; /* inhibit FIRQ and IRQ */
			PCD = RM16(0xfff6);
		} else if((int_state & MC6809_IRQ_BIT) && !(CC & CC_II)) {
			/* standard IRQ */
			int_state &= ~MC6809_IRQ_BIT;
			if(int_state & MC6809_CWAI) {
				int_state &= ~MC6809_CWAI; /* clear CWAI flag */
				icount -= 7; /* subtract +7 cycles */
			} else {
				CC |= CC_E; /* save entire state */
				PUSHWORD(pPC);
				PUSHWORD(pU);
				PUSHWORD(pY);
				PUSHWORD(pX);
				PUSHBYTE(DP);
				PUSHBYTE(B);
				PUSHBYTE(A);
				PUSHBYTE(CC);
				icount -= 19; /* subtract +19 cycles */
			}
			CC |= CC_II; /* inhibit IRQ */
			PCD = RM16(0xfff8);
		}
	}
	if (int_state & (MC6809_CWAI | MC6809_SYNC)) {
		icount = 0;
	} else {
		pPPC = pPC;
		uint8 ireg = ROP(PCD);
		PC++;
		op(ireg);
		icount -= cycles1[ireg];
	}
}

void MC6809::op(uint8 ireg)
{
	switch(ireg) {
	case 0x00: neg_di(); break;
	case 0x01: neg_di(); break;
	case 0x02: com_di(); break;
	case 0x03: com_di(); break;
	case 0x04: lsr_di(); break;
	case 0x05: lsr_di(); break;
	case 0x06: ror_di(); break;
	case 0x07: asr_di(); break;
	case 0x08: asl_di(); break;
	case 0x09: rol_di(); break;
	case 0x0a: dec_di(); break;
	case 0x0b: dec_di(); break;
	case 0x0c: inc_di(); break;
	case 0x0d: tst_di(); break;
	case 0x0e: jmp_di(); break;
	case 0x0f: clr_di(); break;
	case 0x10: pref10(); break;
	case 0x11: pref11(); break;
	case 0x12: nop(); break;
	case 0x13: sync(); break;
	case 0x14: illegal(); break;
	case 0x15: illegal(); break;
	case 0x16: lbra(); break;
	case 0x17: lbsr(); break;
	case 0x18: illegal(); break;
	case 0x19: daa(); break;
	case 0x1a: orcc(); break;
	case 0x1b: illegal(); break;
	case 0x1c: andcc(); break;
	case 0x1d: sex(); break;
	case 0x1e: exg(); break;
	case 0x1f: tfr(); break;
	case 0x20: bra(); break;
	case 0x21: brn(); break;
	case 0x22: bhi(); break;
	case 0x23: bls(); break;
	case 0x24: bcc(); break;
	case 0x25: bcs(); break;
	case 0x26: bne(); break;
	case 0x27: beq(); break;
	case 0x28: bvc(); break;
	case 0x29: bvs(); break;
	case 0x2a: bpl(); break;
	case 0x2b: bmi(); break;
	case 0x2c: bge(); break;
	case 0x2d: blt(); break;
	case 0x2e: bgt(); break;
	case 0x2f: ble(); break;
	case 0x30: leax(); break;
	case 0x31: leay(); break;
	case 0x32: leas(); break;
	case 0x33: leau(); break;
	case 0x34: pshs(); break;
	case 0x35: puls(); break;
	case 0x36: pshu(); break;
	case 0x37: pulu(); break;
	case 0x38: illegal(); break;
	case 0x39: rts(); break;
	case 0x3a: abx(); break;
	case 0x3b: rti(); break;
	case 0x3c: cwai(); break;
	case 0x3d: mul(); break;
	case 0x3e: illegal(); break;
	case 0x3f: swi(); break;
	case 0x40: nega(); break;
	case 0x41: nega(); break;
	case 0x42: coma(); break;
	case 0x43: coma(); break;
	case 0x44: lsra(); break;
	case 0x45: lsra(); break;
	case 0x46: rora(); break;
	case 0x47: asra(); break;
	case 0x48: asla(); break;
	case 0x49: rola(); break;
	case 0x4a: deca(); break;
	case 0x4b: deca(); break;
	case 0x4c: inca(); break;
	case 0x4d: tsta(); break;
	case 0x4e: clra(); break;
	case 0x4f: clra(); break;
	case 0x50: negb(); break;
	case 0x51: negb(); break;
	case 0x52: comb(); break;
	case 0x53: comb(); break;
	case 0x54: lsrb(); break;
	case 0x55: lsrb(); break;
	case 0x56: rorb(); break;
	case 0x57: asrb(); break;
	case 0x58: aslb(); break;
	case 0x59: rolb(); break;
	case 0x5a: decb(); break;
	case 0x5b: decb(); break;
	case 0x5c: incb(); break;
	case 0x5d: tstb(); break;
	case 0x5e: clrb(); break;
	case 0x5f: clrb(); break;
	case 0x60: neg_ix(); break;
	case 0x61: neg_ix(); break;
	case 0x62: com_ix(); break;
	case 0x63: com_ix(); break;
	case 0x64: lsr_ix(); break;
	case 0x65: lsr_ix(); break;
	case 0x66: ror_ix(); break;
	case 0x67: asr_ix(); break;
	case 0x68: asl_ix(); break;
	case 0x69: rol_ix(); break;
	case 0x6a: dec_ix(); break;
	case 0x6b: dec_ix(); break;
	case 0x6c: inc_ix(); break;
	case 0x6d: tst_ix(); break;
	case 0x6e: jmp_ix(); break;
	case 0x6f: clr_ix(); break;
	case 0x70: neg_ex(); break;
	case 0x71: neg_ex(); break;
	case 0x72: com_ex(); break;
	case 0x73: com_ex(); break;
	case 0x74: lsr_ex(); break;
	case 0x75: lsr_ex(); break;
	case 0x76: ror_ex(); break;
	case 0x77: asr_ex(); break;
	case 0x78: asl_ex(); break;
	case 0x79: rol_ex(); break;
	case 0x7a: dec_ex(); break;
	case 0x7b: dec_ex(); break;
	case 0x7c: inc_ex(); break;
	case 0x7d: tst_ex(); break;
	case 0x7e: jmp_ex(); break;
	case 0x7f: clr_ex(); break;
	case 0x80: suba_im(); break;
	case 0x81: cmpa_im(); break;
	case 0x82: sbca_im(); break;
	case 0x83: subd_im(); break;
	case 0x84: anda_im(); break;
	case 0x85: bita_im(); break;
	case 0x86: lda_im(); break;
	case 0x87: sta_im(); break;
	case 0x88: eora_im(); break;
	case 0x89: adca_im(); break;
	case 0x8a: ora_im(); break;
	case 0x8b: adda_im(); break;
	case 0x8c: cmpx_im(); break;
	case 0x8d: bsr(); break;
	case 0x8e: ldx_im(); break;
	case 0x8f: stx_im(); break;
	case 0x90: suba_di(); break;
	case 0x91: cmpa_di(); break;
	case 0x92: sbca_di(); break;
	case 0x93: subd_di(); break;
	case 0x94: anda_di(); break;
	case 0x95: bita_di(); break;
	case 0x96: lda_di(); break;
	case 0x97: sta_di(); break;
	case 0x98: eora_di(); break;
	case 0x99: adca_di(); break;
	case 0x9a: ora_di(); break;
	case 0x9b: adda_di(); break;
	case 0x9c: cmpx_di(); break;
	case 0x9d: jsr_di(); break;
	case 0x9e: ldx_di(); break;
	case 0x9f: stx_di(); break;
	case 0xa0: suba_ix(); break;
	case 0xa1: cmpa_ix(); break;
	case 0xa2: sbca_ix(); break;
	case 0xa3: subd_ix(); break;
	case 0xa4: anda_ix(); break;
	case 0xa5: bita_ix(); break;
	case 0xa6: lda_ix(); break;
	case 0xa7: sta_ix(); break;
	case 0xa8: eora_ix(); break;
	case 0xa9: adca_ix(); break;
	case 0xaa: ora_ix(); break;
	case 0xab: adda_ix(); break;
	case 0xac: cmpx_ix(); break;
	case 0xad: jsr_ix(); break;
	case 0xae: ldx_ix(); break;
	case 0xaf: stx_ix(); break;
	case 0xb0: suba_ex(); break;
	case 0xb1: cmpa_ex(); break;
	case 0xb2: sbca_ex(); break;
	case 0xb3: subd_ex(); break;
	case 0xb4: anda_ex(); break;
	case 0xb5: bita_ex(); break;
	case 0xb6: lda_ex(); break;
	case 0xb7: sta_ex(); break;
	case 0xb8: eora_ex(); break;
	case 0xb9: adca_ex(); break;
	case 0xba: ora_ex(); break;
	case 0xbb: adda_ex(); break;
	case 0xbc: cmpx_ex(); break;
	case 0xbd: jsr_ex(); break;
	case 0xbe: ldx_ex(); break;
	case 0xbf: stx_ex(); break;
	case 0xc0: subb_im(); break;
	case 0xc1: cmpb_im(); break;
	case 0xc2: sbcb_im(); break;
	case 0xc3: addd_im(); break;
	case 0xc4: andb_im(); break;
	case 0xc5: bitb_im(); break;
	case 0xc6: ldb_im(); break;
	case 0xc7: stb_im(); break;
	case 0xc8: eorb_im(); break;
	case 0xc9: adcb_im(); break;
	case 0xca: orb_im(); break;
	case 0xcb: addb_im(); break;
	case 0xcc: ldd_im(); break;
	case 0xcd: std_im(); break;
	case 0xce: ldu_im(); break;
	case 0xcf: stu_im(); break;
	case 0xd0: subb_di(); break;
	case 0xd1: cmpb_di(); break;
	case 0xd2: sbcb_di(); break;
	case 0xd3: addd_di(); break;
	case 0xd4: andb_di(); break;
	case 0xd5: bitb_di(); break;
	case 0xd6: ldb_di(); break;
	case 0xd7: stb_di(); break;
	case 0xd8: eorb_di(); break;
	case 0xd9: adcb_di(); break;
	case 0xda: orb_di(); break;
	case 0xdb: addb_di(); break;
	case 0xdc: ldd_di(); break;
	case 0xdd: std_di(); break;
	case 0xde: ldu_di(); break;
	case 0xdf: stu_di(); break;
	case 0xe0: subb_ix(); break;
	case 0xe1: cmpb_ix(); break;
	case 0xe2: sbcb_ix(); break;
	case 0xe3: addd_ix(); break;
	case 0xe4: andb_ix(); break;
	case 0xe5: bitb_ix(); break;
	case 0xe6: ldb_ix(); break;
	case 0xe7: stb_ix(); break;
	case 0xe8: eorb_ix(); break;
	case 0xe9: adcb_ix(); break;
	case 0xea: orb_ix(); break;
	case 0xeb: addb_ix(); break;
	case 0xec: ldd_ix(); break;
	case 0xed: std_ix(); break;
	case 0xee: ldu_ix(); break;
	case 0xef: stu_ix(); break;
	case 0xf0: subb_ex(); break;
	case 0xf1: cmpb_ex(); break;
	case 0xf2: sbcb_ex(); break;
	case 0xf3: addd_ex(); break;
	case 0xf4: andb_ex(); break;
	case 0xf5: bitb_ex(); break;
	case 0xf6: ldb_ex(); break;
	case 0xf7: stb_ex(); break;
	case 0xf8: eorb_ex(); break;
	case 0xf9: adcb_ex(); break;
	case 0xfa: orb_ex(); break;
	case 0xfb: addb_ex(); break;
	case 0xfc: ldd_ex(); break;
	case 0xfd: std_ex(); break;
	case 0xfe: ldu_ex(); break;
	case 0xff: stu_ex(); break;
	default: __assume(0);
	}
};

inline void MC6809::fetch_effective_address()
{
	uint8 postbyte = ROP_ARG(PCD);
	PC++;
	
	switch(postbyte) {
	case 0x00: EA = X; break;
	case 0x01: EA = X + 1; break;
	case 0x02: EA = X + 2; break;
	case 0x03: EA = X + 3; break;
	case 0x04: EA = X + 4; break;
	case 0x05: EA = X + 5; break;
	case 0x06: EA = X + 6; break;
	case 0x07: EA = X + 7; break;
	case 0x08: EA = X + 8; break;
	case 0x09: EA = X + 9; break;
	case 0x0a: EA = X + 10; break;
	case 0x0b: EA = X + 11; break;
	case 0x0c: EA = X + 12; break;
	case 0x0d: EA = X + 13; break;
	case 0x0e: EA = X + 14; break;
	case 0x0f: EA = X + 15; break;
	case 0x10: EA = X - 16; break;
	case 0x11: EA = X - 15; break;
	case 0x12: EA = X - 14; break;
	case 0x13: EA = X - 13; break;
	case 0x14: EA = X - 12; break;
	case 0x15: EA = X - 11; break;
	case 0x16: EA = X - 10; break;
	case 0x17: EA = X - 9; break;
	case 0x18: EA = X - 8; break;
	case 0x19: EA = X - 7; break;
	case 0x1a: EA = X - 6; break;
	case 0x1b: EA = X - 5; break;
	case 0x1c: EA = X - 4; break;
	case 0x1d: EA = X - 3; break;
	case 0x1e: EA = X - 2; break;
	case 0x1f: EA = X - 1; break;
	case 0x20: EA = Y; break;
	case 0x21: EA = Y + 1; break;
	case 0x22: EA = Y + 2; break;
	case 0x23: EA = Y + 3; break;
	case 0x24: EA = Y + 4; break;
	case 0x25: EA = Y + 5; break;
	case 0x26: EA = Y + 6; break;
	case 0x27: EA = Y + 7; break;
	case 0x28: EA = Y + 8; break;
	case 0x29: EA = Y + 9; break;
	case 0x2a: EA = Y + 10; break;
	case 0x2b: EA = Y + 11; break;
	case 0x2c: EA = Y + 12; break;
	case 0x2d: EA = Y + 13; break;
	case 0x2e: EA = Y + 14; break;
	case 0x2f: EA = Y + 15; break;
	case 0x30: EA = Y - 16; break;
	case 0x31: EA = Y - 15; break;
	case 0x32: EA = Y - 14; break;
	case 0x33: EA = Y - 13; break;
	case 0x34: EA = Y - 12; break;
	case 0x35: EA = Y - 11; break;
	case 0x36: EA = Y - 10; break;
	case 0x37: EA = Y - 9; break;
	case 0x38: EA = Y - 8; break;
	case 0x39: EA = Y - 7; break;
	case 0x3a: EA = Y - 6; break;
	case 0x3b: EA = Y - 5; break;
	case 0x3c: EA = Y - 4; break;
	case 0x3d: EA = Y - 3; break;
	case 0x3e: EA = Y - 2; break;
	case 0x3f: EA = Y - 1; break;
	case 0x40: EA = U; break;
	case 0x41: EA = U + 1; break;
	case 0x42: EA = U + 2; break;
	case 0x43: EA = U + 3; break;
	case 0x44: EA = U + 4; break;
	case 0x45: EA = U + 5; break;
	case 0x46: EA = U + 6; break;
	case 0x47: EA = U + 7; break;
	case 0x48: EA = U + 8; break;
	case 0x49: EA = U + 9; break;
	case 0x4a: EA = U + 10; break;
	case 0x4b: EA = U + 11; break;
	case 0x4c: EA = U + 12; break;
	case 0x4d: EA = U + 13; break;
	case 0x4e: EA = U + 14; break;
	case 0x4f: EA = U + 15; break;
	case 0x50: EA = U - 16; break;
	case 0x51: EA = U - 15; break;
	case 0x52: EA = U - 14; break;
	case 0x53: EA = U - 13; break;
	case 0x54: EA = U - 12; break;
	case 0x55: EA = U - 11; break;
	case 0x56: EA = U - 10; break;
	case 0x57: EA = U - 9; break;
	case 0x58: EA = U - 8; break;
	case 0x59: EA = U - 7; break;
	case 0x5a: EA = U - 6; break;
	case 0x5b: EA = U - 5; break;
	case 0x5c: EA = U - 4; break;
	case 0x5d: EA = U - 3; break;
	case 0x5e: EA = U - 2; break;
	case 0x5f: EA = U - 1; break;
	case 0x60: EA = S; break;
	case 0x61: EA = S + 1; break;
	case 0x62: EA = S + 2; break;
	case 0x63: EA = S + 3; break;
	case 0x64: EA = S + 4; break;
	case 0x65: EA = S + 5; break;
	case 0x66: EA = S + 6; break;
	case 0x67: EA = S + 7; break;
	case 0x68: EA = S + 8; break;
	case 0x69: EA = S + 9; break;
	case 0x6a: EA = S + 10; break;
	case 0x6b: EA = S + 11; break;
	case 0x6c: EA = S + 12; break;
	case 0x6d: EA = S + 13; break;
	case 0x6e: EA = S + 14; break;
	case 0x6f: EA = S + 15; break;
	case 0x70: EA = S - 16; break;
	case 0x71: EA = S - 15; break;
	case 0x72: EA = S - 14; break;
	case 0x73: EA = S - 13; break;
	case 0x74: EA = S - 12; break;
	case 0x75: EA = S - 11; break;
	case 0x76: EA = S - 10; break;
	case 0x77: EA = S - 9; break;
	case 0x78: EA = S - 8; break;
	case 0x79: EA = S - 7; break;
	case 0x7a: EA = S - 6; break;
	case 0x7b: EA = S - 5; break;
	case 0x7c: EA = S - 4; break;
	case 0x7d: EA = S - 3; break;
	case 0x7e: EA = S - 2; break;
	case 0x7f: EA = S - 1; break;
	case 0x80: EA = X; X++; break;
	case 0x81: EA = X; X += 2; break;
	case 0x82: X--; EA = X; break;
	case 0x83: X -= 2; EA = X; break;
	case 0x84: EA = X; break;
	case 0x85: EA = X + SIGNED(B); break;
	case 0x86: EA = X + SIGNED(A); break;
	case 0x87: EA = 0; break; /* ILLEGAL*/
	case 0x88: IMMBYTE(EA); EA = X + SIGNED(EA); break;
	case 0x89: IMMWORD(EAP); EA += X; break;
	case 0x8a: EA = 0; break; /* IIError*/
	case 0x8b: EA = X + D; break;
	case 0x8c: IMMBYTE(EA); EA = PC + SIGNED(EA); break;
	case 0x8d: IMMWORD(EAP); EA += PC; break;
	case 0x8e: EA = 0; break; /* ILLEGAL*/
	case 0x8f: IMMWORD(EAP); break;
	case 0x90: EA = X; X++; EAD = RM16(EAD); break; /* Indirect ,R+ not in my specs */
	case 0x91: EA = X; X += 2; EAD = RM16(EAD); break;
	case 0x92: X--; EA = X; EAD = RM16(EAD); break;
	case 0x93: X -= 2; EA = X; EAD = RM16(EAD); break;
	case 0x94: EA = X; EAD = RM16(EAD); break;
	case 0x95: EA = X + SIGNED(B); EAD = RM16(EAD); break;
	case 0x96: EA = X + SIGNED(A); EAD = RM16(EAD); break;
	case 0x97: EA = 0; break; /* ILLEGAL*/
	case 0x98: IMMBYTE(EA); EA = X + SIGNED(EA); EAD = RM16(EAD); break;
	case 0x99: IMMWORD(EAP); EA += X; EAD = RM16(EAD); break;
	case 0x9a: EA = 0; break; /* ILLEGAL*/
	case 0x9b: EA = X + D; EAD = RM16(EAD); break;
	case 0x9c: IMMBYTE(EA); EA = PC + SIGNED(EA); EAD = RM16(EAD); break;
	case 0x9d: IMMWORD(EAP); EA += PC; EAD = RM16(EAD); break;
	case 0x9e: EA = 0; break; /* ILLEGAL*/
	case 0x9f: IMMWORD(EAP); EAD = RM16(EAD); break;
	case 0xa0: EA = Y; Y++; break;
	case 0xa1: EA = Y; Y += 2; break;
	case 0xa2: Y--; EA = Y; break;
	case 0xa3: Y -= 2; EA = Y; break;
	case 0xa4: EA = Y; break;
	case 0xa5: EA = Y + SIGNED(B); break;
	case 0xa6: EA = Y + SIGNED(A); break;
	case 0xa7: EA = 0; break; /* ILLEGAL*/
	case 0xa8: IMMBYTE(EA); EA = Y + SIGNED(EA); break;
	case 0xa9: IMMWORD(EAP); EA += Y; break;
	case 0xaa: EA = 0; break; /* ILLEGAL*/
	case 0xab: EA = Y + D; break;
	case 0xac: IMMBYTE(EA); EA = PC + SIGNED(EA); break;
	case 0xad: IMMWORD(EAP); EA += PC; break;
	case 0xae: EA = 0; break; /* ILLEGAL*/
	case 0xaf: IMMWORD(EAP); break;
	case 0xb0: EA = Y; Y++; EAD = RM16(EAD); break;
	case 0xb1: EA = Y; Y += 2; EAD = RM16(EAD); break;
	case 0xb2: Y--; EA = Y; EAD = RM16(EAD); break;
	case 0xb3: Y -= 2; EA = Y; EAD = RM16(EAD); break;
	case 0xb4: EA = Y; EAD = RM16(EAD); break;
	case 0xb5: EA = Y + SIGNED(B); EAD = RM16(EAD); break;
	case 0xb6: EA = Y + SIGNED(A); EAD = RM16(EAD); break;
	case 0xb7: EA = 0; break; /* ILLEGAL*/
	case 0xb8: IMMBYTE(EA); EA = Y + SIGNED(EA); EAD = RM16(EAD); break;
	case 0xb9: IMMWORD(EAP); EA += Y; EAD = RM16(EAD); break;
	case 0xba: EA = 0; break; /* ILLEGAL*/
	case 0xbb: EA = Y + D; EAD = RM16(EAD); break;
	case 0xbc: IMMBYTE(EA); EA = PC + SIGNED(EA); EAD = RM16(EAD); break;
	case 0xbd: IMMWORD(EAP); EA += PC; EAD = RM16(EAD); break;
	case 0xbe: EA = 0; break; /* ILLEGAL*/
	case 0xbf: IMMWORD(EAP); EAD = RM16(EAD); break;
	case 0xc0: EA = U; U++; break;
	case 0xc1: EA = U; U += 2; break;
	case 0xc2: U--; EA = U; break;
	case 0xc3: U -= 2; EA = U; break;
	case 0xc4: EA = U; break;
	case 0xc5: EA = U + SIGNED(B); break;
	case 0xc6: EA = U + SIGNED(A); break;
	case 0xc7: EA = 0; break; /*ILLEGAL*/
	case 0xc8: IMMBYTE(EA); EA = U + SIGNED(EA); break;
	case 0xc9: IMMWORD(EAP); EA += U; break;
	case 0xca: EA = 0; break; /*ILLEGAL*/
	case 0xcb: EA = U + D; break;
	case 0xcc: IMMBYTE(EA); EA = PC + SIGNED(EA); break;
	case 0xcd: IMMWORD(EAP); EA += PC; break;
	case 0xce: EA = 0; break; /*ILLEGAL*/
	case 0xcf: IMMWORD(EAP); break;
	case 0xd0: EA = U; U++; EAD = RM16(EAD); break;
	case 0xd1: EA = U; U += 2; EAD = RM16(EAD); break;
	case 0xd2: U--; EA = U; EAD = RM16(EAD); break;
	case 0xd3: U -= 2; EA = U; EAD = RM16(EAD); break;
	case 0xd4: EA = U; EAD = RM16(EAD); break;
	case 0xd5: EA = U + SIGNED(B); EAD = RM16(EAD); break;
	case 0xd6: EA = U + SIGNED(A); EAD = RM16(EAD); break;
	case 0xd7: EA = 0; break; /*ILLEGAL*/
	case 0xd8: IMMBYTE(EA); EA = U + SIGNED(EA); EAD = RM16(EAD); break;
	case 0xd9: IMMWORD(EAP); EA += U; EAD = RM16(EAD); break;
	case 0xda: EA = 0; break; /*ILLEGAL*/
	case 0xdb: EA = U + D; EAD = RM16(EAD); break;
	case 0xdc: IMMBYTE(EA); EA = PC + SIGNED(EA); EAD = RM16(EAD); break;
	case 0xdd: IMMWORD(EAP); EA += PC; EAD = RM16(EAD); break;
	case 0xde: EA = 0; break; /*ILLEGAL*/
	case 0xdf: IMMWORD(EAP); EAD = RM16(EAD); break;
	case 0xe0: EA = S; S++; break;
	case 0xe1: EA = S; S += 2; break;
	case 0xe2: S--; EA = S; break;
	case 0xe3: S -= 2; EA = S; break;
	case 0xe4: EA = S; break;
	case 0xe5: EA = S + SIGNED(B); break;
	case 0xe6: EA = S + SIGNED(A); break;
	case 0xe7: EA = 0; break; /*ILLEGAL*/
	case 0xe8: IMMBYTE(EA); EA = S + SIGNED(EA); break;
	case 0xe9: IMMWORD(EAP); EA += S; break;
	case 0xea: EA = 0; break; /*ILLEGAL*/
	case 0xeb: EA = S + D; break;
	case 0xec: IMMBYTE(EA); EA = PC + SIGNED(EA); break;
	case 0xed: IMMWORD(EAP); EA += PC; break;
	case 0xee: EA = 0; break; /*ILLEGAL*/
	case 0xef: IMMWORD(EAP); break;
	case 0xf0: EA = S; S++; EAD = RM16(EAD); break;
	case 0xf1: EA = S; S += 2; EAD = RM16(EAD); break;
	case 0xf2: S--; EA = S; EAD = RM16(EAD); break;
	case 0xf3: S -= 2; EA = S; EAD = RM16(EAD); break;
	case 0xf4: EA = S; EAD = RM16(EAD); break;
	case 0xf5: EA = S + SIGNED(B); EAD = RM16(EAD); break;
	case 0xf6: EA = S + SIGNED(A); EAD = RM16(EAD); break;
	case 0xf7: EA = 0; break; /*ILLEGAL*/
	case 0xf8: IMMBYTE(EA); EA = S + SIGNED(EA); EAD = RM16(EAD); break;
	case 0xf9: IMMWORD(EAP); EA += S; EAD = RM16(EAD); break;
	case 0xfa: EA = 0; break; /*ILLEGAL*/
	case 0xfb: EA = S + D; EAD = RM16(EAD); break;
	case 0xfc: IMMBYTE(EA); EA = PC + SIGNED(EA); EAD = RM16(EAD); break;
	case 0xfd: IMMWORD(EAP); EA += PC; EAD = RM16(EAD); break;
	case 0xfe: EA = 0; break; /*ILLEGAL*/
	case 0xff: IMMWORD(EAP); EAD = RM16(EAD); break;
	default: __assume(0);
	}
	icount -= index_cycle_em[postbyte];
}

void MC6809::illegal()
{
	//logerror("MC6809: illegal opcode at %04x\n", PC);
}

/* $00 NEG direct ?**** */
void MC6809::neg_di()
{
	uint16 r, t;
	DIRBYTE(t);
	r = -t;
	CLR_NZVC;
	SET_FLAGS8(0, t, r);
	WM(EAD, r);
}

/* $01 ILLEGAL, same as $00 */

/* $02 ILLEGAL, same as $03 */

/* $03 COM direct -**01 */
void MC6809::com_di()
{
	uint8 t;
	DIRBYTE(t);
	t = ~t;
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WM(EAD, t);
}

/* $04 LSR direct -0*-* */
void MC6809::lsr_di()
{
	uint8 t;
	DIRBYTE(t);
	CLR_NZC;
	CC |= (t & CC_C);
	t >>= 1;
	SET_Z8(t);
	WM(EAD, t);
}

/* $05 ILLEGAL, same as $04 */

/* $06 ROR direct -**-* */
void MC6809::ror_di()
{
	uint8 t, r;
	DIRBYTE(t);
	r =  (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	r |= t >> 1;
	SET_NZ8(r);
	WM(EAD, r);
}

/* $07 ASR direct ?**-* */
void MC6809::asr_di()
{
	uint8 t;
	DIRBYTE(t);
	CLR_NZC;
	CC |= (t & CC_C);
	t = (t & 0x80) | (t >> 1);
	SET_NZ8(t);
	WM(EAD, t);
}

/* $08 ASL direct ?**** */
void MC6809::asl_di()
{
	uint16 t, r;
	DIRBYTE(t);
	r = t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WM(EAD, r);
}

/* $09 ROL direct -**** */
void MC6809::rol_di()
{
	uint16 t, r;
	DIRBYTE(t);
	r = (CC & CC_C) | (t << 1);
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WM(EAD, r);
}

/* $0A DEC direct -***- */
void MC6809::dec_di()
{
	uint8 t;
	DIRBYTE(t);
	--t;
	CLR_NZV;
	SET_FLAGS8D(t);
	WM(EAD, t);
}

/* $0B ILLEGAL, same as $0A */

/* $OC INC direct -***- */
void MC6809::inc_di()
{
	uint8 t;
	DIRBYTE(t);
	++t;
	CLR_NZV;
	SET_FLAGS8I(t);
	WM(EAD, t);
}

/* $OD TST direct -**0- */
void MC6809::tst_di()
{
	uint8 t;
	DIRBYTE(t);
	CLR_NZV;
	SET_NZ8(t);
}

/* $0E JMP direct ----- */
void MC6809::jmp_di()
{
	DIRECT;
	PCD = EAD;
}

/* $0F CLR direct -0100 */
void MC6809::clr_di()
{
	DIRECT;
	(void)RM(EAD);
	WM(EAD, 0);
	CLR_NZVC;
	SEZ;
}

/* $10 FLAG */

/* $11 FLAG */

/* $12 NOP inherent ----- */
void MC6809::nop()
{
	;
}

/* $13 SYNC inherent ----- */
void MC6809::sync()
{
	/* SYNC stops processing instructions until an interrupt request happens. */
	/* This doesn't require the corresponding interrupt to be enabled: if it  */
	/* is disabled, execution continues with the next instruction.            */
	int_state |= MC6809_SYNC;	 /* HJB 990227 */
}

/* $14 ILLEGAL */

/* $15 ILLEGAL */

/* $16 LBRA relative ----- */
void MC6809::lbra()
{
	IMMWORD(EAP);
	PC += EA;
}

/* $17 LBSR relative ----- */
void MC6809::lbsr()
{
	IMMWORD(EAP);
	PUSHWORD(pPC);
	PC += EA;
}

/* $18 ILLEGAL */

/* $19 DAA inherent (A) -**0* */
void MC6809::daa()
{
	uint8 msn, lsn;
	uint16 t, cf = 0;
	msn = A & 0xf0; lsn = A & 0x0f;
	if(lsn > 0x09 || (CC & CC_H)) cf |= 0x06;
	if(msn > 0x80 && lsn > 0x09 ) cf |= 0x60;
	if(msn > 0x90 || (CC & CC_C)) cf |= 0x60;
	t = cf + A;
	CLR_NZV; /* keep carry from previous operation */
	SET_NZ8((uint8)t); SET_C8(t);
	A = (uint8)t;
}

/* $1A ORCC immediate ##### */
void MC6809::orcc()
{
	uint8 t;
	IMMBYTE(t);
	CC |= t;
}

/* $1B ILLEGAL */

/* $1C ANDCC immediate ##### */
void MC6809::andcc()
{
	uint8 t;
	IMMBYTE(t);
	CC &= t;
}

/* $1D SEX inherent -**-- */
void MC6809::sex()
{
	uint16 t;
	t = SIGNED(B);
	D = t;
	//  CLR_NZV;    Tim Lindner 20020905: verified that V flag is not affected
	CLR_NZ;
	SET_NZ16(t);
}

/* $1E EXG inherent ----- */
void MC6809::exg()
{
	uint16 t1, t2;
	uint8 tb;

	IMMBYTE(tb);
	if((tb ^ (tb >> 4)) & 0x08) {
		/* transfer $ff to both registers */
		t1 = t2 = 0xff;
	} else {
		switch(tb >> 4) {
		case  0: t1 = D;  break;
		case  1: t1 = X;  break;
		case  2: t1 = Y;  break;
		case  3: t1 = U;  break;
		case  4: t1 = S;  break;
		case  5: t1 = PC; break;
		case  8: t1 = A;  break;
		case  9: t1 = B;  break;
		case 10: t1 = CC; break;
		case 11: t1 = DP; break;
		default: t1 = 0xff;
		}
		switch(tb&15) {
		case  0: t2 = D;  break;
		case  1: t2 = X;  break;
		case  2: t2 = Y;  break;
		case  3: t2 = U;  break;
		case  4: t2 = S;  break;
		case  5: t2 = PC; break;
		case  8: t2 = A;  break;
		case  9: t2 = B;  break;
		case 10: t2 = CC; break;
		case 11: t2 = DP; break;
		default: t2 = 0xff;
		}
	}
	switch(tb >> 4) {
	case  0: D = t2;  break;
	case  1: X = t2;  break;
	case  2: Y = t2;  break;
	case  3: U = t2;  break;
	case  4: S = t2;  break;
	case  5: PC = t2; break;
	case  8: A = (uint8)t2;  break;
	case  9: B = (uint8)t2;  break;
	case 10: CC = (uint8)t2; break;
	case 11: DP = (uint8)t2; break;
	}
	switch(tb&15) {
	case  0: D = t1;  break;
	case  1: X = t1;  break;
	case  2: Y = t1;  break;
	case  3: U = t1;  break;
	case  4: S = t1;  break;
	case  5: PC = t1; break;
	case  8: A = (uint8)t1;  break;
	case  9: B = (uint8)t1;  break;
	case 10: CC = (uint8)t1; break;
	case 11: DP = (uint8)t1; break;
	}
}

/* $1F TFR inherent ----- */
void MC6809::tfr()
{
	uint8 tb;
	uint16 t;

	IMMBYTE(tb);
	if((tb ^ (tb >> 4)) & 0x08) {
		/* transfer $ff to register */
		t = 0xff;
	} else {
		switch(tb >> 4) {
		case  0: t = D;  break;
		case  1: t = X;  break;
		case  2: t = Y;  break;
		case  3: t = U;  break;
		case  4: t = S;  break;
		case  5: t = PC; break;
		case  8: t = A;  break;
		case  9: t = B;  break;
		case 10: t = CC; break;
		case 11: t = DP; break;
		default: t = 0xff;
		}
	}
	switch(tb&15) {
	case  0: D = t;  break;
	case  1: X = t;  break;
	case  2: Y = t;  break;
	case  3: U = t;  break;
	case  4: S = t;  break;
	case  5: PC = t; break;
	case  8: A = (uint8)t;  break;
	case  9: B = (uint8)t;  break;
	case 10: CC = (uint8)t; break;
	case 11: DP = (uint8)t; break;
	}
}

/* $20 BRA relative ----- */
void MC6809::bra()
{
	uint8 t;
	IMMBYTE(t);
	PC += SIGNED(t);
}

/* $21 BRN relative ----- */
void MC6809::brn()
{
	uint8 t;
	IMMBYTE(t);
}

/* $1021 LBRN relative ----- */
void MC6809::lbrn()
{
	IMMWORD(EAP);
}

/* $22 BHI relative ----- */
void MC6809::bhi()
{
	BRANCH(!(CC & (CC_Z | CC_C)));
}

/* $1022 LBHI relative ----- */
void MC6809::lbhi()
{
	LBRANCH(!(CC & (CC_Z | CC_C)));
}

/* $23 BLS relative ----- */
void MC6809::bls()
{
	BRANCH((CC & (CC_Z | CC_C)));
}

/* $1023 LBLS relative ----- */
void MC6809::lbls()
{
	LBRANCH((CC & (CC_Z | CC_C)));
}

/* $24 BCC relative ----- */
void MC6809::bcc()
{
	BRANCH(!(CC & CC_C));
}

/* $1024 LBCC relative ----- */
void MC6809::lbcc()
{
	LBRANCH(!(CC & CC_C));
}

/* $25 BCS relative ----- */
void MC6809::bcs()
{
	BRANCH((CC & CC_C));
}

/* $1025 LBCS relative ----- */
void MC6809::lbcs()
{
	LBRANCH((CC & CC_C));
}

/* $26 BNE relative ----- */
void MC6809::bne()
{
	BRANCH(!(CC & CC_Z));
}

/* $1026 LBNE relative ----- */
void MC6809::lbne()
{
	LBRANCH(!(CC & CC_Z));
}

/* $27 BEQ relative ----- */
void MC6809::beq()
{
	BRANCH((CC & CC_Z));
}

/* $1027 LBEQ relative ----- */
void MC6809::lbeq()
{
	LBRANCH((CC & CC_Z));
}

/* $28 BVC relative ----- */
void MC6809::bvc()
{
	BRANCH(!(CC & CC_V));
}

/* $1028 LBVC relative ----- */
void MC6809::lbvc()
{
	LBRANCH(!(CC & CC_V));
}

/* $29 BVS relative ----- */
void MC6809::bvs()
{
	BRANCH((CC & CC_V));
}

/* $1029 LBVS relative ----- */
void MC6809::lbvs()
{
	LBRANCH((CC & CC_V));
}

/* $2A BPL relative ----- */
void MC6809::bpl()
{
	BRANCH(!(CC & CC_N));
}

/* $102A LBPL relative ----- */
void MC6809::lbpl()
{
	LBRANCH(!(CC & CC_N));
}

/* $2B BMI relative ----- */
void MC6809::bmi()
{
	BRANCH((CC & CC_N));
}

/* $102B LBMI relative ----- */
void MC6809::lbmi()
{
	LBRANCH((CC & CC_N));
}

/* $2C BGE relative ----- */
void MC6809::bge()
{
	BRANCH(!NXORV);
}

/* $102C LBGE relative ----- */
void MC6809::lbge()
{
	LBRANCH(!NXORV);
}

/* $2D BLT relative ----- */
void MC6809::blt()
{
	BRANCH(NXORV);
}

/* $102D LBLT relative ----- */
void MC6809::lblt()
{
	LBRANCH(NXORV);
}

/* $2E BGT relative ----- */
void MC6809::bgt()
{
	BRANCH(!(NXORV || (CC & CC_Z)));
}

/* $102E LBGT relative ----- */
void MC6809::lbgt()
{
	LBRANCH(!(NXORV || (CC & CC_Z)));
}

/* $2F BLE relative ----- */
void MC6809::ble()
{
	BRANCH((NXORV || (CC & CC_Z)));
}

/* $102F LBLE relative ----- */
void MC6809::lble()
{
	LBRANCH((NXORV || (CC & CC_Z)));
}

/* $30 LEAX indexed --*-- */
void MC6809::leax()
{
	fetch_effective_address();
	X = EA;
	CLR_Z;
	SET_Z(X);
}

/* $31 LEAY indexed --*-- */
void MC6809::leay()
{
	fetch_effective_address();
	Y = EA;
	CLR_Z;
	SET_Z(Y);
}

/* $32 LEAS indexed ----- */
void MC6809::leas()
{
	fetch_effective_address();
	S = EA;
	int_state |= MC6809_LDS;
}

/* $33 LEAU indexed ----- */
void MC6809::leau()
{
	fetch_effective_address();
	U = EA;
}

/* $34 PSHS inherent ----- */
void MC6809::pshs()
{
	uint8 t;
	IMMBYTE(t);
	if(t & 0x80) { PUSHWORD(pPC); icount -= 2; }
	if(t & 0x40) { PUSHWORD(pU);  icount -= 2; }
	if(t & 0x20) { PUSHWORD(pY);  icount -= 2; }
	if(t & 0x10) { PUSHWORD(pX);  icount -= 2; }
	if(t & 0x08) { PUSHBYTE(DP);  icount -= 1; }
	if(t & 0x04) { PUSHBYTE(B);   icount -= 1; }
	if(t & 0x02) { PUSHBYTE(A);   icount -= 1; }
	if(t & 0x01) { PUSHBYTE(CC);  icount -= 1; }
}

/* 35 PULS inherent ----- */
void MC6809::puls()
{
	uint8 t;
	IMMBYTE(t);
	if(t & 0x01) { PULLBYTE(CC);  icount -= 1; }
	if(t & 0x02) { PULLBYTE(A);   icount -= 1; }
	if(t & 0x04) { PULLBYTE(B);   icount -= 1; }
	if(t & 0x08) { PULLBYTE(DP);  icount -= 1; }
	if(t & 0x10) { PULLWORD(XD);  icount -= 2; }
	if(t & 0x20) { PULLWORD(YD);  icount -= 2; }
	if(t & 0x40) { PULLWORD(UD);  icount -= 2; }
	if(t & 0x80) { PULLWORD(PCD); icount -= 2; }
}

/* $36 PSHU inherent ----- */
void MC6809::pshu()
{
	uint8 t;
	IMMBYTE(t);
	if(t & 0x80) { PSHUWORD(pPC); icount -= 2; }
	if(t & 0x40) { PSHUWORD(pS);  icount -= 2; }
	if(t & 0x20) { PSHUWORD(pY);  icount -= 2; }
	if(t & 0x10) { PSHUWORD(pX);  icount -= 2; }
	if(t & 0x08) { PSHUBYTE(DP);  icount -= 1; }
	if(t & 0x04) { PSHUBYTE(B);   icount -= 1; }
	if(t & 0x02) { PSHUBYTE(A);   icount -= 1; }
	if(t & 0x01) { PSHUBYTE(CC);  icount -= 1; }
}

/* 37 PULU inherent ----- */
void MC6809::pulu()
{
	uint8 t;
	IMMBYTE(t);
	if(t & 0x01) { PULUBYTE(CC);  icount -= 1; }
	if(t & 0x02) { PULUBYTE(A);   icount -= 1; }
	if(t & 0x04) { PULUBYTE(B);   icount -= 1; }
	if(t & 0x08) { PULUBYTE(DP);  icount -= 1; }
	if(t & 0x10) { PULUWORD(XD);  icount -= 2; }
	if(t & 0x20) { PULUWORD(YD);  icount -= 2; }
	if(t & 0x40) { PULUWORD(SD);  icount -= 2; }
	if(t & 0x80) { PULUWORD(PCD); icount -= 2; }
}

/* $38 ILLEGAL */

/* $39 RTS inherent ----- */
void MC6809::rts()
{
	PULLWORD(PCD);
}

/* $3A ABX inherent ----- */
void MC6809::abx()
{
	X += B;
}

/* $3B RTI inherent ##### */
void MC6809::rti()
{
	uint8 t;
	PULLBYTE(CC);
	t = CC & CC_E;		/* HJB 990225: entire state saved? */
	if(t) {
		icount -= 9;
		PULLBYTE(A);
		PULLBYTE(B);
		PULLBYTE(DP);
		PULLWORD(XD);
		PULLWORD(YD);
		PULLWORD(UD);
	}
	PULLWORD(PCD);
}

/* $3C CWAI inherent ----1 */
void MC6809::cwai()
{
	uint8 t;
	IMMBYTE(t);
	CC &= t;
	/*
	 * CWAI stacks the entire machine state on the hardware stack, 
	 * then waits for an interrupt; when the interrupt is taken
	 * later, the state is *not* saved again after CWAI.
	 */
	CC |= CC_E; 		/* HJB 990225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	int_state |= MC6809_CWAI;	 /* HJB 990228 */
}

/* $3D MUL inherent --*-@ */
void MC6809::mul()
{
	uint16 t;
	t = A * B;
	CLR_ZC; SET_Z16(t);
	if(t & 0x80) {
		SEC;
	}
	D = t;
}

/* $3E ILLEGAL */

/* $3F SWI (SWI2 SWI3) absolute indirect ----- */
void MC6809::swi()
{
	CC |= CC_E; 			/* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	CC |= CC_IF | CC_II;	/* inhibit FIRQ and IRQ */
	PCD = RM16(0xfffa);
}

/* $103F SWI2 absolute indirect ----- */
void MC6809::swi2()
{
	CC |= CC_E; 			/* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	PCD = RM16(0xfff4);
}

/* $113F SWI3 absolute indirect ----- */
void MC6809::swi3()
{
	CC |= CC_E; 			/* HJB 980225: save entire state */
	PUSHWORD(pPC);
	PUSHWORD(pU);
	PUSHWORD(pY);
	PUSHWORD(pX);
	PUSHBYTE(DP);
	PUSHBYTE(B);
	PUSHBYTE(A);
	PUSHBYTE(CC);
	PCD = RM16(0xfff2);
}

/* $40 NEGA inherent ?**** */
void MC6809::nega()
{
	uint16 r;
	r = -A;
	CLR_NZVC;
	SET_FLAGS8(0, A, r);
	A = (uint8)r;
}

/* $41 ILLEGAL, same as $40 */

/* $42 ILLEGAL, same as $43 */

/* $43 COMA inherent -**01 */
void MC6809::coma()
{
	A = ~A;
	CLR_NZV;
	SET_NZ8(A);
	SEC;
}

/* $44 LSRA inherent -0*-* */
void MC6809::lsra()
{
	CLR_NZC;
	CC |= (A & CC_C);
	A >>= 1;
	SET_Z8(A);
}

/* $45 ILLEGAL, same as $44 */

/* $46 RORA inherent -**-* */
void MC6809::rora()
{
	uint8 r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (A & CC_C);
	r |= A >> 1;
	SET_NZ8(r);
	A = r;
}

/* $47 ASRA inherent ?**-* */
void MC6809::asra()
{
	CLR_NZC;
	CC |= (A & CC_C);
	A = (A & 0x80) | (A >> 1);
	SET_NZ8(A);
}

/* $48 ASLA inherent ?**** */
void MC6809::asla()
{
	uint16 r;
	r = A << 1;
	CLR_NZVC;
	SET_FLAGS8(A, A, r);
	A = (uint8)r;
}

/* $49 ROLA inherent -**** */
void MC6809::rola()
{
	uint16 t, r;
	t = A;
	r = (CC & CC_C) | (t << 1);
	CLR_NZVC; SET_FLAGS8(t, t, r);
	A = (uint8)r;
}

/* $4A DECA inherent -***- */
void MC6809::deca()
{
	--A;
	CLR_NZV;
	SET_FLAGS8D(A);
}

/* $4B ILLEGAL, same as $4A */

/* $4C INCA inherent -***- */
void MC6809::inca()
{
	++A;
	CLR_NZV;
	SET_FLAGS8I(A);
}

/* $4D TSTA inherent -**0- */
void MC6809::tsta()
{
	CLR_NZV;
	SET_NZ8(A);
}

/* $4E ILLEGAL, same as $4F */

/* $4F CLRA inherent -0100 */
void MC6809::clra()
{
	A = 0;
	CLR_NZVC; SEZ;
}

/* $50 NEGB inherent ?**** */
void MC6809::negb()
{
	uint16 r;
	r = -B;
	CLR_NZVC;
	SET_FLAGS8(0, B, r);
	B = (uint8)r;
}

/* $51 ILLEGAL, same as $50 */

/* $52 ILLEGAL, same as $53 */

/* $53 COMB inherent -**01 */
void MC6809::comb()
{
	B = ~B;
	CLR_NZV;
	SET_NZ8(B);
	SEC;
}

/* $54 LSRB inherent -0*-* */
void MC6809::lsrb()
{
	CLR_NZC;
	CC |= (B & CC_C);
	B >>= 1;
	SET_Z8(B);
}

/* $55 ILLEGAL, same as $54 */

/* $56 RORB inherent -**-* */
void MC6809::rorb()
{
	uint8 r;
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (B & CC_C);
	r |= B >> 1;
	SET_NZ8(r);
	B = r;
}

/* $57 ASRB inherent ?**-* */
void MC6809::asrb()
{
	CLR_NZC;
	CC |= (B & CC_C);
	B = (B & 0x80) | (B >> 1);
	SET_NZ8(B);
}

/* $58 ASLB inherent ?**** */
void MC6809::aslb()
{
	uint16 r;
	r = B << 1;
	CLR_NZVC;
	SET_FLAGS8(B, B, r);
	B = (uint8)r;
}

/* $59 ROLB inherent -**** */
void MC6809::rolb()
{
	uint16 t, r;
	t = B;
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	B = (uint8)r;
}

/* $5A DECB inherent -***- */
void MC6809::decb()
{
	--B;
	CLR_NZV;
	SET_FLAGS8D(B);
}

/* $5B ILLEGAL, same as $5A */

/* $5C INCB inherent -***- */
void MC6809::incb()
{
	++B;
	CLR_NZV;
	SET_FLAGS8I(B);
}

/* $5D TSTB inherent -**0- */
void MC6809::tstb()
{
	CLR_NZV;
	SET_NZ8(B);
}

/* $5E ILLEGAL, same as $5F */

/* $5F CLRB inherent -0100 */
void MC6809::clrb()
{
	B = 0;
	CLR_NZVC; SEZ;
}

/* $60 NEG indexed ?**** */
void MC6809::neg_ix()
{
	uint16 r, t;
	fetch_effective_address();
	t = RM(EAD);
	r = -t;
	CLR_NZVC;
	SET_FLAGS8(0, t, r);
	WM(EAD, r);
}

/* $61 ILLEGAL, same as $60 */

/* $62 ILLEGAL, same as $63 */

/* $63 COM indexed -**01 */
void MC6809::com_ix()
{
	uint8 t;
	fetch_effective_address();
	t = ~RM(EAD);
	CLR_NZV;
	SET_NZ8(t);
	SEC;
	WM(EAD, t);
}

/* $64 LSR indexed -0*-* */
void MC6809::lsr_ix()
{
	uint8 t;
	fetch_effective_address();
	t = RM(EAD);
	CLR_NZC;
	CC |= (t & CC_C);
	t >>= 1; SET_Z8(t);
	WM(EAD, t);
}

/* $65 ILLEGAL, same as $64 */

/* $66 ROR indexed -**-* */
void MC6809::ror_ix()
{
	uint8 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = (CC & CC_C) << 7;
	CLR_NZC;
	CC |= (t & CC_C);
	r |= t >> 1; SET_NZ8(r);
	WM(EAD, r);
}

/* $67 ASR indexed ?**-* */
void MC6809::asr_ix()
{
	uint8 t;
	fetch_effective_address();
	t = RM(EAD);
	CLR_NZC;
	CC |= (t & CC_C);
	t = (t & 0x80) | (t >> 1);
	SET_NZ8(t);
	WM(EAD, t);
}

/* $68 ASL indexed ?**** */
void MC6809::asl_ix()
{
	uint16 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WM(EAD, r);
}

/* $69 ROL indexed -**** */
void MC6809::rol_ix()
{
	uint16 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = CC & CC_C;
	r |= t << 1;
	CLR_NZVC;
	SET_FLAGS8(t, t, r);
	WM(EAD, r);
}

/* $6A DEC indexed -***- */
void MC6809::dec_ix()
{
	uint8 t;
	fetch_effective_address();
	t = RM(EAD) - 1;
	CLR_NZV; SET_FLAGS8D(t);
	WM(EAD, t);
}

/* $6B ILLEGAL, same as $6A */

/* $6C INC indexed -***- */
void MC6809::inc_ix()
{
	uint8 t;
	fetch_effective_address();
	t = RM(EAD) + 1;
	CLR_NZV; SET_FLAGS8I(t);
	WM(EAD, t);
}

/* $6D TST indexed -**0- */
void MC6809::tst_ix()
{
	uint8 t;
	fetch_effective_address();
	t = RM(EAD);
	CLR_NZV;
	SET_NZ8(t);
}

/* $6E JMP indexed ----- */
void MC6809::jmp_ix()
{
	fetch_effective_address();
	PCD = EAD;
}

/* $6F CLR indexed -0100 */
void MC6809::clr_ix()
{
	fetch_effective_address();
	(void)RM(EAD);
	WM(EAD, 0);
	CLR_NZVC; SEZ;
}

/* $70 NEG extended ?**** */
void MC6809::neg_ex()
{
	uint16 r, t;
	EXTBYTE(t); r = -t;
	CLR_NZVC; SET_FLAGS8(0, t, r);
	WM(EAD, r);
}

/* $71 ILLEGAL, same as $70 */

/* $72 ILLEGAL, same as $73 */

/* $73 COM extended -**01 */
void MC6809::com_ex()
{
	uint8 t;
	EXTBYTE(t); t = ~t;
	CLR_NZV; SET_NZ8(t); SEC;
	WM(EAD, t);
}

/* $74 LSR extended -0*-* */
void MC6809::lsr_ex()
{
	uint8 t;
	EXTBYTE(t); CLR_NZC; CC |= (t & CC_C);
	t >>=1; SET_Z8(t);
	WM(EAD, t);
}

/* $75 ILLEGAL, same as $74 */

/* $76 ROR extended -**-* */
void MC6809::ror_ex()
{
	uint8 t, r;
	EXTBYTE(t); r = (CC & CC_C) << 7;
	CLR_NZC; CC |= (t & CC_C);
	r |= t >> 1; SET_NZ8(r);
	WM(EAD, r);
}

/* $77 ASR extended ?**-* */
void MC6809::asr_ex()
{
	uint8 t;
	EXTBYTE(t); CLR_NZC; CC |= (t & CC_C);
	t = (t & 0x80) | (t >> 1);
	SET_NZ8(t);
	WM(EAD, t);
}

/* $78 ASL extended ?**** */
void MC6809::asl_ex()
{
	uint16 t, r;
	EXTBYTE(t); r = t << 1;
	CLR_NZVC; SET_FLAGS8(t, t, r);
	WM(EAD, r);
}

/* $79 ROL extended -**** */
void MC6809::rol_ex()
{
	uint16 t, r;
	EXTBYTE(t); r = (CC & CC_C) | (t << 1);
	CLR_NZVC; SET_FLAGS8(t, t, r);
	WM(EAD, r);
}

/* $7A DEC extended -***- */
void MC6809::dec_ex()
{
	uint8 t;
	EXTBYTE(t); --t;
	CLR_NZV; SET_FLAGS8D(t);
	WM(EAD, t);
}

/* $7B ILLEGAL, same as $7A */

/* $7C INC extended -***- */
void MC6809::inc_ex()
{
	uint8 t;
	EXTBYTE(t); ++t;
	CLR_NZV; SET_FLAGS8I(t);
	WM(EAD, t);
}

/* $7D TST extended -**0- */
void MC6809::tst_ex()
{
	uint8 t;
	EXTBYTE(t); CLR_NZV; SET_NZ8(t);
}

/* $7E JMP extended ----- */
void MC6809::jmp_ex()
{
	EXTENDED;
	PCD = EAD;
}

/* $7F CLR extended -0100 */
void MC6809::clr_ex()
{
	EXTENDED;
	(void)RM(EAD);
	WM(EAD, 0);
	CLR_NZVC; SEZ;
}

/* $80 SUBA immediate ?**** */
void MC6809::suba_im()
{
	uint16 t, r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8)r;
}

/* $81 CMPA immediate ?**** */
void MC6809::cmpa_im()
{
	uint16 t, r;
	IMMBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
}

/* $82 SBCA immediate ?**** */
void MC6809::sbca_im()
{
	uint16 t, r;
	IMMBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8)r;
}

/* $83 SUBD (CMPD CMPU) immediate -**** */
void MC6809::subd_im()
{
	uint32 r, d;
	pair b;
	IMMWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $1083 CMPD immediate -**** */
void MC6809::cmpd_im()
{
	uint32 r, d;
	pair b;
	IMMWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $1183 CMPU immediate -**** */
void MC6809::cmpu_im()
{
	uint32 r, d;
	pair b;
	IMMWORD(b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $84 ANDA immediate -**0- */
void MC6809::anda_im()
{
	uint8 t;
	IMMBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $85 BITA immediate -**0- */
void MC6809::bita_im()
{
	uint8 t, r;
	IMMBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $86 LDA immediate -**0- */
void MC6809::lda_im()
{
	IMMBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* is this a legal instruction? */
/* $87 STA immediate -**0- */
void MC6809::sta_im()
{
	CLR_NZV;
	SET_NZ8(A);
	IMM8;
	WM(EAD, A);
}

/* $88 EORA immediate -**0- */
void MC6809::eora_im()
{
	uint8 t;
	IMMBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $89 ADCA immediate ***** */
void MC6809::adca_im()
{
	uint16 t, r;
	IMMBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8)r;
}

/* $8A ORA immediate -**0- */
void MC6809::ora_im()
{
	uint8 t;
	IMMBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $8B ADDA immediate ***** */
void MC6809::adda_im()
{
	uint16 t, r;
	IMMBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8)r;
}

/* $8C CMPX (CMPY CMPS) immediate -**** */
void MC6809::cmpx_im()
{
	uint32 r, d;
	pair b;
	IMMWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $108C CMPY immediate -**** */
void MC6809::cmpy_im()
{
	uint32 r, d;
	pair b;
	IMMWORD(b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $118C CMPS immediate -**** */
void MC6809::cmps_im()
{
	uint32 r, d;
	pair b;
	IMMWORD(b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $8D BSR ----- */
void MC6809::bsr()
{
	uint8 t;
	IMMBYTE(t);
	PUSHWORD(pPC);
	PC += SIGNED(t);
}

/* $8E LDX (LDY) immediate -**0- */
void MC6809::ldx_im()
{
	IMMWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $108E LDY immediate -**0- */
void MC6809::ldy_im()
{
	IMMWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* is this a legal instruction? */
/* $8F STX (STY) immediate -**0- */
void MC6809::stx_im()
{
	CLR_NZV;
	SET_NZ16(X);
	IMM16;
	WM16(EAD, &pX);
}

/* is this a legal instruction? */
/* $108F STY immediate -**0- */
void MC6809::sty_im()
{
	CLR_NZV;
	SET_NZ16(Y);
	IMM16;
	WM16(EAD, &pY);
}

/* $90 SUBA direct ?**** */
void MC6809::suba_di()
{
	uint16 t, r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8)r;
}

/* $91 CMPA direct ?**** */
void MC6809::cmpa_di()
{
	uint16 t, r;
	DIRBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
}

/* $92 SBCA direct ?**** */
void MC6809::sbca_di()
{
	uint16 t, r;
	DIRBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8)r;
}

/* $93 SUBD (CMPD CMPU) direct -**** */
void MC6809::subd_di()
{
	uint32 r, d;
	pair b;
	DIRWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $1093 CMPD direct -**** */
void MC6809::cmpd_di()
{
	uint32 r, d;
	pair b;
	DIRWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $1193 CMPU direct -**** */
void MC6809::cmpu_di()
{
	uint32 r, d;
	pair b;
	DIRWORD(b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(U, b.d, r);
}

/* $94 ANDA direct -**0- */
void MC6809::anda_di()
{
	uint8 t;
	DIRBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $95 BITA direct -**0- */
void MC6809::bita_di()
{
	uint8 t, r;
	DIRBYTE(t);
	r = A & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $96 LDA direct -**0- */
void MC6809::lda_di()
{
	DIRBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $97 STA direct -**0- */
void MC6809::sta_di()
{
	CLR_NZV;
	SET_NZ8(A);
	DIRECT;
	WM(EAD, A);
}

/* $98 EORA direct -**0- */
void MC6809::eora_di()
{
	uint8 t;
	DIRBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $99 ADCA direct ***** */
void MC6809::adca_di()
{
	uint16 t, r;
	DIRBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8)r;
}

/* $9A ORA direct -**0- */
void MC6809::ora_di()
{
	uint8 t;
	DIRBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $9B ADDA direct ***** */
void MC6809::adda_di()
{
	uint16 t, r;
	DIRBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8)r;
}

/* $9C CMPX (CMPY CMPS) direct -**** */
void MC6809::cmpx_di()
{
	uint32 r, d;
	pair b;
	DIRWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $109C CMPY direct -**** */
void MC6809::cmpy_di()
{
	uint32 r, d;
	pair b;
	DIRWORD(b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $119C CMPS direct -**** */
void MC6809::cmps_di()
{
	uint32 r, d;
	pair b;
	DIRWORD(b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $9D JSR direct ----- */
void MC6809::jsr_di()
{
	DIRECT;
	PUSHWORD(pPC);
	PCD = EAD;
}

/* $9E LDX (LDY) direct -**0- */
void MC6809::ldx_di()
{
	DIRWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $109E LDY direct -**0- */
void MC6809::ldy_di()
{
	DIRWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $9F STX (STY) direct -**0- */
void MC6809::stx_di()
{
	CLR_NZV;
	SET_NZ16(X);
	DIRECT;
	WM16(EAD, &pX);
}

/* $109F STY direct -**0- */
void MC6809::sty_di()
{
	CLR_NZV;
	SET_NZ16(Y);
	DIRECT;
	WM16(EAD, &pY);
}

/* $a0 SUBA indexed ?**** */
void MC6809::suba_ix()
{
	uint16 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8)r;
}

/* $a1 CMPA indexed ?**** */
void MC6809::cmpa_ix()
{
	uint16 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
}

/* $a2 SBCA indexed ?**** */
void MC6809::sbca_ix()
{
	uint16 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8)r;
}

/* $a3 SUBD (CMPD CMPU) indexed -**** */
void MC6809::subd_ix()
{
	uint32 r, d;
	pair b;
	fetch_effective_address();
	b.d=RM16(EAD);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $10a3 CMPD indexed -**** */
void MC6809::cmpd_ix()
{
	uint32 r, d;
	pair b;
	fetch_effective_address();
	b.d=RM16(EAD);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $11a3 CMPU indexed -**** */
void MC6809::cmpu_ix()
{
	uint32 r;
	pair b;
	fetch_effective_address();
	b.d=RM16(EAD);
	r = U - b.d;
	CLR_NZVC;
	SET_FLAGS16(U, b.d, r);
}

/* $a4 ANDA indexed -**0- */
void MC6809::anda_ix()
{
	fetch_effective_address();
	A &= RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a5 BITA indexed -**0- */
void MC6809::bita_ix()
{
	uint8 r;
	fetch_effective_address();
	r = A & RM(EAD);
	CLR_NZV;
	SET_NZ8(r);
}

/* $a6 LDA indexed -**0- */
void MC6809::lda_ix()
{
	fetch_effective_address();
	A = RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a7 STA indexed -**0- */
void MC6809::sta_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ8(A);
	WM(EAD, A);
}

/* $a8 EORA indexed -**0- */
void MC6809::eora_ix()
{
	fetch_effective_address();
	A ^= RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $a9 ADCA indexed ***** */
void MC6809::adca_ix()
{
	uint16 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8)r;
}

/* $aA ORA indexed -**0- */
void MC6809::ora_ix()
{
	fetch_effective_address();
	A |= RM(EAD);
	CLR_NZV;
	SET_NZ8(A);
}

/* $aB ADDA indexed ***** */
void MC6809::adda_ix()
{
	uint16 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8)r;
}

/* $aC CMPX (CMPY CMPS) indexed -**** */
void MC6809::cmpx_ix()
{
	uint32 r, d;
	pair b;
	fetch_effective_address();
	b.d=RM16(EAD);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $10aC CMPY indexed -**** */
void MC6809::cmpy_ix()
{
	uint32 r, d;
	pair b;
	fetch_effective_address();
	b.d=RM16(EAD);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $11aC CMPS indexed -**** */
void MC6809::cmps_ix()
{
	uint32 r, d;
	pair b;
	fetch_effective_address();
	b.d=RM16(EAD);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $aD JSR indexed ----- */
void MC6809::jsr_ix()
{
	fetch_effective_address();
	PUSHWORD(pPC);
	PCD = EAD;
}

/* $aE LDX (LDY) indexed -**0- */
void MC6809::ldx_ix()
{
	fetch_effective_address();
	X = RM16(EAD);
	CLR_NZV;
	SET_NZ16(X);
}

/* $10aE LDY indexed -**0- */
void MC6809::ldy_ix()
{
	fetch_effective_address();
	Y = RM16(EAD);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $aF STX (STY) indexed -**0- */
void MC6809::stx_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ16(X);
	WM16(EAD, &pX);
}

/* $10aF STY indexed -**0- */
void MC6809::sty_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ16(Y);
	WM16(EAD, &pY);
}

/* $b0 SUBA extended ?**** */
void MC6809::suba_ex()
{
	uint16 t, r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8)r;
}

/* $b1 CMPA extended ?**** */
void MC6809::cmpa_ex()
{
	uint16 t, r;
	EXTBYTE(t);
	r = A - t;
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
}

/* $b2 SBCA extended ?**** */
void MC6809::sbca_ex()
{
	uint16 t, r;
	EXTBYTE(t);
	r = A - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(A, t, r);
	A = (uint8)r;
}

/* $b3 SUBD (CMPD CMPU) extended -**** */
void MC6809::subd_ex()
{
	uint32 r, d;
	pair b;
	EXTWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $10b3 CMPD extended -**** */
void MC6809::cmpd_ex()
{
	uint32 r, d;
	pair b;
	EXTWORD(b);
	d = D;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $11b3 CMPU extended -**** */
void MC6809::cmpu_ex()
{
	uint32 r, d;
	pair b;
	EXTWORD(b);
	d = U;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $b4 ANDA extended -**0- */
void MC6809::anda_ex()
{
	uint8 t;
	EXTBYTE(t);
	A &= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b5 BITA extended -**0- */
void MC6809::bita_ex()
{
	uint8 t, r;
	EXTBYTE(t);
	r = A & t;
	CLR_NZV; SET_NZ8(r);
}

/* $b6 LDA extended -**0- */
void MC6809::lda_ex()
{
	EXTBYTE(A);
	CLR_NZV;
	SET_NZ8(A);
}

/* $b7 STA extended -**0- */
void MC6809::sta_ex()
{
	CLR_NZV;
	SET_NZ8(A);
	EXTENDED;
	WM(EAD, A);
}

/* $b8 EORA extended -**0- */
void MC6809::eora_ex()
{
	uint8 t;
	EXTBYTE(t);
	A ^= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $b9 ADCA extended ***** */
void MC6809::adca_ex()
{
	uint16 t, r;
	EXTBYTE(t);
	r = A + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8)r;
}

/* $bA ORA extended -**0- */
void MC6809::ora_ex()
{
	uint8 t;
	EXTBYTE(t);
	A |= t;
	CLR_NZV;
	SET_NZ8(A);
}

/* $bB ADDA extended ***** */
void MC6809::adda_ex()
{
	uint16 t, r;
	EXTBYTE(t);
	r = A + t;
	CLR_HNZVC;
	SET_FLAGS8(A, t, r);
	SET_H(A, t, r);
	A = (uint8)r;
}

/* $bC CMPX (CMPY CMPS) extended -**** */
void MC6809::cmpx_ex()
{
	uint32 r, d;
	pair b;
	EXTWORD(b);
	d = X;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $10bC CMPY extended -**** */
void MC6809::cmpy_ex()
{
	uint32 r, d;
	pair b;
	EXTWORD(b);
	d = Y;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $11bC CMPS extended -**** */
void MC6809::cmps_ex()
{
	uint32 r, d;
	pair b;
	EXTWORD(b);
	d = S;
	r = d - b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
}

/* $bD JSR extended ----- */
void MC6809::jsr_ex()
{
	EXTENDED;
	PUSHWORD(pPC);
	PCD = EAD;
}

/* $bE LDX (LDY) extended -**0- */
void MC6809::ldx_ex()
{
	EXTWORD(pX);
	CLR_NZV;
	SET_NZ16(X);
}

/* $10bE LDY extended -**0- */
void MC6809::ldy_ex()
{
	EXTWORD(pY);
	CLR_NZV;
	SET_NZ16(Y);
}

/* $bF STX (STY) extended -**0- */
void MC6809::stx_ex()
{
	CLR_NZV;
	SET_NZ16(X);
	EXTENDED;
	WM16(EAD, &pX);
}

/* $10bF STY extended -**0- */
void MC6809::sty_ex()
{
	CLR_NZV;
	SET_NZ16(Y);
	EXTENDED;
	WM16(EAD, &pY);
}

/* $c0 SUBB immediate ?**** */
void MC6809::subb_im()
{
	uint16 t, r;
	IMMBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8)r;
}

/* $c1 CMPB immediate ?**** */
void MC6809::cmpb_im()
{
	uint16 t, r;
	IMMBYTE(t);
	r = B - t;
	CLR_NZVC; SET_FLAGS8(B, t, r);
}

/* $c2 SBCB immediate ?**** */
void MC6809::sbcb_im()
{
	uint16 t, r;
	IMMBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8)r;
}

/* $c3 ADDD immediate -**** */
void MC6809::addd_im()
{
	uint32 r, d;
	pair b;
	IMMWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $c4 ANDB immediate -**0- */
void MC6809::andb_im()
{
	uint8 t;
	IMMBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $c5 BITB immediate -**0- */
void MC6809::bitb_im()
{
	uint8 t, r;
	IMMBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $c6 LDB immediate -**0- */
void MC6809::ldb_im()
{
	IMMBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* is this a legal instruction? */
/* $c7 STB immediate -**0- */
void MC6809::stb_im()
{
	CLR_NZV;
	SET_NZ8(B);
	IMM8;
	WM(EAD, B);
}

/* $c8 EORB immediate -**0- */
void MC6809::eorb_im()
{
	uint8 t;
	IMMBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $c9 ADCB immediate ***** */
void MC6809::adcb_im()
{
	uint16 t, r;
	IMMBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8)r;
}

/* $cA ORB immediate -**0- */
void MC6809::orb_im()
{
	uint8 t;
	IMMBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $cB ADDB immediate ***** */
void MC6809::addb_im()
{
	uint16 t, r;
	IMMBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8)r;
}

/* $cC LDD immediate -**0- */
void MC6809::ldd_im()
{
	IMMWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* is this a legal instruction? */
/* $cD STD immediate -**0- */
void MC6809::std_im()
{
	CLR_NZV;
	SET_NZ16(D);
	IMM16;
	WM16(EAD, &pD);
}

/* $cE LDU (LDS) immediate -**0- */
void MC6809::ldu_im()
{
	IMMWORD(pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10cE LDS immediate -**0- */
void MC6809::lds_im()
{
	IMMWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	int_state |= MC6809_LDS;
}

/* is this a legal instruction? */
/* $cF STU (STS) immediate -**0- */
void MC6809::stu_im()
{
	CLR_NZV;
	SET_NZ16(U);
	IMM16;
	WM16(EAD, &pU);
}

/* is this a legal instruction? */
/* $10cF STS immediate -**0- */
void MC6809::sts_im()
{
	CLR_NZV;
	SET_NZ16(S);
	IMM16;
	WM16(EAD, &pS);
}

/* $d0 SUBB direct ?**** */
void MC6809::subb_di()
{
	uint16 t, r;
	DIRBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8)r;
}

/* $d1 CMPB direct ?**** */
void MC6809::cmpb_di()
{
	uint16 t, r;
	DIRBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
}

/* $d2 SBCB direct ?**** */
void MC6809::sbcb_di()
{
	uint16 t, r;
	DIRBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8)r;
}

/* $d3 ADDD direct -**** */
void MC6809::addd_di()
{
	uint32 r, d;
	pair b;
	DIRWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $d4 ANDB direct -**0- */
void MC6809::andb_di()
{
	uint8 t;
	DIRBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $d5 BITB direct -**0- */
void MC6809::bitb_di()
{
	uint8 t, r;
	DIRBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $d6 LDB direct -**0- */
void MC6809::ldb_di()
{
	DIRBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $d7 STB direct -**0- */
void MC6809::stb_di()
{
	CLR_NZV;
	SET_NZ8(B);
	DIRECT;
	WM(EAD, B);
}

/* $d8 EORB direct -**0- */
void MC6809::eorb_di()
{
	uint8 t;
	DIRBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $d9 ADCB direct ***** */
void MC6809::adcb_di()
{
	uint16 t, r;
	DIRBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8)r;
}

/* $dA ORB direct -**0- */
void MC6809::orb_di()
{
	uint8 t;
	DIRBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $dB ADDB direct ***** */
void MC6809::addb_di()
{
	uint16 t, r;
	DIRBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8)r;
}

/* $dC LDD direct -**0- */
void MC6809::ldd_di()
{
	DIRWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $dD STD direct -**0- */
void MC6809::std_di()
{
	CLR_NZV;
	SET_NZ16(D);
	DIRECT;
	WM16(EAD, &pD);
}

/* $dE LDU (LDS) direct -**0- */
void MC6809::ldu_di()
{
	DIRWORD(pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10dE LDS direct -**0- */
void MC6809::lds_di()
{
	DIRWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	int_state |= MC6809_LDS;
}

/* $dF STU (STS) direct -**0- */
void MC6809::stu_di()
{
	CLR_NZV;
	SET_NZ16(U);
	DIRECT;
	WM16(EAD, &pU);
}

/* $10dF STS direct -**0- */
void MC6809::sts_di()
{
	CLR_NZV;
	SET_NZ16(S);
	DIRECT;
	WM16(EAD, &pS);
}

/* $e0 SUBB indexed ?**** */
void MC6809::subb_ix()
{
	uint16 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8)r;
}

/* $e1 CMPB indexed ?**** */
void MC6809::cmpb_ix()
{
	uint16 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
}

/* $e2 SBCB indexed ?**** */
void MC6809::sbcb_ix()
{
	uint16 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8)r;
}

/* $e3 ADDD indexed -**** */
void MC6809::addd_ix()
{
	uint32 r, d;
	pair b;
	fetch_effective_address();
	b.d=RM16(EAD);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $e4 ANDB indexed -**0- */
void MC6809::andb_ix()
{
	fetch_effective_address();
	B &= RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e5 BITB indexed -**0- */
void MC6809::bitb_ix()
{
	uint8 r;
	fetch_effective_address();
	r = B & RM(EAD);
	CLR_NZV;
	SET_NZ8(r);
}

/* $e6 LDB indexed -**0- */
void MC6809::ldb_ix()
{
	fetch_effective_address();
	B = RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e7 STB indexed -**0- */
void MC6809::stb_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ8(B);
	WM(EAD, B);
}

/* $e8 EORB indexed -**0- */
void MC6809::eorb_ix()
{
	fetch_effective_address();
	B ^= RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $e9 ADCB indexed ***** */
void MC6809::adcb_ix()
{
	uint16 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8)r;
}

/* $eA ORB indexed -**0- */
void MC6809::orb_ix()
{
	fetch_effective_address();
	B |= RM(EAD);
	CLR_NZV;
	SET_NZ8(B);
}

/* $eB ADDB indexed ***** */
void MC6809::addb_ix()
{
	uint16 t, r;
	fetch_effective_address();
	t = RM(EAD);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8)r;
}

/* $eC LDD indexed -**0- */
void MC6809::ldd_ix()
{
	fetch_effective_address();
	D = RM16(EAD);
	CLR_NZV; SET_NZ16(D);
}

/* $eD STD indexed -**0- */
void MC6809::std_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ16(D);
	WM16(EAD, &pD);
}

/* $eE LDU (LDS) indexed -**0- */
void MC6809::ldu_ix()
{
	fetch_effective_address();
	U = RM16(EAD);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10eE LDS indexed -**0- */
void MC6809::lds_ix()
{
	fetch_effective_address();
	S = RM16(EAD);
	CLR_NZV;
	SET_NZ16(S);
	int_state |= MC6809_LDS;
}

/* $eF STU (STS) indexed -**0- */
void MC6809::stu_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ16(U);
	WM16(EAD, &pU);
}

/* $10eF STS indexed -**0- */
void MC6809::sts_ix()
{
	fetch_effective_address();
	CLR_NZV;
	SET_NZ16(S);
	WM16(EAD, &pS);
}

/* $f0 SUBB extended ?**** */
void MC6809::subb_ex()
{
	uint16 t, r;
	EXTBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8)r;
}

/* $f1 CMPB extended ?**** */
void MC6809::cmpb_ex()
{
	uint16 t, r;
	EXTBYTE(t);
	r = B - t;
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
}

/* $f2 SBCB extended ?**** */
void MC6809::sbcb_ex()
{
	uint16 t, r;
	EXTBYTE(t);
	r = B - t - (CC & CC_C);
	CLR_NZVC;
	SET_FLAGS8(B, t, r);
	B = (uint8)r;
}

/* $f3 ADDD extended -**** */
void MC6809::addd_ex()
{
	uint32 r, d;
	pair b;
	EXTWORD(b);
	d = D;
	r = d + b.d;
	CLR_NZVC;
	SET_FLAGS16(d, b.d, r);
	D = r;
}

/* $f4 ANDB extended -**0- */
void MC6809::andb_ex()
{
	uint8 t;
	EXTBYTE(t);
	B &= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f5 BITB extended -**0- */
void MC6809::bitb_ex()
{
	uint8 t, r;
	EXTBYTE(t);
	r = B & t;
	CLR_NZV;
	SET_NZ8(r);
}

/* $f6 LDB extended -**0- */
void MC6809::ldb_ex()
{
	EXTBYTE(B);
	CLR_NZV;
	SET_NZ8(B);
}

/* $f7 STB extended -**0- */
void MC6809::stb_ex()
{
	CLR_NZV;
	SET_NZ8(B);
	EXTENDED;
	WM(EAD, B);
}

/* $f8 EORB extended -**0- */
void MC6809::eorb_ex()
{
	uint8 t;
	EXTBYTE(t);
	B ^= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $f9 ADCB extended ***** */
void MC6809::adcb_ex()
{
	uint16 t, r;
	EXTBYTE(t);
	r = B + t + (CC & CC_C);
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8)r;
}

/* $fA ORB extended -**0- */
void MC6809::orb_ex()
{
	uint8 t;
	EXTBYTE(t);
	B |= t;
	CLR_NZV;
	SET_NZ8(B);
}

/* $fB ADDB extended ***** */
void MC6809::addb_ex()
{
	uint16 t, r;
	EXTBYTE(t);
	r = B + t;
	CLR_HNZVC;
	SET_FLAGS8(B, t, r);
	SET_H(B, t, r);
	B = (uint8)r;
}

/* $fC LDD extended -**0- */
void MC6809::ldd_ex()
{
	EXTWORD(pD);
	CLR_NZV;
	SET_NZ16(D);
}

/* $fD STD extended -**0- */
void MC6809::std_ex()
{
	CLR_NZV;
	SET_NZ16(D);
	EXTENDED;
	WM16(EAD, &pD);
}

/* $fE LDU (LDS) extended -**0- */
void MC6809::ldu_ex()
{
	EXTWORD(pU);
	CLR_NZV;
	SET_NZ16(U);
}

/* $10fE LDS extended -**0- */
void MC6809::lds_ex()
{
	EXTWORD(pS);
	CLR_NZV;
	SET_NZ16(S);
	int_state |= MC6809_LDS;
}

/* $fF STU (STS) extended -**0- */
void MC6809::stu_ex()
{
	CLR_NZV;
	SET_NZ16(U);
	EXTENDED;
	WM16(EAD, &pU);
}

/* $10fF STS extended -**0- */
void MC6809::sts_ex()
{
	CLR_NZV;
	SET_NZ16(S);
	EXTENDED;
	WM16(EAD, &pS);
}

/* $10xx opcodes */
void MC6809::pref10()
{
	uint8 ireg2 = ROP(PCD);
	PC++;
	
	switch(ireg2) {
	case 0x21: lbrn(); icount -= 5; break;
	case 0x22: lbhi(); icount -= 5; break;
	case 0x23: lbls(); icount -= 5; break;
	case 0x24: lbcc(); icount -= 5; break;
	case 0x25: lbcs(); icount -= 5; break;
	case 0x26: lbne(); icount -= 5; break;
	case 0x27: lbeq(); icount -= 5; break;
	case 0x28: lbvc(); icount -= 5; break;
	case 0x29: lbvs(); icount -= 5; break;
	case 0x2a: lbpl(); icount -= 5; break;
	case 0x2b: lbmi(); icount -= 5; break;
	case 0x2c: lbge(); icount -= 5; break;
	case 0x2d: lblt(); icount -= 5; break;
	case 0x2e: lbgt(); icount -= 5; break;
	case 0x2f: lble(); icount -= 5; break;
	case 0x3f: swi2(); icount -= 20; break;
	case 0x83: cmpd_im(); icount -= 5; break;
	case 0x8c: cmpy_im(); icount -= 5; break;
	case 0x8e: ldy_im(); icount -= 4; break;
	case 0x8f: sty_im(); icount -= 4; break;
	case 0x93: cmpd_di(); icount -= 7; break;
	case 0x9c: cmpy_di(); icount -= 7; break;
	case 0x9e: ldy_di(); icount -= 6; break;
	case 0x9f: sty_di(); icount -= 6; break;
	case 0xa3: cmpd_ix(); icount -= 7; break;
	case 0xac: cmpy_ix(); icount -= 7; break;
	case 0xae: ldy_ix(); icount -= 6; break;
	case 0xaf: sty_ix(); icount -= 6; break;
	case 0xb3: cmpd_ex(); icount -= 8; break;
	case 0xbc: cmpy_ex(); icount -= 8; break;
	case 0xbe: ldy_ex(); icount -= 7; break;
	case 0xbf: sty_ex(); icount -= 7; break;
	case 0xce: lds_im(); icount -= 4; break;
	case 0xcf: sts_im(); icount -= 4; break;
	case 0xde: lds_di(); icount -= 6; break;
	case 0xdf: sts_di(); icount -= 6; break;
	case 0xee: lds_ix(); icount -= 6; break;
	case 0xef: sts_ix(); icount -= 6; break;
	case 0xfe: lds_ex(); icount -= 7; break;
	case 0xff: sts_ex(); icount -= 7; break;
	default: illegal(); break;
	}
}

/* $11xx opcodes */
void MC6809::pref11()
{
	uint8 ireg2 = ROP(PCD);
	PC++;
	
	switch(ireg2) {
	case 0x3f: swi3(); icount -= 20; break;
	case 0x83: cmpu_im(); icount -= 5; break;
	case 0x8c: cmps_im(); icount -= 5; break;
	case 0x93: cmpu_di(); icount -= 7; break;
	case 0x9c: cmps_di(); icount -= 7; break;
	case 0xa3: cmpu_ix(); icount -= 7; break;
	case 0xac: cmps_ix(); icount -= 7; break;
	case 0xb3: cmpu_ex(); icount -= 8; break;
	case 0xbc: cmps_ex(); icount -= 8; break;
	default: illegal(); break;
	}
}

