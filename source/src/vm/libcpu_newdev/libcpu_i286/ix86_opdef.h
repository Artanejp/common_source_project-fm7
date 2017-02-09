#ifndef __LIBNEWDEV_LIBCPUI86_IX86_OPDEF_H__
#define __LIBNEWDEV_LIBCPUI86_IX86_OPDEF_H__

#include "./ix86_cpustat.h"
enum {
   IX86_CPU_I8086 = 0,
   IX86_CPU_I8088,
   IX86_CPU_I80C86,
   IX86_CPU_I80C88,
   IX86_CPU_I80186,
   IX86_CPU_I80188,
   IX86_CPU_I80286,
   IX86_CPU_V30,
   IX86_CPU_V20,
   IX86_CPU_V30MX,
   IX86_CPU_V40,
   IX86_CPU_V50,
   IX86_CPU_V33,
   IX86_CPU_V25,
   IX86_CPU_V35,
};

class DEVICE;
class EMU;
class DEBUGGER;
class IX86_OPS_BASE {
protected:
	DEVICE *d_mem;
	DEVICE *d_pic;
	DEVICE *d_io;
	DEVICE *d_dma;
	DEVICE *d_bios;
	EMU *emu;
	DEBUGGER *debugger;
	UINT8 parity_table[256];
	struct i80x86_timing timing;
	void i80286_urinit(void);
	i80286_state *cpustate;
 public:
	IX86_OPS_BASE() {
	}
	IX86_OPS_BASE() {};
	
	i86_state *cpu_init_i8086();
	i86_state *cpu_init_i8088();
	i86_state *cpu_init_i80186();
	i86_state *cpu_init_i80286();
	i86_state *cpu_init_v30();
	virtual void CPU_EXECUTE( i8086 );
	void set_irq_line_86(int irqline, int state);
	void cpu_reset_i8086();

	virtual void CPU_EXECUTE( i80186 );
	void set_irq_line_186(int irqline, int state);
	void cpu_reset_i80186();
   
	virtual void CPU_EXECUTE( i80286 );
	void set_irq_line_286(int irqline, int state);
	void cpu_reset_i80286();
	void i80286_set_a20_line(int state);
	virtual void CPU_EXECUTE( v30 );
	void set_irq_line_v30(int irqline, int state);
	void cpu_reset_v30();
	
	virtual void PREFIX186(_pusha)();    /* Opcode 0x60 */
	virtual void PREFIX186(_popa)();    /* Opcode 0x61 */
	virtual void PREFIX186(_insb)();    /* Opcode 0x6c */
	virtual void PREFIX186(_insw)();    /* Opcode 0x6d */
	virtual void PREFIX186(_outsb)();    /* Opcode 0x6e */
	virtual void PREFIX186(_outsw)();    /* Opcode 0x6f */
	virtual void PREFIX186(_enter)();    /* Opcode 0xc8 */
	
	virtual void PREFIX86(_rotate_shift_Byte)(unsigned ModRM, unsigned count, unsigned src);
	virtual void PREFIX86(_rotate_shift_Word)(unsigned ModRM, unsigned count, unsigned src);
	

	virtual void PREFIX86(_pop_es)();    /* Opcode 0x07 */
	virtual void PREFIX86(_pop_cs)();    /* Opcode 0x0f */
	virtual void PREFIX86(_push_sp)();    /* Opcode 0x54 */
	virtual void PREFIX86(_call_far)();
	virtual void PREFIX86(_wait)();    /* Opcode 0x9b */

	virtual void PREFIX86(_popf)();    /* Opcode 0x9d */
	virtual void PREFIX86(_les_dw)();    /* Opcode 0xc4 */
	virtual void PREFIX86(_lds_dw)();    /* Opcode 0xc5 */
	virtual void PREFIX86(_retf_d16)();    /* Opcode 0xca */
	virtual void PREFIX86(_retf)();    /* Opcode 0xcb */

