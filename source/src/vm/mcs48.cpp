/*
	Skelton for retropc emulator

	Origin : MAME 0.148
	Author : Takeda.Toshiya
	Date   : 2013.05.01-

	[ MCS48 ]
*/
#include "vm.h"
//#include "../emu.h"

#include "mcs48_flags.h"
#include "mcs48.h"
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

/***************************************************************************
    INITIALIZATION/RESET
***************************************************************************/

void MCS48::initialize()
{
	MCS48_BASE::initialize();
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

/***************************************************************************
    EXECUTION
***************************************************************************/


/*-------------------------------------------------
    burn_cycles - burn cycles, processing timers
    and counters
-------------------------------------------------*/

void MCS48::burn_cycles(mcs48_state *cpustate, int count)
{
	int timerover = FALSE;

	/* output (clock*15/3) hz to t0 */
	if (cpustate->t0_clk_enabled)
	{
		for(int i = 0; i < count * 5; i++)
		{
			__mcs48_test_w(0, 1);
			__mcs48_test_w(0, 0);
		}
	}

	/* if the timer is enabled, accumulate prescaler cycles */
	if (cpustate->timecount_enabled & __MCS48_TIMER_ENABLED)
	{
		UINT8 oldtimer = cpustate->timer;
		cpustate->prescaler += count;
		cpustate->timer += cpustate->prescaler >> 5;
		cpustate->prescaler &= 0x1f;
		timerover = (oldtimer != 0 && cpustate->timer == 0);
	}

	/* if the counter is enabled, poll the T1 test input once for each cycle */
	else if (cpustate->timecount_enabled & __MCS48_COUNTER_ENABLED)
		for ( ; count > 0; count--)
		{
			cpustate->t1_history = (cpustate->t1_history << 1) | (__mcs48_test_r(1) & 1);
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
#ifdef USE_DEBUGGER
	total_icount += curcycles * 15;
#endif

	/* iterate over remaining cycles, guaranteeing at least one instruction */
	do
	{
#ifdef USE_DEBUGGER
		bool now_debugging = d_debugger->now_debugging;
		if(now_debugging) {
			d_debugger->check_break_points(cpustate->pc);
			if(d_debugger->now_suspended) {
				d_debugger->now_waiting = true;
				emu->start_waiting_in_debugger();
				while(d_debugger->now_debugging && d_debugger->now_suspended) {
					emu->process_waiting_in_debugger();
				}
				emu->finish_waiting_in_debugger();
				d_debugger->now_waiting = false;
			}
			if(d_debugger->now_debugging) {
				d_mem = d_io = d_debugger;
			} else {
				now_debugging = false;
			}
			
			/* fetch next opcode */
			cpustate->prevpc = cpustate->pc;
			curcycles = op_call(cpustate);

			/* burn the cycles */
			cpustate->icount -= curcycles * 15;
			if (cpustate->timecount_enabled != 0)
				burn_cycles(cpustate, curcycles);
			
			total_icount += curcycles * 15;
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
			d_debugger->add_cpu_trace(cpustate->pc);
			cpustate->prevpc = cpustate->pc;
			curcycles = op_call(cpustate);

			/* burn the cycles */
			cpustate->icount -= curcycles * 15;
			if (cpustate->timecount_enabled != 0)
				burn_cycles(cpustate, curcycles);
#ifdef USE_DEBUGGER
			total_icount += curcycles * 15;
		}
#endif
	} while (cpustate->icount > 0);
	
	return base_icount - cpustate->icount;
}


#define STATE_VERSION	3

bool MCS48::process_state(FILEIO* state_fio, bool loading)
{
	mcs48_state *cpustate = (mcs48_state *)opaque;
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
#ifdef USE_DEBUGGER
	state_fio->StateValue(total_icount);
#endif
	state_fio->StateValue(cpustate->prevpc);
	state_fio->StateValue(cpustate->pc);
	state_fio->StateValue(cpustate->a);
	state_fio->StateValue(cpustate->regptr);
	state_fio->StateValue(cpustate->psw);
	state_fio->StateValue(cpustate->p1);
	state_fio->StateValue(cpustate->p2);
	state_fio->StateValue(cpustate->timer);
	state_fio->StateValue(cpustate->prescaler);
	state_fio->StateValue(cpustate->t1_history);
	state_fio->StateValue(cpustate->sts);
	state_fio->StateValue(cpustate->int_state);
	state_fio->StateValue(cpustate->irq_state);
	state_fio->StateValue(cpustate->irq_in_progress);
	state_fio->StateValue(cpustate->timer_overflow);
	state_fio->StateValue(cpustate->timer_flag);
	state_fio->StateValue(cpustate->tirq_enabled);
	state_fio->StateValue(cpustate->xirq_enabled);
	state_fio->StateValue(cpustate->t0_clk_enabled);
	state_fio->StateValue(cpustate->timecount_enabled);
	state_fio->StateValue(cpustate->a11);
	state_fio->StateValue(cpustate->icount);
//	state_fio->StateArray(cpustate->rom, sizeof(cpustate->rom), 1);

 	// post process
	if(loading) {
		mcs48_state *cpustate = (mcs48_state *)opaque;
		cpustate->mem = d_mem;
		cpustate->io = d_io;
		cpustate->intr = d_intr;
#ifdef USE_DEBUGGER
		prev_total_icount = total_icount;
#endif
	}
 	return true;
}
