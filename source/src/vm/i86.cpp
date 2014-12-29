/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date  : 2011.04.23-

	[ i86/v30 ]
*/

// disable warnings C4146 for microsoft visual c++ 2005
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4146 )
#endif

#include "i86.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif
#include "../fileio.h"

#define DIVIDE_FAULT			0
#define NMI_INT_VECTOR			2
#define OVERFLOW_TRAP			4
#define BOUNDS_CHECK_FAULT		5
#define ILLEGAL_INSTRUCTION		6
#define GENERAL_PROTECTION_FAULT	13

#define INT_REQ_BIT			1
#define NMI_REQ_BIT			2

typedef enum { ES, CS, SS, DS } SREGS;
typedef enum { AX, CX, DX, BX, SP, BP, SI, DI } WREGS;

typedef enum {
#ifdef _BIG_ENDIAN
	 AH,  AL,  CH,  CL,  DH,  DL,  BH,  BL,
	SPH, SPL, BPH, BPL, SIH, SIL, DIH, DIL,
#else
	 AL,  AH,  CL,  CH,  DL,  DH,  BL,  BH,
	SPL, SPH, BPL, BPH, SIL, SIH, DIL, DIH,
#endif
} BREGS;

static struct {
	struct {
		WREGS w[256];
		BREGS b[256];
	} reg;
	struct {
		WREGS w[256];
		BREGS b[256];
	} RM;
} Mod_RM;

static const uint8 parity_table[256] = {
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

/************************************************************************/

struct i80x86_timing {
	uint8	exception, iret;				/* exception, IRET */
	uint8	int3, int_imm, into_nt, into_t;			/* INTs */
	uint8	override;					/* segment overrides */
	uint8	flag_ops, lahf, sahf;				/* flag operations */
	uint8	aaa, aas, aam, aad;				/* arithmetic adjusts */
	uint8	daa, das;					/* decimal adjusts */
	uint8	cbw, cwd;					/* sign extension */
	uint8	hlt, load_ptr, lea, nop, wait, xlat;		/* misc */

	uint8	jmp_short, jmp_near, jmp_far;			/* direct JMPs */
	uint8	jmp_r16, jmp_m16, jmp_m32;			/* indirect JMPs */
	uint8	call_near, call_far;				/* direct CALLs */
	uint8	call_r16, call_m16, call_m32;			/* indirect CALLs */
	uint8	ret_near, ret_far, ret_near_imm, ret_far_imm;	/* returns */
	uint8	jcc_nt, jcc_t, jcxz_nt, jcxz_t;			/* conditional JMPs */
	uint8	loop_nt, loop_t, loope_nt, loope_t;		/* loops */

	uint8	in_imm8, in_imm16, in_dx8, in_dx16;		/* port reads */
	uint8	out_imm8, out_imm16, out_dx8, out_dx16;		/* port writes */

	uint8	mov_rr8, mov_rm8, mov_mr8;			/* move, 8-bit */
	uint8	mov_ri8, mov_mi8;				/* move, 8-bit immediate */
	uint8	mov_rr16, mov_rm16, mov_mr16;			/* move, 16-bit */
	uint8	mov_ri16, mov_mi16;				/* move, 16-bit immediate */
	uint8	mov_am8, mov_am16, mov_ma8, mov_ma16;		/* move, AL/AX memory */
	uint8	mov_sr, mov_sm, mov_rs, mov_ms;			/* move, segment registers */
	uint8	xchg_rr8, xchg_rm8;				/* exchange, 8-bit */
	uint8	xchg_rr16, xchg_rm16, xchg_ar16;		/* exchange, 16-bit */

	uint8	push_r16, push_m16, push_seg, pushf;		/* pushes */
	uint8	pop_r16, pop_m16, pop_seg, popf;		/* pops */

	uint8	alu_rr8, alu_rm8, alu_mr8;			/* ALU ops, 8-bit */
	uint8	alu_ri8, alu_mi8, alu_mi8_ro;			/* ALU ops, 8-bit immediate */
	uint8	alu_rr16, alu_rm16, alu_mr16;			/* ALU ops, 16-bit */
	uint8	alu_ri16, alu_mi16, alu_mi16_ro;		/* ALU ops, 16-bit immediate */
	uint8	alu_r16i8, alu_m16i8, alu_m16i8_ro;		/* ALU ops, 16-bit w/8-bit immediate */
	uint8	mul_r8, mul_r16, mul_m8, mul_m16;		/* MUL */
	uint8	imul_r8, imul_r16, imul_m8, imul_m16;		/* IMUL */
	uint8	div_r8, div_r16, div_m8, div_m16;		/* DIV */
	uint8	idiv_r8, idiv_r16, idiv_m8, idiv_m16;		/* IDIV */
	uint8	incdec_r8, incdec_r16, incdec_m8, incdec_m16;	/* INC/DEC */
	uint8	negnot_r8, negnot_r16, negnot_m8, negnot_m16;	/* NEG/NOT */

	uint8	rot_reg_1, rot_reg_base, rot_reg_bit;		/* reg shift/rotate */
	uint8	rot_m8_1, rot_m8_base, rot_m8_bit;		/* m8 shift/rotate */
	uint8	rot_m16_1, rot_m16_base, rot_m16_bit;		/* m16 shift/rotate */

	uint8	cmps8, rep_cmps8_base, rep_cmps8_count;		/* CMPS 8-bit */
	uint8	cmps16, rep_cmps16_base, rep_cmps16_count;	/* CMPS 16-bit */
	uint8	scas8, rep_scas8_base, rep_scas8_count;		/* SCAS 8-bit */
	uint8	scas16, rep_scas16_base, rep_scas16_count;	/* SCAS 16-bit */
	uint8	lods8, rep_lods8_base, rep_lods8_count;		/* LODS 8-bit */
	uint8	lods16, rep_lods16_base, rep_lods16_count;	/* LODS 16-bit */
	uint8	stos8, rep_stos8_base, rep_stos8_count;		/* STOS 8-bit */
	uint8	stos16, rep_stos16_base, rep_stos16_count;	/* STOS 16-bit */
	uint8	movs8, rep_movs8_base, rep_movs8_count;		/* MOVS 8-bit */
	uint8	movs16, rep_movs16_base, rep_movs16_count;	/* MOVS 16-bit */

	uint8	ins8, rep_ins8_base, rep_ins8_count;		/* (80186) INS 8-bit */
	uint8	ins16, rep_ins16_base, rep_ins16_count;		/* (80186) INS 16-bit */
	uint8	outs8, rep_outs8_base, rep_outs8_count;		/* (80186) OUTS 8-bit */
	uint8	outs16, rep_outs16_base, rep_outs16_count;	/* (80186) OUTS 16-bit */
	uint8	push_imm, pusha, popa;				/* (80186) PUSH immediate, PUSHA/POPA */
	uint8	imul_rri8, imul_rmi8;				/* (80186) IMUL immediate 8-bit */
	uint8	imul_rri16, imul_rmi16;				/* (80186) IMUL immediate 16-bit */
	uint8	enter0, enter1, enter_base, enter_count, leave;	/* (80186) ENTER/LEAVE */
	uint8	bound;						/* (80186) BOUND */
};

#if defined(HAS_I86)
/* these come from the 8088 timings in OPCODE.LST, but with the
   penalty for 16-bit memory accesses removed wherever possible */
static const struct i80x86_timing timing = {
	51, 32,			/* exception, IRET */
	2, 0, 4, 2,		/* INTs */
	2,			/* segment overrides */
	2, 4, 4,		/* flag operations */
	4, 4, 83, 60,		/* arithmetic adjusts */
	4, 4,			/* decimal adjusts */
	2, 5,			/* sign extension */
	2, 24, 2, 2, 3, 11,	/* misc */

	15, 15, 15,		/* direct JMPs */
	11, 18, 24,		/* indirect JMPs */
	19, 28,			/* direct CALLs */
	16, 21, 37,		/* indirect CALLs */
	20, 32, 24, 31,		/* returns */
	4, 16, 6, 18,		/* conditional JMPs */
	5, 17, 6, 18,		/* loops */

	10, 14, 8, 12,		/* port reads */
	10, 14, 8, 12,		/* port writes */

	2, 8, 9,		/* move, 8-bit */
	4, 10,			/* move, 8-bit immediate */
	2, 8, 9,		/* move, 16-bit */
	4, 10,			/* move, 16-bit immediate */
	10, 10, 10, 10,		/* move, AL/AX memory */
	2, 8, 2, 9,		/* move, segment registers */
	4, 17,			/* exchange, 8-bit */
	4, 17, 3,		/* exchange, 16-bit */

	15, 24, 14, 14,		/* pushes */
	12, 25, 12, 12,		/* pops */

	3, 9, 16,		/* ALU ops, 8-bit */
	4, 17, 10,		/* ALU ops, 8-bit immediate */
	3, 9, 16,		/* ALU ops, 16-bit */
	4, 17, 10,		/* ALU ops, 16-bit immediate */
	4, 17, 10,		/* ALU ops, 16-bit w/8-bit immediate */
	70, 118, 76, 128,	/* MUL */
	80, 128, 86, 138,	/* IMUL */
	80, 144, 86, 154,	/* DIV */
	101, 165, 107, 175,	/* IDIV */
	3, 2, 15, 15,		/* INC/DEC */
	3, 3, 16, 16,		/* NEG/NOT */

	2, 8, 4,		/* reg shift/rotate */
	15, 20, 4,		/* m8 shift/rotate */
	15, 20, 4,		/* m16 shift/rotate */

	22, 9, 21,		/* CMPS 8-bit */
	22, 9, 21,		/* CMPS 16-bit */
	15, 9, 14,		/* SCAS 8-bit */
	15, 9, 14,		/* SCAS 16-bit */
	12, 9, 11,		/* LODS 8-bit */
	12, 9, 11,		/* LODS 16-bit */
	11, 9, 10,		/* STOS 8-bit */
	11, 9, 10,		/* STOS 16-bit */
	18, 9, 17,		/* MOVS 8-bit */
	18, 9, 17,		/* MOVS 16-bit */
};
#else
/* these come from the Intel 80186 datasheet */
static const struct i80x86_timing timing = {
	45, 28,			/* exception, IRET */
	0, 2, 4, 3,		/* INTs */
	2,			/* segment overrides */
	2, 2, 3,		/* flag operations */
	8, 7, 19, 15,		/* arithmetic adjusts */
	4, 4,			/* decimal adjusts */
	2, 4,			/* sign extension */
	2, 18, 6, 2, 6, 11,	/* misc */

	14, 14, 14,		/* direct JMPs */
	11, 17, 26,		/* indirect JMPs */
	15, 23,			/* direct CALLs */
	13, 19, 38,		/* indirect CALLs */
	16, 22, 18, 25,		/* returns */
	4, 13, 5, 15,		/* conditional JMPs */
	6, 16, 6, 16,		/* loops */

	10, 10, 8, 8,		/* port reads */
	9, 9, 7, 7,		/* port writes */

	2, 9, 12,		/* move, 8-bit */
	3, 12,			/* move, 8-bit immediate */
	2, 9, 12,		/* move, 16-bit */
	4, 13,			/* move, 16-bit immediate */
	8, 8, 9, 9,		/* move, AL/AX memory */
	2, 11, 2, 11,		/* move, segment registers */
	4, 17,			/* exchange, 8-bit */
	4, 17, 3,		/* exchange, 16-bit */

	10, 16, 9, 9,		/* pushes */
	10, 20, 8, 8,		/* pops */

	3, 10, 10,		/* ALU ops, 8-bit */
	4, 16, 10,		/* ALU ops, 8-bit immediate */
	3, 10, 10,		/* ALU ops, 16-bit */
	4, 16, 10,		/* ALU ops, 16-bit immediate */
	4, 16, 10,		/* ALU ops, 16-bit w/8-bit immediate */
	26, 35, 32, 41,		/* MUL */
	25, 34, 31, 40,		/* IMUL */
	29, 38, 35, 44,		/* DIV */
	44, 53, 50, 59,		/* IDIV */
	3, 3, 15, 15,		/* INC/DEC */
	3, 3, 10, 10,		/* NEG/NOT */

	2, 5, 1,		/* reg shift/rotate */
	15, 17, 1,		/* m8 shift/rotate */
	15, 17, 1,		/* m16 shift/rotate */

	22, 5, 22,		/* CMPS 8-bit */
	22, 5, 22,		/* CMPS 16-bit */
	15, 5, 15,		/* SCAS 8-bit */
	15, 5, 15,		/* SCAS 16-bit */
	12, 6, 11,		/* LODS 8-bit */
	12, 6, 11,		/* LODS 16-bit */
	10, 6, 9,		/* STOS 8-bit */
	10, 6, 9,		/* STOS 16-bit */
	14, 8, 8,		/* MOVS 8-bit */
	14, 8, 8,		/* MOVS 16-bit */

	14, 8, 8,		/* (80186) INS 8-bit */
	14, 8, 8,		/* (80186) INS 16-bit */
	14, 8, 8,		/* (80186) OUTS 8-bit */
	14, 8, 8,		/* (80186) OUTS 16-bit */
	14, 68, 83,		/* (80186) PUSH immediate, PUSHA/POPA */
	22, 29,			/* (80186) IMUL immediate 8-bit */
	25, 32,			/* (80186) IMUL immediate 16-bit */
	15, 25, 4, 16, 8,	/* (80186) ENTER/LEAVE */
	33,			/* (80186) BOUND */
};
#endif

/************************************************************************/

#define SetTF(x)		(TF = (x))
#define SetIF(x)		(IF = (x))
#define SetDF(x)		(DirVal = (x) ? -1 : 1)
#define SetMD(x)		(MF = (x))

#define SetOFW_Add(x, y, z)	(OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x8000)
#define SetOFB_Add(x, y, z)	(OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x80)
#define SetOFW_Sub(x, y, z)	(OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x8000)
#define SetOFB_Sub(x, y, z)	(OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x80)

#define SetCFB(x)		(CarryVal = (x) & 0x100)
#define SetCFW(x)		(CarryVal = (x) & 0x10000)
#define SetAF(x, y, z)		(AuxVal = ((x) ^ ((y) ^ (z))) & 0x10)
#define SetSF(x)		(SignVal = (x))
#define SetZF(x)		(ZeroVal = (x))
#define SetPF(x)		(ParityVal = (x))

#define SetSZPF_Byte(x)		(ParityVal = SignVal = ZeroVal = (int8)(x))
#define SetSZPF_Word(x)		(ParityVal = SignVal = ZeroVal = (int16)(x))

#define ADDB(dst, src)		{ unsigned res = (dst) + (src); SetCFB(res); SetOFB_Add(res, src, dst); SetAF(res, src, dst); SetSZPF_Byte(res); dst = (uint8)res; }
#define ADDW(dst, src)		{ unsigned res = (dst) + (src); SetCFW(res); SetOFW_Add(res, src, dst); SetAF(res, src, dst); SetSZPF_Word(res); dst = (uint16)res; }

#define SUBB(dst, src)		{ unsigned res = (dst) - (src); SetCFB(res); SetOFB_Sub(res, src, dst); SetAF(res, src, dst); SetSZPF_Byte(res); dst = (uint8)res; }
#define SUBW(dst, src)		{ unsigned res = (dst) - (src); SetCFW(res); SetOFW_Sub(res, src, dst); SetAF(res, src, dst); SetSZPF_Word(res); dst = (uint16)res; }

#define ORB(dst, src)		dst |= (src); CarryVal = OverVal = AuxVal = 0; SetSZPF_Byte(dst)
#define ORW(dst, src)		dst |= (src); CarryVal = OverVal = AuxVal = 0; SetSZPF_Word(dst)

#define ANDB(dst, src)		dst &= (src); CarryVal = OverVal = AuxVal = 0; SetSZPF_Byte(dst)
#define ANDW(dst, src)		dst &= (src); CarryVal = OverVal = AuxVal = 0; SetSZPF_Word(dst)

#define XORB(dst, src)		dst ^= (src); CarryVal = OverVal = AuxVal = 0; SetSZPF_Byte(dst)
#define XORW(dst, src)		dst ^= (src); CarryVal = OverVal = AuxVal = 0; SetSZPF_Word(dst)

#define CF			(CarryVal != 0)
#define SF			(SignVal < 0)
#define ZF			(ZeroVal == 0)
#define PF			parity_table[ParityVal]
#define AF			(AuxVal != 0)
#define OF			(OverVal != 0)
#define DF			(DirVal < 0)
#define MD			(MF != 0)

/************************************************************************/

#define AMASK	0xfffff

#define read_mem_byte(a)	d_mem->read_data8((a) & AMASK)
#define read_mem_word(a)	d_mem->read_data16((a) & AMASK)
#define write_mem_byte(a, d)	d_mem->write_data8((a) & AMASK, (d))
#define write_mem_word(a, d)	d_mem->write_data16((a) & AMASK, (d))

#define read_port_byte(a)	d_io->read_io8(a)
#define read_port_word(a)	d_io->read_io16(a)
#define write_port_byte(a, d)	d_io->write_io8((a), (d))
#define write_port_word(a, d)	d_io->write_io16((a), (d))

/************************************************************************/

#define SegBase(Seg)		(sregs[Seg] << 4)

#define DefaultSeg(Seg)		((seg_prefix && (Seg == DS || Seg == SS)) ? prefix_seg : Seg)
#define DefaultBase(Seg)	((seg_prefix && (Seg == DS || Seg == SS)) ? base[prefix_seg] : base[Seg])

#define GetMemB(Seg, Off)	(read_mem_byte((DefaultBase(Seg) + (Off)) & AMASK))
#define GetMemW(Seg, Off)	(read_mem_word((DefaultBase(Seg) + (Off)) & AMASK))
#define PutMemB(Seg, Off, x)	write_mem_byte((DefaultBase(Seg) + (Off)) & AMASK, (x))
#define PutMemW(Seg, Off, x)	write_mem_word((DefaultBase(Seg) + (Off)) & AMASK, (x))

#define ReadByte(ea)		(read_mem_byte((ea) & AMASK))
#define ReadWord(ea)		(read_mem_word((ea) & AMASK))
#define WriteByte(ea, val)	write_mem_byte((ea) & AMASK, val);
#define WriteWord(ea, val)	write_mem_word((ea) & AMASK, val);

#define FETCH			read_mem_byte(pc++)
#define FETCHOP			read_mem_byte(pc++)
#define FETCHWORD(var)		{ var = read_mem_word(pc); pc += 2; }
#define PUSH(val)		{ regs.w[SP] -= 2; WriteWord(((base[SS] + regs.w[SP]) & AMASK), val); }
#define POP(var)		{ regs.w[SP] += 2; var = ReadWord(((base[SS] + ((regs.w[SP]-2) & 0xffff)) & AMASK)); }

/************************************************************************/

#define CompressFlags() (uint16)(CF | (PF << 2) | (AF << 4) | (ZF << 6) | (SF << 7) | (TF << 8) | (IF << 9) | (DF << 10) | (OF << 11) | (MD << 15))

#define ExpandFlags(f) { \
	CarryVal = (f) & 1; \
	ParityVal = !((f) & 4); \
	AuxVal = (f) & 0x10; \
	ZeroVal = !((f) & 0x40); \
	SignVal = ((f) & 0x80) ? -1 : 0; \
	TF = ((f) & 0x100) >> 8; \
	IF = ((f) & 0x200) >> 9; \
	MF = ((f) & 0x8000) >> 15; \
	DirVal = ((f) & 0x400) ? -1 : 1; \
	OverVal = (f) & 0x800; \
}

/************************************************************************/

#define RegWord(ModRM) regs.w[Mod_RM.reg.w[ModRM]]
#define RegByte(ModRM) regs.b[Mod_RM.reg.b[ModRM]]

#define GetRMWord(ModRM) \
	((ModRM) >= 0xc0 ? regs.w[Mod_RM.RM.w[ModRM]] : (GetEA(ModRM), ReadWord(ea)))

#define PutbackRMWord(ModRM, val) { \
	if (ModRM >= 0xc0) { \
		regs.w[Mod_RM.RM.w[ModRM]] = val; \
	} else { \
		WriteWord(ea, val); \
	} \
}

#define GetNextRMWord ( \
	ReadWord(ea + 2) \
)

#define GetRMWordOffset(offs) ( \
	ReadWord(ea - eo + (uint16)(eo + offs)) \
)

#define GetRMByteOffset(offs) ( \
	ReadByte(ea - eo + (uint16)(eo + offs)) \
)

#define PutRMWord(ModRM, val) { \
	if (ModRM >= 0xc0) { \
		regs.w[Mod_RM.RM.w[ModRM]] = val; \
	} else { \
		GetEA(ModRM); \
		WriteWord(ea, val); \
	} \
}

#define PutRMWordOffset(offs, val) \
	WriteWord(ea - eo + (uint16)(eo + offs), val)

#define PutRMByteOffset(offs, val) \
	WriteByte(ea - eo + (uint16)(eo + offs), val)

#define PutImmRMWord(ModRM) { \
	uint16 val; \
	if (ModRM >= 0xc0) { \
		FETCHWORD(regs.w[Mod_RM.RM.w[ModRM]]) \
	} else { \
		GetEA(ModRM); \
		FETCHWORD(val) \
		WriteWord(ea, val); \
	} \
}

#define GetRMByte(ModRM) \
	((ModRM) >= 0xc0 ? regs.b[Mod_RM.RM.b[ModRM]] : (GetEA(ModRM), ReadByte(ea)))

#define PutRMByte(ModRM, val) { \
	if (ModRM >= 0xc0) { \
		regs.b[Mod_RM.RM.b[ModRM]] = val; \
	} else { \
		GetEA(ModRM); \
		WriteByte(ea, val); \
	} \
}

#define PutImmRMByte(ModRM) { \
	if (ModRM >= 0xc0) { \
		regs.b[Mod_RM.RM.b[ModRM]] = FETCH; \
	} else { \
		GetEA(ModRM); \
		WriteByte(ea, FETCH); \
	} \
}

#define PutbackRMByte(ModRM, val) { \
	if (ModRM >= 0xc0) { \
		regs.b[Mod_RM.RM.b[ModRM]] = val; \
	} else { \
		WriteByte(ea, val); \
	} \
}

#define DEF_br8(dst, src) \
	unsigned ModRM = FETCHOP; \
	unsigned src = RegByte(ModRM); \
	unsigned dst = GetRMByte(ModRM)

#define DEF_wr16(dst, src) \
	unsigned ModRM = FETCHOP; \
	unsigned src = RegWord(ModRM); \
	unsigned dst = GetRMWord(ModRM)

#define DEF_r8b(dst, src) \
	unsigned ModRM = FETCHOP; \
	unsigned dst = RegByte(ModRM); \
	unsigned src = GetRMByte(ModRM)

#define DEF_r16w(dst, src) \
	unsigned ModRM = FETCHOP; \
	unsigned dst = RegWord(ModRM); \
	unsigned src = GetRMWord(ModRM)

#define DEF_ald8(dst, src) \
	unsigned src = FETCHOP; \
	unsigned dst = regs.b[AL]

#define DEF_axd16(dst, src) \
	unsigned src = FETCHOP; \
	unsigned dst = regs.w[AX]; \
	src += (FETCH << 8)

/************************************************************************/

#ifndef INLINE
#define INLINE inline
#endif

#define offs_t UINT32

// Disassembler constants
const UINT32 DASMFLAG_SUPPORTED     = 0x80000000;   // are disassembly flags supported?
const UINT32 DASMFLAG_STEP_OUT      = 0x40000000;   // this instruction should be the end of a step out sequence
const UINT32 DASMFLAG_STEP_OVER     = 0x20000000;   // this instruction should be stepped over by setting a breakpoint afterwards
const UINT32 DASMFLAG_OVERINSTMASK  = 0x18000000;   // number of extra instructions to skip when stepping over
const UINT32 DASMFLAG_OVERINSTSHIFT = 27;           // bits to shift after masking to get the value
const UINT32 DASMFLAG_LENGTHMASK    = 0x0000ffff;   // the low 16-bits contain the actual length

/* Highly useful macro for compile-time knowledge of an array size */
#define ARRAY_LENGTH(x)     (sizeof(x) / sizeof(x[0]))

