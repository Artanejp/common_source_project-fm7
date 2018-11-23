
#ifndef __LIB_I386_OPDEF_H__
#define __LIB_I386_OPDEF_H__

#include "../../../common.h"
#ifndef __BIG_ENDIAN__
#define LSB_FIRST
#endif
#include "../../../fileio.h"
#include "../device.h"
#include "./i386priv.h"
#include "./i386ops.h"
#include "./vtlb.h"

#ifndef INLINE
#define INLINE inline
#endif

#define U64(v) UINT64(v)

//#define fatalerror(...) exit(1)
#define fatalerror(...)
#define logerror(...)
#define popmessage(...)

#define X86_NUM_CPUS        4
#define CPU_CYCLES_I386         0
#define CPU_CYCLES_I486         1
#define CPU_CYCLES_PENTIUM      2
#define CPU_CYCLES_MEDIAGX      3

enum X86_CYCLES
{
	CYCLES_MOV_REG_REG,
	CYCLES_MOV_REG_MEM,
	CYCLES_MOV_MEM_REG,
	CYCLES_MOV_IMM_REG,
	CYCLES_MOV_IMM_MEM,
	CYCLES_MOV_ACC_MEM,
	CYCLES_MOV_MEM_ACC,
	CYCLES_MOV_REG_SREG,
	CYCLES_MOV_MEM_SREG,
	CYCLES_MOV_SREG_REG,
	CYCLES_MOV_SREG_MEM,
	CYCLES_MOVSX_REG_REG,
	CYCLES_MOVSX_MEM_REG,
	CYCLES_MOVZX_REG_REG,
	CYCLES_MOVZX_MEM_REG,
	CYCLES_PUSH_RM,
	CYCLES_PUSH_REG_SHORT,
	CYCLES_PUSH_SREG,
	CYCLES_PUSH_IMM,
	CYCLES_PUSHA,
	CYCLES_POP_RM,
	CYCLES_POP_REG_SHORT,
	CYCLES_POP_SREG,
	CYCLES_POPA,
	CYCLES_XCHG_REG_REG,
	CYCLES_XCHG_REG_MEM,
	CYCLES_IN,
	CYCLES_IN_VAR,
	CYCLES_OUT,
	CYCLES_OUT_VAR,
	CYCLES_LEA,
	CYCLES_LDS,
	CYCLES_LES,
	CYCLES_LFS,
	CYCLES_LGS,
	CYCLES_LSS,
	CYCLES_CLC,
	CYCLES_CLD,
	CYCLES_CLI,
	CYCLES_CLTS,
	CYCLES_CMC,
	CYCLES_LAHF,
	CYCLES_POPF,
	CYCLES_PUSHF,
	CYCLES_SAHF,
	CYCLES_STC,
	CYCLES_STD,
	CYCLES_STI,
	CYCLES_ALU_REG_REG,
	CYCLES_ALU_REG_MEM,
	CYCLES_ALU_MEM_REG,
	CYCLES_ALU_IMM_REG,
	CYCLES_ALU_IMM_MEM,
	CYCLES_ALU_IMM_ACC,
	CYCLES_INC_REG,
	CYCLES_INC_MEM,
	CYCLES_DEC_REG,
	CYCLES_DEC_MEM,
	CYCLES_CMP_REG_REG,
	CYCLES_CMP_REG_MEM,
	CYCLES_CMP_MEM_REG,
	CYCLES_CMP_IMM_REG,
	CYCLES_CMP_IMM_MEM,
	CYCLES_CMP_IMM_ACC,
	CYCLES_TEST_REG_REG,
	CYCLES_TEST_REG_MEM,
	CYCLES_TEST_IMM_REG,
	CYCLES_TEST_IMM_MEM,
	CYCLES_TEST_IMM_ACC,
	CYCLES_NEG_REG,
	CYCLES_NEG_MEM,
	CYCLES_AAA,
	CYCLES_AAS,
	CYCLES_DAA,
	CYCLES_DAS,
	CYCLES_MUL8_ACC_REG,
	CYCLES_MUL8_ACC_MEM,
	CYCLES_MUL16_ACC_REG,
	CYCLES_MUL16_ACC_MEM,
	CYCLES_MUL32_ACC_REG,
	CYCLES_MUL32_ACC_MEM,
	CYCLES_IMUL8_ACC_REG,
	CYCLES_IMUL8_ACC_MEM,
	CYCLES_IMUL16_ACC_REG,
	CYCLES_IMUL16_ACC_MEM,
	CYCLES_IMUL32_ACC_REG,
	CYCLES_IMUL32_ACC_MEM,
	CYCLES_IMUL8_REG_REG,
	CYCLES_IMUL8_REG_MEM,
	CYCLES_IMUL16_REG_REG,
	CYCLES_IMUL16_REG_MEM,
	CYCLES_IMUL32_REG_REG,
	CYCLES_IMUL32_REG_MEM,
	CYCLES_IMUL16_REG_IMM_REG,
	CYCLES_IMUL16_MEM_IMM_REG,
	CYCLES_IMUL32_REG_IMM_REG,
	CYCLES_IMUL32_MEM_IMM_REG,
	CYCLES_DIV8_ACC_REG,
	CYCLES_DIV8_ACC_MEM,
	CYCLES_DIV16_ACC_REG,
	CYCLES_DIV16_ACC_MEM,
	CYCLES_DIV32_ACC_REG,
	CYCLES_DIV32_ACC_MEM,
	CYCLES_IDIV8_ACC_REG,
	CYCLES_IDIV8_ACC_MEM,
	CYCLES_IDIV16_ACC_REG,
	CYCLES_IDIV16_ACC_MEM,
	CYCLES_IDIV32_ACC_REG,
	CYCLES_IDIV32_ACC_MEM,
	CYCLES_AAD,
	CYCLES_AAM,
	CYCLES_CBW,
	CYCLES_CWD,
	CYCLES_ROTATE_REG,
	CYCLES_ROTATE_MEM,
	CYCLES_ROTATE_CARRY_REG,
	CYCLES_ROTATE_CARRY_MEM,
	CYCLES_SHLD_REG,
	CYCLES_SHLD_MEM,
	CYCLES_SHRD_REG,
	CYCLES_SHRD_MEM,
	CYCLES_NOT_REG,
	CYCLES_NOT_MEM,
	CYCLES_CMPS,
	CYCLES_INS,
	CYCLES_LODS,
	CYCLES_MOVS,
	CYCLES_OUTS,
	CYCLES_SCAS,
	CYCLES_STOS,
	CYCLES_XLAT,
	CYCLES_REP_CMPS_BASE,
	CYCLES_REP_INS_BASE,
	CYCLES_REP_LODS_BASE,
	CYCLES_REP_MOVS_BASE,
	CYCLES_REP_OUTS_BASE,
	CYCLES_REP_SCAS_BASE,
	CYCLES_REP_STOS_BASE,
	CYCLES_REP_CMPS,
	CYCLES_REP_INS,
	CYCLES_REP_LODS,
	CYCLES_REP_MOVS,
	CYCLES_REP_OUTS,
	CYCLES_REP_SCAS,
	CYCLES_REP_STOS,
	CYCLES_BSF_BASE,
	CYCLES_BSF,
	CYCLES_BSR_BASE,
	CYCLES_BSR,
	CYCLES_BT_IMM_REG,
	CYCLES_BT_IMM_MEM,
	CYCLES_BT_REG_REG,
	CYCLES_BT_REG_MEM,
	CYCLES_BTC_IMM_REG,
	CYCLES_BTC_IMM_MEM,
	CYCLES_BTC_REG_REG,
	CYCLES_BTC_REG_MEM,
	CYCLES_BTR_IMM_REG,
	CYCLES_BTR_IMM_MEM,
	CYCLES_BTR_REG_REG,
	CYCLES_BTR_REG_MEM,
	CYCLES_BTS_IMM_REG,
	CYCLES_BTS_IMM_MEM,
	CYCLES_BTS_REG_REG,
	CYCLES_BTS_REG_MEM,
	CYCLES_CALL,                // E8
	CYCLES_CALL_REG,            // FF /2
	CYCLES_CALL_MEM,            // FF /2
	CYCLES_CALL_INTERSEG,       // 9A
	CYCLES_CALL_REG_INTERSEG,   // FF /3
	CYCLES_CALL_MEM_INTERSEG,   // FF /3
	CYCLES_JMP_SHORT,           // EB
	CYCLES_JMP,                 // E9
	CYCLES_JMP_REG,             // FF /4
	CYCLES_JMP_MEM,             // FF /4
	CYCLES_JMP_INTERSEG,        // EA
	CYCLES_JMP_REG_INTERSEG,    // FF /5
	CYCLES_JMP_MEM_INTERSEG,    // FF /5
	CYCLES_RET,                 // C3
	CYCLES_RET_IMM,             // C2
	CYCLES_RET_INTERSEG,        // CB
	CYCLES_RET_IMM_INTERSEG,    // CA
	CYCLES_JCC_DISP8,
	CYCLES_JCC_FULL_DISP,
	CYCLES_JCC_DISP8_NOBRANCH,
	CYCLES_JCC_FULL_DISP_NOBRANCH,
	CYCLES_JCXZ,
	CYCLES_JCXZ_NOBRANCH,
	CYCLES_LOOP,
	CYCLES_LOOPZ,
	CYCLES_LOOPNZ,
	CYCLES_SETCC_REG,
	CYCLES_SETCC_MEM,
	CYCLES_ENTER,
	CYCLES_LEAVE,
	CYCLES_INT,
	CYCLES_INT3,
	CYCLES_INTO_OF1,
	CYCLES_INTO_OF0,
	CYCLES_BOUND_IN_RANGE,
	CYCLES_BOUND_OUT_RANGE,
	CYCLES_IRET,
	CYCLES_HLT,
	CYCLES_MOV_REG_CR0,
	CYCLES_MOV_REG_CR2,
	CYCLES_MOV_REG_CR3,
	CYCLES_MOV_CR_REG,
	CYCLES_MOV_REG_DR0_3,
	CYCLES_MOV_REG_DR6_7,
	CYCLES_MOV_DR6_7_REG,
	CYCLES_MOV_DR0_3_REG,
	CYCLES_MOV_REG_TR6_7,
	CYCLES_MOV_TR6_7_REG,
	CYCLES_NOP,
	CYCLES_WAIT,
	CYCLES_ARPL_REG,
	CYCLES_ARPL_MEM,
	CYCLES_LAR_REG,
	CYCLES_LAR_MEM,
	CYCLES_LGDT,
	CYCLES_LIDT,
	CYCLES_LLDT_REG,
	CYCLES_LLDT_MEM,
	CYCLES_LMSW_REG,
	CYCLES_LMSW_MEM,
	CYCLES_LSL_REG,
	CYCLES_LSL_MEM,
	CYCLES_LTR_REG,
	CYCLES_LTR_MEM,
	CYCLES_SGDT,
	CYCLES_SIDT,
	CYCLES_SLDT_REG,
	CYCLES_SLDT_MEM,
	CYCLES_SMSW_REG,
	CYCLES_SMSW_MEM,
	CYCLES_STR_REG,
	CYCLES_STR_MEM,
	CYCLES_VERR_REG,
	CYCLES_VERR_MEM,
	CYCLES_VERW_REG,
	CYCLES_VERW_MEM,
	CYCLES_LOCK,

	CYCLES_BSWAP,
	CYCLES_CMPXCHG8B,
	CYCLES_CMPXCHG,
	CYCLES_CPUID,
	CYCLES_CPUID_EAX1,
	CYCLES_INVD,
	CYCLES_XADD,
	CYCLES_RDTSC,
	CYCLES_RSM,
	CYCLES_RDMSR,

	CYCLES_FABS,
	CYCLES_FADD,
	CYCLES_FBLD,
	CYCLES_FBSTP,
	CYCLES_FCHS,
	CYCLES_FCLEX,
	CYCLES_FCOM,
	CYCLES_FCOS,
	CYCLES_FDECSTP,
	CYCLES_FDISI,
	CYCLES_FDIV,
	CYCLES_FDIVR,
	CYCLES_FENI,
	CYCLES_FFREE,
	CYCLES_FIADD,
	CYCLES_FICOM,
	CYCLES_FIDIV,
	CYCLES_FILD,
	CYCLES_FIMUL,
	CYCLES_FINCSTP,
	CYCLES_FINIT,
	CYCLES_FIST,
	CYCLES_FISUB,
	CYCLES_FLD,
	CYCLES_FLDZ,
	CYCLES_FLD1,
	CYCLES_FLDL2E,
	CYCLES_FLDL2T,
	CYCLES_FLDLG2,
	CYCLES_FLDLN2,
	CYCLES_FLDPI,
	CYCLES_FLDCW,
	CYCLES_FLDENV,
	CYCLES_FMUL,
	CYCLES_FNOP,
	CYCLES_FPATAN,
	CYCLES_FPREM,
	CYCLES_FPREM1,
	CYCLES_FPTAN,
	CYCLES_FRNDINT,
	CYCLES_FRSTOR,
	CYCLES_FSAVE,
	CYCLES_FSCALE,
	CYCLES_FSETPM,
	CYCLES_FSIN,
	CYCLES_FSINCOS,
	CYCLES_FSQRT,
	CYCLES_FST,
	CYCLES_FSTCW,
	CYCLES_FSTENV,
	CYCLES_FSTSW,
	CYCLES_FSUB,
	CYCLES_FSUBR,
	CYCLES_FTST,
	CYCLES_FUCOM,
	CYCLES_FXAM,
	CYCLES_FXCH,
	CYCLES_FXTRACT,
	CYCLES_FYL2X,
	CYCLES_FYL2XPI,
	CYCLES_CMPXCHG_REG_REG_T,
	CYCLES_CMPXCHG_REG_REG_F,
	CYCLES_CMPXCHG_REG_MEM_T,
	CYCLES_CMPXCHG_REG_MEM_F,
	CYCLES_XADD_REG_REG,
	CYCLES_XADD_REG_MEM,

	CYCLES_NUM_OPCODES
};

struct X86_CYCLE_TABLE
{
	X86_CYCLES op;
	UINT8 cpu_cycles[X86_NUM_CPUS][2];
};

/*************************************
 *
 * Defines
 *
 *************************************/

#define X87_SW_IE               0x0001
#define X87_SW_DE               0x0002
#define X87_SW_ZE               0x0004
#define X87_SW_OE               0x0008
#define X87_SW_UE               0x0010
#define X87_SW_PE               0x0020
#define X87_SW_SF               0x0040
#define X87_SW_ES               0x0080
#define X87_SW_C0               0x0100
#define X87_SW_C1               0x0200
#define X87_SW_C2               0x0400
#define X87_SW_TOP_SHIFT        11
#define X87_SW_TOP_MASK         7
#define X87_SW_C3               0x4000
#define X87_SW_BUSY             0x8000

#define X87_CW_IM               0x0001
#define X87_CW_DM               0x0002
#define X87_CW_ZM               0x0004
#define X87_CW_OM               0x0008
#define X87_CW_UM               0x0010
#define X87_CW_PM               0x0020
#define X87_CW_IEM              0x0080
#define X87_CW_PC_SHIFT         8
#define X87_CW_PC_MASK          3
#define X87_CW_PC_SINGLE        0
#define X87_CW_PC_DOUBLE        2
#define X87_CW_PC_EXTEND        3
#define X87_CW_RC_SHIFT         10
#define X87_CW_RC_MASK          3
#define X87_CW_RC_NEAREST       0
#define X87_CW_RC_DOWN          1
#define X87_CW_RC_UP            2
#define X87_CW_RC_ZERO          3

#define X87_TW_MASK             3
#define X87_TW_VALID            0
#define X87_TW_ZERO             1
#define X87_TW_SPECIAL          2
#define X87_TW_EMPTY            3


/*************************************
 *
 * Macros
 *
 *************************************/

#define ST_TO_PHYS(x)           (((cpustate->x87_sw >> X87_SW_TOP_SHIFT) + (x)) & X87_SW_TOP_MASK)
#define ST(x)                   (cpustate->x87_reg[ST_TO_PHYS(x)])
#define X87_TW_FIELD_SHIFT(x)   ((x) << 1)
#define X87_TAG(x)              ((cpustate->x87_tw >> X87_TW_FIELD_SHIFT(x)) & X87_TW_MASK)
#define X87_RC                  ((cpustate->x87_cw >> X87_CW_RC_SHIFT) & X87_CW_RC_MASK)
#define X87_IS_ST_EMPTY(x)      (X87_TAG(ST_TO_PHYS(x)) == X87_TW_EMPTY)
#define X87_SW_C3_0             X87_SW_C0

#define UNIMPLEMENTED           fatalerror("Unimplemented x87 op: %s (PC:%x)\n", __FUNCTION__, cpustate->pc)

/*****************************************************************************/
/* src/emu/devcpu.h */

// CPU interface functions
#define CPU_INIT_NAME(name)			I386_OPS_BASE::cpu_init_##name
#define CPU_INIT(name)				void* CPU_INIT_NAME(name)()
#define CPU_INIT_CALL_NAME(name)	cpu_init_##name
#define CPU_INIT_CALL(name)			CPU_INIT_CALL_NAME(name)()

#define CPU_RESET_NAME(name)		I386_OPS_BASE::cpu_reset_##name
#define CPU_RESET(name)				void CPU_RESET_NAME(name)()
#define CPU_RESET_CALL_NAME(name)	cpu_reset_##name
#define CPU_RESET_CALL(name)		CPU_RESET_CALL_NAME(name)()

#define CPU_EXECUTE_NAME(name)		I386_OPS_BASE::cpu_execute_##name
#define CPU_EXECUTE(name)			int CPU_EXECUTE_NAME(name)(int cycles)
#define CPU_EXECUTE_CALL_NAME(name)	cpu_execute_##name
#define CPU_EXECUTE_CALL(name)		CPU_EXECUTE_CALL_NAME(name)(cycles)

