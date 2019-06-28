/*
 * Note: This was splitted from uPD7810 for uPD7907.
 * Author: Kyuma.Ohta <whaisthis.sowhat _at_ gmail.com>
 * Date:   2019-06-28 -
 */
/*****************************************************************************
 *
 *   upd7810.h
 *   Portable uPD7810/11, 7810H/11H, 78C10/C11/C14 emulator V0.3
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *   You can contact me at juergen@mame.net or pullmoll@stop1984.com
 *
 *   - This source code is released as freeware for non-commercial purposes
 *     as part of the M.A.M.E. (Multiple Arcade Machine Emulator) project.
 *     The licensing terms of MAME apply to this piece of code for the MAME
 *     project and derviative works, as defined by the MAME license. You
 *     may opt to make modifications, improvements or derivative works under
 *     that same conditions, and the MAME project may opt to keep
 *     modifications, improvements or derivatives under their terms exclusively.
 *
 *   - Alternatively you can choose to apply the terms of the "GPL" (see
 *     below) to this - and only this - piece of code or your derivative works.
 *     Note that in no case your choice can have any impact on any other
 *     source code of the MAME project, or binary, or executable, be it closely
 *     or losely related to this piece of code.
 *
 *  -  At your choice you are also free to remove either licensing terms from
 *     this file and continue to use it under only one of the two licenses. Do this
 *     if you think that licenses are not compatible (enough) for you, or if you
 *     consider either license 'too restrictive' or 'too free'.
 *
 *  -  GPL (GNU General Public License)
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License
 *     as published by the Free Software Foundation; either version 2
 *     of the License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 *  This work is based on the
 *  "NEC Electronics User's Manual, April 1987"
 *
 * NS20030115:
 * - fixed INRW_wa(cpustate)
 * - TODO: add 7807, differences are listed below.
 *       I only added support for these opcodes needed by homedata.c (yes, I am
 *       lazy):
 *       4C CE (MOV A,PT)
 *       48 AC (EXA)
 *       48 AD (EXR)
 *       48 AE (EXH)
 *       48 AF (EXX)
 *       50 xx (SKN bit)
 *       58 xx (SETB)
 *       5B xx (CLR)
 *       5D xx (SK bit)
 *
 * 2008-02-24 (Wilbert Pol):
 * - Added preliminary support for uPD7801
 *   For the uPD7801 only the basic instruction set was added. The current timer
 *   and serial i/o implementations are most likely incorrect.
 * - Added basic support for uPD78C05 and uPD78C06
 *   Documentation of the actual instruction set layout is missing, so we took
 *   the uPD7801 instruction set and only kept the instructions mentioned in
 *   the little documentation available on the uPD78c05A/06A. The serial i/o
 *   implementation has not been tested and is probably incorrect.
 *
 *****************************************************************************/
/* Hau around 23 May 2004
  gta, gti, dgt fixed
  working reg opcodes fixed
  sio input fixed
--
  PeT around 19 February 2002
  type selection/gamemaster support added
  gamemaster init hack? added
  ORAX added
  jre negativ fixed
  prefixed opcodes skipping fixed
  interrupts fixed and improved
  sub(and related)/add/daa flags fixed
  mvi ports,... fixed
  rll, rlr, drll, drlr fixed
  rets fixed
  l0, l1 skipping fixed
  calt fixed
*/

