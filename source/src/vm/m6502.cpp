/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2010.08.10-

	[ M6502 ]
*/

#include "m6502.h"
#include "../fileio.h"

// vectors
#define NMI_VEC	0xfffa
#define RST_VEC	0xfffc
#define IRQ_VEC	0xfffe

// flags
#define F_C	0x01
#define F_Z	0x02
#define F_I	0x04
#define F_D	0x08
#define F_B	0x10
#define F_T	0x20
#define F_V	0x40
#define F_N	0x80

// some shortcuts for improved readability
#define A	a
#define X	x
#define Y	y
#define P	p
#define S	sp.b.l
#define SPD	sp.d

#define SET_NZ(n) \
	if((n) == 0) \
		P = (P & ~F_N) | F_Z; \
	else \
		P = (P & ~(F_N | F_Z)) | ((n) & F_N)

#define SET_Z(n) \
	if((n) == 0) \
		P |= F_Z; \
	else \
		P &= ~F_Z

#define EAL ea.b.l
#define EAH ea.b.h
#define EAW ea.w.l
#define EAD ea.d

#define ZPL zp.b.l
#define ZPH zp.b.h
#define ZPW zp.w.l
#define ZPD zp.d

#define PCL pc.b.l
#define PCH pc.b.h
#define PCW pc.w.l
#define PCD pc.d

// virtual machine interface

#define RDMEM_ID(addr) d_mem->read_data8(addr)
#define WRMEM_ID(addr, data) d_mem->write_data8(addr, data)

#define RDOP() d_mem->read_data8(PCW++)
#define PEEKOP() d_mem->read_data8(PCW)
#define RDOPARG() d_mem->read_data8(PCW++)

#define RDMEM(addr) d_mem->read_data8(addr)
#define WRMEM(addr, data) d_mem->write_data8(addr, data)

#define CYCLES(c) icount -= (c)

// branch relative

#define BRA(cond) { \
	int8 tmp2 = RDOPARG(); \
	if(cond) { \
		RDMEM(PCW); \
		EAW = PCW + (int8)tmp2; \
		if(EAH != PCH) { \
			RDMEM((PCH << 8) | EAL) ; \
			CYCLES(1); \
		} \
		PCD = EAD; \
		CYCLES(1); \
	} \
}

// Helper macros to build the effective address

#define EA_ZPG \
	ZPL = RDOPARG(); \
	EAD = ZPD

#define EA_ZPX \
	ZPL = RDOPARG(); \
	RDMEM(ZPD); \
	ZPL = X + ZPL; \
	EAD = ZPD

#define EA_ZPY \
	ZPL = RDOPARG(); \
	RDMEM(ZPD); \
	ZPL = Y + ZPL; \
	EAD = ZPD

#define EA_ABS \
	EAL = RDOPARG(); \
	EAH = RDOPARG()

#define EA_ABX_P \
	EA_ABS; \
	if(EAL + X > 0xff) { \
		RDMEM((EAH << 8) | ((EAL + X) & 0xff)); \
		CYCLES(1); \
	} \
	EAW += X;

#define EA_ABX_NP \
	EA_ABS; \
	RDMEM((EAH << 8) | ((EAL + X) & 0xff)); \
	EAW += X

#define EA_ABY_P \
	EA_ABS; \
	if(EAL + Y > 0xff) { \
		RDMEM((EAH << 8) | ((EAL + Y) & 0xff)); \
		CYCLES(1); \
	} \
	EAW += Y;

#define EA_ABY_NP \
	EA_ABS; \
	RDMEM((EAH << 8) | ((EAL + Y) & 0xff)); \
	EAW += Y

#define EA_IDX \
	ZPL = RDOPARG(); \
	RDMEM(ZPD); \
	ZPL = ZPL + X; \
	EAL = RDMEM(ZPD); \
	ZPL++; \
	EAH = RDMEM(ZPD)

#define EA_IDY_P \
	ZPL = RDOPARG(); \
	EAL = RDMEM(ZPD); \
	ZPL++; \
	EAH = RDMEM(ZPD); \
	if(EAL + Y > 0xff) { \
		RDMEM((EAH << 8) | ((EAL + Y) & 0xff)); \
		CYCLES(1); \
	} \
	EAW += Y;

#define EA_IDY_NP \
	ZPL = RDOPARG(); \
	EAL = RDMEM(ZPD); \
	ZPL++; \
	EAH = RDMEM(ZPD); \
	RDMEM((EAH << 8) | ((EAL + Y) & 0xff)); \
	EAW += Y

#define EA_ZPI \
	ZPL = RDOPARG(); \
	EAL = RDMEM(ZPD); \
	ZPL++; \
	EAH = RDMEM(ZPD)

#define EA_IND \
	EA_ABS; \
	tmp = RDMEM(EAD); \
	EAL++; \
	EAH = RDMEM(EAD); \
	EAL = tmp

// read a value into tmp

#define RD_IMM		tmp = RDOPARG()
#define RD_DUM		RDMEM(PCW)
#define RD_ACC		tmp = A
#define RD_ZPG		EA_ZPG; tmp = RDMEM(EAD)
#define RD_ZPX		EA_ZPX; tmp = RDMEM(EAD)
#define RD_ZPY		EA_ZPY; tmp = RDMEM(EAD)
#define RD_ABS		EA_ABS; tmp = RDMEM(EAD)
#define RD_ABX_P	EA_ABX_P; tmp = RDMEM(EAD)
#define RD_ABX_NP	EA_ABX_NP; tmp = RDMEM(EAD)
#define RD_ABY_P	EA_ABY_P; tmp = RDMEM(EAD)
#define RD_ABY_NP	EA_ABY_NP; tmp = RDMEM(EAD)
#define RD_IDX		EA_IDX; tmp = RDMEM_ID(EAD)
#define RD_IDY_P	EA_IDY_P; tmp = RDMEM_ID(EAD)
#define RD_IDY_NP	EA_IDY_NP; tmp = RDMEM_ID(EAD)
#define RD_ZPI		EA_ZPI; tmp = RDMEM(EAD)

// write a value from tmp

#define WR_ZPG		EA_ZPG; WRMEM(EAD, tmp)
#define WR_ZPX		EA_ZPX; WRMEM(EAD, tmp)
#define WR_ZPY		EA_ZPY; WRMEM(EAD, tmp)
#define WR_ABS		EA_ABS; WRMEM(EAD, tmp)
#define WR_ABX_NP	EA_ABX_NP; WRMEM(EAD, tmp)
#define WR_ABY_NP	EA_ABY_NP; WRMEM(EAD, tmp)
#define WR_IDX		EA_IDX; WRMEM_ID(EAD, tmp)
#define WR_IDY_NP	EA_IDY_NP; WRMEM_ID(EAD, tmp)
#define WR_ZPI		EA_ZPI; WRMEM(EAD, tmp)

