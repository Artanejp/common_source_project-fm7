
#ifndef __LIB_I386_OPDEF_H__
#define __LIB_I386_OPDEF_H__

#include "./i386priv.h"
#include "./i386ops.h"

extern "C" {
#include "../libcpu_softfloat/fpu_constant.h"
#include "../libcpu_softfloat/mamesf.h"
#include "../libcpu_softfloat/milieu.h"
#include "../libcpu_softfloat/softfloat.h"
};

#ifndef INLINE
#define INLINE inline
#endif

#define U64(v) UINT64(v)

#define fatalerror(...) exit(1)
#define logerror(...)
#define popmessage(...)

/*****************************************************************************/
/* src/emu/devcpu.h */

// CPU interface functions
#define CPU_INIT_NAME(name)			I386_OPS_BASE::cpu_init_##name
#define CPU_INIT(name)				void* CPU_INIT_NAME(name)()
#define CPU_INIT_CALL_NAME(name)	cpu_init_##name
#define CPU_INIT_CALL(name)			CPU_INIT_CALL_NAME(name)()

#define CPU_RESET_NAME(name)		I386_OPS_BASE::cpu_reset_##name
#define CPU_RESET(name)				void CPU_RESET_NAME(name)(i386_state *cpustate)
#define CPU_RESET_CALL_NAME(name)	cpu_reset_##name
#define CPU_RESET_CALL(name)		CPU_RESET_CALL_NAME(name)(cpustate)

#define CPU_EXECUTE_NAME(name)		I386_OPS_BASE::cpu_execute_##name
#define CPU_EXECUTE(name)			int CPU_EXECUTE_NAME(name)(i386_state *cpustate, int cycles)
#define CPU_EXECUTE_CALL_NAME(name)	cpu_execute_##name
#define CPU_EXECUTE_CALL(name)		CPU_EXECUTE_CALL_NAME(name)(cpustate, cycles)

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



class DEBUG;
class I386_OPS_BASE {
protected:
	i386_state *cpustate;
// Parameters.
	
	int i386_parity_table[256];
	MODRM_TABLE i386_MODRM_table[256];
	
	const X86_OPCODE x86_opcode_table[];
	const X86_CYCLE_TABLE x86_cycle_table[];
	
	UINT8 cycle_table_rm[X86_NUM_CPUS][CYCLES_NUM_OPCODES];
	UINT8 cycle_table_pm[X86_NUM_CPUS][CYCLES_NUM_OPCODES];
	const floatx80 ffx80_zero =   { 0x0000, U64(0x0000000000000000) };
	const floatx80 ffx80_one =    { 0x3fff, U64(0x8000000000000000) };
	const floatx80 ffx80_ninf =   { 0xffff, U64(0x8000000000000000) };
	const floatx80 ffx80_inan =   { 0xffff, U64(0xc000000000000000) };
	const int fx87_to_sf_rc[4];

	UINT32 i386_escape_ea;   // hack around GCC 4.6 error because we need the side effects of GetEA()
public:
	I386_OPS_BASE(int cputypes = I386_OPS_CPUTYPE_I386)
	{
		cpustate = NULL;
		_cputype = cputypes;
	}
	~I386_OPS_BASE() {}
	void I386OP(decode_opcode)();
	
	int i386_translate_address(int intention, offs_t *address, vtlb_entry *entry);
	virtual int cpu_translate_i386(void *cpudevice, address_spacenum space, int intention, offs_t *address);
	virtual int cpu_execute_i386(int cycles);
	void i386_set_irq_line(int irqline, int state);
	void i386_set_a20_line(int state);
	int i386_limit_check( int seg, UINT32 offset);

	
protected:
	// Utilities
	void build_cycle_table();
	i386_state *i386_common_init(int tlbsize);
	void build_opcode_table( UINT32 features);
	void zero_state();
	void pentium_smi();
	
	UINT32 i386_load_protected_mode_segment( I386_SREG *seg, UINT64 *desc );
	static void i386_load_call_gate(I386_CALL_GATE *gate);
	void i386_set_descriptor_accessed( UINT16 selector);
	void i386_load_segment_descriptor( int segment );
	UINT32 i386_get_stack_segment(UINT8 privilege);
	UINT32 i386_get_stack_ptr(UINT8 privilege);
	
	UINT32 get_flags();
	void set_flags( UINT32 f );

	void sib_byte(UINT8 mod, UINT32* out_ea, UINT8* out_segment);
	void modrm_to_EA(UINT8 mod_rm, UINT32* out_ea, UINT8* out_segment);
	
	UINT32 GetNonTranslatedEA(UINT8 modrm,UINT8 *seg);
	UINT32 GetEA(UINT8 modrm, int rwn);