#define CPU_TRANSLATE_NAME(name)	I386_OPS_BASE::cpu_translate_##name
#define CPU_TRANSLATE(name)			int CPU_TRANSLATE_NAME(name)(void *cpudevice, address_spacenum space, int intention, offs_t *address)
#define CPU_TRANSLATE_CALL_NAME(name)	cpu_translate_##name
#define CPU_TRANSLATE_CALL(name)	CPU_TRANSLATE_CALL_NAME(name)(cpudevice, space, intention, address)

#define CPU_DISASSEMBLE_NAME(name)	I386_OPS_BASE::cpu_disassemble_##name
#define CPU_DISASSEMBLE(name)		int CPU_DISASSEMBLE_NAME(name)(_TCHAR *buffer, offs_t eip, const UINT8 *oprom)
#define CPU_DISASSEMBLE_CALL_NAME(name)	cpu_disassemble_##name
#define CPU_DISASSEMBLE_CALL(name)	CPU_DISASSEMBLE_CALL_NAME(name)(buffer, eip, oprom)

/*****************************************************************************/
/* src/emu/didisasm.h */

// Disassembler constants
const UINT32 DASMFLAG_SUPPORTED     = 0x80000000;   // are disassembly flags supported?
const UINT32 DASMFLAG_STEP_OUT      = 0x40000000;   // this instruction should be the end of a step out sequence
const UINT32 DASMFLAG_STEP_OVER     = 0x20000000;   // this instruction should be stepped over by setting a breakpoint afterwards
const UINT32 DASMFLAG_OVERINSTMASK  = 0x18000000;   // number of extra instructions to skip when stepping over
const UINT32 DASMFLAG_OVERINSTSHIFT = 27;           // bits to shift after masking to get the value
const UINT32 DASMFLAG_LENGTHMASK    = 0x0000ffff;   // the low 16-bits contain the actual length

/*****************************************************************************/
/* src/emu/diexec.h */

// I/O line states
enum line_state
{
	CLEAR_LINE = 0,				// clear (a fired or held) line
	ASSERT_LINE,				// assert an interrupt immediately
	HOLD_LINE,				// hold interrupt line until acknowledged
	PULSE_LINE				// pulse interrupt line instantaneously (only for NMI, RESET)
};

enum
{
	INPUT_LINE_IRQ = 0,
	INPUT_LINE_NMI
};

/*****************************************************************************/
/* src/emu/dimemory.h */

enum {
	I386_OPS_CPUTYPE_I386 = 0,
	I386_OPS_CPUTYPE_I486,
	I386_OPS_CPUTYPE_PENTIUM,
	I386_OPS_CPUTYPE_MEDIAGX,
	I386_OPS_CPUTYPE_PENTIUM_PRO,
	I386_OPS_CPUTYPE_PENTIUM_MMX,
	I386_OPS_CPUTYPE_PENTIUM2,
	I386_OPS_CPUTYPE_PENTIUM3,
	I386_OPS_CPUTYPE_PENTIUM4,
	I386_OPS_CPUTYPE_IX87FPU,
	I386_OPS_CPUTYPE_END
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
/* VTLB state */
struct vtlb_state
{
	void *              cpudevice;          /* CPU device */
	address_spacenum    space;              /* address space */
	int                 dynamic;            /* number of dynamic entries */
	int                 fixed;              /* number of fixed entries */
	int                 dynindex;           /* index of next dynamic entry */
	int                 pageshift;          /* bits to shift to get page index */
	int                 addrwidth;          /* logical address bus width */
	offs_t *            live;               /* array of live entries by table index */
	int *               fixedpages;         /* number of pages each fixed entry covers */
	vtlb_entry *        table;              /* table of entries by address */
	vtlb_entry *        save;               /* cache of live table entries for saving */
};

class DEBUG;
class I386_OPS_BASE {
protected:
	i386_state *cpustate;
	int _cputype;
// Parameters.
	//const X86_CYCLE_TABLE *x86_cycle_table;
	//const X86_OPCODE *x86_opcode_table;
	//X86_OPCODE x86_opcode_table[];
	
	int i386_parity_table[256];
	MODRM_TABLE i386_MODRM_table[256];
	i386_state __cpustate;
	UINT8 cycle_table_rm[X86_NUM_CPUS][CYCLES_NUM_OPCODES];
	UINT8 cycle_table_pm[X86_NUM_CPUS][CYCLES_NUM_OPCODES];
	const floatx80 fx80_zero =   { 0x0000, U64(0x0000000000000000) };
	const floatx80 fx80_one =    { 0x3fff, U64(0x8000000000000000) };
	const floatx80 fx80_ninf =   { 0xffff, U64(0x8000000000000000) };
	const floatx80 fx80_inan =   { 0xffff, U64(0xc000000000000000) };
/* Maps x87 round modes to SoftFloat round modes */
	const int x87_to_sf_rc[4] = {
		float_round_nearest_even,
		float_round_down,
		float_round_up,
		float_round_to_zero,
	};
	DEVICE *d_mem;
	DEVICE *d_io;
	DEVICE *d_pic;
	DEVICE *d_bios;
	DEVICE *d_dma;
	
	UINT32 i386_escape_ea;   // hack around GCC 4.6 error because we need the side effects of GetEA()


	void process_state_SREG(I386_SREG* val, FILEIO* state_fio);
	void process_state_SYS_TABLE(I386_SYS_TABLE* val, FILEIO* state_fio);
	void process_state_SEG_DESC(I386_SEG_DESC* val, FILEIO* state_fio);
	void process_state_GPR(I386_GPR* val, FILEIO* state_fio);
	void process_state_floatx80(floatx80* val, FILEIO* state_fio);
	void process_state_XMM_REG(XMM_REG* val, FILEIO* state_fio);
	void process_state_vtlb(vtlb_state* val, FILEIO* state_fio);

public:
	I386_OPS_BASE(int cputypes = I386_OPS_CPUTYPE_I386)
	{
		cpustate = NULL;
		_cputype = cputypes;
	}
	~I386_OPS_BASE() {}
	virtual void I386OP_D(decode_opcode)();
	
	int i386_translate_address(int intention, offs_t *address, vtlb_entry *entry);
	virtual int cpu_translate_i386(void *cpudevice, address_spacenum space, int intention, offs_t *address);
	virtual int cpu_execute_i386(int cycles);
	virtual void i386_trap(int irq, int irq_gate, int trap_level);
	virtual void i386_trap_with_error(int irq, int irq_gate, int trap_level, UINT32 error);
	void i386_set_irq_line(int irqline, int state);
	void i386_set_a20_line(int state);
	int i386_limit_check( int seg, UINT32 offset, UINT32 size);
	void i386_vtlb_free(void);
	void i386_free_state(void);

	i386_state *get_cpu_state(void) { return cpustate; }
	int get_extra_clock() { return cpustate->extra_cycles; }
	void set_extra_clock(int n) { cpustate->extra_cycles += n; }

	uint32_t get_pc() { return cpustate->pc; }
	uint32_t get_prev_pc() { return cpustate->prev_pc; }
	void set_address_mask(uint32_t mask) { cpustate->a20_mask = mask; }
	uint32_t get_address_mask() { return cpustate->a20_mask; }
	void  set_shutdown_flag(int n) { cpustate->shutdown = n; }
	int get_shutdown_flag() { return cpustate->shutdown; }

	void set_busreq(bool n) { cpustate->busreq = n ? 1 : 0; }
	bool get_busreq() { return cpustate->busreq; }
	INLINE void check_ioperm( offs_t port, UINT8 mask);

	void set_context_pic(DEVICE *dev) {
		cpustate->pic = dev;
		d_pic = dev;
	}
	virtual void set_context_progmem_stored(DEVICE *dev) {
		//cpustate->program_stored = dev;
	}
	void set_context_progmem(DEVICE *dev) {
		cpustate->program = dev;
		d_mem = dev;
	}

	virtual void set_context_pseudo_bios(DEVICE *dev) {
		//cpustate->bios = dev;
		//d_bios = dev;
	}

	virtual void set_context_dma(DEVICE *dev) {
		//cpustate->dma = dev;
		//d_dma = dev;
	}

	void set_context_io(DEVICE *dev) {
		cpustate->io = dev;
		d_io = dev;
	}
	
	virtual void set_context_io_stored(DEVICE *dev) {
		//cpustate->io_stored = dev;
		
	}

	virtual void set_context_emu(EMU *p_emu) {
		//cpustate->emu = p_emu;
	}

	virtual void set_context_debugger(DEBUGGER *debugger) {
		//cpustate->debugger = dev;
	}
	
	virtual bool write_debug_reg(const _TCHAR *reg, uint32_t data) { return false; }
	virtual uint32_t read_debug_reg(const _TCHAR *reg) { return 0; }
	
	virtual int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len) { return 0;}
	void vtlb_flush_dynamic(void) { vtlb_flush_dynamic(cpustate->vtlb); }

	bool process_state(FILEIO *state_fio, bool loading);
	
protected:
	// Utilities
	void build_cycle_table();
	virtual i386_state *i386_common_init(int tlbsize);
	void build_opcode_table( UINT32 features);
	void zero_state();
	void pentium_smi();
	
	UINT32 i386_load_protected_mode_segment( I386_SREG *seg, UINT64 *desc );
	void i386_load_call_gate(I386_CALL_GATE *gate);
	void i386_set_descriptor_accessed( UINT16 selector);
	void i386_load_segment_descriptor( int segment );
	UINT32 i386_get_stack_segment(UINT8 privilege);
	UINT32 i386_get_stack_ptr(UINT8 privilege);
	
	UINT32 get_flags();
	void set_flags( UINT32 f );

	void sib_byte(UINT8 mod, UINT32* out_ea, UINT8* out_segment);
	void modrm_to_EA(UINT8 mod_rm, UINT32* out_ea, UINT8* out_segment);
	
	UINT32 GetNonTranslatedEA(UINT8 modrm,UINT8 *seg);
	UINT32 GetEA(UINT8 modrm, int rwn, UINT32 size);

	// 
	void i386_check_sreg_validity(int reg);
	void i386_sreg_load( UINT16 selector, UINT8 reg, bool *fault);
	void i286_task_switch( UINT16 selector, UINT8 nested);
	void i386_task_switch( UINT16 selector, UINT8 nested);
	void i386_check_irq_line();
	
	void i386_protected_mode_jump( UINT16 seg, UINT32 off, int indirect, int operand32);
	void i386_protected_mode_call( UINT16 seg, UINT32 off, int indirect, int operand32);
	void i386_protected_mode_retf(UINT8 count, UINT8 operand32);
	void i386_protected_mode_iret(int operand32);

	// Reporter.
	void report_invalid_opcode();
	void report_invalid_modrm(const char* opcode, UINT8 modrm);

	//LINES
	void i386_postload();

	/* ----- initialization/teardown ----- */
	/* allocate a new VTLB for the given CPU */
	vtlb_state *vtlb_alloc(void *cpu, address_spacenum space, int fixed_entries, int dynamic_entries);
	/* free an allocated VTLB */
	void vtlb_free(vtlb_state *vtlb);
	
	/* called by the CPU core in response to an unmapped access */
	int vtlb_fill(vtlb_state *vtlb, offs_t address, int intention);
	/* load a fixed VTLB entry */
	void vtlb_load(vtlb_state *vtlb, int entrynum, int numpages, offs_t address, vtlb_entry value);
	/* load a dynamic VTLB entry */
	void vtlb_dynload(vtlb_state *vtlb, UINT32 index, offs_t address, vtlb_entry value);
	/* ----- flushing ----- */
	/* flush all knowledge from the dynamic part of the VTLB */
	void vtlb_flush_dynamic(vtlb_state *vtlb);
	/* flush knowledge of a particular address from the VTLB */
	void vtlb_flush_address(vtlb_state *vtlb, offs_t address);
	/* ----- accessors ----- */
	/* return a pointer to the base of the linear VTLB lookup table */
	const vtlb_entry *vtlb_table(vtlb_state *vtlb);

	virtual int cpu_disassemble_x86_16(_TCHAR *buffer, UINT64 eip, const UINT8 *oprom) { return 0; }
	virtual int cpu_disassemble_x86_32(_TCHAR *buffer, UINT64 eip, const UINT8 *oprom) { return 0; }
	virtual int cpu_disassemble_x86_64(_TCHAR *buffer, UINT64 eip, const UINT8 *oprom) { return 0; }

public:
	// Init per vm..
	void *cpu_init_i386(void);
	void *cpu_init_i486(void);
	void *cpu_init_pentium(void);
	void *cpu_init_mediagx(void);
	void *cpu_init_pentium_pro(void);
	void *cpu_init_pentium_mmx(void);
	void *cpu_init_pentium2(void);
	void *cpu_init_pentium3(void);
	void *cpu_init_pentium4(void);
	// Reset per type.
	void cpu_reset_i386(void);
	void cpu_reset_i486(void);
	void cpu_reset_pentium(void);
	void cpu_reset_mediagx(void);
	void cpu_reset_pentium_pro(void);
	void cpu_reset_pentium_mmx(void);
	void cpu_reset_pentium2(void);
	void cpu_reset_pentium3(void);
	void cpu_reset_pentium4(void);