// dummy read from the last EA

#define RD_EA	RDMEM(EAD)

// write back a value from tmp to the last EA

#define WB_ACC	A = (uint8)tmp;
#define WB_EA	WRMEM(EAD, tmp)

// opcodes

#define PUSH(Rg) WRMEM(SPD, Rg); S--
#define PULL(Rg) S++; Rg = RDMEM(SPD)

#ifdef HAS_N2A03
#define ADC \
	{ \
		int c = (P & F_C); \
		int sum = A + tmp + c; \
		P &= ~(F_V | F_C); \
		if(~(A ^ tmp) & (A ^ sum) & F_N) { \
			P |= F_V; \
		} \
		if(sum & 0xff00) { \
			P |= F_C; \
		} \
		A = (uint8)sum; \
	} \
	SET_NZ(A)
#else
#define ADC \
	if(P & F_D) { \
		int c = (P & F_C); \
		int lo = (A & 0x0f) + (tmp & 0x0f) + c; \
		int hi = (A & 0xf0) + (tmp & 0xf0); \
		P &= ~(F_V | F_C | F_N | F_Z); \
		if(!((lo + hi) & 0xff)) { \
			P |= F_Z; \
		} \
		if(lo > 0x09) { \
			hi += 0x10; \
			lo += 0x06; \
		} \
		if(hi & 0x80) { \
			P |= F_N; \
		} \
		if(~(A ^ tmp) & (A ^ hi) & F_N) { \
			P |= F_V; \
		} \
		if(hi > 0x90) { \
			hi += 0x60; \
		} \
		if(hi & 0xff00) { \
			P |= F_C; \
		} \
		A = (lo & 0x0f) + (hi & 0xf0); \
	} else { \
		int c = (P & F_C); \
		int sum = A + tmp + c; \
		P &= ~(F_V | F_C); \
		if(~(A ^ tmp) & (A ^ sum) & F_N) { \
			P |= F_V; \
		} \
		if(sum & 0xff00) { \
			P |= F_C; \
		} \
		A = (uint8)sum; \
		SET_NZ(A); \
	}
#endif

#define AND \
	A = (uint8)(A & tmp); \
	SET_NZ(A)

#define ASL \
	P = (P & ~F_C) | ((tmp >> 7) & F_C); \
	tmp = (uint8)(tmp << 1); \
	SET_NZ(tmp)

#define BCC BRA(!(P & F_C))
#define BCS BRA(P & F_C)
#define BEQ BRA(P & F_Z)

#define BIT \
	P &= ~(F_N | F_V | F_Z); \
	P |= tmp & (F_N | F_V); \
	if((tmp & A) == 0) \
		P |= F_Z

#define BMI BRA(P & F_N)
#define BNE BRA(!(P & F_Z))
#define BPL BRA(!(P & F_N))

#define BRK \
	RDOPARG(); \
	PUSH(PCH); \
	PUSH(PCL); \
	PUSH(P | F_B); \
	P = (P | F_I); \
	PCL = RDMEM(IRQ_VEC); \
	PCH = RDMEM(IRQ_VEC + 1)

#define BVC BRA(!(P & F_V))
#define BVS BRA(P & F_V)

#define CLC P &= ~F_C
#define CLD P &= ~F_D
#define CLI \
	if(irq_state && (P & F_I)) { \
		if(PEEKOP() != 0x40) { \
			after_cli = true; \
		} \
	} \
	P &= ~F_I
#define CLV P &= ~F_V

#define CMP \
	P &= ~F_C; \
	if(A >= tmp) { \
		P |= F_C; \
	} \
	SET_NZ((uint8)(A - tmp))
#define CPX \
	P &= ~F_C; \
	if(X >= tmp) { \
		P |= F_C; \
	} \
	SET_NZ((uint8)(X - tmp))
#define CPY \
	P &= ~F_C; \
	if(Y >= tmp) { \
		P |= F_C; \
	} \
	SET_NZ((uint8)(Y - tmp))

#define DEC \
	tmp = (uint8)(tmp - 1); \
	SET_NZ(tmp)
#define DEX \
	X = (uint8)(X - 1); \
	SET_NZ(X)
#define DEY \
	Y = (uint8)(Y - 1); \
	SET_NZ(Y)

#define EOR \
	A = (uint8)(A ^ tmp); \
	SET_NZ(A)

#define INC \
	tmp = (uint8)(tmp + 1); \
	SET_NZ(tmp)
#define INX \
	X = (uint8)(X + 1); \
	SET_NZ(X)
#define INY \
	Y = (uint8)(Y + 1); \
	SET_NZ(Y)

#define JMP PCD = EAD
#define JSR \
	EAL = RDOPARG(); \
	RDMEM(SPD); \
	PUSH(PCH); \
	PUSH(PCL); \
	EAH = RDOPARG(); \
	PCD = EAD

#define LDA \
	A = (uint8)tmp; \
	SET_NZ(A)
#define LDX \
	X = (uint8)tmp; \
	SET_NZ(X)
#define LDY \
	Y = (uint8)tmp; \
	SET_NZ(Y)

#define LSR \
	P = (P & ~F_C) | (tmp & F_C); \
	tmp = (uint8)tmp >> 1; \
	SET_NZ(tmp)

#define NOP

#define ORA \
	A = (uint8)(A | tmp); \
	SET_NZ(A)

#define PHA PUSH(A)
#define PHP PUSH(P)

#define PLA \
	RDMEM(SPD); \
	PULL(A); \
	SET_NZ(A)
#define PLP \
	RDMEM(SPD); \
	if(P & F_I) { \
		PULL(P); \
		if(irq_state && !(P & F_I)) { \
			after_cli = true; \
		} \
	} else { \
		PULL(P); \
	} \
	P |= (F_T | F_B);

#define ROL \
	tmp = (tmp << 1) | (P & F_C); \
	P = (P & ~F_C) | ((tmp >> 8) & F_C); \
	tmp = (uint8)tmp; \
	SET_NZ(tmp)
#define ROR \
	tmp |= (P & F_C) << 8; \
	P = (P & ~F_C) | (tmp & F_C); \
	tmp = (uint8)(tmp >> 1); \
	SET_NZ(tmp)

#define RTI \
	RDOPARG(); \
	RDMEM(SPD); \
	PULL(P); \
	PULL(PCL); \
	PULL(PCH); \
	P |= F_T | F_B; \
	if(irq_state && !(P & F_I)) { \
		after_cli = true; \
	}
#define RTS \
	RDOPARG(); \
	RDMEM(SPD); \
	PULL(PCL); \
	PULL(PCH); \
	RDMEM(PCW); \
	PCW++

