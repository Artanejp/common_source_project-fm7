// license:BSD-3-Clause
// copyright-holders:Ville Linde, Peter Ferrie
/*
   i386 Disassembler

   Written by Ville Linde
*/

//#include "emu.h"

enum
{
	PARAM_REG = 1,      /* 16 or 32-bit register */
	PARAM_REG8,         /* 8-bit register */
	PARAM_REG16,        /* 16-bit register */
	PARAM_REG32,        /* 32-bit register */
	PARAM_REG3264,      /* 32-bit or 64-bit register */
	PARAM_REG2_32,      /* 32-bit register */
	PARAM_MMX,          /* MMX register */
	PARAM_MMX2,         /* MMX register in modrm */
	PARAM_XMM,          /* XMM register */
	PARAM_RM,           /* 16 or 32-bit memory or register */
	PARAM_RM8,          /* 8-bit memory or register */
	PARAM_RM16,         /* 16-bit memory or register */
	PARAM_RM32,         /* 32-bit memory or register */
	PARAM_RMPTR,        /* 16 or 32-bit memory or register */
	PARAM_RMPTR8,       /* 8-bit memory or register */
	PARAM_RMPTR16,      /* 16-bit memory or register */
	PARAM_RMPTR32,      /* 32-bit memory or register */
	PARAM_RMXMM,        /* 32 or 64-bit memory or register */
	PARAM_REGORXMM,     /* 32 or 64-bit register or XMM register */
	PARAM_M64,          /* 64-bit memory */
	PARAM_M64PTR,       /* 64-bit memory */
	PARAM_MMXM,         /* 64-bit memory or MMX register */
	PARAM_XMMM,         /* 128-bit memory or XMM register */
	PARAM_I4,           /* 4-bit signed immediate */
	PARAM_I8,           /* 8-bit signed immediate */
	PARAM_I16,          /* 16-bit signed immediate */
	PARAM_UI8,          /* 8-bit unsigned immediate */
	PARAM_UI16,         /* 16-bit unsigned immediate */
	PARAM_IMM,          /* 16 or 32-bit immediate */
	PARAM_IMM64,        /* 16, 32 or 64-bit immediate */
	PARAM_ADDR,         /* 16:16 or 16:32 address */
	PARAM_REL,          /* 16 or 32-bit PC-relative displacement */
	PARAM_REL8,         /* 8-bit PC-relative displacement */
	PARAM_MEM_OFFS,     /* 16 or 32-bit mem offset */
	PARAM_PREIMP,       /* prefix with implicit register */
	PARAM_SREG,         /* segment register */
	PARAM_CREG,         /* control register */
	PARAM_DREG,         /* debug register */
	PARAM_TREG,         /* test register */
	PARAM_1,            /* used by shift/rotate instructions */
	PARAM_AL,
	PARAM_CL,
	PARAM_DL,
	PARAM_BL,
	PARAM_AH,
	PARAM_CH,
	PARAM_DH,
	PARAM_BH,
	PARAM_DX,
	PARAM_EAX,          /* EAX or AX */
	PARAM_ECX,          /* ECX or CX */
	PARAM_EDX,          /* EDX or DX */
	PARAM_EBX,          /* EBX or BX */
	PARAM_ESP,          /* ESP or SP */
	PARAM_EBP,          /* EBP or BP */
	PARAM_ESI,          /* ESI or SI */
	PARAM_EDI,          /* EDI or DI */
	PARAM_XMM0,
	PARAM_XMM64,            /* 64-bit memory or XMM register */
	PARAM_XMM32,            /* 32-bit memory or XMM register */
	PARAM_XMM16,            /* 16-bit memory or XMM register */
};

enum
{
	MODRM = 1,
	GROUP,
	FPU,
	OP_SIZE,
	ADDR_SIZE,
	TWO_BYTE,
	PREFIX,
	SEG_CS,
	SEG_DS,
	SEG_ES,
	SEG_FS,
	SEG_GS,
	SEG_SS,
	ISREX,
	THREE_BYTE          /* [prefix] 0f op1 op2 and then mod/rm */
};

#define FLAGS_MASK          0x0ff
#define VAR_NAME            0x100
#define VAR_NAME4           0x200
#define ALWAYS64            0x400
#define SPECIAL64           0x800
#define SPECIAL64_ENT(x)    (SPECIAL64 | ((x) << 24))
#define GROUP_MOD           0x1000

