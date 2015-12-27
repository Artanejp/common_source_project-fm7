/*
   NEC V-series Disassembler

   Originally Written for i386 by Ville Linde
   Converted to NEC-V by Aaron Giles
*/

//#include "emu.h"

struct nec_config
{
	const UINT8*	v25v35_decryptiontable; // internal decryption table
};

/* default configuration */
static const nec_config default_config =
{
	NULL
};

static const nec_config *Iconfig;

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
	_TCHAR mnemonic[32];
	UINT32 flags;
	UINT32 param1;
	UINT32 param2;
	UINT32 param3;
	offs_t dasm_flags;
};

struct GROUP_OP {
	_TCHAR mnemonic[32];
	const I386_OPCODE *opcode;
};

static const UINT8 *opcode_ptr;
static const UINT8 *opcode_ptr_base;

static const I386_OPCODE necv_opcode_table1[256] =
{
	// 0x00
	{_T("add"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("add"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("add"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("add"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("add"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("add"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("push    ds1"),		0,				0,					0,					0				},
	{_T("pop     ds1"),		0,				0,					0,					0				},
	{_T("or"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("or"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("or"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("or"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("or"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("or"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("push    ps"),		0,				0,					0,					0				},
	{_T("two_byte"),		TWO_BYTE,		0,					0,					0				},
	// 0x10
	{_T("addc"),			MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("addc"),			MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("addc"),			MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("addc"),			MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("addc"),			0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("addc"),			0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("push    ss"),		0,				0,					0,					0				},
	{_T("pop     ss"),		0,				0,					0,					0				},
	{_T("subc"),			MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("subc"),			MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("subc"),			MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("subc"),			MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("subc"),			0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("subc"),			0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("push    ds0"),		0,				0,					0,					0				},
	{_T("pop     ds0"),		0,				0,					0,					0				},
	// 0x20
	{_T("and"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("and"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("and"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("and"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("and"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("and"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("ds1:"),			SEG_DS1,		0,					0,					0				},
	{_T("adj4a"),			0,				0,					0,					0				},
	{_T("sub"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("sub"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("sub"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("sub"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("sub"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("sub"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("ps:"),				SEG_PS,			0,					0,					0				},
	{_T("adj4s"),			0,				0,					0,					0				},
	// 0x30
	{_T("xor"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("xor"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("xor"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("xor"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("xor"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("xor"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("ss:"),				SEG_SS,			0,					0,					0				},
	{_T("adjba"),			0,				0,					0,					0				},
	{_T("cmp"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("cmp"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("cmp"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("cmp"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("cmp"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("cmp"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("ds0:"),			SEG_DS0,		0,					0,					0				},
	{_T("adjbs"),			0,				0,					0,					0				},
	// 0x40
	{_T("inc"),				0,				PARAM_AW,			0,					0				},
	{_T("inc"),				0,				PARAM_CW,			0,					0				},
	{_T("inc"),				0,				PARAM_DW,			0,					0				},
	{_T("inc"),				0,				PARAM_BW,			0,					0				},
	{_T("inc"),				0,				PARAM_SP,			0,					0				},
	{_T("inc"),				0,				PARAM_BP,			0,					0				},
	{_T("inc"),				0,				PARAM_IX,			0,					0				},
	{_T("inc"),				0,				PARAM_IY,			0,					0				},
	{_T("dec"),				0,				PARAM_AW,			0,					0				},
	{_T("dec"),				0,				PARAM_CW,			0,					0				},
	{_T("dec"),				0,				PARAM_DW,			0,					0				},
	{_T("dec"),				0,				PARAM_BW,			0,					0				},
	{_T("dec"),				0,				PARAM_SP,			0,					0				},
	{_T("dec"),				0,				PARAM_BP,			0,					0				},
	{_T("dec"),				0,				PARAM_IX,			0,					0				},
	{_T("dec"),				0,				PARAM_IY,			0,					0				},
	// 0x50
	{_T("push"),			0,				PARAM_AW,			0,					0				},
	{_T("push"),			0,				PARAM_CW,			0,					0				},
	{_T("push"),			0,				PARAM_DW,			0,					0				},
	{_T("push"),			0,				PARAM_BW,			0,					0				},
	{_T("push"),			0,				PARAM_SP,			0,					0				},
	{_T("push"),			0,				PARAM_BP,			0,					0				},
	{_T("push"),			0,				PARAM_IX,			0,					0				},
	{_T("push"),			0,				PARAM_IY,			0,					0				},
	{_T("pop"),				0,				PARAM_AW,			0,					0				},
	{_T("pop"),				0,				PARAM_CW,			0,					0				},
	{_T("pop"),				0,				PARAM_DW,			0,					0				},
	{_T("pop"),				0,				PARAM_BW,			0,					0				},
	{_T("pop"),				0,				PARAM_SP,			0,					0				},
	{_T("pop"),				0,				PARAM_BP,			0,					0				},
	{_T("pop"),				0,				PARAM_IX,			0,					0				},
	{_T("pop"),				0,				PARAM_IY,			0,					0				},
	// 0x60
	{_T("push    r"),		0,				0,					0,					0				},
	{_T("pop     r"),		0,				0,					0,					0				},
	{_T("chkind"),			MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("brkn"),			0,				PARAM_UI8,			0,					0,				DASMFLAG_STEP_OVER},	/* V25S/V35S only */
	{_T("repnc"),			PREFIX,			0,					0,					0				},
	{_T("repc"),			PREFIX,			0,					0,					0				},
	{_T("fpo2    0"),		0,				0,					0,					0				},	/* for a coprocessor that was never made */
	{_T("fpo2    1"),		0,				0,					0,					0				},	/* for a coprocessor that was never made */
	{_T("push"),			0,				PARAM_IMM,			0,					0				},
	{_T("mul"),				MODRM,			PARAM_REG16,		PARAM_RM16,			PARAM_IMM		},
	{_T("push"),			0,				PARAM_I8,			0,					0				},
	{_T("mul"),				MODRM,			PARAM_REG16,		PARAM_RM16,			PARAM_I8		},
	{_T("inmb"),			0,				0,					0,					0				},
	{_T("inmw"),			0,				0,					0,					0				},
	{_T("outmb"),			0,				0,					0,					0				},
	{_T("outmw"),			0,				0,					0,					0				},
	// 0x70
	{_T("bv"),				0,				PARAM_REL8,			0,					0				},
	{_T("bnv"),				0,				PARAM_REL8,			0,					0				},
	{_T("bc"),				0,				PARAM_REL8,			0,					0				},
	{_T("bnc"),				0,				PARAM_REL8,			0,					0				},
	{_T("be"),				0,				PARAM_REL8,			0,					0				},
	{_T("bne"),				0,				PARAM_REL8,			0,					0				},
	{_T("bnh"),				0,				PARAM_REL8,			0,					0				},
	{_T("bh"),				0,				PARAM_REL8,			0,					0				},
	{_T("bn"),				0,				PARAM_REL8,			0,					0				},
	{_T("bp"),				0,				PARAM_REL8,			0,					0				},
	{_T("bpe"),				0,				PARAM_REL8,			0,					0				},
	{_T("bpo"),				0,				PARAM_REL8,			0,					0				},
	{_T("blt"),				0,				PARAM_REL8,			0,					0				},
	{_T("bge"),				0,				PARAM_REL8,			0,					0				},
	{_T("ble"),				0,				PARAM_REL8,			0,					0				},
	{_T("bgt"),				0,				PARAM_REL8,			0,					0				},
	// 0x80
	{_T("immb"),			GROUP,			0,					0,					0				},
	{_T("immw"),			GROUP,			0,					0,					0				},
	{_T("immb"),			GROUP,			0,					0,					0				},
	{_T("immws"),			GROUP,			0,					0,					0				},
	{_T("test"),			MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("test"),			MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("xch"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("xch"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("mov"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("mov"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("mov"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("mov"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("mov"),				MODRM,			PARAM_RM16,			PARAM_SREG,			0				},
	{_T("ldea"),			MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("mov"),				MODRM,			PARAM_SREG,			PARAM_RM16,			0				},
	{_T("pop"),				MODRM,			PARAM_RM16,			0,					0				},
	// 0x90
	{_T("nop"),				0,				0,					0,					0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_CW,			0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_DW,			0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_BW,			0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_SP,			0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_BP,			0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_IX,			0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_IY,			0				},
	{_T("cvtbw"),			0,				0,					0,					0				},
	{_T("cvtwl"),			0,				0,					0,					0				},
	{_T("call"),			0,				PARAM_ADDR,			0,					0,				DASMFLAG_STEP_OVER},
	{_T("poll"),			0,				0,					0,					0				},
	{_T("push    psw"),		0,				0,					0,					0				},
	{_T("pop     psw"),		0,				0,					0,					0				},
	{_T("mov     psw,ah"),	0,				0,					0,					0				},
	{_T("mov     ah,psw"),	0,				0,					0,					0				},
	// 0xa0
	{_T("mov"),				0,				PARAM_AL,			PARAM_MEM_OFFS,		0				},
	{_T("mov"),				0,				PARAM_AW,			PARAM_MEM_OFFS,		0				},
	{_T("mov"),				0,				PARAM_MEM_OFFS,		PARAM_AL,			0				},
	{_T("mov"),				0,				PARAM_MEM_OFFS,		PARAM_AW,			0				},
	{_T("movbkb"),			0,				0,					0,					0				},
	{_T("movbkw"),			0,				0,					0,					0				},
	{_T("cmpbkb"),			0,				0,					0,					0				},
	{_T("cmpbkw"),			0,				0,					0,					0				},
	{_T("test"),			0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("test"),			0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("stmb"),			0,				0,					0,					0				},
	{_T("stmw"),			0,				0,					0,					0				},
	{_T("ldmb"),			0,				0,					0,					0				},
	{_T("ldmw"),			0,				0,					0,					0				},
	{_T("cmpmb"),			0,				0,					0,					0				},
	{_T("cmpmw"),			0,				0,					0,					0				},
	// 0xb0
	{_T("mov"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_CL,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_DL,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_BL,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_AH,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_CH,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_DH,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_BH,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_CW,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_DW,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_BW,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_SP,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_BP,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_IX,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_IY,			PARAM_IMM,			0				},
	// 0xc0
	{_T("shiftbi"),			GROUP,			0,					0,					0				},
	{_T("shiftwi"),			GROUP,			0,					0,					0				},
	{_T("ret"),				0,				PARAM_I16,			0,					0,				DASMFLAG_STEP_OUT},
	{_T("ret"),				0,				0,					0,					0,				DASMFLAG_STEP_OUT},
	{_T("mov     ds1,"),	MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("mov     ds0,"),	MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("mov"),				MODRM,			PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("mov"),				MODRM,			PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("prepare"),			0,				PARAM_I16,			PARAM_UI8,			0				},
	{_T("dispose"),			0,				0,					0,					0				},
	{_T("retf"),			0,				PARAM_I16,			0,					0,				DASMFLAG_STEP_OUT},
	{_T("retf"),			0,				0,					0,					0,				DASMFLAG_STEP_OUT},
	{_T("brk     3"),		0,				0,					0,					0,				DASMFLAG_STEP_OVER},
	{_T("brk"),				0,				PARAM_UI8,			0,					0,				DASMFLAG_STEP_OVER},
	{_T("brkv"),			0,				0,					0,					0				},
	{_T("reti"),			0,				0,					0,					0,				DASMFLAG_STEP_OUT},
	// 0xd0
	{_T("shiftb"),			GROUP,			0,					0,					0				},
	{_T("shiftw"),			GROUP,			0,					0,					0				},
	{_T("shiftbv"),			GROUP,			0,					0,					0				},
	{_T("shiftwv"),			GROUP,			0,					0,					0				},
	{_T("cvtbd"),			0,				PARAM_I8,			0,					0				},
	{_T("cvtdb"),			0,				PARAM_I8,			0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("trans"),			0,				0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	// 0xe0
	{_T("dbnzne"),			0,				PARAM_REL8,			0,					0,				DASMFLAG_STEP_OVER},
	{_T("dbnze"),			0,				PARAM_REL8,			0,					0,				DASMFLAG_STEP_OVER},
	{_T("dbnz"),			0,				PARAM_REL8,			0,					0,				DASMFLAG_STEP_OVER},
	{_T("bcwz"),			0,				PARAM_REL8,			0,					0				},
	{_T("in"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("in"),				0,				PARAM_AW,			PARAM_UI8,			0				},
	{_T("out"),				0,				PARAM_UI8,			PARAM_AL,			0				},
	{_T("out"),				0,				PARAM_UI8,			PARAM_AW,			0				},
	{_T("call"),			0,				PARAM_REL16,		0,					0,				DASMFLAG_STEP_OVER},
	{_T("br"),				0,				PARAM_REL16,		0,					0				},
	{_T("br"),				0,				PARAM_ADDR,			0,					0				},
	{_T("br"),				0,				PARAM_REL8,			0,					0				},
	{_T("in"),				0,				PARAM_AL,			PARAM_DW,			0				},
	{_T("in"),				0,				PARAM_AW,			PARAM_DW,			0				},
	{_T("out"),				0,				PARAM_DW,			PARAM_AL,			0				},
	{_T("out"),				0,				PARAM_DW,			PARAM_AW,			0				},
	// 0xf0
	{_T("buslock"),			PREFIX,			0,					0,					0				},
	{_T("brks"),			0,				PARAM_UI8,			0,					0,				DASMFLAG_STEP_OVER},	/* V25S/V35S only */
	{_T("repne"),			PREFIX,			0,					0,					0				},
	{_T("rep"),				PREFIX,			0,					0,					0				},
	{_T("halt"),			0,				0,					0,					0				},
	{_T("not1    cy"),		0,				0,					0,					0				},
	{_T("group1b"),			GROUP,			0,					0,					0				},
	{_T("group1w"),			GROUP,			0,					0,					0				},
	{_T("clr1    cy"),		0,				0,					0,					0				},
	{_T("set1    cy"),		0,				0,					0,					0				},
	{_T("di"),				0,				0,					0,					0				},
	{_T("ei"),				0,				0,					0,					0				},
	{_T("clr1    dir"),		0,				0,					0,					0				},
	{_T("set1    dir"),		0,				0,					0,					0				},
	{_T("group2b"),			GROUP,			0,					0,					0				},
	{_T("group2w"),			GROUP,			0,					0,					0				}
};

static const I386_OPCODE necv_opcode_table2[256] =
{
	// 0x00
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x10
	{_T("test1"),			MODRM,			PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("test1"),			MODRM,			PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("clr1"),			MODRM,			PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("clr1"),			MODRM,			PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("set1"),			MODRM,			PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("set1"),			MODRM,			PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("not1"),			MODRM,			PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("not1"),			MODRM,			PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("test1"),			MODRM,			PARAM_RMPTR8,		PARAM_I3,			0				},
	{_T("test1"),			MODRM,			PARAM_RMPTR16,		PARAM_I4,			0				},
	{_T("clr1"),			MODRM,			PARAM_RMPTR8,		PARAM_I3,			0				},
	{_T("clr1"),			MODRM,			PARAM_RMPTR16,		PARAM_I4,			0				},
	{_T("set1"),			MODRM,			PARAM_RMPTR8,		PARAM_I3,			0				},
	{_T("set1"),			MODRM,			PARAM_RMPTR16,		PARAM_I4,			0				},
	{_T("not1"),			MODRM,			PARAM_RMPTR8,		PARAM_I3,			0				},
	{_T("not1"),			MODRM,			PARAM_RMPTR16,		PARAM_I4,			0				},
	// 0x20
	{_T("add4s"),			0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("sub4s"),			0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("movspa"),			0,				0,					0,					0				},	/* V25/V35 only */
	{_T("cmp4s"),			0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("rol4"),			MODRM,			PARAM_RMPTR8,		0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("ror4"),			MODRM,			PARAM_RMPTR8,		0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("brkcs"),			MODRM,			PARAM_REG2_16,		0,					0,				DASMFLAG_STEP_OVER},	/* V25/V35 only */
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x30
	{_T("???"),				0,				0,					0,					0				},
	{_T("ins"),				MODRM,			PARAM_REG2_8,		PARAM_REG8,			0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("ext"),				MODRM,			PARAM_REG2_8,		PARAM_REG8,			0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("ins"),				MODRM,			PARAM_REG2_8,		PARAM_I4,			0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("ext"),				MODRM,			PARAM_REG2_8,		PARAM_I4,			0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x40
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x50
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x60
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x70
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x80
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x90
	{_T("???"),				0,				0,					0,					0				},
	{_T("retrbi"),			0,				0,					0,					0				},	/* V25/V35 only */
	{_T("fint"),			0,				0,					0,					0				},	/* V25/V35 only */
	{_T("???"),				0,				0,					0,					0				},
	{_T("tsksw"),			MODRM,			PARAM_REG2_16,		0,					0				},	/* V25/V35 only */
	{_T("movspb"),			MODRM,			PARAM_REG2_16,		0,					0				},	/* V25/V35 only */
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("btclr"),			0,				PARAM_SFREG,		PARAM_I3,			PARAM_REL8		},	/* V25/V35 only */
	{_T("???"),				0,				0,					0,					0				},
	{_T("stop"),			0,				0,					0,					0				},	/* V25/V35 only */
	{_T("???"),				0,				0,					0,					0				},
	// 0xa0
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0xb0
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0xc0
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0xd0
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0xe0
	{_T("brkxa"),			0,				PARAM_UI8,			0,					0				},	/* V33,53 only */
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0xf0
	{_T("retxa"),			0,				PARAM_UI8,			0,					0				},	/* V33,53 only */
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("brkem"),			0,				PARAM_UI8,			0,					0				}	/* V20,30,40,50 only */
};

static const I386_OPCODE immb_table[8] =
{
	{_T("add"),				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("or"),				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("addc"),			0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("subc"),			0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("and"),				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("sub"),				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("xor"),				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("cmp"),				0,				PARAM_RMPTR8,		PARAM_UI8,			0				}
};

static const I386_OPCODE immw_table[8] =
{
	{_T("add"),				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("or"),				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("addc"),			0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("subc"),			0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("and"),				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("sub"),				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("xor"),				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("cmp"),				0,				PARAM_RMPTR16,		PARAM_IMM,			0				}
};

static const I386_OPCODE immws_table[8] =
{
	{_T("add"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("or"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("addc"),			0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("subc"),			0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("and"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("sub"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("xor"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("cmp"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				}
};

static const I386_OPCODE shiftbi_table[8] =
{
	{_T("rol"),				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("ror"),				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("rolc"),			0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("rorc"),			0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("shl"),				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("shr"),				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("???"),				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("shra"),			0,				PARAM_RMPTR8,		PARAM_I8,			0				}
};

static const I386_OPCODE shiftwi_table[8] =
{
	{_T("rol"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("ror"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("rolc"),			0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("rorc"),			0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("shl"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("shr"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("???"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("shra"),			0,				PARAM_RMPTR16,		PARAM_I8,			0				}
};

static const I386_OPCODE shiftb_table[8] =
{
	{_T("rol"),				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("ror"),				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("rolc"),			0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("rorc"),			0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("shl"),				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("shr"),				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("???"),				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("shra"),			0,				PARAM_RMPTR8,		PARAM_1,			0				}
};

static const I386_OPCODE shiftw_table[8] =
{
	{_T("rol"),				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("ror"),				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("rolc"),			0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("rorc"),			0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("shl"),				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("shr"),				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("???"),				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("shra"),			0,				PARAM_RMPTR16,		PARAM_1,			0				}
};

static const I386_OPCODE shiftbv_table[8] =
{
	{_T("rol"),				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("ror"),				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("rolc"),			0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("rorc"),			0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("shl"),				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("shr"),				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("???"),				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("shra"),			0,				PARAM_RMPTR8,		PARAM_CL,			0				}
};

static const I386_OPCODE shiftwv_table[8] =
{
	{_T("rol"),				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("ror"),				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("rolc"),			0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("rorc"),			0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("shl"),				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("shr"),				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("???"),				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("shra"),			0,				PARAM_RMPTR16,		PARAM_CL,			0				}
};

static const I386_OPCODE group1b_table[8] =
{
	{_T("test"),			0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("not"),				0,				PARAM_RMPTR8,		0,					0				},
	{_T("neg"),				0,				PARAM_RMPTR8,		0,					0				},
	{_T("mulu"),			0,				PARAM_RMPTR8,		0,					0				},
	{_T("mul"),				0,				PARAM_RMPTR8,		0,					0				},
	{_T("divu"),			0,				PARAM_RMPTR8,		0,					0				},
	{_T("div"),				0,				PARAM_RMPTR8,		0,					0				}
};

static const I386_OPCODE group1w_table[8] =
{
	{_T("test"),			0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("not"),				0,				PARAM_RMPTR16,		0,					0				},
	{_T("neg"),				0,				PARAM_RMPTR16,		0,					0				},
	{_T("mulu"),			0,				PARAM_RMPTR16,		0,					0				},
	{_T("mul"),				0,				PARAM_RMPTR16,		0,					0				},
	{_T("divu"),			0,				PARAM_RMPTR16,		0,					0				},
	{_T("div"),				0,				PARAM_RMPTR16,		0,					0				}
};

static const I386_OPCODE group2b_table[8] =
{
	{_T("inc"),				0,				PARAM_RMPTR8,		0,					0				},
	{_T("dec"),				0,				PARAM_RMPTR8,		0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				}
};

static const I386_OPCODE group2w_table[8] =
{
	{_T("inc"),				0,				PARAM_RMPTR16,		0,					0				},
	{_T("dec"),				0,				PARAM_RMPTR16,		0,					0				},
	{_T("call"),			0,				PARAM_RMPTR16,		0,					0,				DASMFLAG_STEP_OVER},
	{_T("call    far ptr "),0,				PARAM_RM16,			0,					0,				DASMFLAG_STEP_OVER},
	{_T("br"),				0,				PARAM_RMPTR16,		0,					0				},
	{_T("br      far ptr "),0,				PARAM_RM16,			0,					0				},
	{_T("push"),			0,				PARAM_RMPTR16,		0,					0				},
	{_T("???"),				0,				0,					0,					0				}
};

static const GROUP_OP group_op_table[] =
{
	{_T("immb"),				immb_table				},
	{_T("immw"),				immw_table				},
	{_T("immws"),				immws_table				},
	{_T("shiftbi"),			shiftbi_table			},
	{_T("shiftwi"),			shiftwi_table			},
	{_T("shiftb"),				shiftb_table			},
	{_T("shiftw"),				shiftw_table			},
	{_T("shiftbv"),			shiftbv_table			},
	{_T("shiftwv"),			shiftwv_table			},
	{_T("group1b"),			group1b_table			},
	{_T("group1w"),			group1w_table			},
	{_T("group2b"),			group2b_table			},
	{_T("group2w"),			group2w_table			}
};



static const _TCHAR *const nec_reg[8] = { _T("aw"), _T("cw"), _T("dw"), _T("bw"), _T("sp"), _T("bp"), _T("ix"), _T("iy") };
static const _TCHAR *const nec_reg8[8] = { _T("al"), _T("cl"), _T("dl"), _T("bl"), _T("ah"), _T("ch"), _T("dh"), _T("bh") };
static const _TCHAR *const nec_sreg[8] = { _T("ds1"), _T("ps"), _T("ss"), _T("ds0"), _T("???"), _T("???"), _T("???"), _T("???") };
static const _TCHAR *const nec_sfreg[256] =
{
	/* 0x00 */
	_T("p0"),	_T("pm0"),	_T("pmc0"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("p1"),	_T("pm1"),	_T("pmc1"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0x10 */
	_T("p2"),	_T("pm2"),	_T("pmc2"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0x20 */
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0x30 */
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("pt"),	_T("???"),	_T("???"),	_T("pmt"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0x40 */
	_T("intm"),	_T("???"),	_T("???"),	_T("???"),	_T("ems0"),	_T("ems1"),	_T("ems2"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("exic0"),	_T("exic1"),	_T("exic2"),	_T("???"),
	/* 0x50 */
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0x60 */
	_T("rxb0"),	_T("???"),	_T("txb0"),	_T("???"),	_T("???"),	_T("srms0"),	_T("stms0"),	_T("???"),
	_T("scm0"),	_T("scc0"),	_T("brg0"),	_T("scs0"),	_T("seic0"),	_T("sric0"),	_T("stic0"),	_T("???"),
	/* 0x70 */
	_T("rxb1"),	_T("???"),	_T("txb1"),	_T("???"),	_T("???"),	_T("srms1"),	_T("stms1"),	_T("???"),
	_T("scm1"),	_T("scc1"),	_T("brg1"),	_T("scs1"),	_T("seic1"),	_T("sric1"),	_T("stic1"),	_T("???"),
	/* 0x80 */
	_T("tm0"),	_T("???"),	_T("md0"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("tm1"),	_T("???"),	_T("md1"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0x90 */
	_T("tmc0"),	_T("tmc1"),	_T("???"),	_T("???"),	_T("tmms0"),	_T("tmms1"),	_T("tmms2"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("tmic0"),	_T("tmic1"),	_T("tmic2"),	_T("???"),
	/* 0xa0 */
	_T("dmac0"),	_T("dmam0"),	_T("dmac1"),	_T("dmam1"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("dic0"),	_T("dic1"),	_T("???"),	_T("???"),
	/* 0xb0 */
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0xc0 */
	_T("sar0l"),	_T("sar0m"),	_T("sar0h"),	_T("???"),	_T("dar0l"),	_T("dar0m"),	_T("dar0h"),	_T("???"),
	_T("tc0l"),	_T("tc0h"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0xd0 */
	_T("sar1l"),	_T("sar1m"),	_T("sar1h"),	_T("???"),	_T("dar1l"),	_T("dar1m"),	_T("dar1h"),	_T("???"),
	_T("tc1l"),	_T("tc1h"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0xe0 */
	_T("stbc"),	_T("rfm"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("wtc"),	_T("???"),	_T("flag"),	_T("prc"),	_T("tbic"),	_T("???"),	_T("???"),	_T("irqs"),
	/* 0xf0 */
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("ispr"),	_T("???"),	_T("???"),	_T("idb")
};

static UINT32 pc;
static UINT8 modrm;
static UINT32 segment;
static offs_t dasm_flags;
static _TCHAR modrm_string[256];

#define MODRM_REG1	((modrm >> 3) & 0x7)
#define MODRM_REG2	(modrm & 0x7)

#define MAX_LENGTH	8

INLINE UINT8 _FETCH(void)
{
	if ((opcode_ptr - opcode_ptr_base) + 1 > MAX_LENGTH)
		return 0xff;
	pc++;
	return *opcode_ptr++;
}

INLINE UINT16 _FETCH16(void)
{
	UINT16 d;
	if ((opcode_ptr - opcode_ptr_base) + 2 > MAX_LENGTH)
		return 0xffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8);
	opcode_ptr += 2;
	pc += 2;
	return d;
}

INLINE UINT8 _FETCHD(void)
{
	if ((opcode_ptr - opcode_ptr_base) + 1 > MAX_LENGTH)
		return 0xff;
	pc++;
	return *opcode_ptr++;
}

INLINE UINT16 _FETCHD16(void)
{
	UINT16 d;
	if ((opcode_ptr - opcode_ptr_base) + 2 > MAX_LENGTH)
		return 0xffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8);
	opcode_ptr += 2;
	pc += 2;
	return d;
}

static _TCHAR *hexstring(UINT32 value, int digits)
{
	static _TCHAR buffer[20];
	buffer[0] = _T('0');
	if (digits)
		_stprintf(&buffer[1], _T("%0*Xh"), digits, value);
	else
		_stprintf(&buffer[1], _T("%Xh"), value);
	return (buffer[1] >= _T('0') && buffer[1] <= _T('9')) ? &buffer[1] : &buffer[0];
}

static _TCHAR *shexstring(UINT32 value, int digits, int always)
{
	static _TCHAR buffer[20];
	if (value >= 0x80000000)
		_stprintf(buffer, _T("-%s"), hexstring(-value, digits));
	else if (always)
		_stprintf(buffer, _T("+%s"), hexstring(value, digits));
	else
		return hexstring(value, digits);
	return buffer;
}

static void handle_modrm(_TCHAR* s)
{
	INT8 disp8;
	INT16 disp16;
	UINT8 mod, rm;

	modrm = _FETCHD();
	mod = (modrm >> 6) & 0x3;
	rm = (modrm & 0x7);

	if( modrm >= 0xc0 )
		return;

	switch(segment)
	{
		case SEG_PS: s += _stprintf( s, _T("ps:") ); break;
		case SEG_DS0: s += _stprintf( s, _T("ds0:") ); break;
		case SEG_DS1: s += _stprintf( s, _T("ds1:") ); break;
		case SEG_SS: s += _stprintf( s, _T("ss:") ); break;
	}

	s += _stprintf( s, _T("[") );
	switch( rm )
	{
		case 0: s += _stprintf( s, _T("bw+ix") ); break;
		case 1: s += _stprintf( s, _T("bw+iy") ); break;
		case 2: s += _stprintf( s, _T("bp+ix") ); break;
		case 3: s += _stprintf( s, _T("bp+iy") ); break;
		case 4: s += _stprintf( s, _T("ix") ); break;
		case 5: s += _stprintf( s, _T("iy") ); break;
		case 6:
			if( mod == 0 ) {
				disp16 = _FETCHD16();
				s += _stprintf( s, _T("%s"), hexstring((unsigned) (UINT16) disp16, 0) );
			} else {
				s += _stprintf( s, _T("bp") );
			}
			break;
		case 7: s += _stprintf( s, _T("bw") ); break;
	}
	if( mod == 1 ) {
		disp8 = _FETCHD();
		s += _stprintf( s, _T("%s"), shexstring((INT32)disp8, 0, TRUE) );
	} else if( mod == 2 ) {
		disp16 = _FETCHD16();
		s += _stprintf( s, _T("%s"), shexstring((INT32)disp16, 0, TRUE) );
	}
	s += _stprintf( s, _T("]") );
}

static _TCHAR* handle_param(_TCHAR* s, UINT32 param)
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
			s += _stprintf( s, _T("%s"), nec_reg8[MODRM_REG1] );
			break;

		case PARAM_REG16:
			s += _stprintf( s, _T("%s"), nec_reg[MODRM_REG1] );
			break;

		case PARAM_REG2_8:
			s += _stprintf( s, _T("%s"), nec_reg8[MODRM_REG2] );
			break;

		case PARAM_REG2_16:
			s += _stprintf( s, _T("%s"), nec_reg[MODRM_REG2] );
			break;

		case PARAM_RM8:
		case PARAM_RMPTR8:
			if( modrm >= 0xc0 ) {
				s += _stprintf( s, _T("%s"), nec_reg8[MODRM_REG2] );
			} else {
				if (param == PARAM_RMPTR8)
					s += _stprintf( s, _T("byte ptr ") );
				s += _stprintf( s, _T("%s"), modrm_string );
			}
			break;

		case PARAM_RM16:
		case PARAM_RMPTR16:
			if( modrm >= 0xc0 ) {
				s += _stprintf( s, _T("%s"), nec_reg[MODRM_REG2] );
			} else {
				if (param == PARAM_RMPTR16)
					s += _stprintf( s, _T("word ptr ") );
				s += _stprintf( s, _T("%s"), modrm_string );
			}
			break;

		case PARAM_I3:
			i8 = _FETCHD();
			s += _stprintf( s, _T("%d"), i8 & 0x07 );
			break;

		case PARAM_I4:
			i8 = _FETCHD();
			s += _stprintf( s, _T("%d"), i8 & 0x0f );
			break;

		case PARAM_I8:
			i8 = _FETCHD();
			s += _stprintf( s, _T("%s"), shexstring((INT8)i8, 0, FALSE) );
			break;

		case PARAM_I16:
			i16 = _FETCHD16();
			s += _stprintf( s, _T("%s"), shexstring((INT16)i16, 0, FALSE) );
			break;

		case PARAM_UI8:
			i8 = _FETCHD();
			s += _stprintf( s, _T("%s"), shexstring((UINT8)i8, 0, FALSE) );
			break;

		case PARAM_IMM:
			i16 = _FETCHD16();
			s += _stprintf( s, _T("%s"), hexstring(i16, 0) );
			break;

		case PARAM_ADDR:
			addr = _FETCHD16();
			ptr = _FETCHD16();
			s += _stprintf( s, _T("%s:"), hexstring(ptr, 4) );
			s += _stprintf( s, _T("%s"), hexstring(addr, 0) );
			break;

		case PARAM_REL16:
			/* make sure to keep the relative offset within the segment */
			d16 = _FETCHD16();
			s += _stprintf( s, _T("%s"), hexstring((pc & 0xFFFF0000) | ((pc + d16) & 0x0000FFFF), 0) );
			break;

		case PARAM_REL8:
			d8 = _FETCHD();
			s += _stprintf( s, _T("%s"), hexstring(pc + d8, 0) );
			break;

		case PARAM_MEM_OFFS:
			switch(segment)
			{
				case SEG_PS: s += _stprintf( s, _T("ps:") ); break;
				case SEG_DS0: s += _stprintf( s, _T("ds0:") ); break;
				case SEG_DS1: s += _stprintf( s, _T("ds1:") ); break;
				case SEG_SS: s += _stprintf( s, _T("ss:") ); break;
			}

			i16 = _FETCHD16();
			s += _stprintf( s, _T("[%s]"), hexstring(i16, 0) );
			break;

		case PARAM_SREG:
			s += _stprintf( s, _T("%s"), nec_sreg[MODRM_REG1] );
			break;

		case PARAM_SFREG:
			i8 = _FETCHD();
			s += _stprintf( s, _T("%s"), nec_sfreg[i8] );
			break;

		case PARAM_1:
			s += _stprintf( s, _T("1") );
			break;

		case PARAM_AL: s += _stprintf( s, _T("al") ); break;
		case PARAM_CL: s += _stprintf( s, _T("cl") ); break;
		case PARAM_DL: s += _stprintf( s, _T("dl") ); break;
		case PARAM_BL: s += _stprintf( s, _T("bl") ); break;
		case PARAM_AH: s += _stprintf( s, _T("ah") ); break;
		case PARAM_CH: s += _stprintf( s, _T("ch") ); break;
		case PARAM_DH: s += _stprintf( s, _T("dh") ); break;
		case PARAM_BH: s += _stprintf( s, _T("bh") ); break;

		case PARAM_AW: s += _stprintf( s, _T("aw") ); break;
		case PARAM_CW: s += _stprintf( s, _T("cw") ); break;
		case PARAM_DW: s += _stprintf( s, _T("dw") ); break;
		case PARAM_BW: s += _stprintf( s, _T("bw") ); break;
		case PARAM_SP: s += _stprintf( s, _T("sp") ); break;
		case PARAM_BP: s += _stprintf( s, _T("bp") ); break;
		case PARAM_IX: s += _stprintf( s, _T("ix") ); break;
		case PARAM_IY: s += _stprintf( s, _T("iy") ); break;
	}
	return s;
}

static void handle_fpu(_TCHAR *s, UINT8 op1, UINT8 op2)
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
					case 0: _stprintf(s, _T("fadd    dword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("fmul    dword ptr %s"), modrm_string); break;
					case 2: _stprintf(s, _T("fcom    dword ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("fcomp   dword ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("fsub    dword ptr %s"), modrm_string); break;
					case 5: _stprintf(s, _T("fsubr   dword ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("fdiv    dword ptr %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fdivr   dword ptr %s"), modrm_string); break;
				}
			}
			else
			{
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fadd    st(0),st(%d)"), op2 & 0x7); break;
					case 1: _stprintf(s, _T("fcom    st(0),st(%d)"), op2 & 0x7); break;
					case 2: _stprintf(s, _T("fsub    st(0),st(%d)"), op2 & 0x7); break;
					case 3: _stprintf(s, _T("fdiv    st(0),st(%d)"), op2 & 0x7); break;
					case 4: _stprintf(s, _T("fmul    st(0),st(%d)"), op2 & 0x7); break;
					case 5: _stprintf(s, _T("fcomp   st(0),st(%d)"), op2 & 0x7); break;
					case 6: _stprintf(s, _T("fsubr   st(0),st(%d)"), op2 & 0x7); break;
					case 7: _stprintf(s, _T("fdivr   st(0),st(%d)"), op2 & 0x7); break;
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
					case 0: _stprintf(s, _T("fld     dword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("??? (FPU)")); break;
					case 2: _stprintf(s, _T("fst     dword ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("fstp    dword ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("fldenv  word ptr %s"), modrm_string); break;
					case 5: _stprintf(s, _T("fldcw   word ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("fstenv  word ptr %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fstcw   word ptr %s"), modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						_stprintf(s, _T("fld     st(0),st(%d)"), op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						_stprintf(s, _T("fxch    st(0),st(%d)"), op2 & 0x7); break;

					case 0x10: _stprintf(s, _T("fnop")); break;
					case 0x20: _stprintf(s, _T("fchs")); break;
					case 0x21: _stprintf(s, _T("fabs")); break;
					case 0x24: _stprintf(s, _T("ftst")); break;
					case 0x25: _stprintf(s, _T("fxam")); break;
					case 0x28: _stprintf(s, _T("fld1")); break;
					case 0x29: _stprintf(s, _T("fldl2t")); break;
					case 0x2a: _stprintf(s, _T("fldl2e")); break;
					case 0x2b: _stprintf(s, _T("fldpi")); break;
					case 0x2c: _stprintf(s, _T("fldlg2")); break;
					case 0x2d: _stprintf(s, _T("fldln2")); break;
					case 0x2e: _stprintf(s, _T("fldz")); break;
					case 0x30: _stprintf(s, _T("f2xm1")); break;
					case 0x31: _stprintf(s, _T("fyl2x")); break;
					case 0x32: _stprintf(s, _T("fptan")); break;
					case 0x33: _stprintf(s, _T("fpatan")); break;
					case 0x34: _stprintf(s, _T("fxtract")); break;
					case 0x35: _stprintf(s, _T("fprem1")); break;
					case 0x36: _stprintf(s, _T("fdecstp")); break;
					case 0x37: _stprintf(s, _T("fincstp")); break;
					case 0x38: _stprintf(s, _T("fprem")); break;
					case 0x39: _stprintf(s, _T("fyl2xp1")); break;
					case 0x3a: _stprintf(s, _T("fsqrt")); break;
					case 0x3b: _stprintf(s, _T("fsincos")); break;
					case 0x3c: _stprintf(s, _T("frndint")); break;
					case 0x3d: _stprintf(s, _T("fscale")); break;
					case 0x3e: _stprintf(s, _T("fsin")); break;
					case 0x3f: _stprintf(s, _T("fcos")); break;

					default: _stprintf(s, _T("??? (FPU)")); break;
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
					case 0: _stprintf(s, _T("fiadd   dword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("fimul   dword ptr %s"), modrm_string); break;
					case 2: _stprintf(s, _T("ficom   dword ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("ficomp  dword ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("fisub   dword ptr %s"), modrm_string); break;
					case 5: _stprintf(s, _T("fisubr  dword ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("fidiv   dword ptr %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fidivr  dword ptr %s"), modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						_stprintf(s, _T("fcmovb  st(0),st(%d)"), op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						_stprintf(s, _T("fcmove  st(0),st(%d)"), op2 & 0x7); break;

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
						_stprintf(s, _T("fcmovbe st(0),st(%d)"), op2 & 0x7); break;

					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
						_stprintf(s, _T("fcmovu  st(0),st(%d)"), op2 & 0x7); break;

					default: _stprintf(s, _T("??? (FPU)")); break;

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
					case 0: _stprintf(s, _T("fild    dword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("??? (FPU)")); break;
					case 2: _stprintf(s, _T("fist    dword ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("fistp   dword ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("??? (FPU)")); break;
					case 5: _stprintf(s, _T("fld     tword ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("??? (FPU)")); break;
					case 7: _stprintf(s, _T("fstp    tword ptr %s"), modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						_stprintf(s, _T("fcmovnb st(0),st(%d)"), op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						_stprintf(s, _T("fcmovne st(0),st(%d)"), op2 & 0x7); break;

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
						_stprintf(s, _T("fcmovnbe st(0),st(%d)"), op2 & 0x7); break;

					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
						_stprintf(s, _T("fcmovnu st(0),st(%d)"), op2 & 0x7); break;

					case 0x22: _stprintf(s, _T("fclex")); break;
					case 0x23: _stprintf(s, _T("finit")); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						_stprintf(s, _T("fucomi  st(0),st(%d)"), op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						_stprintf(s, _T("fcomi   st(0),st(%d)"), op2 & 0x7); break;

					default: _stprintf(s, _T("??? (FPU)")); break;
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
					case 0: _stprintf(s, _T("fadd    qword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("fmul    qword ptr %s"), modrm_string); break;
					case 2: _stprintf(s, _T("fcom    qword ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("fcomp   qword ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("fsub    qword ptr %s"), modrm_string); break;
					case 5: _stprintf(s, _T("fsubr   qword ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("fdiv    qword ptr %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fdivr   qword ptr %s"), modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						_stprintf(s, _T("fadd    st(%d),st(0)"), op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						_stprintf(s, _T("fmul    st(%d),st(0)"), op2 & 0x7); break;

					case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
						_stprintf(s, _T("fsubr   st(%d),st(0)"), op2 & 0x7); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						_stprintf(s, _T("fsub    st(%d),st(0)"), op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						_stprintf(s, _T("fdivr   st(%d),st(0)"), op2 & 0x7); break;

					case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
						_stprintf(s, _T("fdiv    st(%d),st(0)"), op2 & 0x7); break;

					default: _stprintf(s, _T("??? (FPU)")); break;
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
					case 0: _stprintf(s, _T("fld     qword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("??? (FPU)")); break;
					case 2: _stprintf(s, _T("fst     qword ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("fstp    qword ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("frstor  %s"), modrm_string); break;
					case 5: _stprintf(s, _T("??? (FPU)")); break;
					case 6: _stprintf(s, _T("fsave   %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fstsw   word ptr %s"), modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						_stprintf(s, _T("ffree   st(%d)"), op2 & 0x7); break;

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
						_stprintf(s, _T("fst     st(%d)"), op2 & 0x7); break;

					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
						_stprintf(s, _T("fstp    st(%d)"), op2 & 0x7); break;

					case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
						_stprintf(s, _T("fucom   st(%d), st(0)"), op2 & 0x7); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						_stprintf(s, _T("fucomp  st(%d)"), op2 & 0x7); break;

					default: _stprintf(s, _T("??? (FPU)")); break;
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
					case 0: _stprintf(s, _T("fiadd   word ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("fimul   word ptr %s"), modrm_string); break;
					case 2: _stprintf(s, _T("ficom   word ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("ficomp  word ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("fisub   word ptr %s"), modrm_string); break;
					case 5: _stprintf(s, _T("fisubr  word ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("fidiv   word ptr %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fidivr  word ptr %s"), modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						_stprintf(s, _T("faddp   st(%d)"), op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						_stprintf(s, _T("fmulp   st(%d)"), op2 & 0x7); break;

					case 0x19: _stprintf(s, _T("fcompp")); break;

					case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
						_stprintf(s, _T("fsubrp  st(%d)"), op2 & 0x7); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						_stprintf(s, _T("fsubp   st(%d)"), op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						_stprintf(s, _T("fdivrp  st(%d), st(0)"), op2 & 0x7); break;

					case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
						_stprintf(s, _T("fdivp   st(%d)"), op2 & 0x7); break;

					default: _stprintf(s, _T("??? (FPU)")); break;
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
					case 0: _stprintf(s, _T("fild    word ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("??? (FPU)")); break;
					case 2: _stprintf(s, _T("fist    word ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("fistp   word ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("fbld    %s"), modrm_string); break;
					case 5: _stprintf(s, _T("fild    qword ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("fbstp   %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fistp   qword ptr %s"), modrm_string); break;
				}
			}
			else
			{
				switch (op2 & 0x3f)
				{
					case 0x20: _stprintf(s, _T("fstsw   aw")); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						_stprintf(s, _T("fucomip st(%d)"), op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						_stprintf(s, _T("fcomip  st(%d),st(0)"), op2 & 0x7); break;

					default: _stprintf(s, _T("??? (FPU)")); break;
				}
			}
			break;
		}
	}
}

static void decode_opcode(_TCHAR *s, const I386_OPCODE *op, UINT8 op1 )
{
	int i;
	UINT8 op2;

	switch( op->flags )
	{
		case TWO_BYTE:
			op2 = _FETCHD();
			decode_opcode( s, &necv_opcode_table2[op2], op1 );
			return;

		case SEG_PS:
		case SEG_DS0:
		case SEG_DS1:
		case SEG_SS:
			segment = op->flags;
			op2 = _FETCH();
			if (Iconfig->v25v35_decryptiontable) op2 = Iconfig->v25v35_decryptiontable[op2];
			decode_opcode( s, &necv_opcode_table1[op2], op1 );
			return;

		case PREFIX:
			s += _stprintf( s, _T("%-8s"), op->mnemonic );
			op2 = _FETCH();
			if (Iconfig->v25v35_decryptiontable) op2 = Iconfig->v25v35_decryptiontable[op2];
			decode_opcode( s, &necv_opcode_table1[op2], op1 );
			return;

		case GROUP:
			handle_modrm( modrm_string );
			for( i=0; i < ARRAY_LENGTH(group_op_table); i++ ) {
				if( _tcscmp(op->mnemonic, group_op_table[i].mnemonic) == 0 )
				{
					decode_opcode( s, &group_op_table[i].opcode[MODRM_REG1], op1 );
					return;
				}
			}
			goto handle_unknown;

		case FPU:
			op2 = _FETCHD();
			handle_fpu( s, op1, op2);
			return;

		case MODRM:
			handle_modrm( modrm_string );
			break;
	}

	s += _stprintf( s, _T("%-8s"), op->mnemonic );
	dasm_flags = op->dasm_flags;

	if( op->param1 != 0 ) {
		s = handle_param( s, op->param1 );
	}

	if( op->param2 != 0 ) {
		s += _stprintf( s, _T(",") );
		s = handle_param( s, op->param2 );
	}

	if( op->param3 != 0 ) {
		s += _stprintf( s, _T(",") );
		s = handle_param( s, op->param3 );
	}
	return;

handle_unknown:
	_stprintf(s, _T("???"));
}

int necv_dasm_one(_TCHAR *buffer, UINT32 eip, const UINT8 *oprom, const nec_config *_config)
{
	UINT8 op;
	const nec_config *config = _config ? _config : &default_config;
	Iconfig = config;

	opcode_ptr = opcode_ptr_base = oprom;
	pc = eip;
	dasm_flags = 0;
	segment = 0;

	op = _FETCH();

	if (Iconfig->v25v35_decryptiontable) op = Iconfig->v25v35_decryptiontable[op];

	decode_opcode( buffer, &necv_opcode_table1[op], op );
	return (pc-eip) | dasm_flags | DASMFLAG_SUPPORTED;
}

CPU_DISASSEMBLE( nec_generic )
{
	return necv_dasm_one(buffer, pc, oprom, NULL);
}