	// INSNs.
	// i386/op16
	void I386OP_D(adc_rm16_r16)();      // Opcode 0x11
	void I386OP_D(adc_r16_rm16)();      // Opcode 0x13
	void I386OP_D(adc_ax_i16)();        // Opcode 0x15
	void I386OP_D(add_rm16_r16)();      // Opcode 0x01
	void I386OP_D(add_r16_rm16)();      // Opcode 0x03
	void I386OP_D(add_ax_i16)();        // Opcode 0x05
	void I386OP_D(and_rm16_r16)();      // Opcode 0x21
	void I386OP_D(and_r16_rm16)();      // Opcode 0x23
	void I386OP_D(and_ax_i16)();        // Opcode 0x25
	void I386OP_D(bsf_r16_rm16)();      // Opcode 0x0f bc
	void I386OP_D(bsr_r16_rm16)();      // Opcode 0x0f bd
	void I386OP_D(bt_rm16_r16)();       // Opcode 0x0f a3
	void I386OP_D(btc_rm16_r16)();      // Opcode 0x0f bb
	void I386OP_D(btr_rm16_r16)();      // Opcode 0x0f b3
	void I386OP_D(bts_rm16_r16)();      // Opcode 0x0f ab
	virtual void I386OP_D(call_abs16)();        // Opcode 0x9a
	virtual void I386OP_D(call_rel16)();        // Opcode 0xe8
	void I386OP_D(cbw)();               // Opcode 0x98
	void I386OP_D(cmp_rm16_r16)();      // Opcode 0x39
	void I386OP_D(cmp_r16_rm16)();      // Opcode 0x3b
	void I386OP_D(cmp_ax_i16)();        // Opcode 0x3d
	void I386OP_D(cmpsw)();             // Opcode 0xa7
	void I386OP_D(cwd)();               // Opcode 0x99
	void I386OP_D(dec_ax)();            // Opcode 0x48
	void I386OP_D(dec_cx)();            // Opcode 0x49
	void I386OP_D(dec_dx)();            // Opcode 0x4a
	void I386OP_D(dec_bx)();            // Opcode 0x4b
	void I386OP_D(dec_sp)();            // Opcode 0x4c
	void I386OP_D(dec_bp)();            // Opcode 0x4d
	void I386OP_D(dec_si)();            // Opcode 0x4e
	void I386OP_D(dec_di)();            // Opcode 0x4f
	void I386OP_D(imul_r16_rm16)();     // Opcode 0x0f af
	void I386OP_D(imul_r16_rm16_i16)(); // Opcode 0x69
	void I386OP_D(imul_r16_rm16_i8)();  // Opcode 0x6b
	void I386OP_D(in_ax_i8)();          // Opcode 0xe5
	void I386OP_D(in_ax_dx)();          // Opcode 0xed
	void I386OP_D(inc_ax)();            // Opcode 0x40
	void I386OP_D(inc_cx)();            // Opcode 0x41
	void I386OP_D(inc_dx)();            // Opcode 0x42
	void I386OP_D(inc_bx)();            // Opcode 0x43
	void I386OP_D(inc_sp)();            // Opcode 0x44
	void I386OP_D(inc_bp)();            // Opcode 0x45
	void I386OP_D(inc_si)();            // Opcode 0x46
	void I386OP_D(inc_di)();            // Opcode 0x47
	void I386OP_D(iret16)();            // Opcode 0xcf
	void I386OP_D(ja_rel16)();          // Opcode 0x0f 87
	void I386OP_D(jbe_rel16)();         // Opcode 0x0f 86
	void I386OP_D(jc_rel16)();          // Opcode 0x0f 82
	void I386OP_D(jg_rel16)();          // Opcode 0x0f 8f
	void I386OP_D(jge_rel16)();         // Opcode 0x0f 8d
	void I386OP_D(jl_rel16)();          // Opcode 0x0f 8c
	void I386OP_D(jle_rel16)();         // Opcode 0x0f 8e
	void I386OP_D(jnc_rel16)();         // Opcode 0x0f 83
	void I386OP_D(jno_rel16)();         // Opcode 0x0f 81
	void I386OP_D(jnp_rel16)();         // Opcode 0x0f 8b
	void I386OP_D(jns_rel16)();         // Opcode 0x0f 89
	void I386OP_D(jnz_rel16)();         // Opcode 0x0f 85
	void I386OP_D(jo_rel16)();          // Opcode 0x0f 80
	void I386OP_D(jp_rel16)();          // Opcode 0x0f 8a
	void I386OP_D(js_rel16)();          // Opcode 0x0f 88
	void I386OP_D(jz_rel16)();          // Opcode 0x0f 84
	void I386OP_D(jcxz16)();            // Opcode 0xe3
	void I386OP_D(jmp_rel16)();         // Opcode 0xe9
	void I386OP_D(jmp_abs16)();         // Opcode 0xea
	void I386OP_D(lea16)();             // Opcode 0x8d
	void I386OP_D(enter16)();           // Opcode 0xc8
	void I386OP_D(leave16)();           // Opcode 0xc9
	void I386OP_D(lodsw)();             // Opcode 0xad
	void I386OP_D(loop16)();            // Opcode 0xe2
	void I386OP_D(loopne16)();          // Opcode 0xe0
	void I386OP_D(loopz16)();           // Opcode 0xe1
	void I386OP_D(mov_rm16_r16)();      // Opcode 0x89
	void I386OP_D(mov_r16_rm16)();      // Opcode 0x8b
	void I386OP_D(mov_rm16_i16)();      // Opcode 0xc7
	void I386OP_D(mov_ax_m16)();        // Opcode 0xa1
	void I386OP_D(mov_m16_ax)();        // Opcode 0xa3
	void I386OP_D(mov_ax_i16)();        // Opcode 0xb8
	void I386OP_D(mov_cx_i16)();        // Opcode 0xb9
	void I386OP_D(mov_dx_i16)();        // Opcode 0xba
	void I386OP_D(mov_bx_i16)();        // Opcode 0xbb
	void I386OP_D(mov_sp_i16)();        // Opcode 0xbc
	void I386OP_D(mov_bp_i16)();        // Opcode 0xbd
	void I386OP_D(mov_si_i16)();        // Opcode 0xbe
	void I386OP_D(mov_di_i16)();        // Opcode 0xbf
	void I386OP_D(movsw)();             // Opcode 0xa5
	void I386OP_D(movsx_r16_rm8)();     // Opcode 0x0f be
	void I386OP_D(movzx_r16_rm8)();     // Opcode 0x0f b6
	void I386OP_D(or_rm16_r16)();       // Opcode 0x09
	void I386OP_D(or_r16_rm16)();       // Opcode 0x0b
	void I386OP_D(or_ax_i16)();         // Opcode 0x0d
	void I386OP_D(out_ax_i8)();         // Opcode 0xe7
	void I386OP_D(out_ax_dx)();         // Opcode 0xef
	void I386OP_D(pop_ax)();            // Opcode 0x58
	void I386OP_D(pop_cx)();            // Opcode 0x59
	void I386OP_D(pop_dx)();            // Opcode 0x5a
	void I386OP_D(pop_bx)();            // Opcode 0x5b
	void I386OP_D(pop_sp)();            // Opcode 0x5c
	void I386OP_D(pop_bp)();            // Opcode 0x5d
	void I386OP_D(pop_si)();            // Opcode 0x5e
	void I386OP_D(pop_di)();            // Opcode 0x5f
	void I386OP_D(pop_ds16)();          // Opcode 0x1f
	void I386OP_D(pop_es16)();          // Opcode 0x07
	void I386OP_D(pop_fs16)();          // Opcode 0x0f a1
	void I386OP_D(pop_gs16)();          // Opcode 0x0f a9
	void I386OP_D(pop_ss16)();          // Opcode 0x17
	void I386OP_D(pop_rm16)();          // Opcode 0x8f
	void I386OP_D(popa)();              // Opcode 0x61
	void I386OP_D(popf)();              // Opcode 0x9d
	void I386OP_D(push_ax)();           // Opcode 0x50
	void I386OP_D(push_cx)();           // Opcode 0x51
	void I386OP_D(push_dx)();           // Opcode 0x52
	void I386OP_D(push_bx)();           // Opcode 0x53
	void I386OP_D(push_sp)();           // Opcode 0x54
	void I386OP_D(push_bp)();           // Opcode 0x55
	void I386OP_D(push_si)();           // Opcode 0x56
	void I386OP_D(push_di)();           // Opcode 0x57
	void I386OP_D(push_cs16)();         // Opcode 0x0e
	void I386OP_D(push_ds16)();         // Opcode 0x1e
	void I386OP_D(push_es16)();         // Opcode 0x06
	void I386OP_D(push_fs16)();         // Opcode 0x0f a0
	void I386OP_D(push_gs16)();         // Opcode 0x0f a8
	void I386OP_D(push_ss16)();         // Opcode 0x16
	void I386OP_D(push_i16)();          // Opcode 0x68
	void I386OP_D(pusha)();             // Opcode 0x60
	void I386OP_D(pushf)();             // Opcode 0x9c
	void I386OP_D(ret_near16_i16)();    // Opcode 0xc2
	void I386OP_D(ret_near16)();        // Opcode 0xc3
	void I386OP_D(sbb_rm16_r16)();      // Opcode 0x19
	void I386OP_D(sbb_r16_rm16)();      // Opcode 0x1b
	void I386OP_D(sbb_ax_i16)();        // Opcode 0x1d
	void I386OP_D(scasw)();             // Opcode 0xaf
	void I386OP_D(shld16_i8)();         // Opcode 0x0f a4
	void I386OP_D(shld16_cl)();         // Opcode 0x0f a5
	void I386OP_D(shrd16_i8)();         // Opcode 0x0f ac
	void I386OP_D(shrd16_cl)();         // Opcode 0x0f ad
	void I386OP_D(stosw)();             // Opcode 0xab
	void I386OP_D(sub_rm16_r16)();      // Opcode 0x29
	void I386OP_D(sub_r16_rm16)();      // Opcode 0x2b
	void I386OP_D(sub_ax_i16)();        // Opcode 0x2d
	void I386OP_D(test_ax_i16)();       // Opcode 0xa9
	void I386OP_D(test_rm16_r16)();     // Opcode 0x85
	void I386OP_D(xchg_ax_cx)();        // Opcode 0x91
	void I386OP_D(xchg_ax_dx)();        // Opcode 0x92
	void I386OP_D(xchg_ax_bx)();        // Opcode 0x93
	void I386OP_D(xchg_ax_sp)();        // Opcode 0x94
	void I386OP_D(xchg_ax_bp)();        // Opcode 0x95
	void I386OP_D(xchg_ax_si)();        // Opcode 0x96
	void I386OP_D(xchg_ax_di)();        // Opcode 0x97
	void I386OP_D(xchg_r16_rm16)();     // Opcode 0x87
	void I386OP_D(xor_rm16_r16)();      // Opcode 0x31
	void I386OP_D(xor_r16_rm16)();      // Opcode 0x33
	void I386OP_D(xor_ax_i16)();        // Opcode 0x35
	void I386OP_D(group81_16)();        // Opcode 0x81
	void I386OP_D(group83_16)();        // Opcode 0x83
	void I386OP_D(groupC1_16)();        // Opcode 0xc1
	void I386OP_D(groupD1_16)();        // Opcode 0xd1
	void I386OP_D(groupD3_16)();        // Opcode 0xd3
	void I386OP_D(groupF7_16)();        // Opcode 0xf7
	virtual void I386OP_D(groupFF_16)();        // Opcode 0xff
	void I386OP_D(group0F00_16)();          // Opcode 0x0f 00
	void I386OP_D(group0F01_16)();      // Opcode 0x0f 01
	void I386OP_D(group0FBA_16)();      // Opcode 0x0f ba
	void I386OP_D(lar_r16_rm16)();  // Opcode 0x0f 0x02
	void I386OP_D(lsl_r16_rm16)();  // Opcode 0x0f 0x03
	void I386OP_D(bound_r16_m16_m16)(); // Opcode 0x62
	void I386OP_D(retf16)();            // Opcode 0xcb
	void I386OP_D(retf_i16)();          // Opcode 0xca
	void I386OP_D(lds16)();             // Opcode 0xc5
	void I386OP_D(lss16)();             // Opcode 0x0f 0xb2
	void I386OP_D(les16)();             // Opcode 0xc4
	void I386OP_D(lfs16)();             // Opcode 0x0f 0xb4
	void I386OP_D(lgs16)();             // Opcode 0x0f 0xb5
	UINT16 I386OP_D(shift_rotate16)( UINT8 modrm, UINT32 value, UINT8 shift);
	UINT8 I386OP_D(shift_rotate8)( UINT8 modrm, UINT32 value, UINT8 shift);
	//i386/op32
	void I386OP_D(adc_rm32_r32)();      // Opcode 0x11
	void I386OP_D(adc_r32_rm32)();      // Opcode 0x13
	void I386OP_D(adc_eax_i32)();       // Opcode 0x15
	void I386OP_D(add_rm32_r32)();      // Opcode 0x01
	void I386OP_D(add_r32_rm32)();      // Opcode 0x03
	void I386OP_D(add_eax_i32)();       // Opcode 0x05
	void I386OP_D(and_rm32_r32)();      // Opcode 0x21
	void I386OP_D(and_r32_rm32)();      // Opcode 0x23
	void I386OP_D(and_eax_i32)();       // Opcode 0x25
	void I386OP_D(bsf_r32_rm32)();      // Opcode 0x0f bc
	void I386OP_D(bsr_r32_rm32)();      // Opcode 0x0f bd
	void I386OP_D(bt_rm32_r32)();       // Opcode 0x0f a3
	void I386OP_D(btc_rm32_r32)();      // Opcode 0x0f bb
	void I386OP_D(btr_rm32_r32)();      // Opcode 0x0f b3
	void I386OP_D(bts_rm32_r32)();      // Opcode 0x0f ab
	void I386OP_D(call_abs32)();        // Opcode 0x9a
	void I386OP_D(call_rel32)();        // Opcode 0xe8
	void I386OP_D(cdq)();               // Opcode 0x99
	void I386OP_D(cmp_rm32_r32)();      // Opcode 0x39
	void I386OP_D(cmp_r32_rm32)();      // Opcode 0x3b
	void I386OP_D(cmp_eax_i32)();       // Opcode 0x3d
	void I386OP_D(cmpsd)();             // Opcode 0xa7
	void I386OP_D(cwde)();              // Opcode 0x98
	void I386OP_D(dec_eax)();           // Opcode 0x48
	void I386OP_D(dec_ecx)();           // Opcode 0x49
	void I386OP_D(dec_edx)();           // Opcode 0x4a
	void I386OP_D(dec_ebx)();           // Opcode 0x4b
	void I386OP_D(dec_esp)();           // Opcode 0x4c
	void I386OP_D(dec_ebp)();           // Opcode 0x4d
	void I386OP_D(dec_esi)();           // Opcode 0x4e
	void I386OP_D(dec_edi)();           // Opcode 0x4f
	void I386OP_D(imul_r32_rm32)();     // Opcode 0x0f af
	void I386OP_D(imul_r32_rm32_i32)(); // Opcode 0x69
	void I386OP_D(imul_r32_rm32_i8)();  // Opcode 0x6b
	void I386OP_D(in_eax_i8)();         // Opcode 0xe5
	void I386OP_D(in_eax_dx)();         // Opcode 0xed
	void I386OP_D(inc_eax)();           // Opcode 0x40
	void I386OP_D(inc_ecx)();           // Opcode 0x41
	void I386OP_D(inc_edx)();           // Opcode 0x42
	void I386OP_D(inc_ebx)();           // Opcode 0x43
	void I386OP_D(inc_esp)();           // Opcode 0x44
	void I386OP_D(inc_ebp)();           // Opcode 0x45
	void I386OP_D(inc_esi)();           // Opcode 0x46
	void I386OP_D(inc_edi)();           // Opcode 0x47
	void I386OP_D(iret32)();            // Opcode 0xcf
	void I386OP_D(ja_rel32)();          // Opcode 0x0f 87
	void I386OP_D(jbe_rel32)();         // Opcode 0x0f 86
	void I386OP_D(jc_rel32)();          // Opcode 0x0f 82
	void I386OP_D(jg_rel32)();          // Opcode 0x0f 8f
	void I386OP_D(jge_rel32)();         // Opcode 0x0f 8d
	void I386OP_D(jl_rel32)();          // Opcode 0x0f 8c
	void I386OP_D(jle_rel32)();         // Opcode 0x0f 8e
	void I386OP_D(jnc_rel32)();         // Opcode 0x0f 83
	void I386OP_D(jno_rel32)();         // Opcode 0x0f 81
	void I386OP_D(jnp_rel32)();         // Opcode 0x0f 8b
	void I386OP_D(jns_rel32)();         // Opcode 0x0f 89
	void I386OP_D(jnz_rel32)();         // Opcode 0x0f 85
	void I386OP_D(jo_rel32)();          // Opcode 0x0f 80
	void I386OP_D(jp_rel32)();          // Opcode 0x0f 8a
	void I386OP_D(js_rel32)();          // Opcode 0x0f 88
	void I386OP_D(jz_rel32)();          // Opcode 0x0f 84
	void I386OP_D(jcxz32)();            // Opcode 0xe3
	void I386OP_D(jmp_rel32)();         // Opcode 0xe9
	void I386OP_D(jmp_abs32)();         // Opcode 0xea
	void I386OP_D(lea32)();             // Opcode 0x8d
	void I386OP_D(enter32)();           // Opcode 0xc8
	void I386OP_D(leave32)();           // Opcode 0xc9
	void I386OP_D(lodsd)();             // Opcode 0xad
	void I386OP_D(loop32)();            // Opcode 0xe2
	void I386OP_D(loopne32)();          // Opcode 0xe0
	void I386OP_D(loopz32)();           // Opcode 0xe1
	void I386OP_D(mov_rm32_r32)();      // Opcode 0x89
	void I386OP_D(mov_r32_rm32)();      // Opcode 0x8b
	void I386OP_D(mov_rm32_i32)();      // Opcode 0xc7
	void I386OP_D(mov_eax_m32)();       // Opcode 0xa1
	void I386OP_D(mov_m32_eax)();       // Opcode 0xa3
	void I386OP_D(mov_eax_i32)();       // Opcode 0xb8
	void I386OP_D(mov_ecx_i32)();       // Opcode 0xb9
	void I386OP_D(mov_edx_i32)();       // Opcode 0xba
	void I386OP_D(mov_ebx_i32)();       // Opcode 0xbb
	void I386OP_D(mov_esp_i32)();       // Opcode 0xbc
	void I386OP_D(mov_ebp_i32)();       // Opcode 0xbd
	void I386OP_D(mov_esi_i32)();       // Opcode 0xbe
	void I386OP_D(mov_edi_i32)();       // Opcode 0xbf
	void I386OP_D(movsd)();             // Opcode 0xa5
	void I386OP_D(movsx_r32_rm8)();     // Opcode 0x0f be
	void I386OP_D(movsx_r32_rm16)();    // Opcode 0x0f bf
	void I386OP_D(movzx_r32_rm8)();     // Opcode 0x0f b6
	void I386OP_D(movzx_r32_rm16)();    // Opcode 0x0f b7
	void I386OP_D(or_rm32_r32)();       // Opcode 0x09
	void I386OP_D(or_r32_rm32)();       // Opcode 0x0b
	void I386OP_D(or_eax_i32)();        // Opcode 0x0d
	void I386OP_D(out_eax_i8)();        // Opcode 0xe7
	void I386OP_D(out_eax_dx)();        // Opcode 0xef
	void I386OP_D(pop_eax)();           // Opcode 0x58
	void I386OP_D(pop_ecx)();           // Opcode 0x59
	void I386OP_D(pop_edx)();           // Opcode 0x5a
	void I386OP_D(pop_ebx)();           // Opcode 0x5b
	void I386OP_D(pop_esp)();           // Opcode 0x5c
	void I386OP_D(pop_ebp)();           // Opcode 0x5d
	void I386OP_D(pop_esi)();           // Opcode 0x5e
	void I386OP_D(pop_edi)();           // Opcode 0x5f
	void I386OP_D(pop_ds32)();          // Opcode 0x1f
	void I386OP_D(pop_es32)();          // Opcode 0x07
	void I386OP_D(pop_fs32)();          // Opcode 0x0f a1
	void I386OP_D(pop_gs32)();          // Opcode 0x0f a9
	void I386OP_D(pop_ss32)();          // Opcode 0x17
	void I386OP_D(pop_rm32)();          // Opcode 0x8f
	void I386OP_D(popad)();             // Opcode 0x61
	void I386OP_D(popfd)();             // Opcode 0x9d
	void I386OP_D(push_eax)();          // Opcode 0x50
	void I386OP_D(push_ecx)();          // Opcode 0x51
	void I386OP_D(push_edx)();          // Opcode 0x52
	void I386OP_D(push_ebx)();          // Opcode 0x53
	void I386OP_D(push_esp)();          // Opcode 0x54
	void I386OP_D(push_ebp)();          // Opcode 0x55
	void I386OP_D(push_esi)();          // Opcode 0x56
	void I386OP_D(push_edi)();          // Opcode 0x57
	void I386OP_D(push_cs32)();         // Opcode 0x0e
	void I386OP_D(push_ds32)();         // Opcode 0x1e
	void I386OP_D(push_es32)();         // Opcode 0x06
	void I386OP_D(push_fs32)();         // Opcode 0x0f a0
	void I386OP_D(push_gs32)();         // Opcode 0x0f a8
	void I386OP_D(push_ss32)();         // Opcode 0x16
	void I386OP_D(push_i32)();          // Opcode 0x68
	void I386OP_D(pushad)();            // Opcode 0x60
	void I386OP_D(pushfd)();            // Opcode 0x9c
	void I386OP_D(ret_near32_i16)();    // Opcode 0xc2
	void I386OP_D(ret_near32)();        // Opcode 0xc3
	void I386OP_D(sbb_rm32_r32)();      // Opcode 0x19
	void I386OP_D(sbb_r32_rm32)();      // Opcode 0x1b
	void I386OP_D(sbb_eax_i32)();       // Opcode 0x1d
	void I386OP_D(scasd)();             // Opcode 0xaf
	void I386OP_D(shld32_i8)();         // Opcode 0x0f a4
	void I386OP_D(shld32_cl)();         // Opcode 0x0f a5
	void I386OP_D(shrd32_i8)();         // Opcode 0x0f ac
	void I386OP_D(shrd32_cl)();         // Opcode 0x0f ad
	void I386OP_D(stosd)();             // Opcode 0xab
	void I386OP_D(sub_rm32_r32)();      // Opcode 0x29
	void I386OP_D(sub_r32_rm32)();      // Opcode 0x2b
	void I386OP_D(sub_eax_i32)();       // Opcode 0x2d
	void I386OP_D(test_eax_i32)();      // Opcode 0xa9
	void I386OP_D(test_rm32_r32)();     // Opcode 0x85
	void I386OP_D(xchg_eax_ecx)();      // Opcode 0x91
	void I386OP_D(xchg_eax_edx)();      // Opcode 0x92
	void I386OP_D(xchg_eax_ebx)();      // Opcode 0x93
	void I386OP_D(xchg_eax_esp)();      // Opcode 0x94
	void I386OP_D(xchg_eax_ebp)();      // Opcode 0x95
	void I386OP_D(xchg_eax_esi)();      // Opcode 0x96
	void I386OP_D(xchg_eax_edi)();      // Opcode 0x97
	void I386OP_D(xchg_r32_rm32)();     // Opcode 0x87
	void I386OP_D(xor_rm32_r32)();      // Opcode 0x31
	void I386OP_D(xor_r32_rm32)();      // Opcode 0x33
	void I386OP_D(xor_eax_i32)();       // Opcode 0x35
	void I386OP_D(group81_32)();        // Opcode 0x81
	void I386OP_D(group83_32)();        // Opcode 0x83
	void I386OP_D(groupC1_32)();        // Opcode 0xc1
	void I386OP_D(groupD1_32)();        // Opcode 0xd1
	void I386OP_D(groupD3_32)();        // Opcode 0xd3
	void I386OP_D(groupF7_32)();        // Opcode 0xf7
	void I386OP_D(groupFF_32)();        // Opcode 0xff
	void I386OP_D(group0F00_32)();          // Opcode 0x0f 00
	void I386OP_D(group0F01_32)();      // Opcode 0x0f 01
	void I386OP_D(group0FBA_32)();      // Opcode 0x0f ba
	void I386OP_D(lar_r32_rm32)();  // Opcode 0x0f 0x02
	void I386OP_D(lsl_r32_rm32)();  // Opcode 0x0f 0x03
	void I386OP_D(bound_r32_m32_m32)(); // Opcode 0x62
	void I386OP_D(retf32)();            // Opcode 0xcb
	void I386OP_D(retf_i32)();          // Opcode 0xca
	void I386OP_D(load_far_pointer32)( int s);
	void I386OP_D(lds32)();             // Opcode 0xc5
	void I386OP_D(lss32)();             // Opcode 0x0f 0xb2
	void I386OP_D(les32)();             // Opcode 0xc4
	void I386OP_D(lfs32)();             // Opcode 0x0f 0xb4
	void I386OP_D(lgs32)();             // Opcode 0x0f 0xb5
// i386 other OPS.
	void I386OP_D(adc_rm8_r8)();        // Opcode 0x10
	void I386OP_D(adc_r8_rm8)();        // Opcode 0x12
	void I386OP_D(adc_al_i8)();     // Opcode 0x14
	void I386OP_D(add_rm8_r8)();        // Opcode 0x00
	void I386OP_D(add_r8_rm8)();        // Opcode 0x02
	void I386OP_D(add_al_i8)();     // Opcode 0x04
	void I386OP_D(and_rm8_r8)();        // Opcode 0x20
	void I386OP_D(and_r8_rm8)();        // Opcode 0x22
	void I386OP_D(and_al_i8)();         // Opcode 0x24
	void I386OP_D(clc)();               // Opcode 0xf8
	void I386OP_D(cld)();               // Opcode 0xfc
	void I386OP_D(cli)();               // Opcode 0xfa
	void I386OP_D(cmc)();               // Opcode 0xf5
	void I386OP_D(cmp_rm8_r8)();        // Opcode 0x38
	void I386OP_D(cmp_r8_rm8)();        // Opcode 0x3a
	void I386OP_D(cmp_al_i8)();         // Opcode 0x3c
	void I386OP_D(cmpsb)();             // Opcode 0xa6
	void I386OP_D(in_al_i8)();          // Opcode 0xe4
	void I386OP_D(in_al_dx)();          // Opcode 0xec
	void I386OP_D(ja_rel8)();           // Opcode 0x77
	void I386OP_D(jbe_rel8)();          // Opcode 0x76
	void I386OP_D(jc_rel8)();           // Opcode 0x72
	void I386OP_D(jg_rel8)();           // Opcode 0x7f
	void I386OP_D(jge_rel8)();          // Opcode 0x7d
	void I386OP_D(jl_rel8)();           // Opcode 0x7c
	void I386OP_D(jle_rel8)();      // Opcode 0x7e
	void I386OP_D(jnc_rel8)();          // Opcode 0x73
	void I386OP_D(jno_rel8)();          // Opcode 0x71
	void I386OP_D(jnp_rel8)();          // Opcode 0x7b
	void I386OP_D(jns_rel8)();          // Opcode 0x79
	void I386OP_D(jnz_rel8)();          // Opcode 0x75
	void I386OP_D(jo_rel8)();           // Opcode 0x70
	void I386OP_D(jp_rel8)();           // Opcode 0x7a
	void I386OP_D(js_rel8)();           // Opcode 0x78
	void I386OP_D(jz_rel8)();           // Opcode 0x74
	void I386OP_D(jmp_rel8)();          // Opcode 0xeb
	void I386OP_D(lahf)();              // Opcode 0x9f
	void I386OP_D(lodsb)();             // Opcode 0xac
	void I386OP_D(mov_rm8_r8)();        // Opcode 0x88
	void I386OP_D(mov_r8_rm8)();        // Opcode 0x8a
	void I386OP_D(mov_rm8_i8)();        // Opcode 0xc6
	void I386OP_D(mov_r32_cr)();        // Opcode 0x0f 20
	void I386OP_D(mov_r32_dr)();        // Opcode 0x0f 21
	void I386OP_D(mov_cr_r32)();        // Opcode 0x0f 22
	void I386OP_D(mov_dr_r32)();        // Opcode 0x0f 23
	void I386OP_D(mov_al_m8)();         // Opcode 0xa0
	void I386OP_D(mov_m8_al)();         // Opcode 0xa2
	void I386OP_D(mov_rm16_sreg)();     // Opcode 0x8c
	void I386OP_D(mov_sreg_rm16)();     // Opcode 0x8e
	void I386OP_D(mov_al_i8)();         // Opcode 0xb0
	void I386OP_D(mov_cl_i8)();         // Opcode 0xb1
	void I386OP_D(mov_dl_i8)();         // Opcode 0xb2
	void I386OP_D(mov_bl_i8)();         // Opcode 0xb3
	void I386OP_D(mov_ah_i8)();         // Opcode 0xb4
	void I386OP_D(mov_ch_i8)();         // Opcode 0xb5
	void I386OP_D(mov_dh_i8)();         // Opcode 0xb6
	void I386OP_D(mov_bh_i8)();         // Opcode 0xb7
	void I386OP_D(movsb)();             // Opcode 0xa4
	void I386OP_D(or_rm8_r8)();         // Opcode 0x08
	void I386OP_D(or_r8_rm8)();         // Opcode 0x0a
	void I386OP_D(or_al_i8)();          // Opcode 0x0c
	void I386OP_D(out_al_i8)();         // Opcode 0xe6
	void I386OP_D(out_al_dx)();         // Opcode 0xee
	void I386OP_D(arpl)();           // Opcode 0x63
	void I386OP_D(push_i8)();           // Opcode 0x6a
	void I386OP_D(ins_generic)( int size);
	void I386OP_D(insb)();              // Opcode 0x6c
	void I386OP_D(insw)();              // Opcode 0x6d
	void I386OP_D(insd)();              // Opcode 0x6d
	void I386OP_D(outs_generic)( int size);
	void I386OP_D(outsb)();             // Opcode 0x6e
	void I386OP_D(outsw)();             // Opcode 0x6f
	void I386OP_D(outsd)();             // Opcode 0x6f
	void I386OP_D(repeat)( int invert_flag);
	void I386OP_D(rep)();               // Opcode 0xf3
	void I386OP_D(repne)();             // Opcode 0xf2
	void I386OP_D(sahf)();              // Opcode 0x9e
	void I386OP_D(sbb_rm8_r8)();        // Opcode 0x18
	void I386OP_D(sbb_r8_rm8)();        // Opcode 0x1a
	void I386OP_D(sbb_al_i8)();         // Opcode 0x1c
	void I386OP_D(scasb)();             // Opcode 0xae
	void I386OP_D(setalc)();            // Opcode 0xd6 (undocumented)
	void I386OP_D(seta_rm8)();          // Opcode 0x0f 97
	void I386OP_D(setbe_rm8)();         // Opcode 0x0f 96
	void I386OP_D(setc_rm8)();          // Opcode 0x0f 92
	void I386OP_D(setg_rm8)();          // Opcode 0x0f 9f
	void I386OP_D(setge_rm8)();         // Opcode 0x0f 9d
	void I386OP_D(setl_rm8)();          // Opcode 0x0f 9c
	void I386OP_D(setle_rm8)();         // Opcode 0x0f 9e
	void I386OP_D(setnc_rm8)();         // Opcode 0x0f 93
	void I386OP_D(setno_rm8)();         // Opcode 0x0f 91
	void I386OP_D(setnp_rm8)();         // Opcode 0x0f 9b
	void I386OP_D(setns_rm8)();         // Opcode 0x0f 99
	void I386OP_D(setnz_rm8)();         // Opcode 0x0f 95
	void I386OP_D(seto_rm8)();          // Opcode 0x0f 90
	void I386OP_D(setp_rm8)();          // Opcode 0x0f 9a
	void I386OP_D(sets_rm8)();          // Opcode 0x0f 98
	void I386OP_D(setz_rm8)();          // Opcode 0x0f 94
	void I386OP_D(stc)();               // Opcode 0xf9
	void I386OP_D(std)();               // Opcode 0xfd
	void I386OP_D(sti)();               // Opcode 0xfb
	void I386OP_D(stosb)();             // Opcode 0xaa
	void I386OP_D(sub_rm8_r8)();        // Opcode 0x28
	void I386OP_D(sub_r8_rm8)();        // Opcode 0x2a
	void I386OP_D(sub_al_i8)();         // Opcode 0x2c
	void I386OP_D(test_al_i8)();        // Opcode 0xa8
	void I386OP_D(test_rm8_r8)();       // Opcode 0x84
	void I386OP_D(xchg_r8_rm8)();       // Opcode 0x86
	void I386OP_D(xor_rm8_r8)();        // Opcode 0x30
	void I386OP_D(xor_r8_rm8)();        // Opcode 0x32
	void I386OP_D(xor_al_i8)();         // Opcode 0x34
	void I386OP_D(group80_8)();         // Opcode 0x80
	void I386OP_D(groupC0_8)();         // Opcode 0xc0
	void I386OP_D(groupD0_8)();         // Opcode 0xd0
	void I386OP_D(groupD2_8)();         // Opcode 0xd2
	void I386OP_D(groupF6_8)();         // Opcode 0xf6
	void I386OP_D(groupFE_8)();         // Opcode 0xfe
	void I386OP_D(segment_CS)();        // Opcode 0x2e
	void I386OP_D(segment_DS)();        // Opcode 0x3e
	void I386OP_D(segment_ES)();        // Opcode 0x26
	void I386OP_D(segment_FS)();        // Opcode 0x64
	void I386OP_D(segment_GS)();        // Opcode 0x65
	void I386OP_D(segment_SS)();        // Opcode 0x36
	void I386OP_D(operand_size)();      // Opcode prefix 0x66
	void I386OP_D(address_size)();      // Opcode 0x67
	void I386OP_D(nop)();               // Opcode 0x90
	void I386OP_D(int3)();              // Opcode 0xcc
	virtual void I386OP_D(_int)();               // Opcode 0xcd
	void I386OP_D(into)();              // Opcode 0xce
	void I386OP_D(escape)();            // Opcodes 0xd8 - 0xdf
	void I386OP_D(hlt)();               // Opcode 0xf4
	void I386OP_D(decimal_adjust)(int direction);
	void I386OP_D(daa)();               // Opcode 0x27
	void I386OP_D(das)();               // Opcode 0x2f
	void I386OP_D(aaa)();               // Opcode 0x37
	void I386OP_D(aas)();               // Opcode 0x3f
	void I386OP_D(aad)();               // Opcode 0xd5
	void I386OP_D(aam)();               // Opcode 0xd4
	void I386OP_D(clts)();              // Opcode 0x0f 0x06
	void I386OP_D(wait)();              // Opcode 0x9B
	void I386OP_D(lock)();              // Opcode 0xf0
	void I386OP_D(mov_r32_tr)();        // Opcode 0x0f 24
	void I386OP_D(mov_tr_r32)();        // Opcode 0x0f 26
	void I386OP_D(loadall)();       // Opcode 0x0f 0x07 (0x0f 0x05 on 80286), undocumented
	void I386OP_D(invalid)();
	void I386OP_D(xlat)();          // Opcode 0xd7
	bool I386OP_D(pop_seg16)( int segment);
	bool I386OP_D(load_far_pointer16)(int s);
	bool I386OP_D(pop_seg32)( int segment);
//i486
	void I486OP_D(cpuid)();             // Opcode 0x0F A2
	void I486OP_D(invd)();              // Opcode 0x0f 08
	void I486OP_D(wbinvd)();            // Opcode 0x0f 09
	void I486OP_D(cmpxchg_rm8_r8)();    // Opcode 0x0f b0
	void I486OP_D(cmpxchg_rm16_r16)();  // Opcode 0x0f b1
	void I486OP_D(cmpxchg_rm32_r32)();  // Opcode 0x0f b1
	void I486OP_D(xadd_rm8_r8)();   // Opcode 0x0f c0
	void I486OP_D(xadd_rm16_r16)(); // Opcode 0x0f c1
	void I486OP_D(xadd_rm32_r32)(); // Opcode 0x0f c1
	void I486OP_D(group0F01_16)();      // Opcode 0x0f 01
	void I486OP_D(group0F01_32)();      // Opcode 0x0f 01
	void I486OP_D(bswap_eax)();     // Opcode 0x0f 38
	void I486OP_D(bswap_ecx)();     // Opcode 0x0f 39
	void I486OP_D(bswap_edx)();     // Opcode 0x0f 3A
	void I486OP_D(bswap_ebx)();     // Opcode 0x0f 3B
	void I486OP_D(bswap_esp)();     // Opcode 0x0f 3C
	void I486OP_D(bswap_ebp)();     // Opcode 0x0f 3D
	void I486OP_D(bswap_esi)();     // Opcode 0x0f 3E
	void I486OP_D(bswap_edi)();     // Opcode 0x0f 3F
	void I486OP_D(mov_cr_r32)();        // Opcode 0x0f 22
//Pentium, MMX, SSE.
	void PENTIUMOP_D(rdmsr)();          // Opcode 0x0f 32
	void PENTIUMOP_D(wrmsr)();          // Opcode 0x0f 30
	void PENTIUMOP_D(rdtsc)();          // Opcode 0x0f 31
	void PENTIUMOP_D(ud2)();    // Opcode 0x0f 0b
	void PENTIUMOP_D(rsm)();
	void PENTIUMOP_D(prefetch_m8)();    // Opcode 0x0f 18
	void PENTIUMOP_D(cmovo_r16_rm16)();    // Opcode 0x0f 40
	void PENTIUMOP_D(cmovo_r32_rm32)();    // Opcode 0x0f 40
	void PENTIUMOP_D(cmovno_r16_rm16)();    // Opcode 0x0f 41
	void PENTIUMOP_D(cmovno_r32_rm32)();    // Opcode 0x0f 41
	void PENTIUMOP_D(cmovb_r16_rm16)();    // Opcode 0x0f 42
	void PENTIUMOP_D(cmovb_r32_rm32)();    // Opcode 0x0f 42
	void PENTIUMOP_D(cmovae_r16_rm16)();    // Opcode 0x0f 43
	void PENTIUMOP_D(cmovae_r32_rm32)();    // Opcode 0x0f 43
	void PENTIUMOP_D(cmove_r16_rm16)();    // Opcode 0x0f 44
	void PENTIUMOP_D(cmove_r32_rm32)();    // Opcode 0x0f 44
	void PENTIUMOP_D(cmovne_r16_rm16)();    // Opcode 0x0f 45
	void PENTIUMOP_D(cmovne_r32_rm32)();    // Opcode 0x0f 45
	void PENTIUMOP_D(cmovbe_r16_rm16)();    // Opcode 0x0f 46
	void PENTIUMOP_D(cmovbe_r32_rm32)();    // Opcode 0x0f 46
	void PENTIUMOP_D(cmova_r16_rm16)();    // Opcode 0x0f 47
	void PENTIUMOP_D(cmova_r32_rm32)();    // Opcode 0x0f 47
	void PENTIUMOP_D(cmovs_r16_rm16)();    // Opcode 0x0f 48
	void PENTIUMOP_D(cmovs_r32_rm32)();    // Opcode 0x0f 48
	void PENTIUMOP_D(cmovns_r16_rm16)();    // Opcode 0x0f 49
	void PENTIUMOP_D(cmovns_r32_rm32)();    // Opcode 0x0f 49
	void PENTIUMOP_D(cmovp_r16_rm16)();    // Opcode 0x0f 4a
	void PENTIUMOP_D(cmovp_r32_rm32)();    // Opcode 0x0f 4a
	void PENTIUMOP_D(cmovnp_r16_rm16)();    // Opcode 0x0f 4b
	void PENTIUMOP_D(cmovnp_r32_rm32)();    // Opcode 0x0f 4b
	void PENTIUMOP_D(cmovl_r16_rm16)();    // Opcode 0x0f 4c
	void PENTIUMOP_D(cmovl_r32_rm32)();    // Opcode 0x0f 4c
	void PENTIUMOP_D(cmovge_r16_rm16)();    // Opcode 0x0f 4d
	void PENTIUMOP_D(cmovge_r32_rm32)();    // Opcode 0x0f 4d
	void PENTIUMOP_D(cmovle_r16_rm16)();    // Opcode 0x0f 4e
	void PENTIUMOP_D(cmovle_r32_rm32)();    // Opcode 0x0f 4e
	void PENTIUMOP_D(cmovg_r16_rm16)();    // Opcode 0x0f 4f
	void PENTIUMOP_D(cmovg_r32_rm32)();    // Opcode 0x0f 4f
	void PENTIUMOP_D(movnti_m16_r16)(); // Opcode 0f c3
	void PENTIUMOP_D(movnti_m32_r32)(); // Opcode 0f c3
	void PENTIUMOP_D(cmpxchg8b_m64)();  // Opcode 0x0f c7
	void PENTIUMOP_D(movntq_m64_r64)(); // Opcode 0f e7
	void PENTIUMOP_D(maskmovq_r64_r64)();  // Opcode 0f f7
	void SSEOP_D(maskmovdqu_r128_r128)();  // Opcode 66 0f f7
	void PENTIUMOP_D(popcnt_r16_rm16)();    // Opcode f3 0f b8
	void PENTIUMOP_D(popcnt_r32_rm32)();    // Opcode f3 0f b8
	void PENTIUMOP_D(tzcnt_r16_rm16)();
	void PENTIUMOP_D(tzcnt_r32_rm32)();
	void I386OP_D(cyrix_special)();
	void I386OP_D(cyrix_unknown)();
	void I386OP_D(cyrix_svdc)();
	void I386OP_D(cyrix_rsdc)();
	void I386OP_D(cyrix_svldt)();
	void I386OP_D(cyrix_rsldt)();
	void I386OP_D(cyrix_svts)();
	void I386OP_D(cyrix_rsts)();
	
