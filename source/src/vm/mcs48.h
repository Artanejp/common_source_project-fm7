/*
	Skelton for retropc emulator

	Origin : MAME 0.148
	Author : Takeda.Toshiya
	Date   : 2013.05.01-

	[ MCS48 ]
*/

#ifndef _MCS84_H_ 
#define _MCS48_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#include "mcs48_flags.h"
#ifndef INLINE
#define INLINE inline
#endif

#define MCS48_PORT_P0	0x100	/* Not used */
#define MCS48_PORT_P1	0x101	/* P10-P17 */
#define MCS48_PORT_P2	0x102	/* P20-P28 */
#define MCS48_PORT_T0	0x110
#define MCS48_PORT_T1	0x111
#define MCS48_PORT_BUS	0x120	/* DB0-DB7 */
#define MCS48_PORT_PROG	0x121	/* PROG line to 8243 expander */

//#ifdef USE_DEBUGGER
class DEBUGGER;
//#endif

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* timer/counter enable bits */
#define __MCS48_TIMER_ENABLED   0x01
#define __MCS48_COUNTER_ENABLED 0x02

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* live processor state */
struct mcs48_state
{
	UINT16      prevpc;             /* 16-bit previous program counter */
	UINT16      pc;                 /* 16-bit program counter */

	UINT8       a;                  /* 8-bit accumulator */
	int         regptr;             /* offset of r0-r7 */
	UINT8       psw;                /* 8-bit cpustate->psw */
	UINT8       p1;                 /* 8-bit latched port 1 */
	UINT8       p2;                 /* 8-bit latched port 2 */
	UINT8       timer;              /* 8-bit timer */
	UINT8       prescaler;          /* 5-bit timer prescaler */
	UINT8       t1_history;         /* 8-bit history of the T1 input */
	UINT8       sts;                /* 8-bit status register */

	UINT8       int_state;          /* INT signal status */
	UINT8       irq_state;          /* TRUE if an IRQ is pending */
	UINT8       irq_in_progress;    /* TRUE if an IRQ is in progress */
	UINT8       timer_overflow;     /* TRUE on a timer overflow; cleared by taking interrupt */
	UINT8       timer_flag;         /* TRUE on a timer overflow; cleared on JTF */
	UINT8       tirq_enabled;       /* TRUE if the timer IRQ is enabled */
	UINT8       xirq_enabled;       /* TRUE if the external IRQ is enabled */
	UINT8       t0_clk_enabled;     /* TRUE if ent0_clk is called */
	UINT8       timecount_enabled;  /* bitmask of timer/counter enabled */

	UINT16      a11;                /* A11 value, either 0x000 or 0x800 */

	DEVICE *    mem;
	DEVICE *    io;
	DEVICE *    intr;

	int         icount;

	UINT8       rom[0x1000];
//	UINT8       ram[0x100];
};
/***************************************************************************
    MACROS
***************************************************************************/

#define __mcs48_program_r(a)    cpustate->rom[(a) & 0xfff]

#define __mcs48_ram_r(a)        cpustate->mem->read_data8(a)
#define __mcs48_ram_w(a,V)      cpustate->mem->write_data8(a, V)
#define __mcs48_reg_r(a)        cpustate->mem->read_data8(cpustate->regptr + a)
#define __mcs48_reg_w(a,V)      cpustate->mem->write_data8(cpustate->regptr + a, V)

#define __mcs48_ext_r(a)        cpustate->io->read_io8(a)
#define __mcs48_ext_w(a,V)      cpustate->io->write_io8(a, V)
#define __mcs48_port_r(a)       cpustate->io->read_io8(MCS48_PORT_P0 + a)
#define __mcs48_port_w(a,V)     cpustate->io->write_io8(MCS48_PORT_P0 + a, V)
#define __mcs48_test_r(a)       cpustate->io->read_io8(MCS48_PORT_T0 + a)
#define __mcs48_test_w(a,V)     cpustate->io->write_io8(MCS48_PORT_T0 + a, V)
#define __mcs48_bus_r()         cpustate->io->read_io8(MCS48_PORT_BUS)
#define __mcs48_bus_w(V)        cpustate->io->write_io8(MCS48_PORT_BUS, V)
#define __mcs48_prog_w(V)       cpustate->io->write_io8(MCS48_PORT_PROG, V)



#define __MCS48_OPHANDLER(_name) int MCS48_BASE::_name(mcs48_state *cpustate)
#define __MCS48_OPHANDLER_D(_name) int __FASTCALL _name(mcs48_state *cpustate)

class MCS48MEM : public DEVICE
{
private:
	uint8_t ram[0x100];
public:
	MCS48MEM(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		memset(ram, 0, sizeof(ram));
		set_device_name(_T("MCS48 MEMORY BUS"));
	}
	~MCS48MEM() {}
	
	uint32_t __FASTCALL read_data8(uint32_t addr)
	{
		return ram[addr & 0xff];
	}
	void __FASTCALL write_data8(uint32_t addr, uint32_t data)
	{
		ram[addr & 0xff] = data;
	}
	bool process_state(FILEIO* state_fio, bool loading);
};