	// 
	void i386_check_sreg_validity(int reg);
	void i386_sreg_load( UINT16 selector, UINT8 reg, bool *fault);
	void i386_trap(int irq, int irq_gate, int trap_level);
	void i386_trap_with_error(int irq, int irq_gate, int trap_level, UINT32 error);
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
protected:
	// Init per vm..
	void cpu_init_i386(void);
	void cpu_init_i486(void);
	void cpu_init_pentium(void);
	void cpu_init_mediagx(void);
	void cpu_init_pentium_pro(void);
	void cpu_init_pentium_mmx(void);
	void cpu_init_pentium2(void);
	void cpu_init_pentium3(void);
	void cpu_init_pentium4(void);
	// Reset pewr type.
	void cpu_reset_i386(void);
	void cpu_reset_i486(void);
	void cpu_reset_pentium(void);
	void cpu_reset_mediagx(void);
	void cpu_reset_pentium_pro(void);
	void cpu_reset_pentium_mmx(void);
	void cpu_reset_pentium2(void);
	void cpu_reset_pentium3(void);
	void cpu_reset_pentium4(void);

protected:
	// INSNs.
	// i386/op16
	void I386OP(adc_rm16_r16)();      // Opcode 0x11
	void I386OP(adc_r16_rm16)();      // Opcode 0x13
	void I386OP(adc_ax_i16)();        // Opcode 0x15
	void I386OP(add_rm16_r16)();      // Opcode 0x01
	void I386OP(add_r16_rm16)();      // Opcode 0x03
	void I386OP(add_ax_i16)();        // Opcode 0x05
	void I386OP(and_rm16_r16)();      // Opcode 0x21
	void I386OP(and_r16_rm16)();      // Opcode 0x23
	void I386OP(and_ax_i16)();        // Opcode 0x25
	void I386OP(bsf_r16_rm16)();      // Opcode 0x0f bc
	void I386OP(bsr_r16_rm16)();      // Opcode 0x0f bd
	void I386OP(bt_rm16_r16)();       // Opcode 0x0f a3
	void I386OP(btc_rm16_r16)();      // Opcode 0x0f bb
	void I386OP(btr_rm16_r16)();      // Opcode 0x0f b3
	void I386OP(bts_rm16_r16)();      // Opcode 0x0f ab
	void I386OP(call_abs16)();        // Opcode 0x9a
	void I386OP(call_rel16)();        // Opcode 0xe8
	void I386OP(cbw)();               // Opcode 0x98
	void I386OP(cmp_rm16_r16)();      // Opcode 0x39
	void I386OP(cmp_r16_rm16)();      // Opcode 0x3b
	void I386OP(cmp_ax_i16)();        // Opcode 0x3d
	void I386OP(cmpsw)();             // Opcode 0xa7
	void I386OP(cwd)();               // Opcode 0x99
	void I386OP(dec_ax)();            // Opcode 0x48
	void I386OP(dec_cx)();            // Opcode 0x49
	void I386OP(dec_dx)();            // Opcode 0x4a
	void I386OP(dec_bx)();            // Opcode 0x4b
	void I386OP(dec_sp)();            // Opcode 0x4c
	void I386OP(dec_bp)();            // Opcode 0x4d
	void I386OP(dec_si)();            // Opcode 0x4e
	void I386OP(dec_di)();            // Opcode 0x4f
	void I386OP(imul_r16_rm16)();     // Opcode 0x0f af
	void I386OP(imul_r16_rm16_i16)(); // Opcode 0x69
	void I386OP(imul_r16_rm16_i8)();  // Opcode 0x6b
	void I386OP(in_ax_i8)();          // Opcode 0xe5
	void I386OP(in_ax_dx)();          // Opcode 0xed
	void I386OP(inc_ax)();            // Opcode 0x40
	void I386OP(inc_cx)();            // Opcode 0x41
	void I386OP(inc_dx)();            // Opcode 0x42
	void I386OP(inc_bx)();            // Opcode 0x43
	void I386OP(inc_sp)();            // Opcode 0x44
	void I386OP(inc_bp)();            // Opcode 0x45
	void I386OP(inc_si)();            // Opcode 0x46
	void I386OP(inc_di)();            // Opcode 0x47
	void I386OP(iret16)();            // Opcode 0xcf
	void I386OP(ja_rel16)();          // Opcode 0x0f 87
	void I386OP(jbe_rel16)();         // Opcode 0x0f 86
	void I386OP(jc_rel16)();          // Opcode 0x0f 82
	void I386OP(jg_rel16)();          // Opcode 0x0f 8f
	void I386OP(jge_rel16)();         // Opcode 0x0f 8d
	void I386OP(jl_rel16)();          // Opcode 0x0f 8c
	void I386OP(jle_rel16)();         // Opcode 0x0f 8e
	void I386OP(jnc_rel16)();         // Opcode 0x0f 83
	void I386OP(jno_rel16)();         // Opcode 0x0f 81
	void I386OP(jnp_rel16)();         // Opcode 0x0f 8b
	void I386OP(jns_rel16)();         // Opcode 0x0f 89
	void I386OP(jnz_rel16)();         // Opcode 0x0f 85
	void I386OP(jo_rel16)();          // Opcode 0x0f 80
	void I386OP(jp_rel16)();          // Opcode 0x0f 8a
	void I386OP(js_rel16)();          // Opcode 0x0f 88
	void I386OP(jz_rel16)();          // Opcode 0x0f 84
	void I386OP(jcxz16)();            // Opcode 0xe3
	void I386OP(jmp_rel16)();         // Opcode 0xe9
	void I386OP(jmp_abs16)();         // Opcode 0xea
	void I386OP(lea16)();             // Opcode 0x8d
	void I386OP(enter16)();           // Opcode 0xc8
	void I386OP(leave16)();           // Opcode 0xc9
	void I386OP(lodsw)();             // Opcode 0xad
	void I386OP(loop16)();            // Opcode 0xe2
	void I386OP(loopne16)();          // Opcode 0xe0
	void I386OP(loopz16)();           // Opcode 0xe1
	void I386OP(mov_rm16_r16)();      // Opcode 0x89
	void I386OP(mov_r16_rm16)();      // Opcode 0x8b
	void I386OP(mov_rm16_i16)();      // Opcode 0xc7
	void I386OP(mov_ax_m16)();        // Opcode 0xa1
	void I386OP(mov_m16_ax)();        // Opcode 0xa3
	void I386OP(mov_ax_i16)();        // Opcode 0xb8
	void I386OP(mov_cx_i16)();        // Opcode 0xb9
	void I386OP(mov_dx_i16)();        // Opcode 0xba
	void I386OP(mov_bx_i16)();        // Opcode 0xbb
	void I386OP(mov_sp_i16)();        // Opcode 0xbc
	void I386OP(mov_bp_i16)();        // Opcode 0xbd
	void I386OP(mov_si_i16)();        // Opcode 0xbe
	void I386OP(mov_di_i16)();        // Opcode 0xbf
	void I386OP(movsw)();             // Opcode 0xa5
	void I386OP(movsx_r16_rm8)();     // Opcode 0x0f be
	void I386OP(movzx_r16_rm8)();     // Opcode 0x0f b6
	void I386OP(or_rm16_r16)();       // Opcode 0x09
	void I386OP(or_r16_rm16)();       // Opcode 0x0b
	void I386OP(or_ax_i16)();         // Opcode 0x0d
	void I386OP(out_ax_i8)();         // Opcode 0xe7
	void I386OP(out_ax_dx)();         // Opcode 0xef
	void I386OP(pop_ax)();            // Opcode 0x58
	void I386OP(pop_cx)();            // Opcode 0x59
	void I386OP(pop_dx)();            // Opcode 0x5a
	void I386OP(pop_bx)();            // Opcode 0x5b
	void I386OP(pop_sp)();            // Opcode 0x5c
	void I386OP(pop_bp)();            // Opcode 0x5d
	void I386OP(pop_si)();            // Opcode 0x5e
	void I386OP(pop_di)();            // Opcode 0x5f
	void I386OP(pop_ds16)();          // Opcode 0x1f
	void I386OP(pop_es16)();          // Opcode 0x07
	void I386OP(pop_fs16)();          // Opcode 0x0f a1
	void I386OP(pop_gs16)();          // Opcode 0x0f a9
	void I386OP(pop_ss16)();          // Opcode 0x17
	void I386OP(pop_rm16)();          // Opcode 0x8f
	void I386OP(popa)();              // Opcode 0x61
	void I386OP(popf)();              // Opcode 0x9d
	void I386OP(push_ax)();           // Opcode 0x50
	void I386OP(push_cx)();           // Opcode 0x51
	void I386OP(push_dx)();           // Opcode 0x52
	void I386OP(push_bx)();           // Opcode 0x53
	void I386OP(push_sp)();           // Opcode 0x54
	void I386OP(push_bp)();           // Opcode 0x55
	void I386OP(push_si)();           // Opcode 0x56
	void I386OP(push_di)();           // Opcode 0x57
	void I386OP(push_cs16)();         // Opcode 0x0e
	void I386OP(push_ds16)();         // Opcode 0x1e
	void I386OP(push_es16)();         // Opcode 0x06
	void I386OP(push_fs16)();         // Opcode 0x0f a0
	void I386OP(push_gs16)();         // Opcode 0x0f a8
	void I386OP(push_ss16)();         // Opcode 0x16
	void I386OP(push_i16)();          // Opcode 0x68
	void I386OP(pusha)();             // Opcode 0x60
	void I386OP(pushf)();             // Opcode 0x9c
	void I386OP(ret_near16_i16)();    // Opcode 0xc2
	void I386OP(ret_near16)();        // Opcode 0xc3
	void I386OP(sbb_rm16_r16)();      // Opcode 0x19
	void I386OP(sbb_r16_rm16)();      // Opcode 0x1b
	void I386OP(sbb_ax_i16)();        // Opcode 0x1d
	void I386OP(scasw)();             // Opcode 0xaf
	void I386OP(shld16_i8)();         // Opcode 0x0f a4
	void I386OP(shld16_cl)();         // Opcode 0x0f a5
	void I386OP(shrd16_i8)();         // Opcode 0x0f ac
	void I386OP(shrd16_cl)();         // Opcode 0x0f ad
	void I386OP(stosw)();             // Opcode 0xab
	void I386OP(sub_rm16_r16)();      // Opcode 0x29
	void I386OP(sub_r16_rm16)();      // Opcode 0x2b
	void I386OP(sub_ax_i16)();        // Opcode 0x2d
	void I386OP(test_ax_i16)();       // Opcode 0xa9
	void I386OP(test_rm16_r16)();     // Opcode 0x85
	void I386OP(xchg_ax_cx)();        // Opcode 0x91
	void I386OP(xchg_ax_dx)();        // Opcode 0x92
	void I386OP(xchg_ax_bx)();        // Opcode 0x93
	void I386OP(xchg_ax_sp)();        // Opcode 0x94
	void I386OP(xchg_ax_bp)();        // Opcode 0x95
	void I386OP(xchg_ax_si)();        // Opcode 0x96
	void I386OP(xchg_ax_di)();        // Opcode 0x97
	void I386OP(xchg_r16_rm16)();     // Opcode 0x87
	void I386OP(xor_rm16_r16)();      // Opcode 0x31
	void I386OP(xor_r16_rm16)();      // Opcode 0x33
	void I386OP(xor_ax_i16)();        // Opcode 0x35
	void I386OP(group81_16)();        // Opcode 0x81
	void I386OP(group83_16)();        // Opcode 0x83
	void I386OP(groupC1_16)();        // Opcode 0xc1
	void I386OP(groupD1_16)();        // Opcode 0xd1
	void I386OP(groupD3_16)();        // Opcode 0xd3
	void I386OP(groupF7_16)();        // Opcode 0xf7
	void I386OP(groupFF_16)();        // Opcode 0xff
	void I386OP(group0F00_16)();          // Opcode 0x0f 00
	void I386OP(group0F01_16)();      // Opcode 0x0f 01
	void I386OP(group0FBA_16)();      // Opcode 0x0f ba
	void I386OP(lar_r16_rm16)();  // Opcode 0x0f 0x02
	void I386OP(lsl_r16_rm16)();  // Opcode 0x0f 0x03
	void I386OP(bound_r16_m16_m16)(); // Opcode 0x62
	void I386OP(retf16)();            // Opcode 0xcb
	void I386OP(retf_i16)();          // Opcode 0xca
	void I386OP(lds16)();             // Opcode 0xc5
	void I386OP(lss16)();             // Opcode 0x0f 0xb2
	void I386OP(les16)();             // Opcode 0xc4
	void I386OP(lfs16)();             // Opcode 0x0f 0xb4
	void I386OP(lgs16)();             // Opcode 0x0f 0xb5
	UINT16 I386OP(shift_rotate16)( UINT8 modrm, UINT32 value, UINT8 shift);
	UINT8 I386OP(shift_rotate8)( UINT8 modrm, UINT32 value, UINT8 shift);
	//i386/op32
	void I386OP(adc_rm32_r32)();      // Opcode 0x11
	void I386OP(adc_r32_rm32)();      // Opcode 0x13
	void I386OP(adc_eax_i32)();       // Opcode 0x15
	void I386OP(add_rm32_r32)();      // Opcode 0x01
	void I386OP(add_r32_rm32)();      // Opcode 0x03
	void I386OP(add_eax_i32)();       // Opcode 0x05
	void I386OP(and_rm32_r32)();      // Opcode 0x21
	void I386OP(and_r32_rm32)();      // Opcode 0x23
	void I386OP(and_eax_i32)();       // Opcode 0x25
	void I386OP(bsf_r32_rm32)();      // Opcode 0x0f bc
	void I386OP(bsr_r32_rm32)();      // Opcode 0x0f bd
	void I386OP(bt_rm32_r32)();       // Opcode 0x0f a3
	void I386OP(btc_rm32_r32)();      // Opcode 0x0f bb
	void I386OP(btr_rm32_r32)();      // Opcode 0x0f b3
	void I386OP(bts_rm32_r32)();      // Opcode 0x0f ab
	void I386OP(call_abs32)();        // Opcode 0x9a
	void I386OP(call_rel32)();        // Opcode 0xe8
	void I386OP(cdq)();               // Opcode 0x99
	void I386OP(cmp_rm32_r32)();      // Opcode 0x39
	void I386OP(cmp_r32_rm32)();      // Opcode 0x3b
	void I386OP(cmp_eax_i32)();       // Opcode 0x3d
	void I386OP(cmpsd)();             // Opcode 0xa7
	void I386OP(cwde)();              // Opcode 0x98
	void I386OP(dec_eax)();           // Opcode 0x48
	void I386OP(dec_ecx)();           // Opcode 0x49
	void I386OP(dec_edx)();           // Opcode 0x4a
	void I386OP(dec_ebx)();           // Opcode 0x4b
	void I386OP(dec_esp)();           // Opcode 0x4c
	void I386OP(dec_ebp)();           // Opcode 0x4d
	void I386OP(dec_esi)();           // Opcode 0x4e
	void I386OP(dec_edi)();           // Opcode 0x4f
	void I386OP(imul_r32_rm32)();     // Opcode 0x0f af
	void I386OP(imul_r32_rm32_i32)(); // Opcode 0x69
	void I386OP(imul_r32_rm32_i8)();  // Opcode 0x6b
	void I386OP(in_eax_i8)();         // Opcode 0xe5
	void I386OP(in_eax_dx)();         // Opcode 0xed
	void I386OP(inc_eax)();           // Opcode 0x40
	void I386OP(inc_ecx)();           // Opcode 0x41
	void I386OP(inc_edx)();           // Opcode 0x42
	void I386OP(inc_ebx)();           // Opcode 0x43
	void I386OP(inc_esp)();           // Opcode 0x44
	void I386OP(inc_ebp)();           // Opcode 0x45
	void I386OP(inc_esi)();           // Opcode 0x46
	void I386OP(inc_edi)();           // Opcode 0x47
	void I386OP(iret32)();            // Opcode 0xcf
	void I386OP(ja_rel32)();          // Opcode 0x0f 87
	void I386OP(jbe_rel32)();         // Opcode 0x0f 86
	void I386OP(jc_rel32)();          // Opcode 0x0f 82
	void I386OP(jg_rel32)();          // Opcode 0x0f 8f
	void I386OP(jge_rel32)();         // Opcode 0x0f 8d
	void I386OP(jl_rel32)();          // Opcode 0x0f 8c
	void I386OP(jle_rel32)();         // Opcode 0x0f 8e
	void I386OP(jnc_rel32)();         // Opcode 0x0f 83
	void I386OP(jno_rel32)();         // Opcode 0x0f 81
	void I386OP(jnp_rel32)();         // Opcode 0x0f 8b
	void I386OP(jns_rel32)();         // Opcode 0x0f 89
	void I386OP(jnz_rel32)();         // Opcode 0x0f 85
	void I386OP(jo_rel32)();          // Opcode 0x0f 80
	void I386OP(jp_rel32)();          // Opcode 0x0f 8a
	void I386OP(js_rel32)();          // Opcode 0x0f 88
	void I386OP(jz_rel32)();          // Opcode 0x0f 84
	void I386OP(jcxz32)();            // Opcode 0xe3
	void I386OP(jmp_rel32)();         // Opcode 0xe9
	void I386OP(jmp_abs32)();         // Opcode 0xea
	void I386OP(lea32)();             // Opcode 0x8d
	void I386OP(enter32)();           // Opcode 0xc8
	void I386OP(leave32)();           // Opcode 0xc9
	void I386OP(lodsd)();             // Opcode 0xad
	void I386OP(loop32)();            // Opcode 0xe2
	void I386OP(loopne32)();          // Opcode 0xe0
	void I386OP(loopz32)();           // Opcode 0xe1
	void I386OP(mov_rm32_r32)();      // Opcode 0x89
	void I386OP(mov_r32_rm32)();      // Opcode 0x8b
	void I386OP(mov_rm32_i32)();      // Opcode 0xc7
	void I386OP(mov_eax_m32)();       // Opcode 0xa1
	void I386OP(mov_m32_eax)();       // Opcode 0xa3
	void I386OP(mov_eax_i32)();       // Opcode 0xb8
	void I386OP(mov_ecx_i32)();       // Opcode 0xb9
	void I386OP(mov_edx_i32)();       // Opcode 0xba
	void I386OP(mov_ebx_i32)();       // Opcode 0xbb
	void I386OP(mov_esp_i32)();       // Opcode 0xbc
	void I386OP(mov_ebp_i32)();       // Opcode 0xbd
	void I386OP(mov_esi_i32)();       // Opcode 0xbe
	void I386OP(mov_edi_i32)();       // Opcode 0xbf
	void I386OP(movsd)();             // Opcode 0xa5
	void I386OP(movsx_r32_rm8)();     // Opcode 0x0f be
	void I386OP(movsx_r32_rm16)();    // Opcode 0x0f bf
	void I386OP(movzx_r32_rm8)();     // Opcode 0x0f b6
	void I386OP(movzx_r32_rm16)();    // Opcode 0x0f b7
	void I386OP(or_rm32_r32)();       // Opcode 0x09
	void I386OP(or_r32_rm32)();       // Opcode 0x0b
	void I386OP(or_eax_i32)();        // Opcode 0x0d
	void I386OP(out_eax_i8)();        // Opcode 0xe7
	void I386OP(out_eax_dx)();        // Opcode 0xef
	void I386OP(pop_eax)();           // Opcode 0x58
	void I386OP(pop_ecx)();           // Opcode 0x59
	void I386OP(pop_edx)();           // Opcode 0x5a
	void I386OP(pop_ebx)();           // Opcode 0x5b
	void I386OP(pop_esp)();           // Opcode 0x5c
	void I386OP(pop_ebp)();           // Opcode 0x5d
	void I386OP(pop_esi)();           // Opcode 0x5e
	void I386OP(pop_edi)();           // Opcode 0x5f
	void I386OP(pop_ds32)();          // Opcode 0x1f
	void I386OP(pop_es32)();          // Opcode 0x07
	void I386OP(pop_fs32)();          // Opcode 0x0f a1
	void I386OP(pop_gs32)();          // Opcode 0x0f a9
	void I386OP(pop_ss32)();          // Opcode 0x17
	void I386OP(pop_rm32)();          // Opcode 0x8f
	void I386OP(popad)();             // Opcode 0x61
	void I386OP(popfd)();             // Opcode 0x9d
	void I386OP(push_eax)();          // Opcode 0x50
	void I386OP(push_ecx)();          // Opcode 0x51
	void I386OP(push_edx)();          // Opcode 0x52
	void I386OP(push_ebx)();          // Opcode 0x53
	void I386OP(push_esp)();          // Opcode 0x54
	void I386OP(push_ebp)();          // Opcode 0x55
	void I386OP(push_esi)();          // Opcode 0x56
	void I386OP(push_edi)();          // Opcode 0x57
	void I386OP(push_cs32)();         // Opcode 0x0e
	void I386OP(push_ds32)();         // Opcode 0x1e
	void I386OP(push_es32)();         // Opcode 0x06
	void I386OP(push_fs32)();         // Opcode 0x0f a0
	void I386OP(push_gs32)();         // Opcode 0x0f a8
	void I386OP(push_ss32)();         // Opcode 0x16
	void I386OP(push_i32)();          // Opcode 0x68
	void I386OP(pushad)();            // Opcode 0x60
	void I386OP(pushfd)();            // Opcode 0x9c
	void I386OP(ret_near32_i16)();    // Opcode 0xc2
	void I386OP(ret_near32)();        // Opcode 0xc3
	void I386OP(sbb_rm32_r32)();      // Opcode 0x19
	void I386OP(sbb_r32_rm32)();      // Opcode 0x1b
	void I386OP(sbb_eax_i32)();       // Opcode 0x1d
	void I386OP(scasd)();             // Opcode 0xaf
	void I386OP(shld32_i8)();         // Opcode 0x0f a4
	void I386OP(shld32_cl)();         // Opcode 0x0f a5
	void I386OP(shrd32_i8)();         // Opcode 0x0f ac
	void I386OP(shrd32_cl)();         // Opcode 0x0f ad
	void I386OP(stosd)();             // Opcode 0xab
	void I386OP(sub_rm32_r32)();      // Opcode 0x29
	void I386OP(sub_r32_rm32)();      // Opcode 0x2b
	void I386OP(sub_eax_i32)();       // Opcode 0x2d
	void I386OP(test_eax_i32)();      // Opcode 0xa9
	void I386OP(test_rm32_r32)();     // Opcode 0x85
	void I386OP(xchg_eax_ecx)();      // Opcode 0x91
	void I386OP(xchg_eax_edx)();      // Opcode 0x92
	void I386OP(xchg_eax_ebx)();      // Opcode 0x93
	void I386OP(xchg_eax_esp)();      // Opcode 0x94
	void I386OP(xchg_eax_ebp)();      // Opcode 0x95
	void I386OP(xchg_eax_esi)();      // Opcode 0x96
	void I386OP(xchg_eax_edi)();      // Opcode 0x97
	void I386OP(xchg_r32_rm32)();     // Opcode 0x87
	void I386OP(xor_rm32_r32)();      // Opcode 0x31
	void I386OP(xor_r32_rm32)();      // Opcode 0x33
	void I386OP(xor_eax_i32)();       // Opcode 0x35
	void I386OP(group81_32)();        // Opcode 0x81
	void I386OP(group83_32)();        // Opcode 0x83
	void I386OP(groupC1_32)();        // Opcode 0xc1
	void I386OP(groupD1_32)();        // Opcode 0xd1
	void I386OP(groupD3_32)();        // Opcode 0xd3
	void I386OP(groupF7_32)();        // Opcode 0xf7
	void I386OP(groupFF_32)();        // Opcode 0xff
	void I386OP(group0F00_32)();          // Opcode 0x0f 00
	void I386OP(group0F01_32)();      // Opcode 0x0f 01
	void I386OP(group0FBA_32)();      // Opcode 0x0f ba
	void I386OP(lar_r32_rm32)();  // Opcode 0x0f 0x02
	void I386OP(lsl_r32_rm32)();  // Opcode 0x0f 0x03
	void I386OP(bound_r32_m32_m32)(); // Opcode 0x62
	void I386OP(retf32)();            // Opcode 0xcb
	void I386OP(retf_i32)();          // Opcode 0xca
	void I386OP(load_far_pointer32)( int s);
	void I386OP(lds32)();             // Opcode 0xc5
	void I386OP(lss32)();             // Opcode 0x0f 0xb2
	void I386OP(les32)();             // Opcode 0xc4
	void I386OP(lfs32)();             // Opcode 0x0f 0xb4
	void I386OP(lgs32)();             // Opcode 0x0f 0xb5
// i386 other OPS.
	void I386OP(adc_rm8_r8)();        // Opcode 0x10
	void I386OP(adc_r8_rm8)();        // Opcode 0x12
	void I386OP(adc_al_i8)();     // Opcode 0x14
	void I386OP(add_rm8_r8)();        // Opcode 0x00
	void I386OP(add_r8_rm8)();        // Opcode 0x02
	void I386OP(add_al_i8)();     // Opcode 0x04
	void I386OP(and_rm8_r8)();        // Opcode 0x20
	void I386OP(and_r8_rm8)();        // Opcode 0x22
	void I386OP(and_al_i8)();         // Opcode 0x24
	void I386OP(clc)();               // Opcode 0xf8
	void I386OP(cld)();               // Opcode 0xfc
	void I386OP(cli)();               // Opcode 0xfa
	void I386OP(cmc)();               // Opcode 0xf5
	void I386OP(cmp_rm8_r8)();        // Opcode 0x38
	void I386OP(cmp_r8_rm8)();        // Opcode 0x3a
	void I386OP(cmp_al_i8)();         // Opcode 0x3c
	void I386OP(cmpsb)();             // Opcode 0xa6
	void I386OP(in_al_i8)();          // Opcode 0xe4
	void I386OP(in_al_dx)();          // Opcode 0xec
	void I386OP(ja_rel8)();           // Opcode 0x77
	void I386OP(jbe_rel8)();          // Opcode 0x76
	void I386OP(jc_rel8)();           // Opcode 0x72
	void I386OP(jg_rel8)();           // Opcode 0x7f
	void I386OP(jge_rel8)();          // Opcode 0x7d
	void I386OP(jl_rel8)();           // Opcode 0x7c
	void I386OP(jle_rel8)();      // Opcode 0x7e
	void I386OP(jnc_rel8)();          // Opcode 0x73
	void I386OP(jno_rel8)();          // Opcode 0x71
	void I386OP(jnp_rel8)();          // Opcode 0x7b
	void I386OP(jns_rel8)();          // Opcode 0x79
	void I386OP(jnz_rel8)();          // Opcode 0x75
	void I386OP(jo_rel8)();           // Opcode 0x70
	void I386OP(jp_rel8)();           // Opcode 0x7a
	void I386OP(js_rel8)();           // Opcode 0x78
	void I386OP(jz_rel8)();           // Opcode 0x74
	void I386OP(jmp_rel8)();          // Opcode 0xeb
	void I386OP(lahf)();              // Opcode 0x9f
	void I386OP(lodsb)();             // Opcode 0xac
	void I386OP(mov_rm8_r8)();        // Opcode 0x88
	void I386OP(mov_r8_rm8)();        // Opcode 0x8a
	void I386OP(mov_rm8_i8)();        // Opcode 0xc6
	void I386OP(mov_r32_cr)();        // Opcode 0x0f 20
	void I386OP(mov_r32_dr)();        // Opcode 0x0f 21
	void I386OP(mov_cr_r32)();        // Opcode 0x0f 22
	void I386OP(mov_dr_r32)();        // Opcode 0x0f 23
	void I386OP(mov_al_m8)();         // Opcode 0xa0
	void I386OP(mov_m8_al)();         // Opcode 0xa2
	void I386OP(mov_rm16_sreg)();     // Opcode 0x8c
	void I386OP(mov_sreg_rm16)();     // Opcode 0x8e
	void I386OP(mov_al_i8)();         // Opcode 0xb0
	void I386OP(mov_cl_i8)();         // Opcode 0xb1
	void I386OP(mov_dl_i8)();         // Opcode 0xb2
	void I386OP(mov_bl_i8)();         // Opcode 0xb3
	void I386OP(mov_ah_i8)();         // Opcode 0xb4
	void I386OP(mov_ch_i8)();         // Opcode 0xb5
	void I386OP(mov_dh_i8)();         // Opcode 0xb6
	void I386OP(mov_bh_i8)();         // Opcode 0xb7
	void I386OP(movsb)();             // Opcode 0xa4
	void I386OP(or_rm8_r8)();         // Opcode 0x08
	void I386OP(or_r8_rm8)();         // Opcode 0x0a
	void I386OP(or_al_i8)();          // Opcode 0x0c
	void I386OP(out_al_i8)();         // Opcode 0xe6
	void I386OP(out_al_dx)();         // Opcode 0xee
	void I386OP(arpl)();           // Opcode 0x63
	void I386OP(push_i8)();           // Opcode 0x6a
	void I386OP(ins_generic)( int size);
	void I386OP(insb)();              // Opcode 0x6c
	void I386OP(insw)();              // Opcode 0x6d
	void I386OP(insd)();              // Opcode 0x6d
	void I386OP(outs_generic)( int size);
	void I386OP(outsb)();             // Opcode 0x6e
	void I386OP(outsw)();             // Opcode 0x6f
	void I386OP(outsd)();             // Opcode 0x6f
	void I386OP(repeat)( int invert_flag);
	void I386OP(rep)();               // Opcode 0xf3
	void I386OP(repne)();             // Opcode 0xf2
	void I386OP(sahf)();              // Opcode 0x9e
	void I386OP(sbb_rm8_r8)();        // Opcode 0x18
	void I386OP(sbb_r8_rm8)();        // Opcode 0x1a
	void I386OP(sbb_al_i8)();         // Opcode 0x1c
	void I386OP(scasb)();             // Opcode 0xae
	void I386OP(setalc)();            // Opcode 0xd6 (undocumented)
	void I386OP(seta_rm8)();          // Opcode 0x0f 97
	void I386OP(setbe_rm8)();         // Opcode 0x0f 96
	void I386OP(setc_rm8)();          // Opcode 0x0f 92
	void I386OP(setg_rm8)();          // Opcode 0x0f 9f
	void I386OP(setge_rm8)();         // Opcode 0x0f 9d
	void I386OP(setl_rm8)();          // Opcode 0x0f 9c
	void I386OP(setle_rm8)();         // Opcode 0x0f 9e
	void I386OP(setnc_rm8)();         // Opcode 0x0f 93
	void I386OP(setno_rm8)();         // Opcode 0x0f 91
	void I386OP(setnp_rm8)();         // Opcode 0x0f 9b
	void I386OP(setns_rm8)();         // Opcode 0x0f 99
	void I386OP(setnz_rm8)();         // Opcode 0x0f 95
	void I386OP(seto_rm8)();          // Opcode 0x0f 90
	void I386OP(setp_rm8)();          // Opcode 0x0f 9a
	void I386OP(sets_rm8)();          // Opcode 0x0f 98
	void I386OP(setz_rm8)();          // Opcode 0x0f 94
	void I386OP(stc)();               // Opcode 0xf9
	void I386OP(std)();               // Opcode 0xfd
	void I386OP(sti)();               // Opcode 0xfb
	void I386OP(stosb)();             // Opcode 0xaa
	void I386OP(sub_rm8_r8)();        // Opcode 0x28
	void I386OP(sub_r8_rm8)();        // Opcode 0x2a
	void I386OP(sub_al_i8)();         // Opcode 0x2c
	void I386OP(test_al_i8)();        // Opcode 0xa8
	void I386OP(test_rm8_r8)();       // Opcode 0x84
	void I386OP(xchg_r8_rm8)();       // Opcode 0x86
	void I386OP(xor_rm8_r8)();        // Opcode 0x30
	void I386OP(xor_r8_rm8)();        // Opcode 0x32
	void I386OP(xor_al_i8)();         // Opcode 0x34
	void I386OP(group80_8)();         // Opcode 0x80
	void I386OP(groupC0_8)();         // Opcode 0xc0
	void I386OP(groupD0_8)();         // Opcode 0xd0
	void I386OP(groupD2_8)();         // Opcode 0xd2
	void I386OP(groupF6_8)();         // Opcode 0xf6
	void I386OP(groupFE_8)();         // Opcode 0xfe
	void I386OP(segment_CS)();        // Opcode 0x2e
	void I386OP(segment_DS)();        // Opcode 0x3e
	void I386OP(segment_ES)();        // Opcode 0x26
	void I386OP(segment_FS)();        // Opcode 0x64
	void I386OP(segment_GS)();        // Opcode 0x65
	void I386OP(segment_SS)();        // Opcode 0x36
	void I386OP(operand_size)();      // Opcode prefix 0x66
	void I386OP(address_size)();      // Opcode 0x67
	void I386OP(nop)();               // Opcode 0x90
	void I386OP(int3)();              // Opcode 0xcc
	void I386OP(int)();               // Opcode 0xcd
	void I386OP(into)();              // Opcode 0xce
	void I386OP(escape)();            // Opcodes 0xd8 - 0xdf
	void I386OP(hlt)();               // Opcode 0xf4
	void I386OP(decimal_adjust)(int direction);
	void I386OP(daa)();               // Opcode 0x27
	void I386OP(das)();               // Opcode 0x2f
	void I386OP(aaa)();               // Opcode 0x37
	void I386OP(aas)();               // Opcode 0x3f
	void I386OP(aad)();               // Opcode 0xd5
	void I386OP(aam)();               // Opcode 0xd4
	void I386OP(clts)();              // Opcode 0x0f 0x06
	void I386OP(wait)();              // Opcode 0x9B
	void I386OP(lock)();              // Opcode 0xf0
	void I386OP(mov_r32_tr)();        // Opcode 0x0f 24
	void I386OP(mov_tr_r32)();        // Opcode 0x0f 26
	void I386OP(loadall)();       // Opcode 0x0f 0x07 (0x0f 0x05 on 80286), undocumented
	void I386OP(invalid)();
	void I386OP(xlat)();          // Opcode 0xd7
	bool I386OP(pop_seg16)( int segment);
	bool I386OP(load_far_pointer16)(int s);
	bool I386OP(pop_seg32)( int segment);
//i486
	void I486OP(cpuid)();             // Opcode 0x0F A2
	void I486OP(invd)();              // Opcode 0x0f 08
	void I486OP(wbinvd)();            // Opcode 0x0f 09
	void I486OP(cmpxchg_rm8_r8)();    // Opcode 0x0f b0
	void I486OP(cmpxchg_rm16_r16)();  // Opcode 0x0f b1
	void I486OP(cmpxchg_rm32_r32)();  // Opcode 0x0f b1
	void I486OP(xadd_rm8_r8)();   // Opcode 0x0f c0
	void I486OP(xadd_rm16_r16)(); // Opcode 0x0f c1
	void I486OP(xadd_rm32_r32)(); // Opcode 0x0f c1
	void I486OP(group0F01_16)();      // Opcode 0x0f 01
	void I486OP(group0F01_32)();      // Opcode 0x0f 01
	void I486OP(bswap_eax)();     // Opcode 0x0f 38
	void I486OP(bswap_ecx)();     // Opcode 0x0f 39
	void I486OP(bswap_edx)();     // Opcode 0x0f 3A
	void I486OP(bswap_ebx)();     // Opcode 0x0f 3B
	void I486OP(bswap_esp)();     // Opcode 0x0f 3C
	void I486OP(bswap_ebp)();     // Opcode 0x0f 3D
	void I486OP(bswap_esi)();     // Opcode 0x0f 3E
	void I486OP(bswap_edi)();     // Opcode 0x0f 3F
	void I486OP(mov_cr_r32)();        // Opcode 0x0f 22
//Pentium, MMX, SSE.
	void PENTIUMOP(rdmsr)();          // Opcode 0x0f 32
	void PENTIUMOP(wrmsr)();          // Opcode 0x0f 30
	void PENTIUMOP(rdtsc)();          // Opcode 0x0f 31
	void PENTIUMOP(ud2)();    // Opcode 0x0f 0b
	void PENTIUMOP(rsm)();
	void PENTIUMOP(prefetch_m8)();    // Opcode 0x0f 18
	void PENTIUMOP(cmovo_r16_rm16)();    // Opcode 0x0f 40
	void PENTIUMOP(cmovo_r32_rm32)();    // Opcode 0x0f 40
	void PENTIUMOP(cmovno_r16_rm16)();    // Opcode 0x0f 41
	void PENTIUMOP(cmovno_r32_rm32)();    // Opcode 0x0f 41
	void PENTIUMOP(cmovb_r16_rm16)();    // Opcode 0x0f 42
	void PENTIUMOP(cmovb_r32_rm32)();    // Opcode 0x0f 42
	void PENTIUMOP(cmovae_r16_rm16)();    // Opcode 0x0f 43
	void PENTIUMOP(cmovae_r32_rm32)();    // Opcode 0x0f 43
	void PENTIUMOP(cmove_r16_rm16)();    // Opcode 0x0f 44
	void PENTIUMOP(cmove_r32_rm32)();    // Opcode 0x0f 44
	void PENTIUMOP(cmovne_r16_rm16)();    // Opcode 0x0f 45
	void PENTIUMOP(cmovne_r32_rm32)();    // Opcode 0x0f 45
	void PENTIUMOP(cmovbe_r16_rm16)();    // Opcode 0x0f 46
	void PENTIUMOP(cmovbe_r32_rm32)();    // Opcode 0x0f 46
	void PENTIUMOP(cmova_r16_rm16)();    // Opcode 0x0f 47
	void PENTIUMOP(cmova_r32_rm32)();    // Opcode 0x0f 47
	void PENTIUMOP(cmovs_r16_rm16)();    // Opcode 0x0f 48
	void PENTIUMOP(cmovs_r32_rm32)();    // Opcode 0x0f 48
	void PENTIUMOP(cmovns_r16_rm16)();    // Opcode 0x0f 49
	void PENTIUMOP(cmovns_r32_rm32)();    // Opcode 0x0f 49
	void PENTIUMOP(cmovp_r16_rm16)();    // Opcode 0x0f 4a
	void PENTIUMOP(cmovp_r32_rm32)();    // Opcode 0x0f 4a
	void PENTIUMOP(cmovnp_r16_rm16)();    // Opcode 0x0f 4b
	void PENTIUMOP(cmovnp_r32_rm32)();    // Opcode 0x0f 4b
	void PENTIUMOP(cmovl_r16_rm16)();    // Opcode 0x0f 4c
	void PENTIUMOP(cmovl_r32_rm32)();    // Opcode 0x0f 4c
	void PENTIUMOP(cmovge_r16_rm16)();    // Opcode 0x0f 4d
	void PENTIUMOP(cmovge_r32_rm32)();    // Opcode 0x0f 4d
	void PENTIUMOP(cmovle_r16_rm16)();    // Opcode 0x0f 4e
	void PENTIUMOP(cmovle_r32_rm32)();    // Opcode 0x0f 4e
	void PENTIUMOP(cmovg_r16_rm16)();    // Opcode 0x0f 4f
	void PENTIUMOP(cmovg_r32_rm32)();    // Opcode 0x0f 4f
	void PENTIUMOP(movnti_m16_r16)(); // Opcode 0f c3
	void PENTIUMOP(movnti_m32_r32)(); // Opcode 0f c3
	void PENTIUMOP(cmpxchg8b_m64)();  // Opcode 0x0f c7
	void PENTIUMOP(movntq_m64_r64)(); // Opcode 0f e7
	void PENTIUMOP(maskmovq_r64_r64)();  // Opcode 0f f7
	void SSEOP(maskmovdqu_r128_r128)();  // Opcode 66 0f f7
	void PENTIUMOP(popcnt_r16_rm16)();    // Opcode f3 0f b8
	void PENTIUMOP(popcnt_r32_rm32)();    // Opcode f3 0f b8
	void PENTIUMOP(tzcnt_r16_rm16)();
	void PENTIUMOP(tzcnt_r32_rm32)();
	void MMXOP(group_0f71)();  // Opcode 0f 71
	void SSEOP(group_660f71)();  // Opcode 66 0f 71
	void MMXOP(group_0f72)();  // Opcode 0f 72
	void SSEOP(group_660f72)();  // Opcode 66 0f 72
	void MMXOP(group_0f73)();  // Opcode 0f 73
	void SSEOP(group_660f73)();  // Opcode 66 0f 73
	void MMXOP(psrlw_r64_rm64)();  // Opcode 0f d1
	void MMXOP(psrld_r64_rm64)();  // Opcode 0f d2
	void MMXOP(psrlq_r64_rm64)();  // Opcode 0f d3
	void MMXOP(paddq_r64_rm64)();  // Opcode 0f d4
	void MMXOP(pmullw_r64_rm64)();  // Opcode 0f d5
	void MMXOP(psubusb_r64_rm64)();  // Opcode 0f d8
	void MMXOP(psubusw_r64_rm64)();  // Opcode 0f d9
	void MMXOP(pand_r64_rm64)();  // Opcode 0f db
	void MMXOP(paddusb_r64_rm64)();  // Opcode 0f dc
	void MMXOP(paddusw_r64_rm64)();  // Opcode 0f dd
	void MMXOP(pandn_r64_rm64)();  // Opcode 0f df
	void MMXOP(psraw_r64_rm64)();  // Opcode 0f e1
	void MMXOP(psrad_r64_rm64)();  // Opcode 0f e2
	void MMXOP(pmulhw_r64_rm64)();  // Opcode 0f e5
	void MMXOP(psubsb_r64_rm64)();  // Opcode 0f e8
	void MMXOP(psubsw_r64_rm64)();  // Opcode 0f e9
	void MMXOP(por_r64_rm64)();  // Opcode 0f eb
	void MMXOP(paddsb_r64_rm64)();  // Opcode 0f ec
	void MMXOP(paddsw_r64_rm64)();  // Opcode 0f ed
	void MMXOP(pxor_r64_rm64)();  // Opcode 0f ef
	void MMXOP(psllw_r64_rm64)();  // Opcode 0f f1
	void MMXOP(pslld_r64_rm64)();  // Opcode 0f f2
	void MMXOP(psllq_r64_rm64)();  // Opcode 0f f3
	void MMXOP(pmaddwd_r64_rm64)();  // Opcode 0f f5
	void MMXOP(psubb_r64_rm64)();  // Opcode 0f f8
	void MMXOP(psubw_r64_rm64)();  // Opcode 0f f9
	void MMXOP(psubd_r64_rm64)();  // Opcode 0f fa
	void MMXOP(paddb_r64_rm64)();  // Opcode 0f fc
	void MMXOP(paddw_r64_rm64)();  // Opcode 0f fd
	void MMXOP(paddd_r64_rm64)();  // Opcode 0f fe
	void MMXOP(emms)(); // Opcode 0f 77
	void MMXOP(movd_r64_rm32)(); // Opcode 0f 6e
	void MMXOP(movq_r64_rm64)(); // Opcode 0f 6f
	void MMXOP(movd_rm32_r64)(); // Opcode 0f 7e
	void MMXOP(movq_rm64_r64)(); // Opcode 0f 7f
	void MMXOP(pcmpeqb_r64_rm64)(); // Opcode 0f 74
	void MMXOP(pcmpeqw_r64_rm64)(); // Opcode 0f 75
	void MMXOP(pcmpeqd_r64_rm64)(); // Opcode 0f 76
	void MMXOP(pshufw_r64_rm64_i8)(); // Opcode 0f 70
	void SSEOP(punpcklbw_r128_rm128)(); // Opcode 66 0f 60
	void SSEOP(punpcklwd_r128_rm128)();
	void SSEOP(punpckldq_r128_rm128)();
	void SSEOP(punpcklqdq_r128_rm128)();
	void MMXOP(punpcklbw_r64_r64m32)(); // Opcode 0f 60
	void MMXOP(punpcklwd_r64_r64m32)(); // Opcode 0f 61
	void MMXOP(punpckldq_r64_r64m32)(); // Opcode 0f 62
	void MMXOP(packsswb_r64_rm64)(); // Opcode 0f 63
	void MMXOP(pcmpgtb_r64_rm64)(); // Opcode 0f 64
	void MMXOP(pcmpgtw_r64_rm64)(); // Opcode 0f 65
	void MMXOP(pcmpgtd_r64_rm64)(); // Opcode 0f 66
	void MMXOP(packuswb_r64_rm64)(); // Opcode 0f 67
	void MMXOP(punpckhbw_r64_rm64)(); // Opcode 0f 68
	void MMXOP(punpckhwd_r64_rm64)(); // Opcode 0f 69
	void MMXOP(punpckhdq_r64_rm64)(); // Opcode 0f 6a
	void MMXOP(packssdw_r64_rm64)(); // Opcode 0f 6b
	void SSEOP(group_0fae)();  // Opcode 0f ae
	void SSEOP(cvttps2dq_r128_rm128)(); // Opcode f3 0f 5b
	void SSEOP(cvtss2sd_r128_r128m32)(); // Opcode f3 0f 5a
	void SSEOP(cvttss2si_r32_r128m32)(); // Opcode f3 0f 2c
	void SSEOP(cvtss2si_r32_r128m32)(); // Opcode f3 0f 2d
	void SSEOP(cvtsi2ss_r128_rm32)(); // Opcode f3 0f 2a
	void SSEOP(cvtpi2ps_r128_rm64)(); // Opcode 0f 2a
	void SSEOP(cvttps2pi_r64_r128m64)(); // Opcode 0f 2c
	void SSEOP(cvtps2pi_r64_r128m64)(); // Opcode 0f 2d
	void SSEOP(cvtps2pd_r128_r128m64)(); // Opcode 0f 5a
	void SSEOP(cvtdq2ps_r128_rm128)(); // Opcode 0f 5b
	void SSEOP(cvtdq2pd_r128_r128m64)(); // Opcode f3 0f e6
	void SSEOP(movss_r128_rm128)(); // Opcode f3 0f 10
	void SSEOP(movss_rm128_r128)(); // Opcode f3 0f 11
	void SSEOP(movsldup_r128_rm128)(); // Opcode f3 0f 12
	void SSEOP(movshdup_r128_rm128)(); // Opcode f3 0f 16
	void SSEOP(movaps_r128_rm128)(); // Opcode 0f 28
	void SSEOP(movaps_rm128_r128)(); // Opcode 0f 29
	void SSEOP(movups_r128_rm128)(); // Opcode 0f 10
	void SSEOP(movupd_r128_rm128)(); // Opcode 66 0f 10
	void SSEOP(movups_rm128_r128)(); // Opcode 0f 11
	void SSEOP(movupd_rm128_r128)(); // Opcode 66 0f 11
	void SSEOP(movlps_r128_m64)(); // Opcode 0f 12
	void SSEOP(movlpd_r128_m64)(); // Opcode 66 0f 12
	void SSEOP(movlps_m64_r128)(); // Opcode 0f 13
	void SSEOP(movlpd_m64_r128)(); // Opcode 66 0f 13
	void SSEOP(movhps_r128_m64)(); // Opcode 0f 16
	void SSEOP(movhpd_r128_m64)(); // Opcode 66 0f 16
	void SSEOP(movhps_m64_r128)(); // Opcode 0f 17
	void SSEOP(movhpd_m64_r128)(); // Opcode 66 0f 17
	void SSEOP(movntps_m128_r128)(); // Opcode 0f 2b
	void SSEOP(movmskps_r16_r128)(); // Opcode 0f 50
	void SSEOP(movmskps_r32_r128)(); // Opcode 0f 50
	void SSEOP(movmskpd_r32_r128)(); // Opcode 66 0f 50
	void SSEOP(movq2dq_r128_r64)(); // Opcode f3 0f d6
	void SSEOP(movdqu_r128_rm128)(); // Opcode f3 0f 6f
	void SSEOP(movdqu_rm128_r128)(); // Opcode f3 0f 7f
	void SSEOP(movd_m128_rm32)(); // Opcode 66 0f 6e
	void SSEOP(movdqa_m128_rm128)(); // Opcode 66 0f 6f
	void SSEOP(movq_r128_r128m64)(); // Opcode f3 0f 7e
	void SSEOP(movd_rm32_r128)(); // Opcode 66 0f 7e
	void SSEOP(movdqa_rm128_r128)(); // Opcode 66 0f 7f
	void SSEOP(pmovmskb_r16_r64)(); // Opcode 0f d7
	void SSEOP(pmovmskb_r32_r64)(); // Opcode 0f d7
	void SSEOP(pmovmskb_r32_r128)(); // Opcode 66 0f d7
	void SSEOP(xorps)(); // Opcode 0f 57
	void SSEOP(xorpd_r128_rm128)(); // Opcode 66 0f 57
	void SSEOP(addps)(); // Opcode 0f 58
	void SSEOP(sqrtps_r128_rm128)(); // Opcode 0f 51
	void SSEOP(rsqrtps_r128_rm128)(); // Opcode 0f 52
	void SSEOP(rcpps_r128_rm128)(); // Opcode 0f 53
	void SSEOP(andps_r128_rm128)(); // Opcode 0f 54
	void SSEOP(andpd_r128_rm128)(); // Opcode 66 0f 54
	void SSEOP(andnps_r128_rm128)(); // Opcode 0f 55
	void SSEOP(andnpd_r128_rm128)(); // Opcode 66 0f 55
	void SSEOP(orps_r128_rm128)(); // Opcode 0f 56
	void SSEOP(orpd_r128_rm128)(); // Opcode 66 0f 56
	void SSEOP(mulps)(); // Opcode 0f 59 ????
	void SSEOP(subps)(); // Opcode 0f 5c
	void SSEOP(minps)(); // Opcode 0f 5d
	void SSEOP(divps)(); // Opcode 0f 5e
	void SSEOP(maxps)(); // Opcode 0f 5f
	void SSEOP(maxss_r128_r128m32)(); // Opcode f3 0f 5f
	void SSEOP(addss)(); // Opcode f3 0f 58
	void SSEOP(subss)(); // Opcode f3 0f 5c
	void SSEOP(mulss)(); // Opcode f3 0f 5e
	void SSEOP(divss)(); // Opcode 0f 59
	void SSEOP(rcpss_r128_r128m32)(); // Opcode f3 0f 53
	void SSEOP(sqrtss_r128_r128m32)(); // Opcode f3 0f 51
	void SSEOP(rsqrtss_r128_r128m32)(); // Opcode f3 0f 52
	void SSEOP(minss_r128_r128m32)(); // Opcode f3 0f 5d
	void SSEOP(comiss_r128_r128m32)(); // Opcode 0f 2f
	void SSEOP(comisd_r128_r128m64)(); // Opcode 66 0f 2f
	void SSEOP(ucomiss_r128_r128m32)(); // Opcode 0f 2e
	void SSEOP(ucomisd_r128_r128m64)(); // Opcode 66 0f 2e
	void SSEOP(shufps)(); // Opcode 0f c6
	void SSEOP(shufpd_r128_rm128_i8)(); // Opcode 66 0f c6
	void SSEOP(unpcklps_r128_rm128)(); // Opcode 0f 14
	void SSEOP(unpcklpd_r128_rm128)(); // Opcode 66 0f 14
	void SSEOP(unpckhps_r128_rm128)(); // Opcode 0f 15
	void SSEOP(unpckhpd_r128_rm128)(); // Opcode 66 0f 15
	void SSEOP(predicate_compare_single)(UINT8 imm8, XMM_REG d, XMM_REG s);
	void SSEOP(predicate_compare_double)(UINT8 imm8, XMM_REG d, XMM_REG s);
	void SSEOP(predicate_compare_single_scalar)(UINT8 imm8, XMM_REG d, XMM_REG s);
	void SSEOP(predicate_compare_double_scalar)(UINT8 imm8, XMM_REG d, XMM_REG s);
	void SSEOP(cmpps_r128_rm128_i8)(); // Opcode 0f c2
	void SSEOP(cmppd_r128_rm128_i8)(); // Opcode 66 0f c2
	void SSEOP(cmpss_r128_r128m32_i8)(); // Opcode f3 0f c2
	void SSEOP(pinsrw_r64_r16m16_i8)(); // Opcode 0f c4, 16bit register
	void SSEOP(pinsrw_r64_r32m16_i8)(); // Opcode 0f c4, 32bit register
	void SSEOP(pinsrw_r128_r32m16_i8)(); // Opcode 66 0f c4
	void SSEOP(pextrw_r16_r64_i8)(); // Opcode 0f c5
	void SSEOP(pextrw_r32_r64_i8)(); // Opcode 0f c5
	void SSEOP(pextrw_reg_r128_i8)(); // Opcode 66 0f c5
	void SSEOP(pminub_r64_rm64)(); // Opcode 0f da
	void SSEOP(pminub_r128_rm128)(); // Opcode 66 0f da
	void SSEOP(pmaxub_r64_rm64)(); // Opcode 0f de
	void SSEOP(pavgb_r64_rm64)(); // Opcode 0f e0
	void SSEOP(pavgw_r64_rm64)(); // Opcode 0f e3
	void SSEOP(pmulhuw_r64_rm64)();  // Opcode 0f e4
	void SSEOP(pminsw_r64_rm64)(); // Opcode 0f ea
	void SSEOP(pmaxsw_r64_rm64)(); // Opcode 0f ee
	void SSEOP(pmuludq_r64_rm64)(); // Opcode 0f f4
	void SSEOP(pmuludq_r128_rm128)(); // Opcode 66 0f f4
	void SSEOP(psadbw_r64_rm64)(); // Opcode 0f f6
	void SSEOP(psubq_r64_rm64)();  // Opcode 0f fb
	void SSEOP(psubq_r128_rm128)();  // Opcode 66 0f fb
	void SSEOP(pshufd_r128_rm128_i8)(); // Opcode 66 0f 70
	void SSEOP(pshuflw_r128_rm128_i8)(); // Opcode f2 0f 70
	void SSEOP(pshufhw_r128_rm128_i8)(); // Opcode f3 0f 70
	void SSEOP(packsswb_r128_rm128)(); // Opcode 66 0f 63
	void SSEOP(packssdw_r128_rm128)(); // Opcode 66 0f 6b
	void SSEOP(pcmpgtb_r128_rm128)(); // Opcode 66 0f 64
	void SSEOP(pcmpgtw_r128_rm128)(); // Opcode 66 0f 65
	void SSEOP(pcmpgtd_r128_rm128)(); // Opcode 66 0f 66
	void SSEOP(packuswb_r128_rm128)(); // Opcode 66 0f 67
	void SSEOP(punpckhbw_r128_rm128)(); // Opcode 66 0f 68
	void SSEOP(punpckhwd_r128_rm128)(); // Opcode 66 0f 69
	void SSEOP(unpckhdq_r128_rm128)(); // Opcode 66 0f 6a
	void SSEOP(punpckhqdq_r128_rm128)(); // Opcode 66 0f 6d
	void SSEOP(pcmpeqb_r128_rm128)(); // Opcode 66 0f 74
	void SSEOP(pcmpeqw_r128_rm128)(); // Opcode 66 0f 75
	void SSEOP(pcmpeqd_r128_rm128)(); // Opcode 66 0f 76
	void SSEOP(paddq_r128_rm128)();  // Opcode 66 0f d4
	void SSEOP(pmullw_r128_rm128)();  // Opcode 66 0f d5
	void SSEOP(paddb_r128_rm128)();  // Opcode 66 0f fc
	void SSEOP(paddw_r128_rm128)();  // Opcode 66 0f fd
	void SSEOP(paddd_r128_rm128)();  // Opcode 66 0f fe
	void SSEOP(psubusb_r128_rm128)();  // Opcode 66 0f d8
	void SSEOP(psubusw_r128_rm128)();  // Opcode 66 0f d9
	void SSEOP(pand_r128_rm128)();  // Opcode 66 0f db
	void SSEOP(pandn_r128_rm128)();  // Opcode 66 0f df
	void SSEOP(paddusb_r128_rm128)();  // Opcode 66 0f dc
	void SSEOP(paddusw_r128_rm128)();  // Opcode 66 0f dd
	void SSEOP(pmaxub_r128_rm128)(); // Opcode 66 0f de
	void SSEOP(pmulhuw_r128_rm128)();  // Opcode 66 0f e4
	void SSEOP(pmulhw_r128_rm128)();  // Opcode 66 0f e5
	void SSEOP(psubsb_r128_rm128)();  // Opcode 66 0f e8
	void SSEOP(psubsw_r128_rm128)();  // Opcode 66 0f e9
	void SSEOP(pminsw_r128_rm128)(); // Opcode 66 0f ea
	void SSEOP(pmaxsw_r128_rm128)(); // Opcode 66 0f ee
	void SSEOP(paddsb_r128_rm128)();  // Opcode 66 0f ec
	void SSEOP(paddsw_r128_rm128)();  // Opcode 66 0f ed
	void SSEOP(por_r128_rm128)();  // Opcode 66 0f eb
	void SSEOP(pxor_r128_rm128)();  // Opcode 66 0f ef
	void SSEOP(pmaddwd_r128_rm128)();  // Opcode 66 0f f5
	void SSEOP(psubb_r128_rm128)();  // Opcode 66 0f f8
	void SSEOP(psubw_r128_rm128)();  // Opcode 66 0f f9
	void SSEOP(psubd_r128_rm128)();  // Opcode 66 0f fa
	void SSEOP(psadbw_r128_rm128)(); // Opcode 66 0f f6
	void SSEOP(pavgb_r128_rm128)(); // Opcode 66 0f e0
	void SSEOP(pavgw_r128_rm128)(); // Opcode 66 0f e3
	void SSEOP(psrlw_r128_rm128)();  // Opcode 66 0f d1
	void SSEOP(psrld_r128_rm128)();  // Opcode 66 0f d2
	void SSEOP(psrlq_r128_rm128)();  // Opcode 66 0f d3
	void SSEOP(psllw_r128_rm128)();  // Opcode 66 0f f1
	void SSEOP(pslld_r128_rm128)();  // Opcode 66 0f f2
	void SSEOP(psllq_r128_rm128)();  // Opcode 66 0f f3
	void SSEOP(psraw_r128_rm128)();  // Opcode 66 0f e1
	void SSEOP(psrad_r128_rm128)();  // Opcode 66 0f e2
	void SSEOP(movntdq_m128_r128)();  // Opcode 66 0f e7
	void SSEOP(cvttpd2dq_r128_rm128)();  // Opcode 66 0f e6
	void SSEOP(movq_r128m64_r128)();  // Opcode 66 0f d6
	void SSEOP(addsubpd_r128_rm128)();  // Opcode 66 0f d0
	void SSEOP(haddpd_r128_rm128)();  // Opcode 66 0f 7c
	void SSEOP(hsubpd_r128_rm128)();  // Opcode 66 0f 7d
	void SSEOP(sqrtpd_r128_rm128)();  // Opcode 66 0f 51
	void SSEOP(cvtpi2pd_r128_rm64)();  // Opcode 66 0f 2a
	void SSEOP(cvttpd2pi_r64_rm128)();  // Opcode 66 0f 2c
	void SSEOP(cvtpd2pi_r64_rm128)();  // Opcode 66 0f 2d
	void SSEOP(cvtpd2ps_r128_rm128)();  // Opcode 66 0f 5a
	void SSEOP(cvtps2dq_r128_rm128)();  // Opcode 66 0f 5b
	void SSEOP(addpd_r128_rm128)();  // Opcode 66 0f 58
	void SSEOP(mulpd_r128_rm128)();  // Opcode 66 0f 59
	void SSEOP(subpd_r128_rm128)();  // Opcode 66 0f 5c
	void SSEOP(minpd_r128_rm128)();  // Opcode 66 0f 5d
	void SSEOP(divpd_r128_rm128)();  // Opcode 66 0f 5e
	void SSEOP(maxpd_r128_rm128)();  // Opcode 66 0f 5f
	void SSEOP(movntpd_m128_r128)();  // Opcode 66 0f 2b
	void SSEOP(movapd_r128_rm128)();  // Opcode 66 0f 28
	void SSEOP(movapd_rm128_r128)();  // Opcode 66 0f 29
	void SSEOP(movsd_r128_r128m64)(); // Opcode f2 0f 10
	void SSEOP(movsd_r128m64_r128)(); // Opcode f2 0f 11
	void SSEOP(movddup_r128_r128m64)(); // Opcode f2 0f 12
	void SSEOP(cvtsi2sd_r128_rm32)(); // Opcode f2 0f 2a
	void SSEOP(cvttsd2si_r32_r128m64)(); // Opcode f2 0f 2c
	void SSEOP(cvtsd2si_r32_r128m64)(); // Opcode f2 0f 2d
	void SSEOP(sqrtsd_r128_r128m64)(); // Opcode f2 0f 51
	void SSEOP(addsd_r128_r128m64)(); // Opcode f2 0f 58
	void SSEOP(mulsd_r128_r128m64)(); // Opcode f2 0f 59
	void SSEOP(cvtsd2ss_r128_r128m64)(); // Opcode f2 0f 5a
	void SSEOP(subsd_r128_r128m64)(); // Opcode f2 0f 5c
	void SSEOP(minsd_r128_r128m64)(); // Opcode f2 0f 5d
	void SSEOP(divsd_r128_r128m64)(); // Opcode f2 0f 5e
	void SSEOP(maxsd_r128_r128m64)(); // Opcode f2 0f 5f
	void SSEOP(haddps_r128_rm128)(); // Opcode f2 0f 7c
	void SSEOP(hsubps_r128_rm128)(); // Opcode f2 0f 7d
	void SSEOP(cmpsd_r128_r128m64_i8)(); // Opcode f2 0f c2
	void SSEOP(addsubps_r128_rm128)(); // Opcode f2 0f d0
	void SSEOP(movdq2q_r64_r128)(); // Opcode f2 0f d6
	void SSEOP(cvtpd2dq_r128_rm128)(); // Opcode f2 0f e6
	void SSEOP(lddqu_r128_m128)(); // Opcode f2 0f f0
	// x87 FPU
	INLINE void WRITE80( UINT32 ea, floatx80 t);
	INLINE void x87_set_stack_top( int top);
	INLINE void x87_set_tag( int reg, int tag);
	void x87_write_stack( int i, floatx80 value, int update_tag);
	INLINE void x87_set_stack_underflow();
	INLINE void x87_set_stack_overflow();
	INLINE void x87_write_cw( UINT16 cw);
	void x87_reset();
	void x87_fadd_m32real( UINT8 modrm);
	void x87_fadd_m64real( UINT8 modrm);
	void x87_fadd_st_sti( UINT8 modrm);
	void x87_fadd_sti_st( UINT8 modrm);
	void x87_faddp( UINT8 modrm);
	void x87_fiadd_m32int( UINT8 modrm);
	void x87_fiadd_m16int( UINT8 modrm);
	void x87_fsub_m32real( UINT8 modrm);
	void x87_fsub_m64real( UINT8 modrm);
	void x87_fsub_st_sti( UINT8 modrm);
	void x87_fsub_sti_st( UINT8 modrm);
	void x87_fsubp( UINT8 modrm);
	void x87_fisub_m32int( UINT8 modrm);
	void x87_fisub_m16int( UINT8 modrm);
	void x87_fsubr_m32real( UINT8 modrm);
	void x87_fsubr_m64real( UINT8 modrm);
	void x87_fsubr_st_sti( UINT8 modrm);
	void x87_fsubr_sti_st( UINT8 modrm);
	void x87_fsubrp( UINT8 modrm);
	void x87_fisubr_m32int( UINT8 modrm);
	void x87_fisubr_m16int( UINT8 modrm);
	void x87_fdiv_m32real( UINT8 modrm);
	void x87_fdiv_m64real( UINT8 modrm);
	void x87_fdiv_st_sti( UINT8 modrm);
	void fx87_fdiv_sti_st( UINT8 modrm);
	void x87_fdivp( UINT8 modrm);
	void x87_fidiv_m32int( UINT8 modrm);
	void x87_fidiv_m16int( UINT8 modrm);
	void x87_fdivr_m32real( UINT8 modrm);
	void x87_fdivr_m64real( UINT8 modrm);
	void x87_fdivr_st_sti( UINT8 modrm);
	void x87_fdivr_sti_st( UINT8 modrm);
	void x87_fdivrp( UINT8 modrm);
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
	void I386OP(x87_group_d8);();
	void I386OP(x87_group_d9);();
	void I386OP(x87_group_da);();
	void I386OP(x87_group_db);();
	void I386OP(x87_group_dc);();
	void I386OP(x87_group_dd);();
	void I386OP(x87_group_de);();
	void I386OP(x87_group_df);();
	void build_x87_opcode_table_d8();
	void build_x87_opcode_table_d9();
	void build_x87_opcode_table_da();
	void build_x87_opcode_table_db();
	void build_x87_opcode_table_dc();
	void build_x87_opcode_table_dd();
	void build_x87_opcode_table_de();
	void build_x87_opcode_table_df();
	void build_x87_opcode_table();

