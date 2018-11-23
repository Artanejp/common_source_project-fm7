// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philip Bennett
#pragma once

#ifndef __I386_H__
#define __I386_H__

//#include "i386.h"
#include "../../../lib/softfloat/milieu.h"
#include "../../../lib/softfloat/softfloat.h"
#include "../vtlb.h"

#include <math.h>

//#define DEBUG_MISSING_OPCODE

#define I386OP(XX)      i386_##XX
#define I486OP(XX)      i486_##XX
#define PENTIUMOP(XX)   pentium_##XX
#define MMXOP(XX)       mmx_##XX
#define SSEOP(XX)       sse_##XX

extern int i386_dasm_one(_TCHAR *buffer, UINT32 pc, const UINT8 *oprom, int mode);

enum SREGS { ES, CS, SS, DS, FS, GS };

enum BREGS
{
	AL = NATIVE_ENDIAN_VALUE_LE_BE(0,3),
	AH = NATIVE_ENDIAN_VALUE_LE_BE(1,2),
	CL = NATIVE_ENDIAN_VALUE_LE_BE(4,7),
	CH = NATIVE_ENDIAN_VALUE_LE_BE(5,6),
	DL = NATIVE_ENDIAN_VALUE_LE_BE(8,11),
	DH = NATIVE_ENDIAN_VALUE_LE_BE(9,10),
	BL = NATIVE_ENDIAN_VALUE_LE_BE(12,15),
	BH = NATIVE_ENDIAN_VALUE_LE_BE(13,14)
};

enum WREGS
{
	AX = NATIVE_ENDIAN_VALUE_LE_BE(0,1),
	CX = NATIVE_ENDIAN_VALUE_LE_BE(2,3),
	DX = NATIVE_ENDIAN_VALUE_LE_BE(4,5),
	BX = NATIVE_ENDIAN_VALUE_LE_BE(6,7),
	SP = NATIVE_ENDIAN_VALUE_LE_BE(8,9),
	BP = NATIVE_ENDIAN_VALUE_LE_BE(10,11),
	SI = NATIVE_ENDIAN_VALUE_LE_BE(12,13),
	DI = NATIVE_ENDIAN_VALUE_LE_BE(14,15)
};

enum DREGS { EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI };

enum
{
	I386_PC = 0,

	/* 8-bit registers */
	I386_AL,
	I386_AH,
	I386_BL,
	I386_BH,
	I386_CL,
	I386_CH,
	I386_DL,
	I386_DH,

	/* 16-bit registers */
	I386_AX,
	I386_BX,
	I386_CX,
	I386_DX,
	I386_BP,
	I386_SP,
	I386_SI,
	I386_DI,
	I386_IP,

	/* 32-bit registers */
	I386_EAX,
	I386_ECX,
	I386_EDX,
	I386_EBX,
	I386_EBP,
	I386_ESP,
	I386_ESI,
	I386_EDI,
	I386_EIP,

	/* segment registers */
	I386_CS,
	I386_CS_BASE,
	I386_CS_LIMIT,
	I386_CS_FLAGS,
	I386_SS,
	I386_SS_BASE,
	I386_SS_LIMIT,
	I386_SS_FLAGS,
	I386_DS,
	I386_DS_BASE,
	I386_DS_LIMIT,
	I386_DS_FLAGS,
	I386_ES,
	I386_ES_BASE,
	I386_ES_LIMIT,
	I386_ES_FLAGS,
	I386_FS,
	I386_FS_BASE,
	I386_FS_LIMIT,
	I386_FS_FLAGS,
	I386_GS,
	I386_GS_BASE,
	I386_GS_LIMIT,
	I386_GS_FLAGS,

	/* other */
	I386_EFLAGS,

	I386_CR0,
	I386_CR1,
	I386_CR2,
	I386_CR3,
	I386_CR4,

	I386_DR0,
	I386_DR1,
	I386_DR2,
	I386_DR3,
	I386_DR4,
	I386_DR5,
	I386_DR6,
	I386_DR7,

	I386_TR6,
	I386_TR7,

	I386_GDTR_BASE,
	I386_GDTR_LIMIT,
	I386_IDTR_BASE,
	I386_IDTR_LIMIT,
	I386_TR,
	I386_TR_BASE,
	I386_TR_LIMIT,
	I386_TR_FLAGS,
	I386_LDTR,
	I386_LDTR_BASE,
	I386_LDTR_LIMIT,
	I386_LDTR_FLAGS,

	I386_CPL,

	X87_CTRL,
	X87_STATUS,
	X87_TAG,
	X87_ST0,
	X87_ST1,
	X87_ST2,
	X87_ST3,
	X87_ST4,
	X87_ST5,
	X87_ST6,
	X87_ST7,

	SSE_XMM0,
	SSE_XMM1,
	SSE_XMM2,
	SSE_XMM3,
	SSE_XMM4,
	SSE_XMM5,
	SSE_XMM6,
	SSE_XMM7
};

enum
{
	/* mmx registers aliased to x87 ones */
	MMX_MM0=X87_ST0,
	MMX_MM1=X87_ST1,
	MMX_MM2=X87_ST2,
	MMX_MM3=X87_ST3,
	MMX_MM4=X87_ST4,
	MMX_MM5=X87_ST5,
	MMX_MM6=X87_ST6,
	MMX_MM7=X87_ST7
};

enum smram
{
	SMRAM_SMBASE = 0xF8,
	SMRAM_SMREV  = 0xFC,
	SMRAM_IORSRT = 0x100,
	SMRAM_AHALT  = 0x102,
	SMRAM_IOEDI  = 0x104,
	SMRAM_IOECX  = 0x108,
	SMRAM_IOESI  = 0x10C,

	SMRAM_ES     = 0x1A8,
	SMRAM_CS     = 0x1AC,
	SMRAM_SS     = 0x1B0,
	SMRAM_DS     = 0x1B4,
	SMRAM_FS     = 0x1B8,
	SMRAM_GS     = 0x1BC,
	SMRAM_LDTR   = 0x1C0,
	SMRAM_TR     = 0x1C4,
	SMRAM_DR7    = 0x1C8,
	SMRAM_DR6    = 0x1CC,
	SMRAM_EAX    = 0x1D0,
	SMRAM_ECX    = 0x1D4,
	SMRAM_EDX    = 0x1D8,
	SMRAM_EBX    = 0x1DC,
	SMRAM_ESP    = 0x1E0,
	SMRAM_EBP    = 0x1E4,
	SMRAM_ESI    = 0x1E8,
	SMRAM_EDI    = 0x1EC,
	SMRAM_EIP    = 0x1F0,
	SMRAM_EFLAGS = 0x1F4,
	SMRAM_CR3    = 0x1F8,
	SMRAM_CR0    = 0x1FC,
};

