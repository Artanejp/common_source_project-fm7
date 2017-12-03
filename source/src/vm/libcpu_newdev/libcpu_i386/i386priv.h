// license:BSD-3-Clause
// copyright-holders:Ville Linde, Barry Rodewald, Carl, Philp Bennett
#pragma once

#ifndef __LIB_I386_PRIV_H__
#define __LIB_I386_PRIV_H__
#include "../../../common.h"

#define INLINE inline

// Translation intentions
const int TRANSLATE_TYPE_MASK       = 0x03;     // read write or fetch
const int TRANSLATE_USER_MASK       = 0x04;     // user mode or fully privileged
const int TRANSLATE_DEBUG_MASK      = 0x08;     // debug mode (no side effects)

const int TRANSLATE_READ            = 0;        // translate for read
const int TRANSLATE_WRITE           = 1;        // translate for write
const int TRANSLATE_FETCH           = 2;        // translate for instruction fetch
const int TRANSLATE_READ_USER       = (TRANSLATE_READ | TRANSLATE_USER_MASK);
const int TRANSLATE_WRITE_USER      = (TRANSLATE_WRITE | TRANSLATE_USER_MASK);
const int TRANSLATE_FETCH_USER      = (TRANSLATE_FETCH | TRANSLATE_USER_MASK);
const int TRANSLATE_READ_DEBUG      = (TRANSLATE_READ | TRANSLATE_DEBUG_MASK);
const int TRANSLATE_WRITE_DEBUG     = (TRANSLATE_WRITE | TRANSLATE_DEBUG_MASK);
const int TRANSLATE_FETCH_DEBUG     = (TRANSLATE_FETCH | TRANSLATE_DEBUG_MASK);

/*****************************************************************************/
/* src/emu/emucore.h */

// constants for expression endianness
enum endianness_t
{
	ENDIANNESS_LITTLE,
	ENDIANNESS_BIG
};

// declare native endianness to be one or the other
#ifdef LSB_FIRST
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_LITTLE;
#else
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_BIG;
#endif
// endian-based value: first value is if 'endian' is little-endian, second is if 'endian' is big-endian
#define ENDIAN_VALUE_LE_BE(endian,leval,beval)	(((endian) == ENDIANNESS_LITTLE) ? (leval) : (beval))
// endian-based value: first value is if native endianness is little-endian, second is if native is big-endian
#define NATIVE_ENDIAN_VALUE_LE_BE(leval,beval)	ENDIAN_VALUE_LE_BE(ENDIANNESS_NATIVE, leval, beval)
// endian-based value: first value is if 'endian' matches native, second is if 'endian' doesn't match native
#define ENDIAN_VALUE_NE_NNE(endian,leval,beval)	(((endian) == ENDIANNESS_NATIVE) ? (neval) : (nneval))

/*****************************************************************************/
/* src/emu/emumem.h */

// helpers for checking address alignment
#define WORD_ALIGNED(a)                 (((a) & 1) == 0)
#define DWORD_ALIGNED(a)                (((a) & 3) == 0)
#define QWORD_ALIGNED(a)                (((a) & 7) == 0)

/*****************************************************************************/
/* src/emu/memory.h */

// address spaces
enum address_spacenum
{
	AS_0,                           // first address space
	AS_1,                           // second address space
	AS_2,                           // third address space
	AS_3,                           // fourth address space
	ADDRESS_SPACES,                 // maximum number of address spaces

	// alternate address space names for common use
	AS_PROGRAM = AS_0,              // program address space
	AS_DATA = AS_1,                 // data address space
	AS_IO = AS_2                    // I/O address space
};

// offsets and addresses are 32-bit (for now...)
typedef UINT32	offs_t;

/*****************************************************************************/
/* src/osd/osdcomm.h */

/* Highly useful macro for compile-time knowledge of an array size */
#define ARRAY_LENGTH(x)     (sizeof(x) / sizeof(x[0]))

#include "../libcpu_softfloat/milieu.h"
#include "../libcpu_softfloat/softfloat.h"
#include "./vtlb.h"
#include <math.h>

//#define DEBUG_MISSING_OPCODE