class MCS48_BASE : public DEVICE
{
protected:
	inline UINT8 __FASTCALL argument_fetch(mcs48_state *cpustate);
	inline void __FASTCALL pull_pc(mcs48_state *cpustate);
	inline void __FASTCALL execute_add(mcs48_state *cpustate, UINT8 dat);
	inline void __FASTCALL execute_addc(mcs48_state *cpustate, UINT8 dat);
	inline void __FASTCALL execute_jmp(mcs48_state *cpustate, UINT16 address);
	inline void __FASTCALL execute_call(mcs48_state *cpustate, UINT16 address);
	inline void __FASTCALL execute_jcc(mcs48_state *cpustate, UINT8 result);
	inline void __FASTCALL expander_operation(mcs48_state *cpustate, UINT8 operation, UINT8 port);

	uint64_t total_icount;
	uint64_t prev_total_icount;

private:
	__MCS48_OPHANDLER_D( illegal );
	__MCS48_OPHANDLER_D( add_a_r0 );
	__MCS48_OPHANDLER_D( add_a_r1 );
	__MCS48_OPHANDLER_D( add_a_r2 );
	__MCS48_OPHANDLER_D( add_a_r3 );
	__MCS48_OPHANDLER_D( add_a_r4 );
	__MCS48_OPHANDLER_D( add_a_r5 );
	__MCS48_OPHANDLER_D( add_a_r6 );
	__MCS48_OPHANDLER_D( add_a_r7 );
	__MCS48_OPHANDLER_D( add_a_xr0 );
	__MCS48_OPHANDLER_D( add_a_xr1 );
	__MCS48_OPHANDLER_D( add_a_n );

	__MCS48_OPHANDLER_D( adc_a_r0 );
	__MCS48_OPHANDLER_D( adc_a_r1 );
	__MCS48_OPHANDLER_D( adc_a_r2 );
	__MCS48_OPHANDLER_D( adc_a_r3 );
	__MCS48_OPHANDLER_D( adc_a_r4 );
	__MCS48_OPHANDLER_D( adc_a_r5 );
	__MCS48_OPHANDLER_D( adc_a_r6 );
	__MCS48_OPHANDLER_D( adc_a_r7 );
	__MCS48_OPHANDLER_D( adc_a_xr0 );
	__MCS48_OPHANDLER_D( adc_a_xr1 );
	__MCS48_OPHANDLER_D( adc_a_n );  

	__MCS48_OPHANDLER_D( anl_a_r0 );
	__MCS48_OPHANDLER_D( anl_a_r1 );
	__MCS48_OPHANDLER_D( anl_a_r2 );
	__MCS48_OPHANDLER_D( anl_a_r3 );
	__MCS48_OPHANDLER_D( anl_a_r4 );
	__MCS48_OPHANDLER_D( anl_a_r5 );
	__MCS48_OPHANDLER_D( anl_a_r6 );
	__MCS48_OPHANDLER_D( anl_a_r7 );
	__MCS48_OPHANDLER_D( anl_a_xr0 );
	__MCS48_OPHANDLER_D( anl_a_xr1 );
	__MCS48_OPHANDLER_D( anl_a_n );
	
	__MCS48_OPHANDLER_D( anl_bus_n );
	__MCS48_OPHANDLER_D( anl_p1_n );
	__MCS48_OPHANDLER_D( anl_p2_n );
	__MCS48_OPHANDLER_D( anld_p4_a );
	__MCS48_OPHANDLER_D( anld_p5_a );
	__MCS48_OPHANDLER_D( anld_p6_a );
	__MCS48_OPHANDLER_D( anld_p7_a );

	__MCS48_OPHANDLER_D( call_0 );
	__MCS48_OPHANDLER_D( call_1 );
	__MCS48_OPHANDLER_D( call_2 );
	__MCS48_OPHANDLER_D( call_3 );
	__MCS48_OPHANDLER_D( call_4 );
	__MCS48_OPHANDLER_D( call_5 );
	__MCS48_OPHANDLER_D( call_6 );
	__MCS48_OPHANDLER_D( call_7 );

	__MCS48_OPHANDLER_D( clr_a );
	__MCS48_OPHANDLER_D( clr_c );
	__MCS48_OPHANDLER_D( clr_f0 );
	__MCS48_OPHANDLER_D( clr_f1 );

	__MCS48_OPHANDLER_D( cpl_a );
	__MCS48_OPHANDLER_D( cpl_c );
	__MCS48_OPHANDLER_D( cpl_f0 );
	__MCS48_OPHANDLER_D( cpl_f1 );

	__MCS48_OPHANDLER_D( da_a );

	__MCS48_OPHANDLER_D( dec_a );
	__MCS48_OPHANDLER_D( dec_r0 );
	__MCS48_OPHANDLER_D( dec_r1 );
	__MCS48_OPHANDLER_D( dec_r2 );
	__MCS48_OPHANDLER_D( dec_r3 );
	__MCS48_OPHANDLER_D( dec_r4 );
	__MCS48_OPHANDLER_D( dec_r5 );
	__MCS48_OPHANDLER_D( dec_r6 );
	__MCS48_OPHANDLER_D( dec_r7 );

	__MCS48_OPHANDLER_D( dis_i );
	__MCS48_OPHANDLER_D( dis_tcnti );

	__MCS48_OPHANDLER_D( djnz_r0 );
	__MCS48_OPHANDLER_D( djnz_r1 );
	__MCS48_OPHANDLER_D( djnz_r2 );
	__MCS48_OPHANDLER_D( djnz_r3 );
	__MCS48_OPHANDLER_D( djnz_r4 );
	__MCS48_OPHANDLER_D( djnz_r5 );
	__MCS48_OPHANDLER_D( djnz_r6 );
	__MCS48_OPHANDLER_D( djnz_r7 );

