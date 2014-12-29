/*
	Skelton for retropc emulator

	Origin : MAME 0.148
	Author : Takeda.Toshiya
	Date   : 2013.05.01-

	[ MCS48 ]
*/

#include "mcs48.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif
#include "../fileio.h"

/***************************************************************************

    mcs48.c

    Intel MCS-48/UPI-41 Portable Emulator

    Copyright Mirko Buffoni
    Based on the original work Copyright Dan Boris, an 8048 emulator
    You are not allowed to distribute this software commercially

****************************************************************************

    Note that the default internal divisor for this chip is by 3 and
    then again by 5, or by 15 total.

***************************************************************************/

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4244 )
#endif

#define INLINE inline

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* timer/counter enable bits */
#define TIMER_ENABLED   0x01
#define COUNTER_ENABLED 0x02

/* flag bits */
#define C_FLAG          0x80
#define A_FLAG          0x40
#define F_FLAG          0x20
#define B_FLAG          0x10

/* status bits (UPI-41) */
#define STS_F1          0x08
#define STS_F0          0x04

/* 8243 expander operations */
enum
{
	MCS48_EXPANDER_OP_READ = 0,
	MCS48_EXPANDER_OP_WRITE = 1,
	MCS48_EXPANDER_OP_OR = 2,
	MCS48_EXPANDER_OP_AND = 3
};

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

/* opcode table entry */
typedef int (*mcs48_ophandler)(mcs48_state *state);

/***************************************************************************
    MACROS
***************************************************************************/

#define program_r(a)    cpustate->rom[(a) & 0xfff]

#define ram_r(a)        cpustate->mem->read_data8(a)
#define ram_w(a,V)      cpustate->mem->write_data8(a, V)
#define reg_r(a)        cpustate->mem->read_data8(cpustate->regptr + a)
#define reg_w(a,V)      cpustate->mem->write_data8(cpustate->regptr + a, V)

#define ext_r(a)        cpustate->io->read_io8(a)
#define ext_w(a,V)      cpustate->io->write_io8(a, V)
#define port_r(a)       cpustate->io->read_io8(MCS48_PORT_P0 + a)
#define port_w(a,V)     cpustate->io->write_io8(MCS48_PORT_P0 + a, V)
#define test_r(a)       cpustate->io->read_io8(MCS48_PORT_T0 + a)
#define test_w(a,V)     cpustate->io->write_io8(MCS48_PORT_T0 + a, V)
#define bus_r()         cpustate->io->read_io8(MCS48_PORT_BUS)
#define bus_w(V)        cpustate->io->write_io8(MCS48_PORT_BUS, V)
#define prog_w(V)       cpustate->io->write_io8(MCS48_PORT_PROG, V)

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int check_irqs(mcs48_state *cpustate);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    opcode_fetch - fetch an opcode byte
-------------------------------------------------*/

INLINE UINT8 opcode_fetch(mcs48_state *cpustate)
{
	return cpustate->rom[cpustate->pc++ & 0xfff];
}

/*-------------------------------------------------
    argument_fetch - fetch an opcode argument
    byte
-------------------------------------------------*/

INLINE UINT8 argument_fetch(mcs48_state *cpustate)
{
	return cpustate->rom[cpustate->pc++ & 0xfff];
}

/*-------------------------------------------------
    update_regptr - update the regptr member to
    point to the appropriate register bank
-------------------------------------------------*/

INLINE void update_regptr(mcs48_state *cpustate)
{
	cpustate->regptr = ((cpustate->psw & B_FLAG) ? 24 : 0);
}

/*-------------------------------------------------
    push_pc_psw - push the cpustate->pc and cpustate->psw values onto
    the stack
-------------------------------------------------*/

INLINE void push_pc_psw(mcs48_state *cpustate)
{
	UINT8 sp = cpustate->psw & 0x07;
	ram_w(8 + 2*sp, cpustate->pc);
	ram_w(9 + 2*sp, ((cpustate->pc >> 8) & 0x0f) | (cpustate->psw & 0xf0));
	cpustate->psw = (cpustate->psw & 0xf8) | ((sp + 1) & 0x07);
}

/*-------------------------------------------------
    pull_pc_psw - pull the PC and PSW values from
    the stack
-------------------------------------------------*/

INLINE void pull_pc_psw(mcs48_state *cpustate)
{
	UINT8 sp = (cpustate->psw - 1) & 0x07;
	cpustate->pc = ram_r(8 + 2*sp);
	cpustate->pc |= ram_r(9 + 2*sp) << 8;
	cpustate->psw = ((cpustate->pc >> 8) & 0xf0) | 0x08 | sp;
	cpustate->pc &= 0xfff;
	update_regptr(cpustate);
}

/*-------------------------------------------------
    pull_pc - pull the PC value from the stack,
    leaving the upper part of PSW intact
-------------------------------------------------*/

INLINE void pull_pc(mcs48_state *cpustate)
{
	UINT8 sp = (cpustate->psw - 1) & 0x07;
	cpustate->pc = ram_r(8 + 2*sp);
	cpustate->pc |= ram_r(9 + 2*sp) << 8;
	cpustate->pc &= 0xfff;
	cpustate->psw = (cpustate->psw & 0xf0) | 0x08 | sp;
}

/*-------------------------------------------------
    execute_add - perform the logic of an ADD
    instruction
-------------------------------------------------*/

INLINE void execute_add(mcs48_state *cpustate, UINT8 dat)
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

INLINE void execute_addc(mcs48_state *cpustate, UINT8 dat)
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

INLINE void execute_jmp(mcs48_state *cpustate, UINT16 address)
{
	UINT16 a11 = (cpustate->irq_in_progress) ? 0 : cpustate->a11;
	cpustate->pc = address | a11;
}

/*-------------------------------------------------
    execute_call - perform the logic of a CALL
    instruction
-------------------------------------------------*/

INLINE void execute_call(mcs48_state *cpustate, UINT16 address)
{
	push_pc_psw(cpustate);
	execute_jmp(cpustate, address);
}

/*-------------------------------------------------
    execute_jcc - perform the logic of a
    conditional jump instruction
-------------------------------------------------*/

INLINE void execute_jcc(mcs48_state *cpustate, UINT8 result)
{
	UINT8 offset = argument_fetch(cpustate);
	if (result != 0)
		cpustate->pc = ((cpustate->pc - 1) & 0xf00) | offset;
}

/*-------------------------------------------------
    expander_operation - perform an operation via
    the 8243 expander chip
-------------------------------------------------*/

INLINE void expander_operation(mcs48_state *cpustate, UINT8 operation, UINT8 port)
{
	/* put opcode/data on low 4 bits of P2 */
	port_w(2, cpustate->p2 = (cpustate->p2 & 0xf0) | (operation << 2) | (port & 3));

	/* generate high-to-low transition on PROG line */
	prog_w(0);

	/* put data on low 4 bits of P2 */
	if (operation != 0)
		port_w(2, cpustate->p2 = (cpustate->p2 & 0xf0) | (cpustate->a & 0x0f));
	else
		cpustate->a = port_r(2) | 0x0f;

	/* generate low-to-high transition on PROG line */
	prog_w(1);
}

/***************************************************************************
    OPCODE HANDLERS
***************************************************************************/

#define OPHANDLER(_name) static int _name(mcs48_state *cpustate)

OPHANDLER( illegal )
{
//	logerror("MCS-48 PC:%04X - Illegal opcode = %02x\n", cpustate->pc - 1, program_r(cpustate->pc - 1));
	return 1;
}

OPHANDLER( add_a_r0 )       { execute_add(cpustate, reg_r(0)); return 1; }
OPHANDLER( add_a_r1 )       { execute_add(cpustate, reg_r(1)); return 1; }
OPHANDLER( add_a_r2 )       { execute_add(cpustate, reg_r(2)); return 1; }
OPHANDLER( add_a_r3 )       { execute_add(cpustate, reg_r(3)); return 1; }
OPHANDLER( add_a_r4 )       { execute_add(cpustate, reg_r(4)); return 1; }
OPHANDLER( add_a_r5 )       { execute_add(cpustate, reg_r(5)); return 1; }
OPHANDLER( add_a_r6 )       { execute_add(cpustate, reg_r(6)); return 1; }
OPHANDLER( add_a_r7 )       { execute_add(cpustate, reg_r(7)); return 1; }
OPHANDLER( add_a_xr0 )      { execute_add(cpustate, ram_r(reg_r(0))); return 1; }
OPHANDLER( add_a_xr1 )      { execute_add(cpustate, ram_r(reg_r(1))); return 1; }
OPHANDLER( add_a_n )        { execute_add(cpustate, argument_fetch(cpustate)); return 2; }

OPHANDLER( adc_a_r0 )       { execute_addc(cpustate, reg_r(0)); return 1; }
OPHANDLER( adc_a_r1 )       { execute_addc(cpustate, reg_r(1)); return 1; }
OPHANDLER( adc_a_r2 )       { execute_addc(cpustate, reg_r(2)); return 1; }
OPHANDLER( adc_a_r3 )       { execute_addc(cpustate, reg_r(3)); return 1; }
OPHANDLER( adc_a_r4 )       { execute_addc(cpustate, reg_r(4)); return 1; }
OPHANDLER( adc_a_r5 )       { execute_addc(cpustate, reg_r(5)); return 1; }
OPHANDLER( adc_a_r6 )       { execute_addc(cpustate, reg_r(6)); return 1; }
OPHANDLER( adc_a_r7 )       { execute_addc(cpustate, reg_r(7)); return 1; }
OPHANDLER( adc_a_xr0 )      { execute_addc(cpustate, ram_r(reg_r(0))); return 1; }
OPHANDLER( adc_a_xr1 )      { execute_addc(cpustate, ram_r(reg_r(1))); return 1; }
OPHANDLER( adc_a_n )        { execute_addc(cpustate, argument_fetch(cpustate)); return 2; }

