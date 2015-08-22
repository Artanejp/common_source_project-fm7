/*****************************************************************************

    h6280.h Portable Hu6280 emulator interface

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.

******************************************************************************/

#pragma once

#ifndef __H6280_H__
#define __H6280_H__


#define LAZY_FLAGS  0

#define H6280_RESET_VEC	0xfffe
#define H6280_NMI_VEC	0xfffc
#define H6280_TIMER_VEC	0xfffa
#define H6280_IRQ1_VEC	0xfff8
#define H6280_IRQ2_VEC	0xfff6			/* Aka BRK vector */


/****************************************************************************
 * The 6280 registers.
 ****************************************************************************/
struct h6280_Regs
{
	int ICount;

	PAIR  ppc;			/* previous program counter */
    PAIR  pc;           /* program counter */
    PAIR  sp;           /* stack pointer (always 100 - 1FF) */
    PAIR  zp;           /* zero page address */
    PAIR  ea;           /* effective address */
    UINT8 a;            /* Accumulator */
    UINT8 x;            /* X index register */
    UINT8 y;            /* Y index register */
    UINT8 p;            /* Processor status */
    UINT8 mmr[8];       /* Hu6280 memory mapper registers */
    UINT8 irq_mask;     /* interrupt enable/disable */
    UINT8 timer_status; /* timer status */
	UINT8 timer_ack;	/* timer acknowledge */
    UINT8 clocks_per_cycle; /* 4 = low speed mode, 1 = high speed mode */
    INT32 timer_value;    /* timer interrupt */
    INT32 timer_load;		/* reload value */
    UINT8 nmi_state;
    UINT8 irq_state[3];
	UINT8 irq_pending;
	DEVICE *program;
	DEVICE *io;
#ifdef USE_DEBUGGER
	EMU *emu;
	DEBUGGER *debugger;
	DEVICE *program_stored;
	DEVICE *io_stored;
#endif

#if LAZY_FLAGS
    INT32 NZ;			/* last value (lazy N and Z flag) */
#endif
};

READ8_HANDLER( h6280_irq_status_r );
WRITE8_HANDLER( h6280_irq_status_w );

READ8_HANDLER( h6280_timer_r );
WRITE8_HANDLER( h6280_timer_w );

#endif /* __H6280_H__ */