	void MMXOP_D(group_0f71)();  // Opcode 0f 71
	void SSEOP_D(group_660f71)();  // Opcode 66 0f 71
	void MMXOP_D(group_0f72)();  // Opcode 0f 72
	void SSEOP_D(group_660f72)();  // Opcode 66 0f 72
	void MMXOP_D(group_0f73)();  // Opcode 0f 73
	void SSEOP_D(group_660f73)();  // Opcode 66 0f 73
	void MMXOP_D(psrlw_r64_rm64)();  // Opcode 0f d1
	void MMXOP_D(psrld_r64_rm64)();  // Opcode 0f d2
	void MMXOP_D(psrlq_r64_rm64)();  // Opcode 0f d3
	void MMXOP_D(paddq_r64_rm64)();  // Opcode 0f d4
	void MMXOP_D(pmullw_r64_rm64)();  // Opcode 0f d5
	void MMXOP_D(psubusb_r64_rm64)();  // Opcode 0f d8
	void MMXOP_D(psubusw_r64_rm64)();  // Opcode 0f d9
	void MMXOP_D(pand_r64_rm64)();  // Opcode 0f db
	void MMXOP_D(paddusb_r64_rm64)();  // Opcode 0f dc
	void MMXOP_D(paddusw_r64_rm64)();  // Opcode 0f dd
	void MMXOP_D(pandn_r64_rm64)();  // Opcode 0f df
	void MMXOP_D(psraw_r64_rm64)();  // Opcode 0f e1
	void MMXOP_D(psrad_r64_rm64)();  // Opcode 0f e2
	void MMXOP_D(pmulhw_r64_rm64)();  // Opcode 0f e5
	void MMXOP_D(psubsb_r64_rm64)();  // Opcode 0f e8
	void MMXOP_D(psubsw_r64_rm64)();  // Opcode 0f e9
	void MMXOP_D(por_r64_rm64)();  // Opcode 0f eb
	void MMXOP_D(paddsb_r64_rm64)();  // Opcode 0f ec
	void MMXOP_D(paddsw_r64_rm64)();  // Opcode 0f ed
	void MMXOP_D(pxor_r64_rm64)();  // Opcode 0f ef
	void MMXOP_D(psllw_r64_rm64)();  // Opcode 0f f1
	void MMXOP_D(pslld_r64_rm64)();  // Opcode 0f f2
	void MMXOP_D(psllq_r64_rm64)();  // Opcode 0f f3
	void MMXOP_D(pmaddwd_r64_rm64)();  // Opcode 0f f5
	void MMXOP_D(psubb_r64_rm64)();  // Opcode 0f f8
	void MMXOP_D(psubw_r64_rm64)();  // Opcode 0f f9
	void MMXOP_D(psubd_r64_rm64)();  // Opcode 0f fa
	void MMXOP_D(paddb_r64_rm64)();  // Opcode 0f fc
	void MMXOP_D(paddw_r64_rm64)();  // Opcode 0f fd
	void MMXOP_D(paddd_r64_rm64)();  // Opcode 0f fe
	void MMXOP_D(emms)(); // Opcode 0f 77
	void MMXOP_D(movd_r64_rm32)(); // Opcode 0f 6e
	void MMXOP_D(movq_r64_rm64)(); // Opcode 0f 6f
	void MMXOP_D(movd_rm32_r64)(); // Opcode 0f 7e
	void MMXOP_D(movq_rm64_r64)(); // Opcode 0f 7f
	void MMXOP_D(pcmpeqb_r64_rm64)(); // Opcode 0f 74
	void MMXOP_D(pcmpeqw_r64_rm64)(); // Opcode 0f 75
	void MMXOP_D(pcmpeqd_r64_rm64)(); // Opcode 0f 76
	void MMXOP_D(pshufw_r64_rm64_i8)(); // Opcode 0f 70
	void SSEOP_D(punpcklbw_r128_rm128)(); // Opcode 66 0f 60
	void SSEOP_D(punpcklwd_r128_rm128)();
	void SSEOP_D(punpckldq_r128_rm128)();
	void SSEOP_D(punpcklqdq_r128_rm128)();
	void MMXOP_D(punpcklbw_r64_r64m32)(); // Opcode 0f 60
	void MMXOP_D(punpcklwd_r64_r64m32)(); // Opcode 0f 61
	void MMXOP_D(punpckldq_r64_r64m32)(); // Opcode 0f 62
	void MMXOP_D(packsswb_r64_rm64)(); // Opcode 0f 63
	void MMXOP_D(pcmpgtb_r64_rm64)(); // Opcode 0f 64
	void MMXOP_D(pcmpgtw_r64_rm64)(); // Opcode 0f 65
	void MMXOP_D(pcmpgtd_r64_rm64)(); // Opcode 0f 66
	void MMXOP_D(packuswb_r64_rm64)(); // Opcode 0f 67
	void MMXOP_D(punpckhbw_r64_rm64)(); // Opcode 0f 68
	void MMXOP_D(punpckhwd_r64_rm64)(); // Opcode 0f 69
	void MMXOP_D(punpckhdq_r64_rm64)(); // Opcode 0f 6a
	void MMXOP_D(packssdw_r64_rm64)(); // Opcode 0f 6b
	void SSEOP_D(group_0fae)();  // Opcode 0f ae
	void SSEOP_D(cvttps2dq_r128_rm128)(); // Opcode f3 0f 5b
	void SSEOP_D(cvtss2sd_r128_r128m32)(); // Opcode f3 0f 5a
	void SSEOP_D(cvttss2si_r32_r128m32)(); // Opcode f3 0f 2c
	void SSEOP_D(cvtss2si_r32_r128m32)(); // Opcode f3 0f 2d
	void SSEOP_D(cvtsi2ss_r128_rm32)(); // Opcode f3 0f 2a
	void SSEOP_D(cvtpi2ps_r128_rm64)(); // Opcode 0f 2a
	void SSEOP_D(cvttps2pi_r64_r128m64)(); // Opcode 0f 2c
	void SSEOP_D(cvtps2pi_r64_r128m64)(); // Opcode 0f 2d
	void SSEOP_D(cvtps2pd_r128_r128m64)(); // Opcode 0f 5a
	void SSEOP_D(cvtdq2ps_r128_rm128)(); // Opcode 0f 5b
	void SSEOP_D(cvtdq2pd_r128_r128m64)(); // Opcode f3 0f e6
	void SSEOP_D(movss_r128_rm128)(); // Opcode f3 0f 10
	void SSEOP_D(movss_rm128_r128)(); // Opcode f3 0f 11
	void SSEOP_D(movsldup_r128_rm128)(); // Opcode f3 0f 12
	void SSEOP_D(movshdup_r128_rm128)(); // Opcode f3 0f 16
	void SSEOP_D(movaps_r128_rm128)(); // Opcode 0f 28
	void SSEOP_D(movaps_rm128_r128)(); // Opcode 0f 29
	void SSEOP_D(movups_r128_rm128)(); // Opcode 0f 10
	void SSEOP_D(movupd_r128_rm128)(); // Opcode 66 0f 10
	void SSEOP_D(movups_rm128_r128)(); // Opcode 0f 11
	void SSEOP_D(movupd_rm128_r128)(); // Opcode 66 0f 11
	void SSEOP_D(movlps_r128_m64)(); // Opcode 0f 12
	void SSEOP_D(movlpd_r128_m64)(); // Opcode 66 0f 12
	void SSEOP_D(movlps_m64_r128)(); // Opcode 0f 13
	void SSEOP_D(movlpd_m64_r128)(); // Opcode 66 0f 13
	void SSEOP_D(movhps_r128_m64)(); // Opcode 0f 16
	void SSEOP_D(movhpd_r128_m64)(); // Opcode 66 0f 16
	void SSEOP_D(movhps_m64_r128)(); // Opcode 0f 17
	void SSEOP_D(movhpd_m64_r128)(); // Opcode 66 0f 17
	void SSEOP_D(movntps_m128_r128)(); // Opcode 0f 2b
	void SSEOP_D(movmskps_r16_r128)(); // Opcode 0f 50
	void SSEOP_D(movmskps_r32_r128)(); // Opcode 0f 50
	void SSEOP_D(movmskpd_r32_r128)(); // Opcode 66 0f 50
	void SSEOP_D(movq2dq_r128_r64)(); // Opcode f3 0f d6
	void SSEOP_D(movdqu_r128_rm128)(); // Opcode f3 0f 6f
	void SSEOP_D(movdqu_rm128_r128)(); // Opcode f3 0f 7f
	void SSEOP_D(movd_m128_rm32)(); // Opcode 66 0f 6e
	void SSEOP_D(movdqa_m128_rm128)(); // Opcode 66 0f 6f
	void SSEOP_D(movq_r128_r128m64)(); // Opcode f3 0f 7e
	void SSEOP_D(movd_rm32_r128)(); // Opcode 66 0f 7e
	void SSEOP_D(movdqa_rm128_r128)(); // Opcode 66 0f 7f
	void SSEOP_D(pmovmskb_r16_r64)(); // Opcode 0f d7
	void SSEOP_D(pmovmskb_r32_r64)(); // Opcode 0f d7
	void SSEOP_D(pmovmskb_r32_r128)(); // Opcode 66 0f d7
	void SSEOP_D(xorps)(); // Opcode 0f 57
	void SSEOP_D(xorpd_r128_rm128)(); // Opcode 66 0f 57
	void SSEOP_D(addps)(); // Opcode 0f 58
	void SSEOP_D(sqrtps_r128_rm128)(); // Opcode 0f 51
	void SSEOP_D(rsqrtps_r128_rm128)(); // Opcode 0f 52
	void SSEOP_D(rcpps_r128_rm128)(); // Opcode 0f 53
	void SSEOP_D(andps_r128_rm128)(); // Opcode 0f 54
	void SSEOP_D(andpd_r128_rm128)(); // Opcode 66 0f 54
	void SSEOP_D(andnps_r128_rm128)(); // Opcode 0f 55
	void SSEOP_D(andnpd_r128_rm128)(); // Opcode 66 0f 55
	void SSEOP_D(orps_r128_rm128)(); // Opcode 0f 56
	void SSEOP_D(orpd_r128_rm128)(); // Opcode 66 0f 56
	void SSEOP_D(mulps)(); // Opcode 0f 59 ????
	void SSEOP_D(subps)(); // Opcode 0f 5c
	void SSEOP_D(minps)(); // Opcode 0f 5d
	void SSEOP_D(divps)(); // Opcode 0f 5e
	void SSEOP_D(maxps)(); // Opcode 0f 5f
	void SSEOP_D(maxss_r128_r128m32)(); // Opcode f3 0f 5f
	void SSEOP_D(addss)(); // Opcode f3 0f 58
	void SSEOP_D(subss)(); // Opcode f3 0f 5c
	void SSEOP_D(mulss)(); // Opcode f3 0f 5e
	void SSEOP_D(divss)(); // Opcode 0f 59
	void SSEOP_D(rcpss_r128_r128m32)(); // Opcode f3 0f 53
	void SSEOP_D(sqrtss_r128_r128m32)(); // Opcode f3 0f 51
	void SSEOP_D(rsqrtss_r128_r128m32)(); // Opcode f3 0f 52
	void SSEOP_D(minss_r128_r128m32)(); // Opcode f3 0f 5d
	void SSEOP_D(comiss_r128_r128m32)(); // Opcode 0f 2f
	void SSEOP_D(comisd_r128_r128m64)(); // Opcode 66 0f 2f
	void SSEOP_D(ucomiss_r128_r128m32)(); // Opcode 0f 2e
	void SSEOP_D(ucomisd_r128_r128m64)(); // Opcode 66 0f 2e
	void SSEOP_D(shufps)(); // Opcode 0f c6
	void SSEOP_D(shufpd_r128_rm128_i8)(); // Opcode 66 0f c6
	void SSEOP_D(unpcklps_r128_rm128)(); // Opcode 0f 14
	void SSEOP_D(unpcklpd_r128_rm128)(); // Opcode 66 0f 14
	void SSEOP_D(unpckhps_r128_rm128)(); // Opcode 0f 15
	void SSEOP_D(unpckhpd_r128_rm128)(); // Opcode 66 0f 15
	void SSEOP_D(predicate_compare_single)(UINT8 imm8, XMM_REG d, XMM_REG s);
	void SSEOP_D(predicate_compare_double)(UINT8 imm8, XMM_REG d, XMM_REG s);
	void SSEOP_D(predicate_compare_single_scalar)(UINT8 imm8, XMM_REG d, XMM_REG s);
	void SSEOP_D(predicate_compare_double_scalar)(UINT8 imm8, XMM_REG d, XMM_REG s);
	void SSEOP_D(cmpps_r128_rm128_i8)(); // Opcode 0f c2
	void SSEOP_D(cmppd_r128_rm128_i8)(); // Opcode 66 0f c2
	void SSEOP_D(cmpss_r128_r128m32_i8)(); // Opcode f3 0f c2
	void SSEOP_D(pinsrw_r64_r16m16_i8)(); // Opcode 0f c4, 16bit register
	void SSEOP_D(pinsrw_r64_r32m16_i8)(); // Opcode 0f c4, 32bit register
	void SSEOP_D(pinsrw_r128_r32m16_i8)(); // Opcode 66 0f c4
	void SSEOP_D(pextrw_r16_r64_i8)(); // Opcode 0f c5
	void SSEOP_D(pextrw_r32_r64_i8)(); // Opcode 0f c5
	void SSEOP_D(pextrw_reg_r128_i8)(); // Opcode 66 0f c5
	void SSEOP_D(pminub_r64_rm64)(); // Opcode 0f da
	void SSEOP_D(pminub_r128_rm128)(); // Opcode 66 0f da
	void SSEOP_D(pmaxub_r64_rm64)(); // Opcode 0f de
	void SSEOP_D(pavgb_r64_rm64)(); // Opcode 0f e0
	void SSEOP_D(pavgw_r64_rm64)(); // Opcode 0f e3
	void SSEOP_D(pmulhuw_r64_rm64)();  // Opcode 0f e4
	void SSEOP_D(pminsw_r64_rm64)(); // Opcode 0f ea
	void SSEOP_D(pmaxsw_r64_rm64)(); // Opcode 0f ee
	void SSEOP_D(pmuludq_r64_rm64)(); // Opcode 0f f4
	void SSEOP_D(pmuludq_r128_rm128)(); // Opcode 66 0f f4
	void SSEOP_D(psadbw_r64_rm64)(); // Opcode 0f f6
	void SSEOP_D(psubq_r64_rm64)();  // Opcode 0f fb
	void SSEOP_D(psubq_r128_rm128)();  // Opcode 66 0f fb
	void SSEOP_D(pshufd_r128_rm128_i8)(); // Opcode 66 0f 70
	void SSEOP_D(pshuflw_r128_rm128_i8)(); // Opcode f2 0f 70
	void SSEOP_D(pshufhw_r128_rm128_i8)(); // Opcode f3 0f 70
	void SSEOP_D(packsswb_r128_rm128)(); // Opcode 66 0f 63
	void SSEOP_D(packssdw_r128_rm128)(); // Opcode 66 0f 6b
	void SSEOP_D(pcmpgtb_r128_rm128)(); // Opcode 66 0f 64
	void SSEOP_D(pcmpgtw_r128_rm128)(); // Opcode 66 0f 65
	void SSEOP_D(pcmpgtd_r128_rm128)(); // Opcode 66 0f 66
	void SSEOP_D(packuswb_r128_rm128)(); // Opcode 66 0f 67
	void SSEOP_D(punpckhbw_r128_rm128)(); // Opcode 66 0f 68
	void SSEOP_D(punpckhwd_r128_rm128)(); // Opcode 66 0f 69
	void SSEOP_D(unpckhdq_r128_rm128)(); // Opcode 66 0f 6a
	void SSEOP_D(punpckhqdq_r128_rm128)(); // Opcode 66 0f 6d
	void SSEOP_D(pcmpeqb_r128_rm128)(); // Opcode 66 0f 74
	void SSEOP_D(pcmpeqw_r128_rm128)(); // Opcode 66 0f 75
	void SSEOP_D(pcmpeqd_r128_rm128)(); // Opcode 66 0f 76
	void SSEOP_D(paddq_r128_rm128)();  // Opcode 66 0f d4
	void SSEOP_D(pmullw_r128_rm128)();  // Opcode 66 0f d5
	void SSEOP_D(paddb_r128_rm128)();  // Opcode 66 0f fc
	void SSEOP_D(paddw_r128_rm128)();  // Opcode 66 0f fd
	void SSEOP_D(paddd_r128_rm128)();  // Opcode 66 0f fe
	void SSEOP_D(psubusb_r128_rm128)();  // Opcode 66 0f d8
	void SSEOP_D(psubusw_r128_rm128)();  // Opcode 66 0f d9
	void SSEOP_D(pand_r128_rm128)();  // Opcode 66 0f db
	void SSEOP_D(pandn_r128_rm128)();  // Opcode 66 0f df
	void SSEOP_D(paddusb_r128_rm128)();  // Opcode 66 0f dc
	void SSEOP_D(paddusw_r128_rm128)();  // Opcode 66 0f dd
	void SSEOP_D(pmaxub_r128_rm128)(); // Opcode 66 0f de
	void SSEOP_D(pmulhuw_r128_rm128)();  // Opcode 66 0f e4
	void SSEOP_D(pmulhw_r128_rm128)();  // Opcode 66 0f e5
	void SSEOP_D(psubsb_r128_rm128)();  // Opcode 66 0f e8
	void SSEOP_D(psubsw_r128_rm128)();  // Opcode 66 0f e9
	void SSEOP_D(pminsw_r128_rm128)(); // Opcode 66 0f ea
	void SSEOP_D(pmaxsw_r128_rm128)(); // Opcode 66 0f ee
	void SSEOP_D(paddsb_r128_rm128)();  // Opcode 66 0f ec
	void SSEOP_D(paddsw_r128_rm128)();  // Opcode 66 0f ed
	void SSEOP_D(por_r128_rm128)();  // Opcode 66 0f eb
	void SSEOP_D(pxor_r128_rm128)();  // Opcode 66 0f ef
	void SSEOP_D(pmaddwd_r128_rm128)();  // Opcode 66 0f f5
	void SSEOP_D(psubb_r128_rm128)();  // Opcode 66 0f f8
	void SSEOP_D(psubw_r128_rm128)();  // Opcode 66 0f f9
	void SSEOP_D(psubd_r128_rm128)();  // Opcode 66 0f fa
	void SSEOP_D(psadbw_r128_rm128)(); // Opcode 66 0f f6
	void SSEOP_D(pavgb_r128_rm128)(); // Opcode 66 0f e0
	void SSEOP_D(pavgw_r128_rm128)(); // Opcode 66 0f e3
	void SSEOP_D(psrlw_r128_rm128)();  // Opcode 66 0f d1
	void SSEOP_D(psrld_r128_rm128)();  // Opcode 66 0f d2
	void SSEOP_D(psrlq_r128_rm128)();  // Opcode 66 0f d3
	void SSEOP_D(psllw_r128_rm128)();  // Opcode 66 0f f1
	void SSEOP_D(pslld_r128_rm128)();  // Opcode 66 0f f2
	void SSEOP_D(psllq_r128_rm128)();  // Opcode 66 0f f3
	void SSEOP_D(psraw_r128_rm128)();  // Opcode 66 0f e1
	void SSEOP_D(psrad_r128_rm128)();  // Opcode 66 0f e2
	void SSEOP_D(movntdq_m128_r128)();  // Opcode 66 0f e7
	void SSEOP_D(cvttpd2dq_r128_rm128)();  // Opcode 66 0f e6
	void SSEOP_D(movq_r128m64_r128)();  // Opcode 66 0f d6
	void SSEOP_D(addsubpd_r128_rm128)();  // Opcode 66 0f d0
	void SSEOP_D(haddpd_r128_rm128)();  // Opcode 66 0f 7c
	void SSEOP_D(hsubpd_r128_rm128)();  // Opcode 66 0f 7d
	void SSEOP_D(sqrtpd_r128_rm128)();  // Opcode 66 0f 51
	void SSEOP_D(cvtpi2pd_r128_rm64)();  // Opcode 66 0f 2a
	void SSEOP_D(cvttpd2pi_r64_rm128)();  // Opcode 66 0f 2c
	void SSEOP_D(cvtpd2pi_r64_rm128)();  // Opcode 66 0f 2d
	void SSEOP_D(cvtpd2ps_r128_rm128)();  // Opcode 66 0f 5a
	void SSEOP_D(cvtps2dq_r128_rm128)();  // Opcode 66 0f 5b
	void SSEOP_D(addpd_r128_rm128)();  // Opcode 66 0f 58
	void SSEOP_D(mulpd_r128_rm128)();  // Opcode 66 0f 59
	void SSEOP_D(subpd_r128_rm128)();  // Opcode 66 0f 5c
	void SSEOP_D(minpd_r128_rm128)();  // Opcode 66 0f 5d
	void SSEOP_D(divpd_r128_rm128)();  // Opcode 66 0f 5e
	void SSEOP_D(maxpd_r128_rm128)();  // Opcode 66 0f 5f
	void SSEOP_D(movntpd_m128_r128)();  // Opcode 66 0f 2b
	void SSEOP_D(movapd_r128_rm128)();  // Opcode 66 0f 28
	void SSEOP_D(movapd_rm128_r128)();  // Opcode 66 0f 29
	void SSEOP_D(movsd_r128_r128m64)(); // Opcode f2 0f 10
	void SSEOP_D(movsd_r128m64_r128)(); // Opcode f2 0f 11
	void SSEOP_D(movddup_r128_r128m64)(); // Opcode f2 0f 12
	void SSEOP_D(cvtsi2sd_r128_rm32)(); // Opcode f2 0f 2a
	void SSEOP_D(cvttsd2si_r32_r128m64)(); // Opcode f2 0f 2c
	void SSEOP_D(cvtsd2si_r32_r128m64)(); // Opcode f2 0f 2d
	void SSEOP_D(sqrtsd_r128_r128m64)(); // Opcode f2 0f 51
	void SSEOP_D(addsd_r128_r128m64)(); // Opcode f2 0f 58
	void SSEOP_D(mulsd_r128_r128m64)(); // Opcode f2 0f 59
	void SSEOP_D(cvtsd2ss_r128_r128m64)(); // Opcode f2 0f 5a
	void SSEOP_D(subsd_r128_r128m64)(); // Opcode f2 0f 5c
	void SSEOP_D(minsd_r128_r128m64)(); // Opcode f2 0f 5d
	void SSEOP_D(divsd_r128_r128m64)(); // Opcode f2 0f 5e
	void SSEOP_D(maxsd_r128_r128m64)(); // Opcode f2 0f 5f
	void SSEOP_D(haddps_r128_rm128)(); // Opcode f2 0f 7c
	void SSEOP_D(hsubps_r128_rm128)(); // Opcode f2 0f 7d
	void SSEOP_D(cmpsd_r128_r128m64_i8)(); // Opcode f2 0f c2
	void SSEOP_D(addsubps_r128_rm128)(); // Opcode f2 0f d0
	void SSEOP_D(movdq2q_r64_r128)(); // Opcode f2 0f d6
	void SSEOP_D(cvtpd2dq_r128_rm128)(); // Opcode f2 0f e6
	void SSEOP_D(lddqu_r128_m128)(); // Opcode f2 0f f0
	// x87 FPU
	void x87_write_stack( int i, floatx80 value, int update_tag);
	void x87_reset();
	void x87_fadd_m32real(UINT8 modrm);
	void x87_fadd_m64real(UINT8 modrm);
	void x87_fadd_st_sti(UINT8 modrm);
	void x87_fadd_sti_st(UINT8 modrm);
	void x87_faddp(UINT8 modrm);
	void x87_fiadd_m32int(UINT8 modrm);
	void x87_fiadd_m16int(UINT8 modrm);
	void x87_fsub_m32real(UINT8 modrm);
	void x87_fsub_m64real(UINT8 modrm);
	void x87_fsub_st_sti(UINT8 modrm);
	void x87_fsub_sti_st( UINT8 modrm);
	void x87_fsubp(UINT8 modrm);
	void x87_fisub_m32int(UINT8 modrm);
	void x87_fisub_m16int(UINT8 modrm);
	void x87_fsubr_m32real(UINT8 modrm);
	void x87_fsubr_m64real(UINT8 modrm);
	void x87_fsubr_st_sti(UINT8 modrm);
	void x87_fsubr_sti_st(UINT8 modrm);
	void x87_fsubrp(UINT8 modrm);
	void x87_fisubr_m32int(UINT8 modrm);
	void x87_fisubr_m16int(UINT8 modrm);
	void x87_fdiv_m32real(UINT8 modrm);
	void x87_fdiv_m64real(UINT8 modrm);
	void x87_fdiv_st_sti(UINT8 modrm);
	void x87_fdiv_sti_st(UINT8 modrm);
	void x87_fdivp(UINT8 modrm);
	void x87_fidiv_m32int(UINT8 modrm);
	void x87_fidiv_m16int(UINT8 modrm);
	void x87_fdivr_m32real(UINT8 modrm);
	void x87_fdivr_m64real(UINT8 modrm);
	void x87_fdivr_st_sti(UINT8 modrm);
	void x87_fdivr_sti_st(UINT8 modrm);
	void x87_fdivrp(UINT8 modrm);
	void x87_fidivr_m32int( UINT8 modrm);
	void x87_fidivr_m16int( UINT8 modrm);
	void x87_fmul_m32real( UINT8 modrm);
	void x87_fmul_m64real( UINT8 modrm);
	void x87_fmul_st_sti( UINT8 modrm);
	void x87_fmul_sti_st( UINT8 modrm);
	void x87_fmulp( UINT8 modrm);
	void x87_fimul_m32int( UINT8 modrm);
	void x87_fimul_m16int( UINT8 modrm);
	void x87_fcmovb_sti( UINT8 modrm);
	void x87_fcmove_sti( UINT8 modrm);
	void x87_fcmovbe_sti( UINT8 modrm);
	void x87_fcmovu_sti( UINT8 modrm);
	void x87_fcmovnb_sti( UINT8 modrm);
	void x87_fcmovne_sti( UINT8 modrm);
	void x87_fcmovnbe_sti( UINT8 modrm);
	void x87_fcmovnu_sti( UINT8 modrm);
	void x87_fprem( UINT8 modrm);
	void x87_fprem1( UINT8 modrm);
	void x87_fsqrt( UINT8 modrm);
	void x87_f2xm1( UINT8 modrm);
	void x87_fyl2x( UINT8 modrm);
	void x87_fyl2xp1( UINT8 modrm);
	void x87_fptan( UINT8 modrm);
	void x87_fpatan( UINT8 modrm);
	void x87_fsin( UINT8 modrm);
	void x87_fcos( UINT8 modrm);
	void x87_fsincos( UINT8 modrm);
	void x87_fld_m32real( UINT8 modrm);
	void x87_fld_m64real( UINT8 modrm);
	void x87_fld_m80real( UINT8 modrm);
	void x87_fld_sti( UINT8 modrm);
	void x87_fild_m16int( UINT8 modrm);
	void x87_fild_m32int( UINT8 modrm);
	void x87_fild_m64int( UINT8 modrm);
	void x87_fbld( UINT8 modrm);
	void x87_fst_m32real( UINT8 modrm);
	void x87_fst_m64real( UINT8 modrm);
	void x87_fst_sti( UINT8 modrm);
	void x87_fstp_m32real( UINT8 modrm);
	void x87_fstp_m64real( UINT8 modrm);
	void x87_fstp_m80real( UINT8 modrm);
	void x87_fstp_sti( UINT8 modrm);
	void x87_fist_m16int( UINT8 modrm);
	void x87_fist_m32int( UINT8 modrm);
	void x87_fistp_m16int( UINT8 modrm);
	void x87_fistp_m32int( UINT8 modrm);
	void x87_fistp_m64int( UINT8 modrm);
	void x87_fbstp( UINT8 modrm);
	void x87_fld1( UINT8 modrm);
	void x87_fldl2t( UINT8 modrm);
	void x87_fldl2e( UINT8 modrm);
	void x87_fldpi( UINT8 modrm);
	void x87_fldlg2( UINT8 modrm);
	void x87_fldln2( UINT8 modrm);
	void x87_fldz( UINT8 modrm);
	void x87_fnop( UINT8 modrm);
	void x87_fchs( UINT8 modrm);
	void x87_fabs( UINT8 modrm);
	void x87_fscale( UINT8 modrm);
	void x87_frndint( UINT8 modrm);
	void x87_fxtract( UINT8 modrm);
	void x87_ftst( UINT8 modrm);
	void x87_fxam( UINT8 modrm);
	void x87_ficom_m16int( UINT8 modrm);
	void x87_ficom_m32int( UINT8 modrm);
	void x87_ficomp_m16int( UINT8 modrm);
	void x87_ficomp_m32int( UINT8 modrm);
	void x87_fcom_m32real( UINT8 modrm);
	void x87_fcom_m64real( UINT8 modrm);
	void x87_fcom_sti( UINT8 modrm);
	void x87_fcomp_m32real( UINT8 modrm);
	void x87_fcomp_m64real( UINT8 modrm);
	void x87_fcomp_sti( UINT8 modrm);
	void x87_fcomi_sti( UINT8 modrm);
	void x87_fcomip_sti( UINT8 modrm);
	void x87_fucomi_sti( UINT8 modrm);
	void x87_fucomip_sti( UINT8 modrm);
	void x87_fcompp( UINT8 modrm);
	void x87_fucom_sti( UINT8 modrm);
	void x87_fucomp_sti( UINT8 modrm);
	void x87_fucompp( UINT8 modrm);
	void x87_fdecstp( UINT8 modrm);
	void x87_fincstp( UINT8 modrm);
	void x87_fclex( UINT8 modrm);
	void x87_feni( UINT8 modrm);
	void x87_fdisi( UINT8 modrm);
	void x87_ffree( UINT8 modrm);
	void x87_finit( UINT8 modrm);
	void x87_fldcw( UINT8 modrm);
	void x87_fstcw( UINT8 modrm);
	void x87_fldenv( UINT8 modrm);
	void x87_fstenv( UINT8 modrm);
	void x87_fsave( UINT8 modrm);
	void x87_frstor( UINT8 modrm);
	void x87_fxch( UINT8 modrm);
	void x87_fxch_sti( UINT8 modrm);
	void x87_fstsw_ax( UINT8 modrm);
	void x87_fstsw_m2byte( UINT8 modrm);
	void x87_invalid( UINT8 modrm);
	void I386OP_D(x87_group_d8)();
	void I386OP_D(x87_group_d9)();
	void I386OP_D(x87_group_da)();
	void I386OP_D(x87_group_db)();
	void I386OP_D(x87_group_dc)();
	void I386OP_D(x87_group_dd)();
	void I386OP_D(x87_group_de)();
	void I386OP_D(x87_group_df)();
	void build_x87_opcode_table_d8();
	void build_x87_opcode_table_d9();
	void build_x87_opcode_table_da();
	void build_x87_opcode_table_db();
	void build_x87_opcode_table_dc();
	void build_x87_opcode_table_dd();
	void build_x87_opcode_table_de();
	void build_x87_opcode_table_df();
	void build_x87_opcode_table();