	__MCS48_OPHANDLER_D( en_i );
	__MCS48_OPHANDLER_D( en_tcnti );
	__MCS48_OPHANDLER_D( ent0_clk );

	__MCS48_OPHANDLER_D( in_a_p1 );
	__MCS48_OPHANDLER_D( in_a_p2 );
	__MCS48_OPHANDLER_D( ins_a_bus );

	__MCS48_OPHANDLER_D( inc_a );
	__MCS48_OPHANDLER_D( inc_r0 );
	__MCS48_OPHANDLER_D( inc_r1 );
	__MCS48_OPHANDLER_D( inc_r2 );
	__MCS48_OPHANDLER_D( inc_r3 );
	__MCS48_OPHANDLER_D( inc_r4 );
	__MCS48_OPHANDLER_D( inc_r5 );
	__MCS48_OPHANDLER_D( inc_r6 );
	__MCS48_OPHANDLER_D( inc_r7 );
	__MCS48_OPHANDLER_D( inc_xr0 );
	__MCS48_OPHANDLER_D( inc_xr1 );

	__MCS48_OPHANDLER_D( jb_0 );
	__MCS48_OPHANDLER_D( jb_1 );
	__MCS48_OPHANDLER_D( jb_2 );
	__MCS48_OPHANDLER_D( jb_3 );
	__MCS48_OPHANDLER_D( jb_4 );
	__MCS48_OPHANDLER_D( jb_5 );
	__MCS48_OPHANDLER_D( jb_6 );
	__MCS48_OPHANDLER_D( jb_7 );
	__MCS48_OPHANDLER_D( jc );
	__MCS48_OPHANDLER_D( jf0 );
	__MCS48_OPHANDLER_D( jf1 );
	__MCS48_OPHANDLER_D( jnc );
	__MCS48_OPHANDLER_D( jni );
	__MCS48_OPHANDLER_D( jnt_0 );
	__MCS48_OPHANDLER_D( jnt_1 );
	__MCS48_OPHANDLER_D( jnz );
	__MCS48_OPHANDLER_D( jtf );
	__MCS48_OPHANDLER_D( jt_0 );
	__MCS48_OPHANDLER_D( jt_1 );
	__MCS48_OPHANDLER_D( jz );

	__MCS48_OPHANDLER_D( jmp_0 );
	__MCS48_OPHANDLER_D( jmp_1 );
	__MCS48_OPHANDLER_D( jmp_2 );
	__MCS48_OPHANDLER_D( jmp_3 );
	__MCS48_OPHANDLER_D( jmp_4 );
	__MCS48_OPHANDLER_D( jmp_5 );
	__MCS48_OPHANDLER_D( jmp_6 );
	__MCS48_OPHANDLER_D( jmp_7 );
	__MCS48_OPHANDLER_D( jmpp_xa );

	__MCS48_OPHANDLER_D( mov_a_n );
	__MCS48_OPHANDLER_D( mov_a_psw );
	__MCS48_OPHANDLER_D( mov_a_r0 );
	__MCS48_OPHANDLER_D( mov_a_r1 );
	__MCS48_OPHANDLER_D( mov_a_r2 );
	__MCS48_OPHANDLER_D( mov_a_r3 );
	__MCS48_OPHANDLER_D( mov_a_r4 );
	__MCS48_OPHANDLER_D( mov_a_r5 );
	__MCS48_OPHANDLER_D( mov_a_r6 );
	__MCS48_OPHANDLER_D( mov_a_r7 );
	__MCS48_OPHANDLER_D( mov_a_xr0 );
	__MCS48_OPHANDLER_D( mov_a_xr1 );
	__MCS48_OPHANDLER_D( mov_a_t );

	__MCS48_OPHANDLER_D( mov_psw_a );
	__MCS48_OPHANDLER_D( mov_r0_a );
	__MCS48_OPHANDLER_D( mov_r1_a );
	__MCS48_OPHANDLER_D( mov_r2_a );
	__MCS48_OPHANDLER_D( mov_r3_a );
	__MCS48_OPHANDLER_D( mov_r4_a );
	__MCS48_OPHANDLER_D( mov_r5_a );
	__MCS48_OPHANDLER_D( mov_r6_a );
	__MCS48_OPHANDLER_D( mov_r7_a );
	__MCS48_OPHANDLER_D( mov_r0_n );
	__MCS48_OPHANDLER_D( mov_r1_n );
	__MCS48_OPHANDLER_D( mov_r2_n );
	__MCS48_OPHANDLER_D( mov_r3_n );
	__MCS48_OPHANDLER_D( mov_r4_n );
	__MCS48_OPHANDLER_D( mov_r5_n );
	__MCS48_OPHANDLER_D( mov_r6_n );
	__MCS48_OPHANDLER_D( mov_r7_n );
	__MCS48_OPHANDLER_D( mov_t_a );
	__MCS48_OPHANDLER_D( mov_xr0_a );
	__MCS48_OPHANDLER_D( mov_xr1_a );
	__MCS48_OPHANDLER_D( mov_xr0_n );
	__MCS48_OPHANDLER_D( mov_xr1_n );

