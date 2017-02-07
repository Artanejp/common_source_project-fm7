
#ifndef __LIB_I386_REAL_H__
#define __LIB_I386_REAL_H__

#include "./i386_opdef.h"

struct I386_OPCODE;
class I386_OPS: public I386_OPS_BASE
{
public:
	I386_OPS(int cputypes = I386_OPS_CPUTYPE_I386) : I386_OPS_BASE(cputypes)
	{
	}

	~I386_OPS() {}

	int cpu_translate_i386(void *cpudevice, address_spacenum space, int intention, offs_t *address);
	int cpu_execute_i386(int cycles);
	void i386_trap(int irq, int irq_gate, int trap_level);
	void i386_trap_with_error(int irq, int irq_gate, int trap_level, UINT32 error);
	void i386_call_abs16();        // Opcode 0x9a
	void i386_call_rel16();        // Opcode 0xe8
	void i386_groupFF_16();        // Opcode 0xff
	void i386_decode_opcode();
	//void build_opcode_table(UINT32 features);
	i386_state *i386_common_init(int tlbsize);

	void set_context_io_stored(DEVICE *dev) {
#ifdef USE_DEBUGGER
		cpustate->io_stored = dev;
#endif
	}

	void set_context_emu(EMU *p_emu) {
#ifdef USE_DEBUGGER
		cpustate->emu = p_emu;
#endif
	}

	void set_context_debugger(DEBUGGER *debugger) {
#ifdef USE_DEBUGGER
		cpustate->debugger = debugger;
#endif
	}
	void set_context_progmem_stored(DEVICE *dev) {
#ifdef USE_DEBUGGER
		cpustate->program_stored = dev;
#endif
	}
	void set_context_pseudo_bios(DEVICE *dev) {
#ifdef I386_PSEUDO_BIOS
		cpustate->bios = dev;
		d_bios = dev;
#endif
	}
	void set_context_dma(DEVICE *dev) {
#ifdef SINGLE_MODE_DMA
		cpustate->dma = dev;
		d_dma = dev;
#endif
	}
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	void get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len);

protected:
	const UINT8 *opcode_ptr;
	const UINT8 *opcode_ptr_base;
	int address_size;
	int operand_size;
	int address_prefix;
	int operand_prefix;
	int max_length;
	UINT64 pc;
	UINT8 modrm;
	UINT32 segment;
	offs_t dasm_flags;
	_TCHAR modrm_string[256];
	UINT8 rex, regex, sibex, rmex;
	UINT8 pre0f;
	UINT8 curmode;

	INLINE UINT8 _FETCH(void);
#if 0
	INLINE UINT16 _FETCH16(void);
#endif
	INLINE UINT32 _FETCH32(void);
	INLINE UINT8 _FETCHD(void);
	INLINE UINT16 _FETCHD16(void);
	INLINE UINT32 _FETCHD32(void);
	_TCHAR *hexstring(UINT32 value, int digits);
	_TCHAR *hexstring64(UINT32 lo, UINT32 hi);
	_TCHAR *hexstringpc(UINT64 pc);
	_TCHAR *shexstring(UINT32 value, int digits, int always);
	_TCHAR* handle_sib_byte( _TCHAR* s, UINT8 mod );
	void handle_modrm(_TCHAR* s);
	_TCHAR* handle_param(_TCHAR* s, UINT32 param);
	void handle_fpu(_TCHAR *s, UINT8 op1, UINT8 op2);
	void decode_opcode(_TCHAR *s, const I386_OPCODE *op, UINT8 op1);

	int i386_dasm_one_ex(_TCHAR *buffer, UINT64 eip, const UINT8 *oprom, int mode);
	int i386_dasm_one(_TCHAR *buffer, offs_t eip, const UINT8 *oprom, int mode);
	int cpu_disassemble_x86_16(_TCHAR *buffer, UINT64 eip, const UINT8 *oprom);
	int cpu_disassemble_x86_32(_TCHAR *buffer, UINT64 eip, const UINT8 *oprom);
	int cpu_disassemble_x86_64(_TCHAR *buffer, UINT64 eip, const UINT8 *oprom);
};

INLINE UINT8 I386_OPS::_FETCH(void)
{
	if ((opcode_ptr - opcode_ptr_base) + 1 > max_length)
		return 0xff;
	pc++;
	return *opcode_ptr++;
}

#if 0
INLINE UINT16 I386_OPS::_FETCH16(void)
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

INLINE UINT32 I386_OPS::_FETCH32(void)
{
	UINT32 d;
	if ((opcode_ptr - opcode_ptr_base) + 4 > max_length)
		return 0xffffffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8) | (opcode_ptr[2] << 16) | (opcode_ptr[3] << 24);
	opcode_ptr += 4;
	pc += 4;
	return d;
}

INLINE UINT8 I386_OPS::_FETCHD(void)
{
	if ((opcode_ptr - opcode_ptr_base) + 1 > max_length)
		return 0xff;
	pc++;
	return *opcode_ptr++;
}

INLINE UINT16 I386_OPS::_FETCHD16(void)
{
	UINT16 d;
	if ((opcode_ptr - opcode_ptr_base) + 2 > max_length)
		return 0xffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8);
	opcode_ptr += 2;
	pc += 2;
	return d;
}

INLINE UINT32 I386_OPS::_FETCHD32(void)
{
	UINT32 d;
	if ((opcode_ptr - opcode_ptr_base) + 4 > max_length)
		return 0xffffffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8) | (opcode_ptr[2] << 16) | (opcode_ptr[3] << 24);
	opcode_ptr += 4;
	pc += 4;
	return d;
}



#endif