	floatx80 x87_add( floatx80 a, floatx80 b);
	floatx80 x87_sub( floatx80 a, floatx80 b);
	floatx80 x87_mul( floatx80 a, floatx80 b);
	floatx80 x87_div( floatx80 a, floatx80 b);

	void I386OP_D(decode_two_byte)();
	void I386OP_D(decode_three_byte38)();
	void I386OP_D(decode_three_byte3a)();
	void I386OP_D(decode_three_byte66)();
	void I386OP_D(decode_three_bytef2)();
	void I386OP_D(decode_three_bytef3)();
	void I386OP_D(decode_four_byte3866)();
	void I386OP_D(decode_four_byte3a66)();
	void I386OP_D(decode_four_byte3af2)();
	void I386OP_D(decode_four_byte38f2)();
	void I386OP_D(decode_four_byte38f3)();
protected:

	// Inline Utilities.
	INLINE INT8 SaturatedSignedWordToSignedByte(INT16 word);
	INLINE UINT8 SaturatedSignedWordToUnsignedByte(INT16 word);
	INLINE INT16 SaturatedSignedDwordToSignedWord(INT32 dword);
	INLINE UINT16 SaturatedSignedDwordToUnsignedWord(INT32 dword);
	//
	INLINE vtlb_entry get_permissions(UINT32 pte, int wp);
	INLINE int translate_address(int pl, int type, UINT32 *address, UINT32 *error);
	INLINE UINT32 i386_translate(int segment, UINT32 ip, int rwn, UINT32 size);
	