	__MCS48_OPHANDLER_D( movd_a_p4 );
	__MCS48_OPHANDLER_D( movd_a_p5 );
	__MCS48_OPHANDLER_D( movd_a_p6 );
	__MCS48_OPHANDLER_D( movd_a_p7 );
	__MCS48_OPHANDLER_D( movd_p4_a );
	__MCS48_OPHANDLER_D( movd_p5_a );
	__MCS48_OPHANDLER_D( movd_p6_a );
	__MCS48_OPHANDLER_D( movd_p7_a );

	__MCS48_OPHANDLER_D( movp_a_xa );
	__MCS48_OPHANDLER_D( movp3_a_xa );

	__MCS48_OPHANDLER_D( movx_a_xr0 );
	__MCS48_OPHANDLER_D( movx_a_xr1 );
	__MCS48_OPHANDLER_D( movx_xr0_a );
	__MCS48_OPHANDLER_D( movx_xr1_a );

	__MCS48_OPHANDLER_D( nop );

	__MCS48_OPHANDLER_D( orl_a_r0 );
	__MCS48_OPHANDLER_D( orl_a_r1 );
	__MCS48_OPHANDLER_D( orl_a_r2 );
	__MCS48_OPHANDLER_D( orl_a_r3 );
	__MCS48_OPHANDLER_D( orl_a_r4 );
	__MCS48_OPHANDLER_D( orl_a_r5 );
	__MCS48_OPHANDLER_D( orl_a_r6 );
	__MCS48_OPHANDLER_D( orl_a_r7 );
	__MCS48_OPHANDLER_D( orl_a_xr0 );
	__MCS48_OPHANDLER_D( orl_a_xr1 );
	__MCS48_OPHANDLER_D( orl_a_n );

	__MCS48_OPHANDLER_D( orl_bus_n );
	__MCS48_OPHANDLER_D( orl_p1_n );
	__MCS48_OPHANDLER_D( orl_p2_n );
	__MCS48_OPHANDLER_D( orld_p4_a );
	__MCS48_OPHANDLER_D( orld_p5_a );
	__MCS48_OPHANDLER_D( orld_p6_a );
	__MCS48_OPHANDLER_D( orld_p7_a );

	__MCS48_OPHANDLER_D( outl_bus_a );
	__MCS48_OPHANDLER_D( outl_p1_a );
	__MCS48_OPHANDLER_D( outl_p2_a );

	__MCS48_OPHANDLER_D( ret );
	__MCS48_OPHANDLER_D( retr );

	__MCS48_OPHANDLER_D( rl_a );
	__MCS48_OPHANDLER_D( rlc_a );

	__MCS48_OPHANDLER_D( rr_a );
	__MCS48_OPHANDLER_D( rrc_a );

	__MCS48_OPHANDLER_D( sel_mb0 );
	__MCS48_OPHANDLER_D( sel_mb1 );

	__MCS48_OPHANDLER_D( sel_rb0 );
	__MCS48_OPHANDLER_D( sel_rb1 );

	__MCS48_OPHANDLER_D( stop_tcnt );

	__MCS48_OPHANDLER_D( strt_cnt );
	__MCS48_OPHANDLER_D( strt_t );

	__MCS48_OPHANDLER_D( swap_a );

	__MCS48_OPHANDLER_D( xch_a_r0 );
	__MCS48_OPHANDLER_D( xch_a_r1 );
	__MCS48_OPHANDLER_D( xch_a_r2 );
	__MCS48_OPHANDLER_D( xch_a_r3 );
	__MCS48_OPHANDLER_D( xch_a_r4 );
	__MCS48_OPHANDLER_D( xch_a_r5 );
	__MCS48_OPHANDLER_D( xch_a_r6 );
	__MCS48_OPHANDLER_D( xch_a_r7 );
	__MCS48_OPHANDLER_D( xch_a_xr0 );
	__MCS48_OPHANDLER_D( xch_a_xr1 );

	__MCS48_OPHANDLER_D( xchd_a_xr0 );
	__MCS48_OPHANDLER_D( xchd_a_xr1 );

	__MCS48_OPHANDLER_D( xrl_a_r0 );
	__MCS48_OPHANDLER_D( xrl_a_r1 );
	__MCS48_OPHANDLER_D( xrl_a_r2 );
	__MCS48_OPHANDLER_D( xrl_a_r3 );
	__MCS48_OPHANDLER_D( xrl_a_r4 );
	__MCS48_OPHANDLER_D( xrl_a_r5 );
	__MCS48_OPHANDLER_D( xrl_a_r6 );
	__MCS48_OPHANDLER_D( xrl_a_r7 );
	__MCS48_OPHANDLER_D( xrl_a_xr0 );
	__MCS48_OPHANDLER_D( xrl_a_xr1 );
	__MCS48_OPHANDLER_D( xrl_a_n );

protected:
	/* ---------------------------------------------------------------------------
	contexts
	--------------------------------------------------------------------------- */
	
	DEVICE *d_mem, *d_io, *d_intr;
//#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
	DEVICE *d_mem_stored, *d_io_stored;
//#endif
	void *opaque;
	/* opcode table entry */
	typedef int (__FASTCALL  MCS48_BASE::*mcs48_ophandler)(mcs48_state *state);

