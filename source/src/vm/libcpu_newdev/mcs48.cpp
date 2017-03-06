/*
	Skelton for retropc emulator

	Origin : MAME 0.148
	Author : Takeda.Toshiya
	Date   : 2013.05.01-

	[ MCS48 ]
*/
#include "../../fileio.h"
#include "./mcs48.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

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


extern int _mcs48_check_irqs(mcs48_state *cpustate);
extern void _mcs48_burn_cycles(mcs48_state *cpustate, int count);

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
    update_regptr - update the regptr member to
    point to the appropriate register bank
-------------------------------------------------*/

INLINE void update_regptr(mcs48_state *cpustate)
{
	cpustate->regptr = ((cpustate->psw & B_FLAG) ? 24 : 0);
}

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
	curcycles = _mcs48_check_irqs(cpustate);
	cpustate->icount -= curcycles * 15;
	if (cpustate->timecount_enabled != 0)
		_mcs48_burn_cycles(cpustate, curcycles);

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
					emu->sleep(10);
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
				_mcs48_burn_cycles(cpustate, curcycles);
			
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
				_mcs48_burn_cycles(cpustate, curcycles);
#ifdef USE_DEBUGGER
		}
#endif
	} while (cpustate->icount > 0);
	
	return base_icount - cpustate->icount;
}