	INLINE UINT8 FETCH();
	INLINE UINT16 FETCH16();
	INLINE UINT32 FETCH32();
	
	INLINE UINT8 READ8(UINT32 ea);
	INLINE UINT8 READ8PL0(UINT32 ea);
	INLINE UINT16 READ16(UINT32 ea);
	INLINE UINT16 READ16PL0(UINT32 ea);
	INLINE UINT32 READ32(UINT32 ea);
	INLINE UINT32 READ32PL0(UINT32 ea);
	INLINE UINT64 READ64(UINT32 ea);
	
	INLINE void WRITE8(UINT32 ea, UINT8 value);
	INLINE void WRITE16(UINT32 ea, UINT16 value);
	INLINE void WRITE32(UINT32 ea, UINT32 value);
	INLINE void WRITE64(UINT32 ea, UINT64 value);
	INLINE void WRITE_TEST(UINT32 ea);

	INLINE void WRITEPORT8(offs_t port, UINT8 value);
	INLINE void WRITEPORT16(offs_t port, UINT16 value);
	INLINE void WRITEPORT32(offs_t port, UINT32 value);
	INLINE UINT8 READPORT8( offs_t port);
	INLINE UINT16 READPORT16( offs_t port);
	INLINE UINT32 READPORT32( offs_t port);
	