OPHANDLER( anl_a_r0 )       { cpustate->a &= reg_r(0); return 1; }
OPHANDLER( anl_a_r1 )       { cpustate->a &= reg_r(1); return 1; }
OPHANDLER( anl_a_r2 )       { cpustate->a &= reg_r(2); return 1; }
OPHANDLER( anl_a_r3 )       { cpustate->a &= reg_r(3); return 1; }
OPHANDLER( anl_a_r4 )       { cpustate->a &= reg_r(4); return 1; }
OPHANDLER( anl_a_r5 )       { cpustate->a &= reg_r(5); return 1; }
OPHANDLER( anl_a_r6 )       { cpustate->a &= reg_r(6); return 1; }
OPHANDLER( anl_a_r7 )       { cpustate->a &= reg_r(7); return 1; }
OPHANDLER( anl_a_xr0 )      { cpustate->a &= ram_r(reg_r(0)); return 1; }
OPHANDLER( anl_a_xr1 )      { cpustate->a &= ram_r(reg_r(1)); return 1; }
OPHANDLER( anl_a_n )        { cpustate->a &= argument_fetch(cpustate); return 2; }

OPHANDLER( anl_bus_n )      { bus_w(bus_r() & argument_fetch(cpustate)); return 2; }
OPHANDLER( anl_p1_n )       { port_w(1, cpustate->p1 &= argument_fetch(cpustate)); return 2; }
OPHANDLER( anl_p2_n )       { port_w(2, cpustate->p2 &= argument_fetch(cpustate)); return 2; }
OPHANDLER( anld_p4_a )      { expander_operation(cpustate, MCS48_EXPANDER_OP_AND, 4); return 2; }
OPHANDLER( anld_p5_a )      { expander_operation(cpustate, MCS48_EXPANDER_OP_AND, 5); return 2; }
OPHANDLER( anld_p6_a )      { expander_operation(cpustate, MCS48_EXPANDER_OP_AND, 6); return 2; }
OPHANDLER( anld_p7_a )      { expander_operation(cpustate, MCS48_EXPANDER_OP_AND, 7); return 2; }

OPHANDLER( call_0 )         { execute_call(cpustate, argument_fetch(cpustate) | 0x000); return 2; }
OPHANDLER( call_1 )         { execute_call(cpustate, argument_fetch(cpustate) | 0x100); return 2; }
OPHANDLER( call_2 )         { execute_call(cpustate, argument_fetch(cpustate) | 0x200); return 2; }
OPHANDLER( call_3 )         { execute_call(cpustate, argument_fetch(cpustate) | 0x300); return 2; }
OPHANDLER( call_4 )         { execute_call(cpustate, argument_fetch(cpustate) | 0x400); return 2; }
OPHANDLER( call_5 )         { execute_call(cpustate, argument_fetch(cpustate) | 0x500); return 2; }
OPHANDLER( call_6 )         { execute_call(cpustate, argument_fetch(cpustate) | 0x600); return 2; }
OPHANDLER( call_7 )         { execute_call(cpustate, argument_fetch(cpustate) | 0x700); return 2; }

OPHANDLER( clr_a )          { cpustate->a = 0; return 1; }
OPHANDLER( clr_c )          { cpustate->psw &= ~C_FLAG; return 1; }
OPHANDLER( clr_f0 )         { cpustate->psw &= ~F_FLAG; cpustate->sts &= ~STS_F0; return 1; }
OPHANDLER( clr_f1 )         { cpustate->sts &= ~STS_F1; return 1; }

OPHANDLER( cpl_a )          { cpustate->a ^= 0xff; return 1; }
OPHANDLER( cpl_c )          { cpustate->psw ^= C_FLAG; return 1; }
OPHANDLER( cpl_f0 )         { cpustate->psw ^= F_FLAG; cpustate->sts ^= STS_F0; return 1; }
OPHANDLER( cpl_f1 )         { cpustate->sts ^= STS_F1; return 1; }

OPHANDLER( da_a )
{
	if ((cpustate->a & 0x0f) > 0x09 || (cpustate->psw & A_FLAG))
	{
		cpustate->a += 0x06;
		if ((cpustate->a & 0xf0) == 0x00)
			cpustate->psw |= C_FLAG;
	}
	if ((cpustate->a & 0xf0) > 0x90 || (cpustate->psw & C_FLAG))
	{
		cpustate->a += 0x60;
		cpustate->psw |= C_FLAG;
	}
	else
		cpustate->psw &= ~C_FLAG;
	return 1;
}

OPHANDLER( dec_a )          { cpustate->a--; return 1; }
OPHANDLER( dec_r0 )         { reg_w(0, reg_r(0) - 1); return 1; }
OPHANDLER( dec_r1 )         { reg_w(1, reg_r(1) - 1); return 1; }
OPHANDLER( dec_r2 )         { reg_w(2, reg_r(2) - 1); return 1; }
OPHANDLER( dec_r3 )         { reg_w(3, reg_r(3) - 1); return 1; }
OPHANDLER( dec_r4 )         { reg_w(4, reg_r(4) - 1); return 1; }
OPHANDLER( dec_r5 )         { reg_w(5, reg_r(5) - 1); return 1; }
OPHANDLER( dec_r6 )         { reg_w(6, reg_r(6) - 1); return 1; }
OPHANDLER( dec_r7 )         { reg_w(7, reg_r(7) - 1); return 1; }

OPHANDLER( dis_i )          { cpustate->xirq_enabled = FALSE; return 1; }
OPHANDLER( dis_tcnti )      { cpustate->tirq_enabled = FALSE; cpustate->timer_overflow = FALSE; return 1; }

OPHANDLER( djnz_r0 )        { UINT8 r0 = reg_r(0); reg_w(0, --r0); execute_jcc(cpustate, r0 != 0); return 2; }
OPHANDLER( djnz_r1 )        { UINT8 r1 = reg_r(1); reg_w(1, --r1); execute_jcc(cpustate, r1 != 0); return 2; }
OPHANDLER( djnz_r2 )        { UINT8 r2 = reg_r(2); reg_w(2, --r2); execute_jcc(cpustate, r2 != 0); return 2; }
OPHANDLER( djnz_r3 )        { UINT8 r3 = reg_r(3); reg_w(3, --r3); execute_jcc(cpustate, r3 != 0); return 2; }
OPHANDLER( djnz_r4 )        { UINT8 r4 = reg_r(4); reg_w(4, --r4); execute_jcc(cpustate, r4 != 0); return 2; }
OPHANDLER( djnz_r5 )        { UINT8 r5 = reg_r(5); reg_w(5, --r5); execute_jcc(cpustate, r5 != 0); return 2; }
OPHANDLER( djnz_r6 )        { UINT8 r6 = reg_r(6); reg_w(6, --r6); execute_jcc(cpustate, r6 != 0); return 2; }
OPHANDLER( djnz_r7 )        { UINT8 r7 = reg_r(7); reg_w(7, --r7); execute_jcc(cpustate, r7 != 0); return 2; }

OPHANDLER( en_i )           { cpustate->xirq_enabled = TRUE; return 1 + check_irqs(cpustate); }
OPHANDLER( en_tcnti )       { cpustate->tirq_enabled = TRUE; return 1 + check_irqs(cpustate); }
OPHANDLER( ent0_clk )       { cpustate->t0_clk_enabled = TRUE; return 1; }

OPHANDLER( in_a_p1 )        { cpustate->a = port_r(1) & cpustate->p1; return 2; }
OPHANDLER( in_a_p2 )        { cpustate->a = port_r(2) & cpustate->p2; return 2; }
OPHANDLER( ins_a_bus )      { cpustate->a = bus_r(); return 2; }

OPHANDLER( inc_a )          { cpustate->a++; return 1; }
OPHANDLER( inc_r0 )         { reg_w(0, reg_r(0) + 1); return 1; }
OPHANDLER( inc_r1 )         { reg_w(1, reg_r(1) + 1); return 1; }
OPHANDLER( inc_r2 )         { reg_w(2, reg_r(2) + 1); return 1; }
OPHANDLER( inc_r3 )         { reg_w(3, reg_r(3) + 1); return 1; }
OPHANDLER( inc_r4 )         { reg_w(4, reg_r(4) + 1); return 1; }
OPHANDLER( inc_r5 )         { reg_w(5, reg_r(5) + 1); return 1; }
OPHANDLER( inc_r6 )         { reg_w(6, reg_r(6) + 1); return 1; }
OPHANDLER( inc_r7 )         { reg_w(7, reg_r(7) + 1); return 1; }
OPHANDLER( inc_xr0 )        { UINT8 r0 = reg_r(0); ram_w(r0, ram_r(r0) + 1); return 1; }
OPHANDLER( inc_xr1 )        { UINT8 r1 = reg_r(1); ram_w(r1, ram_r(r1) + 1); return 1; }