	// Inline Utilities.
	INLINE UINT8 SaturatedSignedWordToUnsignedByte(INT16 word);
	INLINE UINT16 SaturatedSignedDwordToUnsignedWord(INT32 dword);
	//
	INLINE int translate_address(int pl, int type, UINT32 *address, UINT32 *error);
	INLINE UINT32 i386_translate(int segment, UINT32 ip, int rwn);
	
	INLINE UINT32 FETCH32();
	INLINE UINT32 READ32(UINT32 ea);
	INLINE UINT32 READ32PL0(UINT32 ea);
	INLINE UINT32 OR32(UINT32 dst, UINT32 src);
	INLINE UINT32 AND32(UINT32 dst, UINT32 src);
	INLINE UINT32 XOR32(UINT32 dst, UINT32 src);
	INLINE UINT32 SBB32(UINT32 dst, UINT32 src, UINT32 b);
	INLINE UINT32 ADC32(UINT32 dst, UINT32 src, UINT32 c);
	INLINE UINT32 INC32(UINT32 dst);
	INLINE UINT32 DEC32(UINT32 dst);
	INLINE UINT32 POP32();
	INLINE UINT32 READPORT32( offs_t port);
	INLINE UINT8 FETCH();
	INLINE UINT8 READ8(UINT32 ea);
	INLINE UINT8 READ8PL0(UINT32 ea);
	INLINE UINT8 OR8(UINT8 dst, UINT8 src);
	INLINE UINT8 AND8(UINT8 dst, UINT8 src);
	INLINE UINT8 XOR8(UINT8 dst, UINT8 src);
	INLINE UINT8 SBB8(UINT8 dst, UINT8 src, UINT8 b);
	INLINE UINT8 ADC8(UINT8 dst, UINT8 src, UINT8 c);
	INLINE UINT8 DEC8(UINT8 dst);
	INLINE UINT8 POP8();
	INLINE UINT8 READPORT8( offs_t port);
	INLINE UINT16 FETCH16();
	INLINE UINT16 READ16(UINT32 ea);
	INLINE UINT16 READ16PL0(UINT32 ea);
	INLINE UINT16 OR16(UINT16 dst, UINT16 src);
	INLINE UINT16 AND16(UINT16 dst, UINT16 src);
	INLINE UINT16 XOR16(UINT16 dst, UINT16 src);
	INLINE UINT16 SBB16(UINT16 dst, UINT16 src, UINT16 b);
	INLINE UINT16 ADC16(UINT16 dst, UINT16 src, UINT8 c);
	INLINE UINT16 INC16(UINT16 dst);
	INLINE UINT16 DEC16(UINT16 dst);
	INLINE UINT16 POP16();
	INLINE UINT16 READPORT16( offs_t port);
	INLINE UINT64 READ64(UINT32 ea);
	INLINE UINT64 MSR_READ(UINT32 offset,UINT8 *valid_msr);
	
