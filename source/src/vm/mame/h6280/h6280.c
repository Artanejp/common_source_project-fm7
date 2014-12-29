/*****************************************************************************

    h6280.c - Portable HuC6280 emulator

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.


    NOTICE:

    This code is around 99% complete!  Several things are unimplemented,
    some due to lack of time, some due to lack of documentation, mainly
    due to lack of programs using these features.

    csh, csl opcodes are not supported.

    I am unsure if flag B is set upon execution of rti.

    Cycle counts should be quite accurate.


    Changelog, version 1.02:
        JMP + indirect X (0x7c) opcode fixed.
        SMB + RMB opcodes fixed in disassembler.
        change_pc function calls removed.
        TSB & TRB now set flags properly.
        BIT opcode altered.

    Changelog, version 1.03:
        Swapped IRQ mask for IRQ1 & IRQ2 (thanks Yasuhiro)

    Changelog, version 1.04, 28/9/99-22/10/99:
        Adjusted RTI (thanks Karl)
        TST opcodes fixed in disassembler (missing break statements in a case!).
        TST behaviour fixed.
        SMB/RMB/BBS/BBR fixed in disassembler.

    Changelog, version 1.05, 8/12/99-16/12/99:
        Added CAB's timer implementation (note: irq ack & timer reload are changed).
        Fixed STA IDX.
        Fixed B flag setting on BRK.
        Assumed CSH & CSL to take 2 cycles each.

        Todo:  Performance could be improved by precalculating timer fire position.

    Changelog, version 1.06, 4/5/00 - last opcode bug found?
        JMP indirect was doing a EAL++; instead of EAD++; - Obviously causing
        a corrupt read when L = 0xff!  This fixes Bloody Wolf and Trio The Punch!

    Changelog, version 1.07, 3/9/00:
        Changed timer to be single shot - fixes Crude Buster music in level 1.

    Changelog, version 1.08, 8/11/05: (Charles MacDonald)

        Changed timer implementation, no longer single shot and reading the
        timer registers returns the count only. Fixes the following:
        - Mesopotamia: Music tempo & in-game timer
        - Dragon Saber: DDA effects
        - Magical Chase: Music tempo and speed regulation
        - Cadash: Allows the first level to start
        - Turrican: Allows the game to start

        Changed PLX and PLY to set NZ flags. Fixes:
        - Afterburner: Graphics unpacking
        - Aoi Blink: Collision detection with background

        Fixed the decimal version of ADC/SBC to *not* update the V flag,
        only the binary ones do.

        Fixed B flag handling so it is always set outside of an interrupt;
        even after being set by PLP and RTI.

        Fixed P state after reset to set I and B, leaving T, D cleared and
        NVZC randomized (cleared in this case).

        Fixed interrupt processing order (Timer has highest priority followed
        by IRQ1 and finally IRQ2).

    Changelog, version 1.09, 1/07/06: (Rob Bohms)

        Added emulation of the T flag, fixes PCE Ankuku Densetsu title screen

    Changelog, version 1.10, 5/09/07: (Wilbert Pol)

        - Taking of interrupts is delayed to respect a pending instruction already
          in the instruction pipeline; fixes After Burner.
        - Added 1 cycle for decimal mode ADC and SBC instructions.
        - Changed cycle counts for CSH and CSL instructions to 3.
        - Added T flag support to the SBC instruction.
        - Fixed ADC T flag to set the Z flag based on the value read.
        - Added 3 cycle penalty to ADC, AND, EOR, ORA, and SBC instructions
          when the T flag is set.
        - Fixed cycle count and support for 65536 byte blocks for the TAI, TDD,
          TIA, TII, and TIN instructions.
        - Fixed RDWORD macro in the disassembler.
        - Fixed setting of N and V flags in the TST instructions.
        - Removed unneeded debug_mmr code.
        - Fixed TSB and TRB instructions.
        - Added 1 delay when accessing the VDC or VCE areas.
        - Implemented low and high speed cpu modes.

    Changelog, version 1.11, 18/09/07: (Wilbert Pol)

        - Improvements to the handling of taking of delayed interrupts.

******************************************************************************/

//#include "emu.h"
//#include "debugger.h"
#include "h6280.h"

static void set_irq_line(h6280_Regs* cpustate, int irqline, int state);

/* include the macros */
#include "h6280ops.h"

/* include the opcode macros, functions and function pointer tables */
#include "tblh6280.c"

/*****************************************************************************/
static CPU_INIT( h6280 )
{
	h6280_Regs *cpustate = (h6280_Regs *)calloc(1, sizeof(h6280_Regs));

	return cpustate;
}

