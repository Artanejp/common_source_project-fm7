
#ifndef __MC6800_CONSTS_H_
#define __MC6800_CONSTS_H_

#define INT_REQ_BIT	1
#define NMI_REQ_BIT	2

#define MC6800_WAI	8
#define HD6301_SLP	0x10

#define pPC	pc
#define pS	sp
#define pX	ix
#define pD	acc_d
#define pEA	ea

#define PC	pc.w.l
#define PCD	pc.d
#define S	sp.w.l
#define SD	sp.d
#define X	ix.w.l
#define D	acc_d.w.l
#define A	acc_d.b.h
#define B	acc_d.b.l
#define CC	cc

#define EAD	ea.d
#define EA	ea.w.l

//#define M_RDOP(Addr)		d_mem->read_data8(Addr)
//#define M_RDOP_ARG(Addr)	d_mem->read_data8(Addr)
#define M_RDOP(Addr)		RM(Addr)
#define M_RDOP_ARG(Addr)	RM(Addr)

/* macros to access memory */
#define IMMBYTE(b)	b = M_RDOP_ARG(PCD); PC++
#define IMMWORD(w)	w.b.h = M_RDOP_ARG(PCD); w.b.l = M_RDOP_ARG((PCD + 1) & 0xffff); PC += 2

#define PUSHBYTE(b)	WM(SD, b); --S
#define PUSHWORD(w)	WM(SD, w.b.l); --S; WM(SD, w.b.h); --S
#define PULLBYTE(b)	S++; b = RM(SD)
#define PULLWORD(w)	S++; w.b.h = RM(SD); S++; w.b.l = RM(SD)

#define CLR_HNZVC	CC &= 0xd0
#define CLR_NZV		CC &= 0xf1
#define CLR_HNZC	CC &= 0xd2
#define CLR_NZVC	CC &= 0xf0
#define CLR_NZ		CC &= 0xf3
#define CLR_Z		CC &= 0xfb
#define CLR_NZC		CC &= 0xf2
#define CLR_ZC		CC &= 0xfa
#define CLR_C		CC &= 0xfe

#define SET_Z(a)	if(!(a)) SEZ
#define SET_Z8(a)	SET_Z((uint8_t)(a))
#define SET_Z16(a)	SET_Z((uint16_t)(a))
#define SET_N8(a)	CC |= (((a) & 0x80) >> 4)
#define SET_N16(a)	CC |= (((a) & 0x8000) >> 12)
#define SET_H(a,b,r)	CC |= ((((a) ^ (b) ^ (r)) & 0x10) << 1)
#define SET_C8(a)	CC |= (((a) & 0x100) >> 8)
#define SET_C16(a)	CC |= (((a) & 0x10000) >> 16)
#define SET_V8(a,b,r)	CC |= ((((a) ^ (b) ^ (r) ^ ((r) >> 1)) & 0x80) >> 6)
#define SET_V16(a,b,r)	CC |= ((((a) ^ (b) ^ (r) ^ ((r) >> 1)) & 0x8000) >> 14)

#define SET_FLAGS8I(a)		{CC |= flags8i[(a) & 0xff];}
#define SET_FLAGS8D(a)		{CC |= flags8d[(a) & 0xff];}

/* combos */
#define SET_NZ8(a)		{SET_N8(a);  SET_Z8(a);}
#define SET_NZ16(a)		{SET_N16(a); SET_Z16(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r);  SET_Z8(r);  SET_V8(a,b,r);  SET_C8(r); }
#define SET_FLAGS16(a,b,r)	{SET_N16(r); SET_Z16(r); SET_V16(a,b,r); SET_C16(r);}

/* for treating an uint8_t as a signed int16_t */
#define SIGNED(b)	((int16_t)(b & 0x80 ? b | 0xff00 : b))

/* Macros for addressing modes */
#define DIRECT		IMMBYTE(EAD)
#define IMM8		EA = PC++
#define IMM16		{EA = PC; PC += 2;}
#define EXTENDED	IMMWORD(pEA)
#define INDEXED		{EA = X + (uint8_t)M_RDOP_ARG(PCD); PC++;}

/* macros to set status flags */
#define SEC	CC |= 0x01
#define CLC	CC &= 0xfe
#define SEZ	CC |= 0x04
#define CLZ	CC &= 0xfb
#define SEN	CC |= 0x08
#define CLN	CC &= 0xf7
#define SEV	CC |= 0x02
#define CLV	CC &= 0xfd
#define SEH	CC |= 0x20
#define CLH	CC &= 0xdf
#define SEI	CC |= 0x10
#define CLI	CC &= ~0x10

/* macros for convenience */
#define DIRBYTE(b)	{DIRECT;   b   = RM(EAD);  }
#define DIRWORD(w)	{DIRECT;   w.d = RM16(EAD);}
#define EXTBYTE(b)	{EXTENDED; b   = RM(EAD);  }
#define EXTWORD(w)	{EXTENDED; w.d = RM16(EAD);}

#define IDXBYTE(b)	{INDEXED;  b   = RM(EAD);  }
#define IDXWORD(w)	{INDEXED;  w.d = RM16(EAD);}

/* Macros for branch instructions */
#define BRANCH(f)	{IMMBYTE(t); if(f) {PC += SIGNED(t);}}
#define NXORV		((CC & 0x08) ^ ((CC & 0x02) << 2))

/* Note: don't use 0 cycles here for invalid opcodes so that we don't */
/* hang in an infinite loop if we hit one */

#endif /* __MC6800_CONSTS_H_ */