	// Utilities around FPU.
	INLINE int floatx80_is_zero(floatx80 fx);
	INLINE int floatx80_is_inf(floatx80 fx);
	INLINE int floatx80_is_denormal(floatx80 fx);
	INLINE void x87_set_stack_top( int top);
	INLINE void x87_set_tag( int reg, int tag);
	void x87_write_stack( int i, floatx80 value, int update_tag);
	int x87_inc_stack();
	int x87_dec_stack();
	int x87_check_exceptions();
	
	INLINE floatx80 ffloatx80_abs(floatx80 fx);
	INLINE floatx80 fdouble_to_fx80(double in);
	INLINE floatx80 fREAD80( UINT32 ea);
	floatx80 fx87_add( floatx80 a, floatx80 b);
	floatx80 fx87_sub( floatx80 a, floatx80 b);
	floatx80 x87_mul( floatx80 a, floatx80 b);
	floatx80 x87_div( floatx80 a, floatx80 b);
	
protected:
	void I386OP(decode_two_byte)()
	UINT64 pentium_msr_read(i386_state *cpustate, UINT32 offset,UINT8 *valid_msr);;
	UINT64 p6_msr_read(i386_state *cpustate, UINT32 offset,UINT8 *valid_msr);;
	UINT64 piv_msr_read(i386_state *cpustate, UINT32 offset,UINT8 *valid_msr);;