/*

7807 DESCRIPTION



   PA0  1     64 Vcc
   PA1  2     63 Vdd
   PA2  3     62 PD7/AD7
   PA3  4     61 PD6/AD6
   PA4  5     60 PD5/AD5
   PA5  6     59 PD4/AD4
   PA6  7     58 PD3/AD3
   PA7  8     57 PD2/AD2
   PB0  9     56 PD1/AD1
   PB1 10     55 PD0/AD0
   PB2 11     54 PF7/AB15
   PB3 12     53 PF6/AB14
   PB4 13     52 PF5/AB13
   PB5 14     51 PF4/AB12
   PB6 15     50 PF3/AB11
   PB7 16     49 PF2/AB10
   PC0 17     48 PF1/AB9
   PC1 18     47 PF0/AB8
   PC2 19     46 ALE
   PC3 20     45 WR*
   PC4 21     44 RD*
   PC5 22     43 HLDA
   PC6 23     42 HOLD
   PC7 24     41 PT7
  NMI* 25     40 PT6
  INT1 26     39 PT5
 MODE1 27     38 PT4
RESET* 28     37 PT3
 MODE0 29     36 PT2
    X2 30     35 PT1
    X1 31     34 PT0
   Vss 32     33 Vth

PA, PB, PC, PD, and PF is bidirectional I/O port
and PT is comparator input port in uPD7808.
uPD7807 uses PD port as data I/O and bottom address output,
and uses PF port as top address output.

NMI* is non maskable interrupt input signal (negative edge trigger).

INT1 is interrupt input (positive edge trigger). It can be used as
AC zero-cross input or trigger source of 16bit timer counter.

MODE0 and MODE1 is input terminal which decides total amount of
external memory of uPD7807 (4KByte, 16KBYte, and 64KByte).
It also decides number of PF ports used as top address output.
 4KByte mode: PF0~PF3=address output, PF4~PF7=data I/O port
16KByte mode: PF0~PF5=address output, PF6~PF7=data I/O port
64KByte mode: PF0~PF7=address output

RESET* is system rest terminal.

X1 and X2 does clock signal generation (connect OSC and condenser).

Vth is used to determine threshold voltage for PT port.
PT0~PT7 is connected to + input of each comparator,
and Vth deterimnes voltage connected to - input of PT0~PT7.
But the voltage of Vth is not directly connected to comapators.
It is connected via 16-level programmable voltage separate circuit.

HOLD and HLDA is terminal for DMA. RD*, WR*, and ALE is bus
interface signal (they are same type of Intel 8085).
Unlike 8085, I/O address space is not available, so IO /M* signal
does not exist. Read/write of external memory can be done
by RD*, WR*, and ALE only.

Vcc and Vss is main power source. Vdd is backup power source
for internal RWM (32 Byte).


PA and PB is I/O port. They have control register MA and MB.
If control register is set to 1, the port is input.
If control register is set to 0, the port is output.
They are set to 1 by reset.

PT is input-only port. It is consisted of input terminal PT0~PT7
and Vth (set threshold voltage). Each PT input has analog comparator
and latch, and + input of analog comparator is connected to
PT terminal. Every - input of analog comparator is connected
to devided voltage of Vth. Voltage dividing level can be set by
bottom 4bits of MT (mode T) register. The range is 1/16~16/16 of Vth.

Other internal I/Os are
8bit timer (x2): Upcounter. If the counter matches to specified value,
the timer is reset and counts again from 0.
You can also set it to generate interrupt, or invert output flip-flop
when the counter matches to specified value.
Furthermore, you can output that flip-flop output to PC4/TO output,
connect it to clock input of timer/event counter or watchdog timer.
Or you can use it as bitrate clock of serial interface.
Note: There is only 1 output flip-flop for 2 timers.
If you use it for timer output of 1 timer, another timer cannot be used
for other than interrupt generator.
Clock input for timer can be switched between internal clock (2 type)
or PC3/TI input. You can set 1 timer's match-output as another timer's
clock input, so that you can use them as 1 16bit timer.

16bit timer/event counter (x1): It can be used as
- Interval timer
- External event counter
- Frequency measurement
- Pulse width measurement
- Programmable rectangle wave output
- One pulse output
Related terminals are PC5/CI input, PC6/CO0 output, and PC7/CO1.
You can measure CI input's H duration, or you can output timing signal
(with phase difference) to CO0 and CO1.

serial I/F (x1): has 3 modes.
- Asynchronous mode
- Synchronous mode
- I/O interface mode
In all 3 modes, bitrate can be internal fixed clock, or timer output,
or external clock.
In asynchronous mode, you can
- switch 7bit/8bit data
- set parity ON/OFF and EVEN/ODD
- set 1/2 stop bit




DIFFERENCES BETWEEN 7810 and 7807

--------------------------
8bit transfer instructions
--------------------------

7810
inst.     1st byte 2nd byte state   action
EXX       00001001            4     Swap BC DE HL
EXA       00001000            4     Swap VA EA
EXH       01010000            4     Swap HL
BLOCK     00110001          13(C+1)  (DE)+ <- (HL)+, C <- C - 1, until CY

7807
inst.     1st byte  2nd byte state   action
EXR       01001000  10101101   8     Swap VA BC DE HL EA
EXX       01001000  10101111   8     Swap BC DE HL
EXA       01001000  10101100   8     Swap VA EA
EXH       01001000  10101110   8     Swap HL
BLOCK  D+ 00010000           13(C+1) (DE)+ <- (HL)+, C <- C - 1, until CY
BLOCK  D- 00010001           13(C+1) (DE)- <- (HL)-, C <- C - 1, until CY


---------------------------
16bit transfer instructions
---------------------------
All instructions are same except operand sr4 of DMOV instruction.
7810
V0-sr4 -function
 0-ECNT-timer/event counter upcounter
 1-ECPT-timer/event counter capture

7807
V1-V0- sr4 -function
 0- 0-ECNT -timer/event counter upcounter
 0- 1-ECPT0-timer/event counter capture 0
 1- 0-ECPT1-timer/event counter capture 1


-----------------------------------------
8bit operation instructions for registers
-----------------------------------------
All instructions are same.


--------------------------------------
8bit operation instructions for memory
--------------------------------------
All instructions are same.


-----------------------------------------
Operation instructions for immediate data
-----------------------------------------
uPD7807 has read-only PT port and special register group sr5 for it.
ins.               1st byte  2nd byte 3rd 4th state func
GTI    sr5, byte   01100100  s0101sss  dd      14   !CY  sr5 - byte - 1
LTI    sr5, byte   01100100  s0111sss  dd      14    CY  sr5 - byte
NEI    sr5, byte   01100100  s1101sss  dd      14   !Z   sr5 - byte
EQI    sr5, byte   01100100  s1111sss  dd      14    Z   sr5 - byte
ONI    sr5, byte   01100100  s1001sss  dd      14   !Z   sr5 & byte
OFFI   sr5, byte   01100100  s1011sss  dd      14    Z   sr5 & byte

S5-S4-S3-S2-S1-S0-sr -sr1-sr2-sr5-register function
 0  0  1  1  1  0 --- PT  --- PT  comparator input port T data
 1  0  0  1  0  0 WDM WDM --- --- watchdog timer mode register
 1  0  0  1  0  1 MT  --- --- --- port T mode

7807 doesn't have registers below
 0  0  1  0  0  0 ANM ANM ANM     A/D channel mode
 1  0  0  0  0  0 --- CR0 ---     A/D conversion result 0
 1  0  0  0  0  1 --- CR1 ---     A/D conversion result 1
 1  0  0  0  1  0 --- CR2 ---     A/D conversion result 2
 1  0  0  0  1  1 --- CR3 ---     A/D conversion result 3
 1  0  1  0  0  0 ZCM --- ---     zero cross mode

Special register operand (includes registers for I/O ports) has
6 groups - sr, sr1, sr2, sr3, sr4, and sr5. Among these groups,
sr, sr1, sr2, and sr5 includes registers described in the table
below, and expressed as bit pattern S5-S0.

S5S4S3S2S1S0 sr  sr1 sr2 sr5 register function
0 0 0 0 0 0  PA  PA  PA  PA  port A
0 0 0 0 0 1  PB  PB  PB  PB  port B
0 0 0 0 1 0  PC  PC  PC  PC  port C
0 0 0 0 1 1  PD  PD  PD  PD  port D
0 0 0 1 0 1  PF  PF  PF  PF  port F
0 0 0 1 1 0  MKH MKH MKH MKH mask high
0 0 0 1 1 1  MKL MKL MKL MKL mask low
0 0 1 0 0 1  SMH SMH SMH SMH serial mode high
0 0 1 0 1 0  SML --- --- --- serial mode low
0 0 1 0 1 1  EOM EOM EOM EOM timer/event counter output mode
0 0 1 1 0 0 ETMM --- --- --- timer/event counter mode
0 0 1 1 0 1  TMM TMM TMM TMM timer mode
0 0 1 1 1 0  --- PT  --- PT  port T
0 1 0 0 0 0  MM  --- --- --- memory mapping
0 1 0 0 0 1  MCC --- --- --- mode control C
0 1 0 0 1 0  MA  --- --- --- mode A
0 1 0 0 1 1  MB  --- --- --- mode B
0 1 0 1 0 0  MC  --- --- --- mode C
0 1 0 1 1 1  MF  --- --- --- mode F
0 1 1 0 0 0  TXB --- --- --- Tx buffer
0 1 1 0 0 1  --- RXB --- --- Rx buffer
0 1 1 0 1 0  TM0 --- --- --- timer register 0
0 1 1 0 1 1  TM1 --- --- --- timer register 1
1 0 0 1 0 0  WDM WDM --- --- watchdog timer mode
1 0 0 1 0 1  MT  --- --- --- mode T

For sr and sr1, all 6bits (S5, S4, S3, S2, S1, and S0) are used.
For sr2 and sr5, only 4bits (S3, S2, S1, AND S0) are used.
They are expressed as 'ssssss' and 's sss' in operation code.
Note that 's sss' (of sr2 and sr5) is located separately.
S0 is rightmost bit (LSB).


--------------------------------------------
Operation instructions for working registers
--------------------------------------------
All instructions are same.


--------------------------------------------------------------------------
16bit operation instructions and divider/multiplier operation instructions
--------------------------------------------------------------------------
All instructions are same.


------------------------------------------
Increment/decrement operation instructions
------------------------------------------
All instructions are same.


----------------------------
Other operation instructions
----------------------------
7807 has CMC instruction (inverts CY flag).
ins. 1st byte 2nd byte 3rd 4th state func
CMC  01001000 10101010           8   CY <- !CY


---------------------------
Rotation/shift instructions
---------------------------
All instructions are same.


-----------------------------
Jump/call/return instructions
-----------------------------
All instructions are same.


-----------------
Skip instructions
-----------------
7807 doesn't have this
ins.            1st byte 2nd byte 3rd 4th state func
BIT bit, wa     01011bbb  wwwwwwww          10*  bit skip if (V.wa).bit = 1

Instead, 7807 has these bit manipulation instructions.
ins.            1st byte 2nd byte 3rd 4th state func
MOV    CY, bit  01011111  bbbbbbbb          10* CY <- (bit)
MOV    bit, CY  01011010  bbbbbbbb          13* (bit) <- CY
AND    CY, bit  00110001  bbbbbbbb          10* CY <- CY & (bit)
OR     CY, bit  01011100  bbbbbbbb          10* CY <- CY | (bit)
XOR    CY, bit  01011110  bbbbbbbb          10* CY <- CY ^ (bit)
SETB   bit      01011000  bbbbbbbb          13* (bit) <- 1
CLR    bit      01011011  bbbbbbbb          13* (bit) <- 0
NOT    bit      01011001  bbbbbbbb          13* (bit) <- !(bit)
SK     bit      01011101  bbbbbbbb          10*  (b) skip if (bit) = 1
SKN    bit      01010000  bbbbbbbb          10* !(b) skip if (bit) = 0


------------------------
CPU control instructions
------------------------
ins.            1st byte 2nd byte 3rd 4th state func
HLT             01001000  00111011        11/12 halt
11 state in uPD7807 and uPD7810, 12 state in uPD78C10.

STOP            01001000  10111011          12  stop
7807 doesn't have STOP instruction.

*/

