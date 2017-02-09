#ifndef __LIBNEWDEV_I86_DEFS_H__
#define __LIBNEWDEV_I86_DEFS_H__

#ifndef INLINE
#define INLINE inline
#endif

#define offs_t UINT32


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
#ifdef __BIG_ENDIAN__
	 AH,  AL,  CH,  CL,  DH,  DL,  BH,  BL,
	SPH, SPL, BPH, BPL, SIH, SIL, DIH, DIL,
#else
	 AL,  AH,  CL,  CH,  DL,  DH,  BL,  BH,
	SPL, SPH, BPL, BPH, SIL, SIH, DIL, DIH,
#endif
} BREGS;

/************************************************************************/

struct i80x86_timing {
	uint8_t	exception, iret;				/* exception, IRET */
	uint8_t	int3, int_imm, into_nt, into_t;			/* INTs */
	uint8_t	override;					/* segment overrides */
	uint8_t	flag_ops, lahf, sahf;				/* flag operations */
	uint8_t	aaa, aas, aam, aad;				/* arithmetic adjusts */
	uint8_t	daa, das;					/* decimal adjusts */
	uint8_t	cbw, cwd;					/* sign extension */
	uint8_t	hlt, load_ptr, lea, nop, wait, xlat;		/* misc */

	uint8_t	jmp_short, jmp_near, jmp_far;			/* direct JMPs */
	uint8_t	jmp_r16, jmp_m16, jmp_m32;			/* indirect JMPs */
	uint8_t	call_near, call_far;				/* direct CALLs */
	uint8_t	call_r16, call_m16, call_m32;			/* indirect CALLs */
	uint8_t	ret_near, ret_far, ret_near_imm, ret_far_imm;	/* returns */
	uint8_t	jcc_nt, jcc_t, jcxz_nt, jcxz_t;			/* conditional JMPs */
	uint8_t	loop_nt, loop_t, loope_nt, loope_t;		/* loops */

	uint8_t	in_imm8, in_imm16, in_dx8, in_dx16;		/* port reads */
	uint8_t	out_imm8, out_imm16, out_dx8, out_dx16;		/* port writes */

	uint8_t	mov_rr8, mov_rm8, mov_mr8;			/* move, 8-bit */
	uint8_t	mov_ri8, mov_mi8;				/* move, 8-bit immediate */
	uint8_t	mov_rr16, mov_rm16, mov_mr16;			/* move, 16-bit */
	uint8_t	mov_ri16, mov_mi16;				/* move, 16-bit immediate */
	uint8_t	mov_am8, mov_am16, mov_ma8, mov_ma16;		/* move, AL/AX memory */
	uint8_t	mov_sr, mov_sm, mov_rs, mov_ms;			/* move, segment registers */
	uint8_t	xchg_rr8, xchg_rm8;				/* exchange, 8-bit */
	uint8_t	xchg_rr16, xchg_rm16, xchg_ar16;		/* exchange, 16-bit */

	uint8_t	push_r16, push_m16, push_seg, pushf;		/* pushes */
	uint8_t	pop_r16, pop_m16, pop_seg, popf;		/* pops */

	uint8_t	alu_rr8, alu_rm8, alu_mr8;			/* ALU ops, 8-bit */
	uint8_t	alu_ri8, alu_mi8, alu_mi8_ro;			/* ALU ops, 8-bit immediate */
	uint8_t	alu_rr16, alu_rm16, alu_mr16;			/* ALU ops, 16-bit */
	uint8_t	alu_ri16, alu_mi16, alu_mi16_ro;		/* ALU ops, 16-bit immediate */
	uint8_t	alu_r16i8, alu_m16i8, alu_m16i8_ro;		/* ALU ops, 16-bit w/8-bit immediate */
	uint8_t	mul_r8, mul_r16, mul_m8, mul_m16;		/* MUL */
	uint8_t	imul_r8, imul_r16, imul_m8, imul_m16;		/* IMUL */
	uint8_t	div_r8, div_r16, div_m8, div_m16;		/* DIV */
	uint8_t	idiv_r8, idiv_r16, idiv_m8, idiv_m16;		/* IDIV */
	uint8_t	incdec_r8, incdec_r16, incdec_m8, incdec_m16;	/* INC/DEC */
	uint8_t	negnot_r8, negnot_r16, negnot_m8, negnot_m16;	/* NEG/NOT */