#ifdef HAS_N2A03
#define SBC \
	{ \
		int c = (P & F_C) ^ F_C; \
		int sum = A - tmp - c; \
		P &= ~(F_V | F_C); \
		if((A ^ tmp) & (A ^ sum) & F_N) { \
			P |= F_V; \
		} \
		if((sum & 0xff00) == 0) { \
			P |= F_C; \
		} \
		A = (uint8)sum; \
	} \
	SET_NZ(A)
#else
#define SBC \
	if(P & F_D) { \
		int c = (P & F_C) ^ F_C; \
		int sum = A - tmp - c; \
		int lo = (A & 0x0f) - (tmp & 0x0f) - c; \
		int hi = (A & 0xf0) - (tmp & 0xf0); \
		if(lo & 0x10) { \
			lo -= 6; \
			hi--; \
		} \
		P &= ~(F_V | F_C | F_Z | F_N); \
		if((A ^ tmp) & (A ^ sum) & F_N) { \
			P |= F_V; \
		} \
		if(hi & 0x0100) { \
			hi -= 0x60; \
		} \
		if((sum & 0xff00) == 0) { \
			P |= F_C; \
		} \
		if(!((A - tmp - c) & 0xff)) { \
			P |= F_Z; \
		}
		if((A - tmp - c) & 0x80) { \
			P |= F_N; \
		} \
		A = (lo & 0x0f) | (hi & 0xf0); \
	} else { \
		int c = (P & F_C) ^ F_C; \
		int sum = A - tmp - c; \
		P &= ~(F_V | F_C); \
		if((A ^ tmp) & (A ^ sum) & F_N) { \
			P |= F_V; \
		} \
		if((sum & 0xff00) == 0) { \
			P |= F_C; \
		} \
		A = (uint8)sum; \
		SET_NZ(A); \
	}
#endif

#define SEC P |= F_C
#define SED P |= F_D
#define SEI P |= F_I

#define STA tmp = A
#define STX tmp = X
#define STY tmp = Y

#define TAX \
	X = A; \
	SET_NZ(X)
#define TAY \
	Y = A; \
	SET_NZ(Y)
#define TSX \
	X = S; \
	SET_NZ(X)
#define TXA \
	A = X; \
	SET_NZ(A)
#define TXS S = X
#define TYA \
	A = Y; \
	SET_NZ(A)

#define ANC \
	P &= ~F_C; \
	A = (uint8)(A & tmp); \
	if(A & 0x80) { \
		P |= F_C; \
	} \
	SET_NZ(A)

#define ASR \
	tmp &= A; \
	LSR

#define AST \
	S &= tmp; \
	A = X = S; \
	SET_NZ(A)

#ifdef HAS_N2A03
#define ARR \
	{ \
		tmp &= A; \
		ROR; \
		P &=~(F_V| F_C); \
		if(tmp & 0x40) { \
			P |= F_C; \
		} \
		if((tmp & 0x60) == 0x20 || (tmp & 0x60) == 0x40) { \
			P |= F_V; \
		} \
	}
#else
#define ARR \
	if(P & F_D) { \
		tmp &= A; \
		int t = tmp; \
		int hi = tmp & 0xf0; \
		int lo = tmp & 0x0f; \
		if(P & F_C) { \
			tmp = (tmp >> 1) | 0x80; \
			P |= F_N; \
		} else { \
			tmp >>= 1; \
			P &= ~F_N; \
		} \
		if(tmp) { \
			P &= ~F_Z; \
		} else { \
			P |= F_Z; \
		} \
		if((t ^ tmp) & 0x40) { \
			P |= F_V; \
		} else { \
			P &= ~F_V; \
		} \
		if(lo + (lo & 0x01) > 0x05) { \
			tmp = (tmp & 0xf0) | ((tmp + 6) & 0x0f); \
		} \
		if(hi + (hi & 0x10) > 0x50) { \
			P |= F_C; \
			tmp = (tmp+0x60) & 0xff; \
		} else { \
			P &= ~F_C; \
		} \
	} else { \
		tmp &= A; \
		ROR; \
		P &=~(F_V| F_C); \
		if(tmp & 0x40) { \
			P |= F_C; \
		} \
		if((tmp & 0x60) == 0x20 || (tmp & 0x60) == 0x40) { \
			P |= F_V; \
		} \
	}
#endif

#define ASX \
	P &= ~F_C; \
	X &= A; \
	if(X >= tmp) { \
		P |= F_C; \
	} \
	X = (uint8)(X - tmp); \
	SET_NZ(X)

#define AXA \
	A = (uint8)((A | 0xee) & X & tmp); \
	SET_NZ(A)

#define DCP \
	tmp = (uint8)(tmp - 1); \
	P &= ~F_C; \
	if(A >= tmp) { \
		P |= F_C; \
	} \
	SET_NZ((uint8)(A - tmp))

#define DOP RDOPARG()

#define ISB \
	tmp = (uint8)(tmp + 1); \
	SBC

#define LAX \
	A = X = (uint8)tmp; \
	SET_NZ(A)

#ifdef HAS_N2A03
#define OAL \
	A = X = (uint8)((A | 0xff) & tmp); \
	SET_NZ(A)
#else
#define OAL \
	A = X = (uint8)((A | 0xee) & tmp); \
	SET_NZ(A)
#endif

#define RLA \
	tmp = (tmp << 1) | (P & F_C); \
	P = (P & ~F_C) | ((tmp >> 8) & F_C); \
	tmp = (uint8)tmp; \
	A &= tmp; \
	SET_NZ(A)
#define RRA \
	tmp |= (P & F_C) << 8; \
	P = (P & ~F_C) | (tmp & F_C); \
	tmp = (uint8)(tmp >> 1); \
	ADC

#define SAX tmp = A & X

#define SLO \
	P = (P & ~F_C) | ((tmp >> 7) & F_C); \
	tmp = (uint8)(tmp << 1); \
	A |= tmp; \
	SET_NZ(A)

#define SRE \
	P = (P & ~F_C) | (tmp & F_C); \
	tmp = (uint8)tmp >> 1; \
	A ^= tmp; \
	SET_NZ(A)

#define SAH tmp = A & X & (EAH + 1)

#define SSH \
	S = A & X; \
	tmp = S & (EAH + 1)

#ifdef HAS_N2A03
#define SXH \
	if(Y && Y > EAL) { \
		EAH |= (Y << 1); \
	} \
	tmp = X & (EAH + 1)
#define SYH \
	if(X && X > EAL) { \
		EAH |= (X << 1); \
	} \
	tmp = Y & (EAH + 1)
#else
#define SXH tmp = X & (EAH + 1)
#define SYH tmp = Y & (EAH + 1)
#endif

#define TOP PCW += 2
#define KIL PCW--

