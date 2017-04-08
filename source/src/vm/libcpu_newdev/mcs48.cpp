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


int MCS48::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	mcs48_state *cpustate = (mcs48_state *)opaque;
	uint32_t ptr = pc;
	
	#define upi41 false
#ifdef USE_DEBUGGER
	switch (program_r(ptr++))
	{
		case 0x00:      my_stprintf_s(buffer, buffer_len, _T("nop"));                                                                                                       break;
		case 0x02:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("out  bus,a"));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("out  dbb,a"));                                                                                                break;
		case 0x03:      my_stprintf_s(buffer, buffer_len, _T("add  a,#$%02X"), program_r(ptr++));                                                                           break;
		case 0x04:      my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x000 | program_r(ptr++)));             break;
		case 0x05:      my_stprintf_s(buffer, buffer_len, _T("en   i"));                                                                                                    break;
		case 0x07:      my_stprintf_s(buffer, buffer_len, _T("dec  a"));                                                                                                    break;
		case 0x08:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("in   a,bus"));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("illegal"));                                                                                                   break;
		case 0x09:      my_stprintf_s(buffer, buffer_len, _T("in   a,p1"));                                                                                                 break;
		case 0x0a:      my_stprintf_s(buffer, buffer_len, _T("in   a,p2"));                                                                                                 break;
		case 0x0c:      my_stprintf_s(buffer, buffer_len, _T("movd a,p4"));                                                                                                 break;
		case 0x0d:      my_stprintf_s(buffer, buffer_len, _T("movd a,p5"));                                                                                                 break;
		case 0x0e:      my_stprintf_s(buffer, buffer_len, _T("movd a,p6"));                                                                                                 break;
		case 0x0f:      my_stprintf_s(buffer, buffer_len, _T("movd a,p7"));                                                                                                 break;
		case 0x10:      my_stprintf_s(buffer, buffer_len, _T("inc  @r0"));                                                                                                  break;
		case 0x11:      my_stprintf_s(buffer, buffer_len, _T("inc  @r1"));                                                                                                  break;
		case 0x12:      my_stprintf_s(buffer, buffer_len, _T("jb0  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x13:      my_stprintf_s(buffer, buffer_len, _T("addc a,#$%02X"), program_r(ptr++));                                                                           break;
		case 0x14:      my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x000 | program_r(ptr++)));             break;
		case 0x15:      my_stprintf_s(buffer, buffer_len, _T("dis  i"));                                                                                                    break;
		case 0x16:      my_stprintf_s(buffer, buffer_len, _T("jtf  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x17:      my_stprintf_s(buffer, buffer_len, _T("inc  a"));                                                                                                    break;
		case 0x18:      my_stprintf_s(buffer, buffer_len, _T("inc  r0"));                                                                                                   break;
		case 0x19:      my_stprintf_s(buffer, buffer_len, _T("inc  r1"));                                                                                                   break;
		case 0x1a:      my_stprintf_s(buffer, buffer_len, _T("inc  r2"));                                                                                                   break;
		case 0x1b:      my_stprintf_s(buffer, buffer_len, _T("inc  r3"));                                                                                                   break;
		case 0x1c:      my_stprintf_s(buffer, buffer_len, _T("inc  r4"));                                                                                                   break;
		case 0x1d:      my_stprintf_s(buffer, buffer_len, _T("inc  r5"));                                                                                                   break;
		case 0x1e:      my_stprintf_s(buffer, buffer_len, _T("inc  r6"));                                                                                                   break;
		case 0x1f:      my_stprintf_s(buffer, buffer_len, _T("inc  r7"));                                                                                                   break;
		case 0x20:      my_stprintf_s(buffer, buffer_len, _T("xch  a,@r0"));                                                                                                break;
		case 0x21:      my_stprintf_s(buffer, buffer_len, _T("xch  a,@r1"));                                                                                                break;
		case 0x22:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("illegal"));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("in   a,dbb"));                                                                                                break;
		case 0x23:      my_stprintf_s(buffer, buffer_len, _T("mov  a,#$%02X"), program_r(ptr++));                                                                           break;
		case 0x24:      my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x100 | program_r(ptr++)));             break;
		case 0x25:      my_stprintf_s(buffer, buffer_len, _T("en   tcnti"));                                                                                                break;
		case 0x26:      my_stprintf_s(buffer, buffer_len, _T("jnt0 %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x27:      my_stprintf_s(buffer, buffer_len, _T("clr  a"));                                                                                                    break;
		case 0x28:      my_stprintf_s(buffer, buffer_len, _T("xch  a,r0"));                                                                                                 break;
		case 0x29:      my_stprintf_s(buffer, buffer_len, _T("xch  a,r1"));                                                                                                 break;
		case 0x2a:      my_stprintf_s(buffer, buffer_len, _T("xch  a,r2"));                                                                                                 break;
		case 0x2b:      my_stprintf_s(buffer, buffer_len, _T("xch  a,r3"));                                                                                                 break;
		case 0x2c:      my_stprintf_s(buffer, buffer_len, _T("xch  a,r4"));                                                                                                 break;
		case 0x2d:      my_stprintf_s(buffer, buffer_len, _T("xch  a,r5"));                                                                                                 break;
		case 0x2e:      my_stprintf_s(buffer, buffer_len, _T("xch  a,r6"));                                                                                                 break;
		case 0x2f:      my_stprintf_s(buffer, buffer_len, _T("xch  a,r7"));                                                                                                 break;
		case 0x30:      my_stprintf_s(buffer, buffer_len, _T("xchd a,@r0"));                                                                                                break;
		case 0x31:      my_stprintf_s(buffer, buffer_len, _T("xchd a,@r1"));                                                                                                break;
		case 0x32:      my_stprintf_s(buffer, buffer_len, _T("jb1  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x34:      my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x100 | program_r(ptr++)));             break;
		case 0x35:      my_stprintf_s(buffer, buffer_len, _T("dis  tcnti"));                                                                                                break;
		case 0x36:      my_stprintf_s(buffer, buffer_len, _T("jt0  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x37:      my_stprintf_s(buffer, buffer_len, _T("cpl  a"));                                                                                                    break;
		case 0x39:      my_stprintf_s(buffer, buffer_len, _T("outl p1,a"));                                                                                                 break;
		case 0x3a:      my_stprintf_s(buffer, buffer_len, _T("outl p2,a"));                                                                                                 break;
		case 0x3c:      my_stprintf_s(buffer, buffer_len, _T("movd p4,a"));                                                                                                 break;
		case 0x3d:      my_stprintf_s(buffer, buffer_len, _T("movd p5,a"));                                                                                                 break;
		case 0x3e:      my_stprintf_s(buffer, buffer_len, _T("movd p6,a"));                                                                                                 break;
		case 0x3f:      my_stprintf_s(buffer, buffer_len, _T("movd p7,a"));                                                                                                 break;
		case 0x40:      my_stprintf_s(buffer, buffer_len, _T("orl  a,@r0"));                                                                                                break;
		case 0x41:      my_stprintf_s(buffer, buffer_len, _T("orl  a,@r1"));                                                                                                break;
		case 0x42:      my_stprintf_s(buffer, buffer_len, _T("mov  a,t"));                                                                                                  break;
		case 0x43:      my_stprintf_s(buffer, buffer_len, _T("orl  a,#$%02X"), program_r(ptr++));                                                                           break;
		case 0x44:      my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x200 | program_r(ptr++)));             break;
		case 0x45:      my_stprintf_s(buffer, buffer_len, _T("strt cnt"));                                                                                                  break;
		case 0x46:      my_stprintf_s(buffer, buffer_len, _T("jnt1 %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x47:      my_stprintf_s(buffer, buffer_len, _T("swap a"));                                                                                                    break;
		case 0x48:      my_stprintf_s(buffer, buffer_len, _T("orl  a,r0"));                                                                                                 break;
		case 0x49:      my_stprintf_s(buffer, buffer_len, _T("orl  a,r1"));                                                                                                 break;
		case 0x4a:      my_stprintf_s(buffer, buffer_len, _T("orl  a,r2"));                                                                                                 break;
		case 0x4b:      my_stprintf_s(buffer, buffer_len, _T("orl  a,r3"));                                                                                                 break;
		case 0x4c:      my_stprintf_s(buffer, buffer_len, _T("orl  a,r4"));                                                                                                 break;
		case 0x4d:      my_stprintf_s(buffer, buffer_len, _T("orl  a,r5"));                                                                                                 break;
		case 0x4e:      my_stprintf_s(buffer, buffer_len, _T("orl  a,r6"));                                                                                                 break;
		case 0x4f:      my_stprintf_s(buffer, buffer_len, _T("orl  a,r7"));                                                                                                 break;
		case 0x50:      my_stprintf_s(buffer, buffer_len, _T("anl  a,@r0"));                                                                                                break;
		case 0x51:      my_stprintf_s(buffer, buffer_len, _T("anl  a,@r1"));                                                                                                break;
		case 0x52:      my_stprintf_s(buffer, buffer_len, _T("jb2  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x53:      my_stprintf_s(buffer, buffer_len, _T("anl  a,#$%02X"), program_r(ptr++));                                                                           break;
		case 0x54:      my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x200 | program_r(ptr++)));             break;
		case 0x55:      my_stprintf_s(buffer, buffer_len, _T("strt t"));                                                                                                    break;
		case 0x56:      my_stprintf_s(buffer, buffer_len, _T("jt1  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x57:      my_stprintf_s(buffer, buffer_len, _T("da   a"));                                                                                                    break;
		case 0x58:      my_stprintf_s(buffer, buffer_len, _T("anl  a,r0"));                                                                                                 break;
		case 0x59:      my_stprintf_s(buffer, buffer_len, _T("anl  a,r1"));                                                                                                 break;
		case 0x5a:      my_stprintf_s(buffer, buffer_len, _T("anl  a,r2"));                                                                                                 break;
		case 0x5b:      my_stprintf_s(buffer, buffer_len, _T("anl  a,r3"));                                                                                                 break;
		case 0x5c:      my_stprintf_s(buffer, buffer_len, _T("anl  a,r4"));                                                                                                 break;
		case 0x5d:      my_stprintf_s(buffer, buffer_len, _T("anl  a,r5"));                                                                                                 break;
		case 0x5e:      my_stprintf_s(buffer, buffer_len, _T("anl  a,r6"));                                                                                                 break;
		case 0x5f:      my_stprintf_s(buffer, buffer_len, _T("anl  a,r7"));                                                                                                 break;
		case 0x60:      my_stprintf_s(buffer, buffer_len, _T("add  a,@r0"));                                                                                                break;
		case 0x61:      my_stprintf_s(buffer, buffer_len, _T("add  a,@r1"));                                                                                                break;
		case 0x62:      my_stprintf_s(buffer, buffer_len, _T("mov  t,a"));                                                                                                  break;
		case 0x64:      my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x300 | program_r(ptr++)));             break;
		case 0x65:      my_stprintf_s(buffer, buffer_len, _T("stop tcnt"));                                                                                                 break;
		case 0x67:      my_stprintf_s(buffer, buffer_len, _T("rrc  a"));                                                                                                    break;
		case 0x68:      my_stprintf_s(buffer, buffer_len, _T("add  a,r0"));                                                                                                 break;
		case 0x69:      my_stprintf_s(buffer, buffer_len, _T("add  a,r1"));                                                                                                 break;
		case 0x6a:      my_stprintf_s(buffer, buffer_len, _T("add  a,r2"));                                                                                                 break;
		case 0x6b:      my_stprintf_s(buffer, buffer_len, _T("add  a,r3"));                                                                                                 break;
		case 0x6c:      my_stprintf_s(buffer, buffer_len, _T("add  a,r4"));                                                                                                 break;
		case 0x6d:      my_stprintf_s(buffer, buffer_len, _T("add  a,r5"));                                                                                                 break;
		case 0x6e:      my_stprintf_s(buffer, buffer_len, _T("add  a,r6"));                                                                                                 break;
		case 0x6f:      my_stprintf_s(buffer, buffer_len, _T("add  a,r7"));                                                                                                 break;
		case 0x70:      my_stprintf_s(buffer, buffer_len, _T("addc a,@r0"));                                                                                                break;
		case 0x71:      my_stprintf_s(buffer, buffer_len, _T("addc a,@r1"));                                                                                                break;
		case 0x72:      my_stprintf_s(buffer, buffer_len, _T("jb3  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x74:      my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x300 | program_r(ptr++)));             break;
		case 0x75:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("ent0 clk"));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("illegal"));                                                                                                   break;
		case 0x76:      my_stprintf_s(buffer, buffer_len, _T("jf1  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x77:      my_stprintf_s(buffer, buffer_len, _T("rr   a"));                                                                                                    break;
		case 0x78:      my_stprintf_s(buffer, buffer_len, _T("addc a,r0"));                                                                                                 break;
		case 0x79:      my_stprintf_s(buffer, buffer_len, _T("addc a,r1"));                                                                                                 break;
		case 0x7a:      my_stprintf_s(buffer, buffer_len, _T("addc a,r2"));                                                                                                 break;
		case 0x7b:      my_stprintf_s(buffer, buffer_len, _T("addc a,r3"));                                                                                                 break;
		case 0x7c:      my_stprintf_s(buffer, buffer_len, _T("addc a,r4"));                                                                                                 break;
		case 0x7d:      my_stprintf_s(buffer, buffer_len, _T("addc a,r5"));                                                                                                 break;
		case 0x7e:      my_stprintf_s(buffer, buffer_len, _T("addc a,r6"));                                                                                                 break;
		case 0x7f:      my_stprintf_s(buffer, buffer_len, _T("addc a,r7"));                                                                                                 break;
		case 0x80:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("movx a,@r0"));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("illegal"));                                                                                                   break;
		case 0x81:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("movx a,@r1"));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("illegal"));                                                                                                   break;
		case 0x83:      my_stprintf_s(buffer, buffer_len, _T("ret"));                                                                                                       break;
		case 0x84:      my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x400 | program_r(ptr++)));             break;
		case 0x85:      my_stprintf_s(buffer, buffer_len, _T("clr  f0"));                                                                                                   break;
		case 0x86:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("jni  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("jobf %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x88:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("orl  bus,#$%02X"), program_r(ptr++));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("illegal"));                                                                                                   break;
		case 0x89:      my_stprintf_s(buffer, buffer_len, _T("orl  p1,#$%02X"), program_r(ptr++));                                                                          break;
		case 0x8a:      my_stprintf_s(buffer, buffer_len, _T("orl  p2,#$%02X"), program_r(ptr++));                                                                          break;
		case 0x8c:      my_stprintf_s(buffer, buffer_len, _T("orld p4,a"));                                                                                                 break;
		case 0x8d:      my_stprintf_s(buffer, buffer_len, _T("orld p5,a"));                                                                                                 break;
		case 0x8e:      my_stprintf_s(buffer, buffer_len, _T("orld p6,a"));                                                                                                 break;
		case 0x8f:      my_stprintf_s(buffer, buffer_len, _T("orld p7,a"));                                                                                                 break;
		case 0x90:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("movx @r0,a"));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("mov  sts,a"));                                                                                                break;
		case 0x91:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("movx @r1,a"));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("illegal"));                                                                                                   break;
		case 0x92:      my_stprintf_s(buffer, buffer_len, _T("jb4  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x93:      my_stprintf_s(buffer, buffer_len, _T("retr"));                                                                                                      break;
		case 0x94:      my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x400 | program_r(ptr++)));             break;
		case 0x95:      my_stprintf_s(buffer, buffer_len, _T("cpl  f0"));                                                                                                   break;
		case 0x96:      my_stprintf_s(buffer, buffer_len, _T("jnz  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0x97:      my_stprintf_s(buffer, buffer_len, _T("clr  c"));                                                                                                    break;
		case 0x98:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("anl  bus,#$%02X"), program_r(ptr++));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("illegal"));                                                                                                   break;
		case 0x99:      my_stprintf_s(buffer, buffer_len, _T("anl  p1,#$%02X"), program_r(ptr++));                                                                          break;
		case 0x9a:      my_stprintf_s(buffer, buffer_len, _T("anl  p2,#$%02X"), program_r(ptr++));                                                                          break;
		case 0x9c:      my_stprintf_s(buffer, buffer_len, _T("anld p4,a"));                                                                                                 break;
		case 0x9d:      my_stprintf_s(buffer, buffer_len, _T("anld p5,a"));                                                                                                 break;
		case 0x9e:      my_stprintf_s(buffer, buffer_len, _T("anld p6,a"));                                                                                                 break;
		case 0x9f:      my_stprintf_s(buffer, buffer_len, _T("anld p7,a"));                                                                                                 break;
		case 0xa0:      my_stprintf_s(buffer, buffer_len, _T("mov  @r0,a"));                                                                                                break;
		case 0xa1:      my_stprintf_s(buffer, buffer_len, _T("mov  @r1,a"));                                                                                                break;
		case 0xa3:      my_stprintf_s(buffer, buffer_len, _T("movp a,@a"));                                                                                                 break;
		case 0xa4:      my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x500 | program_r(ptr++)));             break;
		case 0xa5:      my_stprintf_s(buffer, buffer_len, _T("clr  f1"));                                                                                                   break;
		case 0xa7:      my_stprintf_s(buffer, buffer_len, _T("cpl  c"));                                                                                                    break;
		case 0xa8:      my_stprintf_s(buffer, buffer_len, _T("mov  r0,a"));                                                                                                 break;
		case 0xa9:      my_stprintf_s(buffer, buffer_len, _T("mov  r1,a"));                                                                                                 break;
		case 0xaa:      my_stprintf_s(buffer, buffer_len, _T("mov  r2,a"));                                                                                                 break;
		case 0xab:      my_stprintf_s(buffer, buffer_len, _T("mov  r3,a"));                                                                                                 break;
		case 0xac:      my_stprintf_s(buffer, buffer_len, _T("mov  r4,a"));                                                                                                 break;
		case 0xad:      my_stprintf_s(buffer, buffer_len, _T("mov  r5,a"));                                                                                                 break;
		case 0xae:      my_stprintf_s(buffer, buffer_len, _T("mov  r6,a"));                                                                                                 break;
		case 0xaf:      my_stprintf_s(buffer, buffer_len, _T("mov  r7,a"));                                                                                                 break;
		case 0xb0:      my_stprintf_s(buffer, buffer_len, _T("mov  @r0,#$%02X"), program_r(ptr++));                                                                         break;
		case 0xb1:      my_stprintf_s(buffer, buffer_len, _T("mov  @r1,#$%02X"), program_r(ptr++));                                                                         break;
		case 0xb2:      my_stprintf_s(buffer, buffer_len, _T("jb5  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0xb3:      my_stprintf_s(buffer, buffer_len, _T("jmpp @a"));                                                                                                   break;
		case 0xb4:      my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x500 | program_r(ptr++)));             break;
		case 0xb5:      my_stprintf_s(buffer, buffer_len, _T("cpl  f1"));                                                                                                   break;
		case 0xb6:      my_stprintf_s(buffer, buffer_len, _T("jf0  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0xb8:      my_stprintf_s(buffer, buffer_len, _T("mov  r0,#$%02X"), program_r(ptr++));                                                                          break;
		case 0xb9:      my_stprintf_s(buffer, buffer_len, _T("mov  r1,#$%02X"), program_r(ptr++));                                                                          break;
		case 0xba:      my_stprintf_s(buffer, buffer_len, _T("mov  r2,#$%02X"), program_r(ptr++));                                                                          break;
		case 0xbb:      my_stprintf_s(buffer, buffer_len, _T("mov  r3,#$%02X"), program_r(ptr++));                                                                          break;
		case 0xbc:      my_stprintf_s(buffer, buffer_len, _T("mov  r4,#$%02X"), program_r(ptr++));                                                                          break;
		case 0xbd:      my_stprintf_s(buffer, buffer_len, _T("mov  r5,#$%02X"), program_r(ptr++));                                                                          break;
		case 0xbe:      my_stprintf_s(buffer, buffer_len, _T("mov  r6,#$%02X"), program_r(ptr++));                                                                          break;
		case 0xbf:      my_stprintf_s(buffer, buffer_len, _T("mov  r7,#$%02X"), program_r(ptr++));                                                                          break;
		case 0xc4:      my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x600 | program_r(ptr++)));             break;
		case 0xc5:      my_stprintf_s(buffer, buffer_len, _T("sel  rb0"));                                                                                                  break;
		case 0xc6:      my_stprintf_s(buffer, buffer_len, _T("jz   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0xc7:      my_stprintf_s(buffer, buffer_len, _T("mov  a,psw"));                                                                                                break;
		case 0xc8:      my_stprintf_s(buffer, buffer_len, _T("dec  r0"));                                                                                                   break;
		case 0xc9:      my_stprintf_s(buffer, buffer_len, _T("dec  r1"));                                                                                                   break;
		case 0xca:      my_stprintf_s(buffer, buffer_len, _T("dec  r2"));                                                                                                   break;
		case 0xcb:      my_stprintf_s(buffer, buffer_len, _T("dec  r3"));                                                                                                   break;
		case 0xcc:      my_stprintf_s(buffer, buffer_len, _T("dec  r4"));                                                                                                   break;
		case 0xcd:      my_stprintf_s(buffer, buffer_len, _T("dec  r5"));                                                                                                   break;
		case 0xce:      my_stprintf_s(buffer, buffer_len, _T("dec  r6"));                                                                                                   break;
		case 0xcf:      my_stprintf_s(buffer, buffer_len, _T("dec  r7"));                                                                                                   break;
		case 0xd0:      my_stprintf_s(buffer, buffer_len, _T("xrl  a,@r0"));                                                                                                break;
		case 0xd1:      my_stprintf_s(buffer, buffer_len, _T("xrl  a,@r1"));                                                                                                break;
		case 0xd2:      my_stprintf_s(buffer, buffer_len, _T("jb6  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0xd3:      my_stprintf_s(buffer, buffer_len, _T("xrl  a,#$%02X"), program_r(ptr++));                                                                           break;
		case 0xd4:      my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x600 | program_r(ptr++)));             break;
		case 0xd5:      my_stprintf_s(buffer, buffer_len, _T("sel  rb1"));                                                                                                  break;
		case 0xd6:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("illegal"));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("jnibf %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));     break;
		case 0xd7:      my_stprintf_s(buffer, buffer_len, _T("mov  psw,a"));                                                                                                break;
		case 0xd8:      my_stprintf_s(buffer, buffer_len, _T("xrl  a,r0"));                                                                                                 break;
		case 0xd9:      my_stprintf_s(buffer, buffer_len, _T("xrl  a,r1"));                                                                                                 break;
		case 0xda:      my_stprintf_s(buffer, buffer_len, _T("xrl  a,r2"));                                                                                                 break;
		case 0xdb:      my_stprintf_s(buffer, buffer_len, _T("xrl  a,r3"));                                                                                                 break;
		case 0xdc:      my_stprintf_s(buffer, buffer_len, _T("xrl  a,r4"));                                                                                                 break;
		case 0xdd:      my_stprintf_s(buffer, buffer_len, _T("xrl  a,r5"));                                                                                                 break;
		case 0xde:      my_stprintf_s(buffer, buffer_len, _T("xrl  a,r6"));                                                                                                 break;
		case 0xdf:      my_stprintf_s(buffer, buffer_len, _T("xrl  a,r7"));                                                                                                 break;
		case 0xe3:      my_stprintf_s(buffer, buffer_len, _T("movp3 a,@a"));                                                                                                break;
		case 0xe4:      my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x700 | program_r(ptr++)));             break;
		case 0xe5:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("sel  mb0"));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("en   dma"));                                                                                                  break;
		case 0xe6:      my_stprintf_s(buffer, buffer_len, _T("jnc  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0xe7:      my_stprintf_s(buffer, buffer_len, _T("rl   a"));                                                                                                    break;
		case 0xe8:      my_stprintf_s(buffer, buffer_len, _T("djnz r0,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));   break;
		case 0xe9:      my_stprintf_s(buffer, buffer_len, _T("djnz r1,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));   break;
		case 0xea:      my_stprintf_s(buffer, buffer_len, _T("djnz r2,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));   break;
		case 0xeb:      my_stprintf_s(buffer, buffer_len, _T("djnz r3,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));   break;
		case 0xec:      my_stprintf_s(buffer, buffer_len, _T("djnz r4,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));   break;
		case 0xed:      my_stprintf_s(buffer, buffer_len, _T("djnz r5,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));   break;
		case 0xee:      my_stprintf_s(buffer, buffer_len, _T("djnz r6,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));   break;
		case 0xef:      my_stprintf_s(buffer, buffer_len, _T("djnz r7,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));   break;
		case 0xf0:      my_stprintf_s(buffer, buffer_len, _T("mov  a,@r0"));                                                                                                break;
		case 0xf1:      my_stprintf_s(buffer, buffer_len, _T("mov  a,@r1"));                                                                                                break;
		case 0xf2:      my_stprintf_s(buffer, buffer_len, _T("jb7  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0xf4:      my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), 0x700 | program_r(ptr++)));             break;
		case 0xf5:  if (!upi41)
		                my_stprintf_s(buffer, buffer_len, _T("sel  mb1"));
		            else
		                my_stprintf_s(buffer, buffer_len, _T("en   flags"));                                                                                                break;
		case 0xf6:      my_stprintf_s(buffer, buffer_len, _T("jc   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%03X"), (pc & 0xf00) | program_r(ptr++)));      break;
		case 0xf7:      my_stprintf_s(buffer, buffer_len, _T("rlc  a"));                                                                                                    break;
		case 0xf8:      my_stprintf_s(buffer, buffer_len, _T("mov  a,r0"));                                                                                                 break;
		case 0xf9:      my_stprintf_s(buffer, buffer_len, _T("mov  a,r1"));                                                                                                 break;
		case 0xfa:      my_stprintf_s(buffer, buffer_len, _T("mov  a,r2"));                                                                                                 break;
		case 0xfb:      my_stprintf_s(buffer, buffer_len, _T("mov  a,r3"));                                                                                                 break;
		case 0xfc:      my_stprintf_s(buffer, buffer_len, _T("mov  a,r4"));                                                                                                 break;
		case 0xfd:      my_stprintf_s(buffer, buffer_len, _T("mov  a,r5"));                                                                                                 break;
		case 0xfe:      my_stprintf_s(buffer, buffer_len, _T("mov  a,r6"));                                                                                                 break;
		case 0xff:      my_stprintf_s(buffer, buffer_len, _T("mov  a,r7"));                                                                                                 break;
		default:        my_stprintf_s(buffer, buffer_len, _T("illegal"));                                                                                                   break;

	}
#else
	my_stprintf_s(buffer, buffer_len, _T("**OP $%02x"), program_r(ptr++));
#endif	
	return ptr - pc;
}
//#endif