OPHANDLER( jb_0 )           { execute_jcc(cpustate, (cpustate->a & 0x01) != 0); return 2; }
OPHANDLER( jb_1 )           { execute_jcc(cpustate, (cpustate->a & 0x02) != 0); return 2; }
OPHANDLER( jb_2 )           { execute_jcc(cpustate, (cpustate->a & 0x04) != 0); return 2; }
OPHANDLER( jb_3 )           { execute_jcc(cpustate, (cpustate->a & 0x08) != 0); return 2; }
OPHANDLER( jb_4 )           { execute_jcc(cpustate, (cpustate->a & 0x10) != 0); return 2; }
OPHANDLER( jb_5 )           { execute_jcc(cpustate, (cpustate->a & 0x20) != 0); return 2; }
OPHANDLER( jb_6 )           { execute_jcc(cpustate, (cpustate->a & 0x40) != 0); return 2; }
OPHANDLER( jb_7 )           { execute_jcc(cpustate, (cpustate->a & 0x80) != 0); return 2; }
OPHANDLER( jc )             { execute_jcc(cpustate, (cpustate->psw & C_FLAG) != 0); return 2; }
OPHANDLER( jf0 )            { execute_jcc(cpustate, (cpustate->psw & F_FLAG) != 0); return 2; }
OPHANDLER( jf1 )            { execute_jcc(cpustate, (cpustate->sts & STS_F1) != 0); return 2; }
OPHANDLER( jnc )            { execute_jcc(cpustate, (cpustate->psw & C_FLAG) == 0); return 2; }
OPHANDLER( jni )            { execute_jcc(cpustate, cpustate->int_state == 0); return 2; }
OPHANDLER( jnt_0 )          { execute_jcc(cpustate, test_r(0) == 0); return 2; }
OPHANDLER( jnt_1 )          { execute_jcc(cpustate, test_r(1) == 0); return 2; }
OPHANDLER( jnz )            { execute_jcc(cpustate, cpustate->a != 0); return 2; }
OPHANDLER( jtf )            { execute_jcc(cpustate, cpustate->timer_flag); cpustate->timer_flag = FALSE; return 2; }
OPHANDLER( jt_0 )           { execute_jcc(cpustate, test_r(0) != 0); return 2; }
OPHANDLER( jt_1 )           { execute_jcc(cpustate, test_r(1) != 0); return 2; }
OPHANDLER( jz )             { execute_jcc(cpustate, cpustate->a == 0); return 2; }

OPHANDLER( jmp_0 )          { execute_jmp(cpustate, argument_fetch(cpustate) | 0x000); return 2; }
OPHANDLER( jmp_1 )          { execute_jmp(cpustate, argument_fetch(cpustate) | 0x100); return 2; }
OPHANDLER( jmp_2 )          { execute_jmp(cpustate, argument_fetch(cpustate) | 0x200); return 2; }
OPHANDLER( jmp_3 )          { execute_jmp(cpustate, argument_fetch(cpustate) | 0x300); return 2; }
OPHANDLER( jmp_4 )          { execute_jmp(cpustate, argument_fetch(cpustate) | 0x400); return 2; }
OPHANDLER( jmp_5 )          { execute_jmp(cpustate, argument_fetch(cpustate) | 0x500); return 2; }
OPHANDLER( jmp_6 )          { execute_jmp(cpustate, argument_fetch(cpustate) | 0x600); return 2; }
OPHANDLER( jmp_7 )          { execute_jmp(cpustate, argument_fetch(cpustate) | 0x700); return 2; }
OPHANDLER( jmpp_xa )        { cpustate->pc &= 0xf00; cpustate->pc |= program_r(cpustate->pc | cpustate->a); return 2; }

OPHANDLER( mov_a_n )        { cpustate->a = argument_fetch(cpustate); return 2; }
OPHANDLER( mov_a_psw )      { cpustate->a = cpustate->psw; return 1; }
OPHANDLER( mov_a_r0 )       { cpustate->a = reg_r(0); return 1; }
OPHANDLER( mov_a_r1 )       { cpustate->a = reg_r(1); return 1; }
OPHANDLER( mov_a_r2 )       { cpustate->a = reg_r(2); return 1; }
OPHANDLER( mov_a_r3 )       { cpustate->a = reg_r(3); return 1; }
OPHANDLER( mov_a_r4 )       { cpustate->a = reg_r(4); return 1; }
OPHANDLER( mov_a_r5 )       { cpustate->a = reg_r(5); return 1; }
OPHANDLER( mov_a_r6 )       { cpustate->a = reg_r(6); return 1; }
OPHANDLER( mov_a_r7 )       { cpustate->a = reg_r(7); return 1; }
OPHANDLER( mov_a_xr0 )      { cpustate->a = ram_r(reg_r(0)); return 1; }
OPHANDLER( mov_a_xr1 )      { cpustate->a = ram_r(reg_r(1)); return 1; }
OPHANDLER( mov_a_t )        { cpustate->a = cpustate->timer; return 1; }

OPHANDLER( mov_psw_a )      { cpustate->psw = cpustate->a; update_regptr(cpustate); return 1; }
OPHANDLER( mov_r0_a )       { reg_w(0, cpustate->a); return 1; }
OPHANDLER( mov_r1_a )       { reg_w(1, cpustate->a); return 1; }
OPHANDLER( mov_r2_a )       { reg_w(2, cpustate->a); return 1; }
OPHANDLER( mov_r3_a )       { reg_w(3, cpustate->a); return 1; }
OPHANDLER( mov_r4_a )       { reg_w(4, cpustate->a); return 1; }
OPHANDLER( mov_r5_a )       { reg_w(5, cpustate->a); return 1; }
OPHANDLER( mov_r6_a )       { reg_w(6, cpustate->a); return 1; }
OPHANDLER( mov_r7_a )       { reg_w(7, cpustate->a); return 1; }
OPHANDLER( mov_r0_n )       { reg_w(0, argument_fetch(cpustate)); return 2; }
OPHANDLER( mov_r1_n )       { reg_w(1, argument_fetch(cpustate)); return 2; }
OPHANDLER( mov_r2_n )       { reg_w(2, argument_fetch(cpustate)); return 2; }
OPHANDLER( mov_r3_n )       { reg_w(3, argument_fetch(cpustate)); return 2; }
OPHANDLER( mov_r4_n )       { reg_w(4, argument_fetch(cpustate)); return 2; }
OPHANDLER( mov_r5_n )       { reg_w(5, argument_fetch(cpustate)); return 2; }
OPHANDLER( mov_r6_n )       { reg_w(6, argument_fetch(cpustate)); return 2; }
OPHANDLER( mov_r7_n )       { reg_w(7, argument_fetch(cpustate)); return 2; }
OPHANDLER( mov_t_a )        { cpustate->timer = cpustate->a; return 1; }
OPHANDLER( mov_xr0_a )      { ram_w(reg_r(0), cpustate->a); return 1; }
OPHANDLER( mov_xr1_a )      { ram_w(reg_r(1), cpustate->a); return 1; }
OPHANDLER( mov_xr0_n )      { ram_w(reg_r(0), argument_fetch(cpustate)); return 2; }
OPHANDLER( mov_xr1_n )      { ram_w(reg_r(1), argument_fetch(cpustate)); return 2; }

OPHANDLER( movd_a_p4 )      { expander_operation(cpustate, MCS48_EXPANDER_OP_READ, 4); return 2; }
OPHANDLER( movd_a_p5 )      { expander_operation(cpustate, MCS48_EXPANDER_OP_READ, 5); return 2; }
OPHANDLER( movd_a_p6 )      { expander_operation(cpustate, MCS48_EXPANDER_OP_READ, 6); return 2; }
OPHANDLER( movd_a_p7 )      { expander_operation(cpustate, MCS48_EXPANDER_OP_READ, 7); return 2; }
OPHANDLER( movd_p4_a )      { expander_operation(cpustate, MCS48_EXPANDER_OP_WRITE, 4); return 2; }
OPHANDLER( movd_p5_a )      { expander_operation(cpustate, MCS48_EXPANDER_OP_WRITE, 5); return 2; }
OPHANDLER( movd_p6_a )      { expander_operation(cpustate, MCS48_EXPANDER_OP_WRITE, 6); return 2; }
OPHANDLER( movd_p7_a )      { expander_operation(cpustate, MCS48_EXPANDER_OP_WRITE, 7); return 2; }

OPHANDLER( movp_a_xa )      { cpustate->a = program_r((cpustate->pc & 0xf00) | cpustate->a); return 2; }
OPHANDLER( movp3_a_xa )     { cpustate->a = program_r(0x300 | cpustate->a); return 2; }

OPHANDLER( movx_a_xr0 )     { cpustate->a = ext_r(reg_r(0)); return 2; }
OPHANDLER( movx_a_xr1 )     { cpustate->a = ext_r(reg_r(1)); return 2; }
OPHANDLER( movx_xr0_a )     { ext_w(reg_r(0), cpustate->a); return 2; }
OPHANDLER( movx_xr1_a )     { ext_w(reg_r(1), cpustate->a); return 2; }

OPHANDLER( nop )            { return 1; }