	virtual void PREFIX86(_int)();    /* Opcode 0xcd */
	virtual void PREFIX86(_into)();    /* Opcode 0xce */
	virtual void PREFIX86(_iret)();    /* Opcode 0xcf */
	virtual void PREFIX86(_rotshft_bcl)();    /* Opcode 0xd2 */
	virtual void PREFIX86(_escape)();    /* Opcodes 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde and 0xdf */
	virtual void PREFIX86(_inal)();    /* Opcode 0xe4 */
	virtual void PREFIX86(_inax)();    /* Opcode 0xe5 */
	virtual void PREFIX86(_outal)();    /* Opcode 0xe6 */
	virtual void PREFIX86(_outax)();    /* Opcode 0xe7 */
	virtual void PREFIX86(_call_d16)();    /* Opcode 0xe8 */
	virtual void PREFIX86(_jmp_d16)();    /* Opcode 0xe9 */
	virtual void PREFIX86(_jmp_far)();    /* Opcode 0xea */
	virtual void PREFIX86(_jmp_d8)();    /* Opcode 0xeb */
	virtual void PREFIX86(_inaldx)();    /* Opcode 0xec */
	virtual void PREFIX86(_inaxdx)();    /* Opcode 0xed */
	virtual void PREFIX86(_outdxal)();    /* Opcode 0xee */
	virtual void PREFIX86(_outdxax)();    /* Opcode 0xef */
	virtual void PREFIX86(_lock)();    /* Opcode 0xf0 */
	virtual void PREFIX86(_hlt)();    /* Opcode 0xf4 */
	virtual void PREFIX86(_cli)();    /* Opcode 0xfa */
	virtual void PREFIX86(_ffpre)();    /* Opcode 0xff */
	virtual void PREFIX86(_invalid)();
	virtual void PREFIX86(_invalid_2b)();

public:
	// Not Virtual

	// 80186
	void PREFIX186(_bound)();    /* Opcode 0x62 */
	void PREFIX186(_push_d16)();    /* Opcode 0x68 */
	void PREFIX186(_imul_d16)();    /* Opcode 0x69 */
	void PREFIX186(_push_d8)();    /* Opcode 0x6a */
	void PREFIX186(_imul_d8)();    /* Opcode 0x6b */
	void PREFIX186(_rotshft_bd8)();    /* Opcode 0xc0 */
	void PREFIX186(_rotshft_wd8)();    /* Opcode 0xc1 */
	void PREFIX186(_leave)();    /* Opcode 0xc9 */
	void PREFIX186(_rotshft_bcl)();    /* Opcode 0xd2 */
	void PREFIX186(_rotshft_wcl)();    /* Opcode 0xd3 */
// 80286
	void PREFIX286(_0fpre)();
	void PREFIX286(_arpl)(); /* 0x63 */

	void PREFIX286(_popf)();
	void PREFIX286(_iret)();
	void PREFIX286(_retf_d16)();
	void PREFIX286(_retf)();
	void PREFIX286(_escape)();    /* Opcodes 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde */
	void PREFIX286(_escape_7)();    /* Opcode 0xdf */
// V30
	void PREFIXV30(_0fpre)();	/* Opcode 0x0f */
	void PREFIXV30(_brkn)();		/* Opcode 0x63 BRKN -  Break to Native Mode */
	void PREFIXV30(repc)(int flagval);
	void PREFIXV30(_repnc)();	/* Opcode 0x64 */
	void PREFIXV30(_repc)();		/* Opcode 0x65 */
	void PREFIXV30(_aad)();    /* Opcode 0xd5 */
	void PREFIXV30(_setalc)();	/* Opcode 0xd6 */
	void PREFIXV30(_brks)();		/* Opcode 0xf1 - Break to Security Mode */
// 8086

	void PREFIX86(_add_br8)();    /* Opcode 0x00 */
	void PREFIX86(_add_wr16)();    /* Opcode 0x01 */
	void PREFIX86(_add_r8b)();    /* Opcode 0x02 */
	void PREFIX86(_add_r16w)();    /* Opcode 0x03 */
	void PREFIX86(_add_ald8)();    /* Opcode 0x04 */
	void PREFIX86(_add_axd16)();    /* Opcode 0x05 */
	void PREFIX86(_push_es)();    /* Opcode 0x06 */

	void PREFIX86(_or_br8)();    /* Opcode 0x08 */
	void PREFIX86(_or_wr16)();    /* Opcode 0x09 */
	void PREFIX86(_or_r8b)();    /* Opcode 0x0a */
	void PREFIX86(_or_r16w)();    /* Opcode 0x0b */
	void PREFIX86(_or_ald8)();    /* Opcode 0x0c */
	void PREFIX86(_or_axd16)();    /* Opcode 0x0d */
	void PREFIX86(_push_cs)();    /* Opcode 0x0e */