void M6502::OP(uint8 code)
{
	int tmp;
	
	switch(code) {
	case 0x00: { CYCLES(7);                    BRK;            } break; /* 7 BRK */
	case 0x01: { CYCLES(6); RD_IDX;            ORA;            } break; /* 6 ORA IDX */
	case 0x02: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
	case 0x03: { CYCLES(7); RD_IDX;    RD_EA;  SLO; WB_EA;     } break; /* 7 SLO IDX */
	case 0x04: { CYCLES(3); RD_ZPG;            NOP;            } break; /* 3 NOP ZPG */
	case 0x05: { CYCLES(3); RD_ZPG;            ORA;            } break; /* 3 ORA ZPG */
	case 0x06: { CYCLES(5); RD_ZPG;    RD_EA;  ASL; WB_EA;     } break; /* 5 ASL ZPG */
	case 0x07: { CYCLES(5); RD_ZPG;    RD_EA;  SLO; WB_EA;     } break; /* 5 SLO ZPG */
	case 0x08: { CYCLES(3); RD_DUM;            PHP;            } break; /* 3 PHP */
	case 0x09: { CYCLES(2); RD_IMM;            ORA;            } break; /* 2 ORA IMM */
	case 0x0a: { CYCLES(2); RD_DUM;    RD_ACC; ASL; WB_ACC;    } break; /* 2 ASL A */
	case 0x0b: { CYCLES(2); RD_IMM;            ANC;            } break; /* 2 ANC IMM */
	case 0x0c: { CYCLES(4); RD_ABS;            NOP;            } break; /* 4 NOP ABS */
	case 0x0d: { CYCLES(4); RD_ABS;            ORA;            } break; /* 4 ORA ABS */
	case 0x0e: { CYCLES(6); RD_ABS;    RD_EA;  ASL; WB_EA;     } break; /* 6 ASL ABS */
	case 0x0f: { CYCLES(6); RD_ABS;    RD_EA;  SLO; WB_EA;     } break; /* 6 SLO ABS */
	case 0x10: { CYCLES(2);                    BPL;            } break; /* 2-4 BPL REL */
	case 0x11: { CYCLES(5); RD_IDY_P;          ORA;            } break; /* 5 ORA IDY page penalty */
	case 0x12: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
	case 0x13: { CYCLES(7); RD_IDY_NP; RD_EA;  SLO; WB_EA;     } break; /* 7 SLO IDY */
	case 0x14: { CYCLES(4); RD_ZPX;            NOP;            } break; /* 4 NOP ZPX */
	case 0x15: { CYCLES(4); RD_ZPX;            ORA;            } break; /* 4 ORA ZPX */
	case 0x16: { CYCLES(6); RD_ZPX;    RD_EA;  ASL; WB_EA;     } break; /* 6 ASL ZPX */
	case 0x17: { CYCLES(6); RD_ZPX;    RD_EA;  SLO; WB_EA;     } break; /* 6 SLO ZPX */
	case 0x18: { CYCLES(2); RD_DUM;            CLC;            } break; /* 2 CLC */
	case 0x19: { CYCLES(4); RD_ABY_P;          ORA;            } break; /* 4 ORA ABY page penalty */
	case 0x1a: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
	case 0x1b: { CYCLES(7); RD_ABY_NP; RD_EA;  SLO; WB_EA;     } break; /* 7 SLO ABY */
	case 0x1c: { CYCLES(4); RD_ABX_P;          NOP;            } break; /* 4 NOP ABX page penalty */
	case 0x1d: { CYCLES(4); RD_ABX_P;          ORA;            } break; /* 4 ORA ABX page penalty */
	case 0x1e: { CYCLES(7); RD_ABX_NP; RD_EA;  ASL; WB_EA;     } break; /* 7 ASL ABX */
	case 0x1f: { CYCLES(7); RD_ABX_NP; RD_EA;  SLO; WB_EA;     } break; /* 7 SLO ABX */
	case 0x20: { CYCLES(6);                    JSR;            } break; /* 6 JSR */
	case 0x21: { CYCLES(6); RD_IDX;            AND;            } break; /* 6 AND IDX */
	case 0x22: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
	case 0x23: { CYCLES(7); RD_IDX;    RD_EA;  RLA; WB_EA;     } break; /* 7 RLA IDX */
	case 0x24: { CYCLES(3); RD_ZPG;            BIT;            } break; /* 3 BIT ZPG */
	case 0x25: { CYCLES(3); RD_ZPG;            AND;            } break; /* 3 AND ZPG */
	case 0x26: { CYCLES(5); RD_ZPG;    RD_EA;  ROL; WB_EA;     } break; /* 5 ROL ZPG */
	case 0x27: { CYCLES(5); RD_ZPG;    RD_EA;  RLA; WB_EA;     } break; /* 5 RLA ZPG */
	case 0x28: { CYCLES(4); RD_DUM;            PLP;            } break; /* 4 PLP */
	case 0x29: { CYCLES(2); RD_IMM;            AND;            } break; /* 2 AND IMM */
	case 0x2a: { CYCLES(2); RD_DUM;    RD_ACC; ROL; WB_ACC;    } break; /* 2 ROL A */
	case 0x2b: { CYCLES(2); RD_IMM;            ANC;            } break; /* 2 ANC IMM */
	case 0x2c: { CYCLES(4); RD_ABS;            BIT;            } break; /* 4 BIT ABS */
	case 0x2d: { CYCLES(4); RD_ABS;            AND;            } break; /* 4 AND ABS */
	case 0x2e: { CYCLES(6); RD_ABS;    RD_EA;  ROL; WB_EA;     } break; /* 6 ROL ABS */
	case 0x2f: { CYCLES(6); RD_ABS;    RD_EA;  RLA; WB_EA;     } break; /* 6 RLA ABS */
	case 0x30: { CYCLES(2);                    BMI;            } break; /* 2-4 BMI REL */
	case 0x31: { CYCLES(5); RD_IDY_P;          AND;            } break; /* 5 AND IDY page penalty */
	case 0x32: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
	case 0x33: { CYCLES(7); RD_IDY_NP; RD_EA;  RLA; WB_EA;     } break; /* 7 RLA IDY */
	case 0x34: { CYCLES(4); RD_ZPX;            NOP;            } break; /* 4 NOP ZPX */
	case 0x35: { CYCLES(4); RD_ZPX;            AND;            } break; /* 4 AND ZPX */
	case 0x36: { CYCLES(6); RD_ZPX;    RD_EA;  ROL; WB_EA;     } break; /* 6 ROL ZPX */
	case 0x37: { CYCLES(6); RD_ZPX;    RD_EA;  RLA; WB_EA;     } break; /* 6 RLA ZPX */
	case 0x38: { CYCLES(2); RD_DUM;            SEC;            } break; /* 2 SEC */
	case 0x39: { CYCLES(4); RD_ABY_P;          AND;            } break; /* 4 AND ABY page penalty */
	case 0x3a: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
	case 0x3b: { CYCLES(7); RD_ABY_NP; RD_EA;  RLA; WB_EA;     } break; /* 7 RLA ABY */
	case 0x3c: { CYCLES(4); RD_ABX_P;          NOP;            } break; /* 4 NOP ABX page penalty */
	case 0x3d: { CYCLES(4); RD_ABX_P;          AND;            } break; /* 4 AND ABX page penalty */
	case 0x3e: { CYCLES(7); RD_ABX_NP; RD_EA;  ROL; WB_EA;     } break; /* 7 ROL ABX */
	case 0x3f: { CYCLES(7); RD_ABX_NP; RD_EA;  RLA; WB_EA;     } break; /* 7 RLA ABX */
	case 0x40: { CYCLES(6);                    RTI;            } break; /* 6 RTI */
	case 0x41: { CYCLES(6); RD_IDX;            EOR;            } break; /* 6 EOR IDX */
	case 0x42: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
	case 0x43: { CYCLES(7); RD_IDX;    RD_EA;  SRE; WB_EA;     } break; /* 7 SRE IDX */
	case 0x44: { CYCLES(3); RD_ZPG;            NOP;            } break; /* 3 NOP ZPG */
	case 0x45: { CYCLES(3); RD_ZPG;            EOR;            } break; /* 3 EOR ZPG */
	case 0x46: { CYCLES(5); RD_ZPG;    RD_EA;  LSR; WB_EA;     } break; /* 5 LSR ZPG */
	case 0x47: { CYCLES(5); RD_ZPG;    RD_EA;  SRE; WB_EA;     } break; /* 5 SRE ZPG */
	case 0x48: { CYCLES(3); RD_DUM;            PHA;            } break; /* 3 PHA */
	case 0x49: { CYCLES(2); RD_IMM;            EOR;            } break; /* 2 EOR IMM */
	case 0x4a: { CYCLES(2); RD_DUM;    RD_ACC; LSR; WB_ACC;    } break; /* 2 LSR A */
	case 0x4b: { CYCLES(2); RD_IMM;            ASR; WB_ACC;    } break; /* 2 ASR IMM */
	case 0x4c: { CYCLES(3); EA_ABS;            JMP;            } break; /* 3 JMP ABS */
	case 0x4d: { CYCLES(4); RD_ABS;            EOR;            } break; /* 4 EOR ABS */
	case 0x4e: { CYCLES(6); RD_ABS;    RD_EA;  LSR; WB_EA;     } break; /* 6 LSR ABS */
	case 0x4f: { CYCLES(6); RD_ABS;    RD_EA;  SRE; WB_EA;     } break; /* 6 SRE ABS */
	case 0x50: { CYCLES(2);                    BVC;            } break; /* 2-4 BVC REL */
	case 0x51: { CYCLES(5); RD_IDY_P;          EOR;            } break; /* 5 EOR IDY page penalty */
	case 0x52: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
	case 0x53: { CYCLES(7); RD_IDY_NP; RD_EA;  SRE; WB_EA;     } break; /* 7 SRE IDY */
	case 0x54: { CYCLES(4); RD_ZPX;            NOP;            } break; /* 4 NOP ZPX */
	case 0x55: { CYCLES(4); RD_ZPX;            EOR;            } break; /* 4 EOR ZPX */
	case 0x56: { CYCLES(6); RD_ZPX;    RD_EA;  LSR; WB_EA;     } break; /* 6 LSR ZPX */
	case 0x57: { CYCLES(6); RD_ZPX;    RD_EA;  SRE; WB_EA;     } break; /* 6 SRE ZPX */
	case 0x58: { CYCLES(2); RD_DUM;            CLI;            } break; /* 2 CLI */
	case 0x59: { CYCLES(4); RD_ABY_P;          EOR;            } break; /* 4 EOR ABY page penalty */
	case 0x5a: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
	case 0x5b: { CYCLES(7); RD_ABY_NP; RD_EA;  SRE; WB_EA;     } break; /* 7 SRE ABY */
	case 0x5c: { CYCLES(4); RD_ABX_P;          NOP;            } break; /* 4 NOP ABX page penalty */
	case 0x5d: { CYCLES(4); RD_ABX_P;          EOR;            } break; /* 4 EOR ABX page penalty */
	case 0x5e: { CYCLES(7); RD_ABX_NP; RD_EA;  LSR; WB_EA;     } break; /* 7 LSR ABX */
	case 0x5f: { CYCLES(7); RD_ABX_NP; RD_EA;  SRE; WB_EA;     } break; /* 7 SRE ABX */
	case 0x60: { CYCLES(6);                    RTS;            } break; /* 6 RTS */
	case 0x61: { CYCLES(6); RD_IDX;            ADC;            } break; /* 6 ADC IDX */
	case 0x62: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
	case 0x63: { CYCLES(7); RD_IDX;    RD_EA;  RRA; WB_EA;     } break; /* 7 RRA IDX */
	case 0x64: { CYCLES(3); RD_ZPG;            NOP;            } break; /* 3 NOP ZPG */
	case 0x65: { CYCLES(3); RD_ZPG;            ADC;            } break; /* 3 ADC ZPG */
	case 0x66: { CYCLES(5); RD_ZPG;    RD_EA;  ROR; WB_EA;     } break; /* 5 ROR ZPG */
	case 0x67: { CYCLES(5); RD_ZPG;    RD_EA;  RRA; WB_EA;     } break; /* 5 RRA ZPG */
	case 0x68: { CYCLES(4); RD_DUM;            PLA;            } break; /* 4 PLA */
	case 0x69: { CYCLES(2); RD_IMM;            ADC;            } break; /* 2 ADC IMM */
	case 0x6a: { CYCLES(2); RD_DUM;    RD_ACC; ROR; WB_ACC;    } break; /* 2 ROR A */
	case 0x6b: { CYCLES(2); RD_IMM;            ARR; WB_ACC;    } break; /* 2 ARR IMM */
	case 0x6c: { CYCLES(5); EA_IND;            JMP;            } break; /* 5 JMP IND */
	case 0x6d: { CYCLES(4); RD_ABS;            ADC;            } break; /* 4 ADC ABS */
	case 0x6e: { CYCLES(6); RD_ABS;    RD_EA;  ROR; WB_EA;     } break; /* 6 ROR ABS */
	case 0x6f: { CYCLES(6); RD_ABS;    RD_EA;  RRA; WB_EA;     } break; /* 6 RRA ABS */
	case 0x70: { CYCLES(2);                    BVS;            } break; /* 2-4 BVS REL */
	case 0x71: { CYCLES(5); RD_IDY_P;          ADC;            } break; /* 5 ADC IDY page penalty */
	case 0x72: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
	case 0x73: { CYCLES(7); RD_IDY_NP; RD_EA;  RRA; WB_EA;     } break; /* 7 RRA IDY */
	case 0x74: { CYCLES(4); RD_ZPX;            NOP;            } break; /* 4 NOP ZPX */
	case 0x75: { CYCLES(4); RD_ZPX;            ADC;            } break; /* 4 ADC ZPX */
	case 0x76: { CYCLES(6); RD_ZPX;    RD_EA;  ROR; WB_EA;     } break; /* 6 ROR ZPX */
	case 0x77: { CYCLES(6); RD_ZPX;    RD_EA;  RRA; WB_EA;     } break; /* 6 RRA ZPX */
	case 0x78: { CYCLES(2); RD_DUM;            SEI;            } break; /* 2 SEI */
	case 0x79: { CYCLES(4); RD_ABY_P;          ADC;            } break; /* 4 ADC ABY page penalty */
	case 0x7a: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
	case 0x7b: { CYCLES(7); RD_ABY_NP; RD_EA;  RRA; WB_EA;     } break; /* 7 RRA ABY */
	case 0x7c: { CYCLES(4); RD_ABX_P;          NOP;            } break; /* 4 NOP ABX page penalty */
	case 0x7d: { CYCLES(4); RD_ABX_P;          ADC;            } break; /* 4 ADC ABX page penalty */
	case 0x7e: { CYCLES(7); RD_ABX_NP; RD_EA;  ROR; WB_EA;     } break; /* 7 ROR ABX */
	case 0x7f: { CYCLES(7); RD_ABX_NP; RD_EA;  RRA; WB_EA;     } break; /* 7 RRA ABX */
	case 0x80: { CYCLES(2); RD_IMM;            NOP;            } break; /* 2 NOP IMM */
	case 0x81: { CYCLES(6);                    STA; WR_IDX;    } break; /* 6 STA IDX */
	case 0x82: { CYCLES(2); RD_IMM;            NOP;            } break; /* 2 NOP IMM */
	case 0x83: { CYCLES(6);                    SAX; WR_IDX;    } break; /* 6 SAX IDX */
	case 0x84: { CYCLES(3);                    STY; WR_ZPG;    } break; /* 3 STY ZPG */
	case 0x85: { CYCLES(3);                    STA; WR_ZPG;    } break; /* 3 STA ZPG */
	case 0x86: { CYCLES(3);                    STX; WR_ZPG;    } break; /* 3 STX ZPG */
	case 0x87: { CYCLES(3);                    SAX; WR_ZPG;    } break; /* 3 SAX ZPG */
	case 0x88: { CYCLES(2); RD_DUM;            DEY;            } break; /* 2 DEY */
	case 0x89: { CYCLES(2); RD_IMM;            NOP;            } break; /* 2 NOP IMM */
	case 0x8a: { CYCLES(2); RD_DUM;            TXA;            } break; /* 2 TXA */
	case 0x8b: { CYCLES(2); RD_IMM;            AXA;            } break; /* 2 AXA IMM */
	case 0x8c: { CYCLES(4);                    STY; WR_ABS;    } break; /* 4 STY ABS */
	case 0x8d: { CYCLES(4);                    STA; WR_ABS;    } break; /* 4 STA ABS */
	case 0x8e: { CYCLES(4);                    STX; WR_ABS;    } break; /* 4 STX ABS */
	case 0x8f: { CYCLES(4);                    SAX; WR_ABS;    } break; /* 4 SAX ABS */
	case 0x90: { CYCLES(2);                    BCC;            } break; /* 2-4 BCC REL */
	case 0x91: { CYCLES(6);                    STA; WR_IDY_NP; } break; /* 6 STA IDY */
	case 0x92: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
	case 0x93: { CYCLES(5); EA_IDY_NP;         SAH; WB_EA;     } break; /* 5 SAH IDY */
	case 0x94: { CYCLES(4);                    STY; WR_ZPX;    } break; /* 4 STY ZPX */
	case 0x95: { CYCLES(4);                    STA; WR_ZPX;    } break; /* 4 STA ZPX */
	case 0x96: { CYCLES(4);                    STX; WR_ZPY;    } break; /* 4 STX ZPY */
	case 0x97: { CYCLES(4);                    SAX; WR_ZPY;    } break; /* 4 SAX ZPY */
	case 0x98: { CYCLES(2); RD_DUM;            TYA;            } break; /* 2 TYA */
	case 0x99: { CYCLES(5);                    STA; WR_ABY_NP; } break; /* 5 STA ABY */
	case 0x9a: { CYCLES(2); RD_DUM;            TXS;            } break; /* 2 TXS */
	case 0x9b: { CYCLES(5); EA_ABY_NP;         SSH; WB_EA;     } break; /* 5 SSH ABY */
	case 0x9c: { CYCLES(5); EA_ABX_NP;         SYH; WB_EA;     } break; /* 5 SYH ABX */
	case 0x9d: { CYCLES(5);                    STA; WR_ABX_NP; } break; /* 5 STA ABX */
	case 0x9e: { CYCLES(5); EA_ABY_NP;         SXH; WB_EA;     } break; /* 5 SXH ABY */
	case 0x9f: { CYCLES(5); EA_ABY_NP;         SAH; WB_EA;     } break; /* 5 SAH ABY */
	case 0xa0: { CYCLES(2); RD_IMM;            LDY;            } break; /* 2 LDY IMM */
	case 0xa1: { CYCLES(6); RD_IDX;            LDA;            } break; /* 6 LDA IDX */
	case 0xa2: { CYCLES(2); RD_IMM;            LDX;            } break; /* 2 LDX IMM */
	case 0xa3: { CYCLES(6); RD_IDX;            LAX;            } break; /* 6 LAX IDX */
	case 0xa4: { CYCLES(3); RD_ZPG;            LDY;            } break; /* 3 LDY ZPG */
	case 0xa5: { CYCLES(3); RD_ZPG;            LDA;            } break; /* 3 LDA ZPG */
	case 0xa6: { CYCLES(3); RD_ZPG;            LDX;            } break; /* 3 LDX ZPG */
	case 0xa7: { CYCLES(3); RD_ZPG;            LAX;            } break; /* 3 LAX ZPG */
	case 0xa8: { CYCLES(2); RD_DUM;            TAY;            } break; /* 2 TAY */
	case 0xa9: { CYCLES(2); RD_IMM;            LDA;            } break; /* 2 LDA IMM */
	case 0xaa: { CYCLES(2); RD_DUM;            TAX;            } break; /* 2 TAX */
	case 0xab: { CYCLES(2); RD_IMM;            OAL;            } break; /* 2 OAL IMM */
	case 0xac: { CYCLES(4); RD_ABS;            LDY;            } break; /* 4 LDY ABS */
	case 0xad: { CYCLES(4); RD_ABS;            LDA;            } break; /* 4 LDA ABS */
	case 0xae: { CYCLES(4); RD_ABS;            LDX;            } break; /* 4 LDX ABS */
	case 0xaf: { CYCLES(4); RD_ABS;            LAX;            } break; /* 4 LAX ABS */
	case 0xb0: { CYCLES(2);                    BCS;            } break; /* 2-4 BCS REL */
	case 0xb1: { CYCLES(5); RD_IDY_P;          LDA;            } break; /* 5 LDA IDY page penalty */
	case 0xb2: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
	case 0xb3: { CYCLES(5); RD_IDY_P;          LAX;            } break; /* 5 LAX IDY page penalty */
	case 0xb4: { CYCLES(4); RD_ZPX;            LDY;            } break; /* 4 LDY ZPX */
	case 0xb5: { CYCLES(4); RD_ZPX;            LDA;            } break; /* 4 LDA ZPX */
	case 0xb6: { CYCLES(4); RD_ZPY;            LDX;            } break; /* 4 LDX ZPY */
	case 0xb7: { CYCLES(4); RD_ZPY;            LAX;            } break; /* 4 LAX ZPY */
	case 0xb8: { CYCLES(2); RD_DUM;            CLV;            } break; /* 2 CLV */
	case 0xb9: { CYCLES(4); RD_ABY_P;          LDA;            } break; /* 4 LDA ABY page penalty */
	case 0xba: { CYCLES(2); RD_DUM;            TSX;            } break; /* 2 TSX */
	case 0xbb: { CYCLES(4); RD_ABY_P;          AST;            } break; /* 4 AST ABY page penalty */
	case 0xbc: { CYCLES(4); RD_ABX_P;          LDY;            } break; /* 4 LDY ABX page penalty */
	case 0xbd: { CYCLES(4); RD_ABX_P;          LDA;            } break; /* 4 LDA ABX page penalty */
	case 0xbe: { CYCLES(4); RD_ABY_P;          LDX;            } break; /* 4 LDX ABY page penalty */
	case 0xbf: { CYCLES(4); RD_ABY_P;          LAX;            } break; /* 4 LAX ABY page penalty */
	case 0xc0: { CYCLES(2); RD_IMM;            CPY;            } break; /* 2 CPY IMM */
	case 0xc1: { CYCLES(6); RD_IDX;            CMP;            } break; /* 6 CMP IDX */
	case 0xc2: { CYCLES(2); RD_IMM;            NOP;            } break; /* 2 NOP IMM */
	case 0xc3: { CYCLES(7); RD_IDX;    RD_EA;  DCP; WB_EA;     } break; /* 7 DCP IDX */
	case 0xc4: { CYCLES(3); RD_ZPG;            CPY;            } break; /* 3 CPY ZPG */
	case 0xc5: { CYCLES(3); RD_ZPG;            CMP;            } break; /* 3 CMP ZPG */
	case 0xc6: { CYCLES(5); RD_ZPG;    RD_EA;  DEC; WB_EA;     } break; /* 5 DEC ZPG */
	case 0xc7: { CYCLES(5); RD_ZPG;    RD_EA;  DCP; WB_EA;     } break; /* 5 DCP ZPG */
	case 0xc8: { CYCLES(2); RD_DUM;            INY;            } break; /* 2 INY */
	case 0xc9: { CYCLES(2); RD_IMM;            CMP;            } break; /* 2 CMP IMM */
	case 0xca: { CYCLES(2); RD_DUM;            DEX;            } break; /* 2 DEX */
	case 0xcb: { CYCLES(2); RD_IMM;            ASX;            } break; /* 2 ASX IMM */
	case 0xcc: { CYCLES(4); RD_ABS;            CPY;            } break; /* 4 CPY ABS */
	case 0xcd: { CYCLES(4); RD_ABS;            CMP;            } break; /* 4 CMP ABS */
	case 0xce: { CYCLES(6); RD_ABS;    RD_EA;  DEC; WB_EA;     } break; /* 6 DEC ABS */
	case 0xcf: { CYCLES(6); RD_ABS;    RD_EA;  DCP; WB_EA;     } break; /* 6 DCP ABS */
	case 0xd0: { CYCLES(2);                    BNE;            } break; /* 2-4 BNE REL */
	case 0xd1: { CYCLES(5); RD_IDY_P;          CMP;            } break; /* 5 CMP IDY page penalty */
	case 0xd2: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
	case 0xd3: { CYCLES(7); RD_IDY_NP; RD_EA;  DCP; WB_EA;     } break; /* 7 DCP IDY */
	case 0xd4: { CYCLES(4); RD_ZPX;            NOP;            } break; /* 4 NOP ZPX */
	case 0xd5: { CYCLES(4); RD_ZPX;            CMP;            } break; /* 4 CMP ZPX */
	case 0xd6: { CYCLES(6); RD_ZPX;    RD_EA;  DEC; WB_EA;     } break; /* 6 DEC ZPX */
	case 0xd7: { CYCLES(6); RD_ZPX;    RD_EA;  DCP; WB_EA;     } break; /* 6 DCP ZPX */
	case 0xd8: { CYCLES(2); RD_DUM;            CLD;            } break; /* 2 CLD */
	case 0xd9: { CYCLES(4); RD_ABY_P;          CMP;            } break; /* 4 CMP ABY page penalty */
	case 0xda: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
	case 0xdb: { CYCLES(7); RD_ABY_NP; RD_EA;  DCP; WB_EA;     } break; /* 7 DCP ABY */
	case 0xdc: { CYCLES(4); RD_ABX_P;          NOP;            } break; /* 4 NOP ABX page penalty */
	case 0xdd: { CYCLES(4); RD_ABX_P;          CMP;            } break; /* 4 CMP ABX page penalty */
	case 0xde: { CYCLES(7); RD_ABX_NP; RD_EA;  DEC; WB_EA;     } break; /* 7 DEC ABX */
	case 0xdf: { CYCLES(7); RD_ABX_NP; RD_EA;  DCP; WB_EA;     } break; /* 7 DCP ABX */
	case 0xe0: { CYCLES(2); RD_IMM;            CPX;            } break; /* 2 CPX IMM */
	case 0xe1: { CYCLES(6); RD_IDX;            SBC;            } break; /* 6 SBC IDX */
	case 0xe2: { CYCLES(2); RD_IMM;            NOP;            } break; /* 2 NOP IMM */
	case 0xe3: { CYCLES(7); RD_IDX;    RD_EA;  ISB; WB_EA;     } break; /* 7 ISB IDX */
	case 0xe4: { CYCLES(3); RD_ZPG;            CPX;            } break; /* 3 CPX ZPG */
	case 0xe5: { CYCLES(3); RD_ZPG;            SBC;            } break; /* 3 SBC ZPG */
	case 0xe6: { CYCLES(5); RD_ZPG;    RD_EA;  INC; WB_EA;     } break; /* 5 INC ZPG */
	case 0xe7: { CYCLES(5); RD_ZPG;    RD_EA;  ISB; WB_EA;     } break; /* 5 ISB ZPG */
	case 0xe8: { CYCLES(2); RD_DUM;            INX;            } break; /* 2 INX */
	case 0xe9: { CYCLES(2); RD_IMM;            SBC;            } break; /* 2 SBC IMM */
	case 0xea: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
	case 0xeb: { CYCLES(2); RD_IMM;            SBC;            } break; /* 2 SBC IMM */
	case 0xec: { CYCLES(4); RD_ABS;            CPX;            } break; /* 4 CPX ABS */
	case 0xed: { CYCLES(4); RD_ABS;            SBC;            } break; /* 4 SBC ABS */
	case 0xee: { CYCLES(6); RD_ABS;    RD_EA;  INC; WB_EA;     } break; /* 6 INC ABS */
	case 0xef: { CYCLES(6); RD_ABS;    RD_EA;  ISB; WB_EA;     } break; /* 6 ISB ABS */
	case 0xf0: { CYCLES(2);                    BEQ;            } break; /* 2-4 BEQ REL */
	case 0xf1: { CYCLES(5); RD_IDY_P;          SBC;            } break; /* 5 SBC IDY page penalty */
	case 0xf2: { CYCLES(1);                    KIL;            } break; /* 1 KIL */
	case 0xf3: { CYCLES(7); RD_IDY_NP; RD_EA;  ISB; WB_EA;     } break; /* 7 ISB IDY */
	case 0xf4: { CYCLES(4); RD_ZPX;            NOP;            } break; /* 4 NOP ZPX */
	case 0xf5: { CYCLES(4); RD_ZPX;            SBC;            } break; /* 4 SBC ZPX */
	case 0xf6: { CYCLES(6); RD_ZPX;    RD_EA;  INC; WB_EA;     } break; /* 6 INC ZPX */
	case 0xf7: { CYCLES(6); RD_ZPX;    RD_EA;  ISB; WB_EA;     } break; /* 6 ISB ZPX */
	case 0xf8: { CYCLES(2); RD_DUM;            SED;            } break; /* 2 SED */
	case 0xf9: { CYCLES(4); RD_ABY_P;          SBC;            } break; /* 4 SBC ABY page penalty */
	case 0xfa: { CYCLES(2); RD_DUM;            NOP;            } break; /* 2 NOP */
	case 0xfb: { CYCLES(7); RD_ABY_NP; RD_EA;  ISB; WB_EA;     } break; /* 7 ISB ABY */
	case 0xfc: { CYCLES(4); RD_ABX_P;          NOP;            } break; /* 4 NOP ABX page penalty */
	case 0xfd: { CYCLES(4); RD_ABX_P;          SBC;            } break; /* 4 SBC ABX page penalty */
	case 0xfe: { CYCLES(7); RD_ABX_NP; RD_EA;  INC; WB_EA;     } break; /* 7 INC ABX */
	case 0xff: { CYCLES(7); RD_ABX_NP; RD_EA;  ISB; WB_EA;     } break; /* 7 ISB ABX */
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

inline void M6502::update_irq()
{
	if(!(P & F_I)) {
		EAD = IRQ_VEC;
		CYCLES(2);
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P |= F_I;
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD + 1);
		// call back the cpuintrf to let it clear the line
		d_pic->intr_reti();
		irq_state = false;
	}
	pending_irq = false;
}

// main

void M6502::initialize()
{
	A = X = Y = P = 0;
	SPD = EAD = ZPD = PCD = 0;
}

void M6502::reset()
{
	PCL = RDMEM(RST_VEC);
	PCH = RDMEM(RST_VEC + 1);
	SPD = 0x01ff;
	P = F_T | F_I | F_Z | F_B | (P & F_D);
	
	icount = 0;
	pending_irq = after_cli = false;
	irq_state = nmi_state = so_state = false;
}

int M6502::run(int clock)
{
	// return now if BUSREQ
	if(busreq) {
		icount = 0;
		return 1;
	}
	
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
		
		while(icount > 0 && !busreq) {
			run_one_opecode();
		}
		int passed_icount = first_icount - icount;
		if(busreq && icount > 0) {
			icount = 0;
		}
		return passed_icount;
	}
}