OPHANDLER( orl_a_r0 )       { cpustate->a |= reg_r(0); return 1; }
OPHANDLER( orl_a_r1 )       { cpustate->a |= reg_r(1); return 1; }
OPHANDLER( orl_a_r2 )       { cpustate->a |= reg_r(2); return 1; }
OPHANDLER( orl_a_r3 )       { cpustate->a |= reg_r(3); return 1; }
OPHANDLER( orl_a_r4 )       { cpustate->a |= reg_r(4); return 1; }
OPHANDLER( orl_a_r5 )       { cpustate->a |= reg_r(5); return 1; }
OPHANDLER( orl_a_r6 )       { cpustate->a |= reg_r(6); return 1; }
OPHANDLER( orl_a_r7 )       { cpustate->a |= reg_r(7); return 1; }
OPHANDLER( orl_a_xr0 )      { cpustate->a |= ram_r(reg_r(0)); return 1; }
OPHANDLER( orl_a_xr1 )      { cpustate->a |= ram_r(reg_r(1)); return 1; }
OPHANDLER( orl_a_n )        { cpustate->a |= argument_fetch(cpustate); return 2; }

OPHANDLER( orl_bus_n )      { bus_w(bus_r() | argument_fetch(cpustate)); return 2; }
OPHANDLER( orl_p1_n )       { port_w(1, cpustate->p1 |= argument_fetch(cpustate)); return 2; }
OPHANDLER( orl_p2_n )       { port_w(2, cpustate->p2 |= argument_fetch(cpustate)); return 2; }
OPHANDLER( orld_p4_a )      { expander_operation(cpustate, MCS48_EXPANDER_OP_OR, 4); return 2; }
OPHANDLER( orld_p5_a )      { expander_operation(cpustate, MCS48_EXPANDER_OP_OR, 5); return 2; }
OPHANDLER( orld_p6_a )      { expander_operation(cpustate, MCS48_EXPANDER_OP_OR, 6); return 2; }
OPHANDLER( orld_p7_a )      { expander_operation(cpustate, MCS48_EXPANDER_OP_OR, 7); return 2; }

OPHANDLER( outl_bus_a )     { bus_w(cpustate->a); return 2; }
OPHANDLER( outl_p1_a )      { port_w(1, cpustate->p1 = cpustate->a); return 2; }
OPHANDLER( outl_p2_a )      { port_w(2, cpustate->p2 = cpustate->a); return 2; }

OPHANDLER( ret )            { pull_pc(cpustate); return 2; }
OPHANDLER( retr )
{
	pull_pc_psw(cpustate);

	/* implicitly clear the IRQ in progress flip flop and re-check interrupts */
	cpustate->irq_in_progress = FALSE;
	return 2 + check_irqs(cpustate);
}

OPHANDLER( rl_a )           { cpustate->a = (cpustate->a << 1) | (cpustate->a >> 7); return 1; }
OPHANDLER( rlc_a )          { UINT8 newc = cpustate->a & C_FLAG; cpustate->a = (cpustate->a << 1) | (cpustate->psw >> 7); cpustate->psw = (cpustate->psw & ~C_FLAG) | newc; return 1; }

OPHANDLER( rr_a )           { cpustate->a = (cpustate->a >> 1) | (cpustate->a << 7); return 1; }
OPHANDLER( rrc_a )          { UINT8 newc = (cpustate->a << 7) & C_FLAG; cpustate->a = (cpustate->a >> 1) | (cpustate->psw & C_FLAG); cpustate->psw = (cpustate->psw & ~C_FLAG) | newc; return 1; }

OPHANDLER( sel_mb0 )        { cpustate->a11 = 0x000; return 1; }
OPHANDLER( sel_mb1 )        { cpustate->a11 = 0x800; return 1; }

OPHANDLER( sel_rb0 )        { cpustate->psw &= ~B_FLAG; update_regptr(cpustate); return 1; }
OPHANDLER( sel_rb1 )        { cpustate->psw |=  B_FLAG; update_regptr(cpustate); return 1; }

OPHANDLER( stop_tcnt )      { cpustate->timecount_enabled = 0; return 1; }

OPHANDLER( strt_cnt )       { cpustate->timecount_enabled = COUNTER_ENABLED; cpustate->t1_history = test_r(1); return 1; }
OPHANDLER( strt_t )         { cpustate->timecount_enabled = TIMER_ENABLED; cpustate->prescaler = 0; return 1; }

OPHANDLER( swap_a )         { cpustate->a = (cpustate->a << 4) | (cpustate->a >> 4); return 1; }

OPHANDLER( xch_a_r0 )       { UINT8 tmp = cpustate->a; cpustate->a = reg_r(0); reg_w(0, tmp); return 1; }
OPHANDLER( xch_a_r1 )       { UINT8 tmp = cpustate->a; cpustate->a = reg_r(1); reg_w(1, tmp); return 1; }
OPHANDLER( xch_a_r2 )       { UINT8 tmp = cpustate->a; cpustate->a = reg_r(2); reg_w(2, tmp); return 1; }
OPHANDLER( xch_a_r3 )       { UINT8 tmp = cpustate->a; cpustate->a = reg_r(3); reg_w(3, tmp); return 1; }
OPHANDLER( xch_a_r4 )       { UINT8 tmp = cpustate->a; cpustate->a = reg_r(4); reg_w(4, tmp); return 1; }
OPHANDLER( xch_a_r5 )       { UINT8 tmp = cpustate->a; cpustate->a = reg_r(5); reg_w(5, tmp); return 1; }
OPHANDLER( xch_a_r6 )       { UINT8 tmp = cpustate->a; cpustate->a = reg_r(6); reg_w(6, tmp); return 1; }
OPHANDLER( xch_a_r7 )       { UINT8 tmp = cpustate->a; cpustate->a = reg_r(7); reg_w(7, tmp); return 1; }
OPHANDLER( xch_a_xr0 )      { UINT8 r0 = reg_r(0); UINT8 tmp = cpustate->a; cpustate->a = ram_r(r0); ram_w(r0, tmp); return 1; }
OPHANDLER( xch_a_xr1 )      { UINT8 r1 = reg_r(1); UINT8 tmp = cpustate->a; cpustate->a = ram_r(r1); ram_w(r1, tmp); return 1; }

OPHANDLER( xchd_a_xr0 )     { UINT8 r0 = reg_r(0); UINT8 oldram = ram_r(r0); ram_w(r0, (oldram & 0xf0) | (cpustate->a & 0x0f)); cpustate->a = (cpustate->a & 0xf0) | (oldram & 0x0f); return 1; }
OPHANDLER( xchd_a_xr1 )     { UINT8 r1 = reg_r(1); UINT8 oldram = ram_r(r1); ram_w(r1, (oldram & 0xf0) | (cpustate->a & 0x0f)); cpustate->a = (cpustate->a & 0xf0) | (oldram & 0x0f); return 1; }

OPHANDLER( xrl_a_r0 )       { cpustate->a ^= reg_r(0); return 1; }
OPHANDLER( xrl_a_r1 )       { cpustate->a ^= reg_r(1); return 1; }
OPHANDLER( xrl_a_r2 )       { cpustate->a ^= reg_r(2); return 1; }
OPHANDLER( xrl_a_r3 )       { cpustate->a ^= reg_r(3); return 1; }
OPHANDLER( xrl_a_r4 )       { cpustate->a ^= reg_r(4); return 1; }
OPHANDLER( xrl_a_r5 )       { cpustate->a ^= reg_r(5); return 1; }
OPHANDLER( xrl_a_r6 )       { cpustate->a ^= reg_r(6); return 1; }
OPHANDLER( xrl_a_r7 )       { cpustate->a ^= reg_r(7); return 1; }
OPHANDLER( xrl_a_xr0 )      { cpustate->a ^= ram_r(reg_r(0)); return 1; }
OPHANDLER( xrl_a_xr1 )      { cpustate->a ^= ram_r(reg_r(1)); return 1; }
OPHANDLER( xrl_a_n )        { cpustate->a ^= argument_fetch(cpustate); return 2; }

/***************************************************************************
    OPCODE TABLES
***************************************************************************/