struct I386_OPCODE {
	const _TCHAR *mnemonic;
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

static const I386_OPCODE i386_opcode_table1[256] =
{
	// 0x00
	{_T("add"),             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{_T("add"),             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("add"),             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{_T("add"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("add"),             0,              PARAM_AL,           PARAM_UI8,          0               },
	{_T("add"),             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{_T("push    es"),      0,              0,                  0,                  0               },
	{_T("pop     es"),      0,              0,                  0,                  0               },
	{_T("or"),              MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{_T("or"),              MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("or"),              MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{_T("or"),              MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("or"),              0,              PARAM_AL,           PARAM_UI8,          0               },
	{_T("or"),              0,              PARAM_EAX,          PARAM_IMM,          0               },
	{_T("push    cs"),      0,              0,                  0,                  0               },
	{_T("two_byte"),        TWO_BYTE,       0,                  0,                  0               },
	// 0x10
	{_T("adc"),             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{_T("adc"),             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("adc"),             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{_T("adc"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("adc"),             0,              PARAM_AL,           PARAM_UI8,          0               },
	{_T("adc"),             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{_T("push    ss"),      0,              0,                  0,                  0               },
	{_T("pop     ss"),      0,              0,                  0,                  0               },
	{_T("sbb"),             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{_T("sbb"),             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("sbb"),             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{_T("sbb"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("sbb"),             0,              PARAM_AL,           PARAM_UI8,          0               },
	{_T("sbb"),             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{_T("push    ds"),      0,              0,                  0,                  0               },
	{_T("pop     ds"),      0,              0,                  0,                  0               },
	// 0x20
	{_T("and"),             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{_T("and"),             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("and"),             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{_T("and"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("and"),             0,              PARAM_AL,           PARAM_UI8,          0               },
	{_T("and"),             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{_T("seg_es"),          SEG_ES,         0,                  0,                  0               },
	{_T("daa"),             0,              0,                  0,                  0               },
	{_T("sub"),             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{_T("sub"),             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("sub"),             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{_T("sub"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("sub"),             0,              PARAM_AL,           PARAM_UI8,          0               },
	{_T("sub"),             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{_T("seg_cs"),          SEG_CS,         0,                  0,                  0               },
	{_T("das"),             0,              0,                  0,                  0               },
	// 0x30
	{_T("xor"),             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{_T("xor"),             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("xor"),             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{_T("xor"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("xor"),             0,              PARAM_AL,           PARAM_UI8,          0               },
	{_T("xor"),             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{_T("seg_ss"),          SEG_SS,         0,                  0,                  0               },
	{_T("aaa"),             0,              0,                  0,                  0               },
	{_T("cmp"),             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{_T("cmp"),             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("cmp"),             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{_T("cmp"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmp"),             0,              PARAM_AL,           PARAM_UI8,          0               },
	{_T("cmp"),             0,              PARAM_EAX,          PARAM_IMM,          0               },
	{_T("seg_ds"),          SEG_DS,         0,                  0,                  0               },
	{_T("aas"),             0,              0,                  0,                  0               },
	// 0x40
	{_T("inc"),             ISREX,          PARAM_EAX,          0,                  0               },
	{_T("inc"),             ISREX,          PARAM_ECX,          0,                  0               },
	{_T("inc"),             ISREX,          PARAM_EDX,          0,                  0               },
	{_T("inc"),             ISREX,          PARAM_EBX,          0,                  0               },
	{_T("inc"),             ISREX,          PARAM_ESP,          0,                  0               },
	{_T("inc"),             ISREX,          PARAM_EBP,          0,                  0               },
	{_T("inc"),             ISREX,          PARAM_ESI,          0,                  0               },
	{_T("inc"),             ISREX,          PARAM_EDI,          0,                  0               },
	{_T("dec"),             ISREX,          PARAM_EAX,          0,                  0               },
	{_T("dec"),             ISREX,          PARAM_ECX,          0,                  0               },
	{_T("dec"),             ISREX,          PARAM_EDX,          0,                  0               },
	{_T("dec"),             ISREX,          PARAM_EBX,          0,                  0               },
	{_T("dec"),             ISREX,          PARAM_ESP,          0,                  0               },
	{_T("dec"),             ISREX,          PARAM_EBP,          0,                  0               },
	{_T("dec"),             ISREX,          PARAM_ESI,          0,                  0               },
	{_T("dec"),             ISREX,          PARAM_EDI,          0,                  0               },
	// 0x50
	{_T("push"),            ALWAYS64,       PARAM_EAX,          0,                  0               },
	{_T("push"),            ALWAYS64,       PARAM_ECX,          0,                  0               },
	{_T("push"),            ALWAYS64,       PARAM_EDX,          0,                  0               },
	{_T("push"),            ALWAYS64,       PARAM_EBX,          0,                  0               },
	{_T("push"),            ALWAYS64,       PARAM_ESP,          0,                  0               },
	{_T("push"),            ALWAYS64,       PARAM_EBP,          0,                  0               },
	{_T("push"),            ALWAYS64,       PARAM_ESI,          0,                  0               },
	{_T("push"),            ALWAYS64,       PARAM_EDI,          0,                  0               },
	{_T("pop"),             ALWAYS64,       PARAM_EAX,          0,                  0               },
	{_T("pop"),             ALWAYS64,       PARAM_ECX,          0,                  0               },
	{_T("pop"),             ALWAYS64,       PARAM_EDX,          0,                  0               },
	{_T("pop"),             ALWAYS64,       PARAM_EBX,          0,                  0               },
	{_T("pop"),             ALWAYS64,       PARAM_ESP,          0,                  0               },
	{_T("pop"),             ALWAYS64,       PARAM_EBP,          0,                  0               },
	{_T("pop"),             ALWAYS64,       PARAM_ESI,          0,                  0               },
	{_T("pop"),             ALWAYS64,       PARAM_EDI,          0,                  0               },
	// 0x60
	{_T("pusha\0pushad\0<invalid>"),VAR_NAME,0,                 0,                  0               },
	{_T("popa\0popad\0<invalid>"),  VAR_NAME,0,                 0,                  0               },
	{_T("bound"),           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("arpl"),            MODRM | SPECIAL64_ENT(0),PARAM_RM,  PARAM_REG16,        0               },
	{_T("seg_fs"),          SEG_FS,         0,                  0,                  0               },
	{_T("seg_gs"),          SEG_GS,         0,                  0,                  0               },
	{_T("op_size"),         OP_SIZE,        0,                  0,                  0               },
	{_T("addr_size"),       ADDR_SIZE,      0,                  0,                  0               },
	{_T("push"),            0,              PARAM_IMM,          0,                  0               },
	{_T("imul"),            MODRM,          PARAM_REG,          PARAM_RM,           PARAM_IMM       },
	{_T("push"),            0,              PARAM_I8,           0,                  0               },
	{_T("imul"),            MODRM,          PARAM_REG,          PARAM_RM,           PARAM_I8        },
	{_T("insb"),            0,              0,                  0,                  0               },
	{_T("insw\0insd\0insd"),VAR_NAME,       0,                  0,                  0               },
	{_T("outsb"),           0,              PARAM_PREIMP,       0,                  0               },
	{_T("outsw\0outsd\0outsd"),VAR_NAME,    PARAM_PREIMP,       0,                  0               },
	// 0x70
	{_T("jo"),              0,              PARAM_REL8,         0,                  0               },
	{_T("jno"),             0,              PARAM_REL8,         0,                  0               },
	{_T("jb"),              0,              PARAM_REL8,         0,                  0               },
	{_T("jae"),             0,              PARAM_REL8,         0,                  0               },
	{_T("je"),              0,              PARAM_REL8,         0,                  0               },
	{_T("jne"),             0,              PARAM_REL8,         0,                  0               },
	{_T("jbe"),             0,              PARAM_REL8,         0,                  0               },
	{_T("ja"),              0,              PARAM_REL8,         0,                  0               },
	{_T("js"),              0,              PARAM_REL8,         0,                  0               },
	{_T("jns"),             0,              PARAM_REL8,         0,                  0               },
	{_T("jp"),              0,              PARAM_REL8,         0,                  0               },
	{_T("jnp"),             0,              PARAM_REL8,         0,                  0               },
	{_T("jl"),              0,              PARAM_REL8,         0,                  0               },
	{_T("jge"),             0,              PARAM_REL8,         0,                  0               },
	{_T("jle"),             0,              PARAM_REL8,         0,                  0               },
	{_T("jg"),              0,              PARAM_REL8,         0,                  0               },
	// 0x80
	{_T("group80"),         GROUP,          0,                  0,                  0               },
	{_T("group81"),         GROUP,          0,                  0,                  0               },
	{_T("group80"),         GROUP,          0,                  0,                  0               },
	{_T("group83"),         GROUP,          0,                  0,                  0               },
	{_T("test"),            MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{_T("test"),            MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("xchg"),            MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{_T("xchg"),            MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("mov"),             MODRM,          PARAM_RM8,          PARAM_REG8,         0               },
	{_T("mov"),             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("mov"),             MODRM,          PARAM_REG8,         PARAM_RM8,          0               },
	{_T("mov"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("mov"),             MODRM,          PARAM_RM,           PARAM_SREG,         0               },
	{_T("lea"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("mov"),             MODRM,          PARAM_SREG,         PARAM_RM,           0               },
	{_T("pop"),             MODRM,          PARAM_RM,           0,                  0               },
	// 0x90
	{_T("nop\0???\0???\0pause"),    VAR_NAME4,          0,                  0,                  0               },
	{_T("xchg"),            0,              PARAM_EAX,          PARAM_ECX,          0               },
	{_T("xchg"),            0,              PARAM_EAX,          PARAM_EDX,          0               },
	{_T("xchg"),            0,              PARAM_EAX,          PARAM_EBX,          0               },
	{_T("xchg"),            0,              PARAM_EAX,          PARAM_ESP,          0               },
	{_T("xchg"),            0,              PARAM_EAX,          PARAM_EBP,          0               },
	{_T("xchg"),            0,              PARAM_EAX,          PARAM_ESI,          0               },
	{_T("xchg"),            0,              PARAM_EAX,          PARAM_EDI,          0               },
	{_T("cbw\0cwde\0cdqe"), VAR_NAME,       0,                  0,                  0               },
	{_T("cwd\0cdq\0cqo"),   VAR_NAME,       0,                  0,                  0               },
	{_T("call"),            ALWAYS64,       PARAM_ADDR,         0,                  0,              DASMFLAG_STEP_OVER},
	{_T("wait"),            0,              0,                  0,                  0               },
	{_T("pushf\0pushfd\0pushfq"),VAR_NAME,  0,                  0,                  0               },
	{_T("popf\0popfd\0popfq"),VAR_NAME,     0,                  0,                  0               },
	{_T("sahf"),            0,              0,                  0,                  0               },
	{_T("lahf"),            0,              0,                  0,                  0               },
	// 0xa0
	{_T("mov"),             0,              PARAM_AL,           PARAM_MEM_OFFS,     0               },
	{_T("mov"),             0,              PARAM_EAX,          PARAM_MEM_OFFS,     0               },
	{_T("mov"),             0,              PARAM_MEM_OFFS,     PARAM_AL,           0               },
	{_T("mov"),             0,              PARAM_MEM_OFFS,     PARAM_EAX,          0               },
	{_T("movsb"),           0,              PARAM_PREIMP,       0,                  0               },
	{_T("movsw\0movsd\0movsq"),VAR_NAME,    PARAM_PREIMP,       0,                  0               },
	{_T("cmpsb"),           0,              PARAM_PREIMP,       0,                  0               },
	{_T("cmpsw\0cmpsd\0cmpsq"),VAR_NAME,    PARAM_PREIMP,       0,                  0               },
	{_T("test"),            0,              PARAM_AL,           PARAM_UI8,          0               },
	{_T("test"),            0,              PARAM_EAX,          PARAM_IMM,          0               },
	{_T("stosb"),           0,              0,                  0,                  0               },
	{_T("stosw\0stosd\0stosq"),VAR_NAME,    0,                  0,                  0               },
	{_T("lodsb"),           0,              PARAM_PREIMP,       0,                  0               },
	{_T("lodsw\0lodsd\0lodsq"),VAR_NAME,    PARAM_PREIMP,       0,                  0               },
	{_T("scasb"),           0,              0,                  0,                  0               },
	{_T("scasw\0scasd\0scasq"),VAR_NAME,    0,                  0,                  0               },
	// 0xb0
	{_T("mov"),             0,              PARAM_AL,           PARAM_UI8,          0               },
	{_T("mov"),             0,              PARAM_CL,           PARAM_UI8,          0               },
	{_T("mov"),             0,              PARAM_DL,           PARAM_UI8,          0               },
	{_T("mov"),             0,              PARAM_BL,           PARAM_UI8,          0               },
	{_T("mov"),             0,              PARAM_AH,           PARAM_UI8,          0               },
	{_T("mov"),             0,              PARAM_CH,           PARAM_UI8,          0               },
	{_T("mov"),             0,              PARAM_DH,           PARAM_UI8,          0               },
	{_T("mov"),             0,              PARAM_BH,           PARAM_UI8,          0               },
	{_T("mov"),             0,              PARAM_EAX,          PARAM_IMM64,        0               },
	{_T("mov"),             0,              PARAM_ECX,          PARAM_IMM64,        0               },
	{_T("mov"),             0,              PARAM_EDX,          PARAM_IMM64,        0               },
	{_T("mov"),             0,              PARAM_EBX,          PARAM_IMM64,        0               },
	{_T("mov"),             0,              PARAM_ESP,          PARAM_IMM64,        0               },
	{_T("mov"),             0,              PARAM_EBP,          PARAM_IMM64,        0               },
	{_T("mov"),             0,              PARAM_ESI,          PARAM_IMM64,        0               },
	{_T("mov"),             0,              PARAM_EDI,          PARAM_IMM64,        0               },
	// 0xc0
	{_T("groupC0"),         GROUP,          0,                  0,                  0               },
	{_T("groupC1"),         GROUP,          0,                  0,                  0               },
	{_T("ret"),             0,              PARAM_UI16,         0,                  0,              DASMFLAG_STEP_OUT},
	{_T("ret"),             0,              0,                  0,                  0,              DASMFLAG_STEP_OUT},
	{_T("les"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("lds"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("mov"),             MODRM,          PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("mov"),             MODRM,          PARAM_RMPTR,        PARAM_IMM,          0               },
	{_T("enter"),           0,              PARAM_UI16,         PARAM_UI8,          0               },
	{_T("leave"),           0,              0,                  0,                  0               },
	{_T("retf"),            0,              PARAM_UI16,         0,                  0,              DASMFLAG_STEP_OUT},
	{_T("retf"),            0,              0,                  0,                  0,              DASMFLAG_STEP_OUT},
	{_T("int 3"),           0,              0,                  0,                  0,              DASMFLAG_STEP_OVER},
	{_T("int"),             0,              PARAM_UI8,          0,                  0,              DASMFLAG_STEP_OVER},
	{_T("into"),            0,              0,                  0,                  0               },
	{_T("iret"),            0,              0,                  0,                  0,              DASMFLAG_STEP_OUT},
	// 0xd0
	{_T("groupD0"),         GROUP,          0,                  0,                  0               },
	{_T("groupD1"),         GROUP,          0,                  0,                  0               },
	{_T("groupD2"),         GROUP,          0,                  0,                  0               },
	{_T("groupD3"),         GROUP,          0,                  0,                  0               },
	{_T("aam"),             0,              PARAM_UI8,          0,                  0               },
	{_T("aad"),             0,              PARAM_UI8,          0,                  0               },
	{_T("salc"),            0,              0,                  0,                  0               }, //AMD docs name it
	{_T("xlat"),            0,              0,                  0,                  0               },
	{_T("escape"),          FPU,            0,                  0,                  0               },
	{_T("escape"),          FPU,            0,                  0,                  0               },
	{_T("escape"),          FPU,            0,                  0,                  0               },
	{_T("escape"),          FPU,            0,                  0,                  0               },
	{_T("escape"),          FPU,            0,                  0,                  0               },
	{_T("escape"),          FPU,            0,                  0,                  0               },
	{_T("escape"),          FPU,            0,                  0,                  0               },
	{_T("escape"),          FPU,            0,                  0,                  0               },
	// 0xe0
	{_T("loopne"),          0,              PARAM_REL8,         0,                  0,              DASMFLAG_STEP_OVER},
	{_T("loopz"),           0,              PARAM_REL8,         0,                  0,              DASMFLAG_STEP_OVER},
	{_T("loop"),            0,              PARAM_REL8,         0,                  0,              DASMFLAG_STEP_OVER},
	{_T("jcxz\0jecxz\0jrcxz"),VAR_NAME,     PARAM_REL8,         0,                  0               },
	{_T("in"),              0,              PARAM_AL,           PARAM_UI8,          0               },
	{_T("in"),              0,              PARAM_EAX,          PARAM_UI8,          0               },
	{_T("out"),             0,              PARAM_UI8,          PARAM_AL,           0               },
	{_T("out"),             0,              PARAM_UI8,          PARAM_EAX,          0               },
	{_T("call"),            0,              PARAM_REL,          0,                  0,              DASMFLAG_STEP_OVER},
	{_T("jmp"),             0,              PARAM_REL,          0,                  0               },
	{_T("jmp"),             0,              PARAM_ADDR,         0,                  0               },
	{_T("jmp"),             0,              PARAM_REL8,         0,                  0               },
	{_T("in"),              0,              PARAM_AL,           PARAM_DX,           0               },
	{_T("in"),              0,              PARAM_EAX,          PARAM_DX,           0               },
	{_T("out"),             0,              PARAM_DX,           PARAM_AL,           0               },
	{_T("out"),             0,              PARAM_DX,           PARAM_EAX,          0               },
	// 0xf0
	{_T("lock"),            0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("repne"),           PREFIX,         0,                  0,                  0               },
	{_T("rep"),             PREFIX,         0,                  0,                  0               },
	{_T("hlt"),             0,              0,                  0,                  0               },
	{_T("cmc"),             0,              0,                  0,                  0               },
	{_T("groupF6"),         GROUP,          0,                  0,                  0               },
	{_T("groupF7"),         GROUP,          0,                  0,                  0               },
	{_T("clc"),             0,              0,                  0,                  0               },
	{_T("stc"),             0,              0,                  0,                  0               },
	{_T("cli"),             0,              0,                  0,                  0               },
	{_T("sti"),             0,              0,                  0,                  0               },
	{_T("cld"),             0,              0,                  0,                  0               },
	{_T("std"),             0,              0,                  0,                  0               },
	{_T("groupFE"),         GROUP,          0,                  0,                  0               },
	{_T("groupFF"),         GROUP,          0,                  0,                  0               }
};

static const I386_OPCODE x64_opcode_alt[] =
{
	{_T("movsxd"),          MODRM | ALWAYS64,PARAM_REG,         PARAM_RMPTR32,      0               },
};

static const I386_OPCODE i386_opcode_table2[256] =
{
	// 0x00
	{_T("group0F00"),       GROUP,          0,                  0,                  0               },
	{_T("group0F01"),       GROUP,          0,                  0,                  0               },
	{_T("lar"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("lsl"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("syscall"),         0,              0,                  0,                  0               },
	{_T("clts"),            0,              0,                  0,                  0               },
	{_T("sysret"),          0,              0,                  0,                  0               },
	{_T("invd"),            0,              0,                  0,                  0               },
	{_T("wbinvd"),          0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("ud2"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("group0F0D"),           GROUP,              0,                  0,                  0               }, //AMD only
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	// 0x10
	{_T("movups\0")
		_T("movupd\0")
		_T("movsd\0")
		_T("movss"),            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("movups\0")
		_T("movupd\0")
		_T("movsd\0")
		_T("movss"),            MODRM|VAR_NAME4,PARAM_XMMM,         PARAM_XMM,          0               },
	{_T("movlps\0")
		_T("movlpd\0")
		_T("movddup\0")
		_T("movsldup"),     MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("movlps\0")
		_T("movlpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMMM,         PARAM_XMM,          0               },
	{_T("unpcklps\0")
		_T("unpcklpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("unpckhps\0")
		_T("unpckhpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("group0F16"),     GROUP|GROUP_MOD, 0,                  0,                  0                   },
	{_T("movhps\0")
		_T("movhpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMMM,          PARAM_XMM,         0               },
	{_T("group0F18"),       GROUP,          0,                  0,                  0               },
	{_T("nop_hint"),        0,              PARAM_RMPTR8,               0,                  0               },
	{_T("nop_hint"),        0,              PARAM_RMPTR8,               0,                  0               },
	{_T("nop_hint"),        0,              PARAM_RMPTR8,               0,                  0               },
	{_T("nop_hint"),        0,              PARAM_RMPTR8,               0,                  0               },
	{_T("nop_hint"),        0,              PARAM_RMPTR8,               0,                  0               },
	{_T("nop_hint"),        0,              PARAM_RMPTR8,               0,                  0               },
	{_T("nop_hint"),        0,              PARAM_RMPTR8,               0,                  0               },
	// 0x20
	{_T("mov"),             MODRM,          PARAM_REG2_32,      PARAM_CREG,         0               },
	{_T("mov"),             MODRM,          PARAM_REG2_32,      PARAM_DREG,         0               },
	{_T("mov"),             MODRM,          PARAM_CREG,         PARAM_REG2_32,      0               },
	{_T("mov"),             MODRM,          PARAM_DREG,         PARAM_REG2_32,      0               },
	{_T("mov"),             MODRM,          PARAM_REG2_32,      PARAM_TREG,         0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("mov"),             MODRM,          PARAM_TREG,         PARAM_REG2_32,      0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("movaps\0")
		_T("movapd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("movaps\0")
		_T("movapd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMMM,         PARAM_XMM,          0               },
	{_T("cvtpi2ps\0")
		_T("cvtpi2pd\0")
		_T("cvtsi2sd\0")
		_T("cvtsi2ss"),     MODRM|VAR_NAME4,PARAM_XMM,          PARAM_RMXMM,        0               },
	{_T("movntps\0")
		_T("movntpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMMM,         PARAM_XMM,          0               },
	{_T("cvttps2pi\0")
		_T("cvttpd2pi\0")
		_T("cvttsd2si\0")
		_T("cvttss2si"),        MODRM|VAR_NAME4,PARAM_REGORXMM,     PARAM_XMMM,         0               },
	{_T("cvtps2pi\0")
		_T("cvtpd2pi\0")
		_T("cvtsd2si\0")
		_T("cvtss2si"),     MODRM|VAR_NAME4,PARAM_REGORXMM,     PARAM_XMMM,         0               },
	{_T("ucomiss\0")
		_T("ucomisd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("comiss\0")
		_T("comisd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	// 0x30
	{_T("wrmsr"),           0,              0,                  0,                  0               },
	{_T("rdtsc"),           0,              0,                  0,                  0               },
	{_T("rdmsr"),           0,              0,                  0,                  0               },
	{_T("rdpmc"),           0,              0,                  0,                  0               },
	{_T("sysenter"),        0,              0,                  0,                  0               },
	{_T("sysexit"),         0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("three_byte"),          THREE_BYTE,         0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("three_byte"),          THREE_BYTE,         0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	// 0x40
	{_T("cmovo"),           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovno"),          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovb"),           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovae"),          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmove"),           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovne"),          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovbe"),          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmova"),           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovs"),           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovns"),          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovpe"),          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovpo"),          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovl"),           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovge"),          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovle"),          MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("cmovg"),           MODRM,          PARAM_REG,          PARAM_RM,           0               },
	// 0x50
	{_T("movmskps\0")
		_T("movmskpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_REG3264,      PARAM_XMMM,         0               },
	{_T("sqrtps\0")
		_T("sqrtpd\0")
		_T("sqrtsd\0")
		_T("sqrtss"),           MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("rsqrtps\0")
		_T("???\0")
		_T("???\0")
		_T("rsqrtss"),          MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("rcpps\0")
		_T("???\0")
		_T("???\0")
		_T("rcpss"),            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("andps\0")
		_T("andpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("andnps\0")
		_T("andnpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("orps\0")
		_T("orpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("xorps\0")
		_T("xorpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("addps\0")
		_T("addpd\0")
		_T("addsd\0")
		_T("addss"),            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("mulps\0")
		_T("mulpd\0")
		_T("mulsd\0")
		_T("mulss"),            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("cvtps2pd\0")
		_T("cvtpd2ps\0")
		_T("cvtsd2ss\0")
		_T("cvtss2sd"),     MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("cvtdq2ps\0")
		_T("cvtps2dq\0")
		_T("???\0")
		_T("cvttps2dq"),        MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("subps\0")
		_T("subpd\0")
		_T("subsd\0")
		_T("subss"),            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("minps\0")
		_T("minpd\0")
		_T("minsd\0")
		_T("minss"),            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("divps\0")
		_T("divpd\0")
		_T("divsd\0")
		_T("divss"),            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("maxps\0")
		_T("maxpd\0")
		_T("maxsd\0")
		_T("maxss"),            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	// 0x60
	{_T("punpcklbw"),       MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("punpcklwd"),       MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("punpckldq"),       MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("packsswb"),        MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pcmpgtb"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pcmpgtw"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pcmpgtd"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("packuswb"),        MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("punpckhbw"),       MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("punpckhwd"),       MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("punpckhdq"),       MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("packssdw"),        MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("???\0")
		_T("punpcklqdq\0")
		_T("???\0")
		_T("???\0"),        MODRM|VAR_NAME4,            PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("punpckhqdq\0")
		_T("???\0")
		_T("???\0"),        MODRM|VAR_NAME4,            PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("movd"),            MODRM,          PARAM_MMX,          PARAM_RM,           0               },
	{_T("movq\0")
		_T("movdqa\0")
		_T("???\0")
		_T("movdqu"),           MODRM|VAR_NAME4,PARAM_MMX,          PARAM_MMXM,         0               },
	// 0x70
	{_T("pshufw\0")
		_T("pshufd\0")
		_T("pshuflw\0")
		_T("pshufhw"),          MODRM|VAR_NAME4,PARAM_MMX,          PARAM_MMXM,         PARAM_UI8       },
	{_T("group0F71"),       GROUP,          0,                  0,                  0               },
	{_T("group0F72"),       GROUP,          0,                  0,                  0               },
	{_T("group0F73"),       GROUP,          0,                  0,                  0               },
	{_T("pcmpeqb"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pcmpeqw"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pcmpeqd"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("emms"),            0,              0,                  0,                  0               },
	{_T("vmread"),          MODRM,              PARAM_RM,               PARAM_REG,              0               },
	{_T("vmwrite"),         MODRM,              PARAM_RM,               PARAM_REG,              0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???\0")
		_T("haddpd\0")
		_T("haddps\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("???\0")
		_T("hsubpd\0")
		_T("hsubps\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("movd\0")
		_T("movd\0")
		_T("???\0")
		_T("movq"),         MODRM|VAR_NAME4,PARAM_RM,           PARAM_MMX,          0               },
	{_T("movq\0")
		_T("movdqa\0")
		_T("???\0")
		_T("movdqu"),           MODRM|VAR_NAME4,PARAM_MMXM,         PARAM_MMX,          0               },
	// 0x80
	{_T("jo"),              0,              PARAM_REL,          0,                  0               },
	{_T("jno"),             0,              PARAM_REL,          0,                  0               },
	{_T("jb"),              0,              PARAM_REL,          0,                  0               },
	{_T("jae"),             0,              PARAM_REL,          0,                  0               },
	{_T("je"),              0,              PARAM_REL,          0,                  0               },
	{_T("jne"),             0,              PARAM_REL,          0,                  0               },
	{_T("jbe"),             0,              PARAM_REL,          0,                  0               },
	{_T("ja"),              0,              PARAM_REL,          0,                  0               },
	{_T("js"),              0,              PARAM_REL,          0,                  0               },
	{_T("jns"),             0,              PARAM_REL,          0,                  0               },
	{_T("jp"),              0,              PARAM_REL,          0,                  0               },
	{_T("jnp"),             0,              PARAM_REL,          0,                  0               },
	{_T("jl"),              0,              PARAM_REL,          0,                  0               },
	{_T("jge"),             0,              PARAM_REL,          0,                  0               },
	{_T("jle"),             0,              PARAM_REL,          0,                  0               },
	{_T("jg"),              0,              PARAM_REL,          0,                  0               },
	// 0x90
	{_T("seto"),            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("setno"),           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("setb"),            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("setae"),           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("sete"),            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("setne"),           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("setbe"),           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("seta"),            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("sets"),            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("setns"),           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("setp"),            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("setnp"),           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("setl"),            MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("setge"),           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("setle"),           MODRM,          PARAM_RMPTR8,       0,                  0               },
	{_T("setg"),            MODRM,          PARAM_RMPTR8,       0,                  0               },
	// 0xa0
	{_T("push    fs"),      0,              0,                  0,                  0               },
	{_T("pop     fs"),      0,              0,                  0,                  0               },
	{_T("cpuid"),           0,              0,                  0,                  0               },
	{_T("bt"),              MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("shld"),            MODRM,          PARAM_RM,           PARAM_REG,          PARAM_UI8       },
	{_T("shld"),            MODRM,          PARAM_RM,           PARAM_REG,          PARAM_CL        },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("push    gs"),      0,              0,                  0,                  0               },
	{_T("pop     gs"),      0,              0,                  0,                  0               },
	{_T("rsm"),             0,              0,                  0,                  0               },
	{_T("bts"),             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("shrd"),            MODRM,          PARAM_RM,           PARAM_REG,          PARAM_UI8       },
	{_T("shrd"),            MODRM,          PARAM_RM,           PARAM_REG,          PARAM_CL        },
	{_T("group0FAE"),       GROUP,          0,                  0,                  0               },
	{_T("imul"),            MODRM,          PARAM_REG,          PARAM_RM,           0               },
	// 0xb0
	{_T("cmpxchg"),         MODRM,          PARAM_RM8,          PARAM_REG,          0               },
	{_T("cmpxchg"),         MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("lss"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("btr"),             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("lfs"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("lgs"),             MODRM,          PARAM_REG,          PARAM_RM,           0               },
	{_T("movzx"),           MODRM,          PARAM_REG,          PARAM_RMPTR8,       0               },
	{_T("movzx"),           MODRM,          PARAM_REG,          PARAM_RMPTR16,      0               },
	{_T("???\0")
		_T("???\0")
		_T("???\0")
		_T("popcnt"),           MODRM|VAR_NAME4,        PARAM_REG,              PARAM_RM16,             0               },
	{_T("ud2"),             0,              0,                  0,                  0               },
	{_T("group0FBA"),       GROUP,          0,                  0,                  0               },
	{_T("btc"),             MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("bsf\0")
		_T("???\0")
		_T("???\0")
		_T("tzcnt"),            MODRM|VAR_NAME4,    PARAM_REG,          PARAM_RM,           0               },
	{_T("bsr\0")
		_T("???\0")
		_T("???\0")
		_T("lzcnt"),            MODRM|VAR_NAME4,    PARAM_REG,          PARAM_RM,           0,              DASMFLAG_STEP_OVER},
	{_T("movsx"),           MODRM,          PARAM_REG,          PARAM_RMPTR8,       0               },
	{_T("movsx"),           MODRM,          PARAM_REG,          PARAM_RMPTR16,      0               },
	// 0xc0
	{_T("xadd"),            MODRM,          PARAM_RM8,          PARAM_REG,          0               },
	{_T("xadd"),            MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("cmpps\0")
		_T("cmppd\0")
		_T("cmpsd\0")
		_T("cmpss"),            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("movnti"),          MODRM,          PARAM_RM,           PARAM_REG,          0               },
	{_T("pinsrw"),          MODRM,          PARAM_MMX,          PARAM_RM,           PARAM_UI8       },
	{_T("pextrw"),          MODRM,          PARAM_MMX,          PARAM_RM,           PARAM_UI8       },
	{_T("shufps\0")
		_T("shufpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         PARAM_UI8       },
	{_T("group0FC7"),           GROUP,          0,          0,                  0               },
	{_T("bswap"),           0,              PARAM_EAX,          0,                  0               },
	{_T("bswap"),           0,              PARAM_ECX,          0,                  0               },
	{_T("bswap"),           0,              PARAM_EDX,          0,                  0               },
	{_T("bswap"),           0,              PARAM_EBX,          0,                  0               },
	{_T("bswap"),           0,              PARAM_ESP,          0,                  0               },
	{_T("bswap"),           0,              PARAM_EBP,          0,                  0               },
	{_T("bswap"),           0,              PARAM_ESI,          0,                  0               },
	{_T("bswap"),           0,              PARAM_EDI,          0,                  0               },
	// 0xd0
	{_T("???\0")
		_T("addsubpd\0")
		_T("addsubps\0")
		_T("???\0"),            MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("psrlw"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("psrld"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("psrlq"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("paddq"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pmullw"),          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("???\0")
		_T("movq\0")
		_T("movdq2q\0")
		_T("movq2dq"),          MODRM|VAR_NAME4,PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pmovmskb"),        MODRM,          PARAM_REG3264,      PARAM_MMXM,         0               },
	{_T("psubusb"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("psubusw"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pminub"),          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pand"),            MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("paddusb"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("paddusw"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pmaxub"),          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pandn"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	// 0xe0
	{_T("pavgb"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("psraw"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("psrad"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pavgw"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pmulhuw"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pmulhw"),          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("???\0")
		_T("cvttpd2dq\0")
		_T("cvtpd2dq\0")
		_T("cvtdq2pd"),     MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("movntq\0")
		_T("movntdq\0")
		_T("???\0")
		_T("???\0"),            MODRM|VAR_NAME4,    PARAM_M64,          PARAM_MMX,          0               },
	{_T("psubsb"),          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("psubsw"),          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pminsw"),          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("por"),             MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("paddsb"),          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("paddsw"),          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pmaxsw"),          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pxor"),            MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	// 0xf0
	{_T("???\0")
		_T("???\0")
		_T("lddqu\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("psllw"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pslld"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("psllq"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pmuludq"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("pmaddwd"),         MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("psadbw"),          MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("maskmovq\0")
		_T("maskmovdqu\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("psubb"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("psubw"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("psubd"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("psubq"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("paddb"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("paddw"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("paddd"),           MODRM,          PARAM_MMX,          PARAM_MMXM,         0               },
	{_T("???"),             0,              0,                  0,                  0               }
};

static const I386_OPCODE i386_opcode_table0F38[256] =
{
	// 0x00
	{_T("pshufb\0")
		_T("pshufb\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("phaddw\0")
		_T("phaddw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("phaddd\0")
		_T("phadd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("phaddsw\0")
		_T("phaddsw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("pmaddubsw\0")
		_T("pmaddubsw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("phsubw\0")
		_T("phsubw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("phsubd\0")
		_T("phsubd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("phsubsw\0")
		_T("phsubsw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("psignb\0")
		_T("psignb\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("psignw\0")
		_T("psignw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("psignd\0")
		_T("psignd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("pmulhrsw\0")
		_T("pmulhrsw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x10
	{_T("???\0")
		_T("pblendvb\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_XMM0          },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???\0")
		_T("blendvps\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_XMM0          },
	{_T("???\0")
		_T("blendvpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_XMM0          },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???\0")
		_T("ptest\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("pabsb\0")
		_T("pabsb\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("pabsw\0")
		_T("pabsw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("pabsd\0")
		_T("pabsd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x20
	{_T("???\0")
		_T("pmovsxbw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM64,            0               },
	{_T("???\0")
		_T("pmovsxbd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM32,            0               },
	{_T("???\0")
		_T("pmovsxbq\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM16,            0               },
	{_T("???\0")
		_T("pmovsxwd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM64,            0               },
	{_T("???\0")
		_T("pmovsxwq\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM32,            0               },
	{_T("???\0")
		_T("pmovsxdq\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM64,            0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???\0")
		_T("pmuldq\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("pcmpeqq\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("movntdqa\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("packusdw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x30
	{_T("???\0")
		_T("pmovzxbw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM64,            0               },
	{_T("???\0")
		_T("pmovzxbd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM32,            0               },
	{_T("???\0")
		_T("pmovzxbq\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM16,            0               },
	{_T("???\0")
		_T("pmovzxwd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM64,            0               },
	{_T("???\0")
		_T("pmovzxwq\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM32,            0               },
	{_T("???\0")
		_T("pmovzxdq\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMM64,            0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???\0")
		_T("pcmpgtq\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("pminsb\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("pminsd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("pminuw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("pminud\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("pmaxsb\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("pmaxsd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("pmaxuw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("pmaxud\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	// 0x40
	{_T("???\0")
		_T("pmulld\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("phminposuw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x50
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x60
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x70
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x80
	{_T("???\0")
		_T("invept\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_REG32,            PARAM_XMMM,         0               },
	{_T("???\0")
		_T("invvpid\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_REG32,            PARAM_XMMM,         0               },
	{_T("???\0")
		_T("invpcid\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_REG32,            PARAM_XMMM,         0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x90
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0xa0
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0xb0
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0xc0
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0xd0
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???\0")
		_T("aesimc\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("aesenc\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("aesenclast\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("aesdec\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("???\0")
		_T("aesdeclast\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         0               },
	// 0xe0
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0xf0
	{_T("movbe\0")
		_T("???\0")
		_T("crc32\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_REG32,            PARAM_RMPTR,            0               }, // not quite correct
	{_T("movbe\0")
		_T("???\0")
		_T("crc32\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_RMPTR,            PARAM_REG32,            0               }, // not quite correct
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
};

static const I386_OPCODE i386_opcode_table0F3A[256] =
{
	// 0x00
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???\0")
		_T("roundps\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???\0")
		_T("roundpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???\0")
		_T("roundss\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???\0")
		_T("roundsd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???\0")
		_T("blendps\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???\0")
		_T("blendpd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???\0")
		_T("pblendw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("palignr\0")
		_T("palignr\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	// 0x10
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???\0")
		_T("pextrb\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_RM8,          PARAM_XMM,          PARAM_UI8           },
	{_T("???\0")
		_T("pextrw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_RM16,         PARAM_XMM,          PARAM_UI8           },
	{_T("???\0")
		_T("pextrd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_RM8,          PARAM_XMM,          PARAM_UI8           },
	{_T("???\0")
		_T("extractps\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_RM32,         PARAM_XMM,          PARAM_UI8           },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x20
	{_T("???\0")
		_T("pinsrb\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_RM8,          PARAM_UI8           },
	{_T("???\0")
		_T("insertps\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_RM8,          PARAM_UI8           },
	{_T("???\0")
		_T("pinsrd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_RM32,         PARAM_UI8           },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x30
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x40
	{_T("???\0")
		_T("dpps\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???\0")
		_T("dppd\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???\0")
		_T("mpsadbw\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???\0")
		_T("pclmulqdq\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x50
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x60
	{_T("???\0")
		_T("pcmestrm\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???\0")
		_T("pcmestri\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???\0")
		_T("pcmistrm\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???\0")
		_T("pcmistri\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x70
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x80
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0x90
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0xa0
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0xb0
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0xc0
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0xd0
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???\0")
		_T("aeskeygenassist\0")
		_T("???\0")
		_T("???"),              MODRM|VAR_NAME4,    PARAM_XMM,          PARAM_XMMM,         PARAM_UI8           },
	// 0xe0
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	// 0xf0
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
	{_T("???"),             0,              0,          0,              0               },
};

static const I386_OPCODE group80_table[8] =
{
	{_T("add"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("or"),              0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("adc"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("sbb"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("and"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("sub"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("xor"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("cmp"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               }
};

static const I386_OPCODE group81_table[8] =
{
	{_T("add"),             0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{_T("or"),              0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{_T("adc"),             0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{_T("sbb"),             0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{_T("and"),             0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{_T("sub"),             0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{_T("xor"),             0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{_T("cmp"),             0,              PARAM_RMPTR,        PARAM_IMM,          0               }
};

static const I386_OPCODE group83_table[8] =
{
	{_T("add"),             0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{_T("or"),              0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{_T("adc"),             0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{_T("sbb"),             0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{_T("and"),             0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{_T("sub"),             0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{_T("xor"),             0,              PARAM_RMPTR,        PARAM_I8,           0               },
	{_T("cmp"),             0,              PARAM_RMPTR,        PARAM_I8,           0               }
};

static const I386_OPCODE groupC0_table[8] =
{
	{_T("rol"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("ror"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("rcl"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("rcr"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("shl"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("shr"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("sal"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("sar"),             0,              PARAM_RMPTR8,       PARAM_UI8,          0               }
};

static const I386_OPCODE groupC1_table[8] =
{
	{_T("rol"),             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{_T("ror"),             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{_T("rcl"),             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{_T("rcr"),             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{_T("shl"),             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{_T("shr"),             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{_T("sal"),             0,              PARAM_RMPTR,        PARAM_UI8,          0               },
	{_T("sar"),             0,              PARAM_RMPTR,        PARAM_UI8,          0               }
};

static const I386_OPCODE groupD0_table[8] =
{
	{_T("rol"),             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{_T("ror"),             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{_T("rcl"),             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{_T("rcr"),             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{_T("shl"),             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{_T("shr"),             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{_T("sal"),             0,              PARAM_RMPTR8,       PARAM_1,            0               },
	{_T("sar"),             0,              PARAM_RMPTR8,       PARAM_1,            0               }
};

static const I386_OPCODE groupD1_table[8] =
{
	{_T("rol"),             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{_T("ror"),             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{_T("rcl"),             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{_T("rcr"),             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{_T("shl"),             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{_T("shr"),             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{_T("sal"),             0,              PARAM_RMPTR,        PARAM_1,            0               },
	{_T("sar"),             0,              PARAM_RMPTR,        PARAM_1,            0               }
};

static const I386_OPCODE groupD2_table[8] =
{
	{_T("rol"),             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{_T("ror"),             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{_T("rcl"),             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{_T("rcr"),             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{_T("shl"),             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{_T("shr"),             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{_T("sal"),             0,              PARAM_RMPTR8,       PARAM_CL,           0               },
	{_T("sar"),             0,              PARAM_RMPTR8,       PARAM_CL,           0               }
};

static const I386_OPCODE groupD3_table[8] =
{
	{_T("rol"),             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{_T("ror"),             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{_T("rcl"),             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{_T("rcr"),             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{_T("shl"),             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{_T("shr"),             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{_T("sal"),             0,              PARAM_RMPTR,        PARAM_CL,           0               },
	{_T("sar"),             0,              PARAM_RMPTR,        PARAM_CL,           0               }
};

static const I386_OPCODE groupF6_table[8] =
{
	{_T("test"),            0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("test"),            0,              PARAM_RMPTR8,       PARAM_UI8,          0               },
	{_T("not"),             0,              PARAM_RMPTR8,       0,                  0               },
	{_T("neg"),             0,              PARAM_RMPTR8,       0,                  0               },
	{_T("mul"),             0,              PARAM_RMPTR8,       0,                  0               },
	{_T("imul"),            0,              PARAM_RMPTR8,       0,                  0               },
	{_T("div"),             0,              PARAM_RMPTR8,       0,                  0               },
	{_T("idiv"),            0,              PARAM_RMPTR8,       0,                  0               }
};

static const I386_OPCODE groupF7_table[8] =
{
	{_T("test"),            0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{_T("test"),            0,              PARAM_RMPTR,        PARAM_IMM,          0               },
	{_T("not"),             0,              PARAM_RMPTR,        0,                  0               },
	{_T("neg"),             0,              PARAM_RMPTR,        0,                  0               },
	{_T("mul"),             0,              PARAM_RMPTR,        0,                  0               },
	{_T("imul"),            0,              PARAM_RMPTR,        0,                  0               },
	{_T("div"),             0,              PARAM_RMPTR,        0,                  0               },
	{_T("idiv"),            0,              PARAM_RMPTR,        0,                  0               }
};

static const I386_OPCODE groupFE_table[8] =
{
	{_T("inc"),             0,              PARAM_RMPTR8,       0,                  0               },
	{_T("dec"),             0,              PARAM_RMPTR8,       0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               }
};

static const I386_OPCODE groupFF_table[8] =
{
	{_T("inc"),             0,              PARAM_RMPTR,        0,                  0               },
	{_T("dec"),             0,              PARAM_RMPTR,        0,                  0               },
	{_T("call"),            ALWAYS64,       PARAM_RMPTR,        0,                  0,              DASMFLAG_STEP_OVER},
	{_T("call    far ptr "),0,              PARAM_RM,           0,                  0,              DASMFLAG_STEP_OVER},
	{_T("jmp"),             ALWAYS64,       PARAM_RMPTR,        0,                  0               },
	{_T("jmp     far ptr "),0,              PARAM_RM,           0,                  0               },
	{_T("push"),            0,              PARAM_RMPTR,        0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               }
};

static const I386_OPCODE group0F00_table[8] =
{
	{_T("sldt"),            0,              PARAM_RM,           0,                  0               },
	{_T("str"),             0,              PARAM_RM,           0,                  0               },
	{_T("lldt"),            0,              PARAM_RM,           0,                  0               },
	{_T("ltr"),             0,              PARAM_RM,           0,                  0               },
	{_T("verr"),            0,              PARAM_RM,           0,                  0               },
	{_T("verw"),            0,              PARAM_RM,           0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               }
};

static const I386_OPCODE group0F01_table[8] =
{
	{_T("sgdt"),            0,              PARAM_RM,           0,                  0               },
	{_T("sidt"),            0,              PARAM_RM,           0,                  0               },
	{_T("lgdt"),            0,              PARAM_RM,           0,                  0               },
	{_T("lidt"),            0,              PARAM_RM,           0,                  0               },
	{_T("smsw"),            0,              PARAM_RM,           0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("lmsw"),            0,              PARAM_RM,           0,                  0               },
	{_T("invlpg"),          0,              PARAM_RM,           0,                  0               }
};

static const I386_OPCODE group0F0D_table[8] =
{
	{_T("prefetch"),        0,              PARAM_RM8,          0,                  0               },
	{_T("prefetchw"),       0,              PARAM_RM8,          0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               }
};

static const I386_OPCODE group0F12_table[4] =
{
	{_T("movlps\0")
		_T("movlpd\0")
		_T("movddup\0")
		_T("movsldup"),     VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("movlps\0")
		_T("movlpd\0")
		_T("movddup\0")
		_T("movsldup"),     VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("movlps\0")
		_T("movlpd\0")
		_T("movddup\0")
		_T("movsldup"),     VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               },
	{_T("movhlps\0")
		_T("???\0")
		_T("movddup\0")
		_T("movsldup"),     VAR_NAME4,PARAM_XMM,          PARAM_XMMM,         0               }
};

static const I386_OPCODE group0F16_table[4] =
{
	{_T("movhps\0")
		_T("movhpd\0")
		_T("???\0")
		_T("movshdup"),     VAR_NAME4,PARAM_XMM,         PARAM_XMMM,          0               },
	{_T("movhps\0")
		_T("movhpd\0")
		_T("???\0")
		_T("movshdup"),     VAR_NAME4,PARAM_XMM,         PARAM_XMMM,          0               },
	{_T("movhps\0")
		_T("movhpd\0")
		_T("???\0")
		_T("movshdup"),     VAR_NAME4,PARAM_XMM,         PARAM_XMMM,          0               },
	{_T("movlhps\0")
		_T("movhpd\0")
		_T("???\0")
		_T("movshdup"),     VAR_NAME4,PARAM_XMM,         PARAM_XMMM,          0               }
};

static const I386_OPCODE group0F18_table[8] =
{
	{_T("prefetchnta"),     0,              PARAM_RM8,          0,                  0               },
	{_T("prefetch0"),       0,              PARAM_RM8,          0,                  0               },
	{_T("prefetch1"),       0,              PARAM_RM8,          0,                  0               },
	{_T("prefetch2"),       0,              PARAM_RM8,          0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               }
};

static const I386_OPCODE group0F71_table[8] =
{
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("psrlw"),           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("psraw"),           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("psllw"),           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{_T("???"),             0,              0,                  0,                  0               }
};

static const I386_OPCODE group0F72_table[8] =
{
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("psrld"),           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("psrad"),           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("pslld"),           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{_T("???"),             0,              0,                  0,                  0               }
};

static const I386_OPCODE group0F73_table[8] =
{
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("psrlq"),           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{_T("psrldq"),          0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("psllq"),           0,              PARAM_MMX2,         PARAM_UI8,          0               },
	{_T("pslldq"),          0,              PARAM_MMX2,         PARAM_UI8,          0               },
};

static const I386_OPCODE group0FAE_table[8] =
{
	{_T("fxsave"),          0,              PARAM_RM,           0,                  0               },
	{_T("fxrstor"),         0,              PARAM_RM,           0,                  0               },
	{_T("ldmxcsr"),         0,              PARAM_RM,           0,                  0               },
	{_T("stmxscr"),         0,              PARAM_RM,           0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("lfence"),          0,              0,                  0,                  0               },
	{_T("mfence"),          0,              0,                  0,                  0               },
	{_T("sfence"),          0,              0,                  0,                  0               }
};


static const I386_OPCODE group0FBA_table[8] =
{
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("bt"),              0,              PARAM_RM,           PARAM_UI8,          0               },
	{_T("bts"),             0,              PARAM_RM,           PARAM_UI8,          0               },
	{_T("btr"),             0,              PARAM_RM,           PARAM_UI8,          0               },
	{_T("btc"),             0,              PARAM_RM,           PARAM_UI8,          0               }
};

static const I386_OPCODE group0FC7_table[8] =
{
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("cmpxchg8b"),           MODRM,              PARAM_M64PTR,               0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("???"),             0,              0,                  0,                  0               },
	{_T("vmptrld\0")
		_T("vmclear\0")
		_T("???\0")
		_T("vmxon"),            MODRM|VAR_NAME4,        PARAM_M64PTR,               0,                  0               },
	{_T("vmptrtst"),            MODRM,              PARAM_M64PTR,               0,                  0               }
};

static const GROUP_OP group_op_table[] =
{
	{_T("group80"),            group80_table           },
	{_T("group81"),            group81_table           },
	{_T("group83"),            group83_table           },
	{_T("groupC0"),            groupC0_table           },
	{_T("groupC1"),            groupC1_table           },
	{_T("groupD0"),            groupD0_table           },
	{_T("groupD1"),            groupD1_table           },
	{_T("groupD2"),            groupD2_table           },
	{_T("groupD3"),            groupD3_table           },
	{_T("groupF6"),            groupF6_table           },
	{_T("groupF7"),            groupF7_table           },
	{_T("groupFE"),            groupFE_table           },
	{_T("groupFF"),            groupFF_table           },
	{_T("group0F00"),          group0F00_table         },
	{_T("group0F01"),          group0F01_table         },
	{_T("group0F0D"),          group0F0D_table         },
	{_T("group0F12"),          group0F12_table         },
	{_T("group0F16"),          group0F16_table         },
	{_T("group0F18"),          group0F18_table         },
	{_T("group0F71"),          group0F71_table         },
	{_T("group0F72"),          group0F72_table         },
	{_T("group0F73"),          group0F73_table         },
	{_T("group0FAE"),          group0FAE_table         },
	{_T("group0FBA"),          group0FBA_table         },
	{_T("group0FC7"),          group0FC7_table         }
};



static const _TCHAR *const i386_reg[3][16] =
{
	{_T("ax"),  _T("cx"),  _T("dx"),  _T("bx"),  _T("sp"),  _T("bp"),  _T("si"),  _T("di"),  _T("r8w"), _T("r9w"), _T("r10w"),_T("r11w"),_T("r12w"),_T("r13w"),_T("r14w"),_T("r15w")},
	{_T("eax"), _T("ecx"), _T("edx"), _T("ebx"), _T("esp"), _T("ebp"), _T("esi"), _T("edi"), _T("r8d"), _T("r9d"), _T("r10d"),_T("r11d"),_T("r12d"),_T("r13d"),_T("r14d"),_T("r15d")},
	{_T("rax"), _T("rcx"), _T("rdx"), _T("rbx"), _T("rsp"), _T("rbp"), _T("rsi"), _T("rdi"), _T("r8"),  _T("r9"),  _T("r10"), _T("r11"), _T("r12"), _T("r13"), _T("r14"), _T("r15")}
};

static const _TCHAR *const i386_reg8[8] = {_T("al"), _T("cl"), _T("dl"), _T("bl"), _T("ah"), _T("ch"), _T("dh"), _T("bh")};
static const _TCHAR *const i386_reg8rex[16] = {_T("al"), _T("cl"), _T("dl"), _T("bl"), _T("spl"), _T("bpl"), _T("sil"), _T("dil"), _T("r8l"), _T("r9l"), _T("r10l"), _T("r11l"), _T("r12l"), _T("r13l"), _T("r14l"), _T("r15l")};
static const _TCHAR *const i386_sreg[8] = {_T("es"), _T("cs"), _T("ss"), _T("ds"), _T("fs"), _T("gs"), _T("???"), _T("???")};

static int address_size;
static int operand_size;
static int address_prefix;
static int operand_prefix;
static int max_length;
static UINT64 pc;
static UINT8 modrm;
static UINT32 segment;
static offs_t dasm_flags;
static _TCHAR modrm_string[256];
static UINT8 rex, regex, sibex, rmex;
static UINT8 pre0f;
static UINT8 curmode;

#define MODRM_REG1  ((modrm >> 3) & 0x7)
#define MODRM_REG2  (modrm & 0x7)
#define MODRM_MOD   ((modrm >> 6) & 0x3)

INLINE UINT8 _FETCH(void)
{
	if ((opcode_ptr - opcode_ptr_base) + 1 > max_length)
		return 0xff;
	pc++;
	return *opcode_ptr++;
}

#if 0
INLINE UINT16 _FETCH16(void)
{
	UINT16 d;
	if ((opcode_ptr - opcode_ptr_base) + 2 > max_length)
		return 0xffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8);
	opcode_ptr += 2;
	pc += 2;
	return d;
}
#endif

INLINE UINT32 _FETCH32(void)
{
	UINT32 d;
	if ((opcode_ptr - opcode_ptr_base) + 4 > max_length)
		return 0xffffffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8) | (opcode_ptr[2] << 16) | (opcode_ptr[3] << 24);
	opcode_ptr += 4;
	pc += 4;
	return d;
}

INLINE UINT8 _FETCHD(void)
{
	if ((opcode_ptr - opcode_ptr_base) + 1 > max_length)
		return 0xff;
	pc++;
	return *opcode_ptr++;
}

INLINE UINT16 _FETCHD16(void)
{
	UINT16 d;
	if ((opcode_ptr - opcode_ptr_base) + 2 > max_length)
		return 0xffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8);
	opcode_ptr += 2;
	pc += 2;
	return d;
}

INLINE UINT32 _FETCHD32(void)
{
	UINT32 d;
	if ((opcode_ptr - opcode_ptr_base) + 4 > max_length)
		return 0xffffffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8) | (opcode_ptr[2] << 16) | (opcode_ptr[3] << 24);
	opcode_ptr += 4;
	pc += 4;
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

static _TCHAR *hexstring64(UINT32 lo, UINT32 hi)
{
	static _TCHAR buffer[40];
	buffer[0] = _T('0');
	if (hi != 0)
		_stprintf(&buffer[1], _T("%X%08Xh"), hi, lo);
	else
		_stprintf(&buffer[1], _T("%Xh"), lo);
	return (buffer[1] >= _T('0') && buffer[1] <= _T('9')) ? &buffer[1] : &buffer[0];
}

static _TCHAR *hexstringpc(UINT64 pc)
{
	if (curmode == 64)
		return hexstring64((UINT32)pc, (UINT32)(pc >> 32));
	else
		return hexstring((UINT32)pc, 0);
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

static _TCHAR* handle_sib_byte( _TCHAR* s, UINT8 mod )
{
	UINT32 i32;
	UINT8 scale, i, base;
	UINT8 sib = _FETCHD();

	scale = (sib >> 6) & 0x3;
	i = ((sib >> 3) & 0x7) | sibex;
	base = (sib & 0x7) | rmex;

	if (base == 5 && mod == 0) {
		i32 = _FETCH32();
		s += _stprintf( s, _T("%s"), hexstring(i32, 0) );
	} else if (base != 5 || mod != 3)
		s += _stprintf( s, _T("%s"), i386_reg[address_size][base] );

	if ( i != 4 ) {
		s += _stprintf( s, _T("+%s"), i386_reg[address_size][i] );
		if (scale)
			s += _stprintf( s, _T("*%d"), 1 << scale );
	}
	return s;
}

static void handle_modrm(_TCHAR* s)
{
	INT8 disp8;
	INT16 disp16;
	INT32 disp32;
	UINT8 mod, rm;

	modrm = _FETCHD();
	mod = (modrm >> 6) & 0x3;
	rm = (modrm & 0x7) | rmex;

	if( modrm >= 0xc0 )
		return;

	switch(segment)
	{
		case SEG_CS: s += _stprintf( s, _T("cs:") ); break;
		case SEG_DS: s += _stprintf( s, _T("ds:") ); break;
		case SEG_ES: s += _stprintf( s, _T("es:") ); break;
		case SEG_FS: s += _stprintf( s, _T("fs:") ); break;
		case SEG_GS: s += _stprintf( s, _T("gs:") ); break;
		case SEG_SS: s += _stprintf( s, _T("ss:") ); break;
	}

	s += _stprintf( s, _T("[") );
	if( address_size == 2 ) {
		if ((rm & 7) == 4)
			s = handle_sib_byte( s, mod );
		else if ((rm & 7) == 5 && mod == 0) {
			disp32 = _FETCHD32();
			s += _stprintf( s, _T("rip%s"), shexstring(disp32, 0, TRUE) );
		} else
			s += _stprintf( s, _T("%s"), i386_reg[2][rm]);
		if( mod == 1 ) {
			disp8 = _FETCHD();
			if (disp8 != 0)
				s += _stprintf( s, _T("%s"), shexstring((INT32)disp8, 0, TRUE) );
		} else if( mod == 2 ) {
			disp32 = _FETCHD32();
			if (disp32 != 0)
				s += _stprintf( s, _T("%s"), shexstring(disp32, 0, TRUE) );
		}
	} else if (address_size == 1) {
		if ((rm & 7) == 4)
			s = handle_sib_byte( s, mod );
		else if ((rm & 7) == 5 && mod == 0) {
			disp32 = _FETCHD32();
			if (curmode == 64)
				s += _stprintf( s, _T("eip%s"), shexstring(disp32, 0, TRUE) );
			else
				s += _stprintf( s, _T("%s"), hexstring(disp32, 0) );
		} else
			s += _stprintf( s, _T("%s"), i386_reg[1][rm]);
		if( mod == 1 ) {
			disp8 = _FETCHD();
			if (disp8 != 0)
				s += _stprintf( s, _T("%s"), shexstring((INT32)disp8, 0, TRUE) );
		} else if( mod == 2 ) {
			disp32 = _FETCHD32();
			if (disp32 != 0)
				s += _stprintf( s, _T("%s"), shexstring(disp32, 0, TRUE) );
		}
	} else {
		switch( rm )
		{
			case 0: s += _stprintf( s, _T("bx+si") ); break;
			case 1: s += _stprintf( s, _T("bx+di") ); break;
			case 2: s += _stprintf( s, _T("bp+si") ); break;
			case 3: s += _stprintf( s, _T("bp+di") ); break;
			case 4: s += _stprintf( s, _T("si") ); break;
			case 5: s += _stprintf( s, _T("di") ); break;
			case 6:
				if( mod == 0 ) {
					disp16 = _FETCHD16();
					s += _stprintf( s, _T("%s"), hexstring((unsigned) (UINT16) disp16, 0) );
				} else {
					s += _stprintf( s, _T("bp") );
				}
				break;
			case 7: s += _stprintf( s, _T("bx") ); break;
		}
		if( mod == 1 ) {
			disp8 = _FETCHD();
			if (disp8 != 0)
				s += _stprintf( s, _T("%s"), shexstring((INT32)disp8, 0, TRUE) );
		} else if( mod == 2 ) {
			disp16 = _FETCHD16();
			if (disp16 != 0)
				s += _stprintf( s, _T("%s"), shexstring((INT32)disp16, 0, TRUE) );
		}
	}
	s += _stprintf( s, _T("]") );
}

static _TCHAR* handle_param(_TCHAR* s, UINT32 param)
{
	UINT8 i8;
	UINT16 i16;
	UINT32 i32;
	UINT16 ptr;
	UINT32 addr;
	INT8 d8;
	INT16 d16;
	INT32 d32;

	switch(param)
	{
		case PARAM_REG:
			s += _stprintf( s, _T("%s"), i386_reg[operand_size][MODRM_REG1 | regex] );
			break;

		case PARAM_REG8:
			s += _stprintf( s, _T("%s"), (rex ? i386_reg8rex : i386_reg8)[MODRM_REG1 | regex] );
			break;

		case PARAM_REG16:
			s += _stprintf( s, _T("%s"), i386_reg[0][MODRM_REG1 | regex] );
			break;

		case PARAM_REG32:
			s += _stprintf( s, _T("%s"), i386_reg[1][MODRM_REG1 | regex] );
			break;

		case PARAM_REG3264:
			s += _stprintf( s, _T("%s"), i386_reg[(operand_size == 2) ? 2 : 1][MODRM_REG1 | regex] );
			break;

		case PARAM_MMX:
			if (pre0f == 0x66 || pre0f == 0xf2 || pre0f == 0xf3)
				s += _stprintf( s, _T("xmm%d"), MODRM_REG1 | regex );
			else
				s += _stprintf( s, _T("mm%d"), MODRM_REG1 | regex );
			break;

		case PARAM_MMX2:
			if (pre0f == 0x66 || pre0f == 0xf2 || pre0f == 0xf3)
				s += _stprintf( s, _T("xmm%d"), MODRM_REG2 | regex );
			else
				s += _stprintf( s, _T("mm%d"), MODRM_REG2 | regex );
			break;

		case PARAM_XMM:
			s += _stprintf( s, _T("xmm%d"), MODRM_REG1 | regex );
			break;

		case PARAM_REGORXMM:
			if (pre0f != 0xf2 && pre0f != 0xf3)
				s += _stprintf( s, _T("xmm%d"), MODRM_REG1 | regex );
			else
				s += _stprintf( s, _T("%s"), i386_reg[(operand_size == 2) ? 2 : 1][MODRM_REG1 | regex] );
			break;

		case PARAM_REG2_32:
			s += _stprintf( s, _T("%s"), i386_reg[1][MODRM_REG2 | rmex] );
			break;

		case PARAM_RM:
		case PARAM_RMPTR:
			if( modrm >= 0xc0 ) {
				s += _stprintf( s, _T("%s"), i386_reg[operand_size][MODRM_REG2 | rmex] );
			} else {
				if (param == PARAM_RMPTR)
				{
					if( operand_size == 2 )
						s += _stprintf( s, _T("qword ptr ") );
					else if (operand_size == 1)
						s += _stprintf( s, _T("dword ptr ") );
					else
						s += _stprintf( s, _T("word ptr ") );
				}
				s += _stprintf( s, _T("%s"), modrm_string );
			}
			break;

		case PARAM_RM8:
		case PARAM_RMPTR8:
			if( modrm >= 0xc0 ) {
				s += _stprintf( s, _T("%s"), (rex ? i386_reg8rex : i386_reg8)[MODRM_REG2 | rmex] );
			} else {
				if (param == PARAM_RMPTR8)
					s += _stprintf( s, _T("byte ptr ") );
				s += _stprintf( s, _T("%s"), modrm_string );
			}
			break;

		case PARAM_RM16:
		case PARAM_RMPTR16:
			if( modrm >= 0xc0 ) {
				s += _stprintf( s, _T("%s"), i386_reg[0][MODRM_REG2 | rmex] );
			} else {
				if (param == PARAM_RMPTR16)
					s += _stprintf( s, _T("word ptr ") );
				s += _stprintf( s, _T("%s"), modrm_string );
			}
			break;

		case PARAM_RM32:
		case PARAM_RMPTR32:
			if( modrm >= 0xc0 ) {
				s += _stprintf( s, _T("%s"), i386_reg[1][MODRM_REG2 | rmex] );
			} else {
				if (param == PARAM_RMPTR32)
					s += _stprintf( s, _T("dword ptr ") );
				s += _stprintf( s, _T("%s"), modrm_string );
			}
			break;

		case PARAM_RMXMM:
			if( modrm >= 0xc0 ) {
				if (pre0f != 0xf2 && pre0f != 0xf3)
					s += _stprintf( s, _T("xmm%d"), MODRM_REG2 | rmex );
				else
					s += _stprintf( s, _T("%s"), i386_reg[(operand_size == 2) ? 2 : 1][MODRM_REG2 | rmex] );
			} else {
				if (param == PARAM_RMPTR32)
					s += _stprintf( s, _T("dword ptr ") );
				s += _stprintf( s, _T("%s"), modrm_string );
			}
			break;

		case PARAM_M64:
		case PARAM_M64PTR:
			if( modrm >= 0xc0 ) {
				s += _stprintf( s, _T("???") );
			} else {
				if (param == PARAM_M64PTR)
					s += _stprintf( s, _T("qword ptr ") );
				s += _stprintf( s, _T("%s"), modrm_string );
			}
			break;

		case PARAM_MMXM:
			if( modrm >= 0xc0 ) {
				if (pre0f == 0x66 || pre0f == 0xf2 || pre0f == 0xf3)
					s += _stprintf( s, _T("xmm%d"), MODRM_REG2 | rmex );
				else
					s += _stprintf( s, _T("mm%d"), MODRM_REG2 | rmex );
			} else {
				s += _stprintf( s, _T("%s"), modrm_string );
			}
			break;

		case PARAM_XMMM:
			if( modrm >= 0xc0 ) {
				s += _stprintf( s, _T("xmm%d"), MODRM_REG2 | rmex );
			} else {
				s += _stprintf( s, _T("%s"), modrm_string );
			}
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

		case PARAM_UI16:
			i16 = _FETCHD16();
			s += _stprintf( s, _T("%s"), shexstring((UINT16)i16, 0, FALSE) );
			break;

		case PARAM_IMM64:
			if (operand_size == 2) {
				UINT32 lo32 = _FETCHD32();
				i32 = _FETCHD32();
				s += _stprintf( s, _T("%s"), hexstring64(lo32, i32) );
			} else if( operand_size ) {
				i32 = _FETCHD32();
				s += _stprintf( s, _T("%s"), hexstring(i32, 0) );
			} else {
				i16 = _FETCHD16();
				s += _stprintf( s, _T("%s"), hexstring(i16, 0) );
			}
			break;

		case PARAM_IMM:
			if( operand_size ) {
				i32 = _FETCHD32();
				s += _stprintf( s, _T("%s"), hexstring(i32, 0) );
			} else {
				i16 = _FETCHD16();
				s += _stprintf( s, _T("%s"), hexstring(i16, 0) );
			}
			break;

		case PARAM_ADDR:
			if( operand_size ) {
				addr = _FETCHD32();
				ptr = _FETCHD16();
				s += _stprintf( s, _T("%s:"), hexstring(ptr, 4) );
				s += _stprintf( s, _T("%s"), hexstring(addr, 0) );
			} else {
				addr = _FETCHD16();
				ptr = _FETCHD16();
				s += _stprintf( s, _T("%s:"), hexstring(ptr, 4) );
				s += _stprintf( s, _T("%s"), hexstring(addr, 0) );
			}
			break;

		case PARAM_REL:
			if( operand_size ) {
				d32 = _FETCHD32();
				s += _stprintf( s, _T("%s"), hexstringpc(pc + d32) );
			} else {
				/* make sure to keep the relative offset within the segment */
				d16 = _FETCHD16();
				s += _stprintf( s, _T("%s"), hexstringpc((pc & 0xFFFF0000) | ((pc + d16) & 0x0000FFFF)) );
			}
			break;

		case PARAM_REL8:
			d8 = _FETCHD();
			s += _stprintf( s, _T("%s"), hexstringpc(pc + d8) );
			break;

		case PARAM_MEM_OFFS:
			switch(segment)
			{
				case SEG_CS: s += _stprintf( s, _T("cs:") ); break;
				case SEG_DS: s += _stprintf( s, _T("ds:") ); break;
				case SEG_ES: s += _stprintf( s, _T("es:") ); break;
				case SEG_FS: s += _stprintf( s, _T("fs:") ); break;
				case SEG_GS: s += _stprintf( s, _T("gs:") ); break;
				case SEG_SS: s += _stprintf( s, _T("ss:") ); break;
			}

			if( address_size ) {
				i32 = _FETCHD32();
				s += _stprintf( s, _T("[%s]"), hexstring(i32, 0) );
			} else {
				i16 = _FETCHD16();
				s += _stprintf( s, _T("[%s]"), hexstring(i16, 0) );
			}
			break;

		case PARAM_PREIMP:
			switch(segment)
			{
				case SEG_CS: s += _stprintf( s, _T("cs:") ); break;
				case SEG_DS: s += _stprintf( s, _T("ds:") ); break;
				case SEG_ES: s += _stprintf( s, _T("es:") ); break;
				case SEG_FS: s += _stprintf( s, _T("fs:") ); break;
				case SEG_GS: s += _stprintf( s, _T("gs:") ); break;
				case SEG_SS: s += _stprintf( s, _T("ss:") ); break;
			}
			break;

		case PARAM_SREG:
			s += _stprintf( s, _T("%s"), i386_sreg[MODRM_REG1] );
			break;

		case PARAM_CREG:
			s += _stprintf( s, _T("cr%d"), MODRM_REG1 | regex );
			break;

		case PARAM_TREG:
			s += _stprintf( s, _T("tr%d"), MODRM_REG1 | regex );
			break;

		case PARAM_DREG:
			s += _stprintf( s, _T("dr%d"), MODRM_REG1 | regex );
			break;

		case PARAM_1:
			s += _stprintf( s, _T("1") );
			break;

		case PARAM_DX:
			s += _stprintf( s, _T("dx") );
			break;

		case PARAM_XMM0:
			s += _stprintf( s, _T("xmm0") );
			break;

		case PARAM_AL: s += _stprintf( s, _T("al") ); break;
		case PARAM_CL: s += _stprintf( s, _T("cl") ); break;
		case PARAM_DL: s += _stprintf( s, _T("dl") ); break;
		case PARAM_BL: s += _stprintf( s, _T("bl") ); break;
		case PARAM_AH: s += _stprintf( s, _T("ah") ); break;
		case PARAM_CH: s += _stprintf( s, _T("ch") ); break;
		case PARAM_DH: s += _stprintf( s, _T("dh") ); break;
		case PARAM_BH: s += _stprintf( s, _T("bh") ); break;

		case PARAM_EAX: s += _stprintf( s, _T("%s"), i386_reg[operand_size][0 | rmex] ); break;
		case PARAM_ECX: s += _stprintf( s, _T("%s"), i386_reg[operand_size][1 | rmex] ); break;
		case PARAM_EDX: s += _stprintf( s, _T("%s"), i386_reg[operand_size][2 | rmex] ); break;
		case PARAM_EBX: s += _stprintf( s, _T("%s"), i386_reg[operand_size][3 | rmex] ); break;
		case PARAM_ESP: s += _stprintf( s, _T("%s"), i386_reg[operand_size][4 | rmex] ); break;
		case PARAM_EBP: s += _stprintf( s, _T("%s"), i386_reg[operand_size][5 | rmex] ); break;
		case PARAM_ESI: s += _stprintf( s, _T("%s"), i386_reg[operand_size][6 | rmex] ); break;
		case PARAM_EDI: s += _stprintf( s, _T("%s"), i386_reg[operand_size][7 | rmex] ); break;
	}
	return s;
}

static void handle_fpu(_TCHAR *s, UINT8 op1, UINT8 op2)
{
	switch (op1 & 0x7)
	{
		case 0:     // Group D8
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
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
					case 1: _stprintf(s, _T("fmul    st(0),st(%d)"), op2 & 0x7); break;
					case 2: _stprintf(s, _T("fcom    st(0),st(%d)"), op2 & 0x7); break;
					case 3: _stprintf(s, _T("fcomp   st(0),st(%d)"), op2 & 0x7); break;
					case 4: _stprintf(s, _T("fsub    st(0),st(%d)"), op2 & 0x7); break;
					case 5: _stprintf(s, _T("fsubr   st(0),st(%d)"), op2 & 0x7); break;
					case 6: _stprintf(s, _T("fdiv    st(0),st(%d)"), op2 & 0x7); break;
					case 7: _stprintf(s, _T("fdivr   st(0),st(%d)"), op2 & 0x7); break;
				}
			}
			break;
		}

		case 1:     // Group D9
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
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

		case 2:     // Group DA
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
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
					case 0x29:
						_stprintf(s, _T("fucompp")); break;

					default: _stprintf(s, _T("??? (FPU)")); break;

				}
			}
			break;
		}

		case 3:     // Group DB
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fild    dword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("fisttp  dword ptr %s"), modrm_string); break;
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

		case 4:     // Group DC
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
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

		case 5:     // Group DD
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fld     qword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("fisttp  qword ptr %s"), modrm_string); break;
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

		case 6:     // Group DE
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
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

		case 7:     // Group DF
		{
			if (op2 < 0xc0)
			{
				pc--;       // adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fild    word ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("fisttp  word ptr %s"), modrm_string); break;
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
					case 0x20: _stprintf(s, _T("fstsw   ax")); break;

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

static void decode_opcode(_TCHAR *s, const I386_OPCODE *op, UINT8 op1)
{
	int i;
	UINT8 op2;

	if ((op->flags & SPECIAL64) && (address_size == 2))
		op = &x64_opcode_alt[op->flags >> 24];

	switch( op->flags & FLAGS_MASK )
	{
		case ISREX:
			if (curmode == 64)
			{
				rex = op1;
				operand_size = (op1 & 8) ? 2 : 1;
				regex = (op1 << 1) & 8;
				sibex = (op1 << 2) & 8;
				rmex = (op1 << 3) & 8;
				op2 = _FETCH();
				decode_opcode( s, &i386_opcode_table1[op2], op1 );
				return;
			}
			break;

		case OP_SIZE:
			rex = regex = sibex = rmex = 0;
			if (operand_size < 2 && operand_prefix == 0)
			{
				operand_size ^= 1;
				operand_prefix = 1;
			}
			op2 = _FETCH();
			decode_opcode( s, &i386_opcode_table1[op2], op2 );
			return;

		case ADDR_SIZE:
			rex = regex = sibex = rmex = 0;
			if(address_prefix == 0)
			{
				if (curmode != 64)
					address_size ^= 1;
				else
					address_size ^= 3;
				address_prefix = 1;
			}
			op2 = _FETCH();
			decode_opcode( s, &i386_opcode_table1[op2], op2 );
			return;

		case TWO_BYTE:
			if (&opcode_ptr[-2] >= opcode_ptr_base)
				pre0f = opcode_ptr[-2];
			op2 = _FETCHD();
			decode_opcode( s, &i386_opcode_table2[op2], op1 );
			return;

		case THREE_BYTE:
			op2 = _FETCHD();
			if (opcode_ptr[-2] == 0x38)
				decode_opcode( s, &i386_opcode_table0F38[op2], op1 );
			else
				decode_opcode( s, &i386_opcode_table0F3A[op2], op1 );
			return;

		case SEG_CS:
		case SEG_DS:
		case SEG_ES:
		case SEG_FS:
		case SEG_GS:
		case SEG_SS:
			rex = regex = sibex = rmex = 0;
			segment = op->flags;
			op2 = _FETCH();
			decode_opcode( s, &i386_opcode_table1[op2], op2 );
			return;

		case PREFIX:
			op2 = _FETCH();
			if ((op2 != 0x0f) && (op2 != 0x90))
				s += _stprintf( s, _T("%-7s "), op->mnemonic );
			if ((op2 == 0x90) && !pre0f)
				pre0f = op1;
			decode_opcode( s, &i386_opcode_table1[op2], op2 );
			return;

		case GROUP:
			handle_modrm( modrm_string );
			for( i=0; i < ARRAY_LENGTH(group_op_table); i++ ) {
				if( _tcscmp(op->mnemonic, group_op_table[i].mnemonic) == 0 ) {
					if (op->flags & GROUP_MOD)
						decode_opcode( s, &group_op_table[i].opcode[MODRM_MOD], op1 );
					else
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

	if ((op->flags & ALWAYS64) && curmode == 64)
		operand_size = 2;

	if ((op->flags & VAR_NAME) && operand_size > 0)
	{
		const _TCHAR *mnemonic = op->mnemonic + _tcslen(op->mnemonic) + 1;
		if (operand_size == 2)
			mnemonic += _tcslen(mnemonic) + 1;
		s += _stprintf( s, _T("%-7s "), mnemonic );
	}
	else if (op->flags & VAR_NAME4)
	{
		const _TCHAR *mnemonic = op->mnemonic;
		int which = (pre0f == 0xf3) ? 3 : (pre0f == 0xf2) ? 2 : (pre0f == 0x66) ? 1 : 0;
		while (which--)
			mnemonic += _tcslen(mnemonic) + 1;
		s += _stprintf( s, _T("%-7s "), mnemonic );
	}
	else
		s += _stprintf( s, _T("%-7s "), op->mnemonic );
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

int i386_dasm_one_ex(_TCHAR *buffer, UINT64 eip, const UINT8 *oprom, int mode)
{
	UINT8 op;

	opcode_ptr = opcode_ptr_base = oprom;
	switch(mode)
	{
		case 1: /* 8086/8088/80186/80188 */
			address_size = 0;
			operand_size = 0;
			max_length = 8; /* maximum without redundant prefixes - not enforced by chip */
			break;
		case 2: /* 80286 */
			address_size = 0;
			operand_size = 0;
			max_length = 10;
			break;
		case 16: /* 80386+ 16-bit code segment */
			address_size = 0;
			operand_size = 0;
			max_length = 15;
			break;
		case 32: /* 80386+ 32-bit code segment */
			address_size = 1;
			operand_size = 1;
			max_length = 15;
			break;
		case 64: /* x86_64 */
			address_size = 2;
			operand_size = 1;
			max_length = 15;
			break;
	}
	pc = eip;
	dasm_flags = 0;
	segment = 0;
	curmode = mode;
	pre0f = 0;
	rex = regex = sibex = rmex = 0;
	address_prefix = 0;
	operand_prefix = 0;

	op = _FETCH();

	decode_opcode( buffer, &i386_opcode_table1[op], op );
	return (pc-eip) | dasm_flags | DASMFLAG_SUPPORTED;
}

int i386_dasm_one(_TCHAR *buffer, offs_t eip, const UINT8 *oprom, int mode)
{
	return i386_dasm_one_ex(buffer, eip, oprom, mode);
}

CPU_DISASSEMBLE( x86_16 )
{
	return i386_dasm_one_ex(buffer, eip, oprom, 16);
}

CPU_DISASSEMBLE( x86_32 )
{
	return i386_dasm_one_ex(buffer, eip, oprom, 32);
}

CPU_DISASSEMBLE( x86_64 )
{
	return i386_dasm_one_ex(buffer, eip, oprom, 64);
}