//#include "emu.h"
//#include "debugger.h"

#define HAS_UPD7907
#include "./upd7810_common.c"
#undef HAS_UPD7907

static int __FASTCALL run_one_opecode_upd7907(upd7810_state *cpustate)
{
	cpustate->icount = 0;

//	do
//	{
		int cc = 0;

//		debugger_instruction_hook(device, PC);

		PPC = PC;
		RDOP(OP);

		/*
		 * clear L0 and/or L1 flags for all opcodes except
		 * L0   for "MVI L,xx" or "LXI H,xxxx"
		 * L1   for "MVI A,xx"
		 */
		PSW &= ~cpustate->opXX[OP].mask_l0_l1;

		/* skip flag set and not SOFTI opcode? */
		if ((PSW & SK) && (OP != 0x72))
		{
			if (cpustate->opXX[OP].cycles)
			{
				cc = cpustate->opXX[OP].cycles_skip;
				PC += cpustate->opXX[OP].oplen - 1;
			}
			else
			{
				RDOP(OP2);
				switch (OP)
				{
				case 0x48:
					cc = cpustate->op48[OP2].cycles_skip;
					PC += cpustate->op48[OP2].oplen - 2;
					break;
				case 0x4c:
					cc = cpustate->op4C[OP2].cycles_skip;
					PC += cpustate->op4C[OP2].oplen - 2;
					break;
				case 0x4d:
					cc = cpustate->op4D[OP2].cycles_skip;
					PC += cpustate->op4D[OP2].oplen - 2;
					break;
				case 0x60:
					cc = cpustate->op60[OP2].cycles_skip;
					PC += cpustate->op60[OP2].oplen - 2;
					break;
				case 0x64:
					cc = cpustate->op64[OP2].cycles_skip;
					PC += cpustate->op64[OP2].oplen - 2;
					break;
				case 0x70:
					cc = cpustate->op70[OP2].cycles_skip;
					PC += cpustate->op70[OP2].oplen - 2;
					break;
				case 0x74:
					cc = cpustate->op74[OP2].cycles_skip;
					PC += cpustate->op74[OP2].oplen - 2;
					break;
				default:
//					fatalerror("uPD7810 internal error: check cycle counts for main\n");
					break;
				}
			}
			PSW &= ~SK;
			cpustate->handle_timers( cpustate, cc );
		}
		else
		{
			cc = cpustate->opXX[OP].cycles;
			cpustate->handle_timers( cpustate, cc );
			(*cpustate->opXX[OP].opfunc)(cpustate);
			// The vector register is always pointing to FF00
			V = 0xFF;
		}
		cpustate->icount -= cc;
		upd7810_take_irq(cpustate);

//	} while (cpustate->icount > 0);

	return -cpustate->icount;
}