static CPU_RESET( h6280 )
{
	int i;

	/* wipe out the h6280 structure */
	DEVICE *save_program = cpustate->program;
	DEVICE *save_io = cpustate->io;
	memset(cpustate, 0, sizeof(h6280_Regs));
	cpustate->program = save_program;
	cpustate->io = save_io;

	/* set I and B flags */
	P = _fI | _fB;

    /* stack starts at 0x01ff */
	cpustate->sp.d = 0x1ff;

    /* read the reset vector into PC */
	PCL = RDMEM(cpustate, H6280_RESET_VEC);
	PCH = RDMEM(cpustate, (H6280_RESET_VEC+1));

	/* CPU starts in low speed mode */
    cpustate->clocks_per_cycle = 4;

	/* timer off by default */
	cpustate->timer_status=0;
	cpustate->timer_load = 128 * 1024;

    /* clear pending interrupts */
	for (i = 0; i < 3; i++)
		cpustate->irq_state[i] = CLEAR_LINE;
	cpustate->nmi_state = CLEAR_LINE;

	cpustate->irq_pending = 0;
}

static CPU_EXECUTE( h6280 )
{
	int in;

	if (ICount == -1) {
		cpustate->ICount = 1;
	} else {
		cpustate->ICount += ICount;
	}
	int Base_ICount = cpustate->ICount;

	if ( cpustate->irq_pending == 2 ) {
		cpustate->irq_pending--;
	}

	/* Execute instructions */
	do
    {
		cpustate->ppc = cpustate->pc;

		/* Execute 1 instruction */
		in=RDOP();
		PCW++;
		insnh6280[in](cpustate);

		if ( cpustate->irq_pending ) {
			if ( cpustate->irq_pending == 1 ) {
				if ( !(P & _fI) ) {
					cpustate->irq_pending--;
					CHECK_AND_TAKE_IRQ_LINES;
				}
			} else {
				cpustate->irq_pending--;
			}
		}

		/* Check internal timer */
		if(cpustate->timer_status)
		{
			if(cpustate->timer_value<=0)
			{
				if ( ! cpustate->irq_pending )
					cpustate->irq_pending = 1;
				while( cpustate->timer_value <= 0 )
					cpustate->timer_value += cpustate->timer_load;
				set_irq_line(cpustate, 2,ASSERT_LINE);
			}
		}
	} while (cpustate->ICount > 0);

	return Base_ICount - cpustate->ICount;
}

/*****************************************************************************/

static void set_irq_line(h6280_Regs* cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if ( state != ASSERT_LINE ) return;
		cpustate->nmi_state = state;
		CHECK_IRQ_LINES;
	}
	else if (irqline < 3)
	{
		/* If the state has not changed, just return */
		if ( cpustate->irq_state[irqline] == state )
			return;

	    cpustate->irq_state[irqline] = state;

		CHECK_IRQ_LINES;
	}
}



/*****************************************************************************/

READ8_HANDLER( h6280_irq_status_r )
{
	int status;

	switch (offset&3)
	{
	default: return 0;
	case 3:
		{
			status=0;
			if(cpustate->irq_state[1]!=CLEAR_LINE) status|=1; /* IRQ 2 */
			if(cpustate->irq_state[0]!=CLEAR_LINE) status|=2; /* IRQ 1 */
			if(cpustate->irq_state[2]!=CLEAR_LINE) status|=4; /* TIMER */
			return status;
		}
	case 2: return cpustate->irq_mask;
	}
}

WRITE8_HANDLER( h6280_irq_status_w )
{
	switch (offset&3)
	{
		case 2: /* Write irq mask */
			cpustate->irq_mask=data&0x7;
			CHECK_IRQ_LINES;
			break;

		case 3: /* Timer irq ack */
			set_irq_line(cpustate, 2, CLEAR_LINE);
			break;
	}
}

READ8_HANDLER( h6280_timer_r )
{
	/* only returns countdown */
	return ((cpustate->timer_value >> 10)&0x7F);
}

WRITE8_HANDLER( h6280_timer_w )
{
	switch (offset & 1) {
		case 0: /* Counter preload */
			cpustate->timer_load=cpustate->timer_value=((data&127)+1)*1024;
			return;

		case 1: /* Counter enable */
			if(data&1)
			{	/* stop -> start causes reload */
				if(cpustate->timer_status==0) cpustate->timer_value=cpustate->timer_load;
			}
			cpustate->timer_status=data&1;
			return;
	}
}

/*****************************************************************************/

