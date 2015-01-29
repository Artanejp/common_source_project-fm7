/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/

//#include "emu.h"
//#include "debugger.h"
//#include "host.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) mame_printf_debug x; } while (0)

/* All post-i286 CPUs have a 16MB address space */
#define AMASK   cpustate->amask


#define INPUT_LINE_A20      1

#include "i286.h"


#include "i86time.c"

/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/
/* I86 registers */
union i80286basicregs
{                   /* eight general registers */
	UINT16 w[8];    /* viewed as 16 bits registers */
	UINT8  b[16];   /* or as 8 bit registers */
};

struct i80286_state
{
	i80286basicregs regs;
	UINT32  amask;          /* address mask */
	UINT32  pc;
	UINT32  prevpc;
	UINT16  flags;
	UINT16  msw;
	UINT32  base[4];
	UINT16  sregs[4];
	UINT16  limit[4];
	UINT8 rights[4];
	bool valid[4];
	struct {
		UINT32 base;
		UINT16 limit;
	} gdtr, idtr;
	struct {
		UINT16 sel;
		UINT32 base;
		UINT16 limit;
		UINT8 rights;
	} ldtr, tr;
	DEVICE *pic;
	DEVICE *program;
	DEVICE *io;
#ifdef I86_BIOS_CALL
	DEVICE *bios;
#endif
#ifdef SINGLE_MODE_DMA
	DEVICE *dma;
#endif
#ifdef USE_DEBUGGER
	EMU *emu;
	DEBUGGER *debugger;
	DEVICE *program_stored;
	DEVICE *io_stored;
#endif
	INT32   AuxVal, OverVal, SignVal, ZeroVal, CarryVal, DirVal; /* 0 or non-0 valued flags */
	UINT8   ParityVal;
	UINT8   TF, IF;     /* 0 or 1 valued flags */
	INT8    nmi_state;
	INT8    irq_state;
	INT8    test_state;
	UINT8 rep_in_progress;
	INT32   extra_cycles;       /* extra cycles for interrupts */

	int halted;         /* Is the CPU halted ? */
	int busreq;
	int trap_level;
	int shutdown;

	int icount;
	char seg_prefix;
	UINT8   prefix_seg;
	unsigned ea;
	UINT16 eo; /* HJB 12/13/98 effective offset of the address (before segment is added) */
	UINT8 ea_seg;   /* effective segment of the address */
};

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

static UINT8 parity_table[256];

static struct i80x86_timing timing;

/***************************************************************************/

#define I80286
#include "i86priv.h"
#define PREFIX(fname) i80286##fname
#define PREFIX86(fname) i80286##fname
#define PREFIX186(fname) i80286##fname
#define PREFIX286(fname) i80286##fname
#define i8086_state i80286_state

#include "ea.h"
#include "modrm286.h"
#include "instr86.h"
#include "instr186.h"
#include "instr286.h"
#include "table286.h"
#include "instr86.c"
#include "instr186.c"
#include "instr286.c"

static void i80286_urinit(void)
{
	unsigned int i,j,c;
	static const BREGS reg_name[8]={ AL, CL, DL, BL, AH, CH, DH, BH };

	for (i = 0;i < 256; i++)
	{
		for (j = i, c = 0; j > 0; j >>= 1)
			if (j & 1) c++;

		parity_table[i] = !(c & 1);
	}

	for (i = 0; i < 256; i++)
	{
		Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = (WREGS) ( (i & 0x38) >> 3) ;
	}

	for (i = 0xc0; i < 0x100; i++)
	{
		Mod_RM.RM.w[i] = (WREGS)( i & 7 );
		Mod_RM.RM.b[i] = (BREGS)reg_name[i & 7];
	}
}

static void i80286_set_a20_line(i80286_state *cpustate, int state)
{
	cpustate->amask = state ? 0x00ffffff : 0x000fffff;
}

static CPU_RESET( i80286 )
{
	memset(&cpustate->regs, 0, sizeof(i80286basicregs));
	cpustate->sregs[CS] = 0xf000;
	cpustate->base[CS] = 0xff0000;
//	/* temporary, until I have the right reset vector working */
//	cpustate->base[CS] = cpustate->sregs[CS] << 4;
//	cpustate->pc = 0xffff0;
	cpustate->pc = 0xfffff0;
	cpustate->limit[CS]=cpustate->limit[SS]=cpustate->limit[DS]=cpustate->limit[ES]=0xffff;
	cpustate->sregs[DS]=cpustate->sregs[SS]=cpustate->sregs[ES]=0;
	cpustate->base[DS]=cpustate->base[SS]=cpustate->base[ES]=0;
	cpustate->rights[DS]=cpustate->rights[SS]=cpustate->rights[ES]=0x93;
	cpustate->rights[CS]=0x9a;
	cpustate->valid[CS]=cpustate->valid[SS]=cpustate->valid[DS]=cpustate->valid[ES]=1;
	cpustate->msw=0xfff0;
	cpustate->flags=2;
	ExpandFlags(cpustate->flags);
	cpustate->idtr.base=0;cpustate->idtr.limit=0x3ff;
	cpustate->gdtr.base=cpustate->ldtr.base=cpustate->tr.base=0;
	cpustate->gdtr.limit=cpustate->ldtr.limit=cpustate->tr.limit=0;
	cpustate->ldtr.rights=cpustate->tr.rights=0;
	cpustate->ldtr.sel=cpustate->tr.sel=0;
	cpustate->rep_in_progress = FALSE;
	cpustate->seg_prefix = FALSE;

	CHANGE_PC(cpustate->pc);

	cpustate->icount = cpustate->extra_cycles = 0;
	cpustate->halted = 0;
}