static const mcs48_ophandler opcode_table[256]=
{
	nop,        illegal,    outl_bus_a,add_a_n,   jmp_0,     en_i,       illegal,   dec_a,         /* 00 */
	ins_a_bus,  in_a_p1,    in_a_p2,   illegal,   movd_a_p4, movd_a_p5,  movd_a_p6, movd_a_p7,
	inc_xr0,    inc_xr1,    jb_0,      adc_a_n,   call_0,    dis_i,      jtf,       inc_a,         /* 10 */
	inc_r0,     inc_r1,     inc_r2,    inc_r3,    inc_r4,    inc_r5,     inc_r6,    inc_r7,
	xch_a_xr0,  xch_a_xr1,  illegal,   mov_a_n,   jmp_1,     en_tcnti,   jnt_0,     clr_a,         /* 20 */
	xch_a_r0,   xch_a_r1,   xch_a_r2,  xch_a_r3,  xch_a_r4,  xch_a_r5,   xch_a_r6,  xch_a_r7,
	xchd_a_xr0, xchd_a_xr1, jb_1,      illegal,   call_1,    dis_tcnti,  jt_0,      cpl_a,         /* 30 */
	illegal,    outl_p1_a,  outl_p2_a, illegal,   movd_p4_a, movd_p5_a,  movd_p6_a, movd_p7_a,
	orl_a_xr0,  orl_a_xr1,  mov_a_t,   orl_a_n,   jmp_2,     strt_cnt,   jnt_1,     swap_a,        /* 40 */
	orl_a_r0,   orl_a_r1,   orl_a_r2,  orl_a_r3,  orl_a_r4,  orl_a_r5,   orl_a_r6,  orl_a_r7,
	anl_a_xr0,  anl_a_xr1,  jb_2,      anl_a_n,   call_2,    strt_t,     jt_1,      da_a,          /* 50 */
	anl_a_r0,   anl_a_r1,   anl_a_r2,  anl_a_r3,  anl_a_r4,  anl_a_r5,   anl_a_r6,  anl_a_r7,
	add_a_xr0,  add_a_xr1,  mov_t_a,   illegal,   jmp_3,     stop_tcnt,  illegal,   rrc_a,         /* 60 */
	add_a_r0,   add_a_r1,   add_a_r2,  add_a_r3,  add_a_r4,  add_a_r5,   add_a_r6,  add_a_r7,
	adc_a_xr0,  adc_a_xr1,  jb_3,      illegal,   call_3,    ent0_clk,   jf1,       rr_a,          /* 70 */
	adc_a_r0,   adc_a_r1,   adc_a_r2,  adc_a_r3,  adc_a_r4,  adc_a_r5,   adc_a_r6,  adc_a_r7,
	movx_a_xr0, movx_a_xr1, illegal,   ret,       jmp_4,     clr_f0,     jni,       illegal,       /* 80 */
	orl_bus_n,  orl_p1_n,   orl_p2_n,  illegal,   orld_p4_a, orld_p5_a,  orld_p6_a, orld_p7_a,
	movx_xr0_a, movx_xr1_a, jb_4,      retr,      call_4,    cpl_f0,     jnz,       clr_c,         /* 90 */
	anl_bus_n,  anl_p1_n,   anl_p2_n,  illegal,   anld_p4_a, anld_p5_a,  anld_p6_a, anld_p7_a,
	mov_xr0_a,  mov_xr1_a,  illegal,   movp_a_xa, jmp_5,     clr_f1,     illegal,   cpl_c,         /* A0 */
	mov_r0_a,   mov_r1_a,   mov_r2_a,  mov_r3_a,  mov_r4_a,  mov_r5_a,   mov_r6_a,  mov_r7_a,
	mov_xr0_n,  mov_xr1_n,  jb_5,      jmpp_xa,   call_5,    cpl_f1,     jf0,       illegal,       /* B0 */
	mov_r0_n,   mov_r1_n,   mov_r2_n,  mov_r3_n,  mov_r4_n,  mov_r5_n,   mov_r6_n,  mov_r7_n,
	illegal,    illegal,    illegal,   illegal,   jmp_6,     sel_rb0,    jz,        mov_a_psw,     /* C0 */
	dec_r0,     dec_r1,     dec_r2,    dec_r3,    dec_r4,    dec_r5,     dec_r6,    dec_r7,
	xrl_a_xr0,  xrl_a_xr1,  jb_6,      xrl_a_n,   call_6,    sel_rb1,    illegal,   mov_psw_a,     /* D0 */
	xrl_a_r0,   xrl_a_r1,   xrl_a_r2,  xrl_a_r3,  xrl_a_r4,  xrl_a_r5,   xrl_a_r6,  xrl_a_r7,
	illegal,    illegal,    illegal,   movp3_a_xa,jmp_7,     sel_mb0,    jnc,       rl_a,          /* E0 */
	djnz_r0,    djnz_r1,    djnz_r2,   djnz_r3,   djnz_r4,   djnz_r5,    djnz_r6,   djnz_r7,
	mov_a_xr0,  mov_a_xr1,  jb_7,      illegal,   call_7,    sel_mb1,    jc,        rlc_a,         /* F0 */
	mov_a_r0,   mov_a_r1,   mov_a_r2,  mov_a_r3,  mov_a_r4,  mov_a_r5,   mov_a_r6,  mov_a_r7
};

/***************************************************************************
    INITIALIZATION/RESET
***************************************************************************/

void MCS48::initialize()
{
	opaque = calloc(1, sizeof(mcs48_state));
	
	mcs48_state *cpustate = (mcs48_state *)opaque;
	
	cpustate->mem = d_mem;
	cpustate->io = d_io;
	cpustate->intr = d_intr;
#ifdef USE_DEBUGGER
	d_mem_stored = d_mem;
	d_io_stored = d_io;
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void MCS48::release()
{
	free(opaque);
}

void MCS48::reset()
{
	mcs48_state *cpustate = (mcs48_state *)opaque;
	
	/* confirmed from reset description */
	cpustate->pc = 0;
	cpustate->psw = (cpustate->psw & (C_FLAG | A_FLAG)) | 0x08;
	cpustate->a11 = 0x000;
//	bus_w(0xff);
	cpustate->p1 = 0xff;
	cpustate->p2 = 0xff;
//	port_w(1, cpustate->p1);
//	port_w(2, cpustate->p2);
	cpustate->tirq_enabled = FALSE;
	cpustate->xirq_enabled = FALSE;
	cpustate->t0_clk_enabled = FALSE;
	cpustate->timecount_enabled = 0;
	cpustate->timer_flag = FALSE;
	cpustate->sts = 0;
	
	cpustate->icount = 0;
	
	/* confirmed from interrupt logic description */
	cpustate->int_state = TRUE;
	cpustate->irq_state = cpustate->irq_in_progress = FALSE;
	cpustate->timer_overflow = FALSE;
}

void MCS48::load_rom_image(_TCHAR *file_path)
{
	mcs48_state *cpustate = (mcs48_state *)opaque;
	
	memset(cpustate->rom, 0, sizeof(cpustate->rom));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(cpustate->rom, sizeof(cpustate->rom), 1);
		fio->Fclose();
	}
	delete fio;
}

uint8 *MCS48::get_rom_ptr()
{
	mcs48_state *cpustate = (mcs48_state *)opaque;
	return cpustate->rom;
}

/***************************************************************************
    EXECUTION
***************************************************************************/

/*-------------------------------------------------
    check_irqs - check for and process IRQs
-------------------------------------------------*/

static int check_irqs(mcs48_state *cpustate)
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
			cpustate->intr->intr_ack();
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

/*-------------------------------------------------
    burn_cycles - burn cycles, processing timers
    and counters
-------------------------------------------------*/

static void burn_cycles(mcs48_state *cpustate, int count)
{
	int timerover = FALSE;

	/* output (clock*15/3) hz to t0 */
	if (cpustate->t0_clk_enabled)
	{
		for(int i = 0; i < count * 5; i++)
		{
			test_w(0, 1);
			test_w(0, 0);
		}
	}

	/* if the timer is enabled, accumulate prescaler cycles */
	if (cpustate->timecount_enabled & TIMER_ENABLED)
	{
		UINT8 oldtimer = cpustate->timer;
		cpustate->prescaler += count;
		cpustate->timer += cpustate->prescaler >> 5;
		cpustate->prescaler &= 0x1f;
		timerover = (oldtimer != 0 && cpustate->timer == 0);
	}

	/* if the counter is enabled, poll the T1 test input once for each cycle */
	else if (cpustate->timecount_enabled & COUNTER_ENABLED)
		for ( ; count > 0; count--)
		{
			cpustate->t1_history = (cpustate->t1_history << 1) | (test_r(1) & 1);
			if ((cpustate->t1_history & 3) == 2)
				timerover = (++cpustate->timer == 0);
		}

	/* if either source caused a timer overflow, set the flags and check IRQs */
	if (timerover)
	{
		cpustate->timer_flag = TRUE;

		/* according to the docs, if an overflow occurs with interrupts disabled, the overflow is not stored */
		if (cpustate->tirq_enabled)
		{
			cpustate->timer_overflow = TRUE;
			check_irqs(cpustate);
		}
	}
}

int MCS48::run(int icount)
{
	mcs48_state *cpustate = (mcs48_state *)opaque;
	int curcycles;
	
	if (icount == -1) {
		cpustate->icount = 1;
	} else {
		cpustate->icount += icount;
		if (cpustate->icount < 0) {
			return 0;
		}
	}
	
	int base_icount = cpustate->icount;
	
	update_regptr(cpustate);
	
	/* external interrupts may have been set since we last checked */
	curcycles = check_irqs(cpustate);
	cpustate->icount -= curcycles * 15;
	if (cpustate->timecount_enabled != 0)
		burn_cycles(cpustate, curcycles);

	/* iterate over remaining cycles, guaranteeing at least one instruction */
	do
	{
#ifdef USE_DEBUGGER
		bool now_debugging = d_debugger->now_debugging;
		if(now_debugging) {
			d_debugger->check_break_points(cpustate->pc);
			if(d_debugger->now_suspended) {
				emu->mute_sound();
				while(d_debugger->now_debugging && d_debugger->now_suspended) {
					Sleep(10);
				}
			}
			if(d_debugger->now_debugging) {
				d_mem = d_io = d_debugger;
			} else {
				now_debugging = false;
			}
			
			/* fetch next opcode */
			cpustate->prevpc = cpustate->pc;
			unsigned opcode = opcode_fetch(cpustate);

			/* process opcode and count cycles */
			curcycles = (*opcode_table[opcode])(cpustate);

			/* burn the cycles */
			cpustate->icount -= curcycles * 15;
			if (cpustate->timecount_enabled != 0)
				burn_cycles(cpustate, curcycles);
			
			if(now_debugging) {
				if(!d_debugger->now_going) {
					d_debugger->now_suspended = true;
				}
				d_mem = d_mem_stored;
				d_io = d_io_stored;
			}
		} else {
#endif
			/* fetch next opcode */
			cpustate->prevpc = cpustate->pc;
			unsigned opcode = opcode_fetch(cpustate);

			/* process opcode and count cycles */
			curcycles = (*opcode_table[opcode])(cpustate);

			/* burn the cycles */
			cpustate->icount -= curcycles * 15;
			if (cpustate->timecount_enabled != 0)
				burn_cycles(cpustate, curcycles);
#ifdef USE_DEBUGGER
		}
#endif
	} while (cpustate->icount > 0);
	
	return base_icount - cpustate->icount;
}