int necv_dasm_one(char *buffer, UINT32 eip, const UINT8 *oprom);

void I86::initialize()
{
	static const BREGS reg_name[8] = {AL, CL, DL, BL, AH, CH, DH, BH};
	
	for(int i = 0; i < 256; i++) {
		Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = (WREGS)((i & 0x38) >> 3);
	}
	for(int i = 0xc0; i < 0x100; i++) {
		Mod_RM.RM.w[i] = (WREGS)(i & 7);
		Mod_RM.RM.b[i] = (BREGS)reg_name[i & 7];
	}
#ifdef USE_DEBUGGER
	d_mem_stored = d_mem;
	d_io_stored = d_io;
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void I86::reset()
{
	for(int i = 0; i < 8; i++) {
		regs.w[i] = 0;
	}
	sregs[CS] = 0xf000;
	sregs[SS] = sregs[DS] = sregs[ES] = 0;
	
	base[CS] = SegBase(CS);
	base[SS] = base[DS] = base[ES] = 0;
	
	ea = 0;
	eo = 0;
	AuxVal = OverVal = SignVal = ZeroVal = CarryVal = 0;
	DirVal = 1;
	ParityVal = TF = IF = MF = 0;
	
	icount = extra_icount = 0;
	int_state = 0;
	test_state = false;
	halted = false;
	
	pc = 0xffff0 & AMASK;
	flags = 0;
	ExpandFlags(flags);
#ifdef HAS_V30
	SetMD(1);
#endif
	seg_prefix = false;
}

int I86::run(int clock)
{
	/* return now if BUSREQ */
	if(busreq) {
#ifdef SINGLE_MODE_DMA
		if(d_dma) {
			d_dma->do_dma();
		}
#endif
		int passed_icount = max(1, extra_icount);
		icount = extra_icount = 0;
		return passed_icount;
	}
	
	// run cpu
	if(clock == -1) {
		// run only one opcode
		icount = -extra_icount;
		extra_icount = 0;
#ifdef USE_DEBUGGER
		run_one_opecode_debugger();
#else
		run_one_opecode();
#endif
		return -icount;
	} else {
		/* run cpu while given clocks */
		icount += clock;
		int first_icount = icount;
		icount -= extra_icount;
		extra_icount = 0;
		
		while(icount > 0 && !busreq) {
#ifdef USE_DEBUGGER
			run_one_opecode_debugger();
#else
			run_one_opecode();
#endif
		}
		int passed_icount = first_icount - icount;
		if(busreq && icount > 0) {
			icount = 0;
		}
		return passed_icount;
	}
}

#ifdef USE_DEBUGGER
void I86::run_one_opecode_debugger()
{
	bool now_debugging = d_debugger->now_debugging;
	if(now_debugging) {
		d_debugger->check_break_points(pc);
		if(d_debugger->now_suspended) {
			emu->mute_sound();
			while(d_debugger->now_debugging && d_debugger->now_suspended) {
				Sleep(10);
			}
		}
		if(d_debugger->now_debugging) {
			d_mem = d_io = d_debugger;
		} else {
			now_debugging = false;
		}
		
		run_one_opecode();
		if(now_debugging) {
			if(!d_debugger->now_going) {
				d_debugger->now_suspended = true;
			}
			d_mem = d_mem_stored;
			d_io = d_io_stored;
		}
	} else {
		run_one_opecode();
	}
}
#endif

void I86::run_one_opecode()
{
	seg_prefix = false;
#ifdef _JX
	// ugly patch for PC/JX hardware diagnostics :-(
#ifdef TIMER_HACK
	if(pc == 0xff040) pc = 0xff04a;
	if(pc == 0xff17d) pc = 0xff18f;
#endif
#ifdef KEYBOARD_HACK
	if(pc == 0xfa909) { regs.b[BH] = read_port_byte(0xa1); pc = 0xfa97c; }
	if(pc == 0xff6e1) { regs.b[AL] = 0x0d; pc += 2; }
#endif
#endif
	instruction(FETCHOP);
	if(int_state & NMI_REQ_BIT) {
		if(halted) {
			pc++;
			halted = false;
		}
		int_state &= ~NMI_REQ_BIT;
		interrupt(NMI_INT_VECTOR);
	} else if((int_state & INT_REQ_BIT) && IF) {
		if(halted) {
			pc++;
			halted = false;
		}
		interrupt(-1);
	}
#ifdef SINGLE_MODE_DMA
	if(d_dma) {
		d_dma->do_dma();
	}
#endif
	icount -= extra_icount;
	extra_icount = 0;
}

void I86::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_CPU_NMI) {
		if(data & mask) {
			int_state |= NMI_REQ_BIT;
		} else {
			int_state &= ~NMI_REQ_BIT;
		}
	} else if(id == SIG_CPU_BUSREQ) {
		busreq = ((data & mask) != 0);
	} else if(id == SIG_I86_TEST) {
		test_state = ((data & mask) != 0);
	}
}

void I86::set_intr_line(bool line, bool pending, uint32 bit)
{
	if(line) {
		int_state |= INT_REQ_BIT;
	} else {
		int_state &= ~INT_REQ_BIT;
	}
}

#ifdef USE_DEBUGGER
void I86::debug_write_data8(uint32 addr, uint32 data)
{
	int wait;
	d_mem_stored->write_data8w(addr, data, &wait);
}

uint32 I86::debug_read_data8(uint32 addr)
{
	int wait;
	return d_mem_stored->read_data8w(addr, &wait);
}

void I86::debug_write_data16(uint32 addr, uint32 data)
{
	int wait;
	d_mem_stored->write_data16w(addr, data, &wait);
}

uint32 I86::debug_read_data16(uint32 addr)
{
	int wait;
	return d_mem_stored->read_data16w(addr, &wait);
}

void I86::debug_write_io8(uint32 addr, uint32 data)
{
	int wait;
	d_io_stored->write_io8w(addr, data, &wait);
}

uint32 I86::debug_read_io8(uint32 addr) {
	int wait;
	return d_io_stored->read_io8w(addr, &wait);
}

void I86::debug_write_io16(uint32 addr, uint32 data)
{
	int wait;
	d_io_stored->write_io16w(addr, data, &wait);
}

uint32 I86::debug_read_io16(uint32 addr) {
	int wait;
	return d_io_stored->read_io16w(addr, &wait);
}

bool I86::debug_write_reg(_TCHAR *reg, uint32 data)
{
	if(_tcsicmp(reg, _T("IP")) == 0) {
		pc = ((data & 0xffff) + base[CS]) & AMASK;
	} else if(_tcsicmp(reg, _T("AX")) == 0) {
		regs.w[AX] = data;
	} else if(_tcsicmp(reg, _T("BX")) == 0) {
		regs.w[BX] = data;
	} else if(_tcsicmp(reg, _T("CX")) == 0) {
		regs.w[CX] = data;
	} else if(_tcsicmp(reg, _T("DX")) == 0) {
		regs.w[DX] = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		regs.w[SP] = data;
	} else if(_tcsicmp(reg, _T("BP")) == 0) {
		regs.w[BP] = data;
	} else if(_tcsicmp(reg, _T("SI")) == 0) {
		regs.w[SI] = data;
	} else if(_tcsicmp(reg, _T("DI")) == 0) {
		regs.w[DI] = data;
	} else if(_tcsicmp(reg, _T("AL")) == 0) {
		regs.b[AL] = data;
	} else if(_tcsicmp(reg, _T("AH")) == 0) {
		regs.b[AH] = data;
	} else if(_tcsicmp(reg, _T("BL")) == 0) {
		regs.b[BL] = data;
	} else if(_tcsicmp(reg, _T("BH")) == 0) {
		regs.b[BH] = data;
	} else if(_tcsicmp(reg, _T("CL")) == 0) {
		regs.b[CL] = data;
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		regs.b[CH] = data;
	} else if(_tcsicmp(reg, _T("DL")) == 0) {
		regs.b[DL] = data;
	} else if(_tcsicmp(reg, _T("DH")) == 0) {
		regs.b[DH] = data;
	} else {
		return false;
	}
	return true;
}

void I86::debug_regs_info(_TCHAR *buffer)
{
	_stprintf(buffer, _T("AX=%04X  BX=%04X CX=%04X DX=%04X SP=%04X  BP=%04X  SI=%04X  DI=%04X\nDS=%04X  ES=%04X SS=%04X CS=%04X IP=%04X  FLAG=[%c%c%c%c%c%c%c%c%c]"),
	regs.w[AX], regs.w[BX], regs.w[CX], regs.w[DX], regs.w[SP], regs.w[BP], regs.w[SI], regs.w[DI], sregs[DS], sregs[ES], sregs[SS], sregs[CS], (uint16)(pc - base[CS]),
	OF ? _T('O') : _T('-'), DF ? _T('D') : _T('-'), IF ? _T('I') : _T('-'), TF ? _T('T') : _T('-'),
	SF ? _T('S') : _T('-'), ZF ? _T('Z') : _T('-'), AF ? _T('A') : _T('-'), PF ? _T('P') : _T('-'), CF ? _T('C') : _T('-'));
}

int I86::debug_dasm(uint32 pc, _TCHAR *buffer)
{
	UINT32 eip = (UINT32)(uint16)(pc - base[CS]);
	UINT8 ops[16];
	for(int i = 0; i < 16; i++) {
		int wait;
		ops[i] = d_mem->read_data8w(pc + i, &wait);
	}
	return necv_dasm_one(buffer, eip, ops) & DASMFLAG_LENGTHMASK;
}
#endif

void I86::interrupt(int int_num)
{
	unsigned dest_seg, dest_off;
	uint16 ip = pc - base[CS];
	
	if(int_num == -1) {
		int_num = d_pic->intr_ack() & 0xff;
		int_state &= ~INT_REQ_BIT;
	}
	dest_off = ReadWord(int_num * 4);
	dest_seg = ReadWord(int_num * 4 + 2);
	
	_pushf();
	TF = IF = 0;
	PUSH(sregs[CS]);
	PUSH(ip);
	sregs[CS] = (uint16)dest_seg;
	base[CS] = SegBase(CS);
	pc = (base[CS] + dest_off) & AMASK;
	icount -= timing.exception;
}

void I86::trap()
{
	instruction(FETCHOP);
	interrupt(1);
}