	uint8_t	rot_reg_1, rot_reg_base, rot_reg_bit;		/* reg shift/rotate */
	uint8_t	rot_m8_1, rot_m8_base, rot_m8_bit;		/* m8 shift/rotate */
	uint8_t	rot_m16_1, rot_m16_base, rot_m16_bit;		/* m16 shift/rotate */

	uint8_t	cmps8, rep_cmps8_base, rep_cmps8_count;		/* CMPS 8-bit */
	uint8_t	cmps16, rep_cmps16_base, rep_cmps16_count;	/* CMPS 16-bit */
	uint8_t	scas8, rep_scas8_base, rep_scas8_count;		/* SCAS 8-bit */
	uint8_t	scas16, rep_scas16_base, rep_scas16_count;	/* SCAS 16-bit */
	uint8_t	lods8, rep_lods8_base, rep_lods8_count;		/* LODS 8-bit */
	uint8_t	lods16, rep_lods16_base, rep_lods16_count;	/* LODS 16-bit */
	uint8_t	stos8, rep_stos8_base, rep_stos8_count;		/* STOS 8-bit */
	uint8_t	stos16, rep_stos16_base, rep_stos16_count;	/* STOS 16-bit */
	uint8_t	movs8, rep_movs8_base, rep_movs8_count;		/* MOVS 8-bit */
	uint8_t	movs16, rep_movs16_base, rep_movs16_count;	/* MOVS 16-bit */

	uint8_t	ins8, rep_ins8_base, rep_ins8_count;		/* (80186) INS 8-bit */
	uint8_t	ins16, rep_ins16_base, rep_ins16_count;		/* (80186) INS 16-bit */
	uint8_t	outs8, rep_outs8_base, rep_outs8_count;		/* (80186) OUTS 8-bit */
	uint8_t	outs16, rep_outs16_base, rep_outs16_count;	/* (80186) OUTS 16-bit */
	uint8_t	push_imm, pusha, popa;				/* (80186) PUSH immediate, PUSHA/POPA */
	uint8_t	imul_rri8, imul_rmi8;				/* (80186) IMUL immediate 8-bit */
	uint8_t	imul_rri16, imul_rmi16;				/* (80186) IMUL immediate 16-bit */
	uint8_t	enter0, enter1, enter_base, enter_count, leave;	/* (80186) ENTER/LEAVE */
	uint8_t	bound;						/* (80186) BOUND */
};

//static const uint8_t _i80x86_parity_table[256] = {
static const uint8_t parity_table[256] = {
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

static const struct i80x86_timing _i80x86_timing_tbl_i8086 = {
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
/* these come from the Intel 80186 datasheet */
static const struct i80x86_timing _i80x86_timing_tbl_i80186 = {
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

// Disassembler constants
const UINT32 DASMFLAG_SUPPORTED     = 0x80000000;   // are disassembly flags supported?
const UINT32 DASMFLAG_STEP_OUT      = 0x40000000;   // this instruction should be the end of a step out sequence
const UINT32 DASMFLAG_STEP_OVER     = 0x20000000;   // this instruction should be stepped over by setting a breakpoint afterwards
const UINT32 DASMFLAG_OVERINSTMASK  = 0x18000000;   // number of extra instructions to skip when stepping over
const UINT32 DASMFLAG_OVERINSTSHIFT = 27;           // bits to shift after masking to get the value
const UINT32 DASMFLAG_LENGTHMASK    = 0x0000ffff;   // the low 16-bits contain the actual length

#endif