	const mcs48_ophandler opcode_table[256] =
	{
		&MCS48_BASE::nop,        &MCS48_BASE::illegal,     &MCS48_BASE::outl_bus_a, &MCS48_BASE::add_a_n,   &MCS48_BASE::jmp_0,     &MCS48_BASE::en_i,      &MCS48_BASE::illegal,   &MCS48_BASE::dec_a,         /* 00 */
		&MCS48_BASE::ins_a_bus,  &MCS48_BASE::in_a_p1,     &MCS48_BASE::in_a_p2,    &MCS48_BASE::illegal,   &MCS48_BASE::movd_a_p4, &MCS48_BASE::movd_a_p5, &MCS48_BASE::movd_a_p6, &MCS48_BASE::movd_a_p7,
		&MCS48_BASE::inc_xr0,    &MCS48_BASE::inc_xr1,     &MCS48_BASE::jb_0,       &MCS48_BASE::adc_a_n,   &MCS48_BASE::call_0,    &MCS48_BASE::dis_i,     &MCS48_BASE::jtf,       &MCS48_BASE::inc_a,         /* 10 */
		&MCS48_BASE::inc_r0,     &MCS48_BASE::inc_r1,      &MCS48_BASE::inc_r2,     &MCS48_BASE::inc_r3,    &MCS48_BASE::inc_r4,    &MCS48_BASE::inc_r5,    &MCS48_BASE::inc_r6,    &MCS48_BASE::inc_r7,
		&MCS48_BASE::xch_a_xr0,  &MCS48_BASE::xch_a_xr1,   &MCS48_BASE::illegal,    &MCS48_BASE::mov_a_n,   &MCS48_BASE::jmp_1,     &MCS48_BASE::en_tcnti,  &MCS48_BASE::jnt_0,     &MCS48_BASE::clr_a,         /* 20 */
		&MCS48_BASE::xch_a_r0,   &MCS48_BASE::xch_a_r1,    &MCS48_BASE::xch_a_r2,   &MCS48_BASE::xch_a_r3,  &MCS48_BASE::xch_a_r4,  &MCS48_BASE::xch_a_r5,  &MCS48_BASE::xch_a_r6,  &MCS48_BASE::xch_a_r7,
		&MCS48_BASE::xchd_a_xr0, &MCS48_BASE::xchd_a_xr1, &MCS48_BASE::jb_1,       &MCS48_BASE::illegal,   &MCS48_BASE::call_1,    &MCS48_BASE::dis_tcnti, &MCS48_BASE::jt_0,     &MCS48_BASE::cpl_a,         /* 30 */
		&MCS48_BASE::illegal,    &MCS48_BASE::outl_p1_a,   &MCS48_BASE::outl_p2_a,  &MCS48_BASE::illegal,   &MCS48_BASE::movd_p4_a, &MCS48_BASE::movd_p5_a, &MCS48_BASE::movd_p6_a, &MCS48_BASE::movd_p7_a,
		&MCS48_BASE::orl_a_xr0,  &MCS48_BASE::orl_a_xr1,   &MCS48_BASE::mov_a_t,    &MCS48_BASE::orl_a_n,   &MCS48_BASE::jmp_2,     &MCS48_BASE::strt_cnt,  &MCS48_BASE::jnt_1,     &MCS48_BASE::swap_a,        /* 40 */
		&MCS48_BASE::orl_a_r0,   &MCS48_BASE::orl_a_r1,    &MCS48_BASE::orl_a_r2,   &MCS48_BASE::orl_a_r3,  &MCS48_BASE::orl_a_r4,  &MCS48_BASE::orl_a_r5,  &MCS48_BASE::orl_a_r6,  &MCS48_BASE::orl_a_r7,
		&MCS48_BASE::anl_a_xr0,  &MCS48_BASE::anl_a_xr1,   &MCS48_BASE::jb_2,       &MCS48_BASE::anl_a_n,   &MCS48_BASE::call_2,    &MCS48_BASE::strt_t,    &MCS48_BASE::jt_1,      &MCS48_BASE::da_a,          /* 50 */
		&MCS48_BASE::anl_a_r0,   &MCS48_BASE::anl_a_r1,    &MCS48_BASE::anl_a_r2,   &MCS48_BASE::anl_a_r3,  &MCS48_BASE::anl_a_r4,  &MCS48_BASE::anl_a_r5,  &MCS48_BASE::anl_a_r6,  &MCS48_BASE::anl_a_r7,
		&MCS48_BASE::add_a_xr0,  &MCS48_BASE::add_a_xr1,   &MCS48_BASE::mov_t_a,    &MCS48_BASE::illegal,   &MCS48_BASE::jmp_3,     &MCS48_BASE::stop_tcnt, &MCS48_BASE::illegal,   &MCS48_BASE::rrc_a,         /* 60 */
		&MCS48_BASE::add_a_r0,   &MCS48_BASE::add_a_r1,    &MCS48_BASE::add_a_r2,   &MCS48_BASE::add_a_r3,  &MCS48_BASE::add_a_r4,  &MCS48_BASE::add_a_r5,  &MCS48_BASE::add_a_r6,  &MCS48_BASE::add_a_r7,
		&MCS48_BASE::adc_a_xr0,  &MCS48_BASE::adc_a_xr1,   &MCS48_BASE::jb_3,       &MCS48_BASE::illegal,   &MCS48_BASE::call_3,    &MCS48_BASE::ent0_clk,  &MCS48_BASE::jf1,       &MCS48_BASE::rr_a,          /* 70 */
		&MCS48_BASE::adc_a_r0,   &MCS48_BASE::adc_a_r1,    &MCS48_BASE::adc_a_r2,   &MCS48_BASE::adc_a_r3,  &MCS48_BASE::adc_a_r4,  &MCS48_BASE::adc_a_r5,  &MCS48_BASE::adc_a_r6,  &MCS48_BASE::adc_a_r7,
		&MCS48_BASE::movx_a_xr0, &MCS48_BASE::movx_a_xr1,  &MCS48_BASE::illegal,    &MCS48_BASE::ret,       &MCS48_BASE::jmp_4,     &MCS48_BASE::clr_f0,    &MCS48_BASE::jni,       &MCS48_BASE::illegal,       /* 80 */
		&MCS48_BASE::orl_bus_n,  &MCS48_BASE::orl_p1_n,    &MCS48_BASE::orl_p2_n,   &MCS48_BASE::illegal,   &MCS48_BASE::orld_p4_a, &MCS48_BASE::orld_p5_a, &MCS48_BASE::orld_p6_a, &MCS48_BASE::orld_p7_a,
		&MCS48_BASE::movx_xr0_a, &MCS48_BASE::movx_xr1_a,  &MCS48_BASE::jb_4,       &MCS48_BASE::retr,      &MCS48_BASE::call_4,    &MCS48_BASE::cpl_f0,    &MCS48_BASE::jnz,       &MCS48_BASE::clr_c,         /* 90 */
		&MCS48_BASE::anl_bus_n,  &MCS48_BASE::anl_p1_n,    &MCS48_BASE::anl_p2_n,   &MCS48_BASE::illegal,   &MCS48_BASE::anld_p4_a, &MCS48_BASE::anld_p5_a, &MCS48_BASE::anld_p6_a, &MCS48_BASE::anld_p7_a,
		&MCS48_BASE::mov_xr0_a,  &MCS48_BASE::mov_xr1_a,   &MCS48_BASE::illegal,    &MCS48_BASE::movp_a_xa, &MCS48_BASE::jmp_5,     &MCS48_BASE::clr_f1,    &MCS48_BASE::illegal,   &MCS48_BASE::cpl_c,         /* A0 */
		&MCS48_BASE::mov_r0_a,   &MCS48_BASE::mov_r1_a,    &MCS48_BASE::mov_r2_a,   &MCS48_BASE::mov_r3_a,  &MCS48_BASE::mov_r4_a,  &MCS48_BASE::mov_r5_a,  &MCS48_BASE::mov_r6_a,  &MCS48_BASE::mov_r7_a,
		&MCS48_BASE::mov_xr0_n,  &MCS48_BASE::mov_xr1_n,   &MCS48_BASE::jb_5,       &MCS48_BASE::jmpp_xa,   &MCS48_BASE::call_5,    &MCS48_BASE::cpl_f1,    &MCS48_BASE::jf0,       &MCS48_BASE::illegal,       /* B0 */
		&MCS48_BASE::mov_r0_n,   &MCS48_BASE::mov_r1_n,    &MCS48_BASE::mov_r2_n,   &MCS48_BASE::mov_r3_n,  &MCS48_BASE::mov_r4_n,  &MCS48_BASE::mov_r5_n,  &MCS48_BASE::mov_r6_n,  &MCS48_BASE::mov_r7_n,
		&MCS48_BASE::illegal,    &MCS48_BASE::illegal,     &MCS48_BASE::illegal,    &MCS48_BASE::illegal,   &MCS48_BASE::jmp_6,     &MCS48_BASE::sel_rb0,   &MCS48_BASE::jz,        &MCS48_BASE::mov_a_psw,     /* C0 */
		&MCS48_BASE::dec_r0,     &MCS48_BASE::dec_r1,      &MCS48_BASE::dec_r2,     &MCS48_BASE::dec_r3,    &MCS48_BASE::dec_r4,    &MCS48_BASE::dec_r5,    &MCS48_BASE::dec_r6,    &MCS48_BASE::dec_r7,
		&MCS48_BASE::xrl_a_xr0,  &MCS48_BASE::xrl_a_xr1,   &MCS48_BASE::jb_6,       &MCS48_BASE::xrl_a_n,   &MCS48_BASE::call_6,    &MCS48_BASE::sel_rb1,   &MCS48_BASE::illegal,   &MCS48_BASE::mov_psw_a,     /* D0 */
		&MCS48_BASE::xrl_a_r0,   &MCS48_BASE::xrl_a_r1,    &MCS48_BASE::xrl_a_r2,   &MCS48_BASE::xrl_a_r3,  &MCS48_BASE::xrl_a_r4,  &MCS48_BASE::xrl_a_r5,  &MCS48_BASE::xrl_a_r6,  &MCS48_BASE::xrl_a_r7,
		&MCS48_BASE::illegal,    &MCS48_BASE::illegal,     &MCS48_BASE::illegal,    &MCS48_BASE::movp3_a_xa,&MCS48_BASE::jmp_7,     &MCS48_BASE::sel_mb0,   &MCS48_BASE::jnc,       &MCS48_BASE::rl_a,          /* E0 */
		&MCS48_BASE::djnz_r0,    &MCS48_BASE::djnz_r1,     &MCS48_BASE::djnz_r2,    &MCS48_BASE::djnz_r3,   &MCS48_BASE::djnz_r4,   &MCS48_BASE::djnz_r5,   &MCS48_BASE::djnz_r6,   &MCS48_BASE::djnz_r7,
		&MCS48_BASE::mov_a_xr0,  &MCS48_BASE::mov_a_xr1,   &MCS48_BASE::jb_7,       &MCS48_BASE::illegal,   &MCS48_BASE::call_7,    &MCS48_BASE::sel_mb1,   &MCS48_BASE::jc,        &MCS48_BASE::rlc_a,         /* F0 */
		&MCS48_BASE::mov_a_r0,   &MCS48_BASE::mov_a_r1,    &MCS48_BASE::mov_a_r2,   &MCS48_BASE::mov_a_r3,  &MCS48_BASE::mov_a_r4,  &MCS48_BASE::mov_a_r5,  &MCS48_BASE::mov_a_r6,  &MCS48_BASE::mov_a_r7
	};
	