	void PREFIX86(_adc_br8)();    /* Opcode 0x10 */
	void PREFIX86(_adc_wr16)();    /* Opcode 0x11 */
	void PREFIX86(_adc_r8b)();    /* Opcode 0x12 */
	void PREFIX86(_adc_r16w)();    /* Opcode 0x13 */
	void PREFIX86(_adc_ald8)();    /* Opcode 0x14 */
	void PREFIX86(_adc_axd16)();    /* Opcode 0x15 */
	
	void PREFIX86(_push_ss)();    /* Opcode 0x16 */
	void PREFIX86(_sbb_br8)();    /* Opcode 0x18 */
	void PREFIX86(_sbb_wr16)();    /* Opcode 0x19 */
	void PREFIX86(_sbb_r8b)();    /* Opcode 0x1a */
	void PREFIX86(_sbb_r16w)();    /* Opcode 0x1b */
	void PREFIX86(_sbb_ald8)();    /* Opcode 0x1c */
	void PREFIX86(_sbb_axd16)();    /* Opcode 0x1d */
	void PREFIX86(_push_ds)();    /* Opcode 0x1e */
	void PREFIX86(_pop_ds)();    /* Opcode 0x1f */
	void PREFIX86(_and_br8)();    /* Opcode 0x20 */
	void PREFIX86(_and_wr16)();    /* Opcode 0x21 */
	void PREFIX86(_and_r8b)();    /* Opcode 0x22 */
	void PREFIX86(_and_r16w)();    /* Opcode 0x23 */
	void PREFIX86(_and_ald8)();    /* Opcode 0x24 */
	void PREFIX86(_and_axd16)();    /* Opcode 0x25 */
	void PREFIX86(_daa)();    /* Opcode 0x27 */
	void PREFIX86(_sub_br8)();    /* Opcode 0x28 */
	void PREFIX86(_sub_wr16)();    /* Opcode 0x29 */
	void PREFIX86(_sub_r8b)();    /* Opcode 0x2a */
	void PREFIX86(_sub_r16w)();    /* Opcode 0x2b */
	void PREFIX86(_sub_ald8)();    /* Opcode 0x2c */
	void PREFIX86(_sub_axd16)();    /* Opcode 0x2d */
	void PREFIX86(_das)();    /* Opcode 0x2f */
	void PREFIX86(_xor_br8)();    /* Opcode 0x30 */
	void PREFIX86(_xor_wr16)();    /* Opcode 0x31 */
	void PREFIX86(_xor_r8b)();    /* Opcode 0x32 */
	void PREFIX86(_xor_r16w)();    /* Opcode 0x33 */
	void PREFIX86(_xor_ald8)();    /* Opcode 0x34 */
	void PREFIX86(_xor_axd16)();    /* Opcode 0x35 */
	void PREFIX86(_aaa)();    /* Opcode 0x37 */
	void PREFIX86(_cmp_br8)();    /* Opcode 0x38 */
	void PREFIX86(_cmp_wr16)();    /* Opcode 0x39 */
	void PREFIX86(_cmp_r8b)();    /* Opcode 0x3a */
	void PREFIX86(_cmp_r16w)();    /* Opcode 0x3b */
	void PREFIX86(_cmp_ald8)();    /* Opcode 0x3c */
	void PREFIX86(_cmp_axd16)();    /* Opcode 0x3d */
	void PREFIX86(_aas)();    /* Opcode 0x3f */
	void PREFIX86(_inc_ax)();    /* Opcode 0x40 */
	void PREFIX86(_inc_cx)();    /* Opcode 0x41 */
	void PREFIX86(_inc_dx)();    /* Opcode 0x42 */
	
	void PREFIX86(_inc_sp)();    /* Opcode 0x44 */
	void PREFIX86(_inc_bp)();    /* Opcode 0x45 */
	void PREFIX86(_inc_si)();    /* Opcode 0x46 */
	void PREFIX86(_inc_di)();    /* Opcode 0x47 */
	void PREFIX86(_dec_ax)();    /* Opcode 0x48 */
	void PREFIX86(_dec_cx)();    /* Opcode 0x49 */
	void PREFIX86(_dec_dx)();    /* Opcode 0x4a */
	void PREFIX86(_dec_bx)();    /* Opcode 0x4b */
	void PREFIX86(_dec_sp)();    /* Opcode 0x4c */
	void PREFIX86(_dec_bp)();    /* Opcode 0x4d */
	void PREFIX86(_dec_si)();    /* Opcode 0x4e */
	void PREFIX86(_dec_di)();    /* Opcode 0x4f */
	void PREFIX86(_push_ax)();    /* Opcode 0x50 */
	void PREFIX86(_push_cx)();    /* Opcode 0x51 */
	void PREFIX86(_push_dx)();    /* Opcode 0x52 */
	void PREFIX86(_push_bx)();    /* Opcode 0x53 */

