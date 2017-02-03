
#ifndef __LIB_I386_OPDEF_H__
#define __LIB_I386_OPDEF_H__

#include "./i386priv.h"
#include "./i386ops.h"

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


class DEBUG;
class I386_OPS_BASE {
protected:
	int i386_parity_table[256];
	MODRM_TABLE i386_MODRM_table[256];
	i386_state *cpustate;
	const X86_OPCODE x86_opcode_table[];
	const X86_CYCLE_TABLE x86_cycle_table[];
	UINT8 cycle_table_rm[X86_NUM_CPUS][CYCLES_NUM_OPCODES];
	UINT8 cycle_table_pm[X86_NUM_CPUS][CYCLES_NUM_OPCODES];


protected:
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
	void i386_check_sreg_validity(int reg);
	int i386_limit_check( int seg, UINT32 offset);
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
	void build_cycle_table();
	void report_invalid_opcode();
	void report_invalid_modrm(const char* opcode, UINT8 modrm);
	void i386_postload();
	i386_state *i386_common_init(int tlbsize);
	void build_opcode_table( UINT32 features);
	void zero_state();
	void pentium_smi();
	void i386_set_irq_line(int irqline, int state);
	void i386_set_a20_line(int state);
protected:
	// Utilities.
	void cpu_init_i386(void);
	void cpu_init_i486(void);
	void cpu_init_pentium(void);
	void cpu_init_mediagx(void);
	void cpu_init_pentium_pro(void);
	void cpu_init_pentium_mmx(void);
	void cpu_init_pentium2(void);
	void cpu_init_pentium3(void);
	void cpu_init_pentium4(void);

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
	void I386OP(decode_two_byte)();
	INLINE UINT32 i386_translate(int segment, UINT32 ip, int rwn);
	INLINE vtlb_entry get_permissions(UINT32 pte, int wp);
	int i386_translate_address(int intention, offs_t *address, vtlb_entry *entry);

public:
	I386_OPS_BASE(void)
	{
		cpustate = NULL;
	}
	~I386_OPS_BASE() {}
	void I386OP(decode_opcode)();
	
	virtual int cpu_translate_i386(void *cpudevice, address_spacenum space, int intention, offs_t *address);
	virtual int cpu_execute_i386(int cycles);
};

#endif