	INLINE void PUSH8(UINT8 value);
	INLINE UINT8 POP8();
	INLINE void PUSH16(UINT16 value);
	INLINE UINT16 POP16();
	INLINE void PUSH32(UINT32 value);
	INLINE void PUSH32SEG(UINT32 value);
	INLINE UINT32 POP32();
	
	INLINE UINT8 OR8(UINT8 dst, UINT8 src);
	INLINE UINT8 AND8(UINT8 dst, UINT8 src);
	INLINE UINT8 XOR8(UINT8 dst, UINT8 src);
	INLINE UINT8 SBB8(UINT8 dst, UINT8 src, UINT8 b);
	INLINE UINT8 ADC8(UINT8 dst, UINT8 src, UINT8 c);
	INLINE UINT8 INC8(UINT8 dst);
	INLINE UINT8 DEC8(UINT8 dst);

	INLINE UINT16 OR16(UINT16 dst, UINT16 src);
	INLINE UINT16 AND16(UINT16 dst, UINT16 src);
	INLINE UINT16 XOR16(UINT16 dst, UINT16 src);
	INLINE UINT16 SBB16(UINT16 dst, UINT16 src, UINT16 b);
	INLINE UINT16 ADC16(UINT16 dst, UINT16 src, UINT8 c);
	INLINE UINT16 INC16(UINT16 dst);
	INLINE UINT16 DEC16(UINT16 dst);

	INLINE UINT32 OR32(UINT32 dst, UINT32 src);
	INLINE UINT32 AND32(UINT32 dst, UINT32 src);
	INLINE UINT32 XOR32(UINT32 dst, UINT32 src);
	INLINE UINT32 SBB32(UINT32 dst, UINT32 src, UINT32 b);
	INLINE UINT32 ADC32(UINT32 dst, UINT32 src, UINT32 c);
	INLINE UINT32 INC32(UINT32 dst);
	INLINE UINT32 DEC32(UINT32 dst);
	
	INLINE UINT64 MSR_READ(UINT32 offset,UINT8 *valid_msr);
	INLINE void MSR_WRITE(UINT32 offset, UINT64 data, UINT8 *valid_msr);
	
	INLINE void CHANGE_PC(UINT32 pc);
	INLINE void NEAR_BRANCH(INT32 offs);
	INLINE void BUMP_SI(int adjustment);
	INLINE void BUMP_DI(int adjustment);
	INLINE void CYCLES(int x);
	INLINE void CYCLES_RM(int modrm, int r, int m);

	INLINE void MMXPROLOG();
	INLINE void READMMX(UINT32 ea,MMX_REG &r);
	INLINE void WRITEMMX(UINT32 ea,MMX_REG &r);
	INLINE void READXMM(UINT32 ea,XMM_REG &r);
	INLINE void WRITEXMM(UINT32 ea,XMM_REG &r);
	INLINE void READXMM_LO64(UINT32 ea,XMM_REG &r);
	INLINE void WRITEXMM_LO64(UINT32 ea,XMM_REG &r);
	INLINE void READXMM_HI64(UINT32 ea,XMM_REG &r);
	INLINE void WRITEXMM_HI64(UINT32 ea,XMM_REG &r);

	INLINE flag floatx80_is_quiet_nan(floatx80 a);
	INLINE int floatx80_is_zero(floatx80 fx);
	INLINE int floatx80_is_inf(floatx80 fx);
	INLINE int floatx80_is_denormal(floatx80 fx);
	INLINE floatx80 floatx80_abs(floatx80 fx);

	INLINE UINT64 __SWAP64(UINT64 in);
	INLINE double fx80_to_double(floatx80 fx);
	INLINE floatx80 double_to_fx80(double in);
	INLINE floatx80 READ80( UINT32 ea);
	INLINE void WRITE80( UINT32 ea, floatx80 t);
	INLINE void x87_set_stack_top(int top);
	INLINE void x87_set_tag(int reg, int tag);
	INLINE void x87_set_stack_underflow();
	INLINE void x87_set_stack_overflow();
	INLINE void x87_write_cw( UINT16 cw);

	UINT32 I386OP_D(shift_rotate32)(UINT8 modrm, UINT32 value, UINT8 shift);

	UINT64 pentium_msr_read(UINT32 offset,UINT8 *valid_msr);;
	void pentium_msr_write(UINT32 offset, UINT64 data, UINT8 *valid_msr);
	UINT64 p6_msr_read(UINT32 offset,UINT8 *valid_msr);;
	void p6_msr_write(UINT32 offset, UINT64 data, UINT8 *valid_msr);
	UINT64 piv_msr_read(UINT32 offset,UINT8 *valid_msr);;
	void piv_msr_write(UINT32 offset, UINT64 data, UINT8 *valid_msr);
	
	int x87_inc_stack();
	int x87_dec_stack();
	int x87_check_exceptions();
public:

};
extern const X86_OPCODE x86_opcode_table[];
extern const X86_CYCLE_TABLE x86_cycle_table[];

/***********************************************************************************/

#define CYCLES_NUM(x)   (cpustate->cycles -= (x))

INLINE void I386_OPS_BASE::CYCLES(int x)
{
	if (PROTECTED_MODE)
	{
		cpustate->cycles -= cpustate->cycle_table_pm[x];
	}
	else
	{
		cpustate->cycles -= cpustate->cycle_table_rm[x];
	}
}

INLINE void I386_OPS_BASE::CYCLES_RM(int modrm, int r, int m)
{
	if (modrm >= 0xc0)
	{
		if (PROTECTED_MODE)
		{
			cpustate->cycles -= cpustate->cycle_table_pm[r];
		}
		else
		{
			cpustate->cycles -= cpustate->cycle_table_rm[r];
		}
	}
	else
	{
		if (PROTECTED_MODE)
		{
			cpustate->cycles -= cpustate->cycle_table_pm[m];
		}
		else
		{
			cpustate->cycles -= cpustate->cycle_table_rm[m];
		}
	}
}


INLINE UINT32 I386_OPS_BASE::i386_translate(int segment, UINT32 ip, int rwn, UINT32 size)
{
	// TODO: segment limit access size, execution permission, handle exception thrown from exception handler
	if(PROTECTED_MODE && !V8086_MODE && (rwn != -1))
	{
		if(!(cpustate->sreg[segment].valid))
			FAULT_THROW((segment==SS)?FAULT_SS:FAULT_GP, 0);
		if(i386_limit_check(segment, ip, size))
			FAULT_THROW((segment==SS)?FAULT_SS:FAULT_GP, 0);
		if((rwn == 0) && ((cpustate->sreg[segment].flags & 8) && !(cpustate->sreg[segment].flags & 2)))
			FAULT_THROW(FAULT_GP, 0);
		if((rwn == 1) && ((cpustate->sreg[segment].flags & 8) || !(cpustate->sreg[segment].flags & 2)))
			FAULT_THROW(FAULT_GP, 0);
	}
	return cpustate->sreg[segment].base + ip;
}

#define VTLB_FLAG_DIRTY 0x100

INLINE vtlb_entry I386_OPS_BASE::get_permissions(UINT32 pte, int wp)
{
	vtlb_entry ret = VTLB_READ_ALLOWED | ((pte & 4) ? VTLB_USER_READ_ALLOWED : 0);
	if(!wp)
		ret |= VTLB_WRITE_ALLOWED;
	if(pte & 2)
		ret |= VTLB_WRITE_ALLOWED | ((pte & 4) ? VTLB_USER_WRITE_ALLOWED : 0);
	return ret;
}

//#define TEST_TLB

INLINE int I386_OPS_BASE::translate_address(int pl, int type, UINT32 *address, UINT32 *error)
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
		if(!i386_translate_address( type, address, &entry))
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
	int test_ret = i386_translate_address( type | TRANSLATE_DEBUG_MASK, &test_addr, NULL);
	if(!test_ret || (test_addr != *address))
		logerror("TLB-PTE mismatch! %06X %06X %06x\n", *address, test_addr, cpustate->pc);
#endif
	return TRUE;
}

INLINE void I386_OPS_BASE::CHANGE_PC(UINT32 pc)
{
	cpustate->pc = i386_translate(CS, pc, -1, 1 );
}

INLINE void I386_OPS_BASE::NEAR_BRANCH(INT32 offs)
{
	/* TODO: limit */
	cpustate->eip += offs;
	cpustate->pc += offs;
}