static CPU_RESET( upd7907 )
{
	CPU_RESET_CALL(upd7810);

	cpustate->opXX = opXX_7907;
	cpustate->op48 = op48_7907;
	cpustate->op4C = op4C_7907;
	cpustate->op4D = op4D_7907;
	cpustate->op60 = op60_7907;
	cpustate->op64 = op64_7907;
	cpustate->op70 = op70_7907;
	cpustate->op74 = op74_7907;
	cpustate->config.type = TYPE_78C06;

	EOM = 0xff;
	MA = 0;		/* All outputs */
	MB = 0;
	V = 0xFF;	/* The vector register is always pointing to FF00 */
	cpustate->handle_timers = upd78c05_timers;
	TM0 = 0xFF;	/* Timer seems to be running from boot */
	cpustate->ovc0 = ( ( TMM & 0x04 ) ? 16 * 8 : 8 ) * TM0;
	cpustate->softi = 0;
}


static CPU_EXECUTE( upd7907 )
{
	int icount;
	

	bool now_debugging = false;
	if(cpustate->debugger != NULL) {
		now_debugging = cpustate->debugger->now_debugging;
	}
	if(now_debugging) {
		cpustate->debugger->check_break_points(cpustate->pc.w.l);
		if(cpustate->debugger->now_suspended) {
			cpustate->debugger->now_waiting = true;
			cpustate->emu->start_waiting_in_debugger();
			while(cpustate->debugger->now_debugging && cpustate->debugger->now_suspended) {
				cpustate->emu->process_waiting_in_debugger();
			}
			cpustate->emu->finish_waiting_in_debugger();
			cpustate->debugger->now_waiting = false;
		}
		if(cpustate->debugger->now_debugging) {
			cpustate->program = cpustate->io = cpustate->debugger;
		} else {
			now_debugging = false;
		}
		
		icount = run_one_opecode_upd7907(cpustate);
		
		if(now_debugging) {
			if(!cpustate->debugger->now_going) {
				cpustate->debugger->now_suspended = true;
			}
			cpustate->program = cpustate->program_stored;
			cpustate->io = cpustate->io_stored;
		}
	} else {
		icount = run_one_opecode_upd7907(cpustate);
	}
	return icount;
}