	void PREFIX86(_push_bp)();    /* Opcode 0x55 */
	void PREFIX86(_push_si)();    /* Opcode 0x56 */
	void PREFIX86(_push_di)();    /* Opcode 0x57 */
	void PREFIX86(_pop_ax)();    /* Opcode 0x58 */
	void PREFIX86(_pop_cx)();    /* Opcode 0x59 */
	void PREFIX86(_pop_dx)();    /* Opcode 0x5a */
	void PREFIX86(_pop_bx)();    /* Opcode 0x5b */
	void PREFIX86(_pop_sp)();    /* Opcode 0x5c */
	void PREFIX86(_pop_bp)();    /* Opcode 0x5d */
	void PREFIX86(_pop_si)();    /* Opcode 0x5e */
	void PREFIX86(_pop_di)();    /* Opcode 0x5f */
	void PREFIX86(_jo)();    /* Opcode 0x70 */
	void PREFIX86(_jno)();    /* Opcode 0x71 */
	void PREFIX86(_jb)();    /* Opcode 0x72 */
	void PREFIX86(_jnb)();    /* Opcode 0x73 */
	void PREFIX86(_jz)();    /* Opcode 0x74 */
	void PREFIX86(_jnz)();    /* Opcode 0x75 */
	void PREFIX86(_jbe)();    /* Opcode 0x76 */
	void PREFIX86(_jnbe)();    /* Opcode 0x77 */
	void PREFIX86(_js)();    /* Opcode 0x78 */
	void PREFIX86(_jns)();    /* Opcode 0x79 */
	void PREFIX86(_jp)();    /* Opcode 0x7a */
	void PREFIX86(_jnp)();    /* Opcode 0x7b */
	void PREFIX86(_jl)();    /* Opcode 0x7c */
	void PREFIX86(_jnl)();    /* Opcode 0x7d */
	void PREFIX86(_jle)();    /* Opcode 0x7e */
	void PREFIX86(_jnle)();    /* Opcode 0x7f */
	void PREFIX86(_80pre)();    /* Opcode 0x80 */
	void PREFIX86(_81pre)();    /* Opcode 0x81 */
	void PREFIX86(_82pre)();  /* Opcode 0x82 */
	void PREFIX86(_83pre)();    /* Opcode 0x83 */
	void PREFIX86(_test_br8)();    /* Opcode 0x84 */
	void PREFIX86(_test_wr16)();    /* Opcode 0x85 */
	void PREFIX86(_xchg_br8)();    /* Opcode 0x86 */
	void PREFIX86(_xchg_wr16)();    /* Opcode 0x87 */
	void PREFIX86(_mov_br8)();    /* Opcode 0x88 */
	void PREFIX86(_mov_wr16)();    /* Opcode 0x89 */
	void PREFIX86(_mov_r8b)();    /* Opcode 0x8a */
	void PREFIX86(_mov_r16w)();    /* Opcode 0x8b */
	void PREFIX86(_mov_wsreg)();    /* Opcode 0x8c */
	void PREFIX86(_lea)();    /* Opcode 0x8d */
	void PREFIX86(_popw)();    /* Opcode 0x8f */
	void PREFIX86(_nop)();    /* Opcode 0x90 */
	void PREFIX86(_xchg_axcx)();    /* Opcode 0x91 */
	void PREFIX86(_xchg_axdx)();    /* Opcode 0x92 */
	void PREFIX86(_xchg_axbx)();    /* Opcode 0x93 */
	void PREFIX86(_xchg_axsp)();    /* Opcode 0x94 */
	void PREFIX86(_xchg_axbp)();    /* Opcode 0x95 */
	void PREFIX86(_xchg_axsi)();    /* Opcode 0x96 */
	void PREFIX86(_xchg_axdi)();    /* Opcode 0x97 */
	void PREFIX86(_cbw)();    /* Opcode 0x98 */
	void PREFIX86(_cwd)();    /* Opcode 0x99 */
	