	inline void __FASTCALL update_regptr(mcs48_state *cpustate);
	inline void __FASTCALL push_pc_psw(mcs48_state *cpustate);
	inline void __FASTCALL pull_pc_psw(mcs48_state *cpustate);
	inline int __FASTCALL check_irqs(mcs48_state *cpustate);
	inline UINT8 __FASTCALL opcode_fetch(mcs48_state *cpustate);
	int __FASTCALL op_call(mcs48_state *);
public:
	MCS48_BASE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		d_mem = d_io = d_intr = NULL;
		d_debugger = NULL;
		d_mem_stored = d_io_stored = NULL;
		total_icount = prev_total_icount = 0;
		set_device_name(_T("MCS48 MCU"));
	}
	~MCS48_BASE() {}
	
	// common functions
	virtual void initialize();
	virtual void release();
	void reset();
	virtual int run(int icount);
	void __FASTCALL write_signal(int id, uint32_t data, uint32_t mask);
	uint32_t get_pc();
	uint32_t get_next_pc();
//#ifdef USE_DEBUGGER
	void __FASTCALL write_debug_data8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_data8(uint32_t addr);
	void __FASTCALL write_debug_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_debug_io8(uint32_t addr);
	bool write_debug_reg(const _TCHAR *reg, uint32_t data);
	bool get_debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	int debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata = 0);