static void __FASTCALL upd7810_take_irq(upd7810_state *cpustate)
{
	UINT16 vector = 0;
	int irqline = 0;

	/* global interrupt disable? */
	if (0 == IFF)
		return;

	/* check the interrupts in priority sequence */
	if ((IRR & INTFT0)  && 0 == (MKL & 0x02))
	{
		vector = 0x0008;
		if (!((IRR & INTFT1)    && 0 == (MKL & 0x04)))
			IRR&=~INTFT0;
	}
	else
	if ((IRR & INTFT1)  && 0 == (MKL & 0x04))
	{
		vector = 0x0008;
		IRR&=~INTFT1;
	}
	else
	if ((IRR & INTF1)   && 0 == (MKL & 0x08))
	{
		irqline = UPD7907_INTF1;
		vector = 0x0010;
		if (!((IRR & INTF2) && 0 == (MKL & 0x10)))
			IRR&=~INTF1;
	}
	else
	if ((IRR & INTF2)   && 0 == (MKL & 0x10))
	{
		irqline = UPD7907_INTF2;
		vector = 0x0010;
		IRR&=~INTF2;
	}
	else
	if ((IRR & INTFE0)  && 0 == (MKL & 0x20))
	{
		vector = 0x0018;
		if (!((IRR & INTFE1)    && 0 == (MKL & 0x40)))
			IRR&=~INTFE0;
	}
	else
	if ((IRR & INTFE1)  && 0 == (MKL & 0x40))
	{
		vector = 0x0018;
		IRR&=~INTFE1;
	}
	else
	if ((IRR & INTFEIN) && 0 == (MKL & 0x80))
	{
		vector = 0x0020;
	}
	else
	if ((IRR & INTFAD)  && 0 == (MKH & 0x01))
	{
		vector = 0x0020;
	}
	else
	if ((IRR & INTFSR)  && 0 == (MKH & 0x02))
	{
		vector = 0x0028;
		IRR&=~INTFSR;
	}
	else
	if ((IRR & INTFST)  && 0 == (MKH & 0x04))
	{
		vector = 0x0028;
		IRR&=~INTFST;
	}
	if (vector)
	{
		/* acknowledge external IRQ */
//		if (irqline)
//			(*cpustate->irq_callback)(cpustate->device, irqline);
		SP--;
		WM( SP, PSW );
		SP--;
		WM( SP, PCH );
		SP--;
		WM( SP, PCL );
		IFF = 0;
		PSW &= ~(SK|L0|L1);
		PC = vector;
	}
}

static void __FASTCALL set_irq_line(upd7810_state *cpustate, int irqline, int state)
{
	if (state != CLEAR_LINE)
	{
		if (irqline == INPUT_LINE_NMI)
		{
			/* no nested NMIs ? */
//              if (0 == (IRR & INTNMI))
			{
				IRR |= INTNMI;
				SP--;
				WM( SP, PSW );
				SP--;
				WM( SP, PCH );
				SP--;
				WM( SP, PCL );
				IFF = 0;
				PSW &= ~(SK|L0|L1);
				PC = 0x0004;
			}
		}
		else
		if (irqline == UPD7907_INTF1)
			IRR |= INTF1;
		else
			if ( irqline == UPD7907_INTF2 && ( MKL & 0x20 ) )
				IRR |= INTF2;
		// gamemaster hack
		else
			if (irqline == UPD7907_INTFE1)
				IRR |= INTFE1;
//			else
//				logerror("upd7810_set_irq_line invalid irq line #%d\n", irqline);
	}
	/* resetting interrupt requests is done with the SKIT/SKNIT opcodes only! */
}