	void PREFIX86(_sahf)();    /* Opcode 0x9e */
	void PREFIX86(_lahf)();    /* Opcode 0x9f */
	void PREFIX86(_mov_aldisp)();    /* Opcode 0xa0 */
	void PREFIX86(_mov_axdisp)();    /* Opcode 0xa1 */
	void PREFIX86(_mov_dispal)();    /* Opcode 0xa2 */
	void PREFIX86(_mov_dispax)();    /* Opcode 0xa3 */
	void PREFIX86(_movsb)();    /* Opcode 0xa4 */
	void PREFIX86(_movsw)();    /* Opcode 0xa5 */
	void PREFIX86(_cmpsb)();    /* Opcode 0xa6 */
	void PREFIX86(_cmpsw)();    /* Opcode 0xa7 */
	void PREFIX86(_test_ald8)();    /* Opcode 0xa8 */
	void PREFIX86(_test_axd16)();    /* Opcode 0xa9 */
	void PREFIX86(_stosb)();    /* Opcode 0xaa */
	void PREFIX86(_stosw)();    /* Opcode 0xab */
	void PREFIX86(_lodsb)();    /* Opcode 0xac */
	void PREFIX86(_lodsw)();    /* Opcode 0xad */
	void PREFIX86(_scasb)();    /* Opcode 0xae */
	void PREFIX86(_scasw)();    /* Opcode 0xaf */
	void PREFIX86(_mov_ald8)();    /* Opcode 0xb0 */
	void PREFIX86(_mov_cld8)();    /* Opcode 0xb1 */
	void PREFIX86(_mov_dld8)();    /* Opcode 0xb2 */
	void PREFIX86(_mov_bld8)();    /* Opcode 0xb3 */
	void PREFIX86(_mov_ahd8)();    /* Opcode 0xb4 */
	void PREFIX86(_mov_chd8)();    /* Opcode 0xb5 */
	void PREFIX86(_mov_dhd8)();    /* Opcode 0xb6 */
	void PREFIX86(_mov_bhd8)();    /* Opcode 0xb7 */
	void PREFIX86(_mov_axd16)();    /* Opcode 0xb8 */
	void PREFIX86(_mov_cxd16)();    /* Opcode 0xb9 */
	void PREFIX86(_mov_dxd16)();    /* Opcode 0xba */
	void PREFIX86(_mov_bxd16)();    /* Opcode 0xbb */
	void PREFIX86(_mov_spd16)();    /* Opcode 0xbc */
	void PREFIX86(_mov_bpd16)();    /* Opcode 0xbd */
	void PREFIX86(_mov_sid16)();    /* Opcode 0xbe */
	void PREFIX86(_mov_did16)();    /* Opcode 0xbf */
	void PREFIX86(_ret_d16)();    /* Opcode 0xc2 */
	void PREFIX86(_ret)();    /* Opcode 0xc3 */
	
	void PREFIX86(_mov_bd8)();    /* Opcode 0xc6 */
	void PREFIX86(_mov_wd16)();    /* Opcode 0xc7 */
	
	void PREFIX86(_rotshft_b)();    /* Opcode 0xd0 */
	void PREFIX86(_rotshft_w)();    /* Opcode 0xd1 */
	
	void PREFIX86(_aad)();    /* Opcode 0xd5 */
	void PREFIX86(_xlat)();    /* Opcode 0xd7 */

	void PREFIX86(_loopne)();    /* Opcode 0xe0 */
	void PREFIX86(_loope)();    /* Opcode 0xe1 */
	void PREFIX86(_loop)();    /* Opcode 0xe2 */
	void PREFIX86(_jcxz)();    /* Opcode 0xe3 */
	
	void PREFIX86(_jmp_d16)();    /* Opcode 0xe9 */
	void PREFIX86(_jmp_far)();    /* Opcode 0xea */
	void PREFIX86(_jmp_d8)();    /* Opcode 0xeb */
	
	void PREFIX86(_cmc)();    /* Opcode 0xf5 */
	void PREFIX86(_f6pre)();
	void PREFIX86(_f7pre)();
	void PREFIX86(_clc)();    /* Opcode 0xf8 */
	void PREFIX86(_stc)();    /* Opcode 0xf9 */

	void PREFIX86(_cld)();    /* Opcode 0xfc */
	void PREFIX86(_std)();    /* Opcode 0xfd */
	void PREFIX86(_fepre)();    /* Opcode 0xfe */

	// PREFIX()
	void PREFIX86(_pushf)();    /* Opcode 0x9c */
	void PREFIX186(_pushf)();    /* Opcode 0x9c */
	void PREFIX286(_pushf)();    /* Opcode 0x9c */
	void PREFIXV30(_pushf)();    /* Opcode 0x9c */

	
	void PREFIX86(_pop_ss)();    /* Opcode 0x17 */
	void PREFIX86(_mov_sregw)();    /* Opcode 0x8e */
	void PREFIX86(_sti)();    /* Opcode 0xfb */
	