#define I386OP(XX)      I386_OPS_BASE::i386_##XX
#define I486OP(XX)      I386_OPS_BASE::i486_##XX
#define PENTIUMOP(XX)   I386_OPS_BASE::pentium_##XX
#define MMXOP(XX)       I386_OPS_BASE::mmx_##XX
#define SSEOP(XX)       I386_OPS_BASE::sse_##XX

#define I386OP_D(XX)      i386_##XX
#define I486OP_D(XX)      i486_##XX
#define PENTIUMOP_D(XX)   pentium_##XX
#define MMXOP_D(XX)       mmx_##XX
#define SSEOP_D(XX)       sse_##XX

//extern int i386_dasm_one(_TCHAR *buffer, UINT32 pc, const UINT8 *oprom, int mode);

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

class EMU;
class DEBUGGER;
class DEVICE;
class I386_OPS_BASE;
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

	uint64_t total_cycles;
	uint64_t prev_total_cycles;
	
	int cycles;
	int extra_cycles;
	int base_cycles;
	UINT8 opcode;

	UINT8 irq_state;
	DEVICE *pic;
	DEVICE *program;
	DEVICE *io;
//#ifdef I386_PSEUDO_BIOS
	DEVICE *bios;
//#endif
//#ifdef SINGLE_MODE_DMA
	DEVICE *dma;
//#endif
//#ifdef USE_DEBUGGER
	EMU *emu;
	DEBUGGER *debugger;
	DEVICE *program_stored;
	DEVICE *io_stored;
//#endif
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

	void (I386_OPS_BASE::*opcode_table_x87_d8[256])(UINT8 modrm);
	void (I386_OPS_BASE::*opcode_table_x87_d9[256])(UINT8 modrm);
	void (I386_OPS_BASE::*opcode_table_x87_da[256])(UINT8 modrm);
	void (I386_OPS_BASE::*opcode_table_x87_db[256])(UINT8 modrm);
	void (I386_OPS_BASE::*opcode_table_x87_dc[256])(UINT8 modrm);
	void (I386_OPS_BASE::*opcode_table_x87_dd[256])(UINT8 modrm);
	void (I386_OPS_BASE::*opcode_table_x87_de[256])(UINT8 modrm);
	void (I386_OPS_BASE::*opcode_table_x87_df[256])(UINT8 modrm);

	// SSE
	XMM_REG sse_reg[8];
	UINT32 mxcsr;

	void (I386_OPS_BASE::*opcode_table1_16[256])();
	void (I386_OPS_BASE::*opcode_table1_32[256])();
	void (I386_OPS_BASE::*opcode_table2_16[256])();
	void (I386_OPS_BASE::*opcode_table2_32[256])();
	void (I386_OPS_BASE::*opcode_table338_16[256])();
	void (I386_OPS_BASE::*opcode_table338_32[256])();
	void (I386_OPS_BASE::*opcode_table33a_16[256])();
	void (I386_OPS_BASE::*opcode_table33a_32[256])();
	void (I386_OPS_BASE::*opcode_table366_16[256])();
	void (I386_OPS_BASE::*opcode_table366_32[256])();
	void (I386_OPS_BASE::*opcode_table3f2_16[256])();
	void (I386_OPS_BASE::*opcode_table3f2_32[256])();
	void (I386_OPS_BASE::*opcode_table3f3_16[256])();
	void (I386_OPS_BASE::*opcode_table3f3_32[256])();
	void (I386_OPS_BASE::*opcode_table46638_16[256])();
	void (I386_OPS_BASE::*opcode_table46638_32[256])();
	void (I386_OPS_BASE::*opcode_table4f238_16[256])();
	void (I386_OPS_BASE::*opcode_table4f238_32[256])();
	void (I386_OPS_BASE::*opcode_table4f338_16[256])();
	void (I386_OPS_BASE::*opcode_table4f338_32[256])();
	void (I386_OPS_BASE::*opcode_table4663a_16[256])();
	void (I386_OPS_BASE::*opcode_table4663a_32[256])();
	void (I386_OPS_BASE::*opcode_table4f23a_16[256])();
	void (I386_OPS_BASE::*opcode_table4f23a_32[256])();

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


#endif /* __I386_H__ */