enum smram_intel_p5
{
	SMRAM_IP5_IOEIP   = 0x110,
	SMRAM_IP5_CR4     = 0x128,
	SMRAM_IP5_ESLIM   = 0x130,
	SMRAM_IP5_ESBASE  = 0x134,
	SMRAM_IP5_ESACC   = 0x138,
	SMRAM_IP5_CSLIM   = 0x13C,
	SMRAM_IP5_CSBASE  = 0x140,
	SMRAM_IP5_CSACC   = 0x144,
	SMRAM_IP5_SSLIM   = 0x148,
	SMRAM_IP5_SSBASE  = 0x14C,
	SMRAM_IP5_SSACC   = 0x150,
	SMRAM_IP5_DSLIM   = 0x154,
	SMRAM_IP5_DSBASE  = 0x158,
	SMRAM_IP5_DSACC   = 0x15C,
	SMRAM_IP5_FSLIM   = 0x160,
	SMRAM_IP5_FSBASE  = 0x164,
	SMRAM_IP5_FSACC   = 0x168,
	SMRAM_IP5_GSLIM   = 0x16C,
	SMRAM_IP5_GSBASE  = 0x170,
	SMRAM_IP5_GSACC   = 0x174,
	SMRAM_IP5_LDTLIM  = 0x178,
	SMRAM_IP5_LDTBASE = 0x17C,
	SMRAM_IP5_LDTACC  = 0x180,
	SMRAM_IP5_GDTLIM  = 0x184,
	SMRAM_IP5_GDTBASE = 0x188,
	SMRAM_IP5_GDTACC  = 0x18C,
	SMRAM_IP5_IDTLIM  = 0x190,
	SMRAM_IP5_IDTBASE = 0x194,
	SMRAM_IP5_IDTACC  = 0x198,
	SMRAM_IP5_TRLIM   = 0x19C,
	SMRAM_IP5_TRBASE  = 0x1A0,
	SMRAM_IP5_TRACC   = 0x1A4,
};

/* Protected mode exceptions */
#define FAULT_UD 6   // Invalid Opcode
#define FAULT_NM 7   // Coprocessor not available
#define FAULT_DF 8   // Double Fault
#define FAULT_TS 10  // Invalid TSS
#define FAULT_NP 11  // Segment or Gate not present
#define FAULT_SS 12  // Stack fault
#define FAULT_GP 13  // General Protection Fault
#define FAULT_PF 14  // Page Fault
#define FAULT_MF 16  // Match (Coprocessor) Fault

/* MXCSR Control and Status Register */
#define MXCSR_IE  (1<<0)  // Invalid Operation Flag
#define MXCSR_DE  (1<<1)  // Denormal Flag
#define MXCSR_ZE  (1<<2)  // Divide-by-Zero Flag
#define MXCSR_OE  (1<<3)  // Overflow Flag
#define MXCSR_UE  (1<<4)  // Underflow Flag
#define MXCSR_PE  (1<<5)  // Precision Flag
#define MXCSR_DAZ (1<<6)  // Denormals Are Zeros
#define MXCSR_IM  (1<<7)  // Invalid Operation Mask
#define MXCSR_DM  (1<<8)  // Denormal Operation Mask
#define MXCSR_ZM  (1<<9)  // Divide-by-Zero Mask
#define MXCSR_OM  (1<<10) // Overflow Mask
#define MXCSR_UM  (1<<11) // Underflow Mask
#define MXCSR_PM  (1<<12) // Precision Mask
#define MXCSR_RC  (3<<13) // Rounding Control
#define MXCSR_FZ  (1<<15) // Flush to Zero

struct I386_SREG {
	UINT16 selector;
	UINT16 flags;
	UINT32 base;
	UINT32 limit;
	int d;      // Operand size
	bool valid;
};

struct I386_CALL_GATE
{
	UINT16 segment;
	UINT16 selector;
	UINT32 offset;
	UINT8 ar;  // access rights
	UINT8 dpl;
	UINT8 dword_count;
	UINT8 present;
};

struct I386_SYS_TABLE {
	UINT32 base;
	UINT16 limit;
};

struct I386_SEG_DESC {
	UINT16 segment;
	UINT16 flags;
	UINT32 base;
	UINT32 limit;
};

union I386_GPR {
	UINT32 d[8];
	UINT16 w[16];
	UINT8 b[32];
};

union MMX_REG {
	UINT32 d[2];
	INT32  i[2];
	UINT16 w[4];
	INT16  s[4];
	UINT8  b[8];
	INT8   c[8];
	float  f[2];
	UINT64 q;
	INT64  l;
};

union XMM_REG {
	UINT8  b[16];
	UINT16 w[8];
	UINT32 d[4];
	UINT64 q[2];
	INT8   c[16];
	INT16  s[8];
	INT32  i[4];
	INT64  l[2];
	float  f[4];
	double  f64[2];
};

struct i386_state
{
	I386_GPR reg;
	I386_SREG sreg[6];
	UINT32 eip;
	UINT32 pc;
	UINT32 prev_eip;
	UINT32 prev_pc;
	UINT32 eflags;
	UINT32 eflags_mask;
	UINT8 CF;
	UINT8 DF;
	UINT8 SF;
	UINT8 OF;
	UINT8 ZF;
	UINT8 PF;
	UINT8 AF;
	UINT8 IF;
	UINT8 TF;
	UINT8 IOP1;
	UINT8 IOP2;
	UINT8 NT;
	UINT8 RF;
	UINT8 VM;
	UINT8 AC;
	UINT8 VIF;
	UINT8 VIP;
	UINT8 ID;

	UINT8 CPL;  // current privilege level

	UINT8 performed_intersegment_jump;
	UINT8 delayed_interrupt_enable;

	UINT32 cr[5];       // Control registers
	UINT32 dr[8];       // Debug registers
	UINT32 tr[8];       // Test registers

	I386_SYS_TABLE gdtr;    // Global Descriptor Table Register
	I386_SYS_TABLE idtr;    // Interrupt Descriptor Table Register
	I386_SEG_DESC task;     // Task register
	I386_SEG_DESC ldtr;     // Local Descriptor Table Register

	UINT8 ext;  // external interrupt

	int halted;
	int busreq;
	int shutdown;

	int operand_size;
	int xmm_operand_size;
	int address_size;
	int operand_prefix;
	int address_prefix;

	int segment_prefix;
	int segment_override;

#ifdef USE_DEBUGGER
	uint64_t total_cycles;
	uint64_t prev_total_cycles;
#endif
	int cycles;
	int extra_cycles;
	int base_cycles;
	UINT8 opcode;

	UINT8 irq_state;
	DEVICE *pic;
	DEVICE *program;
	DEVICE *io;
#ifdef I386_PSEUDO_BIOS
	DEVICE *bios;
#endif
#ifdef SINGLE_MODE_DMA
	DEVICE *dma;
#endif
#ifdef USE_DEBUGGER
	EMU *emu;
	DEBUGGER *debugger;
	DEVICE *program_stored;
	DEVICE *io_stored;
#endif
	UINT32 a20_mask;

	int cpuid_max_input_value_eax;
	UINT32 cpuid_id0, cpuid_id1, cpuid_id2;
	UINT32 cpu_version;
	UINT32 feature_flags;
	UINT64 tsc;
	UINT64 perfctr[2];

	// FPU
	floatx80 x87_reg[8];

	UINT16 x87_cw;
	UINT16 x87_sw;
	UINT16 x87_tw;
	UINT64 x87_data_ptr;
	UINT64 x87_inst_ptr;
	UINT16 x87_opcode;