INLINE UINT8 I386_OPS_BASE::FETCH()
{
	UINT8 value;
	UINT32 address = cpustate->pc, error;

	if(!translate_address(cpustate->CPL,TRANSLATE_FETCH,&address,&error))
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

INLINE UINT16 I386_OPS_BASE::FETCH16()
{
	UINT16 value;
	UINT32 address = cpustate->pc, error;

	if( !WORD_ALIGNED(address) ) {       /* Unaligned read */
		value = (FETCH() << 0);
		value |= (FETCH() << 8);
	} else {
		if(!translate_address(cpustate->CPL,TRANSLATE_FETCH,&address,&error))
			PF_THROW(error);
		address &= cpustate->a20_mask;
		value = cpustate->program->read_data16(address);
		cpustate->eip += 2;
		cpustate->pc += 2;
	}
	return value;
}
INLINE UINT32 I386_OPS_BASE::FETCH32()
{
	UINT32 value;
	UINT32 address = cpustate->pc, error;

	if( !DWORD_ALIGNED(cpustate->pc) ) {      /* Unaligned read */
		value = (FETCH() << 0);
		value |= (FETCH() << 8);
		value |= (FETCH() << 16);
		value |= (FETCH() << 24);
	} else {
		if(!translate_address(cpustate->CPL,TRANSLATE_FETCH,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		value = cpustate->program->read_data32(address);
		cpustate->eip += 4;
		cpustate->pc += 4;
	}
	return value;
}

INLINE UINT8 I386_OPS_BASE::READ8(UINT32 ea)
{
	UINT32 address = ea, error;

	if(!translate_address(cpustate->CPL,TRANSLATE_READ,&address, &error))
		PF_THROW(error);

	address &= cpustate->a20_mask;
	return cpustate->program->read_data8(address);
}

INLINE UINT16 I386_OPS_BASE::READ16(UINT32 ea)
{
	UINT16 value;
	UINT32 address = ea, error;

	if( !WORD_ALIGNED(ea) ) {      /* Unaligned read */
		value = (READ8(address+0) << 0);
		value |= (READ8(address+1) << 8);
	} else {
		if(!translate_address(cpustate->CPL,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		value = cpustate->program->read_data16( address );
	}
	return value;
}
INLINE UINT32 I386_OPS_BASE::READ32(UINT32 ea)
{
	UINT32 value;
	UINT32 address = ea, error;

	if( !DWORD_ALIGNED(ea) ) {      /* Unaligned read */
		value = (READ8(address+0) << 0);
		value |= (READ8(address+1) << 8);
		value |= (READ8(address+2) << 16),
		value |= (READ8(address+3) << 24);
	} else {
		if(!translate_address(cpustate->CPL,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		value = cpustate->program->read_data32( address );
	}
	return value;
}

INLINE UINT64 I386_OPS_BASE::READ64(UINT32 ea)
{
	UINT64 value;
	UINT32 address = ea, error;

	if( !QWORD_ALIGNED(ea) ) {      /* Unaligned read */
		value = (((UINT64) READ8( address+0 )) << 0);
		value |= (((UINT64) READ8( address+1 )) << 8);
		value |= (((UINT64) READ8( address+2 )) << 16);
		value |= (((UINT64) READ8( address+3 )) << 24);
		value |= (((UINT64) READ8( address+4 )) << 32);
		value |= (((UINT64) READ8( address+5 )) << 40);
		value |= (((UINT64) READ8( address+6 )) << 48);
		value |= (((UINT64) READ8( address+7 )) << 56);
	} else {
		if(!translate_address(cpustate->CPL,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		value = (((UINT64) cpustate->program->read_data32( address+0 )) << 0);
		value |= (((UINT64) cpustate->program->read_data32( address+4 )) << 32);
	}
	return value;
}

INLINE UINT8 I386_OPS_BASE::READ8PL0(UINT32 ea)
{
	UINT32 address = ea, error;

	if(!translate_address(0,TRANSLATE_READ,&address,&error))
		PF_THROW(error);

	address &= cpustate->a20_mask;
	return cpustate->program->read_data8(address);
}

INLINE UINT16 I386_OPS_BASE::READ16PL0(UINT32 ea)
{
	UINT16 value;
	UINT32 address = ea, error;

	if( !WORD_ALIGNED(ea) ) {      /* Unaligned read */
		value = (READ8PL0( address+0 ) << 0);
		value |= (READ8PL0( address+1 ) << 8);
	} else {
		if(!translate_address(0,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		value = cpustate->program->read_data16( address );
	}
	return value;
}

INLINE UINT32 I386_OPS_BASE::READ32PL0(UINT32 ea)
{
	UINT32 value;
	UINT32 address = ea, error;

	if( !DWORD_ALIGNED(ea) ) {      /* Unaligned read */
		value = (READ8PL0( address+0 ) << 0);
		value |= (READ8PL0( address+1 ) << 8);
		value |= (READ8PL0( address+2 ) << 16);
		value |= (READ8PL0( address+3 ) << 24);
	} else {
		if(!translate_address(0,TRANSLATE_READ,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		value = cpustate->program->read_data32( address );
	}
	return value;
}

INLINE void I386_OPS_BASE::WRITE_TEST(UINT32 ea)
{
	UINT32 address = ea, error;
	if(!translate_address(cpustate->CPL,TRANSLATE_WRITE,&address,&error))
		PF_THROW(error);
}

INLINE void I386_OPS_BASE::WRITE8(UINT32 ea, UINT8 value)
{
	UINT32 address = ea, error;

	if(!translate_address(cpustate->CPL,TRANSLATE_WRITE,&address,&error))
		PF_THROW(error);

	address &= cpustate->a20_mask;
	cpustate->program->write_data8(address, value);
}

INLINE void I386_OPS_BASE::WRITE16(UINT32 ea, UINT16 value)
{
	UINT32 address = ea, error;

	if( !WORD_ALIGNED(ea) ) {      /* Unaligned write */
		WRITE8( address+0, value & 0xff );
		WRITE8( address+1, (value >> 8) & 0xff );
	} else {
		if(!translate_address(cpustate->CPL,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		cpustate->program->write_data16(address, value);
	}
}

INLINE void I386_OPS_BASE::WRITE32(UINT32 ea, UINT32 value)
{
	UINT32 address = ea, error;

	if( !DWORD_ALIGNED(ea) ) {      /* Unaligned write */
		WRITE8( address+0, value & 0xff );
		WRITE8( address+1, (value >> 8) & 0xff );
		WRITE8( address+2, (value >> 16) & 0xff );
		WRITE8( address+3, (value >> 24) & 0xff );
	} else {
		if(!translate_address(cpustate->CPL,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		cpustate->program->write_data32(address, value);
	}
}

INLINE void I386_OPS_BASE::WRITE64(UINT32 ea, UINT64 value)
{
	UINT32 address = ea, error;

	if( !QWORD_ALIGNED(ea) ) {      /* Unaligned write */
		WRITE8( address+0, value & 0xff );
		WRITE8( address+1, (value >> 8) & 0xff );
		WRITE8( address+2, (value >> 16) & 0xff );
		WRITE8( address+3, (value >> 24) & 0xff );
		WRITE8( address+4, (value >> 32) & 0xff );
		WRITE8( address+5, (value >> 40) & 0xff );
		WRITE8( address+6, (value >> 48) & 0xff );
		WRITE8( address+7, (value >> 56) & 0xff );
	} else {
		if(!translate_address(cpustate->CPL,TRANSLATE_WRITE,&address,&error))
			PF_THROW(error);

		address &= cpustate->a20_mask;
		cpustate->program->write_data32(address+0, value & 0xffffffff);
		cpustate->program->write_data32(address+4, (value >> 32) & 0xffffffff);
	}
}

/***********************************************************************************/

INLINE UINT8 I386_OPS_BASE::OR8(UINT8 dst, UINT8 src)
{
	UINT8 res = dst | src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF8(res);
	return res;
}

INLINE UINT16 I386_OPS_BASE::OR16(UINT16 dst, UINT16 src)
{
	UINT16 res = dst | src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF16(res);
	return res;
}

INLINE UINT32 I386_OPS_BASE::OR32(UINT32 dst, UINT32 src)
{
	UINT32 res = dst | src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF32(res);
	return res;
}

INLINE UINT8 I386_OPS_BASE::AND8(UINT8 dst, UINT8 src)
{
	UINT8 res = dst & src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF8(res);
	return res;
}
INLINE UINT16 I386_OPS_BASE::AND16(UINT16 dst, UINT16 src)
{
	UINT16 res = dst & src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF16(res);
	return res;
}
INLINE UINT32 I386_OPS_BASE::AND32(UINT32 dst, UINT32 src)
{
	UINT32 res = dst & src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF32(res);
	return res;
}

INLINE UINT8 I386_OPS_BASE::XOR8(UINT8 dst, UINT8 src)
{
	UINT8 res = dst ^ src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF8(res);
	return res;
}
INLINE UINT16 I386_OPS_BASE::XOR16(UINT16 dst, UINT16 src)
{
	UINT16 res = dst ^ src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF16(res);
	return res;
}
INLINE UINT32 I386_OPS_BASE::XOR32(UINT32 dst, UINT32 src)
{
	UINT32 res = dst ^ src;
	cpustate->CF = cpustate->OF = 0;
	SetSZPF32(res);
	return res;
}

#define SUB8(dst, src) SBB8(dst, src, 0)
INLINE UINT8 I386_OPS_BASE::SBB8(UINT8 dst, UINT8 src, UINT8 b)
{
	UINT16 res = (UINT16)dst - (UINT16)src - (UINT8)b;
	SetCF8(res);
	SetOF_Sub8(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF8(res);
	return (UINT8)res;
}

#define SUB16(dst, src) SBB16(dst, src, 0)
INLINE UINT16 I386_OPS_BASE::SBB16(UINT16 dst, UINT16 src, UINT16 b)
{
	UINT32 res = (UINT32)dst - (UINT32)src - (UINT32)b;
	SetCF16(res);
	SetOF_Sub16(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF16(res);
	return (UINT16)res;
}

#define SUB32(dst, src) SBB32(dst, src, 0)
INLINE UINT32 I386_OPS_BASE::SBB32(UINT32 dst, UINT32 src, UINT32 b)
{
	UINT64 res = (UINT64)dst - (UINT64)src - (UINT64) b;
	SetCF32(res);
	SetOF_Sub32(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF32(res);
	return (UINT32)res;
}

#define ADD8(dst, src) ADC8(dst, src, 0)
INLINE UINT8 I386_OPS_BASE::ADC8(UINT8 dst, UINT8 src, UINT8 c)
{
	UINT16 res = (UINT16)dst + (UINT16)src + (UINT16)c;
	SetCF8(res);
	SetOF_Add8(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF8(res);
	return (UINT8)res;
}

#define ADD16(dst, src) ADC16(dst, src, 0)
INLINE UINT16 I386_OPS_BASE::ADC16(UINT16 dst, UINT16 src, UINT8 c)
{
	UINT32 res = (UINT32)dst + (UINT32)src + (UINT32)c;
	SetCF16(res);
	SetOF_Add16(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF16(res);
	return (UINT16)res;
}

#define ADD32(dst, src) ADC32(dst, src, 0)
INLINE UINT32 I386_OPS_BASE::ADC32(UINT32 dst, UINT32 src, UINT32 c)
{
	UINT64 res = (UINT64)dst + (UINT64)src + (UINT64) c;
	SetCF32(res);
	SetOF_Add32(res,src,dst);
	SetAF(res,src,dst);
	SetSZPF32(res);
	return (UINT32)res;
}

INLINE UINT8 I386_OPS_BASE::INC8(UINT8 dst)
{
	UINT16 res = (UINT16)dst + 1;
	SetOF_Add8(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF8(res);
	return (UINT8)res;
}
INLINE UINT16 I386_OPS_BASE::INC16(UINT16 dst)
{
	UINT32 res = (UINT32)dst + 1;
	SetOF_Add16(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF16(res);
	return (UINT16)res;
}
INLINE UINT32 I386_OPS_BASE::INC32(UINT32 dst)
{
	UINT64 res = (UINT64)dst + 1;
	SetOF_Add32(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF32(res);
	return (UINT32)res;
}

INLINE UINT8 I386_OPS_BASE::DEC8(UINT8 dst)
{
	UINT16 res = (UINT16)dst - 1;
	SetOF_Sub8(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF8(res);
	return (UINT8)res;
}
INLINE UINT16 I386_OPS_BASE::DEC16(UINT16 dst)
{
	UINT32 res = (UINT32)dst - 1;
	SetOF_Sub16(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF16(res);
	return (UINT16)res;
}
INLINE UINT32 I386_OPS_BASE::DEC32(UINT32 dst)
{
	UINT64 res = (UINT64)dst - 1;
	SetOF_Sub32(res,1,dst);
	SetAF(res,1,dst);
	SetSZPF32(res);
	return (UINT32)res;
}



INLINE void I386_OPS_BASE::PUSH16(UINT16 value)
{
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 2;
		ea = i386_translate( SS, new_esp, 1, 2);
		WRITE16( ea, value );
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 2) & 0xffff;
		ea = i386_translate( SS, new_esp, 1, 2);
		WRITE16( ea, value );
		REG16(SP) = new_esp;
	}
}
INLINE void I386_OPS_BASE::PUSH32(UINT32 value)
{
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 4;
		ea = i386_translate( SS, new_esp, 1, 4);
		WRITE32( ea, value );
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 4) & 0xffff;
		ea = i386_translate( SS, new_esp, 1, 4);
		WRITE32( ea, value );
		REG16(SP) = new_esp;
	}
}

INLINE void I386_OPS_BASE::PUSH32SEG(UINT32 value)
{
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) - 4;
		if( (cpustate->cpu_version & 0xf00) == 0x300 ) {
			ea = i386_translate(SS, new_esp, 1, 2);
			WRITE16(ea, value); // 486 also?
		} else {
			ea = i386_translate(SS, new_esp, 1, 4);
			WRITE32(ea, value ); // 486 also?
		}
		REG32(ESP) = new_esp;
	} else {
		new_esp = (REG16(SP) - 4) & 0xffff;
		if( (cpustate->cpu_version & 0xf00) == 0x300 ) {
			ea = i386_translate(SS, new_esp, 1, 2);
			WRITE16(ea, value);
		} else {
			ea = i386_translate(SS, new_esp, 1, 4);
			WRITE32(ea, value );
		}
		REG16(SP) = new_esp;
	}
}

INLINE void I386_OPS_BASE::PUSH8(UINT8 value)
{
	if( cpustate->operand_size ) {
		PUSH32((INT32)(INT8)value);
	} else {
		PUSH16((INT16)(INT8)value);
	}
}

INLINE UINT8 I386_OPS_BASE::POP8()
{
	UINT8 value;
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 1;
		ea = i386_translate( SS, new_esp - 1, 0, 1);
		value = READ8( ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 1;
		ea = i386_translate( SS, (new_esp - 1) & 0xffff, 0, 1);
		value = READ8( ea );
		REG16(SP) = new_esp;
	}
	return value;
}

INLINE UINT16 I386_OPS_BASE::POP16()
{
	UINT16 value;
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 2;
		ea = i386_translate( SS, new_esp - 2, 0, 2);
		value = READ16( ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 2;
		ea = i386_translate( SS, (new_esp - 2) & 0xffff, 0, 2);
		value = READ16( ea );
		REG16(SP) = new_esp;
	}
	return value;
}
INLINE UINT32 I386_OPS_BASE::POP32()
{
	UINT32 value;
	UINT32 ea, new_esp;
	if( STACK_32BIT ) {
		new_esp = REG32(ESP) + 4;
		ea = i386_translate( SS, new_esp - 4, 0, 4);
		value = READ32( ea );
		REG32(ESP) = new_esp;
	} else {
		new_esp = REG16(SP) + 4;
		ea = i386_translate( SS, (new_esp - 4) & 0xffff, 0, 4);
		value = READ32( ea );
		REG16(SP) = new_esp;
	}
	return value;
}

INLINE void I386_OPS_BASE::BUMP_SI(int adjustment)
{
	if ( cpustate->address_size )
		REG32(ESI) += ((cpustate->DF) ? -adjustment : +adjustment);
	else
		REG16(SI) += ((cpustate->DF) ? -adjustment : +adjustment);
}

INLINE void I386_OPS_BASE::BUMP_DI(int adjustment)
{
	if ( cpustate->address_size )
		REG32(EDI) += ((cpustate->DF) ? -adjustment : +adjustment);
	else
		REG16(DI) += ((cpustate->DF) ? -adjustment : +adjustment);
}



/***********************************************************************************
    I/O ACCESS
***********************************************************************************/

INLINE void I386_OPS_BASE::check_ioperm( offs_t port, UINT8 mask)
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
	IOPB = READ16PL0( address+0x66);
	if((IOPB+(port/8)) > cpustate->task.limit)
		FAULT_THROW(FAULT_GP,0);

	map = READ8PL0( address+IOPB+(port/8));
	map >>= (port%8);
	if(map & mask)
		FAULT_THROW(FAULT_GP,0);
}

INLINE UINT8 I386_OPS_BASE::READPORT8( offs_t port)
{
	check_ioperm( port, 1);
	return cpustate->io->read_io8(port);
}

INLINE void I386_OPS_BASE::WRITEPORT8( offs_t port, UINT8 value)
{
	check_ioperm( port, 1);
	cpustate->io->write_io8(port, value);
}

INLINE UINT16 I386_OPS_BASE::READPORT16( offs_t port)
{
	if (port & 1)
	{
		UINT16 value = READPORT8( port);
		value |= (READPORT8( port + 1) << 8);
		return value;
	}
	else
	{
		check_ioperm( port, 3);
		return cpustate->io->read_io16(port);
	}
}

INLINE void I386_OPS_BASE::WRITEPORT16( offs_t port, UINT16 value)
{
	if (port & 1)
	{
		WRITEPORT8( port, value & 0xff);
		WRITEPORT8( port + 1, (value >> 8) & 0xff);
	}
	else
	{
		check_ioperm( port, 3);
		cpustate->io->write_io16(port, value);
	}
}

INLINE UINT32 I386_OPS_BASE::READPORT32( offs_t port)
{
	if (port & 3)
	{
		UINT32 value = READPORT8( port);
		value |= (READPORT8( port + 1) << 8);
		value |= (READPORT8( port + 2) << 16);
		value |= (READPORT8( port + 3) << 24);
		return value;
	}
	else
	{
		check_ioperm( port, 0xf);
		return cpustate->io->read_io32(port);
	}
}

INLINE void I386_OPS_BASE::WRITEPORT32( offs_t port, UINT32 value)
{
	if (port & 3)
	{
		WRITEPORT8( port, value & 0xff);
		WRITEPORT8( port + 1, (value >> 8) & 0xff);
		WRITEPORT8( port + 2, (value >> 16) & 0xff);
		WRITEPORT8( port + 3, (value >> 24) & 0xff);
	}
	else
	{
		check_ioperm( port, 0xf);
		cpustate->io->write_io32(port, value);
	}
}


INLINE UINT64 I386_OPS_BASE::MSR_READ(UINT32 offset,UINT8 *valid_msr)
{
	UINT64 res;
	UINT8 cpu_type = (cpustate->cpu_version >> 8) & 0x0f;

	*valid_msr = 0;

	switch(cpu_type)
	{
	case 5:  // Pentium
		res = pentium_msr_read(offset,valid_msr);
		break;
	case 6:  // Pentium Pro, Pentium II, Pentium III
		res = p6_msr_read(offset,valid_msr);
		break;
	case 15:  // Pentium 4+
		res = piv_msr_read(offset,valid_msr);
		break;
	default:
		res = 0;
		break;
	}

	return res;
}

INLINE void I386_OPS_BASE::MSR_WRITE(UINT32 offset, UINT64 data, UINT8 *valid_msr)
{
	*valid_msr = 0;
	UINT8 cpu_type = (cpustate->cpu_version >> 8) & 0x0f;

	switch(cpu_type)
	{
	case 5:  // Pentium
		pentium_msr_write(offset,data,valid_msr);
		break;
	case 6:  // Pentium Pro, Pentium II, Pentium III
		p6_msr_write(offset,data,valid_msr);
		break;
	case 15:  // Pentium 4+
		piv_msr_write(offset,data,valid_msr);
		break;
	}
}

INLINE INT8 I386_OPS_BASE::SaturatedSignedWordToSignedByte(INT16 word)
{
	if (word > 127)
		return 127;
	if (word < -128)
		return -128;
	return (INT8)word;
}

INLINE UINT8 I386_OPS_BASE::SaturatedSignedWordToUnsignedByte(INT16 word)
{
	if (word > 255)
		return 255;
	if (word < 0)
		return 0;
	return (UINT8)word;
}

INLINE INT16 I386_OPS_BASE::SaturatedSignedDwordToSignedWord(INT32 dword)
{
	if (dword > 32767)
		return 32767;
	if (dword < -32768)
		return -32768;
	return (INT16)dword;
}

INLINE UINT16 I386_OPS_BASE::SaturatedSignedDwordToUnsignedWord(INT32 dword)
{
	if (dword > 65535)
		return 65535;
	if (dword < 0)
		return 0;
	return (UINT16)dword;
}

INLINE void I386_OPS_BASE::MMXPROLOG()
{
	//cpustate->x87_sw &= ~(X87_SW_TOP_MASK << X87_SW_TOP_SHIFT); // top = 0
	cpustate->x87_tw = 0; // tag word = 0
}

INLINE void I386_OPS_BASE::READMMX(UINT32 ea,MMX_REG &r)
{
	r.q=READ64( ea);
}

INLINE void I386_OPS_BASE::WRITEMMX(UINT32 ea,MMX_REG &r)
{
	WRITE64( ea, r.q);
}

INLINE void I386_OPS_BASE::READXMM(UINT32 ea,XMM_REG &r)
{
	r.q[0]=READ64( ea);
	r.q[1]=READ64( ea+8);
}

INLINE void I386_OPS_BASE::WRITEXMM(UINT32 ea,XMM_REG &r)
{
	WRITE64( ea, r.q[0]);
	WRITE64( ea+8, r.q[1]);
}

INLINE void I386_OPS_BASE::READXMM_LO64(UINT32 ea,XMM_REG &r)
{
	r.q[0]=READ64( ea);
}

INLINE void I386_OPS_BASE::WRITEXMM_LO64(UINT32 ea,XMM_REG &r)
{
	WRITE64( ea, r.q[0]);
}

INLINE void I386_OPS_BASE::READXMM_HI64(UINT32 ea,XMM_REG &r)
{
	r.q[1]=READ64( ea);
}

INLINE void I386_OPS_BASE::WRITEXMM_HI64(UINT32 ea,XMM_REG &r)
{
	WRITE64( ea, r.q[1]);
}

INLINE flag I386_OPS_BASE::floatx80_is_quiet_nan(floatx80 a)
{
	bits64 aLow;

	aLow = a.low & ~LIT64(0x4000000000000000);
	return
		((a.high & 0x7FFF) == 0x7FFF)
		&& (bits64)(aLow << 1)
		&& (a.low != aLow);
}

INLINE int I386_OPS_BASE::floatx80_is_zero(floatx80 fx)
{
	return (((fx.high & 0x7fff) == 0) && ((fx.low << 1) == 0));
}

INLINE int I386_OPS_BASE::floatx80_is_inf(floatx80 fx)
{
	return (((fx.high & 0x7fff) == 0x7fff) && ((fx.low << 1) == 0));
}

INLINE int I386_OPS_BASE::floatx80_is_denormal(floatx80 fx)
{
	return (((fx.high & 0x7fff) == 0) &&
			((fx.low & U64(0x8000000000000000)) == 0) &&
			((fx.low << 1) != 0));
}

INLINE floatx80 I386_OPS_BASE::floatx80_abs(floatx80 fx)
{
	fx.high &= 0x7fff;
	return fx;
}

inline UINT64 I386_OPS_BASE::__SWAP64(UINT64 in)
{						
	typedef union {										
		struct {									
			uint8_t h7, h6, h5, h4, h3, h2, h, l;	
		} b;   
		UINT64 ld;
	} d1_t;
	
	d1_t id, od;
	id.ld = in;
	od.b.h7 = id.b.l;
	od.b.h6 = id.b.h;
	od.b.h5 = id.b.h2;
	od.b.h4 = id.b.h3;
	od.b.h3 = id.b.h4;
	od.b.h2 = id.b.h5;
	od.b.h  = id.b.h6;
	od.b.l  = id.b.h7;
	
	return od.ld;
}

INLINE double I386_OPS_BASE::fx80_to_double(floatx80 fx)
{
	union {
		UINT64 ld;
		double fd; // WIP: If sizeof(double) != sizeof(UINT64).(or IEEE 754 format has changed).
	} d;
	UINT64 nd;
	nd = floatx80_to_float64(fx);
#if __FLOAT_WORD_ORDER != __BYTE_ORDER
	nd = __SWAP64(nd);
#endif
	d.ld = nd;
	return d.fd;
}

INLINE floatx80 I386_OPS_BASE::double_to_fx80(double in)
{
	union {
		UINT64 ld;
		double fd; // WIP: If sizeof(double) != sizeof(UINT64).(or IEEE 754 format has changed).
	} d;
	UINT64 nd;
	d.fd = in;
	nd = d.ld;
#if __FLOAT_WORD_ORDER != __BYTE_ORDER
	nd = __SWAP64(nd);
#endif
	return float64_to_floatx80(nd);
}

INLINE floatx80 I386_OPS_BASE::READ80( UINT32 ea)
{
	floatx80 t;

	t.low = READ64( ea);
	t.high = READ16( ea + 8);

	return t;
}

INLINE void I386_OPS_BASE::WRITE80( UINT32 ea, floatx80 t)
{
	WRITE64( ea, t.low);
	WRITE16( ea + 8, t.high);
}

/*************************************
 *
 * x87 stack handling
 *
 *************************************/

INLINE void I386_OPS_BASE::x87_set_stack_top(int top)
{
	cpustate->x87_sw &= ~(X87_SW_TOP_MASK << X87_SW_TOP_SHIFT);
	cpustate->x87_sw |= (top << X87_SW_TOP_SHIFT);
}

INLINE void I386_OPS_BASE::x87_set_tag(int reg, int tag)
{
	int shift = X87_TW_FIELD_SHIFT(reg);

	cpustate->x87_tw &= ~(X87_TW_MASK << shift);
	cpustate->x87_tw |= (tag << shift);
}

INLINE void I386_OPS_BASE::x87_set_stack_underflow()
{
	cpustate->x87_sw &= ~X87_SW_C1;
	cpustate->x87_sw |= X87_SW_IE | X87_SW_SF;
}

INLINE void I386_OPS_BASE::x87_set_stack_overflow()
{
	cpustate->x87_sw |= X87_SW_C1 | X87_SW_IE | X87_SW_SF;
}

INLINE void I386_OPS_BASE::x87_write_cw( UINT16 cw)
{
	cpustate->x87_cw = cw;

	/* Update the SoftFloat rounding mode */
	float_rounding_mode = x87_to_sf_rc[(cpustate->x87_cw >> X87_CW_RC_SHIFT) & X87_CW_RC_MASK];
}

#endif