unsigned I86::GetEA(unsigned ModRM)
{
	switch(ModRM) {
	case 0x00: case 0x08: case 0x10: case 0x18: case 0x20: case 0x28: case 0x30: case 0x38:
		icount -= 7; eo = (uint16)(regs.w[BX] + regs.w[SI]); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x01: case 0x09: case 0x11: case 0x19: case 0x21: case 0x29: case 0x31: case 0x39:
		icount -= 8; eo = (uint16)(regs.w[BX] + regs.w[DI]); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x02: case 0x0a: case 0x12: case 0x1a: case 0x22: case 0x2a: case 0x32: case 0x3a:
		icount -= 8; eo = (uint16)(regs.w[BP] + regs.w[SI]); ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + eo; return ea;
	case 0x03: case 0x0b: case 0x13: case 0x1b: case 0x23: case 0x2b: case 0x33: case 0x3b:
		icount -= 7; eo = (uint16)(regs.w[BP] + regs.w[DI]); ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + eo; return ea;
	case 0x04: case 0x0c: case 0x14: case 0x1c: case 0x24: case 0x2c: case 0x34: case 0x3c:
		icount -= 5; eo = regs.w[SI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x05: case 0x0d: case 0x15: case 0x1d: case 0x25: case 0x2d: case 0x35: case 0x3d:
		icount -= 5; eo = regs.w[DI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x06: case 0x0e: case 0x16: case 0x1e: case 0x26: case 0x2e: case 0x36: case 0x3e:
		icount -= 6; eo = FETCHOP; eo += FETCHOP << 8; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x07: case 0x0f: case 0x17: case 0x1f: case 0x27: case 0x2f: case 0x37: case 0x3f:
		icount -= 5; eo = regs.w[BX]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;

	case 0x40: case 0x48: case 0x50: case 0x58: case 0x60: case 0x68: case 0x70: case 0x78:
		icount -= 11; eo = (uint16)(regs.w[BX] + regs.w[SI] + (int8)FETCHOP); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x41: case 0x49: case 0x51: case 0x59: case 0x61: case 0x69: case 0x71: case 0x79:
		icount -= 12; eo = (uint16)(regs.w[BX] + regs.w[DI] + (int8)FETCHOP); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x42: case 0x4a: case 0x52: case 0x5a: case 0x62: case 0x6a: case 0x72: case 0x7a:
		icount -= 12; eo = (uint16)(regs.w[BP] + regs.w[SI] + (int8)FETCHOP); ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + eo; return ea;
	case 0x43: case 0x4b: case 0x53: case 0x5b: case 0x63: case 0x6b: case 0x73: case 0x7b:
		icount -= 11; eo = (uint16)(regs.w[BP] + regs.w[DI] + (int8)FETCHOP); ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + eo; return ea;
	case 0x44: case 0x4c: case 0x54: case 0x5c: case 0x64: case 0x6c: case 0x74: case 0x7c:
		icount -= 9; eo = (uint16)(regs.w[SI] + (int8)FETCHOP); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x45: case 0x4d: case 0x55: case 0x5d: case 0x65: case 0x6d: case 0x75: case 0x7d:
		icount -= 9; eo = (uint16)(regs.w[DI] + (int8)FETCHOP); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x46: case 0x4e: case 0x56: case 0x5e: case 0x66: case 0x6e: case 0x76: case 0x7e:
		icount -= 9; eo = (uint16)(regs.w[BP] + (int8)FETCHOP); ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + eo; return ea;
	case 0x47: case 0x4f: case 0x57: case 0x5f: case 0x67: case 0x6f: case 0x77: case 0x7f:
		icount -= 9; eo = (uint16)(regs.w[BX] + (int8)FETCHOP); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;

	case 0x80: case 0x88: case 0x90: case 0x98: case 0xa0: case 0xa8: case 0xb0: case 0xb8:
		icount -= 11; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[BX] + regs.w[SI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + (uint16)eo; return ea;
	case 0x81: case 0x89: case 0x91: case 0x99: case 0xa1: case 0xa9: case 0xb1: case 0xb9:
		icount -= 12; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[BX] + regs.w[DI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + (uint16)eo; return ea;
	case 0x82: case 0x8a: case 0x92: case 0x9a: case 0xa2: case 0xaa: case 0xb2: case 0xba:
		icount -= 12; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[BP] + regs.w[SI]; ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + (uint16)eo; return ea;
	case 0x83: case 0x8b: case 0x93: case 0x9b: case 0xa3: case 0xab: case 0xb3: case 0xbb:
		icount -= 11; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[BP] + regs.w[DI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(SS) + (uint16)eo; return ea;
	case 0x84: case 0x8c: case 0x94: case 0x9c: case 0xa4: case 0xac: case 0xb4: case 0xbc:
		icount -= 9; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[SI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + (uint16)eo; return ea;
	case 0x85: case 0x8d: case 0x95: case 0x9d: case 0xa5: case 0xad: case 0xb5: case 0xbd:
		icount -= 9; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[DI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + (uint16)eo; return ea;
	case 0x86: case 0x8e: case 0x96: case 0x9e: case 0xa6: case 0xae: case 0xb6: case 0xbe:
		icount -= 9; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[BP]; ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + (uint16)eo; return ea;
	case 0x87: case 0x8f: case 0x97: case 0x9f: case 0xa7: case 0xaf: case 0xb7: case 0xbf:
		icount -= 9; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[BX]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + (uint16)eo; return ea;
	}
	return 0;
}

void I86::rotate_shift_byte(unsigned ModRM, unsigned count)
{
	unsigned src = (unsigned)GetRMByte(ModRM);
	unsigned dst = src;
	
	if(count == 0) {
		icount -= (ModRM >= 0xc0) ? timing.rot_reg_base : timing.rot_m8_base;
	} else if(count == 1) {
		icount -= (ModRM >= 0xc0) ? timing.rot_reg_1 : timing.rot_m8_1;
		
		switch((ModRM >> 3) & 7) {
		case 0:	/* ROL eb, 1 */
			CarryVal = src & 0x80;
			dst = (src << 1) + CF;
			PutbackRMByte(ModRM, dst);
			OverVal = (src ^ dst) & 0x80;
			break;
		case 1:	/* ROR eb, 1 */
			CarryVal = src & 0x01;
			dst = ((CF << 8) + src) >> 1;
			PutbackRMByte(ModRM, dst);
			OverVal = (src ^ dst) & 0x80;
			break;
		case 2:	/* RCL eb, 1 */
			dst = (src << 1) + CF;
			PutbackRMByte(ModRM, dst);
			SetCFB(dst);
			OverVal = (src ^ dst) & 0x80;
			break;
		case 3:	/* RCR eb, 1 */
			dst = ((CF << 8) + src) >> 1;
			PutbackRMByte(ModRM, dst);
			CarryVal = src & 0x01;
			OverVal = (src ^ dst) & 0x80;
			break;
		case 4:	/* SHL eb, 1 */
		case 6:
			dst = src << 1;
			PutbackRMByte(ModRM, dst);
			SetCFB(dst);
			OverVal = (src ^ dst) & 0x80;
			AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		case 5:	/* SHR eb, 1 */
			dst = src >> 1;
			PutbackRMByte(ModRM, dst);
			CarryVal = src & 0x01;
			OverVal = src & 0x80;
			AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		case 7:	/* SAR eb, 1 */
			dst = ((int8)src) >> 1;
			PutbackRMByte(ModRM, dst);
			CarryVal = src & 0x01;
			OverVal = 0;
			AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		default:
			__assume(0);
		}
	} else {
		icount -= (ModRM >= 0xc0) ? timing.rot_reg_base + timing.rot_reg_bit : timing.rot_m8_base + timing.rot_m8_bit;
		
		switch((ModRM >> 3) & 7) {
		case 0:	/* ROL eb, count */
			for(; count > 0; count--) {
				CarryVal = dst & 0x80;
				dst = (dst << 1) + CF;
			}
			PutbackRMByte(ModRM, (uint8)dst);
			break;
		case 1:	/* ROR eb, count */
			for(; count > 0; count--) {
				CarryVal = dst & 0x01;
				dst = (dst >> 1) + (CF << 7);
			}
			PutbackRMByte(ModRM, (uint8)dst);
			break;
		case 2:	/* RCL eb, count */
			for(; count > 0; count--) {
				dst = (dst << 1) + CF;
				SetCFB(dst);
			}
			PutbackRMByte(ModRM, (uint8)dst);
			break;
		case 3:	/* RCR eb, count */
			for(; count > 0; count--) {
				dst = (CF << 8) + dst;
				CarryVal = dst & 0x01;
				dst >>= 1;
			}
			PutbackRMByte(ModRM, (uint8)dst);
			break;
		case 4:	/* SHL eb, count */
		case 6:
			dst <<= count;
			SetCFB(dst);
			AuxVal = 1;
			SetSZPF_Byte(dst);
			PutbackRMByte(ModRM, (uint8)dst);
			break;
		case 5:	/* SHR eb, count */
			dst >>= count - 1;
			CarryVal = dst & 0x01;
			dst >>= 1;
			SetSZPF_Byte(dst);
			AuxVal = 1;
			PutbackRMByte(ModRM, (uint8)dst);
			break;
		case 7:	/* SAR eb, count */
			dst = ((int8)dst) >> (count - 1);
			CarryVal = dst & 0x01;
			dst = ((int8)((uint8)dst)) >> 1;
			SetSZPF_Byte(dst);
			AuxVal = 1;
			PutbackRMByte(ModRM, (uint8)dst);
			break;
		default:
			__assume(0);
		}
	}
}

void I86::rotate_shift_word(unsigned ModRM, unsigned count)
{
	unsigned src = GetRMWord(ModRM);
	unsigned dst = src;
	
	if(count == 0) {
		icount -= (ModRM >= 0xc0) ? timing.rot_reg_base : timing.rot_m16_base;
	} else if(count == 1) {
		icount -= (ModRM >= 0xc0) ? timing.rot_reg_1 : timing.rot_m16_1;
		
		switch((ModRM >> 3) & 7) {
		case 0:	/* ROL ew, 1 */
			CarryVal = src & 0x8000;
			dst = (src << 1) + CF;
			PutbackRMWord(ModRM, dst);
			OverVal = (src ^ dst) & 0x8000;
			break;
		case 1:	/* ROR ew, 1 */
			CarryVal = src & 0x01;
			dst = ((CF << 16) + src) >> 1;
			PutbackRMWord(ModRM, dst);
			OverVal = (src ^ dst) & 0x8000;
			break;
		case 2:	/* RCL ew, 1 */
			dst = (src << 1) + CF;
			PutbackRMWord(ModRM, dst);
			SetCFW(dst);
			OverVal = (src ^ dst) & 0x8000;
			break;
		case 3:	/* RCR ew, 1 */
			dst = ((CF << 16) + src) >> 1;
			PutbackRMWord(ModRM, dst);
			CarryVal = src & 0x01;
			OverVal = (src ^ dst) & 0x8000;
			break;
		case 4:	/* SHL ew, 1 */
		case 6:
			dst = src << 1;
			PutbackRMWord(ModRM, dst);
			SetCFW(dst);
			OverVal = (src ^ dst) & 0x8000;
			AuxVal = 1;
			SetSZPF_Word(dst);
			break;
		case 5:	/* SHR ew, 1 */
			dst = src >> 1;
			PutbackRMWord(ModRM, dst);
			CarryVal = src & 0x01;
			OverVal = src & 0x8000;
			AuxVal = 1;
			SetSZPF_Word(dst);
			break;
		case 7:	/* SAR ew, 1 */
			dst = ((int16)src) >> 1;
			PutbackRMWord(ModRM, dst);
			CarryVal = src & 0x01;
			OverVal = 0;
			AuxVal = 1;
			SetSZPF_Word(dst);
			break;
		default:
			__assume(0);
		}
	} else {
		icount -= (ModRM >= 0xc0) ? timing.rot_reg_base + timing.rot_reg_bit : timing.rot_m8_base + timing.rot_m16_bit;
		
		switch((ModRM >> 3) & 7) {
		case 0:	/* ROL ew, count */
			for(; count > 0; count--) {
				CarryVal = dst & 0x8000;
				dst = (dst << 1) + CF;
			}
			PutbackRMWord(ModRM, dst);
			break;
		case 1:	/* ROR ew, count */
			for(; count > 0; count--) {
				CarryVal = dst & 0x01;
				dst = (dst >> 1) + (CF << 15);
			}
			PutbackRMWord(ModRM, dst);
			break;
		case 2:	/* RCL ew, count */
			for(; count > 0; count--) {
				dst = (dst << 1) + CF;
				SetCFW(dst);
			}
			PutbackRMWord(ModRM, dst);
			break;
		case 3:	/* RCR ew, count */
			for(; count > 0; count--) {
				dst = dst + (CF << 16);
				CarryVal = dst & 0x01;
				dst >>= 1;
			}
			PutbackRMWord(ModRM, dst);
			break;
		case 4:	/* SHL ew, count */
		case 6:
			dst <<= count;
			SetCFW(dst);
			AuxVal = 1;
			SetSZPF_Word(dst);
			PutbackRMWord(ModRM, dst);
			break;
		case 5:	/* SHR ew, count */
			dst >>= count - 1;
			CarryVal = dst & 0x01;
			dst >>= 1;
			SetSZPF_Word(dst);
			AuxVal = 1;
			PutbackRMWord(ModRM, dst);
			break;
		case 7:	/* SAR ew, count */
			dst = ((int16)dst) >> (count - 1);
			CarryVal = dst & 0x01;
			dst = ((int16)((uint16)dst)) >> 1;
			SetSZPF_Word(dst);
			AuxVal = 1;
			PutbackRMWord(ModRM, dst);
			break;
		default:
			__assume(0);
		}
	}
}

void I86::instruction(uint8 code)
{
	prevpc = pc - 1;
	
	switch(code) {
	case 0x00: _add_br8(); break;
	case 0x01: _add_wr16(); break;
	case 0x02: _add_r8b(); break;
	case 0x03: _add_r16w(); break;
	case 0x04: _add_ald8(); break;
	case 0x05: _add_axd16(); break;
	case 0x06: _push_es(); break;
	case 0x07: _pop_es(); break;
	case 0x08: _or_br8(); break;
	case 0x09: _or_wr16(); break;
	case 0x0a: _or_r8b(); break;
	case 0x0b: _or_r16w(); break;
	case 0x0c: _or_ald8(); break;
	case 0x0d: _or_axd16(); break;
	case 0x0e: _push_cs(); break;
#if defined(HAS_V30)
	case 0x0f: _0fpre(); break;
#else
	case 0x0f: _invalid(); break;
#endif
	case 0x10: _adc_br8(); break;
	case 0x11: _adc_wr16(); break;
	case 0x12: _adc_r8b(); break;
	case 0x13: _adc_r16w(); break;
	case 0x14: _adc_ald8(); break;
	case 0x15: _adc_axd16(); break;
	case 0x16: _push_ss(); break;
	case 0x17: _pop_ss(); break;
	case 0x18: _sbb_br8(); break;
	case 0x19: _sbb_wr16(); break;
	case 0x1a: _sbb_r8b(); break;
	case 0x1b: _sbb_r16w(); break;
	case 0x1c: _sbb_ald8(); break;
	case 0x1d: _sbb_axd16(); break;
	case 0x1e: _push_ds(); break;
	case 0x1f: _pop_ds(); break;
	case 0x20: _and_br8(); break;
	case 0x21: _and_wr16(); break;
	case 0x22: _and_r8b(); break;
	case 0x23: _and_r16w(); break;
	case 0x24: _and_ald8(); break;
	case 0x25: _and_axd16(); break;
	case 0x26: _es(); break;
	case 0x27: _daa(); break;
	case 0x28: _sub_br8(); break;
	case 0x29: _sub_wr16(); break;
	case 0x2a: _sub_r8b(); break;
	case 0x2b: _sub_r16w(); break;
	case 0x2c: _sub_ald8(); break;
	case 0x2d: _sub_axd16(); break;
	case 0x2e: _cs(); break;
	case 0x2f: _das(); break;
	case 0x30: _xor_br8(); break;
	case 0x31: _xor_wr16(); break;
	case 0x32: _xor_r8b(); break;
	case 0x33: _xor_r16w(); break;
	case 0x34: _xor_ald8(); break;
	case 0x35: _xor_axd16(); break;
	case 0x36: _ss(); break;
	case 0x37: _aaa(); break;
	case 0x38: _cmp_br8(); break;
	case 0x39: _cmp_wr16(); break;
	case 0x3a: _cmp_r8b(); break;
	case 0x3b: _cmp_r16w(); break;
	case 0x3c: _cmp_ald8(); break;
	case 0x3d: _cmp_axd16(); break;
	case 0x3e: _ds(); break;
	case 0x3f: _aas(); break;
	case 0x40: _inc_ax(); break;
	case 0x41: _inc_cx(); break;
	case 0x42: _inc_dx(); break;
	case 0x43: _inc_bx(); break;
	case 0x44: _inc_sp(); break;
	case 0x45: _inc_bp(); break;
	case 0x46: _inc_si(); break;
	case 0x47: _inc_di(); break;
	case 0x48: _dec_ax(); break;
	case 0x49: _dec_cx(); break;
	case 0x4a: _dec_dx(); break;
	case 0x4b: _dec_bx(); break;
	case 0x4c: _dec_sp(); break;
	case 0x4d: _dec_bp(); break;
	case 0x4e: _dec_si(); break;
	case 0x4f: _dec_di(); break;
	case 0x50: _push_ax(); break;
	case 0x51: _push_cx(); break;
	case 0x52: _push_dx(); break;
	case 0x53: _push_bx(); break;
	case 0x54: _push_sp(); break;
	case 0x55: _push_bp(); break;
	case 0x56: _push_si(); break;
	case 0x57: _push_di(); break;
	case 0x58: _pop_ax(); break;
	case 0x59: _pop_cx(); break;
	case 0x5a: _pop_dx(); break;
	case 0x5b: _pop_bx(); break;
	case 0x5c: _pop_sp(); break;
	case 0x5d: _pop_bp(); break;
	case 0x5e: _pop_si(); break;
	case 0x5f: _pop_di(); break;
#if defined(HAS_V30)
	case 0x60: _pusha(); break;
	case 0x61: _popa(); break;
	case 0x62: _bound(); break;
#else
	case 0x60: _invalid(); break;
	case 0x61: _invalid(); break;
	case 0x62: _invalid(); break;
#endif
	case 0x63: _invalid(); break;
#if defined(HAS_V30)
	case 0x64: _repc(0); break;
	case 0x65: _repc(1); break;
#else
	case 0x64: _invalid(); break;
	case 0x65: _invalid(); break;
#endif
	case 0x66: _invalid(); break;
	case 0x67: _invalid(); break;
#if defined(HAS_V30)
	case 0x68: _push_d16(); break;
	case 0x69: _imul_d16(); break;
	case 0x6a: _push_d8(); break;
	case 0x6b: _imul_d8(); break;
	case 0x6c: _insb(); break;
	case 0x6d: _insw(); break;
	case 0x6e: _outsb(); break;
	case 0x6f: _outsw(); break;
#else
	case 0x68: _invalid(); break;
	case 0x69: _invalid(); break;
	case 0x6a: _invalid(); break;
	case 0x6b: _invalid(); break;
	case 0x6c: _invalid(); break;
	case 0x6d: _invalid(); break;
	case 0x6e: _invalid(); break;
	case 0x6f: _invalid(); break;
#endif
	case 0x70: _jo(); break;
	case 0x71: _jno(); break;
	case 0x72: _jb(); break;
	case 0x73: _jnb(); break;
	case 0x74: _jz(); break;
	case 0x75: _jnz(); break;
	case 0x76: _jbe(); break;
	case 0x77: _jnbe(); break;
	case 0x78: _js(); break;
	case 0x79: _jns(); break;
	case 0x7a: _jp(); break;
	case 0x7b: _jnp(); break;
	case 0x7c: _jl(); break;
	case 0x7d: _jnl(); break;
	case 0x7e: _jle(); break;
	case 0x7f: _jnle(); break;
	case 0x80: _80pre(); break;
	case 0x81: _81pre(); break;
	case 0x82: _82pre(); break;
	case 0x83: _83pre(); break;
	case 0x84: _test_br8(); break;
	case 0x85: _test_wr16(); break;
	case 0x86: _xchg_br8(); break;
	case 0x87: _xchg_wr16(); break;
	case 0x88: _mov_br8(); break;
	case 0x89: _mov_wr16(); break;
	case 0x8a: _mov_r8b(); break;
	case 0x8b: _mov_r16w(); break;
	case 0x8c: _mov_wsreg(); break;
	case 0x8d: _lea(); break;
	case 0x8e: _mov_sregw(); break;
	case 0x8f: _popw(); break;
	case 0x90: _nop(); break;
	case 0x91: _xchg_axcx(); break;
	case 0x92: _xchg_axdx(); break;
	case 0x93: _xchg_axbx(); break;
	case 0x94: _xchg_axsp(); break;
	case 0x95: _xchg_axbp(); break;
	case 0x96: _xchg_axsi(); break;
	case 0x97: _xchg_axdi(); break;
	case 0x98: _cbw(); break;
	case 0x99: _cwd(); break;
	case 0x9a: _call_far(); break;
	case 0x9b: _wait(); break;
	case 0x9c: _pushf(); break;
	case 0x9d: _popf(); break;
	case 0x9e: _sahf(); break;
	case 0x9f: _lahf(); break;
	case 0xa0: _mov_aldisp(); break;
	case 0xa1: _mov_axdisp(); break;
	case 0xa2: _mov_dispal(); break;
	case 0xa3: _mov_dispax(); break;
	case 0xa4: _movsb(); break;
	case 0xa5: _movsw(); break;
	case 0xa6: _cmpsb(); break;
	case 0xa7: _cmpsw(); break;
	case 0xa8: _test_ald8(); break;
	case 0xa9: _test_axd16(); break;
	case 0xaa: _stosb(); break;
	case 0xab: _stosw(); break;
	case 0xac: _lodsb(); break;
	case 0xad: _lodsw(); break;
	case 0xae: _scasb(); break;
	case 0xaf: _scasw(); break;
	case 0xb0: _mov_ald8(); break;
	case 0xb1: _mov_cld8(); break;
	case 0xb2: _mov_dld8(); break;
	case 0xb3: _mov_bld8(); break;
	case 0xb4: _mov_ahd8(); break;
	case 0xb5: _mov_chd8(); break;
	case 0xb6: _mov_dhd8(); break;
	case 0xb7: _mov_bhd8(); break;
	case 0xb8: _mov_axd16(); break;
	case 0xb9: _mov_cxd16(); break;
	case 0xba: _mov_dxd16(); break;
	case 0xbb: _mov_bxd16(); break;
	case 0xbc: _mov_spd16(); break;
	case 0xbd: _mov_bpd16(); break;
	case 0xbe: _mov_sid16(); break;
	case 0xbf: _mov_did16(); break;
#if defined(HAS_V30)
	case 0xc0: _rotshft_bd8(); break;
	case 0xc1: _rotshft_wd8(); break;
#else
	case 0xc0: _invalid(); break;
	case 0xc1: _invalid(); break;
#endif
	case 0xc2: _ret_d16(); break;
	case 0xc3: _ret(); break;
	case 0xc4: _les_dw(); break;
	case 0xc5: _lds_dw(); break;
	case 0xc6: _mov_bd8(); break;
	case 0xc7: _mov_wd16(); break;
#if defined(HAS_V30)
	case 0xc8: _enter(); break;
	case 0xc9: _leav(); break;	/* _leave() */
#else
	case 0xc8: _invalid(); break;
	case 0xc9: _invalid(); break;
#endif
	case 0xca: _retf_d16(); break;
	case 0xcb: _retf(); break;
	case 0xcc: _int3(); break;
	case 0xcd: _int(); break;
	case 0xce: _into(); break;
	case 0xcf: _iret(); break;
	case 0xd0: _rotshft_b(); break;
	case 0xd1: _rotshft_w(); break;
	case 0xd2: _rotshft_bcl(); break;
	case 0xd3: _rotshft_wcl(); break;
	case 0xd4: _aam(); break;
	case 0xd5: _aad(); break;
#if defined(HAS_V30)
	case 0xd6: _setalc(); break;
#else
	case 0xd6: _invalid(); break;
#endif
	case 0xd7: _xlat(); break;
	case 0xd8: _escape(); break;
	case 0xd9: _escape(); break;
	case 0xda: _escape(); break;
	case 0xdb: _escape(); break;
	case 0xdc: _escape(); break;
	case 0xdd: _escape(); break;
	case 0xde: _escape(); break;
	case 0xdf: _escape(); break;
	case 0xe0: _loopne(); break;
	case 0xe1: _loope(); break;
	case 0xe2: _loop(); break;
	case 0xe3: _jcxz(); break;
	case 0xe4: _inal(); break;
	case 0xe5: _inax(); break;
	case 0xe6: _outal(); break;
	case 0xe7: _outax(); break;
	case 0xe8: _call_d16(); break;
	case 0xe9: _jmp_d16(); break;
	case 0xea: _jmp_far(); break;
	case 0xeb: _jmp_d8(); break;
	case 0xec: _inaldx(); break;
	case 0xed: _inaxdx(); break;
	case 0xee: _outdxal(); break;
	case 0xef: _outdxax(); break;
	case 0xf0: _lock(); break;
	case 0xf1: _invalid(); break;
	case 0xf2: _repne(); break;
	case 0xf3: _repe(); break;
	case 0xf4: _hlt(); break;
	case 0xf5: _cmc(); break;
	case 0xf6: _f6pre(); break;
	case 0xf7: _f7pre(); break;
	case 0xf8: _clc(); break;
	case 0xf9: _stc(); break;
	case 0xfa: _cli(); break;
	case 0xfb: _sti(); break;
	case 0xfc: _cld(); break;
	case 0xfd: _std(); break;
	case 0xfe: _fepre(); break;
	case 0xff: _ffpre(); break;
	default: __assume(0);
	}
}

inline void I86::_add_br8()    /* Opcode 0x00 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	ADDB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86::_add_wr16()    /* Opcode 0x01 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	ADDW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86::_add_r8b()    /* Opcode 0x02 */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ADDB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86::_add_r16w()    /* Opcode 0x03 */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ADDW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86::_add_ald8()    /* Opcode 0x04 */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	ADDB(dst, src);
	regs.b[AL] = dst;
}

inline void I86::_add_axd16()    /* Opcode 0x05 */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	ADDW(dst, src);
	regs.w[AX] = dst;
}

inline void I86::_push_es()    /* Opcode 0x06 */
{
	icount -= timing.push_seg;
	PUSH(sregs[ES]);
}

inline void I86::_pop_es()    /* Opcode 0x07 */
{
	POP(sregs[ES]);
	base[ES] = SegBase(ES);
	icount -= timing.pop_seg;
}

inline void I86::_or_br8()    /* Opcode 0x08 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	ORB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86::_or_wr16()    /* Opcode 0x09 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	ORW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86::_or_r8b()    /* Opcode 0x0a */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ORB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86::_or_r16w()    /* Opcode 0x0b */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ORW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86::_or_ald8()    /* Opcode 0x0c */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	ORB(dst, src);
	regs.b[AL] = dst;
}

inline void I86::_or_axd16()    /* Opcode 0x0d */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	ORW(dst, src);
	regs.w[AX] = dst;
}

inline void I86::_push_cs()    /* Opcode 0x0e */
{
	icount -= timing.push_seg;
	PUSH(sregs[CS]);
}

#if defined(HAS_V30)
inline void I86::_0fpre()    /* Opcode 0x0f */
{
	static const uint16 bytes[] = {
		1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
	};
	unsigned code = FETCH;
	unsigned ModRM;
	unsigned tmp;
	unsigned tmp2;
	
	switch(code) {
	case 0x10:  /* 0F 10 47 30 - TEST1 [bx+30h], cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 3;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 12;
		}
		tmp2 = regs.b[CL] & 7;
		SetZF(tmp & bytes[tmp2]);
		break;
	case 0x11:  /* 0F 11 47 30 - TEST1 [bx+30h], cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 3;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 12;
		}
		tmp2 = regs.b[CL] & 0xf;
		SetZF(tmp & bytes[tmp2]);
		break;
	case 0x12:  /* 0F 12 [mod:000:r/m] - CLR1 reg/m8, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 5;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 14;
		}
		tmp2 = regs.b[CL] & 7;
		tmp &= ~bytes[tmp2];
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x13:  /* 0F 13 [mod:000:r/m] - CLR1 reg/m16, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 5;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 14;
		}
		tmp2 = regs.b[CL] & 0xf;
		tmp &= ~bytes[tmp2];
		PutbackRMWord(ModRM, tmp);
		break;
	case 0x14:  /* 0F 14 47 30 - SET1 [bx+30h], cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 4;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 13;
		}
		tmp2 = regs.b[CL] & 7;
		tmp |= bytes[tmp2];
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x15:  /* 0F 15 C6 - SET1 si, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 4;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 13;
		}
		tmp2 = regs.b[CL] & 0xf;
		tmp |= bytes[tmp2];
		PutbackRMWord(ModRM, tmp);
		break;
	case 0x16:  /* 0F 16 C6 - NOT1 si, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 4;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 18;
		}
		tmp2 = regs.b[CL] & 7;
		if(tmp & bytes[tmp2]) {
			tmp &= ~bytes[tmp2];
		} else {
			tmp |= bytes[tmp2];
		}
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x17:  /* 0F 17 C6 - NOT1 si, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 4;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 18;
		}
		tmp2 = regs.b[CL] & 0xf;
		if(tmp & bytes[tmp2]) {
			tmp &= ~bytes[tmp2];
		} else {
			tmp |= bytes[tmp2];
		}
		PutbackRMWord(ModRM, tmp);
		break;
	case 0x18:  /* 0F 18 XX - TEST1 [bx+30h], 07 */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 4;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 13;
		}
		tmp2 = FETCH;
		tmp2 &= 0xf;
		SetZF(tmp & bytes[tmp2]);
		break;
	case 0x19:  /* 0F 19 XX - TEST1 [bx+30h], 07 */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 4;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 13;
		}
		tmp2 = FETCH;
		tmp2 &= 0xf;
		SetZF(tmp & bytes[tmp2]);
		break;
	case 0x1a:  /* 0F 1A 06 - CLR1 si, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 6;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 15;
		}
		tmp2 = FETCH;
		tmp2 &= 7;
		tmp &= ~bytes[tmp2];
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x1B:  /* 0F 1B 06 - CLR1 si, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 6;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 15;
		}
		tmp2 = FETCH;
		tmp2 &= 0xf;
		tmp &= ~bytes[tmp2];
		PutbackRMWord(ModRM, tmp);
		break;
	case 0x1C:  /* 0F 1C 47 30 - SET1 [bx+30h], cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 5;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 14;
		}
		tmp2 = FETCH;
		tmp2 &= 7;
		tmp |= bytes[tmp2];
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x1D:  /* 0F 1D C6 - SET1 si, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 5;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 14;
		}
		tmp2 = FETCH;
		tmp2 &= 0xf;
		tmp |= bytes[tmp2];
		PutbackRMWord(ModRM, tmp);
		break;
	case 0x1e:  /* 0F 1e C6 - NOT1 si, 07 */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 5;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 19;
		}
		tmp2 = FETCH;
		tmp2 &= 7;
		if(tmp & bytes[tmp2]) {
			tmp &= ~bytes[tmp2];
		} else {
			tmp |= bytes[tmp2];
		}
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x1f:  /* 0F 1f C6 - NOT1 si, 07 */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 5;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 19;
		}
		tmp2 = FETCH;
		tmp2 &= 0xf;
		if(tmp & bytes[tmp2]) {
			tmp &= ~bytes[tmp2];
		} else {
			tmp |= bytes[tmp2];
		}
		PutbackRMWord(ModRM, tmp);
		break;
	case 0x20:  /* 0F 20 59 - add4s */
		{
			/* length in words ! */
			int count = (regs.b[CL] + 1) / 2;
			unsigned di = regs.w[DI];
			unsigned si = regs.w[SI];
			
			ZeroVal = 1;
			CarryVal = 0;	/* NOT ADC */
			for(int i = 0; i < count; i++) {
				tmp = GetMemB(DS, si);
				tmp2 = GetMemB(ES, di);
				int v1 = (tmp >> 4) * 10 + (tmp & 0xf);
				int v2 = (tmp2 >> 4) * 10 + (tmp2 & 0xf);
				int result = v1 + v2 + CarryVal;
				CarryVal = result > 99 ? 1 : 0;
				result = result % 100;
				v1 = ((result / 10) << 4) | (result % 10);
				PutMemB(ES, di, v1);
				if(v1) {
					ZeroVal = 0;
				}
				si++;
				di++;
			}
			OverVal = CarryVal;
			icount -= 7 + 19 * count;
		}
		break;
	case 0x22:  /* 0F 22 59 - sub4s */
		{
			int count = (regs.b[CL] + 1) / 2;
			unsigned di = regs.w[DI];
			unsigned si = regs.w[SI];
			
			ZeroVal = 1;
			CarryVal = 0;  /* NOT ADC */
			for(int i = 0; i < count; i++) {
				tmp = GetMemB(ES, di);
				tmp2 = GetMemB(DS, si);
				int v1 = (tmp >> 4) * 10 + (tmp & 0xf);
				int v2 = (tmp2 >> 4) * 10 + (tmp2 & 0xf), result;
				if(v1 < (v2 + CarryVal)) {
					v1 += 100;
					result = v1 - (v2 + CarryVal);
					CarryVal = 1;
				} else {
					result = v1 - (v2 + CarryVal);
					CarryVal = 0;
				}
				v1 = ((result / 10) << 4) | (result % 10);
				PutMemB(ES, di, v1);
				if(v1) {
					ZeroVal = 0;
				}
				si++;
				di++;
			}
			OverVal = CarryVal;
			icount -= 7 + 19 * count;
		}
		break;
	case 0x25:
		icount -= 16;
		break;
	case 0x26:  /* 0F 22 59 - cmp4s */
		{
			int count = (regs.b[CL] + 1) / 2;
			unsigned di = regs.w[DI];
			unsigned si = regs.w[SI];
			
			ZeroVal = 1;
			CarryVal = 0;	/* NOT ADC */
			for(int i = 0; i < count; i++) {
				tmp = GetMemB(ES, di);
				tmp2 = GetMemB(DS, si);
				int v1 = (tmp >> 4) * 10 + (tmp & 0xf);
				int v2 = (tmp2 >> 4) * 10 + (tmp2 & 0xf), result;
				if(v1 < (v2 + CarryVal)) {
					v1 += 100;
					result = v1 - (v2 + CarryVal);
					CarryVal = 1;
				} else {
					result = v1 - (v2 + CarryVal);
					CarryVal = 0;
				}
				v1 = ((result / 10) << 4) | (result % 10);
				/* PutMemB(ES, di, v1);	/* no store, only compare */
				if(v1) {
					ZeroVal = 0;
				}
				si++;
				di++;
			}
			OverVal = CarryVal;
			icount -= 7 + 19 * (regs.b[CL] + 1);
		}
		break;
	case 0x28:  /* 0F 28 C7 - ROL4 bh */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 25;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 28;
		}
		tmp <<= 4;
		tmp |= regs.b[AL] & 0xf;
		regs.b[AL] = (regs.b[AL] & 0xf0) | ((tmp >> 8) & 0xf);
		tmp &= 0xff;
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x29:  /* 0F 29 C7 - ROL4 bx */
		ModRM = FETCH;
		break;
	case 0x2A:  /* 0F 2a c2 - ROR4 bh */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 29;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 33;
		}
		tmp2 = (regs.b[AL] & 0xf) << 4;
		regs.b[AL] = (regs.b[AL] & 0xf0) | (tmp & 0xf);
		tmp = tmp2 | (tmp >> 4);
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x2B:  /* 0F 2b c2 - ROR4 bx */
		ModRM = FETCH;
		break;
	case 0x2D:  /* 0Fh 2Dh < 1111 1RRR> */
		ModRM = FETCH;
		icount -= 15;
		break;
	case 0x31:  /* 0F 31 [mod:reg:r/m] - INS reg8, reg8 or INS reg8, imm4 */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 29;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 33;
		}
		break;
	case 0x33:  /* 0F 33 [mod:reg:r/m] - EXT reg8, reg8 or EXT reg8, imm4 */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 29;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 33;
		}
		break;
	case 0x91:
		icount -= 12;
		break;
	case 0x94:
		ModRM = FETCH;
		icount -= 11;
		break;
	case 0x95:
		ModRM = FETCH;
		icount -= 11;
		break;
	case 0xbe:
		icount -= 2;
		break;
	case 0xe0:
		ModRM = FETCH;
		icount -= 12;
		break;
	case 0xf0:
		ModRM = FETCH;
		icount -= 12;
		break;
	case 0xff:  /* 0F ff imm8 - BRKEM */
		ModRM = FETCH;
		icount -= 38;
		interrupt(ModRM);
		break;
	}
}
#endif

inline void I86::_adc_br8()    /* Opcode 0x10 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	src += CF;
	ADDB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86::_adc_wr16()    /* Opcode 0x11 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	src += CF;
	ADDW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86::_adc_r8b()    /* Opcode 0x12 */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	src += CF;
	ADDB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86::_adc_r16w()    /* Opcode 0x13 */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	src += CF;
	ADDW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86::_adc_ald8()    /* Opcode 0x14 */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	src += CF;
	ADDB(dst, src);
	regs.b[AL] = dst;
}

inline void I86::_adc_axd16()    /* Opcode 0x15 */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	src += CF;
	ADDW(dst, src);
	regs.w[AX] = dst;
}

inline void I86::_push_ss()    /* Opcode 0x16 */
{
	PUSH(sregs[SS]);
	icount -= timing.push_seg;
}

inline void I86::_pop_ss()    /* Opcode 0x17 */
{
	POP(sregs[SS]);
	base[SS] = SegBase(SS);
	icount -= timing.pop_seg;
	instruction(FETCHOP); /* no interrupt before next instruction */
}