	INLINE UINT32 i386_translate(int segment, UINT32 ip, int rwn);;
	INLINE vtlb_entry get_permissions(UINT32 pte, int wp);;
	void pentium_msr_write(i386_state *cpustate, UINT32 offset, UINT64 data, UINT8 *valid_msr);;
	void p6_msr_write(i386_state *cpustate, UINT32 offset, UINT64 data, UINT8 *valid_msr);;
	void piv_msr_write(i386_state *cpustate, UINT32 offset, UINT64 data, UINT8 *valid_msr);;
	void i386_load_call_gate(I386_CALL_GATE *gate);;
	void i386_set_descriptor_accessed( UINT16 selector);;
	void i386_load_segment_descriptor( int segment );
	void set_flags( UINT32 f );;
	void sib_byte(UINT8 mod, UINT32* out_ea, UINT8* out_segment);;
	void modrm_to_EA(UINT8 mod_rm, UINT32;* out_ea, UINT8* out_segment);;
	void i386_check_sreg_validity(int reg);;
	void i386_sreg_load( UINT16 selector, UINT8 reg, bool *fault);
	void i386_trap(int irq, int irq_gate, int trap_level);
	void i386_trap_with_error(int irq, int irq_gate, int trap_level, UINT32 error);
	void i286_task_switch( UINT16 selector, UINT8 nested);
	void i386_task_switch( UINT16 selector, UINT8 nested);

	void i386_protected_mode_jump( UINT16 seg, UINT32 off, int indirect, int operand32);
	void i386_protected_mode_call( UINT16 seg, UINT32 off, int indirect, int operand32);
	void i386_protected_mode_retf(UINT8 count, UINT8 operand32);
	void i386_protected_mode_iret(int operand32);
	void build_cycle_table();;;
	void report_invalid_opcode();;;
	void report_invalid_modrm( const char* opcode, UINT8 modrm);
	void i386_postload();;;

};

#endif