//#endif
	
	// unique functions
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_io(DEVICE* device)
	{
		d_io = device;
	}
	void set_context_intr(DEVICE* device)
	{
		d_intr = device;
	}
	void load_rom_image(const _TCHAR *file_path);
	uint8_t *get_rom_ptr();
};


/*-------------------------------------------------
    update_regptr - update the regptr member to
    point to the appropriate register bank
-------------------------------------------------*/

inline void MCS48_BASE::update_regptr(mcs48_state *cpustate)
{
	cpustate->regptr = ((cpustate->psw & B_FLAG) ? 24 : 0);
}

/*-------------------------------------------------
    push_pc_psw - push the cpustate->pc and cpustate->psw values onto
    the stack
-------------------------------------------------*/

inline void MCS48_BASE::push_pc_psw(mcs48_state *cpustate)
{
	UINT8 sp = cpustate->psw & 0x07;
	__mcs48_ram_w(8 + 2*sp, cpustate->pc);
	__mcs48_ram_w(9 + 2*sp, ((cpustate->pc >> 8) & 0x0f) | (cpustate->psw & 0xf0));
	cpustate->psw = (cpustate->psw & 0xf8) | ((sp + 1) & 0x07);
}

/*-------------------------------------------------
    pull_pc_psw - pull the PC and PSW values from
    the stack
-------------------------------------------------*/

inline void MCS48_BASE::pull_pc_psw(mcs48_state *cpustate)
{
	UINT8 sp = (cpustate->psw - 1) & 0x07;
	cpustate->pc = __mcs48_ram_r(8 + 2*sp);
	cpustate->pc |= __mcs48_ram_r(9 + 2*sp) << 8;
	cpustate->psw = ((cpustate->pc >> 8) & 0xf0) | 0x08 | sp;
	cpustate->pc &= 0xfff;
	update_regptr(cpustate);
}

/*-------------------------------------------------
    check_irqs - check for and process IRQs
-------------------------------------------------*/

inline int MCS48_BASE::check_irqs(mcs48_state *cpustate)
{
	/* if something is in progress, we do nothing */
	if (cpustate->irq_in_progress)
		return 0;

	/* external interrupts take priority */
	if (cpustate->irq_state && cpustate->xirq_enabled)
	{
		cpustate->irq_state = FALSE;
		cpustate->irq_in_progress = TRUE;

		/* transfer to location 0x03 */
		push_pc_psw(cpustate);
		cpustate->pc = 0x03;

		/* indicate we took the external IRQ */
		if (cpustate->intr != NULL)
			cpustate->intr->get_intr_ack();
		return 2;
	}

	/* timer overflow interrupts follow */
	if (cpustate->timer_overflow && cpustate->tirq_enabled)
	{
		cpustate->irq_in_progress = TRUE;

		/* transfer to location 0x07 */
		push_pc_psw(cpustate);
		cpustate->pc = 0x07;

		/* timer overflow flip-flop is reset once taken */
		cpustate->timer_overflow = FALSE;
		return 2;
	}
	return 0;
}

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    opcode_fetch - fetch an opcode byte
-------------------------------------------------*/