	void (*opcode_table_x87_d8[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_d9[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_da[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_db[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_dc[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_dd[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_de[256])(i386_state *cpustate, UINT8 modrm);
	void (*opcode_table_x87_df[256])(i386_state *cpustate, UINT8 modrm);

	// SSE
	XMM_REG sse_reg[8];
	UINT32 mxcsr;

	void (*opcode_table1_16[256])(i386_state *cpustate);
	void (*opcode_table1_32[256])(i386_state *cpustate);
	void (*opcode_table2_16[256])(i386_state *cpustate);
	void (*opcode_table2_32[256])(i386_state *cpustate);
	void (*opcode_table338_16[256])(i386_state *cpustate);
	void (*opcode_table338_32[256])(i386_state *cpustate);
	void (*opcode_table33a_16[256])(i386_state *cpustate);
	void (*opcode_table33a_32[256])(i386_state *cpustate);
	void (*opcode_table366_16[256])(i386_state *cpustate);
	void (*opcode_table366_32[256])(i386_state *cpustate);
	void (*opcode_table3f2_16[256])(i386_state *cpustate);
	void (*opcode_table3f2_32[256])(i386_state *cpustate);
	void (*opcode_table3f3_16[256])(i386_state *cpustate);
	void (*opcode_table3f3_32[256])(i386_state *cpustate);
	void (*opcode_table46638_16[256])(i386_state *cpustate);
	void (*opcode_table46638_32[256])(i386_state *cpustate);
	void (*opcode_table4f238_16[256])(i386_state *cpustate);
	void (*opcode_table4f238_32[256])(i386_state *cpustate);
	void (*opcode_table4f338_16[256])(i386_state *cpustate);
	void (*opcode_table4f338_32[256])(i386_state *cpustate);
	void (*opcode_table4663a_16[256])(i386_state *cpustate);
	void (*opcode_table4663a_32[256])(i386_state *cpustate);
	void (*opcode_table4f23a_16[256])(i386_state *cpustate);
	void (*opcode_table4f23a_32[256])(i386_state *cpustate);

	bool lock_table[2][256];

	UINT8 *cycle_table_pm;
	UINT8 *cycle_table_rm;

	vtlb_state *vtlb;

	bool smm;
	bool smi;
	bool smi_latched;
	bool nmi_masked;
	bool nmi_latched;
	UINT32 smbase;
//	devcb_resolved_write_line smiact;
	bool lock;

	// bytes in current opcode, debug only
#ifdef DEBUG_MISSING_OPCODE
	UINT8 opcode_bytes[16];
	UINT32 opcode_pc;
	int opcode_bytes_length;
#endif
};

extern int i386_parity_table[256];
static int i386_limit_check(i386_state *cpustate, int seg, UINT32 offset, UINT32 size);

#define FAULT_THROW(fault,error) { throw (UINT64)(fault | (UINT64)error << 32); }
#define PF_THROW(error) { cpustate->cr[2] = address; FAULT_THROW(FAULT_PF,error); }

#define PROTECTED_MODE      (cpustate->cr[0] & 0x1)
#define STACK_32BIT         (cpustate->sreg[SS].d)
#define V8086_MODE          (cpustate->VM)
#define NESTED_TASK         (cpustate->NT)
#define WP                  (cpustate->cr[0] & 0x10000)

#define SetOF_Add32(r,s,d)  (cpustate->OF = (((r) ^ (s)) & ((r) ^ (d)) & 0x80000000) ? 1: 0)
#define SetOF_Add16(r,s,d)  (cpustate->OF = (((r) ^ (s)) & ((r) ^ (d)) & 0x8000) ? 1 : 0)
#define SetOF_Add8(r,s,d)   (cpustate->OF = (((r) ^ (s)) & ((r) ^ (d)) & 0x80) ? 1 : 0)

#define SetOF_Sub32(r,s,d)  (cpustate->OF = (((d) ^ (s)) & ((d) ^ (r)) & 0x80000000) ? 1 : 0)
#define SetOF_Sub16(r,s,d)  (cpustate->OF = (((d) ^ (s)) & ((d) ^ (r)) & 0x8000) ? 1 : 0)
#define SetOF_Sub8(r,s,d)   (cpustate->OF = (((d) ^ (s)) & ((d) ^ (r)) & 0x80) ? 1 : 0)

#define SetCF8(x)           {cpustate->CF = ((x) & 0x100) ? 1 : 0; }
#define SetCF16(x)          {cpustate->CF = ((x) & 0x10000) ? 1 : 0; }
#define SetCF32(x)          {cpustate->CF = ((x) & (((UINT64)1) << 32)) ? 1 : 0; }

#define SetSF(x)            (cpustate->SF = (x))
#define SetZF(x)            (cpustate->ZF = (x))
#define SetAF(x,y,z)        (cpustate->AF = (((x) ^ ((y) ^ (z))) & 0x10) ? 1 : 0)
#define SetPF(x)            (cpustate->PF = i386_parity_table[(x) & 0xFF])

#define SetSZPF8(x)         {cpustate->ZF = ((UINT8)(x)==0);  cpustate->SF = ((x)&0x80) ? 1 : 0; cpustate->PF = i386_parity_table[x & 0xFF]; }
#define SetSZPF16(x)        {cpustate->ZF = ((UINT16)(x)==0);  cpustate->SF = ((x)&0x8000) ? 1 : 0; cpustate->PF = i386_parity_table[x & 0xFF]; }
#define SetSZPF32(x)        {cpustate->ZF = ((UINT32)(x)==0);  cpustate->SF = ((x)&0x80000000) ? 1 : 0; cpustate->PF = i386_parity_table[x & 0xFF]; }

#define MMX(n)              (*((MMX_REG *)(&cpustate->x87_reg[(n)].low)))
#define XMM(n)              cpustate->sse_reg[(n)]

/***********************************************************************************/

struct MODRM_TABLE {
	struct {
		int b;
		int w;
		int d;
	} reg;
	struct {
		int b;
		int w;
		int d;
	} rm;
};

extern MODRM_TABLE i386_MODRM_table[256];

#define REG8(x)         (cpustate->reg.b[x])
#define REG16(x)        (cpustate->reg.w[x])
#define REG32(x)        (cpustate->reg.d[x])

#define LOAD_REG8(x)    (REG8(i386_MODRM_table[x].reg.b))
#define LOAD_REG16(x)   (REG16(i386_MODRM_table[x].reg.w))
#define LOAD_REG32(x)   (REG32(i386_MODRM_table[x].reg.d))
#define LOAD_RM8(x)     (REG8(i386_MODRM_table[x].rm.b))
#define LOAD_RM16(x)    (REG16(i386_MODRM_table[x].rm.w))
#define LOAD_RM32(x)    (REG32(i386_MODRM_table[x].rm.d))

#define STORE_REG8(x, value)    (REG8(i386_MODRM_table[x].reg.b) = value)
#define STORE_REG16(x, value)   (REG16(i386_MODRM_table[x].reg.w) = value)
#define STORE_REG32(x, value)   (REG32(i386_MODRM_table[x].reg.d) = value)
#define STORE_RM8(x, value)     (REG8(i386_MODRM_table[x].rm.b) = value)
#define STORE_RM16(x, value)    (REG16(i386_MODRM_table[x].rm.w) = value)
#define STORE_RM32(x, value)    (REG32(i386_MODRM_table[x].rm.d) = value)

#define SWITCH_ENDIAN_32(x) (((((x) << 24) & (0xff << 24)) | (((x) << 8) & (0xff << 16)) | (((x) >> 8) & (0xff << 8)) | (((x) >> 24) & (0xff << 0))))

/***********************************************************************************/

INLINE UINT32 i386_translate(i386_state *cpustate, int segment, UINT32 ip, int rwn, UINT32 size)
{
	// TODO: segment limit access size, execution permission, handle exception thrown from exception handler
	if(PROTECTED_MODE && !V8086_MODE && (rwn != -1))
	{
		if(!(cpustate->sreg[segment].valid))
			FAULT_THROW((segment==SS)?FAULT_SS:FAULT_GP, 0);
		if(i386_limit_check(cpustate, segment, ip, size))
			FAULT_THROW((segment==SS)?FAULT_SS:FAULT_GP, 0);
		if((rwn == 0) && ((cpustate->sreg[segment].flags & 8) && !(cpustate->sreg[segment].flags & 2)))
			FAULT_THROW(FAULT_GP, 0);
		if((rwn == 1) && ((cpustate->sreg[segment].flags & 8) || !(cpustate->sreg[segment].flags & 2)))
			FAULT_THROW(FAULT_GP, 0);
	}
	return cpustate->sreg[segment].base + ip;
}

#define VTLB_FLAG_DIRTY 0x100

INLINE vtlb_entry get_permissions(UINT32 pte, int wp)
{
	vtlb_entry ret = VTLB_READ_ALLOWED | ((pte & 4) ? VTLB_USER_READ_ALLOWED : 0);
	if(!wp)
		ret |= VTLB_WRITE_ALLOWED;
	if(pte & 2)
		ret |= VTLB_WRITE_ALLOWED | ((pte & 4) ? VTLB_USER_WRITE_ALLOWED : 0);
	return ret;
}

static int i386_translate_address(i386_state *cpustate, int intention, offs_t *address, vtlb_entry *entry)
{
	UINT32 a = *address;
	UINT32 pdbr = cpustate->cr[3] & 0xfffff000;
	UINT32 directory = (a >> 22) & 0x3ff;
	UINT32 table = (a >> 12) & 0x3ff;
	vtlb_entry perm = 0;
	int ret = FALSE;
	bool user = (intention & TRANSLATE_USER_MASK) ? true : false;
	bool write = (intention & TRANSLATE_WRITE) ? true : false;
	bool debug = (intention & TRANSLATE_DEBUG_MASK) ? true : false;

	if(!(cpustate->cr[0] & 0x80000000))
	{
		if(entry)
			*entry = 0x77;
		return TRUE;
	}

	UINT32 page_dir = cpustate->program->read_data32(pdbr + directory * 4);
	if(page_dir & 1)
	{
		if ((page_dir & 0x80) && (cpustate->cr[4] & 0x10))
		{
			a = (page_dir & 0xffc00000) | (a & 0x003fffff);
			if(debug)
			{
				*address = a;
				return TRUE;
			}
			perm = get_permissions(page_dir, WP);
			if(write && (!(perm & VTLB_WRITE_ALLOWED) || (user && !(perm & VTLB_USER_WRITE_ALLOWED))))
				ret = FALSE;
			else if(user && !(perm & VTLB_USER_READ_ALLOWED))
				ret = FALSE;
			else
			{
				if(write)
					perm |= VTLB_FLAG_DIRTY;
				if(!(page_dir & 0x40) && write)
					cpustate->program->write_data32(pdbr + directory * 4, page_dir | 0x60);
				else if(!(page_dir & 0x20))
					cpustate->program->write_data32(pdbr + directory * 4, page_dir | 0x20);
				ret = TRUE;
			}
		}
		else
		{
			UINT32 page_entry = cpustate->program->read_data32((page_dir & 0xfffff000) + (table * 4));
			if(!(page_entry & 1))
				ret = FALSE;
			else
			{
				a = (page_entry & 0xfffff000) | (a & 0xfff);
				if(debug)
				{
					*address = a;
					return TRUE;
				}
				perm = get_permissions(page_entry, WP);
				if(write && (!(perm & VTLB_WRITE_ALLOWED) || (user && !(perm & VTLB_USER_WRITE_ALLOWED))))
					ret = FALSE;
				else if(user && !(perm & VTLB_USER_READ_ALLOWED))
					ret = FALSE;
				else
				{
					if(write)
						perm |= VTLB_FLAG_DIRTY;
					if(!(page_dir & 0x20))
						cpustate->program->write_data32(pdbr + directory * 4, page_dir | 0x20);
					if(!(page_entry & 0x40) && write)
						cpustate->program->write_data32((page_dir & 0xfffff000) + (table * 4), page_entry | 0x60);
					else if(!(page_entry & 0x20))
						cpustate->program->write_data32((page_dir & 0xfffff000) + (table * 4), page_entry | 0x20);
					ret = TRUE;
				}
			}
		}
	}
	else
		ret = FALSE;
	if(entry)
		*entry = perm;
	if(ret)
		*address = a;
	return ret;
}

//#define TEST_TLB

INLINE int translate_address(i386_state *cpustate, int pl, int type, UINT32 *address, UINT32 *error)
{
	if(!(cpustate->cr[0] & 0x80000000)) // Some (very few) old OS's won't work with this
		return TRUE;

	const vtlb_entry *table = vtlb_table(cpustate->vtlb);
	UINT32 index = *address >> 12;
	vtlb_entry entry = table[index];
	if(type == TRANSLATE_FETCH)
		type = TRANSLATE_READ;
	if(pl == 3)
		type |= TRANSLATE_USER_MASK;
#ifdef TEST_TLB
	UINT32 test_addr = *address;
#endif

	if(!(entry & VTLB_FLAG_VALID) || ((type & TRANSLATE_WRITE) && !(entry & VTLB_FLAG_DIRTY)))
	{
		if(!i386_translate_address(cpustate, type, address, &entry))
		{
			*error = ((type & TRANSLATE_WRITE) ? 2 : 0) | ((cpustate->CPL == 3) ? 4 : 0);
			if(entry)
				*error |= 1;
			return FALSE;
		}
		vtlb_dynload(cpustate->vtlb, index, *address, entry);
		return TRUE;
	}
	if(!(entry & (1 << type)))
	{
		*error = ((type & TRANSLATE_WRITE) ? 2 : 0) | ((cpustate->CPL == 3) ? 4 : 0) | 1;
		return FALSE;
	}
	*address = (entry & 0xfffff000) | (*address & 0xfff);
#ifdef TEST_TLB
	int test_ret = i386_translate_address(cpustate, type | TRANSLATE_DEBUG_MASK, &test_addr, NULL);
	if(!test_ret || (test_addr != *address))
		logerror("TLB-PTE mismatch! %06X %06X %06x\n", *address, test_addr, cpustate->pc);
#endif
	return TRUE;
}

INLINE void CHANGE_PC(i386_state *cpustate, UINT32 pc)
{
	cpustate->pc = i386_translate(cpustate, CS, pc, -1, 1 );
}

INLINE void NEAR_BRANCH(i386_state *cpustate, INT32 offs)
{
	/* TODO: limit */
	cpustate->eip += offs;
	cpustate->pc += offs;
}

INLINE UINT8 FETCH(i386_state *cpustate)
{
	UINT8 value;
	UINT32 address = cpustate->pc, error;

	if(!translate_address(cpustate,cpustate->CPL,TRANSLATE_FETCH,&address,&error))
		PF_THROW(error);

	value = cpustate->program->read_data8(address & cpustate->a20_mask);
#ifdef DEBUG_MISSING_OPCODE
	cpustate->opcode_bytes[cpustate->opcode_bytes_length] = value;
	cpustate->opcode_bytes_length = (cpustate->opcode_bytes_length + 1) & 15;
#endif
	cpustate->eip++;
	cpustate->pc++;
	return value;
}
INLINE UINT16 FETCH16(i386_state *cpustate)
{
	UINT16 value;
	UINT32 address = cpustate->pc, error;

	if( !WORD_ALIGNED(address) ) {       /* Unaligned read */
		value = (FETCH(cpustate) << 0);
		value |= (FETCH(cpustate) << 8);
	} else {
		if(!translate_address(cpustate,cpustate->CPL,TRANSLATE_FETCH,&address,&error))
			PF_THROW(error);
		address &= cpustate->a20_mask;
		value = cpustate->program->read_data16(address);
		cpustate->eip += 2;
		cpustate->pc += 2;
	}
	return value;
}
INLINE UINT32 FETCH32(i386_state *cpustate)
{
	UINT32 value;
	UINT32 address = cpustate->pc, error;

	if( !DWORD_ALIGNED(cpustate->pc) ) {      /* Unaligned read */
		value = (FETCH(cpustate) << 0);
		value |= (FETCH(cpustate) << 8);
		value |= (FETCH(cpustate) << 16);
		value |= (FETCH(cpustate) << 24);
	} else {
		if(!translate_address(cpustate,cpustate->CPL,TRANSLATE_FETCH,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		value = cpustate->program->read_data32(address);
		cpustate->eip += 4;
		cpustate->pc += 4;
	}
	return value;
}

INLINE UINT8 READ8(i386_state *cpustate,UINT32 ea)
{
	UINT32 address = ea, error;

	if(!translate_address(cpustate,cpustate->CPL,TRANSLATE_READ,&address, &error))
		PF_THROW(error);

	address &= cpustate->a20_mask;
	return cpustate->program->read_data8(address);
}
INLINE UINT16 READ16(i386_state *cpustate,UINT32 ea)
{
	UINT16 value;
	UINT32 address = ea, error;

	if( !WORD_ALIGNED(ea) ) {        /* Unaligned read */
		value = (READ8( cpustate, address+0 ) << 0);
		value |= (READ8( cpustate, address+1 ) << 8);
	} else {
		if(!translate_address(cpustate,cpustate->CPL,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		value = cpustate->program->read_data16( address );
	}
	return value;
}
INLINE UINT32 READ32(i386_state *cpustate,UINT32 ea)
{
	UINT32 value;
	UINT32 address = ea, error;

	if( !DWORD_ALIGNED(ea) ) {        /* Unaligned read */
		value = (READ8( cpustate, address+0 ) << 0);
		value |= (READ8( cpustate, address+1 ) << 8);
		value |= (READ8( cpustate, address+2 ) << 16),
		value |= (READ8( cpustate, address+3 ) << 24);
	} else {
		if(!translate_address(cpustate,cpustate->CPL,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		value = cpustate->program->read_data32( address );
	}
	return value;
}

INLINE UINT64 READ64(i386_state *cpustate,UINT32 ea)
{
	UINT64 value;
	UINT32 address = ea, error;

	if( !QWORD_ALIGNED(ea) ) {        /* Unaligned read */
		value = (((UINT64) READ8( cpustate, address+0 )) << 0);
		value |= (((UINT64) READ8( cpustate, address+1 )) << 8);
		value |= (((UINT64) READ8( cpustate, address+2 )) << 16);
		value |= (((UINT64) READ8( cpustate, address+3 )) << 24);
		value |= (((UINT64) READ8( cpustate, address+4 )) << 32);
		value |= (((UINT64) READ8( cpustate, address+5 )) << 40);
		value |= (((UINT64) READ8( cpustate, address+6 )) << 48);
		value |= (((UINT64) READ8( cpustate, address+7 )) << 56);
	} else {
		if(!translate_address(cpustate,cpustate->CPL,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		value = (((UINT64) cpustate->program->read_data32( address+0 )) << 0);
		value |= (((UINT64) cpustate->program->read_data32( address+4 )) << 32);
	}
	return value;
}
INLINE UINT8 READ8PL0(i386_state *cpustate,UINT32 ea)
{
	UINT32 address = ea, error;

	if(!translate_address(cpustate,0,TRANSLATE_READ,&address,&error))
		PF_THROW(error);

	address &= cpustate->a20_mask;
	return cpustate->program->read_data8(address);
}
INLINE UINT16 READ16PL0(i386_state *cpustate,UINT32 ea)
{
	UINT16 value;
	UINT32 address = ea, error;

	if( !WORD_ALIGNED(ea) ) {        /* Unaligned read */
		value = (READ8PL0( cpustate, address+0 ) << 0);
		value |= (READ8PL0( cpustate, address+1 ) << 8);
	} else {
		if(!translate_address(cpustate,0,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		value = cpustate->program->read_data16( address );
	}
	return value;
}

INLINE UINT32 READ32PL0(i386_state *cpustate,UINT32 ea)
{
	UINT32 value;
	UINT32 address = ea, error;

	if( !DWORD_ALIGNED(ea) ) {        /* Unaligned read */
		value = (READ8PL0( cpustate, address+0 ) << 0);
		value |= (READ8PL0( cpustate, address+1 ) << 8);
		value |= (READ8PL0( cpustate, address+2 ) << 16);
		value |= (READ8PL0( cpustate, address+3 ) << 24);
	} else {
		if(!translate_address(cpustate,0,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		value = cpustate->program->read_data32( address );
	}
	return value;
}

INLINE void WRITE_TEST(i386_state *cpustate,UINT32 ea)
{
	UINT32 address = ea, error;
	if(!translate_address(cpustate,cpustate->CPL,TRANSLATE_WRITE,&address,&error))
		PF_THROW(error);
}

INLINE void WRITE8(i386_state *cpustate,UINT32 ea, UINT8 value)
{
	UINT32 address = ea, error;

	if(!translate_address(cpustate,cpustate->CPL,TRANSLATE_WRITE,&address,&error))
		PF_THROW(error);

	address &= cpustate->a20_mask;
	cpustate->program->write_data8(address, value);
}
INLINE void WRITE16(i386_state *cpustate,UINT32 ea, UINT16 value)
{
	UINT32 address = ea, error;

	if( !WORD_ALIGNED(ea) ) {        /* Unaligned write */
		WRITE8( cpustate, address+0, value & 0xff );
		WRITE8( cpustate, address+1, (value >> 8) & 0xff );
	} else {
		if(!translate_address(cpustate,cpustate->CPL,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		cpustate->program->write_data16(address, value);
	}
}
INLINE void WRITE32(i386_state *cpustate,UINT32 ea, UINT32 value)
{
	UINT32 address = ea, error;

	if( !DWORD_ALIGNED(ea) ) {        /* Unaligned write */
		WRITE8( cpustate, address+0, value & 0xff );
		WRITE8( cpustate, address+1, (value >> 8) & 0xff );
		WRITE8( cpustate, address+2, (value >> 16) & 0xff );
		WRITE8( cpustate, address+3, (value >> 24) & 0xff );
	} else {
		if(!translate_address(cpustate,cpustate->CPL,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		cpustate->program->write_data32(address, value);
	}
}

INLINE void WRITE64(i386_state *cpustate,UINT32 ea, UINT64 value)
{
	UINT32 address = ea, error;

	if( !QWORD_ALIGNED(ea) ) {        /* Unaligned write */
		WRITE8( cpustate, address+0, value & 0xff );
		WRITE8( cpustate, address+1, (value >> 8) & 0xff );
		WRITE8( cpustate, address+2, (value >> 16) & 0xff );
		WRITE8( cpustate, address+3, (value >> 24) & 0xff );
		WRITE8( cpustate, address+4, (value >> 32) & 0xff );
		WRITE8( cpustate, address+5, (value >> 40) & 0xff );
		WRITE8( cpustate, address+6, (value >> 48) & 0xff );
		WRITE8( cpustate, address+7, (value >> 56) & 0xff );
	} else {
		if(!translate_address(cpustate,cpustate->CPL,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		cpustate->program->write_data32(address+0, value & 0xffffffff);
		cpustate->program->write_data32(address+4, (value >> 32) & 0xffffffff);
	}
}

/***********************************************************************************/

INLINE UINT8 OR8(i386_state *cpustate,UINT8 dst, UINT8 src)
{
	UINT8 res = dst | src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF8(res);
	return res;
}
INLINE UINT16 OR16(i386_state *cpustate,UINT16 dst, UINT16 src)
{
	UINT16 res = dst | src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF16(res);
	return res;
}
INLINE UINT32 OR32(i386_state *cpustate,UINT32 dst, UINT32 src)
{
	UINT32 res = dst | src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF32(res);
	return res;
}

INLINE UINT8 AND8(i386_state *cpustate,UINT8 dst, UINT8 src)
{
	UINT8 res = dst & src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF8(res);
	return res;
}
INLINE UINT16 AND16(i386_state *cpustate,UINT16 dst, UINT16 src)
{
	UINT16 res = dst & src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF16(res);
	return res;
}
INLINE UINT32 AND32(i386_state *cpustate,UINT32 dst, UINT32 src)
{
	UINT32 res = dst & src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF32(res);
	return res;
}

INLINE UINT8 XOR8(i386_state *cpustate,UINT8 dst, UINT8 src)
{
	UINT8 res = dst ^ src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF8(res);
	return res;
}
INLINE UINT16 XOR16(i386_state *cpustate,UINT16 dst, UINT16 src)
{
	UINT16 res = dst ^ src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF16(res);
	return res;
}
INLINE UINT32 XOR32(i386_state *cpustate,UINT32 dst, UINT32 src)
{
	UINT32 res = dst ^ src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF32(res);
	return res;
}

#define SUB8(cpu, dst, src) SBB8(cpu, dst, src, 0)
INLINE UINT8 SBB8(i386_state *cpustate,UINT8 dst, UINT8 src, UINT8 b)
{
	UINT16 res = (UINT16)dst - (UINT16)src - (UINT8)b;
	SetCF8(res);
	SetOF_Sub8(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF8(res);
	return (UINT8)res;
}

#define SUB16(cpu, dst, src) SBB16(cpu, dst, src, 0)
INLINE UINT16 SBB16(i386_state *cpustate,UINT16 dst, UINT16 src, UINT16 b)
{
	UINT32 res = (UINT32)dst - (UINT32)src - (UINT32)b;
	SetCF16(res);
	SetOF_Sub16(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF16(res);
	return (UINT16)res;
}

#define SUB32(cpu, dst, src) SBB32(cpu, dst, src, 0)
INLINE UINT32 SBB32(i386_state *cpustate,UINT32 dst, UINT32 src, UINT32 b)
{
	UINT64 res = (UINT64)dst - (UINT64)src - (UINT64) b;
	SetCF32(res);
	SetOF_Sub32(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF32(res);
	return (UINT32)res;
}

#define ADD8(cpu, dst, src) ADC8(cpu, dst, src, 0)
INLINE UINT8 ADC8(i386_state *cpustate,UINT8 dst, UINT8 src, UINT8 c)
{
	UINT16 res = (UINT16)dst + (UINT16)src + (UINT16)c;
	SetCF8(res);
	SetOF_Add8(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF8(res);
	return (UINT8)res;
}

#define ADD16(cpu, dst, src) ADC16(cpu, dst, src, 0)
INLINE UINT16 ADC16(i386_state *cpustate,UINT16 dst, UINT16 src, UINT8 c)
{
	UINT32 res = (UINT32)dst + (UINT32)src + (UINT32)c;
	SetCF16(res);
	SetOF_Add16(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF16(res);
	return (UINT16)res;
}

#define ADD32(cpu, dst, src) ADC32(cpu, dst, src, 0)
INLINE UINT32 ADC32(i386_state *cpustate,UINT32 dst, UINT32 src, UINT32 c)
{
	UINT64 res = (UINT64)dst + (UINT64)src + (UINT64) c;
	SetCF32(res);
	SetOF_Add32(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF32(res);
	return (UINT32)res;
}

INLINE UINT8 INC8(i386_state *cpustate,UINT8 dst)
{
	UINT16 res = (UINT16)dst + 1;
	SetOF_Add8(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF8(res);
	return (UINT8)res;
}
INLINE UINT16 INC16(i386_state *cpustate,UINT16 dst)
{
	UINT32 res = (UINT32)dst + 1;
	SetOF_Add16(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF16(res);
	return (UINT16)res;
}
INLINE UINT32 INC32(i386_state *cpustate,UINT32 dst)
{
	UINT64 res = (UINT64)dst + 1;
	SetOF_Add32(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF32(res);
	return (UINT32)res;
}

INLINE UINT8 DEC8(i386_state *cpustate,UINT8 dst)
{
	UINT16 res = (UINT16)dst - 1;
	SetOF_Sub8(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF8(res);
	return (UINT8)res;
}
INLINE UINT16 DEC16(i386_state *cpustate,UINT16 dst)
{
	UINT32 res = (UINT32)dst - 1;
	SetOF_Sub16(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF16(res);
	return (UINT16)res;
}
INLINE UINT32 DEC32(i386_state *cpustate,UINT32 dst)
{
	UINT64 res = (UINT64)dst - 1;
	SetOF_Sub32(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF32(res);
	return (UINT32)res;
}



INLINE void PUSH16(i386_state *cpustate,UINT16 value)
{
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 2;
		ea = i386_translate(cpustate, SS, new_esp, 1, 2);
		WRITE16(cpustate, ea, value );
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 2) & 0xffff;
		ea = i386_translate(cpustate, SS, new_esp, 1, 2);
		WRITE16(cpustate, ea, value );
		REG16(SP) = new_esp;
	}
}
INLINE void PUSH32(i386_state *cpustate,UINT32 value)
{
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 4;
		ea = i386_translate(cpustate, SS, new_esp, 1, 4);
		WRITE32(cpustate, ea, value );
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 4) & 0xffff;
		ea = i386_translate(cpustate, SS, new_esp, 1, 4);
		WRITE32(cpustate, ea, value );
		REG16(SP) = new_esp;
	}
}

INLINE void PUSH32SEG(i386_state *cpustate,UINT32 value)
{
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 4;
		if( (cpustate->cpu_version & 0xf00) == 0x300 ) {
			ea = i386_translate(cpustate, SS, new_esp, 1, 2);
			WRITE16(cpustate, ea, value); // 486 also?
		} else {
			ea = i386_translate(cpustate, SS, new_esp, 1, 4);
			WRITE32(cpustate, ea, value ); // 486 also?
		}
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 4) & 0xffff;
		if( (cpustate->cpu_version & 0xf00) == 0x300 ) {
			ea = i386_translate(cpustate, SS, new_esp, 1, 2);
			WRITE16(cpustate, ea, value);
		} else {
			ea = i386_translate(cpustate, SS, new_esp, 1, 4);
			WRITE32(cpustate, ea, value );
		}
		REG16(SP) = new_esp;
	}
}

INLINE void PUSH8(i386_state *cpustate,UINT8 value)
{
	if( cpustate->operand_size ) {
		PUSH32(cpustate,(INT32)(INT8)value);
	} else {
		PUSH16(cpustate,(INT16)(INT8)value);
	}
}

INLINE UINT8 POP8(i386_state *cpustate)
{
	UINT8 value;
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 1;
		ea = i386_translate(cpustate, SS, new_esp - 1, 0, 1);
		value = READ8(cpustate, ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 1;
		ea = i386_translate(cpustate, SS, (new_esp - 1) & 0xffff, 0, 1);
		value = READ8(cpustate, ea );
		REG16(SP) = new_esp;
	}
	return value;
}
INLINE UINT16 POP16(i386_state *cpustate)
{
	UINT16 value;
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 2;
		ea = i386_translate(cpustate, SS, new_esp - 2, 0, 2);
		value = READ16(cpustate, ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 2;
		ea = i386_translate(cpustate, SS, (new_esp - 2) & 0xffff, 0, 2);
		value = READ16(cpustate, ea );
		REG16(SP) = new_esp;
	}
	return value;
}
INLINE UINT32 POP32(i386_state *cpustate)
{
	UINT32 value;
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 4;
		ea = i386_translate(cpustate, SS, new_esp - 4, 0, 4);
		value = READ32(cpustate, ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 4;
		ea = i386_translate(cpustate, SS, (new_esp - 4) & 0xffff, 0, 4);
		value = READ32(cpustate, ea );
		REG16(SP) = new_esp;
	}
	return value;
}

INLINE void BUMP_SI(i386_state *cpustate,int adjustment)
{
	if ( cpustate->address_size )
		REG32(ESI) += ((cpustate->DF) ? -adjustment : +adjustment);
	else
		REG16(SI) += ((cpustate->DF) ? -adjustment : +adjustment);
}

INLINE void BUMP_DI(i386_state *cpustate,int adjustment)
{
	if ( cpustate->address_size )
		REG32(EDI) += ((cpustate->DF) ? -adjustment : +adjustment);
	else
		REG16(DI) += ((cpustate->DF) ? -adjustment : +adjustment);
}



/***********************************************************************************
    I/O ACCESS
***********************************************************************************/

INLINE void check_ioperm(i386_state *cpustate, offs_t port, UINT8 mask)
{
	UINT8 IOPL, map;
	UINT16 IOPB;
	UINT32 address;

	if(!PROTECTED_MODE)
		return;

	IOPL = cpustate->IOP1 | (cpustate->IOP2 << 1);
	if(!V8086_MODE && (cpustate->CPL <= IOPL))
		return;

	if((cpustate->task.limit < 0x67) || ((cpustate->task.flags & 0xd) != 9))
		FAULT_THROW(FAULT_GP,0);

	address = cpustate->task.base;
	IOPB = READ16PL0(cpustate, address+0x66);
	if((IOPB+(port/8)) > cpustate->task.limit)
		FAULT_THROW(FAULT_GP,0);

	map = READ8PL0(cpustate, address+IOPB+(port/8));
	map >>= (port%8);
	if(map & mask)
		FAULT_THROW(FAULT_GP,0);
}

INLINE UINT8 READPORT8(i386_state *cpustate, offs_t port)
{
	check_ioperm(cpustate, port, 1);
	return cpustate->io->read_io8(port);
}

INLINE void WRITEPORT8(i386_state *cpustate, offs_t port, UINT8 value)
{
	check_ioperm(cpustate, port, 1);
	cpustate->io->write_io8(port, value);
}

INLINE UINT16 READPORT16(i386_state *cpustate, offs_t port)
{
	if (port & 1)
	{
		UINT16 value = READPORT8(cpustate, port);
		value |= (READPORT8(cpustate, port + 1) << 8);
		return value;
	}
	else
	{
		check_ioperm(cpustate, port, 3);
		return cpustate->io->read_io16(port);
	}
}

INLINE void WRITEPORT16(i386_state *cpustate, offs_t port, UINT16 value)
{
	if (port & 1)
	{
		WRITEPORT8(cpustate, port, value & 0xff);
		WRITEPORT8(cpustate, port + 1, (value >> 8) & 0xff);
	}
	else
	{
		check_ioperm(cpustate, port, 3);
		cpustate->io->write_io16(port, value);
	}
}

INLINE UINT32 READPORT32(i386_state *cpustate, offs_t port)
{
	if (port & 3)
	{
		UINT32 value = READPORT8(cpustate, port);
		value |= (READPORT8(cpustate, port + 1) << 8);
		value |= (READPORT8(cpustate, port + 2) << 16);
		value |= (READPORT8(cpustate, port + 3) << 24);
		return value;
	}
	else
	{
		check_ioperm(cpustate, port, 0xf);
		return cpustate->io->read_io32(port);
	}
}

INLINE void WRITEPORT32(i386_state *cpustate, offs_t port, UINT32 value)
{
	if (port & 3)
	{
		WRITEPORT8(cpustate, port, value & 0xff);
		WRITEPORT8(cpustate, port + 1, (value >> 8) & 0xff);
		WRITEPORT8(cpustate, port + 2, (value >> 16) & 0xff);
		WRITEPORT8(cpustate, port + 3, (value >> 24) & 0xff);
	}
	else
	{
		check_ioperm(cpustate, port, 0xf);
		cpustate->io->write_io32(port, value);
	}
}

/***********************************************************************************
    MSR ACCESS
***********************************************************************************/

// Pentium MSR handling
UINT64 pentium_msr_read(i386_state *cpustate, UINT32 offset,UINT8 *valid_msr)
{
	switch(offset)
	{
	// Machine Check Exception (TODO)
	case 0x00:
		*valid_msr = 1;
		popmessage("RDMSR: Reading P5_MC_ADDR");
		return 0;
	case 0x01:
		*valid_msr = 1;
		popmessage("RDMSR: Reading P5_MC_TYPE");
		return 0;
	// Time Stamp Counter
	case 0x10:
		*valid_msr = 1;
		popmessage("RDMSR: Reading TSC");
		return cpustate->tsc;
	// Event Counters (TODO)
	case 0x11:  // CESR
		*valid_msr = 1;
		popmessage("RDMSR: Reading CESR");
		return 0;
	case 0x12:  // CTR0
		*valid_msr = 1;
		return cpustate->perfctr[0];
	case 0x13:  // CTR1
		*valid_msr = 1;
		return cpustate->perfctr[1];
	default:
		if(!(offset & ~0xf)) // 2-f are test registers
		{
			*valid_msr = 1;
			logerror("RDMSR: Reading test MSR %x", offset);
			return 0;
		}
		logerror("RDMSR: invalid P5 MSR read %08x at %08x\n",offset,cpustate->pc-2);
		*valid_msr = 0;
		return 0;
	}
	return -1;
}

void pentium_msr_write(i386_state *cpustate, UINT32 offset, UINT64 data, UINT8 *valid_msr)
{
	switch(offset)
	{
	// Machine Check Exception (TODO)
	case 0x00:
		popmessage("WRMSR: Writing P5_MC_ADDR");
		*valid_msr = 1;
		break;
	case 0x01:
		popmessage("WRMSR: Writing P5_MC_TYPE");
		*valid_msr = 1;
		break;
	// Time Stamp Counter
	case 0x10:
		cpustate->tsc = data;
		popmessage("WRMSR: Writing to TSC");
		*valid_msr = 1;
		break;
	// Event Counters (TODO)
	case 0x11:  // CESR
		popmessage("WRMSR: Writing to CESR");
		*valid_msr = 1;
		break;
	case 0x12:  // CTR0
		cpustate->perfctr[0] = data;
		*valid_msr = 1;
		break;
	case 0x13:  // CTR1
		cpustate->perfctr[1] = data;
		*valid_msr = 1;
		break;
	default:
		if(!(offset & ~0xf)) // 2-f are test registers
		{
			*valid_msr = 1;
			logerror("WRMSR: Writing test MSR %x", offset);
			break;
		}
		logerror("WRMSR: invalid MSR write %08x (%08x%08x) at %08x\n",offset,(UINT32)(data >> 32),(UINT32)data,cpustate->pc-2);
		*valid_msr = 0;
		break;
	}
}

// P6 (Pentium Pro, Pentium II, Pentium III) MSR handling
UINT64 p6_msr_read(i386_state *cpustate, UINT32 offset,UINT8 *valid_msr)
{
	switch(offset)
	{
	// Machine Check Exception (TODO)
	case 0x00:
		*valid_msr = 1;
		popmessage("RDMSR: Reading P5_MC_ADDR");
		return 0;
	case 0x01:
		*valid_msr = 1;
		popmessage("RDMSR: Reading P5_MC_TYPE");
		return 0;
	// Time Stamp Counter
	case 0x10:
		*valid_msr = 1;
		popmessage("RDMSR: Reading TSC");
		return cpustate->tsc;
	// Performance Counters (TODO)
	case 0xc1:  // PerfCtr0
		*valid_msr = 1;
		return cpustate->perfctr[0];
	case 0xc2:  // PerfCtr1
		*valid_msr = 1;
		return cpustate->perfctr[1];
	default:
		logerror("RDMSR: unimplemented register called %08x at %08x\n",offset,cpustate->pc-2);
		*valid_msr = 1;
		return 0;
	}
	return -1;
}

void p6_msr_write(i386_state *cpustate, UINT32 offset, UINT64 data, UINT8 *valid_msr)
{
	switch(offset)
	{
	// Time Stamp Counter
	case 0x10:
		cpustate->tsc = data;
		popmessage("WRMSR: Writing to TSC");
		*valid_msr = 1;
		break;
	// Performance Counters (TODO)
	case 0xc1:  // PerfCtr0
		cpustate->perfctr[0] = data;
		*valid_msr = 1;
		break;
	case 0xc2:  // PerfCtr1
		cpustate->perfctr[1] = data;
		*valid_msr = 1;
		break;
	default:
		logerror("WRMSR: unimplemented register called %08x (%08x%08x) at %08x\n",offset,(UINT32)(data >> 32),(UINT32)data,cpustate->pc-2);
		*valid_msr = 1;
		break;
	}
}

// PIV (Pentium 4+)
UINT64 piv_msr_read(i386_state *cpustate, UINT32 offset,UINT8 *valid_msr)
{
	switch(offset)
	{
	default:
		logerror("RDMSR: unimplemented register called %08x at %08x\n",offset,cpustate->pc-2);
		*valid_msr = 1;
		return 0;
	}
	return -1;
}

void piv_msr_write(i386_state *cpustate, UINT32 offset, UINT64 data, UINT8 *valid_msr)
{
	switch(offset)
	{
	default:
		logerror("WRMSR: unimplemented register called %08x (%08x%08x) at %08x\n",offset,(UINT32)(data >> 32),(UINT32)data,cpustate->pc-2);
		*valid_msr = 1;
		break;
	}
}

INLINE UINT64 MSR_READ(i386_state *cpustate, UINT32 offset,UINT8 *valid_msr)
{
	UINT64 res;
	UINT8 cpu_type = (cpustate->cpu_version >> 8) & 0x0f;

	*valid_msr = 0;

	switch(cpu_type)
	{
	case 5:  // Pentium
		res = pentium_msr_read(cpustate,offset,valid_msr);
		break;
	case 6:  // Pentium Pro, Pentium II, Pentium III
		res = p6_msr_read(cpustate,offset,valid_msr);
		break;
	case 15:  // Pentium 4+
		res = piv_msr_read(cpustate,offset,valid_msr);
		break;
	default:
		res = 0;
		break;
	}

	return res;
}

INLINE void MSR_WRITE(i386_state *cpustate, UINT32 offset, UINT64 data, UINT8 *valid_msr)
{
	*valid_msr = 0;
	UINT8 cpu_type = (cpustate->cpu_version >> 8) & 0x0f;

	switch(cpu_type)
	{
	case 5:  // Pentium
		pentium_msr_write(cpustate,offset,data,valid_msr);
		break;
	case 6:  // Pentium Pro, Pentium II, Pentium III
		p6_msr_write(cpustate,offset,data,valid_msr);
		break;
	case 15:  // Pentium 4+
		piv_msr_write(cpustate,offset,data,valid_msr);
		break;
	}
}

#endif /* __I386_H__ */