inline void I86::_sbb_br8()    /* Opcode 0x18 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	src += CF;
	SUBB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86::_sbb_wr16()    /* Opcode 0x19 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	src += CF;
	SUBW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86::_sbb_r8b()    /* Opcode 0x1a */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	src += CF;
	SUBB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86::_sbb_r16w()    /* Opcode 0x1b */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	src += CF;
	SUBW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86::_sbb_ald8()    /* Opcode 0x1c */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	src += CF;
	SUBB(dst, src);
	regs.b[AL] = dst;
}

inline void I86::_sbb_axd16()    /* Opcode 0x1d */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	src += CF;
	SUBW(dst, src);
	regs.w[AX] = dst;
}

inline void I86::_push_ds()    /* Opcode 0x1e */
{
	PUSH(sregs[DS]);
	icount -= timing.push_seg;
}

inline void I86::_pop_ds()    /* Opcode 0x1f */
{
	POP(sregs[DS]);
	base[DS] = SegBase(DS);
	icount -= timing.push_seg;
}

inline void I86::_and_br8()    /* Opcode 0x20 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	ANDB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86::_and_wr16()    /* Opcode 0x21 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	ANDW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86::_and_r8b()    /* Opcode 0x22 */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ANDB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86::_and_r16w()    /* Opcode 0x23 */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ANDW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86::_and_ald8()    /* Opcode 0x24 */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	ANDB(dst, src);
	regs.b[AL] = dst;
}

inline void I86::_and_axd16()    /* Opcode 0x25 */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	ANDW(dst, src);
	regs.w[AX] = dst;
}

inline void I86::_es()    /* Opcode 0x26 */
{
	seg_prefix = true;
	prefix_seg = ES;
	icount -= timing.override;
	instruction(FETCHOP);
}

inline void I86::_daa()    /* Opcode 0x27 */
{
	if(AF || ((regs.b[AL] & 0xf) > 9)) {
		int tmp;
		regs.b[AL] = tmp = regs.b[AL] + 6;
		AuxVal = 1;
		CarryVal |= tmp & 0x100;
	}
	
	if(CF || (regs.b[AL] > 0x9f)) {
		regs.b[AL] += 0x60;
		CarryVal = 1;
	}
	
	SetSZPF_Byte(regs.b[AL]);
	icount -= timing.daa;
}

inline void I86::_sub_br8()    /* Opcode 0x28 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	SUBB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86::_sub_wr16()    /* Opcode 0x29 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	SUBW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86::_sub_r8b()    /* Opcode 0x2a */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	SUBB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86::_sub_r16w()    /* Opcode 0x2b */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	SUBW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86::_sub_ald8()    /* Opcode 0x2c */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	SUBB(dst, src);
	regs.b[AL] = dst;
}

inline void I86::_sub_axd16()    /* Opcode 0x2d */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	SUBW(dst, src);
	regs.w[AX] = dst;
}

inline void I86::_cs()    /* Opcode 0x2e */
{
	seg_prefix = true;
	prefix_seg = CS;
	icount -= timing.override;
	instruction(FETCHOP);
}

inline void I86::_das()    /* Opcode 0x2f */
{
	uint8 tmpAL = regs.b[AL];
	if(AF || ((regs.b[AL] & 0xf) > 9)) {
		int tmp;
		regs.b[AL] = tmp = regs.b[AL] - 6;
		AuxVal = 1;
		CarryVal |= tmp & 0x100;
	}
	
	if(CF || (tmpAL > 0x9f)) {
		regs.b[AL] -= 0x60;
		CarryVal = 1;
	}
	
	SetSZPF_Byte(regs.b[AL]);
	icount -= timing.das;
}

inline void I86::_xor_br8()    /* Opcode 0x30 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_mr8;
	XORB(dst, src);
	PutbackRMByte(ModRM, dst);
}

inline void I86::_xor_wr16()    /* Opcode 0x31 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_mr16;
	XORW(dst, src);
	PutbackRMWord(ModRM, dst);
}

inline void I86::_xor_r8b()    /* Opcode 0x32 */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	XORB(dst, src);
	RegByte(ModRM) = dst;
}

inline void I86::_xor_r16w()    /* Opcode 0x33 */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	XORW(dst, src);
	RegWord(ModRM) = dst;
}

inline void I86::_xor_ald8()    /* Opcode 0x34 */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	XORB(dst, src);
	regs.b[AL] = dst;
}

inline void I86::_xor_axd16()    /* Opcode 0x35 */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	XORW(dst, src);
	regs.w[AX] = dst;
}

inline void I86::_ss()    /* Opcode 0x36 */
{
	seg_prefix = true;
	prefix_seg = SS;
	icount -= timing.override;
	instruction(FETCHOP);
}

inline void I86::_aaa()    /* Opcode 0x37 */
{
	uint8 ALcarry = 1;
	if(regs.b[AL]>0xf9) {
		ALcarry = 2;
	}
	if(AF || ((regs.b[AL] & 0xf) > 9)) {
		regs.b[AL] += 6;
		regs.b[AH] += ALcarry;
		AuxVal = 1;
		CarryVal = 1;
	} else {
		AuxVal = 0;
		CarryVal = 0;
	}
	regs.b[AL] &= 0x0F;
	icount -= timing.aaa;
}

inline void I86::_cmp_br8()    /* Opcode 0x38 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	SUBB(dst, src);
}

inline void I86::_cmp_wr16()    /* Opcode 0x39 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	SUBW(dst, src);
}

inline void I86::_cmp_r8b()    /* Opcode 0x3a */
{
	DEF_r8b(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	SUBB(dst, src);
}

inline void I86::_cmp_r16w()    /* Opcode 0x3b */
{
	DEF_r16w(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	SUBW(dst, src);
}

inline void I86::_cmp_ald8()    /* Opcode 0x3c */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	SUBB(dst, src);
}

inline void I86::_cmp_axd16()    /* Opcode 0x3d */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	SUBW(dst, src);
}

inline void I86::_ds()    /* Opcode 0x3e */
{
	seg_prefix = true;
	prefix_seg = DS;
	icount -= timing.override;
	instruction(FETCHOP);
}

inline void I86::_aas()    /* Opcode 0x3f */
{
	uint8 ALcarry = 1;
	if(regs.b[AL] > 0xf9) {
		ALcarry = 2;
	}
	if(AF || ((regs.b[AL] & 0xf) > 9)) {
		regs.b[AL] -= 6;
		regs.b[AH] -= 1;
		AuxVal = 1;
		CarryVal = 1;
	} else {
		AuxVal = 0;
		CarryVal = 0;
	}
	regs.b[AL] &= 0x0F;
	icount -= timing.aas;
}

#define IncWordReg(Reg) { \
	unsigned tmp = (unsigned)regs.w[Reg]; \
	unsigned tmp1 = tmp + 1; \
	SetOFW_Add(tmp1, tmp, 1); \
	SetAF(tmp1, tmp, 1); \
	SetSZPF_Word(tmp1); \
	regs.w[Reg] = tmp1; \
	icount -= timing.incdec_r16; \
}

inline void I86::_inc_ax()    /* Opcode 0x40 */
{
	IncWordReg(AX);
}

inline void I86::_inc_cx()    /* Opcode 0x41 */
{
	IncWordReg(CX);
}

inline void I86::_inc_dx()    /* Opcode 0x42 */
{
	IncWordReg(DX);
}

inline void I86::_inc_bx()    /* Opcode 0x43 */
{
	IncWordReg(BX);
}

inline void I86::_inc_sp()    /* Opcode 0x44 */
{
	IncWordReg(SP);
}

inline void I86::_inc_bp()    /* Opcode 0x45 */
{
	IncWordReg(BP);
}

inline void I86::_inc_si()    /* Opcode 0x46 */
{
	IncWordReg(SI);
}

inline void I86::_inc_di()    /* Opcode 0x47 */
{
	IncWordReg(DI);
}

#define DecWordReg(Reg) { \
	unsigned tmp = (unsigned)regs.w[Reg]; \
	unsigned tmp1 = tmp - 1; \
	SetOFW_Sub(tmp1, 1, tmp); \
	SetAF(tmp1, tmp, 1); \
	SetSZPF_Word(tmp1); \
	regs.w[Reg] = tmp1; \
	icount -= timing.incdec_r16; \
}

inline void I86::_dec_ax()    /* Opcode 0x48 */
{
	DecWordReg(AX);
}

inline void I86::_dec_cx()    /* Opcode 0x49 */
{
	DecWordReg(CX);
}

inline void I86::_dec_dx()    /* Opcode 0x4a */
{
	DecWordReg(DX);
}

inline void I86::_dec_bx()    /* Opcode 0x4b */
{
	DecWordReg(BX);
}

inline void I86::_dec_sp()    /* Opcode 0x4c */
{
	DecWordReg(SP);
}

inline void I86::_dec_bp()    /* Opcode 0x4d */
{
	DecWordReg(BP);
}

inline void I86::_dec_si()    /* Opcode 0x4e */
{
	DecWordReg(SI);
}

inline void I86::_dec_di()    /* Opcode 0x4f */
{
	DecWordReg(DI);
}

inline void I86::_push_ax()    /* Opcode 0x50 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[AX]);
}

inline void I86::_push_cx()    /* Opcode 0x51 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[CX]);
}

inline void I86::_push_dx()    /* Opcode 0x52 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[DX]);
}

inline void I86::_push_bx()    /* Opcode 0x53 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[BX]);
}

inline void I86::_push_sp()    /* Opcode 0x54 */
{
	unsigned tmp = regs.w[SP];
	
	icount -= timing.push_r16;
	PUSH(tmp - 2);
}

inline void I86::_push_bp()    /* Opcode 0x55 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[BP]);
}

inline void I86::_push_si()    /* Opcode 0x56 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[SI]);
}

inline void I86::_push_di()    /* Opcode 0x57 */
{
	icount -= timing.push_r16;
	PUSH(regs.w[DI]);
}

inline void I86::_pop_ax()    /* Opcode 0x58 */
{
	icount -= timing.pop_r16;
	POP(regs.w[AX]);
}

inline void I86::_pop_cx()    /* Opcode 0x59 */
{
	icount -= timing.pop_r16;
	POP(regs.w[CX]);
}

inline void I86::_pop_dx()    /* Opcode 0x5a */
{
	icount -= timing.pop_r16;
	POP(regs.w[DX]);
}

inline void I86::_pop_bx()    /* Opcode 0x5b */
{
	icount -= timing.pop_r16;
	POP(regs.w[BX]);
}

inline void I86::_pop_sp()    /* Opcode 0x5c */
{
	unsigned tmp;
	
	icount -= timing.pop_r16;
	POP(tmp);
	regs.w[SP] = tmp;
}

inline void I86::_pop_bp()    /* Opcode 0x5d */
{
	icount -= timing.pop_r16;
	POP(regs.w[BP]);
}

inline void I86::_pop_si()    /* Opcode 0x5e */
{
	icount -= timing.pop_r16;
	POP(regs.w[SI]);
}

inline void I86::_pop_di()    /* Opcode 0x5f */
{
	icount -= timing.pop_r16;
	POP(regs.w[DI]);
}

inline void I86::_pusha()    /* Opcode 0x60 */
{
	unsigned tmp = regs.w[SP];
	
	icount -= timing.pusha;
	PUSH(regs.w[AX]);
	PUSH(regs.w[CX]);
	PUSH(regs.w[DX]);
	PUSH(regs.w[BX]);
	PUSH(tmp);
	PUSH(regs.w[BP]);
	PUSH(regs.w[SI]);
	PUSH(regs.w[DI]);
}

inline void I86::_popa()    /* Opcode 0x61 */
{
	unsigned tmp;
	
	icount -= timing.popa;
	POP(regs.w[DI]);
	POP(regs.w[SI]);
	POP(regs.w[BP]);
	POP(tmp);
	POP(regs.w[BX]);
	POP(regs.w[DX]);
	POP(regs.w[CX]);
	POP(regs.w[AX]);
}

inline void I86::_bound()    /* Opcode 0x62 */
{
	unsigned ModRM = FETCHOP;
	int low = (int16)GetRMWord(ModRM);
	int high = (int16)GetNextRMWord;
	int tmp = (int16)RegWord(ModRM);
	if(tmp < low || tmp>high) {
		pc -= (seg_prefix ? 3 : 2);
		interrupt(BOUNDS_CHECK_FAULT);
	}
	icount -= timing.bound;
}

inline void I86::_repc(int flagval)
{
#ifdef HAS_V30
	unsigned next = FETCHOP;
	unsigned count = regs.w[CX];
	
	switch(next) {
	case 0x26:	/* ES: */
		seg_prefix = true;
		prefix_seg = ES;
		icount -= 2;
		_repc(flagval);
		break;
	case 0x2e:	/* CS: */
		seg_prefix = true;
		prefix_seg = CS;
		icount -= 2;
		_repc(flagval);
		break;
	case 0x36:	/* SS: */
		seg_prefix = true;
		prefix_seg = SS;
		icount -= 2;
		_repc(flagval);
		break;
	case 0x3e:	/* DS: */
		seg_prefix = true;
		prefix_seg = DS;
		icount -= 2;
		_repc(flagval);
		break;
	case 0x6c:	/* REP INSB */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_insb();
		}
		regs.w[CX] = count;
		break;
	case 0x6d:	/* REP INSW */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_insw();
		}
		regs.w[CX] = count;
		break;
	case 0x6e:	/* REP OUTSB */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_outsb();
		}
		regs.w[CX] = count;
		break;
	case 0x6f:	/* REP OUTSW */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_outsw();
		}
		regs.w[CX] = count;
		break;
	case 0xa4:	/* REP MOVSB */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_movsb();
		}
		regs.w[CX] = count;
		break;
	case 0xa5:	/* REP MOVSW */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_movsw();
		}
		regs.w[CX] = count;
		break;
	case 0xa6:	/* REP(N)E CMPSB */
		icount -= 9;
		for(ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--) {
			_cmpsb();
		}
		regs.w[CX] = count;
		break;
	case 0xa7:	/* REP(N)E CMPSW */
		icount -= 9;
		for(ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--) {
			_cmpsw();
		}
		regs.w[CX] = count;
		break;
	case 0xaa:	/* REP STOSB */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_stosb();
		}
		regs.w[CX] = count;
		break;
	case 0xab:	/* REP STOSW */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_stosw();
		}
		regs.w[CX] = count;
		break;
	case 0xac:	/* REP LODSB */
		icount -= 9;
		for(; (CF == flagval) && (count > 0); count--) {
			_lodsb();
		}
		regs.w[CX] = count;
		break;
	case 0xad:	/* REP LODSW */
		icount -= 9;
		for(; (CF == flagval) && (count > 0); count--) {
			_lodsw();
		}
		regs.w[CX] = count;
		break;
	case 0xae:	/* REP(N)E SCASB */
		icount -= 9;
		for(ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--) {
			_scasb();
		}
		regs.w[CX] = count;
		break;
	case 0xaf:	/* REP(N)E SCASW */
		icount -= 9;
		for(ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--) {
			_scasw();
		}
		regs.w[CX] = count;
		break;
	default:
		instruction(next);
	}
#endif
}

inline void I86::_push_d16()    /* Opcode 0x68 */
{
	unsigned tmp = FETCH;
	icount -= timing.push_imm;
	tmp += FETCH << 8;
	PUSH(tmp);
}

inline void I86::_imul_d16()    /* Opcode 0x69 */
{
	DEF_r16w(dst, src);
	unsigned src2 = FETCH;
	src += (FETCH << 8);
	icount -= (ModRM >= 0xc0) ? timing.imul_rri16 : timing.imul_rmi16;
	dst = (int32)((int16)src) * (int32)((int16)src2);
	CarryVal = OverVal = (((int32)dst) >> 15 != 0) && (((int32)dst) >> 15 != -1);
	RegWord(ModRM) = (uint16)dst;
}

inline void I86::_push_d8()    /* Opcode 0x6a */
{
	unsigned tmp = (uint16)((int16)((int8)FETCH));
	icount -= timing.push_imm;
	PUSH(tmp);
}

inline void I86::_imul_d8()    /* Opcode 0x6b */
{
	DEF_r16w(dst, src);
	unsigned src2 = (uint16)((int16)((int8)FETCH));
	icount -= (ModRM >= 0xc0) ? timing.imul_rri8 : timing.imul_rmi8;
	dst = (int32)((int16)src) * (int32)((int16)src2);
	CarryVal = OverVal = (((int32)dst) >> 15 != 0) && (((int32)dst) >> 15 != -1);
	RegWord(ModRM) = (uint16)dst;
}

inline void I86::_insb()    /* Opcode 0x6c */
{
	icount -= timing.ins8;
	PutMemB(ES, regs.w[DI], read_port_byte(regs.w[DX]));
	regs.w[DI] += DirVal;
}

inline void I86::_insw()    /* Opcode 0x6d */
{
	icount -= timing.ins16;
	PutMemW(ES, regs.w[DI], read_port_word(regs.w[DX]));
	regs.w[DI] += 2 * DirVal;
}

inline void I86::_outsb()    /* Opcode 0x6e */
{
	icount -= timing.outs8;
	write_port_byte(regs.w[DX], GetMemB(DS, regs.w[SI]));
	regs.w[SI] += DirVal; /* GOL 11/27/01 */
}

inline void I86::_outsw()    /* Opcode 0x6f */
{
	icount -= timing.outs16;
	write_port_word(regs.w[DX], GetMemW(DS, regs.w[SI]));
	regs.w[SI] += 2 * DirVal; /* GOL 11/27/01 */
}

