/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date   : 2011.05.06-

	[ MC6809 ]
*/

// Fixed IRQ/FIRQ by Mr.Sasaji at 2011.06.17

#ifndef _MC6809_CONSTS_H
#define _MC6809_CONSTS_H

#define MC6809_IRQ_BIT	1	/* IRQ line number  */
#define MC6809_FIRQ_BIT	2	/* FIRQ line number */
#define MC6809_NMI_BIT	4	/* NMI line number  */
#define MC6809_HALT_BIT	8	/* HALT line number  */

/* flag bits in the cc register */
#define MC6809_CWAI	 0x0010	/* set when CWAI is waiting for an interrupt */
#define MC6809_SYNC	 0x0020	/* set when SYNC is waiting for an interrupt */
#define MC6809_CWAI_IN	 0x0040	/* set when CWAI is waiting for an interrupt */
#define MC6809_CWAI_OUT	 0x0080	/* set when CWAI is waiting for an interrupt */
#define MC6809_SYNC_IN	 0x0100	/* set when SYNC is waiting for an interrupt */
#define MC6809_SYNC_OUT	 0x0200	/* set when SYNC is waiting for an interrupt */
#define MC6809_LDS	 0x0400	/* set when LDS occured at least once */
#define MC6809_NMI_LC	 0x1000	/* NMI割り込み信号3サイクル未満 */
#define MC6809_FIRQ_LC	 0x2000	/* FIRQ割り込み信号3サイクル未満 */
#define MC6809_IRQ_LC	 0x4000	/* IRQ割り込み信号3サイクル未満 */
#define MC6809_INSN_HALT 0x8000	/* IRQ割り込み信号3サイクル未満 */

#define CC_C	0x01		/* Carry */
#define CC_V	0x02		/* Overflow */
#define CC_Z	0x04		/* Zero */
#define CC_N	0x08		/* Negative */
#define CC_II	0x10		/* Inhibit IRQ */
#define CC_H	0x20		/* Half (auxiliary) carry */
#define CC_IF	0x40		/* Inhibit FIRQ */
#define CC_E	0x80		/* entire state pushed */

// Macros move from mc6809.cpp .
#define pPPC    ppc
#define pPC	pc
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



#define RM(Addr)	d_mem->read_data8(Addr & 0xffff)
#define WM(Addr,Value)	d_mem->write_data8(Addr & 0xffff, Value)

#define ROP(Addr)	d_mem->read_data8(Addr & 0xffff)
#define ROP_ARG(Addr)	d_mem->read_data8(Addr & 0xffff)

/* macros to access memory */
#define IMMBYTE(b)	b = ROP_ARG(PCD); PC++
#define IMMWORD(w)	w = RM16_PAIR(PCD); PC = (PC + 2) & 0xffff;

#define PUSHBYTE(b)	S = (S - 1) & 0xffff; WM(SD,b) ; 
#define PUSHWORD(w)	S = (S - 2) & 0xffff; WM16(SD, &w);
#define PULLBYTE(b)	b = RM(SD); S = (S + 1) & 0xffff;
#define PULLWORD(w)	w = RM16_PAIR(SD); SD = (SD + 2) & 0xffff;


#define PSHUBYTE(b)	U = (U - 1) & 0xffff; WM(UD, b);
#define PSHUWORD(w)	U = (U - 2) & 0xffff; WM16(UD, &w);
#define PULUBYTE(b)	b = RM(UD); U = (U + 1) & 0xffff
#define PULUWORD(w)	w = RM16_PAIR(UD); UD = (UD + 2) & 0xffff;


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

#define CLR_HNZVC	CC &= ~(CC_H | CC_N | CC_Z | CC_V | CC_C)
#define CLR_NZV 	CC &= ~(CC_N | CC_Z | CC_V)
#define CLR_NZ		CC &= ~(CC_N | CC_Z)
#define CLR_HNZC	CC &= ~(CC_H | CC_N | CC_Z | CC_C)
#define CLR_NZVC	CC &= ~(CC_N | CC_Z | CC_V | CC_C)
#define CLR_Z		CC &= ~(CC_Z)
#define CLR_NZC 	CC &= ~(CC_N | CC_Z | CC_C)
#define CLR_ZC		CC &= ~(CC_Z | CC_C)

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z8(a)		if((a & 0x00ff) == 0) SEZ
#define SET_Z16(a)		if((a & 0x00ffff) == 0) SEZ
//#define SET_N8(a)		CC |= ((a & 0x80) >> 4)
//#define SET_N16(a)		CC |= ((a & 0x8000) >> 12)
#define SET_H(a,b,r)		if(((a ^ b ^ r) & 0x10) != 0) SEH
#define SET_N8(a)       if(a & 0x80) SEN
#define SET_N16(a)       if(a & 0x8000) SEN
//#define SET_H(a,b,r)	if((a^b^r)&0x10) SEH

#define SET_C8(a)	if((a&0x0100) != 0) SEC
#define SET_C16(a)	if((a&0x010000) != 0) SEC
#define SET_V8(a,b,r)	if(((a^b^r^(r>>1))&0x80) != 0) SEV
#define SET_V16(a,b,r)	if(((a^b^r^(r>>1))&0x8000) != 0) SEV

#define SET_FLAGS8I(a)		{CC |= flags8i[a & 0xff];}
#define SET_FLAGS8D(a)		{CC |= flags8d[a & 0xff];}

/* combos */
#define SET_NZ8(a)		{SET_N8(a); SET_Z8(a);}
#define SET_NZ16(a)		{SET_N16(a); SET_Z16(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r); SET_Z8(r); SET_V8(a, b, r); SET_C8(r);}
#define SET_FLAGS16(a,b,r)	{SET_N16(r); SET_Z16(r); SET_V16(a, b, r); SET_C16(r);}
#define SET_HNZVC8(a,b,r)	{SET_H(a,b,r);SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_HNZVC16(a,b,r)	{SET_H(a,b,r);SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}


//#define NXORV		((CC & CC_N) ^ ((CC & CC_V) << 2))
#define NXORV			(((CC&CC_N)^((CC&CC_V)<<2)) !=0)
/* for treating an unsigned byte as a signed word */
#define SIGNED(b)	((uint16_t)((b & 0x80) ? (b | 0xff00) : (b & 0x00ff)))

   
   
/* macros for addressing modes (postbytes have their own code) */
#define DIRECT		EAD = DPD; IMMBYTE(ea.b.l)

#define IMM8		EAD = PCD; PC++
#define IMM16		EAD = PCD; PC += 2
#define EXTENDED	IMMWORD(EAP)

/* macros for convenience */
#define DIRBYTE(b)	{DIRECT;   b   = RM(EAD);  }
#define DIRWORD(w)	{DIRECT;   w = RM16_PAIR(EAD);}
#define EXTBYTE(b)	{EXTENDED; b   = RM(EAD);  }
#define EXTWORD(w)	{EXTENDED; w = RM16_PAIR(EAD);}

      
#endif //#ifndef _MC6809_CONSTS_H