	void PREFIX186(_pop_ss)();    /* Opcode 0x17 */
	void PREFIX186(_mov_sregw)();    /* Opcode 0x8e */
	void PREFIX186(_sti)();    /* Opcode 0xfb */
	
	void PREFIX286(_pop_ss)();    /* Opcode 0x17 */
	void PREFIX286(_mov_sregw)();    /* Opcode 0x8e */
	void PREFIX286(_sti)();    /* Opcode 0xfb */
	
	void PREFIXV30(_pop_ss)();    /* Opcode 0x17 */
	void PREFIXV30(_mov_sregw)();    /* Opcode 0x8e */
	void PREFIXV30(_sti)();    /* Opcode 0xfb */
	
	void PREFIX86(_es)();    /* Opcode 0x26 */
	void PREFIX86(_cs)();    /* Opcode 0x2e */
	void PREFIX86(_ss)();    /* Opcode 0x36 */
	void PREFIX86(_ds)();    /* Opcode 0x3e */
	void PREFIX86(_inc_bx)();    /* Opcode 0x43 */
	void PREFIX86(_repne)();    /* Opcode 0xf2 */
	void PREFIX86(_repe)();    /* Opcode 0xf3 */

	void PREFIX186(_es)();    /* Opcode 0x26 */
	void PREFIX186(_cs)();    /* Opcode 0x2e */
	void PREFIX186(_ss)();    /* Opcode 0x36 */
	void PREFIX186(_ds)();    /* Opcode 0x3e */
	void PREFIX186(_inc_bx)();    /* Opcode 0x43 */
	void PREFIX186(_repne)();    /* Opcode 0xf2 */
	void PREFIX186(_repe)();    /* Opcode 0xf3 */

	void PREFIX286(_es)();    /* Opcode 0x26 */
	void PREFIX286(_cs)();    /* Opcode 0x2e */
	void PREFIX286(_ss)();    /* Opcode 0x36 */
	void PREFIX286(_ds)();    /* Opcode 0x3e */
	void PREFIX286(_inc_bx)();    /* Opcode 0x43 */
	void PREFIX286(_repne)();    /* Opcode 0xf2 */
	void PREFIX286(_repe)();    /* Opcode 0xf3 */

	void PREFIXV30(_es)();    /* Opcode 0x26 */
	void PREFIXV30(_cs)();    /* Opcode 0x2e */
	void PREFIXV30(_ss)();    /* Opcode 0x36 */
	void PREFIXV30(_ds)();    /* Opcode 0x3e */
	void PREFIXV30(_inc_bx)();    /* Opcode 0x43 */
	void PREFIXV30(_repne)();    /* Opcode 0xf2 */
	void PREFIXV30(_repe)();    /* Opcode 0xf3 */

protected:
	void i80286_trap2(UINT32 error);
	void i80286_pop_seg(int reg);
	void i80286_data_descriptor_full(int reg, UINT16 selector, int cpl, UINT32 trap, UINT16 offset, int size);
	void i80286_data_descriptor(int reg, UINT16 selector);
	void i80286_switch_task(UINT16 ntask, int type);
	void i80286_code_descriptor(UINT16 selector, UINT16 offset, int gate);
	void i80286_interrupt_descriptor(UINT16 number, int hwint, int error);
	void i80286_load_flags(UINT16 flags, int cpl);
	void i80286_check_permission(UINT8 check_seg, UINT32 offset, UINT16 size, i80286_operation operation);

	void PREFIX86(_interrupt)(unsigned int_num);
	void PREFIX86(_trap)();
	
	void PREFIX186(_interrupt)(unsigned int_num);
	void PREFIX186(_trap)();
	void PREFIX286(_interrupt)(unsigned int_num);
	void PREFIX286(_trap)();
	
	void PREFIXV30(_interrupt)(unsigned int_num);
	void PREFIXV30(_trap)();

	void PREFIX86(rep)(int flagval);
	void PREFIX186(rep)(int flagval);
	void PREFIX286(rep)(int flagval);
	void PREFIXV30(rep)(int flagval);
public:
};
extern const struct i80x86_timing i8086_cycles;
extern const struct i80x86_timing i80186_cycles;
extern const struct i80x86_timing i80286_cycles;
extern unsigned (IX86_OPS_BASE::*GetEA[192])();

#endif