inline void I86::_jo()    /* Opcode 0x70 */
{
	int tmp = (int)((int8)FETCH);
	if(OF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jno()    /* Opcode 0x71 */
{
	int tmp = (int)((int8)FETCH);
	if(!OF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jb()    /* Opcode 0x72 */
{
	int tmp = (int)((int8)FETCH);
	if(CF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jnb()    /* Opcode 0x73 */
{
	int tmp = (int)((int8)FETCH);
	if(!CF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jz()    /* Opcode 0x74 */
{
	int tmp = (int)((int8)FETCH);
	if(ZF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jnz()    /* Opcode 0x75 */
{
	int tmp = (int)((int8)FETCH);
	if(!ZF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jbe()    /* Opcode 0x76 */
{
	int tmp = (int)((int8)FETCH);
	if(CF || ZF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jnbe()    /* Opcode 0x77 */
{
	int tmp = (int)((int8)FETCH);
	if(!(CF || ZF)) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_js()    /* Opcode 0x78 */
{
	int tmp = (int)((int8)FETCH);
	if(SF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jns()    /* Opcode 0x79 */
{
	int tmp = (int)((int8)FETCH);
	if(!SF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jp()    /* Opcode 0x7a */
{
	int tmp = (int)((int8)FETCH);
	if(PF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jnp()    /* Opcode 0x7b */
{
	int tmp = (int)((int8)FETCH);
	if(!PF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jl()    /* Opcode 0x7c */
{
	int tmp = (int)((int8)FETCH);
	if((SF!= OF) && !ZF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jnl()    /* Opcode 0x7d */
{
	int tmp = (int)((int8)FETCH);
	if(ZF || (SF == OF)) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jle()    /* Opcode 0x7e */
{
	int tmp = (int)((int8)FETCH);
	if(ZF || (SF!= OF)) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_jnle()    /* Opcode 0x7f */
{
	int tmp = (int)((int8)FETCH);
	if((SF == OF) && !ZF) {
		pc += tmp;
		icount -= timing.jcc_t;
	} else {
		icount -= timing.jcc_nt;
	}
}

inline void I86::_80pre()    /* Opcode 0x80 */
{
	unsigned ModRM = FETCHOP;
	unsigned dst = GetRMByte(ModRM);
	unsigned src = FETCH;
	
	switch((ModRM >> 3) & 7) {
	case 0:	/* ADD eb, d8 */
		ADDB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 1:	/* OR eb, d8 */
		ORB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 2:	/* ADC eb, d8 */
		src += CF;
		ADDB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 3:	/* SBB eb, b8 */
		src += CF;
		SUBB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 4:	/* AND eb, d8 */
		ANDB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 5:	/* SUB eb, d8 */
		SUBB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 6:	/* XOR eb, d8 */
		XORB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 7:	/* CMP eb, d8 */
		SUBB(dst, src);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8_ro;
		break;
	default:
		__assume(0);
	}
}

inline void I86::_81pre()    /* Opcode 0x81 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMWord(ModRM);
	unsigned src = FETCH;
	src += (FETCH << 8);
	
	switch((ModRM >> 3) & 7) {
	case 0:	/* ADD ew, d16 */
		ADDW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 1:	/* OR ew, d16 */
		ORW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 2:	/* ADC ew, d16 */
		src += CF;
		ADDW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 3:	/* SBB ew, d16 */
		src += CF;
		SUBW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 4:	/* AND ew, d16 */
		ANDW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 5:	/* SUB ew, d16 */
		SUBW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 6:	/* XOR ew, d16 */
		XORW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16;
		break;
	case 7:	/* CMP ew, d16 */
		SUBW(dst, src);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16_ro;
		break;
	default:
		__assume(0);
	}
}

inline void I86::_82pre()    /* Opcode 0x82 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMByte(ModRM);
	unsigned src = FETCH;
	
	switch((ModRM >> 3) & 7) {
	case 0:	/* ADD eb, d8 */
		ADDB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 1:	/* OR eb, d8 */
		ORB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 2:	/* ADC eb, d8 */
		src += CF;
		ADDB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 3:	/* SBB eb, d8 */
		src += CF;
		SUBB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 4:	/* AND eb, d8 */
		ANDB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 5:	/* SUB eb, d8 */
		SUBB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 6:	/* XOR eb, d8 */
		XORB(dst, src);
		PutbackRMByte(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8;
		break;
	case 7:	/* CMP eb, d8 */
		SUBB(dst, src);
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8_ro;
		break;
	default:
		__assume(0);
	}
}

inline void I86::_83pre()    /* Opcode 0x83 */
{
	unsigned ModRM = FETCH;
	unsigned dst = GetRMWord(ModRM);
	unsigned src = (uint16)((int16)((int8)FETCH));
	
	switch((ModRM >> 3) & 7) {
	case 0:	/* ADD ew, d16 */
		ADDW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 1:	/* OR ew, d16 */
		ORW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 2:	/* ADC ew, d16 */
		src += CF;
		ADDW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 3:	/* SBB ew, d16 */
		src += CF;
		SUBW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 4:	/* AND ew, d16 */
		ANDW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 5:	/* SUB ew, d16 */
		SUBW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 6:	/* XOR ew, d16 */
		XORW(dst, src);
		PutbackRMWord(ModRM, dst);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8;
		break;
	case 7:	/* CMP ew, d16 */
		SUBW(dst, src);
		icount -= (ModRM >= 0xc0) ? timing.alu_r16i8 : timing.alu_m16i8_ro;
		break;
	default:
		__assume(0);
	}
}

inline void I86::_test_br8()    /* Opcode 0x84 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr8 : timing.alu_rm8;
	ANDB(dst, src);
}

inline void I86::_test_wr16()    /* Opcode 0x85 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.alu_rr16 : timing.alu_rm16;
	ANDW(dst, src);
}

inline void I86::_xchg_br8()    /* Opcode 0x86 */
{
	DEF_br8(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.xchg_rr8 : timing.xchg_rm8;
	RegByte(ModRM) = dst;
	PutbackRMByte(ModRM, src);
}

inline void I86::_xchg_wr16()    /* Opcode 0x87 */
{
	DEF_wr16(dst, src);
	icount -= (ModRM >= 0xc0) ? timing.xchg_rr16 : timing.xchg_rm16;
	RegWord(ModRM) = dst;
	PutbackRMWord(ModRM, src);
}

inline void I86::_mov_br8()    /* Opcode 0x88 */
{
	unsigned ModRM = FETCH;
	uint8 src = RegByte(ModRM);
	icount -= (ModRM >= 0xc0) ? timing.mov_rr8 : timing.mov_mr8;
	PutRMByte(ModRM, src);
}

inline void I86::_mov_wr16()    /* Opcode 0x89 */
{
	unsigned ModRM = FETCH;
	uint16 src = RegWord(ModRM);
	icount -= (ModRM >= 0xc0) ? timing.mov_rr16 : timing.mov_mr16;
	PutRMWord(ModRM, src);
}

inline void I86::_mov_r8b()    /* Opcode 0x8a */
{
	unsigned ModRM = FETCH;
	uint8 src = GetRMByte(ModRM);
	icount -= (ModRM >= 0xc0) ? timing.mov_rr8 : timing.mov_rm8;
	RegByte(ModRM) = src;
}

inline void I86::_mov_r16w()    /* Opcode 0x8b */
{
	unsigned ModRM = FETCH;
	uint16 src = GetRMWord(ModRM);
	icount -= (ModRM >= 0xc0) ? timing.mov_rr8 : timing.mov_rm16;
	RegWord(ModRM) = src;
}

inline void I86::_mov_wsreg()    /* Opcode 0x8c */
{
	unsigned ModRM = FETCH;
	icount -= (ModRM >= 0xc0) ? timing.mov_rs : timing.mov_ms;
	if(ModRM & 0x20) {
		return;	/* 1xx is invalid */
	}
	PutRMWord(ModRM, sregs[(ModRM & 0x38) >> 3]);
}

inline void I86::_lea()    /* Opcode 0x8d */
{
	unsigned ModRM = FETCH;
	icount -= timing.lea;
	GetEA(ModRM);
	RegWord(ModRM) = eo;	/* effective offset (no segment part) */
}

inline void I86::_mov_sregw()    /* Opcode 0x8e */
{
	unsigned ModRM = FETCH;
	uint16 src = GetRMWord(ModRM);
	
	icount -= (ModRM >= 0xc0) ? timing.mov_sr : timing.mov_sm;
	switch((ModRM >> 3) & 7) {
	case 0:  /* mov es, ew */
		sregs[ES] = src;
		base[ES] = SegBase(ES);
		break;
	case 1:  /* mov cs, ew */
		break;  /* doesn't do a jump far */
	case 2:  /* mov ss, ew */
		sregs[SS] = src;
		base[SS] = SegBase(SS); /* no interrupt allowed before next instr */
		instruction(FETCHOP);
		break;
	case 3:  /* mov ds, ew */
		sregs[DS] = src;
		base[DS] = SegBase(DS);
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		break;
	default:
		__assume(0);
	}
}

inline void I86::_popw()    /* Opcode 0x8f */
{
	unsigned ModRM = FETCH;
	uint16 tmp;
	POP(tmp);
	icount -= (ModRM >= 0xc0) ? timing.pop_r16 : timing.pop_m16;
	PutRMWord(ModRM, tmp);
}

#define XchgAXReg(Reg) { \
	uint16 tmp; \
	tmp = regs.w[Reg]; \
	regs.w[Reg] = regs.w[AX]; \
	regs.w[AX] = tmp; \
	icount -= timing.xchg_ar16; \
}

inline void I86::_nop()    /* Opcode 0x90 */
{
	/* this is XchgAXReg(AX); */
	icount -= timing.nop;
}

inline void I86::_xchg_axcx()    /* Opcode 0x91 */
{
	XchgAXReg(CX);
}

inline void I86::_xchg_axdx()    /* Opcode 0x92 */
{
	XchgAXReg(DX);
}

inline void I86::_xchg_axbx()    /* Opcode 0x93 */
{
	XchgAXReg(BX);
}

inline void I86::_xchg_axsp()    /* Opcode 0x94 */
{
	XchgAXReg(SP);
}

inline void I86::_xchg_axbp()    /* Opcode 0x95 */
{
	XchgAXReg(BP);
}

inline void I86::_xchg_axsi()    /* Opcode 0x96 */
{
	XchgAXReg(SI);
}

inline void I86::_xchg_axdi()    /* Opcode 0x97 */
{
	XchgAXReg(DI);
}

inline void I86::_cbw()    /* Opcode 0x98 */
{
	icount -= timing.cbw;
	regs.b[AH] = (regs.b[AL] & 0x80) ? 0xff : 0;
}

inline void I86::_cwd()    /* Opcode 0x99 */
{
	icount -= timing.cwd;
	regs.w[DX] = (regs.b[AH] & 0x80) ? 0xffff : 0;
}

inline void I86::_call_far()    /* Opcode 0x9a */
{
	unsigned tmp, tmp2;
	uint16 ip;
	
	tmp = FETCH;
	tmp += FETCH << 8;
	
	tmp2 = FETCH;
	tmp2 += FETCH << 8;
	
	ip = pc - base[CS];
	PUSH(sregs[CS]);
	PUSH(ip);
	sregs[CS] = (uint16)tmp2;
	base[CS] = SegBase(CS);
	pc = (base[CS] + (uint16)tmp) & AMASK;
#ifdef I86_BIOS_CALL
	if(d_bios && d_bios->bios_call(pc, regs.w, sregs, &ZeroVal, &CarryVal)) {
		/* bios call */
		_retf();
	}
#endif
	icount -= timing.call_far;
}

inline void I86::_wait()    /* Opcode 0x9b */
{
	if(test_state) {
		pc--;
	}
	icount -= timing.wait;
}

inline void I86::_pushf()    /* Opcode 0x9c */
{
	unsigned tmp;
	icount -= timing.pushf;
	
	tmp = CompressFlags();
	PUSH(tmp | 0xf000);
}

inline void I86::_popf()    /* Opcode 0x9d */
{
	unsigned tmp;
	POP(tmp);
	icount -= timing.popf;
	ExpandFlags(tmp);
	
	if(TF) {
		trap();
	}
	
	/* if the IF is set, and an interrupt is pending, signal an interrupt */
	if(IF && (int_state & INT_REQ_BIT)) {
		interrupt(-1);
	}
}

inline void I86::_sahf()    /* Opcode 0x9e */
{
	unsigned tmp = (CompressFlags() & 0xff00) | (regs.b[AH] & 0xd5);
	icount -= timing.sahf;
	ExpandFlags(tmp);
}

inline void I86::_lahf()    /* Opcode 0x9f */
{
	regs.b[AH] = CompressFlags() & 0xff;
	icount -= timing.lahf;
}

inline void I86::_mov_aldisp()    /* Opcode 0xa0 */
{
	unsigned addr;
	
	addr = FETCH;
	addr += FETCH << 8;
	
	icount -= timing.mov_am8;
	regs.b[AL] = GetMemB(DS, addr);
}

inline void I86::_mov_axdisp()    /* Opcode 0xa1 */
{
	unsigned addr;
	
	addr = FETCH;
	addr += FETCH << 8;
	
	icount -= timing.mov_am16;
	regs.w[AX] = GetMemW(DS, addr);
}

inline void I86::_mov_dispal()    /* Opcode 0xa2 */
{
	unsigned addr;
	
	addr = FETCH;
	addr += FETCH << 8;
	
	icount -= timing.mov_ma8;
	PutMemB(DS, addr, regs.b[AL]);
}

inline void I86::_mov_dispax()    /* Opcode 0xa3 */
{
	unsigned addr;
	
	addr = FETCH;
	addr += FETCH << 8;
	
	icount -= timing.mov_ma16;
	PutMemW(DS, addr, regs.w[AX]);
}

inline void I86::_movsb()    /* Opcode 0xa4 */
{
	uint8 tmp = GetMemB(DS, regs.w[SI]);
	PutMemB(ES, regs.w[DI], tmp);
	regs.w[DI] += DirVal;
	regs.w[SI] += DirVal;
	icount -= timing.movs8;
}

inline void I86::_movsw()    /* Opcode 0xa5 */
{
	uint16 tmp = GetMemW(DS, regs.w[SI]);
	PutMemW(ES, regs.w[DI], tmp);
	regs.w[DI] += 2 * DirVal;
	regs.w[SI] += 2 * DirVal;
	icount -= timing.movs16;
}

inline void I86::_cmpsb()    /* Opcode 0xa6 */
{
	unsigned dst = GetMemB(ES, regs.w[DI]);
	unsigned src = GetMemB(DS, regs.w[SI]);
	SUBB(src, dst); /* opposite of the usual convention */
	regs.w[DI] += DirVal;
	regs.w[SI] += DirVal;
	icount -= timing.cmps8;
}

inline void I86::_cmpsw()    /* Opcode 0xa7 */
{
	unsigned dst = GetMemW(ES, regs.w[DI]);
	unsigned src = GetMemW(DS, regs.w[SI]);
	SUBW(src, dst); /* opposite of the usual convention */
	regs.w[DI] += 2 * DirVal;
	regs.w[SI] += 2 * DirVal;
	icount -= timing.cmps16;
}

inline void I86::_test_ald8()    /* Opcode 0xa8 */
{
	DEF_ald8(dst, src);
	icount -= timing.alu_ri8;
	ANDB(dst, src);
}

inline void I86::_test_axd16()    /* Opcode 0xa9 */
{
	DEF_axd16(dst, src);
	icount -= timing.alu_ri16;
	ANDW(dst, src);
}

inline void I86::_stosb()    /* Opcode 0xaa */
{
	PutMemB(ES, regs.w[DI], regs.b[AL]);
	regs.w[DI] += DirVal;
	icount -= timing.stos8;
}

inline void I86::_stosw()    /* Opcode 0xab */
{
	PutMemW(ES, regs.w[DI], regs.w[AX]);
	regs.w[DI] += 2 * DirVal;
	icount -= timing.stos16;
}

inline void I86::_lodsb()    /* Opcode 0xac */
{
	regs.b[AL] = GetMemB(DS, regs.w[SI]);
	regs.w[SI] += DirVal;
	icount -= timing.lods8;
}

inline void I86::_lodsw()    /* Opcode 0xad */
{
	regs.w[AX] = GetMemW(DS, regs.w[SI]);
	regs.w[SI] += 2 * DirVal;
	icount -= timing.lods16;
}

inline void I86::_scasb()    /* Opcode 0xae */
{
	unsigned src = GetMemB(ES, regs.w[DI]);
	unsigned dst = regs.b[AL];
	SUBB(dst, src);
	regs.w[DI] += DirVal;
	icount -= timing.scas8;
}

inline void I86::_scasw()    /* Opcode 0xaf */
{
	unsigned src = GetMemW(ES, regs.w[DI]);
	unsigned dst = regs.w[AX];
	SUBW(dst, src);
	regs.w[DI] += 2 * DirVal;
	icount -= timing.scas16;
}

inline void I86::_mov_ald8()    /* Opcode 0xb0 */
{
	regs.b[AL] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86::_mov_cld8()    /* Opcode 0xb1 */
{
	regs.b[CL] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86::_mov_dld8()    /* Opcode 0xb2 */
{
	regs.b[DL] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86::_mov_bld8()    /* Opcode 0xb3 */
{
	regs.b[BL] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86::_mov_ahd8()    /* Opcode 0xb4 */
{
	regs.b[AH] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86::_mov_chd8()    /* Opcode 0xb5 */
{
	regs.b[CH] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86::_mov_dhd8()    /* Opcode 0xb6 */
{
	regs.b[DH] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86::_mov_bhd8()    /* Opcode 0xb7 */
{
	regs.b[BH] = FETCH;
	icount -= timing.mov_ri8;
}

inline void I86::_mov_axd16()    /* Opcode 0xb8 */
{
	regs.b[AL] = FETCH;
	regs.b[AH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86::_mov_cxd16()    /* Opcode 0xb9 */
{
	regs.b[CL] = FETCH;
	regs.b[CH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86::_mov_dxd16()    /* Opcode 0xba */
{
	regs.b[DL] = FETCH;
	regs.b[DH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86::_mov_bxd16()    /* Opcode 0xbb */
{
	regs.b[BL] = FETCH;
	regs.b[BH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86::_mov_spd16()    /* Opcode 0xbc */
{
	regs.b[SPL] = FETCH;
	regs.b[SPH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86::_mov_bpd16()    /* Opcode 0xbd */
{
	regs.b[BPL] = FETCH;
	regs.b[BPH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86::_mov_sid16()    /* Opcode 0xbe */
{
	regs.b[SIL] = FETCH;
	regs.b[SIH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86::_mov_did16()    /* Opcode 0xbf */
{
	regs.b[DIL] = FETCH;
	regs.b[DIH] = FETCH;
	icount -= timing.mov_ri16;
}

inline void I86::_rotshft_bd8()    /* Opcode 0xc0 */
{
	unsigned ModRM = FETCH;
	unsigned count = FETCH;
	
	rotate_shift_byte(ModRM, count);
}

inline void I86::_rotshft_wd8()    /* Opcode 0xc1 */
{
	unsigned ModRM = FETCH;
	unsigned count = FETCH;
	
	rotate_shift_word(ModRM, count);
}

inline void I86::_ret_d16()    /* Opcode 0xc2 */
{
	unsigned count = FETCH;
	count += FETCH << 8;
	POP(pc);
	pc = (pc + base[CS]) & AMASK;
	regs.w[SP] += count;
	icount -= timing.ret_near_imm;
}

inline void I86::_ret()    /* Opcode 0xc3 */
{
	POP(pc);
	pc = (pc + base[CS]) & AMASK;
	icount -= timing.ret_near;
}

inline void I86::_les_dw()    /* Opcode 0xc4 */
{
	unsigned ModRM = FETCH;
	uint16 tmp = GetRMWord(ModRM);
	RegWord(ModRM) = tmp;
	sregs[ES] = GetNextRMWord;
	base[ES] = SegBase(ES);
	icount -= timing.load_ptr;
}

inline void I86::_lds_dw()    /* Opcode 0xc5 */
{
	unsigned ModRM = FETCH;
	uint16 tmp = GetRMWord(ModRM);
	RegWord(ModRM) = tmp;
	sregs[DS] = GetNextRMWord;
	base[DS] = SegBase(DS);
	icount -= timing.load_ptr;
}

inline void I86::_mov_bd8()    /* Opcode 0xc6 */
{
	unsigned ModRM = FETCH;
	icount -= (ModRM >= 0xc0) ? timing.mov_ri8 : timing.mov_mi8;
	PutImmRMByte(ModRM);
}

inline void I86::_mov_wd16()    /* Opcode 0xc7 */
{
	unsigned ModRM = FETCH;
	icount -= (ModRM >= 0xc0) ? timing.mov_ri16 : timing.mov_mi16;
	PutImmRMWord(ModRM);
}

inline void I86::_enter()    /* Opcode 0xc8 */
{
	unsigned nb = FETCH;
	unsigned i, level;
	
	nb += FETCH << 8;
	level = FETCH;
	icount -= (level == 0) ? timing.enter0 : (level == 1) ? timing.enter1 : timing.enter_base + level * timing.enter_count;
	PUSH(regs.w[BP]);
	regs.w[BP] = regs.w[SP];
	regs.w[SP] -= nb;
	for(i = 1; i < level; i++) {
		PUSH(GetMemW(SS, regs.w[BP] - i * 2));
	}
	if(level) {
		PUSH(regs.w[BP]);
	}
}

inline void I86::_leav()    /* Opcode 0xc9 */
{
	icount -= timing.leave;
	regs.w[SP] = regs.w[BP];
	POP(regs.w[BP]);
}

inline void I86::_retf_d16()    /* Opcode 0xca */
{
	unsigned count = FETCH;
	count += FETCH << 8;
	POP(pc);
	POP(sregs[CS]);
	base[CS] = SegBase(CS);
	pc = (pc + base[CS]) & AMASK;
	regs.w[SP] += count;
	icount -= timing.ret_far_imm;
}

inline void I86::_retf()    /* Opcode 0xcb */
{
	POP(pc);
	POP(sregs[CS]);
	base[CS] = SegBase(CS);
	pc = (pc + base[CS]) & AMASK;
	icount -= timing.ret_far;
}

inline void I86::_int3()    /* Opcode 0xcc */
{
	icount -= timing.int3;
	interrupt(3);
}

inline void I86::_int()    /* Opcode 0xcd */
{
	unsigned int_num = FETCH;
	icount -= timing.int_imm;
#ifdef I86_BIOS_CALL
	if(d_bios && d_bios->bios_int(int_num, regs.w, sregs, &ZeroVal, &CarryVal)) {
		/* bios call */
		return;
	}
#endif
	interrupt(int_num);
}

inline void I86::_into()    /* Opcode 0xce */
{
	if(OF) {
		icount -= timing.into_t;
		interrupt(OVERFLOW_TRAP);
	} else {
		icount -= timing.into_nt;
	}
}

inline void I86::_iret()    /* Opcode 0xcf */
{
	icount -= timing.iret;
	POP(pc);
	POP(sregs[CS]);
	base[CS] = SegBase(CS);
	pc = (pc + base[CS]) & AMASK;
	_popf();
	
	/* if the IF is set, and an interrupt is pending, signal an interrupt */
	if(IF && (int_state & INT_REQ_BIT)) {
		interrupt(-1);
	}
}

inline void I86::_rotshft_b()    /* Opcode 0xd0 */
{
	rotate_shift_byte(FETCHOP, 1);
}

inline void I86::_rotshft_w()    /* Opcode 0xd1 */
{
	rotate_shift_word(FETCHOP, 1);
}

inline void I86::_rotshft_bcl()    /* Opcode 0xd2 */
{
	rotate_shift_byte(FETCHOP, regs.b[CL]);
}

inline void I86::_rotshft_wcl()    /* Opcode 0xd3 */
{
	rotate_shift_word(FETCHOP, regs.b[CL]);
}

/* OB: Opcode works on NEC V-Series but not the Variants        */
/*     one could specify any byte value as operand but the NECs */
/*     always substitute 0x0a.                                  */
inline void I86::_aam()    /* Opcode 0xd4 */
{
	unsigned mult = FETCH;
	icount -= timing.aam;
	if(mult == 0) {
		interrupt(DIVIDE_FAULT);
	} else {
		regs.b[AH] = regs.b[AL] / mult;
		regs.b[AL] %= mult;
		SetSZPF_Word(regs.w[AX]);
	}
}

inline void I86::_aad()    /* Opcode 0xd5 */
{
	unsigned mult = FETCH;
	icount -= timing.aad;
	regs.b[AL] = regs.b[AH] * mult + regs.b[AL];
	regs.b[AH] = 0;
	SetZF(regs.b[AL]);
	SetPF(regs.b[AL]);
	SignVal = 0;
}

inline void I86::_setalc()    /* Opcode 0xd6 */
{
	regs.b[AL] = (CF) ? 0xff : 0x00;
	icount -= 3;
}

inline void I86::_xlat()    /* Opcode 0xd7 */
{
	unsigned dest = regs.w[BX] + regs.b[AL];
	icount -= timing.xlat;
	regs.b[AL] = GetMemB(DS, dest);
}

inline void I86::_escape()    /* Opcodes 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde and 0xdf */
{
	unsigned ModRM = FETCH;
	icount -= timing.nop;
	GetRMByte(ModRM);
}

inline void I86::_loopne()    /* Opcode 0xe0 */
{
	int disp = (int)((int8)FETCH);
	unsigned tmp = regs.w[CX] - 1;
	regs.w[CX] = tmp;
	if(!ZF && tmp) {
		icount -= timing.loop_t;
		pc += disp;
	} else {
		icount -= timing.loop_nt;
	}
}

inline void I86::_loope()    /* Opcode 0xe1 */
{
	int disp = (int)((int8)FETCH);
	unsigned tmp = regs.w[CX] - 1;
	regs.w[CX] = tmp;
	if(ZF && tmp) {
		icount -= timing.loope_t;
		pc += disp;
	} else {
		icount -= timing.loope_nt;
	}
}

inline void I86::_loop()    /* Opcode 0xe2 */
{
	int disp = (int)((int8)FETCH);
	unsigned tmp = regs.w[CX] - 1;
	regs.w[CX] = tmp;
	if(tmp) {
		icount -= timing.loop_t;
		pc += disp;
	} else {
		icount -= timing.loop_nt;
	}
}

inline void I86::_jcxz()    /* Opcode 0xe3 */
{
	int disp = (int)((int8)FETCH);
	if(regs.w[CX] == 0) {
		icount -= timing.jcxz_t;
		pc += disp;
	} else {
		icount -= timing.jcxz_nt;
	}
}

inline void I86::_inal()    /* Opcode 0xe4 */
{
	unsigned port = FETCH;
	icount -= timing.in_imm8;
	regs.b[AL] = read_port_byte(port);
}

inline void I86::_inax()    /* Opcode 0xe5 */
{
	unsigned port = FETCH;
	icount -= timing.in_imm16;
	regs.w[AX] = read_port_word(port);
}

inline void I86::_outal()    /* Opcode 0xe6 */
{
	unsigned port = FETCH;
	icount -= timing.out_imm8;
	write_port_byte(port, regs.b[AL]);
}

inline void I86::_outax()    /* Opcode 0xe7 */
{
	unsigned port = FETCH;
	icount -= timing.out_imm16;
	write_port_word(port, regs.w[AX]);
}

inline void I86::_call_d16()    /* Opcode 0xe8 */
{
	uint16 ip, tmp;
	
	FETCHWORD(tmp);
	ip = pc - base[CS];
	PUSH(ip);
	ip += tmp;
	pc = (ip + base[CS]) & AMASK;
#ifdef I86_BIOS_CALL
	if(d_bios && d_bios->bios_call(pc, regs.w, sregs, &ZeroVal, &CarryVal)) {
		/* bios call */
		_ret();
	}
#endif
	icount -= timing.call_near;
}

inline void I86::_jmp_d16()    /* Opcode 0xe9 */
{
	uint16 ip, tmp;
	
	FETCHWORD(tmp);
	ip = pc - base[CS] + tmp;
	pc = (ip + base[CS]) & AMASK;
	icount -= timing.jmp_near;
}

inline void I86::_jmp_far()    /* Opcode 0xea */
{
	unsigned tmp, tmp1;
	
	tmp = FETCH;
	tmp += FETCH << 8;
	
	tmp1 = FETCH;
	tmp1 += FETCH << 8;
	
	sregs[CS] = (uint16)tmp1;
	base[CS] = SegBase(CS);
	pc = (base[CS] + tmp) & AMASK;
	icount -= timing.jmp_far;
}

inline void I86::_jmp_d8()    /* Opcode 0xeb */
{
	int tmp = (int)((int8)FETCH);
	pc += tmp;
	icount -= timing.jmp_short;
}

inline void I86::_inaldx()    /* Opcode 0xec */
{
	icount -= timing.in_dx8;
	regs.b[AL] = read_port_byte(regs.w[DX]);
}

inline void I86::_inaxdx()    /* Opcode 0xed */
{
	unsigned port = regs.w[DX];
	icount -= timing.in_dx16;
	regs.w[AX] = read_port_word(port);
}

inline void I86::_outdxal()    /* Opcode 0xee */
{
	icount -= timing.out_dx8;
	write_port_byte(regs.w[DX], regs.b[AL]);
}

inline void I86::_outdxax()    /* Opcode 0xef */
{
	unsigned port = regs.w[DX];
	icount -= timing.out_dx16;
	write_port_word(port, regs.w[AX]);
}

/* I think thats not a V20 instruction...*/
inline void I86::_lock()    /* Opcode 0xf0 */
{
	icount -= timing.nop;
	instruction(FETCHOP);  /* un-interruptible */
}

inline void I86::_rep(int flagval)
{
	/* Handles rep- and repnz- prefixes. flagval is the value of ZF for the
	   loop  to continue for CMPS and SCAS instructions. */
	
	unsigned next = FETCHOP;
	unsigned count = regs.w[CX];
	
	switch(next) {
	case 0x26:  /* ES: */
		seg_prefix = true;
		prefix_seg = ES;
		icount -= timing.override;
		_rep(flagval);
		break;
	case 0x2e:  /* CS: */
		seg_prefix = true;
		prefix_seg = CS;
		icount -= timing.override;
		_rep(flagval);
		break;
	case 0x36:  /* SS: */
		seg_prefix = true;
		prefix_seg = SS;
		icount -= timing.override;
		_rep(flagval);
		break;
	case 0x3e:  /* DS: */
		seg_prefix = true;
		prefix_seg = DS;
		icount -= timing.override;
		_rep(flagval);
		break;
#ifndef HAS_I86
	case 0x6c:  /* REP INSB */
		icount -= timing.rep_ins8_base;
		for(; count > 0; count--) {
			PutMemB(ES, regs.w[DI], read_port_byte(regs.w[DX]));
			regs.w[DI] += DirVal;
			icount -= timing.rep_ins8_count;
		}
		regs.w[CX] = count;
		break;
	case 0x6d:  /* REP INSW */
		icount -= timing.rep_ins16_base;
		for(; count > 0; count--) {
			PutMemW(ES, regs.w[DI], read_port_word(regs.w[DX]));
			regs.w[DI] += 2 * DirVal;
			icount -= timing.rep_ins16_count;
		}
		regs.w[CX] = count;
		break;
	case 0x6e:  /* REP OUTSB */
		icount -= timing.rep_outs8_base;
		for(; count > 0; count--) {
			write_port_byte(regs.w[DX], GetMemB(DS, regs.w[SI]));
			regs.w[SI] += DirVal; /* GOL 11/27/01 */
			icount -= timing.rep_outs8_count;
		}
		regs.w[CX] = count;
		break;
	case 0x6f:  /* REP OUTSW */
		icount -= timing.rep_outs16_base;
		for(; count > 0; count--) {
			write_port_word(regs.w[DX], GetMemW(DS, regs.w[SI]));
			regs.w[SI] += 2 * DirVal; /* GOL 11/27/01 */
			icount -= timing.rep_outs16_count;
		}
		regs.w[CX] = count;
		break;
#endif
	case 0xa4:	/* REP MOVSB */
		icount -= timing.rep_movs8_base;
		for(; count > 0; count--) {
			uint8 tmp;
			tmp = GetMemB(DS, regs.w[SI]);
			PutMemB(ES, regs.w[DI], tmp);
			regs.w[DI] += DirVal;
			regs.w[SI] += DirVal;
			icount -= timing.rep_movs8_count;
		}
		regs.w[CX] = count;
		break;
	case 0xa5:  /* REP MOVSW */
		icount -= timing.rep_movs16_base;
		for(; count > 0; count--) {
			uint16 tmp;
			tmp = GetMemW(DS, regs.w[SI]);
			PutMemW(ES, regs.w[DI], tmp);
			regs.w[DI] += 2 * DirVal;
			regs.w[SI] += 2 * DirVal;
			icount -= timing.rep_movs16_count;
		}
		regs.w[CX] = count;
		break;
	case 0xa6:  /* REP(N)E CMPSB */
		icount -= timing.rep_cmps8_base;
		for(ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--) {
			unsigned dst, src;
			dst = GetMemB(ES, regs.w[DI]);
			src = GetMemB(DS, regs.w[SI]);
			SUBB(src, dst); /* opposite of the usual convention */
			regs.w[DI] += DirVal;
			regs.w[SI] += DirVal;
			icount -= timing.rep_cmps8_count;
		}
		regs.w[CX] = count;
		break;
	case 0xa7:  /* REP(N)E CMPSW */
		icount -= timing.rep_cmps16_base;
		for(ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--) {
			unsigned dst, src;
			dst = GetMemW(ES, regs.w[DI]);
			src = GetMemW(DS, regs.w[SI]);
			SUBW(src, dst); /* opposite of the usual convention */
			regs.w[DI] += 2 * DirVal;
			regs.w[SI] += 2 * DirVal;
			icount -= timing.rep_cmps16_count;
		}
		regs.w[CX] = count;
		break;
	case 0xaa:  /* REP STOSB */
		icount -= timing.rep_stos8_base;
		for(; count > 0; count--) {
			PutMemB(ES, regs.w[DI], regs.b[AL]);
			regs.w[DI] += DirVal;
			icount -= timing.rep_stos8_count;
		}
		regs.w[CX] = count;
		break;
	case 0xab:  /* REP STOSW */
		icount -= timing.rep_stos16_base;
		for(; count > 0; count--) {
			PutMemW(ES, regs.w[DI], regs.w[AX]);
			regs.w[DI] += 2 * DirVal;
			icount -= timing.rep_stos16_count;
		}
		regs.w[CX] = count;
		break;
	case 0xac:  /* REP LODSB */
		icount -= timing.rep_lods8_base;
		for(; count > 0; count--) {
			regs.b[AL] = GetMemB(DS, regs.w[SI]);
			regs.w[SI] += DirVal;
			icount -= timing.rep_lods8_count;
		}
		regs.w[CX] = count;
		break;
	case 0xad:  /* REP LODSW */
		icount -= timing.rep_lods16_base;
		for(; count > 0; count--) {
			regs.w[AX] = GetMemW(DS, regs.w[SI]);
			regs.w[SI] += 2 * DirVal;
			icount -= timing.rep_lods16_count;
		}
		regs.w[CX] = count;
		break;
	case 0xae:  /* REP(N)E SCASB */
		icount -= timing.rep_scas8_base;
		for(ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--) {
			unsigned src, dst;
			src = GetMemB(ES, regs.w[DI]);
			dst = regs.b[AL];
			SUBB(dst, src);
			regs.w[DI] += DirVal;
			icount -= timing.rep_scas8_count;
		}
		regs.w[CX] = count;
		break;
	case 0xaf:  /* REP(N)E SCASW */
		icount -= timing.rep_scas16_base;
		for(ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--) {
			unsigned src, dst;
			src = GetMemW(ES, regs.w[DI]);
			dst = regs.w[AX];
			SUBW(dst, src);
			regs.w[DI] += 2 * DirVal;
			icount -= timing.rep_scas16_count;
		}
		regs.w[CX] = count;
		break;
	default:
		instruction(next);
	}
}

inline void I86::_repne()    /* Opcode 0xf2 */
{
	_rep(0);
}

inline void I86::_repe()    /* Opcode 0xf3 */
{
	_rep(1);
}

inline void I86::_hlt()    /* Opcode 0xf4 */
{
	pc--;
	halted = true;
	icount -= 2;
}

inline void I86::_cmc()    /* Opcode 0xf5 */
{
	icount -= timing.flag_ops;
	CarryVal = !CF;
}

inline void I86::_f6pre()    /* Opcode 0xf6 */
{
	unsigned ModRM = FETCH;
	unsigned tmp = (unsigned)GetRMByte(ModRM);
	unsigned tmp2;
	
	switch((ModRM >> 3) & 7) {
	case 0:  /* TEST Eb, data8 */
	case 1:  /* ??? */
		icount -= (ModRM >= 0xc0) ? timing.alu_ri8 : timing.alu_mi8_ro;
		tmp &= FETCH;
		
		CarryVal = OverVal = AuxVal = 0;
		SetSZPF_Byte(tmp);
		break;
		
	case 2:  /* NOT Eb */
		icount -= (ModRM >= 0xc0) ? timing.negnot_r8 : timing.negnot_m8;
		PutbackRMByte(ModRM, ~tmp);
		break;
		
	case 3:  /* NEG Eb */
		icount -= (ModRM >= 0xc0) ? timing.negnot_r8 : timing.negnot_m8;
		tmp2 = 0;
		SUBB(tmp2, tmp);
		PutbackRMByte(ModRM, tmp2);
		break;
		
	case 4:  /* MUL AL, Eb */
		icount -= (ModRM >= 0xc0) ? timing.mul_r8 : timing.mul_m8;
		{
			uint16 result;
			
			tmp2 = regs.b[AL];
			
			SetSF((int8)tmp2);
			SetPF(tmp2);
			
			result = (uint16)tmp2 * tmp;
			regs.w[AX] = (uint16)result;
			
			SetZF(regs.w[AX]);
			CarryVal = OverVal = (regs.b[AH] != 0);
		}
		break;
		
	case 5:  /* IMUL AL, Eb */
		icount -= (ModRM >= 0xc0) ? timing.imul_r8 : timing.imul_m8;
		{
			int16 result;
			
			tmp2 = (unsigned)regs.b[AL];
			
			SetSF((int8)tmp2);
			SetPF(tmp2);
			
			result = (int16)((int8)tmp2) * (int16)((int8)tmp);
			regs.w[AX] = (uint16)result;
			
			SetZF(regs.w[AX]);
			CarryVal = OverVal = (result >> 7 != 0) && (result >> 7 != -1);
		}
		break;
		
	case 6:  /* DIV AL, Ew */
		icount -= (ModRM >= 0xc0) ? timing.div_r8 : timing.div_m8;
		{
			uint16 result;
			
			result = regs.w[AX];
			
			if(tmp) {
				if((result / tmp) > 0xff) {
					interrupt(DIVIDE_FAULT);
					break;
				} else {
					regs.b[AH] = result % tmp;
					regs.b[AL] = result / tmp;
				}
			} else {
				interrupt(DIVIDE_FAULT);
				break;
			}
		}
		break;
		
	case 7:  /* IDIV AL, Ew */
		icount -= (ModRM >= 0xc0) ? timing.idiv_r8 : timing.idiv_m8;
		{
			int16 result;
			
			result = regs.w[AX];
			
			if(tmp) {
				tmp2 = result % (int16)((int8)tmp);
				
				if((result /= (int16)((int8)tmp)) > 0xff) {
					interrupt(DIVIDE_FAULT);
					break;
				} else {
					regs.b[AL] = (uint8)result;
					regs.b[AH] = tmp2;
				}
			} else {
				interrupt(DIVIDE_FAULT);
				break;
			}
		}
		break;
		
	default:
		__assume(0);
	}
}

inline void I86::_f7pre()    /* Opcode 0xf7 */
{
	unsigned ModRM = FETCH;
	unsigned tmp = GetRMWord(ModRM);
	unsigned tmp2;
	
	switch((ModRM >> 3) & 7) {
	case 0:  /* TEST Ew, data16 */
	case 1:  /* ??? */
		icount -= (ModRM >= 0xc0) ? timing.alu_ri16 : timing.alu_mi16_ro;
		tmp2 = FETCH;
		tmp2 += FETCH << 8;
		
		tmp &= tmp2;
		
		CarryVal = OverVal = AuxVal = 0;
		SetSZPF_Word(tmp);
		break;
		
	case 2:  /* NOT Ew */
		icount -= (ModRM >= 0xc0) ? timing.negnot_r16 : timing.negnot_m16;
		tmp = ~tmp;
		PutbackRMWord(ModRM, tmp);
		break;
		
	case 3:  /* NEG Ew */
		icount -= (ModRM >= 0xc0) ? timing.negnot_r16 : timing.negnot_m16;
		tmp2 = 0;
		SUBW(tmp2, tmp);
		PutbackRMWord(ModRM, tmp2);
		break;
		
	case 4:  /* MUL AX, Ew */
		icount -= (ModRM >= 0xc0) ? timing.mul_r16 : timing.mul_m16;
		{
			uint32 result;
			tmp2 = regs.w[AX];
			
			SetSF((int16)tmp2);
			SetPF(tmp2);
			
			result = (uint32)tmp2 * tmp;
			regs.w[AX] = (uint16)result;
			result >>= 16;
			regs.w[DX] = result;
			
			SetZF(regs.w[AX] | regs.w[DX]);
			CarryVal = OverVal = (regs.w[DX] != 0);
		}
		break;
		
	case 5:  /* IMUL AX, Ew */
		icount -= (ModRM >= 0xc0) ? timing.imul_r16 : timing.imul_m16;
		{
			int32 result;
			
			tmp2 = regs.w[AX];
			
			SetSF((int16)tmp2);
			SetPF(tmp2);
			
			result = (int32)((int16)tmp2) * (int32)((int16)tmp);
			CarryVal = OverVal = (result >> 15 != 0) && (result >> 15 != -1);
			
			regs.w[AX] = (uint16)result;
			result = (uint16)(result >> 16);
			regs.w[DX] = result;
			
			SetZF(regs.w[AX] | regs.w[DX]);
		}
		break;
		
	case 6:  /* DIV AX, Ew */
		icount -= (ModRM >= 0xc0) ? timing.div_r16 : timing.div_m16;
		{
			uint32 result;
			
			result = (regs.w[DX] << 16) + regs.w[AX];
			
			if(tmp) {
				tmp2 = result % tmp;
				if((result / tmp) > 0xffff) {
					interrupt(DIVIDE_FAULT);
					break;
				} else {
					regs.w[DX] = tmp2;
					result /= tmp;
					regs.w[AX] = result;
				}
			} else {
				interrupt(DIVIDE_FAULT);
				break;
			}
		}
		break;
		
	case 7:  /* IDIV AX, Ew */
		icount -= (ModRM >= 0xc0) ? timing.idiv_r16 : timing.idiv_m16;
		{
			int32 result;
			
			result = (regs.w[DX] << 16) + regs.w[AX];
			
			if(tmp) {
				tmp2 = result % (int32)((int16)tmp);
				if((result /= (int32)((int16)tmp)) > 0xffff) {
					interrupt(DIVIDE_FAULT);
					break;
				} else {
					regs.w[AX] = result;
					regs.w[DX] = tmp2;
				}
			} else {
				interrupt(DIVIDE_FAULT);
				break;
			}
		}
		break;
		
	default:
		__assume(0);
	}
}

inline void I86::_clc()    /* Opcode 0xf8 */
{
	icount -= timing.flag_ops;
	CarryVal = 0;
}

inline void I86::_stc()    /* Opcode 0xf9 */
{
	icount -= timing.flag_ops;
	CarryVal = 1;
}

inline void I86::_cli()    /* Opcode 0xfa */
{
	icount -= timing.flag_ops;
	SetIF(0);
}

inline void I86::_sti()    /* Opcode 0xfb */
{
	icount -= timing.flag_ops;
	SetIF(1);
	instruction(FETCHOP); /* no interrupt before next instruction */

	/* if an interrupt is pending, signal an interrupt */
	if(IF && (int_state & INT_REQ_BIT)) {
		interrupt(-1);
	}
}

inline void I86::_cld()    /* Opcode 0xfc */
{
	icount -= timing.flag_ops;
	SetDF(0);
}

inline void I86::_std()    /* Opcode 0xfd */
{
	icount -= timing.flag_ops;
	SetDF(1);
}

inline void I86::_fepre()    /* Opcode 0xfe */
{
	unsigned ModRM = FETCH;
	unsigned tmp = GetRMByte(ModRM);
	unsigned tmp1;
	
	icount -= (ModRM >= 0xc0) ? timing.incdec_r8 : timing.incdec_m8;
	if((ModRM & 0x38) == 0) {
		/* INC eb */
		tmp1 = tmp + 1;
		SetOFB_Add(tmp1, tmp, 1);
	} else {
		/* DEC eb */
		tmp1 = tmp - 1;
		SetOFB_Sub(tmp1, 1, tmp);
	}
	SetAF(tmp1, tmp, 1);
	SetSZPF_Byte(tmp1);
	PutbackRMByte(ModRM, (uint8)tmp1);
}

inline void I86::_ffpre()    /* Opcode 0xff */
{
	unsigned ModRM = FETCHOP;
	unsigned tmp;
	unsigned tmp1;
	uint16 ip;
	
	switch((ModRM >> 3) & 7) {
	case 0:  /* INC ew */
		icount -= (ModRM >= 0xc0) ? timing.incdec_r16 : timing.incdec_m16;
		tmp = GetRMWord(ModRM);
		tmp1 = tmp + 1;
		SetOFW_Add(tmp1, tmp, 1);
		SetAF(tmp1, tmp, 1);
		SetSZPF_Word(tmp1);
		PutbackRMWord(ModRM, (uint16)tmp1);
		break;
	case 1:  /* DEC ew */
		icount -= (ModRM >= 0xc0) ? timing.incdec_r16 : timing.incdec_m16;
		tmp = GetRMWord(ModRM);
		tmp1 = tmp - 1;
		SetOFW_Sub(tmp1, 1, tmp);
		SetAF(tmp1, tmp, 1);
		SetSZPF_Word(tmp1);
		PutbackRMWord(ModRM, (uint16)tmp1);
		break;
	case 2:  /* CALL ew */
		icount -= (ModRM >= 0xc0) ? timing.call_r16 : timing.call_m16;
		tmp = GetRMWord(ModRM);
		ip = pc - base[CS];
		PUSH(ip);
		pc = (base[CS] + (uint16)tmp) & AMASK;
#ifdef I86_BIOS_CALL
		if(d_bios && d_bios->bios_call(pc, regs.w, sregs, &ZeroVal, &CarryVal)) {
			/* bios call */
			_ret();
		}
#endif
		break;
	case 3:  /* CALL FAR ea */
		icount -= timing.call_m32;
		tmp = sregs[CS];	/* need to skip displacements of ea */
		tmp1 = GetRMWord(ModRM);
		ip = pc - base[CS];
		PUSH(tmp);
		PUSH(ip);
		sregs[CS] = GetNextRMWord;
		base[CS] = SegBase(CS);
		pc = (base[CS] + tmp1) & AMASK;
#ifdef I86_BIOS_CALL
		if(d_bios && d_bios->bios_call(pc, regs.w, sregs, &ZeroVal, &CarryVal)) {
			/* bios call */
			_ret();
		}
#endif
		break;
	case 4:  /* JMP ea */
		icount -= (ModRM >= 0xc0) ? timing.jmp_r16 : timing.jmp_m16;
		ip = GetRMWord(ModRM);
		pc = (base[CS] + ip) & AMASK;
		break;
	case 5:  /* JMP FAR ea */
		icount -= timing.jmp_m32;
		pc = GetRMWord(ModRM);
		sregs[CS] = GetNextRMWord;
		base[CS] = SegBase(CS);
		pc = (pc + base[CS]) & AMASK;
		break;
	case 6:  /* PUSH ea */
		icount -= (ModRM >= 0xc0) ? timing.push_r16 : timing.push_m16;
		tmp = GetRMWord(ModRM);
		PUSH(tmp);
		break;
	case 7:  /* invalid ??? */
		icount -= 10;
		break;
	default:
		__assume(0);
	}
}

inline void I86::_invalid()
{
	/* i8086/i8088 ignore an invalid opcode. */
	/* i80186/i80188 probably also ignore an invalid opcode. */
	icount -= 10;
}

/*
   NEC V-series Disassembler

   Originally Written for i386 by Ville Linde
   Converted to NEC-V by Aaron Giles
*/

enum
{
	PARAM_REG8 = 1,		/* 8-bit register */
	PARAM_REG16,		/* 16-bit register */
	PARAM_REG2_8,		/* 8-bit register */
	PARAM_REG2_16,		/* 16-bit register */
	PARAM_RM8,			/* 8-bit memory or register */
	PARAM_RM16,			/* 16-bit memory or register */
	PARAM_RMPTR8,		/* 8-bit memory or register */
	PARAM_RMPTR16,		/* 16-bit memory or register */
	PARAM_I3,			/* 3-bit immediate */
	PARAM_I4,			/* 4-bit immediate */
	PARAM_I8,			/* 8-bit signed immediate */
	PARAM_I16,			/* 16-bit signed immediate */
	PARAM_UI8,			/* 8-bit unsigned immediate */
	PARAM_IMM,			/* 16-bit immediate */
	PARAM_ADDR,			/* 16:16 address */
	PARAM_REL8,			/* 8-bit PC-relative displacement */
	PARAM_REL16,		/* 16-bit PC-relative displacement */
	PARAM_MEM_OFFS,		/* 16-bit mem offset */
	PARAM_SREG,			/* segment register */
	PARAM_SFREG,		/* V25/V35 special function register */
	PARAM_1,			/* used by shift/rotate instructions */
	PARAM_AL,
	PARAM_CL,
	PARAM_DL,
	PARAM_BL,
	PARAM_AH,
	PARAM_CH,
	PARAM_DH,
	PARAM_BH,
	PARAM_AW,
	PARAM_CW,
	PARAM_DW,
	PARAM_BW,
	PARAM_SP,
	PARAM_BP,
	PARAM_IX,
	PARAM_IY
};

enum
{
	MODRM = 1,
	GROUP,
	FPU,
	TWO_BYTE,
	PREFIX,
	SEG_PS,
	SEG_DS0,
	SEG_DS1,
	SEG_SS
};

struct I386_OPCODE {
	char mnemonic[32];
	UINT32 flags;
	UINT32 param1;
	UINT32 param2;
	UINT32 param3;
	offs_t dasm_flags;
};

struct GROUP_OP {
	char mnemonic[32];
	const I386_OPCODE *opcode;
};

static const UINT8 *opcode_ptr;
static const UINT8 *opcode_ptr_base;

static const I386_OPCODE necv_opcode_table1[256] =
{
	// 0x00
	{"add",				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{"add",				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{"add",				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{"add",				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"add",				0,				PARAM_AL,			PARAM_UI8,			0				},
	{"add",				0,				PARAM_AW,			PARAM_IMM,			0				},
	{"push    ds1",		0,				0,					0,					0				},
	{"pop     ds1",		0,				0,					0,					0				},
	{"or",				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{"or",				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{"or",				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{"or",				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"or",				0,				PARAM_AL,			PARAM_UI8,			0				},
	{"or",				0,				PARAM_AW,			PARAM_IMM,			0				},
	{"push    ps",		0,				0,					0,					0				},
	{"two_byte",		TWO_BYTE,		0,					0,					0				},
	// 0x10
	{"addc",			MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{"addc",			MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{"addc",			MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{"addc",			MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"addc",			0,				PARAM_AL,			PARAM_UI8,			0				},
	{"addc",			0,				PARAM_AW,			PARAM_IMM,			0				},
	{"push    ss",		0,				0,					0,					0				},
	{"pop     ss",		0,				0,					0,					0				},
	{"subc",			MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{"subc",			MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{"subc",			MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{"subc",			MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"subc",			0,				PARAM_AL,			PARAM_UI8,			0				},
	{"subc",			0,				PARAM_AW,			PARAM_IMM,			0				},
	{"push    ds0",		0,				0,					0,					0				},
	{"pop     ds0",		0,				0,					0,					0				},
	// 0x20
	{"and",				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{"and",				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{"and",				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{"and",				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"and",				0,				PARAM_AL,			PARAM_UI8,			0				},
	{"and",				0,				PARAM_AW,			PARAM_IMM,			0				},
	{"ds1:",			SEG_DS1,		0,					0,					0				},
	{"adj4a",			0,				0,					0,					0				},
	{"sub",				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{"sub",				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{"sub",				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{"sub",				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"sub",				0,				PARAM_AL,			PARAM_UI8,			0				},
	{"sub",				0,				PARAM_AW,			PARAM_IMM,			0				},
	{"ps:",				SEG_PS,			0,					0,					0				},
	{"adj4s",			0,				0,					0,					0				},
	// 0x30
	{"xor",				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{"xor",				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{"xor",				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{"xor",				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"xor",				0,				PARAM_AL,			PARAM_UI8,			0				},
	{"xor",				0,				PARAM_AW,			PARAM_IMM,			0				},
	{"ss:",				SEG_SS,			0,					0,					0				},
	{"adjba",			0,				0,					0,					0				},
	{"cmp",				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{"cmp",				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{"cmp",				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{"cmp",				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"cmp",				0,				PARAM_AL,			PARAM_UI8,			0				},
	{"cmp",				0,				PARAM_AW,			PARAM_IMM,			0				},
	{"ds0:",			SEG_DS0,		0,					0,					0				},
	{"adjbs",			0,				0,					0,					0				},
	// 0x40
	{"inc",				0,				PARAM_AW,			0,					0				},
	{"inc",				0,				PARAM_CW,			0,					0				},
	{"inc",				0,				PARAM_DW,			0,					0				},
	{"inc",				0,				PARAM_BW,			0,					0				},
	{"inc",				0,				PARAM_SP,			0,					0				},
	{"inc",				0,				PARAM_BP,			0,					0				},
	{"inc",				0,				PARAM_IX,			0,					0				},
	{"inc",				0,				PARAM_IY,			0,					0				},
	{"dec",				0,				PARAM_AW,			0,					0				},
	{"dec",				0,				PARAM_CW,			0,					0				},
	{"dec",				0,				PARAM_DW,			0,					0				},
	{"dec",				0,				PARAM_BW,			0,					0				},
	{"dec",				0,				PARAM_SP,			0,					0				},
	{"dec",				0,				PARAM_BP,			0,					0				},
	{"dec",				0,				PARAM_IX,			0,					0				},
	{"dec",				0,				PARAM_IY,			0,					0				},
	// 0x50
	{"push",			0,				PARAM_AW,			0,					0				},
	{"push",			0,				PARAM_CW,			0,					0				},
	{"push",			0,				PARAM_DW,			0,					0				},
	{"push",			0,				PARAM_BW,			0,					0				},
	{"push",			0,				PARAM_SP,			0,					0				},
	{"push",			0,				PARAM_BP,			0,					0				},
	{"push",			0,				PARAM_IX,			0,					0				},
	{"push",			0,				PARAM_IY,			0,					0				},
	{"pop",				0,				PARAM_AW,			0,					0				},
	{"pop",				0,				PARAM_CW,			0,					0				},
	{"pop",				0,				PARAM_DW,			0,					0				},
	{"pop",				0,				PARAM_BW,			0,					0				},
	{"pop",				0,				PARAM_SP,			0,					0				},
	{"pop",				0,				PARAM_BP,			0,					0				},
	{"pop",				0,				PARAM_IX,			0,					0				},
	{"pop",				0,				PARAM_IY,			0,					0				},
	// 0x60
	{"push    r",		0,				0,					0,					0				},
	{"pop     r",		0,				0,					0,					0				},
	{"chkind",			MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"brkn",			0,				PARAM_UI8,			0,					0,				DASMFLAG_STEP_OVER},	/* V25S/V35S only */
	{"repnc",			PREFIX,			0,					0,					0				},
	{"repc",			PREFIX,			0,					0,					0				},
	{"fpo2    0",		0,				0,					0,					0				},	/* for a coprocessor that was never made */
	{"fpo2    1",		0,				0,					0,					0				},	/* for a coprocessor that was never made */
	{"push",			0,				PARAM_IMM,			0,					0				},
	{"mul",				MODRM,			PARAM_REG16,		PARAM_RM16,			PARAM_IMM		},
	{"push",			0,				PARAM_I8,			0,					0				},
	{"mul",				MODRM,			PARAM_REG16,		PARAM_RM16,			PARAM_I8		},
	{"inmb",			0,				0,					0,					0				},
	{"inmw",			0,				0,					0,					0				},
	{"outmb",			0,				0,					0,					0				},
	{"outmw",			0,				0,					0,					0				},
	// 0x70
	{"bv",				0,				PARAM_REL8,			0,					0				},
	{"bnv",				0,				PARAM_REL8,			0,					0				},
	{"bc",				0,				PARAM_REL8,			0,					0				},
	{"bnc",				0,				PARAM_REL8,			0,					0				},
	{"be",				0,				PARAM_REL8,			0,					0				},
	{"bne",				0,				PARAM_REL8,			0,					0				},
	{"bnh",				0,				PARAM_REL8,			0,					0				},
	{"bh",				0,				PARAM_REL8,			0,					0				},
	{"bn",				0,				PARAM_REL8,			0,					0				},
	{"bp",				0,				PARAM_REL8,			0,					0				},
	{"bpe",				0,				PARAM_REL8,			0,					0				},
	{"bpo",				0,				PARAM_REL8,			0,					0				},
	{"blt",				0,				PARAM_REL8,			0,					0				},
	{"bge",				0,				PARAM_REL8,			0,					0				},
	{"ble",				0,				PARAM_REL8,			0,					0				},
	{"bgt",				0,				PARAM_REL8,			0,					0				},
	// 0x80
	{"immb",			GROUP,			0,					0,					0				},
	{"immw",			GROUP,			0,					0,					0				},
	{"immb",			GROUP,			0,					0,					0				},
	{"immws",			GROUP,			0,					0,					0				},
	{"test",			MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{"test",			MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{"xch",				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{"xch",				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"mov",				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{"mov",				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{"mov",				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{"mov",				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"mov",				MODRM,			PARAM_RM16,			PARAM_SREG,			0				},
	{"ldea",			MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"mov",				MODRM,			PARAM_SREG,			PARAM_RM16,			0				},
	{"pop",				MODRM,			PARAM_RM16,			0,					0				},
	// 0x90
	{"nop",				0,				0,					0,					0				},
	{"xch",				0,				PARAM_AW,			PARAM_CW,			0				},
	{"xch",				0,				PARAM_AW,			PARAM_DW,			0				},
	{"xch",				0,				PARAM_AW,			PARAM_BW,			0				},
	{"xch",				0,				PARAM_AW,			PARAM_SP,			0				},
	{"xch",				0,				PARAM_AW,			PARAM_BP,			0				},
	{"xch",				0,				PARAM_AW,			PARAM_IX,			0				},
	{"xch",				0,				PARAM_AW,			PARAM_IY,			0				},
	{"cvtbw",			0,				0,					0,					0				},
	{"cvtwl",			0,				0,					0,					0				},
	{"call",			0,				PARAM_ADDR,			0,					0,				DASMFLAG_STEP_OVER},
	{"poll",			0,				0,					0,					0				},
	{"push    psw",		0,				0,					0,					0				},
	{"pop     psw",		0,				0,					0,					0				},
	{"mov     psw,ah",	0,				0,					0,					0				},
	{"mov     ah,psw",	0,				0,					0,					0				},
	// 0xa0
	{"mov",				0,				PARAM_AL,			PARAM_MEM_OFFS,		0				},
	{"mov",				0,				PARAM_AW,			PARAM_MEM_OFFS,		0				},
	{"mov",				0,				PARAM_MEM_OFFS,		PARAM_AL,			0				},
	{"mov",				0,				PARAM_MEM_OFFS,		PARAM_AW,			0				},
	{"movbkb",			0,				0,					0,					0				},
	{"movbkw",			0,				0,					0,					0				},
	{"cmpbkb",			0,				0,					0,					0				},
	{"cmpbkw",			0,				0,					0,					0				},
	{"test",			0,				PARAM_AL,			PARAM_UI8,			0				},
	{"test",			0,				PARAM_AW,			PARAM_IMM,			0				},
	{"stmb",			0,				0,					0,					0				},
	{"stmw",			0,				0,					0,					0				},
	{"ldmb",			0,				0,					0,					0				},
	{"ldmw",			0,				0,					0,					0				},
	{"cmpmb",			0,				0,					0,					0				},
	{"cmpmw",			0,				0,					0,					0				},
	// 0xb0
	{"mov",				0,				PARAM_AL,			PARAM_UI8,			0				},
	{"mov",				0,				PARAM_CL,			PARAM_UI8,			0				},
	{"mov",				0,				PARAM_DL,			PARAM_UI8,			0				},
	{"mov",				0,				PARAM_BL,			PARAM_UI8,			0				},
	{"mov",				0,				PARAM_AH,			PARAM_UI8,			0				},
	{"mov",				0,				PARAM_CH,			PARAM_UI8,			0				},
	{"mov",				0,				PARAM_DH,			PARAM_UI8,			0				},
	{"mov",				0,				PARAM_BH,			PARAM_UI8,			0				},
	{"mov",				0,				PARAM_AW,			PARAM_IMM,			0				},
	{"mov",				0,				PARAM_CW,			PARAM_IMM,			0				},
	{"mov",				0,				PARAM_DW,			PARAM_IMM,			0				},
	{"mov",				0,				PARAM_BW,			PARAM_IMM,			0				},
	{"mov",				0,				PARAM_SP,			PARAM_IMM,			0				},
	{"mov",				0,				PARAM_BP,			PARAM_IMM,			0				},
	{"mov",				0,				PARAM_IX,			PARAM_IMM,			0				},
	{"mov",				0,				PARAM_IY,			PARAM_IMM,			0				},
	// 0xc0
	{"shiftbi",			GROUP,			0,					0,					0				},
	{"shiftwi",			GROUP,			0,					0,					0				},
	{"ret",				0,				PARAM_I16,			0,					0,				DASMFLAG_STEP_OUT},
	{"ret",				0,				0,					0,					0,				DASMFLAG_STEP_OUT},
	{"mov     ds1,",	MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"mov     ds0,",	MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{"mov",				MODRM,			PARAM_RMPTR8,		PARAM_UI8,			0				},
	{"mov",				MODRM,			PARAM_RMPTR16,		PARAM_IMM,			0				},
	{"prepare",			0,				PARAM_I16,			PARAM_UI8,			0				},
	{"dispose",			0,				0,					0,					0				},
	{"retf",			0,				PARAM_I16,			0,					0,				DASMFLAG_STEP_OUT},
	{"retf",			0,				0,					0,					0,				DASMFLAG_STEP_OUT},
	{"brk     3",		0,				0,					0,					0,				DASMFLAG_STEP_OVER},
	{"brk",				0,				PARAM_UI8,			0,					0,				DASMFLAG_STEP_OVER},
	{"brkv",			0,				0,					0,					0				},
	{"reti",			0,				0,					0,					0,				DASMFLAG_STEP_OUT},
	// 0xd0
	{"shiftb",			GROUP,			0,					0,					0				},
	{"shiftw",			GROUP,			0,					0,					0				},
	{"shiftbv",			GROUP,			0,					0,					0				},
	{"shiftwv",			GROUP,			0,					0,					0				},
	{"cvtbd",			0,				PARAM_I8,			0,					0				},
	{"cvtdb",			0,				PARAM_I8,			0,					0				},
	{"???",				0,				0,					0,					0				},
	{"trans",			0,				0,					0,					0				},
	{"escape",			FPU,			0,					0,					0				},
	{"escape",			FPU,			0,					0,					0				},
	{"escape",			FPU,			0,					0,					0				},
	{"escape",			FPU,			0,					0,					0				},
	{"escape",			FPU,			0,					0,					0				},
	{"escape",			FPU,			0,					0,					0				},
	{"escape",			FPU,			0,					0,					0				},
	{"escape",			FPU,			0,					0,					0				},
	// 0xe0
	{"dbnzne",			0,				PARAM_REL8,			0,					0,				DASMFLAG_STEP_OVER},
	{"dbnze",			0,				PARAM_REL8,			0,					0,				DASMFLAG_STEP_OVER},
	{"dbnz",			0,				PARAM_REL8,			0,					0,				DASMFLAG_STEP_OVER},
	{"bcwz",			0,				PARAM_REL8,			0,					0				},
	{"in",				0,				PARAM_AL,			PARAM_UI8,			0				},
	{"in",				0,				PARAM_AW,			PARAM_UI8,			0				},
	{"out",				0,				PARAM_UI8,			PARAM_AL,			0				},
	{"out",				0,				PARAM_UI8,			PARAM_AW,			0				},
	{"call",			0,				PARAM_REL16,		0,					0,				DASMFLAG_STEP_OVER},
	{"br",				0,				PARAM_REL16,		0,					0				},
	{"br",				0,				PARAM_ADDR,			0,					0				},
	{"br",				0,				PARAM_REL8,			0,					0				},
	{"in",				0,				PARAM_AL,			PARAM_DW,			0				},
	{"in",				0,				PARAM_AW,			PARAM_DW,			0				},
	{"out",				0,				PARAM_DW,			PARAM_AL,			0				},
	{"out",				0,				PARAM_DW,			PARAM_AW,			0				},
	// 0xf0
	{"buslock",			PREFIX,			0,					0,					0				},
	{"brks",			0,				PARAM_UI8,			0,					0,				DASMFLAG_STEP_OVER},	/* V25S/V35S only */
	{"repne",			PREFIX,			0,					0,					0				},
	{"rep",				PREFIX,			0,					0,					0				},
	{"halt",			0,				0,					0,					0				},
	{"not1    cy",		0,				0,					0,					0				},
	{"group1b",			GROUP,			0,					0,					0				},
	{"group1w",			GROUP,			0,					0,					0				},
	{"clr1    cy",		0,				0,					0,					0				},
	{"set1    cy",		0,				0,					0,					0				},
	{"di",				0,				0,					0,					0				},
	{"ei",				0,				0,					0,					0				},
	{"clr1    dir",		0,				0,					0,					0				},
	{"set1    dir",		0,				0,					0,					0				},
	{"group2b",			GROUP,			0,					0,					0				},
	{"group2w",			GROUP,			0,					0,					0				}
};

static const I386_OPCODE necv_opcode_table2[256] =
{
	// 0x00
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0x10
	{"test1",			MODRM,			PARAM_RMPTR8,		PARAM_CL,			0				},
	{"test1",			MODRM,			PARAM_RMPTR16,		PARAM_CL,			0				},
	{"clr1",			MODRM,			PARAM_RMPTR8,		PARAM_CL,			0				},
	{"clr1",			MODRM,			PARAM_RMPTR16,		PARAM_CL,			0				},
	{"set1",			MODRM,			PARAM_RMPTR8,		PARAM_CL,			0				},
	{"set1",			MODRM,			PARAM_RMPTR16,		PARAM_CL,			0				},
	{"not1",			MODRM,			PARAM_RMPTR8,		PARAM_CL,			0				},
	{"not1",			MODRM,			PARAM_RMPTR16,		PARAM_CL,			0				},
	{"test1",			MODRM,			PARAM_RMPTR8,		PARAM_I3,			0				},
	{"test1",			MODRM,			PARAM_RMPTR16,		PARAM_I4,			0				},
	{"clr1",			MODRM,			PARAM_RMPTR8,		PARAM_I3,			0				},
	{"clr1",			MODRM,			PARAM_RMPTR16,		PARAM_I4,			0				},
	{"set1",			MODRM,			PARAM_RMPTR8,		PARAM_I3,			0				},
	{"set1",			MODRM,			PARAM_RMPTR16,		PARAM_I4,			0				},
	{"not1",			MODRM,			PARAM_RMPTR8,		PARAM_I3,			0				},
	{"not1",			MODRM,			PARAM_RMPTR16,		PARAM_I4,			0				},
	// 0x20
	{"add4s",			0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"sub4s",			0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"movspa",			0,				0,					0,					0				},	/* V25/V35 only */
	{"cmp4s",			0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"rol4",			MODRM,			PARAM_RMPTR8,		0,					0				},
	{"???",				0,				0,					0,					0				},
	{"ror4",			MODRM,			PARAM_RMPTR8,		0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"brkcs",			MODRM,			PARAM_REG2_16,		0,					0,				DASMFLAG_STEP_OVER},	/* V25/V35 only */
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0x30
	{"???",				0,				0,					0,					0				},
	{"ins",				MODRM,			PARAM_REG2_8,		PARAM_REG8,			0				},
	{"???",				0,				0,					0,					0				},
	{"ext",				MODRM,			PARAM_REG2_8,		PARAM_REG8,			0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"ins",				MODRM,			PARAM_REG2_8,		PARAM_I4,			0				},
	{"???",				0,				0,					0,					0				},
	{"ext",				MODRM,			PARAM_REG2_8,		PARAM_I4,			0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0x40
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0x50
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0x60
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0x70
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0x80
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0x90
	{"???",				0,				0,					0,					0				},
	{"retrbi",			0,				0,					0,					0				},	/* V25/V35 only */
	{"fint",			0,				0,					0,					0				},	/* V25/V35 only */
	{"???",				0,				0,					0,					0				},
	{"tsksw",			MODRM,			PARAM_REG2_16,		0,					0				},	/* V25/V35 only */
	{"movspb",			MODRM,			PARAM_REG2_16,		0,					0				},	/* V25/V35 only */
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"btclr",			0,				PARAM_SFREG,		PARAM_I3,			PARAM_REL8		},	/* V25/V35 only */
	{"???",				0,				0,					0,					0				},
	{"stop",			0,				0,					0,					0				},	/* V25/V35 only */
	{"???",				0,				0,					0,					0				},
	// 0xa0
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0xb0
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0xc0
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0xd0
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0xe0
	{"brkxa",			0,				PARAM_UI8,			0,					0				},	/* V33,53 only */
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	// 0xf0
	{"retxa",			0,				PARAM_UI8,			0,					0				},	/* V33,53 only */
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"brkem",			0,				PARAM_UI8,			0,					0				}	/* V20,30,40,50 only */
};

static const I386_OPCODE immb_table[8] =
{
	{"add",				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{"or",				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{"addc",			0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{"subc",			0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{"and",				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{"sub",				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{"xor",				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{"cmp",				0,				PARAM_RMPTR8,		PARAM_UI8,			0				}
};

static const I386_OPCODE immw_table[8] =
{
	{"add",				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{"or",				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{"addc",			0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{"subc",			0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{"and",				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{"sub",				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{"xor",				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{"cmp",				0,				PARAM_RMPTR16,		PARAM_IMM,			0				}
};

static const I386_OPCODE immws_table[8] =
{
	{"add",				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"or",				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"addc",			0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"subc",			0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"and",				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"sub",				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"xor",				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"cmp",				0,				PARAM_RMPTR16,		PARAM_I8,			0				}
};

static const I386_OPCODE shiftbi_table[8] =
{
	{"rol",				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{"ror",				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{"rolc",			0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{"rorc",			0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{"shl",				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{"shr",				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{"???",				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{"shra",			0,				PARAM_RMPTR8,		PARAM_I8,			0				}
};

static const I386_OPCODE shiftwi_table[8] =
{
	{"rol",				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"ror",				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"rolc",			0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"rorc",			0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"shl",				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"shr",				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"???",				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{"shra",			0,				PARAM_RMPTR16,		PARAM_I8,			0				}
};

static const I386_OPCODE shiftb_table[8] =
{
	{"rol",				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{"ror",				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{"rolc",			0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{"rorc",			0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{"shl",				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{"shr",				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{"???",				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{"shra",			0,				PARAM_RMPTR8,		PARAM_1,			0				}
};

static const I386_OPCODE shiftw_table[8] =
{
	{"rol",				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{"ror",				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{"rolc",			0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{"rorc",			0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{"shl",				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{"shr",				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{"???",				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{"shra",			0,				PARAM_RMPTR16,		PARAM_1,			0				}
};

static const I386_OPCODE shiftbv_table[8] =
{
	{"rol",				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{"ror",				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{"rolc",			0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{"rorc",			0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{"shl",				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{"shr",				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{"???",				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{"shra",			0,				PARAM_RMPTR8,		PARAM_CL,			0				}
};

static const I386_OPCODE shiftwv_table[8] =
{
	{"rol",				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{"ror",				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{"rolc",			0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{"rorc",			0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{"shl",				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{"shr",				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{"???",				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{"shra",			0,				PARAM_RMPTR16,		PARAM_CL,			0				}
};

static const I386_OPCODE group1b_table[8] =
{
	{"test",			0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{"???",				0,				0,					0,					0				},
	{"not",				0,				PARAM_RMPTR8,		0,					0				},
	{"neg",				0,				PARAM_RMPTR8,		0,					0				},
	{"mulu",			0,				PARAM_RMPTR8,		0,					0				},
	{"mul",				0,				PARAM_RMPTR8,		0,					0				},
	{"divu",			0,				PARAM_RMPTR8,		0,					0				},
	{"div",				0,				PARAM_RMPTR8,		0,					0				}
};

static const I386_OPCODE group1w_table[8] =
{
	{"test",			0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{"???",				0,				0,					0,					0				},
	{"not",				0,				PARAM_RMPTR16,		0,					0				},
	{"neg",				0,				PARAM_RMPTR16,		0,					0				},
	{"mulu",			0,				PARAM_RMPTR16,		0,					0				},
	{"mul",				0,				PARAM_RMPTR16,		0,					0				},
	{"divu",			0,				PARAM_RMPTR16,		0,					0				},
	{"div",				0,				PARAM_RMPTR16,		0,					0				}
};

static const I386_OPCODE group2b_table[8] =
{
	{"inc",				0,				PARAM_RMPTR8,		0,					0				},
	{"dec",				0,				PARAM_RMPTR8,		0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				},
	{"???",				0,				0,					0,					0				}
};

static const I386_OPCODE group2w_table[8] =
{
	{"inc",				0,				PARAM_RMPTR16,		0,					0				},
	{"dec",				0,				PARAM_RMPTR16,		0,					0				},
	{"call",			0,				PARAM_RMPTR16,		0,					0,				DASMFLAG_STEP_OVER},
	{"call    far ptr ",0,				PARAM_RM16,			0,					0,				DASMFLAG_STEP_OVER},
	{"br",				0,				PARAM_RMPTR16,		0,					0				},
	{"br      far ptr ",0,				PARAM_RM16,			0,					0				},
	{"push",			0,				PARAM_RMPTR16,		0,					0				},
	{"???",				0,				0,					0,					0				}
};

static const GROUP_OP group_op_table[] =
{
	{ "immb",				immb_table				},
	{ "immw",				immw_table				},
	{ "immws",				immws_table				},
	{ "shiftbi",			shiftbi_table			},
	{ "shiftwi",			shiftwi_table			},
	{ "shiftb",				shiftb_table			},
	{ "shiftw",				shiftw_table			},
	{ "shiftbv",			shiftbv_table			},
	{ "shiftwv",			shiftwv_table			},
	{ "group1b",			group1b_table			},
	{ "group1w",			group1w_table			},
	{ "group2b",			group2b_table			},
	{ "group2w",			group2w_table			}
};



static const char *const nec_reg[8] = { "aw",  "cw",  "dw",  "bw",  "sp",  "bp",  "ix",  "iy" };
static const char *const nec_reg8[8] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
static const char *const nec_sreg[8] = { "ds1", "ps", "ss", "ds0", "???", "???", "???", "???" };
static const char *const nec_sfreg[256] =
{
	/* 0x00 */
	"p0",	"pm0",	"pmc0",	"???",	"???",	"???",	"???",	"???",
	"p1",	"pm1",	"pmc1",	"???",	"???",	"???",	"???",	"???",
	/* 0x10 */
	"p2",	"pm2",	"pmc2",	"???",	"???",	"???",	"???",	"???",
	"???",	"???",	"???",	"???",	"???",	"???",	"???",	"???",
	/* 0x20 */
	"???",	"???",	"???",	"???",	"???",	"???",	"???",	"???",
	"???",	"???",	"???",	"???",	"???",	"???",	"???",	"???",
	/* 0x30 */
	"???",	"???",	"???",	"???",	"???",	"???",	"???",	"???",
	"pt",	"???",	"???",	"pmt",	"???",	"???",	"???",	"???",
	/* 0x40 */
	"intm",	"???",	"???",	"???",	"ems0",	"ems1",	"ems2",	"???",
	"???",	"???",	"???",	"???",	"exic0","exic1","exic2","???",
	/* 0x50 */
	"???",	"???",	"???",	"???",	"???",	"???",	"???",	"???",
	"???",	"???",	"???",	"???",	"???",	"???",	"???",	"???",
	/* 0x60 */
	"rxb0",	"???",	"txb0",	"???",	"???",	"srms0","stms0","???",
	"scm0",	"scc0",	"brg0",	"scs0",	"seic0","sric0","stic0","???",
	/* 0x70 */
	"rxb1",	"???",	"txb1",	"???",	"???",	"srms1","stms1","???",
	"scm1",	"scc1",	"brg1",	"scs1",	"seic1","sric1","stic1","???",
	/* 0x80 */
	"tm0",	"???",	"md0",	"???",	"???",	"???",	"???",	"???",
	"tm1",	"???",	"md1",	"???",	"???",	"???",	"???",	"???",
	/* 0x90 */
	"tmc0",	"tmc1",	"???",	"???",	"tmms0","tmms1","tmms2","???",
	"???",	"???",	"???",	"???",	"tmic0","tmic1","tmic2","???",
	/* 0xa0 */
	"dmac0","dmam0","dmac1","dmam1","???",	"???",	"???",	"???",
	"???",	"???",	"???",	"???",	"dic0",	"dic1",	"???",	"???",
	/* 0xb0 */
	"???",	"???",	"???",	"???",	"???",	"???",	"???",	"???",
	"???",	"???",	"???",	"???",	"???",	"???",	"???",	"???",
	/* 0xc0 */
	"sar0l","sar0m","sar0h","???",	"dar0l","dar0m","dar0h","???",
	"tc0l",	"tc0h",	"???",	"???",	"???",	"???",	"???",	"???",
	/* 0xd0 */
	"sar1l","sar1m","sar1h","???",	"dar1l","dar1m","dar1h","???",
	"tc1l",	"tc1h",	"???",	"???",	"???",	"???",	"???",	"???",
	/* 0xe0 */
	"stbc",	"rfm",	"???",	"???",	"???",	"???",	"???",	"???",
	"wtc",	"???",	"flag",	"prc",	"tbic",	"???",	"???",	"irqs",
	/* 0xf0 */
	"???",	"???",	"???",	"???",	"???",	"???",	"???",	"???",
	"???",	"???",	"???",	"???",	"ispr",	"???",	"???",	"idb"
};

static UINT32 pc;
static UINT8 modrm;
static UINT32 segment;
static offs_t dasm_flags;
static char modrm_string[256];

#define MODRM_REG1	((modrm >> 3) & 0x7)
#define MODRM_REG2	(modrm & 0x7)

#define MAX_LENGTH	8

INLINE UINT8 FETCHD(void)
{
	if ((opcode_ptr - opcode_ptr_base) + 1 > MAX_LENGTH)
		return 0xff;
	pc++;
	return *opcode_ptr++;
}

INLINE UINT16 FETCHD16(void)
{
	UINT16 d;
	if ((opcode_ptr - opcode_ptr_base) + 2 > MAX_LENGTH)
		return 0xffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8);
	opcode_ptr += 2;
	pc += 2;
	return d;
}

static char *hexstring(UINT32 value, int digits)
{
	static char buffer[20];
	buffer[0] = '0';
	if (digits) {
		sprintf(&buffer[1], "%0*Xh", digits, value);
	} else {
		sprintf(&buffer[1], "%Xh", value);
	}
	return (buffer[1] >= '0' && buffer[1] <= '9') ? &buffer[1] : &buffer[0];
}

static char *shexstring(UINT32 value, int digits, int always)
{
	static char buffer[20];
	if (value >= 0x80000000) {
		sprintf(buffer, "-%s", hexstring(-value, digits));
	} else if (always) {
		sprintf(buffer, "+%s", hexstring(value, digits));
	} else {
		return hexstring(value, digits);
	}
	return buffer;
}

static void handle_modrm(char* s)
{
	INT8 disp8;
	INT16 disp16;
	UINT8 mod, rm;

	modrm = FETCHD();
	mod = (modrm >> 6) & 0x3;
	rm = (modrm & 0x7);

	if( modrm >= 0xc0 )
		return;

	switch(segment)
	{
		case SEG_PS: s += sprintf( s, "ps:" ); break;
		case SEG_DS0: s += sprintf( s, "ds0:" ); break;
		case SEG_DS1: s += sprintf( s, "ds1:" ); break;
		case SEG_SS: s += sprintf( s, "ss:" ); break;
	}

	s += sprintf( s, "[" );
	switch( rm )
	{
		case 0: s += sprintf( s, "bw+ix" ); break;
		case 1: s += sprintf( s, "bw+iy" ); break;
		case 2: s += sprintf( s, "bp+ix" ); break;
		case 3: s += sprintf( s, "bp+iy" ); break;
		case 4: s += sprintf( s, "ix" ); break;
		case 5: s += sprintf( s, "iy" ); break;
		case 6:
			if( mod == 0 ) {
				disp16 = FETCHD16();
				s += sprintf( s, "%s", hexstring((unsigned) (UINT16) disp16, 0) );
			} else {
				s += sprintf( s, "bp" );
			}
			break;
		case 7: s += sprintf( s, "bw" ); break;
	}
	if( mod == 1 ) {
		disp8 = FETCHD();
		s += sprintf( s, "%s", shexstring((INT32)disp8, 0, TRUE) );
	} else if( mod == 2 ) {
		disp16 = FETCHD16();
		s += sprintf( s, "%s", shexstring((INT32)disp16, 0, TRUE) );
	}
	s += sprintf( s, "]" );
}

static char* handle_param(char* s, UINT32 param)
{
	UINT8 i8;
	UINT16 i16;
	UINT16 ptr;
	UINT32 addr;
	INT8 d8;
	INT16 d16;

	switch(param)
	{
		case PARAM_REG8:
			s += sprintf( s, "%s", nec_reg8[MODRM_REG1] );
			break;

		case PARAM_REG16:
			s += sprintf( s, "%s", nec_reg[MODRM_REG1] );
			break;

		case PARAM_REG2_8:
			s += sprintf( s, "%s", nec_reg8[MODRM_REG2] );
			break;

		case PARAM_REG2_16:
			s += sprintf( s, "%s", nec_reg[MODRM_REG2] );
			break;

		case PARAM_RM8:
		case PARAM_RMPTR8:
			if( modrm >= 0xc0 ) {
				s += sprintf( s, "%s", nec_reg8[MODRM_REG2] );
			} else {
				if (param == PARAM_RMPTR8)
					s += sprintf( s, "byte ptr " );
				s += sprintf( s, "%s", modrm_string );
			}
			break;

		case PARAM_RM16:
		case PARAM_RMPTR16:
			if( modrm >= 0xc0 ) {
				s += sprintf( s, "%s", nec_reg[MODRM_REG2] );
			} else {
				if (param == PARAM_RMPTR16)
					s += sprintf( s, "word ptr " );
				s += sprintf( s, "%s", modrm_string );
			}
			break;

		case PARAM_I3:
			i8 = FETCHD();
			s += sprintf( s, "%d", i8 & 0x07 );
			break;

		case PARAM_I4:
			i8 = FETCHD();
			s += sprintf( s, "%d", i8 & 0x0f );
			break;

		case PARAM_I8:
			i8 = FETCHD();
			s += sprintf( s, "%s", shexstring((INT8)i8, 0, FALSE) );
			break;

		case PARAM_I16:
			i16 = FETCHD16();
			s += sprintf( s, "%s", shexstring((INT16)i16, 0, FALSE) );
			break;

		case PARAM_UI8:
			i8 = FETCHD();
			s += sprintf( s, "%s", shexstring((UINT8)i8, 0, FALSE) );
			break;

		case PARAM_IMM:
			i16 = FETCHD16();
			s += sprintf( s, "%s", hexstring(i16, 0) );
			break;

		case PARAM_ADDR:
			addr = FETCHD16();
			ptr = FETCHD16();
			s += sprintf( s, "%s:", hexstring(ptr, 4) );
			s += sprintf( s, "%s", hexstring(addr, 0) );
			break;

		case PARAM_REL16:
			/* make sure to keep the relative offset within the segment */
			d16 = FETCHD16();
			s += sprintf( s, "%s", hexstring((pc & 0xFFFF0000) | ((pc + d16) & 0x0000FFFF), 0) );
			break;

		case PARAM_REL8:
			d8 = FETCHD();
			s += sprintf( s, "%s", hexstring(pc + d8, 0) );
			break;

		case PARAM_MEM_OFFS:
			switch(segment)
			{
				case SEG_PS: s += sprintf( s, "ps:" ); break;
				case SEG_DS0: s += sprintf( s, "ds0:" ); break;
				case SEG_DS1: s += sprintf( s, "ds1:" ); break;
				case SEG_SS: s += sprintf( s, "ss:" ); break;
			}

			i16 = FETCHD16();
			s += sprintf( s, "[%s]", hexstring(i16, 0) );
			break;

		case PARAM_SREG:
			s += sprintf( s, "%s", nec_sreg[MODRM_REG1] );
			break;

		case PARAM_SFREG:
			i8 = FETCHD();
			s += sprintf( s, "%s", nec_sfreg[i8] );
			break;

		case PARAM_1:
			s += sprintf( s, "1" );
			break;

		case PARAM_AL: s += sprintf( s, "al" ); break;
		case PARAM_CL: s += sprintf( s, "cl" ); break;
		case PARAM_DL: s += sprintf( s, "dl" ); break;
		case PARAM_BL: s += sprintf( s, "bl" ); break;
		case PARAM_AH: s += sprintf( s, "ah" ); break;
		case PARAM_CH: s += sprintf( s, "ch" ); break;
		case PARAM_DH: s += sprintf( s, "dh" ); break;
		case PARAM_BH: s += sprintf( s, "bh" ); break;

		case PARAM_AW: s += sprintf( s, "aw" ); break;
		case PARAM_CW: s += sprintf( s, "cw" ); break;
		case PARAM_DW: s += sprintf( s, "dw" ); break;
		case PARAM_BW: s += sprintf( s, "bw" ); break;
		case PARAM_SP: s += sprintf( s, "sp" ); break;
		case PARAM_BP: s += sprintf( s, "bp" ); break;
		case PARAM_IX: s += sprintf( s, "ix" ); break;
		case PARAM_IY: s += sprintf( s, "iy" ); break;
	}
	return s;
}

static void handle_fpu(char *s, UINT8 op1, UINT8 op2)
{
	switch (op1 & 0x7)
	{
		case 0:		// Group D8
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: sprintf(s, "fadd    dword ptr %s", modrm_string); break;
					case 1: sprintf(s, "fmul    dword ptr %s", modrm_string); break;
					case 2: sprintf(s, "fcom    dword ptr %s", modrm_string); break;
					case 3: sprintf(s, "fcomp   dword ptr %s", modrm_string); break;
					case 4: sprintf(s, "fsub    dword ptr %s", modrm_string); break;
					case 5: sprintf(s, "fsubr   dword ptr %s", modrm_string); break;
					case 6: sprintf(s, "fdiv    dword ptr %s", modrm_string); break;
					case 7: sprintf(s, "fdivr   dword ptr %s", modrm_string); break;
				}
			} else {
				switch ((op2 >> 3) & 0x7)
				{
					case 0: sprintf(s, "fadd    st(0),st(%d)", op2 & 0x7); break;
					case 1: sprintf(s, "fcom    st(0),st(%d)", op2 & 0x7); break;
					case 2: sprintf(s, "fsub    st(0),st(%d)", op2 & 0x7); break;
					case 3: sprintf(s, "fdiv    st(0),st(%d)", op2 & 0x7); break;
					case 4: sprintf(s, "fmul    st(0),st(%d)", op2 & 0x7); break;
					case 5: sprintf(s, "fcomp   st(0),st(%d)", op2 & 0x7); break;
					case 6: sprintf(s, "fsubr   st(0),st(%d)", op2 & 0x7); break;
					case 7: sprintf(s, "fdivr   st(0),st(%d)", op2 & 0x7); break;
				}
			}
			break;
		}

		case 1:		// Group D9
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: sprintf(s, "fld     dword ptr %s", modrm_string); break;
					case 1: sprintf(s, "??? (FPU)"); break;
					case 2: sprintf(s, "fst     dword ptr %s", modrm_string); break;
					case 3: sprintf(s, "fstp    dword ptr %s", modrm_string); break;
					case 4: sprintf(s, "fldenv  word ptr %s", modrm_string); break;
					case 5: sprintf(s, "fldcw   word ptr %s", modrm_string); break;
					case 6: sprintf(s, "fstenv  word ptr %s", modrm_string); break;
					case 7: sprintf(s, "fstcw   word ptr %s", modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						sprintf(s, "fld     st(0),st(%d)", op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						sprintf(s, "fxch    st(0),st(%d)", op2 & 0x7); break;

					case 0x10: sprintf(s, "fnop"); break;
					case 0x20: sprintf(s, "fchs"); break;
					case 0x21: sprintf(s, "fabs"); break;
					case 0x24: sprintf(s, "ftst"); break;
					case 0x25: sprintf(s, "fxam"); break;
					case 0x28: sprintf(s, "fld1"); break;
					case 0x29: sprintf(s, "fldl2t"); break;
					case 0x2a: sprintf(s, "fldl2e"); break;
					case 0x2b: sprintf(s, "fldpi"); break;
					case 0x2c: sprintf(s, "fldlg2"); break;
					case 0x2d: sprintf(s, "fldln2"); break;
					case 0x2e: sprintf(s, "fldz"); break;
					case 0x30: sprintf(s, "f2xm1"); break;
					case 0x31: sprintf(s, "fyl2x"); break;
					case 0x32: sprintf(s, "fptan"); break;
					case 0x33: sprintf(s, "fpatan"); break;
					case 0x34: sprintf(s, "fxtract"); break;
					case 0x35: sprintf(s, "fprem1"); break;
					case 0x36: sprintf(s, "fdecstp"); break;
					case 0x37: sprintf(s, "fincstp"); break;
					case 0x38: sprintf(s, "fprem"); break;
					case 0x39: sprintf(s, "fyl2xp1"); break;
					case 0x3a: sprintf(s, "fsqrt"); break;
					case 0x3b: sprintf(s, "fsincos"); break;
					case 0x3c: sprintf(s, "frndint"); break;
					case 0x3d: sprintf(s, "fscale"); break;
					case 0x3e: sprintf(s, "fsin"); break;
					case 0x3f: sprintf(s, "fcos"); break;

					default: sprintf(s, "??? (FPU)"); break;
				}
			}
			break;
		}

		case 2:		// Group DA
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: sprintf(s, "fiadd   dword ptr %s", modrm_string); break;
					case 1: sprintf(s, "fimul   dword ptr %s", modrm_string); break;
					case 2: sprintf(s, "ficom   dword ptr %s", modrm_string); break;
					case 3: sprintf(s, "ficomp  dword ptr %s", modrm_string); break;
					case 4: sprintf(s, "fisub   dword ptr %s", modrm_string); break;
					case 5: sprintf(s, "fisubr  dword ptr %s", modrm_string); break;
					case 6: sprintf(s, "fidiv   dword ptr %s", modrm_string); break;
					case 7: sprintf(s, "fidivr  dword ptr %s", modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						sprintf(s, "fcmovb  st(0),st(%d)", op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						sprintf(s, "fcmove  st(0),st(%d)", op2 & 0x7); break;

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
						sprintf(s, "fcmovbe st(0),st(%d)", op2 & 0x7); break;

					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
						sprintf(s, "fcmovu  st(0),st(%d)", op2 & 0x7); break;

					default: sprintf(s, "??? (FPU)"); break;

				}
			}
			break;
		}

		case 3:		// Group DB
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: sprintf(s, "fild    dword ptr %s", modrm_string); break;
					case 1: sprintf(s, "??? (FPU)"); break;
					case 2: sprintf(s, "fist    dword ptr %s", modrm_string); break;
					case 3: sprintf(s, "fistp   dword ptr %s", modrm_string); break;
					case 4: sprintf(s, "??? (FPU)"); break;
					case 5: sprintf(s, "fld     tword ptr %s", modrm_string); break;
					case 6: sprintf(s, "??? (FPU)"); break;
					case 7: sprintf(s, "fstp    tword ptr %s", modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						sprintf(s, "fcmovnb st(0),st(%d)", op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						sprintf(s, "fcmovne st(0),st(%d)", op2 & 0x7); break;

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
						sprintf(s, "fcmovnbe st(0),st(%d)", op2 & 0x7); break;

					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
						sprintf(s, "fcmovnu st(0),st(%d)", op2 & 0x7); break;

					case 0x22: sprintf(s, "fclex"); break;
					case 0x23: sprintf(s, "finit"); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						sprintf(s, "fucomi  st(0),st(%d)", op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						sprintf(s, "fcomi   st(0),st(%d)", op2 & 0x7); break;

					default: sprintf(s, "??? (FPU)"); break;
				}
			}
			break;
		}

		case 4:		// Group DC
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: sprintf(s, "fadd    qword ptr %s", modrm_string); break;
					case 1: sprintf(s, "fmul    qword ptr %s", modrm_string); break;
					case 2: sprintf(s, "fcom    qword ptr %s", modrm_string); break;
					case 3: sprintf(s, "fcomp   qword ptr %s", modrm_string); break;
					case 4: sprintf(s, "fsub    qword ptr %s", modrm_string); break;
					case 5: sprintf(s, "fsubr   qword ptr %s", modrm_string); break;
					case 6: sprintf(s, "fdiv    qword ptr %s", modrm_string); break;
					case 7: sprintf(s, "fdivr   qword ptr %s", modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						sprintf(s, "fadd    st(%d),st(0)", op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						sprintf(s, "fmul    st(%d),st(0)", op2 & 0x7); break;

					case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
						sprintf(s, "fsubr   st(%d),st(0)", op2 & 0x7); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						sprintf(s, "fsub    st(%d),st(0)", op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						sprintf(s, "fdivr   st(%d),st(0)", op2 & 0x7); break;

					case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
						sprintf(s, "fdiv    st(%d),st(0)", op2 & 0x7); break;

					default: sprintf(s, "??? (FPU)"); break;
				}
			}
			break;
		}

		case 5:		// Group DD
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: sprintf(s, "fld     qword ptr %s", modrm_string); break;
					case 1: sprintf(s, "??? (FPU)"); break;
					case 2: sprintf(s, "fst     qword ptr %s", modrm_string); break;
					case 3: sprintf(s, "fstp    qword ptr %s", modrm_string); break;
					case 4: sprintf(s, "frstor  %s", modrm_string); break;
					case 5: sprintf(s, "??? (FPU)"); break;
					case 6: sprintf(s, "fsave   %s", modrm_string); break;
					case 7: sprintf(s, "fstsw   word ptr %s", modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						sprintf(s, "ffree   st(%d)", op2 & 0x7); break;

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
						sprintf(s, "fst     st(%d)", op2 & 0x7); break;

					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
						sprintf(s, "fstp    st(%d)", op2 & 0x7); break;

					case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
						sprintf(s, "fucom   st(%d), st(0)", op2 & 0x7); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						sprintf(s, "fucomp  st(%d)", op2 & 0x7); break;

					default: sprintf(s, "??? (FPU)"); break;
				}
			}
			break;
		}

		case 6:		// Group DE
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: sprintf(s, "fiadd   word ptr %s", modrm_string); break;
					case 1: sprintf(s, "fimul   word ptr %s", modrm_string); break;
					case 2: sprintf(s, "ficom   word ptr %s", modrm_string); break;
					case 3: sprintf(s, "ficomp  word ptr %s", modrm_string); break;
					case 4: sprintf(s, "fisub   word ptr %s", modrm_string); break;
					case 5: sprintf(s, "fisubr  word ptr %s", modrm_string); break;
					case 6: sprintf(s, "fidiv   word ptr %s", modrm_string); break;
					case 7: sprintf(s, "fidivr  word ptr %s", modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						sprintf(s, "faddp   st(%d)", op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						sprintf(s, "fmulp   st(%d)", op2 & 0x7); break;

					case 0x19: sprintf(s, "fcompp"); break;

					case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
						sprintf(s, "fsubrp  st(%d)", op2 & 0x7); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						sprintf(s, "fsubp   st(%d)", op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						sprintf(s, "fdivrp  st(%d), st(0)", op2 & 0x7); break;

					case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
						sprintf(s, "fdivp   st(%d)", op2 & 0x7); break;

					default: sprintf(s, "??? (FPU)"); break;
				}
			}
			break;
		}

		case 7:		// Group DF
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: sprintf(s, "fild    word ptr %s", modrm_string); break;
					case 1: sprintf(s, "??? (FPU)"); break;
					case 2: sprintf(s, "fist    word ptr %s", modrm_string); break;
					case 3: sprintf(s, "fistp   word ptr %s", modrm_string); break;
					case 4: sprintf(s, "fbld    %s", modrm_string); break;
					case 5: sprintf(s, "fild    qword ptr %s", modrm_string); break;
					case 6: sprintf(s, "fbstp   %s", modrm_string); break;
					case 7: sprintf(s, "fistp   qword ptr %s", modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x20: sprintf(s, "fstsw   aw"); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						sprintf(s, "fucomip st(%d)", op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						sprintf(s, "fcomip  st(%d),st(0)", op2 & 0x7); break;

					default: sprintf(s, "??? (FPU)"); break;
				}
			}
			break;
		}
	}
}

static void decode_opcode(char *s, const I386_OPCODE *op, UINT8 op1 )
{
	int i;
	UINT8 op2;

	switch( op->flags )
	{
		case TWO_BYTE:
			op2 = FETCHD();
			decode_opcode( s, &necv_opcode_table2[op2], op1 );
			return;

		case SEG_PS:
		case SEG_DS0:
		case SEG_DS1:
		case SEG_SS:
			segment = op->flags;
			op2 = FETCHD();
			decode_opcode( s, &necv_opcode_table1[op2], op1 );
			return;

		case PREFIX:
			s += sprintf( s, "%-8s", op->mnemonic );
			op2 = FETCHD();
			decode_opcode( s, &necv_opcode_table1[op2], op1 );
			return;

		case GROUP:
			handle_modrm( modrm_string );
			for( i=0; i < ARRAY_LENGTH(group_op_table); i++ ) {
				if( strcmp(op->mnemonic, group_op_table[i].mnemonic) == 0 )
				{
					decode_opcode( s, &group_op_table[i].opcode[MODRM_REG1], op1 );
					return;
				}
			}
			goto handle_unknown;

		case FPU:
			op2 = FETCHD();
			handle_fpu( s, op1, op2);
			return;

		case MODRM:
			handle_modrm( modrm_string );
			break;
	}

	s += sprintf( s, "%-8s", op->mnemonic );
	dasm_flags = op->dasm_flags;

	if( op->param1 != 0 ) {
		s = handle_param( s, op->param1 );
	}

	if( op->param2 != 0 ) {
		s += sprintf( s, "," );
		s = handle_param( s, op->param2 );
	}

	if( op->param3 != 0 ) {
		s += sprintf( s, "," );
		s = handle_param( s, op->param3 );
	}
	return;

handle_unknown:
	sprintf(s, "???");
}

int necv_dasm_one(char *buffer, UINT32 eip, const UINT8 *oprom)
{
	UINT8 op;

	opcode_ptr = opcode_ptr_base = oprom;
	pc = eip;
	dasm_flags = 0;
	segment = 0;

	op = FETCHD();

	decode_opcode( buffer, &necv_opcode_table1[op], op );
	return (pc-eip) | dasm_flags | DASMFLAG_SUPPORTED;
}

#define STATE_VERSION	1

void I86::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(&regs, sizeof(regs), 1);
	state_fio->FputUint32(pc);
	state_fio->FputUint32(prevpc);
	state_fio->Fwrite(base, sizeof(base), 1);
	state_fio->Fwrite(sregs, sizeof(sregs), 1);
	state_fio->FputUint16(flags);
	state_fio->FputInt32(AuxVal);
	state_fio->FputInt32(OverVal);
	state_fio->FputInt32(SignVal);
	state_fio->FputInt32(ZeroVal);
	state_fio->FputInt32(CarryVal);
	state_fio->FputInt32(DirVal);
	state_fio->FputUint8(ParityVal);
	state_fio->FputUint8(TF);
	state_fio->FputUint8(IF);
	state_fio->FputUint8(MF);
	state_fio->FputInt32(int_state);
	state_fio->FputBool(test_state);
	state_fio->FputBool(busreq);
	state_fio->FputBool(halted);
	state_fio->FputInt32(icount);
	state_fio->FputInt32(extra_icount);
	state_fio->FputBool(seg_prefix);
	state_fio->FputUint8(prefix_seg);
	state_fio->FputUint32(ea);
	state_fio->FputUint16(eo);
	state_fio->FputUint8(ea_seg);
}

bool I86::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(&regs, sizeof(regs), 1);
	pc = state_fio->FgetUint32();
	prevpc = state_fio->FgetUint32();
	state_fio->Fread(base, sizeof(base), 1);
	state_fio->Fread(sregs, sizeof(sregs), 1);
	flags = state_fio->FgetUint16();
	AuxVal = state_fio->FgetInt32();
	OverVal = state_fio->FgetInt32();
	SignVal = state_fio->FgetInt32();
	ZeroVal = state_fio->FgetInt32();
	CarryVal = state_fio->FgetInt32();
	DirVal = state_fio->FgetInt32();
	ParityVal = state_fio->FgetUint8();
	TF = state_fio->FgetUint8();
	IF = state_fio->FgetUint8();
	MF = state_fio->FgetUint8();
	int_state = state_fio->FgetInt32();
	test_state = state_fio->FgetBool();
	busreq = state_fio->FgetBool();
	halted = state_fio->FgetBool();
	icount = state_fio->FgetInt32();
	extra_icount = state_fio->FgetInt32();
	seg_prefix = state_fio->FgetBool();
	prefix_seg = state_fio->FgetUint8();
	ea = state_fio->FgetUint32();
	eo = state_fio->FgetUint16();
	ea_seg = state_fio->FgetUint8();
	return true;
}