void M6502::run_one_opecode()
{
	// if an irq is pending, take it now
	if(nmi_state) {
		EAD = NMI_VEC;
		CYCLES(2);
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P |= F_I;	// set I flag
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD + 1);
		nmi_state = false;
	} else if(pending_irq) {
		update_irq();
	}
	prev_pc = pc.w.l;
	uint8 code = RDOP();
	OP(code);
	
	// check if the I flag was just reset (interrupts enabled)
	if(after_cli) {
		after_cli = false;
		if(irq_state) {
			pending_irq = true;
		}
	} else if(pending_irq) {
		update_irq();
	}
}

void M6502::write_signal(int id, uint32 data, uint32 mask)
{
	bool state = ((data & mask) != 0);
	
	if(id == SIG_CPU_NMI) {
		nmi_state = state;
	} else if(id == SIG_CPU_IRQ) {
		irq_state = state;
		if(state) {
			pending_irq = true;
		}
	} else if(id == SIG_M6502_OVERFLOW) {
		if(so_state && !state) {
			P |= F_V;
		}
		so_state = state;
	} else if(id == SIG_CPU_BUSREQ) {
		busreq = ((data & mask) != 0);
	}
}

#define STATE_VERSION	1

void M6502::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint32(pc.d);
	state_fio->FputUint32(sp.d);
	state_fio->FputUint32(zp.d);
	state_fio->FputUint32(ea.d);
	state_fio->FputUint16(prev_pc);
	state_fio->FputUint8(a);
	state_fio->FputUint8(x);
	state_fio->FputUint8(y);
	state_fio->FputUint8(p);
	state_fio->FputBool(pending_irq);
	state_fio->FputBool(after_cli);
	state_fio->FputBool(nmi_state);
	state_fio->FputBool(irq_state);
	state_fio->FputBool(so_state);
	state_fio->FputInt32(icount);
	state_fio->FputBool(busreq);
}

bool M6502::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	pc.d = state_fio->FgetUint32();
	sp.d = state_fio->FgetUint32();
	zp.d = state_fio->FgetUint32();
	ea.d = state_fio->FgetUint32();
	prev_pc = state_fio->FgetUint16();
	a = state_fio->FgetUint8();
	x = state_fio->FgetUint8();
	y = state_fio->FgetUint8();
	p = state_fio->FgetUint8();
	pending_irq = state_fio->FgetBool();
	after_cli = state_fio->FgetBool();
	nmi_state = state_fio->FgetBool();
	irq_state = state_fio->FgetBool();
	so_state = state_fio->FgetBool();
	icount = state_fio->FgetInt32();
	busreq = state_fio->FgetBool();
	return true;
}