uint32 MCS48::get_pc()
{
	mcs48_state *cpustate = (mcs48_state *)opaque;
	return cpustate->prevpc;
}

uint32 MCS48::get_next_pc()
{
	mcs48_state *cpustate = (mcs48_state *)opaque;
	return cpustate->pc;
}

/***************************************************************************
    GENERAL CONTEXT ACCESS
***************************************************************************/

void MCS48::write_signal(int id, uint32 data, uint32 mask)
{
	mcs48_state *cpustate = (mcs48_state *)opaque;
	
	if(id == SIG_CPU_IRQ) {
		UINT8 prev = cpustate->int_state;
		cpustate->int_state = ((data & mask) != 0);
		// INT H->L
		if(prev && !cpustate->int_state) {
			cpustate->irq_state = TRUE;
		}
	}
}

#ifdef USE_DEBUGGER
void MCS48::debug_write_data8(uint32 addr, uint32 data)
{
	d_mem_stored->write_data8(addr, data);
}

uint32 MCS48::debug_read_data8(uint32 addr)
{
	return d_mem_stored->read_data8(addr);
}

void MCS48::debug_write_io8(uint32 addr, uint32 data)
{
	d_io_stored->write_io8(addr, data);
}

uint32 MCS48::debug_read_io8(uint32 addr)
{
	return d_io_stored->read_io8(addr);
}

bool MCS48::debug_write_reg(_TCHAR *reg, uint32 data)
{
	mcs48_state *cpustate = (mcs48_state *)opaque;
	
	if(_tcsicmp(reg, _T("R0")) == 0) {
		reg_w(0, data);
	} else if(_tcsicmp(reg, _T("R1")) == 0) {
		reg_w(1, data);
	} else if(_tcsicmp(reg, _T("R2")) == 0) {
		reg_w(2, data);
	} else if(_tcsicmp(reg, _T("R3")) == 0) {
		reg_w(3, data);
	} else if(_tcsicmp(reg, _T("R4")) == 0) {
		reg_w(4, data);
	} else if(_tcsicmp(reg, _T("R5")) == 0) {
		reg_w(5, data);
	} else if(_tcsicmp(reg, _T("R6")) == 0) {
		reg_w(6, data);
	} else if(_tcsicmp(reg, _T("R7")) == 0) {
		reg_w(7, data);
	} else {
		return false;
	}
	return true;
}

void MCS48::debug_regs_info(_TCHAR *buffer)
{
/*
R0 = 00  R1 = 00  R2 = 00  R3 = 00 (R0)= 00 (R1)= 00 (SP-1)= 0000  PC = 0000
R4 = 00  R5 = 00  R6 = 00  R7 = 00  AC = 00  SP = 00 [MB F1 C AC F0 BS]
*/
	mcs48_state *cpustate = (mcs48_state *)opaque;
	UINT8 sp = 8 + 2 * (cpustate->psw & 7);
	UINT8 prev_sp = 8 + 2 * ((cpustate->psw - 1) & 7);
	
	_stprintf(buffer, _T("R0 = %02X  R1 = %02X  R2 = %02X  R3 = %02X (R0)= %02X (R1)= %02X (SP-1)= %04X  PC = %04X\nR4 = %02X  R5 = %02X  R6 = %02X  R7 = %02X  AC = %02X  SP = %02X [%s %s %s %s %s %s]"),
	reg_r(0), reg_r(1), reg_r(2), reg_r(3), d_mem_stored->read_data8(reg_r(0)), d_mem_stored->read_data8(reg_r(1)),
	d_mem_stored->read_data8(prev_sp) | (d_mem_stored->read_data8(prev_sp + 1) << 8), cpustate->pc,
	reg_r(4), reg_r(5), reg_r(6), reg_r(7), cpustate->a, sp,
	(cpustate->a11 == 0x800) ? _T("MB") : _T("--"), (cpustate->sts & STS_F1) ? _T("F1") : _T("--"),
	(cpustate->psw & C_FLAG) ? _T("C" ) : _T("-" ), (cpustate->psw & A_FLAG) ? _T("AC") : _T("--"),
	(cpustate->psw & F_FLAG) ? _T("F0") : _T("--"), (cpustate->psw & B_FLAG) ? _T("BS") : _T("--"));
}

// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mcs48dsm.c

    Simple MCS-48/UPI-41 disassembler.
    Written by Aaron Giles

***************************************************************************/