/****************************************************************************/

/* ASG 971222 -- added these interface functions */

static void set_irq_line(i80286_state *cpustate, int irqline, int state)
{
	if (state != CLEAR_LINE && cpustate->halted)
	{
		cpustate->halted = 0;
	}
	try
	{
		if (irqline == INPUT_LINE_NMI)
		{
			if (cpustate->nmi_state == state)
				return;
			cpustate->nmi_state = state;

			/* on a rising edge, signal the NMI */
			if (state != CLEAR_LINE) {
				i80286_interrupt_descriptor(cpustate, I8086_NMI_INT_VECTOR, 2, -1);
				cpustate->nmi_state = CLEAR_LINE;
			}
		}
		else
		{
			cpustate->irq_state = state;

			/* if the IF is set, signal an interrupt */
			if (state != CLEAR_LINE && cpustate->IF) {
				i80286_interrupt_descriptor(cpustate, cpustate->pic->intr_ack(), 2, -1);
				cpustate->irq_state = CLEAR_LINE;
			}
		}
	}
	catch (UINT32 e)
	{
		i80286_trap2(cpustate, e);
	}
}

static CPU_EXECUTE( i80286 )
{
	if (cpustate->halted || cpustate->busreq)
	{
#ifdef SINGLE_MODE_DMA
		if (cpustate->dma != NULL) {
			cpustate->dma->do_dma();
		}
#endif
		int passed_icount = min(1, cpustate->extra_cycles);
		cpustate->icount = cpustate->extra_cycles = 0;
		return passed_icount;
	}

	if (icount == -1) {
		cpustate->icount = 1;
	} else {
		cpustate->icount += icount;
	}
	int base_icount = cpustate->icount;

	/* copy over the cycle counts if they're not correct */
	if (timing.id != 80286)
		timing = i80286_cycles;

	/* adjust for any interrupts that came in */
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	/* run until we're out */
	while(cpustate->icount > 0 && !cpustate->busreq)
	{
#ifdef USE_DEBUGGER
		bool now_debugging = cpustate->debugger->now_debugging;
		if(now_debugging) {
			cpustate->debugger->check_break_points(cpustate->pc);
			if(cpustate->debugger->now_suspended) {
				cpustate->emu->mute_sound();
				while(cpustate->debugger->now_debugging && cpustate->debugger->now_suspended) {
					Sleep(10);
				}
			}
			if(cpustate->debugger->now_debugging) {
				cpustate->program = cpustate->io = cpustate->debugger;
			} else {
				now_debugging = false;
			}
			cpustate->seg_prefix=FALSE;
			try
			{
				if (PM && ((cpustate->pc-cpustate->base[CS]) > cpustate->limit[CS]))
					throw TRAP(GENERAL_PROTECTION_FAULT, cpustate->sregs[CS] & ~3);
				cpustate->prevpc = cpustate->pc;

				TABLE286 // call instruction
			}
			catch (UINT32 e)
			{
				i80286_trap2(cpustate,e);
			}
			if(now_debugging) {
				if(!cpustate->debugger->now_going) {
					cpustate->debugger->now_suspended = true;
				}
				cpustate->program = cpustate->program_stored;
				cpustate->io = cpustate->io_stored;
			}
		} else {
#endif
			cpustate->seg_prefix=FALSE;
			try
			{
				if (PM && ((cpustate->pc-cpustate->base[CS]) > cpustate->limit[CS]))
					throw TRAP(GENERAL_PROTECTION_FAULT, cpustate->sregs[CS] & ~3);
				cpustate->prevpc = cpustate->pc;

				TABLE286 // call instruction
			}
			catch (UINT32 e)
			{
				i80286_trap2(cpustate,e);
			}
#ifdef USE_DEBUGGER
		}
#endif
#ifdef SINGLE_MODE_DMA
		if (cpustate->dma != NULL) {
			cpustate->dma->do_dma();
		}
#endif
		/* adjust for any interrupts that came in */
		cpustate->icount -= cpustate->extra_cycles;
		cpustate->extra_cycles = 0;
	}

	int passed_icount = base_icount - cpustate->icount;

	if (cpustate->icount > 0 && cpustate->busreq) {
		cpustate->icount = 0;
	}

	return passed_icount;
}

static CPU_INIT( i80286 )
{
	i80286_state *cpustate = (i80286_state *)calloc(1, sizeof(i80286_state));

	cpustate->amask = 0xfffff;

	i80286_urinit();

	return cpustate;
}

#undef I80286