inline UINT8 MCS48_BASE::opcode_fetch(mcs48_state *cpustate)
{
	return cpustate->rom[cpustate->pc++ & 0xfff];
}

/*-------------------------------------------------
    argument_fetch - fetch an opcode argument
    byte
-------------------------------------------------*/

inline UINT8 MCS48_BASE::argument_fetch(mcs48_state *cpustate)
{
	return cpustate->rom[cpustate->pc++ & 0xfff];
}

/*-------------------------------------------------
    pull_pc - pull the PC value from the stack,
    leaving the upper part of PSW intact
-------------------------------------------------*/

inline void MCS48_BASE::pull_pc(mcs48_state *cpustate)
{
	UINT8 sp = (cpustate->psw - 1) & 0x07;
	cpustate->pc = __mcs48_ram_r(8 + 2*sp);
	cpustate->pc |= __mcs48_ram_r(9 + 2*sp) << 8;
	cpustate->pc &= 0xfff;
	cpustate->psw = (cpustate->psw & 0xf0) | 0x08 | sp;
}

/*-------------------------------------------------
    execute_add - perform the logic of an ADD
    instruction
-------------------------------------------------*/

inline void MCS48_BASE::execute_add(mcs48_state *cpustate, UINT8 dat)
{
	UINT16 temp = cpustate->a + dat;
	UINT16 temp4 = (cpustate->a & 0x0f) + (dat & 0x0f);

	cpustate->psw &= ~(C_FLAG | A_FLAG);
	cpustate->psw |= (temp4 << 2) & A_FLAG;
	cpustate->psw |= (temp >> 1) & C_FLAG;
	cpustate->a = temp;
}

/*-------------------------------------------------
    execute_addc - perform the logic of an ADDC
    instruction
-------------------------------------------------*/

inline void MCS48_BASE::execute_addc(mcs48_state *cpustate, UINT8 dat)
{
	UINT8 carryin = (cpustate->psw & C_FLAG) >> 7;
	UINT16 temp = cpustate->a + dat + carryin;
	UINT16 temp4 = (cpustate->a & 0x0f) + (dat & 0x0f) + carryin;

	cpustate->psw &= ~(C_FLAG | A_FLAG);
	cpustate->psw |= (temp4 << 2) & A_FLAG;
	cpustate->psw |= (temp >> 1) & C_FLAG;
	cpustate->a = temp;
}

/*-------------------------------------------------
    execute_jmp - perform the logic of a JMP
    instruction
-------------------------------------------------*/

inline void MCS48_BASE::execute_jmp(mcs48_state *cpustate, UINT16 address)
{
	UINT16 a11 = (cpustate->irq_in_progress) ? 0 : cpustate->a11;
	cpustate->pc = address | a11;
}

/*-------------------------------------------------
    execute_call - perform the logic of a CALL
    instruction
-------------------------------------------------*/

inline void MCS48_BASE::execute_call(mcs48_state *cpustate, UINT16 address)
{
	push_pc_psw(cpustate);
	execute_jmp(cpustate, address);
}

/*-------------------------------------------------
    execute_jcc - perform the logic of a
    conditional jump instruction
-------------------------------------------------*/

inline void MCS48_BASE::execute_jcc(mcs48_state *cpustate, UINT8 result)
{
	UINT8 offset = argument_fetch(cpustate);
	if (result != 0)
		cpustate->pc = ((cpustate->pc - 1) & 0xf00) | offset;
}

/*-------------------------------------------------
    expander_operation - perform an operation via
    the 8243 expander chip
-------------------------------------------------*/

inline void MCS48_BASE::expander_operation(mcs48_state *cpustate, UINT8 operation, UINT8 port)
{
	/* put opcode/data on low 4 bits of P2 */
	__mcs48_port_w(2, cpustate->p2 = (cpustate->p2 & 0xf0) | (operation << 2) | (port & 3));

	/* generate high-to-low transition on PROG line */
	__mcs48_prog_w(0);

	/* put data on low 4 bits of P2 */
	if (operation != 0)
		__mcs48_port_w(2, cpustate->p2 = (cpustate->p2 & 0xf0) | (cpustate->a & 0x0f));
	else
		cpustate->a = __mcs48_port_r(2) | 0x0f;

	/* generate low-to-high transition on PROG line */
	__mcs48_prog_w(1);
}


class MCS48 : public MCS48_BASE
{
private:
	void __FASTCALL burn_cycles(mcs48_state *cpustate, int count);
	/* ---------------------------------------------------------------------------
	registers
	--------------------------------------------------------------------------- */
	
	
public:
	MCS48(VM_TEMPLATE* parent_vm, EMU* parent_emu) : MCS48_BASE(parent_vm, parent_emu)
	{
	}
	~MCS48() {}
	void initialize();
	void release();
	int run(int icount);
	bool process_state(FILEIO* state_fio, bool loading);
#ifdef USE_DEBUGGER
	bool is_cpu()
	{
		return true;
	}
	bool is_debugger_available()
	{
		return true;
	}
	void *get_debugger()
	{
		return d_debugger;
	}
	uint32_t get_debug_prog_addr_mask()
	{
		return 0xfff;
	}
	uint32_t get_debug_data_addr_mask()
	{
		return 0xff;
	}
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
#endif
};	

#endif