int MCS48::debug_dasm(uint32 pc, _TCHAR *buffer)
{
	mcs48_state *cpustate = (mcs48_state *)opaque;
	uint32 ptr = pc;
	
	#define upi41 false
	
	switch (program_r(ptr++))
	{
		case 0x00:      _stprintf(buffer, _T("nop"));                                                 break;
		case 0x02:  if (!upi41)
		                _stprintf(buffer, _T("out  bus,a"));
		            else
		                _stprintf(buffer, _T("out  dbb,a"));                                          break;
		case 0x03:      _stprintf(buffer, _T("add  a,#$%02X"), program_r(ptr++));                     break;
		case 0x04:      _stprintf(buffer, _T("jmp  $0%02X"), program_r(ptr++));                       break;
		case 0x05:      _stprintf(buffer, _T("en   i"));                                              break;
		case 0x07:      _stprintf(buffer, _T("dec  a"));                                              break;
		case 0x08:  if (!upi41)
		                _stprintf(buffer, _T("in   a,bus"));
		            else
		                _stprintf(buffer, _T("illegal"));                                             break;
		case 0x09:      _stprintf(buffer, _T("in   a,p1"));                                           break;
		case 0x0a:      _stprintf(buffer, _T("in   a,p2"));                                           break;
		case 0x0c:      _stprintf(buffer, _T("movd a,p4"));                                           break;
		case 0x0d:      _stprintf(buffer, _T("movd a,p5"));                                           break;
		case 0x0e:      _stprintf(buffer, _T("movd a,p6"));                                           break;
		case 0x0f:      _stprintf(buffer, _T("movd a,p7"));                                           break;
		case 0x10:      _stprintf(buffer, _T("inc  @r0"));                                            break;
		case 0x11:      _stprintf(buffer, _T("inc  @r1"));                                            break;
		case 0x12:      _stprintf(buffer, _T("jb0  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x13:      _stprintf(buffer, _T("addc a,#$%02X"), program_r(ptr++));                     break;
		case 0x14:      _stprintf(buffer, _T("call $0%02X"), program_r(ptr++));                       break;
		case 0x15:      _stprintf(buffer, _T("dis  i"));                                              break;
		case 0x16:      _stprintf(buffer, _T("jtf  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x17:      _stprintf(buffer, _T("inc  a"));                                              break;
		case 0x18:      _stprintf(buffer, _T("inc  r0"));                                             break;
		case 0x19:      _stprintf(buffer, _T("inc  r1"));                                             break;
		case 0x1a:      _stprintf(buffer, _T("inc  r2"));                                             break;
		case 0x1b:      _stprintf(buffer, _T("inc  r3"));                                             break;
		case 0x1c:      _stprintf(buffer, _T("inc  r4"));                                             break;
		case 0x1d:      _stprintf(buffer, _T("inc  r5"));                                             break;
		case 0x1e:      _stprintf(buffer, _T("inc  r6"));                                             break;
		case 0x1f:      _stprintf(buffer, _T("inc  r7"));                                             break;
		case 0x20:      _stprintf(buffer, _T("xch  a,@r0"));                                          break;
		case 0x21:      _stprintf(buffer, _T("xch  a,@r1"));                                          break;
		case 0x22:  if (!upi41)
		                _stprintf(buffer, _T("illegal"));
		            else
		                _stprintf(buffer, _T("in   a,dbb"));                                          break;
		case 0x23:      _stprintf(buffer, _T("mov  a,#$%02X"), program_r(ptr++));                     break;
		case 0x24:      _stprintf(buffer, _T("jmp  $1%02X"), program_r(ptr++));                       break;
		case 0x25:      _stprintf(buffer, _T("en   tcnti"));                                          break;
		case 0x26:      _stprintf(buffer, _T("jnt0 $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x27:      _stprintf(buffer, _T("clr  a"));                                              break;
		case 0x28:      _stprintf(buffer, _T("xch  a,r0"));                                           break;
		case 0x29:      _stprintf(buffer, _T("xch  a,r1"));                                           break;
		case 0x2a:      _stprintf(buffer, _T("xch  a,r2"));                                           break;
		case 0x2b:      _stprintf(buffer, _T("xch  a,r3"));                                           break;
		case 0x2c:      _stprintf(buffer, _T("xch  a,r4"));                                           break;
		case 0x2d:      _stprintf(buffer, _T("xch  a,r5"));                                           break;
		case 0x2e:      _stprintf(buffer, _T("xch  a,r6"));                                           break;
		case 0x2f:      _stprintf(buffer, _T("xch  a,r7"));                                           break;
		case 0x30:      _stprintf(buffer, _T("xchd a,@r0"));                                          break;
		case 0x31:      _stprintf(buffer, _T("xchd a,@r1"));                                          break;
		case 0x32:      _stprintf(buffer, _T("jb1  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x34:      _stprintf(buffer, _T("call $1%02X"), program_r(ptr++));                       break;
		case 0x35:      _stprintf(buffer, _T("dis  tcnti"));                                          break;
		case 0x36:      _stprintf(buffer, _T("jt0  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x37:      _stprintf(buffer, _T("cpl  a"));                                              break;
		case 0x39:      _stprintf(buffer, _T("outl p1,a"));                                           break;
		case 0x3a:      _stprintf(buffer, _T("outl p2,a"));                                           break;
		case 0x3c:      _stprintf(buffer, _T("movd p4,a"));                                           break;
		case 0x3d:      _stprintf(buffer, _T("movd p5,a"));                                           break;
		case 0x3e:      _stprintf(buffer, _T("movd p6,a"));                                           break;
		case 0x3f:      _stprintf(buffer, _T("movd p7,a"));                                           break;
		case 0x40:      _stprintf(buffer, _T("orl  a,@r0"));                                          break;
		case 0x41:      _stprintf(buffer, _T("orl  a,@r1"));                                          break;
		case 0x42:      _stprintf(buffer, _T("mov  a,t"));                                            break;
		case 0x43:      _stprintf(buffer, _T("orl  a,#$%02X"), program_r(ptr++));                     break;
		case 0x44:      _stprintf(buffer, _T("jmp  $2%02X"), program_r(ptr++));                       break;
		case 0x45:      _stprintf(buffer, _T("strt cnt"));                                            break;
		case 0x46:      _stprintf(buffer, _T("jnt1 $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x47:      _stprintf(buffer, _T("swap a"));                                              break;
		case 0x48:      _stprintf(buffer, _T("orl  a,r0"));                                           break;
		case 0x49:      _stprintf(buffer, _T("orl  a,r1"));                                           break;
		case 0x4a:      _stprintf(buffer, _T("orl  a,r2"));                                           break;
		case 0x4b:      _stprintf(buffer, _T("orl  a,r3"));                                           break;
		case 0x4c:      _stprintf(buffer, _T("orl  a,r4"));                                           break;
		case 0x4d:      _stprintf(buffer, _T("orl  a,r5"));                                           break;
		case 0x4e:      _stprintf(buffer, _T("orl  a,r6"));                                           break;
		case 0x4f:      _stprintf(buffer, _T("orl  a,r7"));                                           break;
		case 0x50:      _stprintf(buffer, _T("anl  a,@r0"));                                          break;
		case 0x51:      _stprintf(buffer, _T("anl  a,@r1"));                                          break;
		case 0x52:      _stprintf(buffer, _T("jb2  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x53:      _stprintf(buffer, _T("anl  a,#$%02X"), program_r(ptr++));                     break;
		case 0x54:      _stprintf(buffer, _T("call $2%02X"), program_r(ptr++));                       break;
		case 0x55:      _stprintf(buffer, _T("strt t"));                                              break;
		case 0x56:      _stprintf(buffer, _T("jt1  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x57:      _stprintf(buffer, _T("da   a"));                                              break;
		case 0x58:      _stprintf(buffer, _T("anl  a,r0"));                                           break;
		case 0x59:      _stprintf(buffer, _T("anl  a,r1"));                                           break;
		case 0x5a:      _stprintf(buffer, _T("anl  a,r2"));                                           break;
		case 0x5b:      _stprintf(buffer, _T("anl  a,r3"));                                           break;
		case 0x5c:      _stprintf(buffer, _T("anl  a,r4"));                                           break;
		case 0x5d:      _stprintf(buffer, _T("anl  a,r5"));                                           break;
		case 0x5e:      _stprintf(buffer, _T("anl  a,r6"));                                           break;
		case 0x5f:      _stprintf(buffer, _T("anl  a,r7"));                                           break;
		case 0x60:      _stprintf(buffer, _T("add  a,@r0"));                                          break;
		case 0x61:      _stprintf(buffer, _T("add  a,@r1"));                                          break;
		case 0x62:      _stprintf(buffer, _T("mov  t,a"));                                            break;
		case 0x64:      _stprintf(buffer, _T("jmp  $3%02X"), program_r(ptr++));                       break;
		case 0x65:      _stprintf(buffer, _T("stop tcnt"));                                           break;
		case 0x67:      _stprintf(buffer, _T("rrc  a"));                                              break;
		case 0x68:      _stprintf(buffer, _T("add  a,r0"));                                           break;
		case 0x69:      _stprintf(buffer, _T("add  a,r1"));                                           break;
		case 0x6a:      _stprintf(buffer, _T("add  a,r2"));                                           break;
		case 0x6b:      _stprintf(buffer, _T("add  a,r3"));                                           break;
		case 0x6c:      _stprintf(buffer, _T("add  a,r4"));                                           break;
		case 0x6d:      _stprintf(buffer, _T("add  a,r5"));                                           break;
		case 0x6e:      _stprintf(buffer, _T("add  a,r6"));                                           break;
		case 0x6f:      _stprintf(buffer, _T("add  a,r7"));                                           break;
		case 0x70:      _stprintf(buffer, _T("addc a,@r0"));                                          break;
		case 0x71:      _stprintf(buffer, _T("addc a,@r1"));                                          break;
		case 0x72:      _stprintf(buffer, _T("jb3  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x74:      _stprintf(buffer, _T("call $3%02X"), program_r(ptr++));                       break;
		case 0x75:  if (!upi41)
		                _stprintf(buffer, _T("ent0 clk"));
		            else
		                _stprintf(buffer, _T("illegal"));                                             break;
		case 0x76:      _stprintf(buffer, _T("jf1  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x77:      _stprintf(buffer, _T("rr   a"));                                              break;
		case 0x78:      _stprintf(buffer, _T("addc a,r0"));                                           break;
		case 0x79:      _stprintf(buffer, _T("addc a,r1"));                                           break;
		case 0x7a:      _stprintf(buffer, _T("addc a,r2"));                                           break;
		case 0x7b:      _stprintf(buffer, _T("addc a,r3"));                                           break;
		case 0x7c:      _stprintf(buffer, _T("addc a,r4"));                                           break;
		case 0x7d:      _stprintf(buffer, _T("addc a,r5"));                                           break;
		case 0x7e:      _stprintf(buffer, _T("addc a,r6"));                                           break;
		case 0x7f:      _stprintf(buffer, _T("addc a,r7"));                                           break;
		case 0x80:  if (!upi41)
		                _stprintf(buffer, _T("movx a,@r0"));
		            else
		                _stprintf(buffer, _T("illegal"));                                             break;
		case 0x81:  if (!upi41)
		                _stprintf(buffer, _T("movx a,@r1"));
		            else
		                _stprintf(buffer, _T("illegal"));                                             break;
		case 0x83:      _stprintf(buffer, _T("ret"));                                                 break;
		case 0x84:      _stprintf(buffer, _T("jmp  $4%02X"), program_r(ptr++));                       break;
		case 0x85:      _stprintf(buffer, _T("clr  f0"));                                             break;
		case 0x86:  if (!upi41)
		                _stprintf(buffer, _T("jni  $%03X"), (pc & 0xf00) | program_r(ptr++));
		            else
		                _stprintf(buffer, _T("jobf $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x88:  if (!upi41)
		                _stprintf(buffer, _T("orl  bus,#$%02X"), program_r(ptr++));
		            else
		                _stprintf(buffer, _T("illegal"));                                             break;
		case 0x89:      _stprintf(buffer, _T("orl  p1,#$%02X"), program_r(ptr++));                    break;
		case 0x8a:      _stprintf(buffer, _T("orl  p2,#$%02X"), program_r(ptr++));                    break;
		case 0x8c:      _stprintf(buffer, _T("orld p4,a"));                                           break;
		case 0x8d:      _stprintf(buffer, _T("orld p5,a"));                                           break;
		case 0x8e:      _stprintf(buffer, _T("orld p6,a"));                                           break;
		case 0x8f:      _stprintf(buffer, _T("orld p7,a"));                                           break;
		case 0x90:  if (!upi41)
		                _stprintf(buffer, _T("movx @r0,a"));
		            else
		                _stprintf(buffer, _T("mov  sts,a"));                                          break;
		case 0x91:  if (!upi41)
		                _stprintf(buffer, _T("movx @r1,a"));
		            else
		                _stprintf(buffer, _T("illegal"));                                             break;
		case 0x92:      _stprintf(buffer, _T("jb4  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x93:      _stprintf(buffer, _T("retr"));                                                break;
		case 0x94:      _stprintf(buffer, _T("call $4%02X"), program_r(ptr++));                       break;
		case 0x95:      _stprintf(buffer, _T("cpl  f0"));                                             break;
		case 0x96:      _stprintf(buffer, _T("jnz  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0x97:      _stprintf(buffer, _T("clr  c"));                                              break;
		case 0x98:  if (!upi41)
		                _stprintf(buffer, _T("anl  bus,#$%02X"), program_r(ptr++));
		            else
		                _stprintf(buffer, _T("illegal"));                                             break;
		case 0x99:      _stprintf(buffer, _T("anl  p1,#$%02X"), program_r(ptr++));                    break;
		case 0x9a:      _stprintf(buffer, _T("anl  p2,#$%02X"), program_r(ptr++));                    break;
		case 0x9c:      _stprintf(buffer, _T("anld p4,a"));                                           break;
		case 0x9d:      _stprintf(buffer, _T("anld p5,a"));                                           break;
		case 0x9e:      _stprintf(buffer, _T("anld p6,a"));                                           break;
		case 0x9f:      _stprintf(buffer, _T("anld p7,a"));                                           break;
		case 0xa0:      _stprintf(buffer, _T("mov  @r0,a"));                                          break;
		case 0xa1:      _stprintf(buffer, _T("mov  @r1,a"));                                          break;
		case 0xa3:      _stprintf(buffer, _T("movp a,@a"));                                           break;
		case 0xa4:      _stprintf(buffer, _T("jmp  $5%02X"), program_r(ptr++));                       break;
		case 0xa5:      _stprintf(buffer, _T("clr  f1"));                                             break;
		case 0xa7:      _stprintf(buffer, _T("cpl  c"));                                              break;
		case 0xa8:      _stprintf(buffer, _T("mov  r0,a"));                                           break;
		case 0xa9:      _stprintf(buffer, _T("mov  r1,a"));                                           break;
		case 0xaa:      _stprintf(buffer, _T("mov  r2,a"));                                           break;
		case 0xab:      _stprintf(buffer, _T("mov  r3,a"));                                           break;
		case 0xac:      _stprintf(buffer, _T("mov  r4,a"));                                           break;
		case 0xad:      _stprintf(buffer, _T("mov  r5,a"));                                           break;
		case 0xae:      _stprintf(buffer, _T("mov  r6,a"));                                           break;
		case 0xaf:      _stprintf(buffer, _T("mov  r7,a"));                                           break;
		case 0xb0:      _stprintf(buffer, _T("mov  @r0,#$%02X"), program_r(ptr++));                   break;
		case 0xb1:      _stprintf(buffer, _T("mov  @r1,#$%02X"), program_r(ptr++));                   break;
		case 0xb2:      _stprintf(buffer, _T("jb5  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0xb3:      _stprintf(buffer, _T("jmpp @a"));                                             break;
		case 0xb4:      _stprintf(buffer, _T("call $5%02X"), program_r(ptr++));                       break;
		case 0xb5:      _stprintf(buffer, _T("cpl  f1"));                                             break;
		case 0xb6:      _stprintf(buffer, _T("jf0  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0xb8:      _stprintf(buffer, _T("mov  r0,#$%02X"), program_r(ptr++));                    break;
		case 0xb9:      _stprintf(buffer, _T("mov  r1,#$%02X"), program_r(ptr++));                    break;
		case 0xba:      _stprintf(buffer, _T("mov  r2,#$%02X"), program_r(ptr++));                    break;
		case 0xbb:      _stprintf(buffer, _T("mov  r3,#$%02X"), program_r(ptr++));                    break;
		case 0xbc:      _stprintf(buffer, _T("mov  r4,#$%02X"), program_r(ptr++));                    break;
		case 0xbd:      _stprintf(buffer, _T("mov  r5,#$%02X"), program_r(ptr++));                    break;
		case 0xbe:      _stprintf(buffer, _T("mov  r6,#$%02X"), program_r(ptr++));                    break;
		case 0xbf:      _stprintf(buffer, _T("mov  r7,#$%02X"), program_r(ptr++));                    break;
		case 0xc4:      _stprintf(buffer, _T("jmp  $6%02X"), program_r(ptr++));                       break;
		case 0xc5:      _stprintf(buffer, _T("sel  rb0"));                                            break;
		case 0xc6:      _stprintf(buffer, _T("jz   $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0xc7:      _stprintf(buffer, _T("mov  a,psw"));                                          break;
		case 0xc8:      _stprintf(buffer, _T("dec  r0"));                                             break;
		case 0xc9:      _stprintf(buffer, _T("dec  r1"));                                             break;
		case 0xca:      _stprintf(buffer, _T("dec  r2"));                                             break;
		case 0xcb:      _stprintf(buffer, _T("dec  r3"));                                             break;
		case 0xcc:      _stprintf(buffer, _T("dec  r4"));                                             break;
		case 0xcd:      _stprintf(buffer, _T("dec  r5"));                                             break;
		case 0xce:      _stprintf(buffer, _T("dec  r6"));                                             break;
		case 0xcf:      _stprintf(buffer, _T("dec  r7"));                                             break;
		case 0xd0:      _stprintf(buffer, _T("xrl  a,@r0"));                                          break;
		case 0xd1:      _stprintf(buffer, _T("xrl  a,@r1"));                                          break;
		case 0xd2:      _stprintf(buffer, _T("jb6  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0xd3:      _stprintf(buffer, _T("xrl  a,#$%02X"), program_r(ptr++));                     break;
		case 0xd4:      _stprintf(buffer, _T("call $6%02X"), program_r(ptr++));                       break;
		case 0xd5:      _stprintf(buffer, _T("sel  rb1"));                                            break;
		case 0xd6:  if (!upi41)
		                _stprintf(buffer, _T("illegal"));
		            else
		                _stprintf(buffer, _T("jnibf $%03X"), (pc & 0xf00) | program_r(ptr++));        break;
		case 0xd7:      _stprintf(buffer, _T("mov  psw,a"));                                          break;
		case 0xd8:      _stprintf(buffer, _T("xrl  a,r0"));                                           break;
		case 0xd9:      _stprintf(buffer, _T("xrl  a,r1"));                                           break;
		case 0xda:      _stprintf(buffer, _T("xrl  a,r2"));                                           break;
		case 0xdb:      _stprintf(buffer, _T("xrl  a,r3"));                                           break;
		case 0xdc:      _stprintf(buffer, _T("xrl  a,r4"));                                           break;
		case 0xdd:      _stprintf(buffer, _T("xrl  a,r5"));                                           break;
		case 0xde:      _stprintf(buffer, _T("xrl  a,r6"));                                           break;
		case 0xdf:      _stprintf(buffer, _T("xrl  a,r7"));                                           break;
		case 0xe3:      _stprintf(buffer, _T("movp3 a,@a"));                                          break;
		case 0xe4:      _stprintf(buffer, _T("jmp  $7%02X"), program_r(ptr++));                       break;
		case 0xe5:  if (!upi41)
		                _stprintf(buffer, _T("sel  mb0"));
		            else
		                _stprintf(buffer, _T("en   dma"));                                            break;
		case 0xe6:      _stprintf(buffer, _T("jnc  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0xe7:      _stprintf(buffer, _T("rl   a"));                                              break;
		case 0xe8:      _stprintf(buffer, _T("djnz r0,$%03X"), (pc & 0xf00) | program_r(ptr++));      break;
		case 0xe9:      _stprintf(buffer, _T("djnz r1,$%03X"), (pc & 0xf00) | program_r(ptr++));      break;
		case 0xea:      _stprintf(buffer, _T("djnz r2,$%03X"), (pc & 0xf00) | program_r(ptr++));      break;
		case 0xeb:      _stprintf(buffer, _T("djnz r3,$%03X"), (pc & 0xf00) | program_r(ptr++));      break;
		case 0xec:      _stprintf(buffer, _T("djnz r4,$%03X"), (pc & 0xf00) | program_r(ptr++));      break;
		case 0xed:      _stprintf(buffer, _T("djnz r5,$%03X"), (pc & 0xf00) | program_r(ptr++));      break;
		case 0xee:      _stprintf(buffer, _T("djnz r6,$%03X"), (pc & 0xf00) | program_r(ptr++));      break;
		case 0xef:      _stprintf(buffer, _T("djnz r7,$%03X"), (pc & 0xf00) | program_r(ptr++));      break;
		case 0xf0:      _stprintf(buffer, _T("mov  a,@r0"));                                          break;
		case 0xf1:      _stprintf(buffer, _T("mov  a,@r1"));                                          break;
		case 0xf2:      _stprintf(buffer, _T("jb7  $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0xf4:      _stprintf(buffer, _T("call $7%02X"), program_r(ptr++));                       break;
		case 0xf5:  if (!upi41)
		                _stprintf(buffer, _T("sel  mb1"));
		            else
		                _stprintf(buffer, _T("en   flags"));                                          break;
		case 0xf6:      _stprintf(buffer, _T("jc   $%03X"), (pc & 0xf00) | program_r(ptr++));         break;
		case 0xf7:      _stprintf(buffer, _T("rlc  a"));                                              break;
		case 0xf8:      _stprintf(buffer, _T("mov  a,r0"));                                           break;
		case 0xf9:      _stprintf(buffer, _T("mov  a,r1"));                                           break;
		case 0xfa:      _stprintf(buffer, _T("mov  a,r2"));                                           break;
		case 0xfb:      _stprintf(buffer, _T("mov  a,r3"));                                           break;
		case 0xfc:      _stprintf(buffer, _T("mov  a,r4"));                                           break;
		case 0xfd:      _stprintf(buffer, _T("mov  a,r5"));                                           break;
		case 0xfe:      _stprintf(buffer, _T("mov  a,r6"));                                           break;
		case 0xff:      _stprintf(buffer, _T("mov  a,r7"));                                           break;
		default:        _stprintf(buffer, _T("illegal"));                                             break;
	}
	return ptr - pc;
}
#endif

#define STATE_VERSION	1

void MCS48MEM::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(ram, sizeof(ram), 1);
}

bool MCS48MEM::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(ram, sizeof(ram), 1);
	return true;
}

void MCS48::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(opaque, sizeof(mcs48_state), 1);
}

bool MCS48::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(opaque, sizeof(mcs48_state), 1);
	
	mcs48_state *cpustate = (mcs48_state *)opaque;
	cpustate->mem = d_mem;
	cpustate->io = d_io;
	cpustate->intr = d_intr;
	return true;
}

