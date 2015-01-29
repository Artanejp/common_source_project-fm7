/****************************************************************************
*             real mode i286 emulator v1.4 by Fabrice Frances               *
*               (initial work based on David Hedley's pcemu)                *
****************************************************************************/
/* 26.March 2000 PeT changed set_irq_line */

//#include "emu.h"
//#include "debugger.h"

//#include "host.h"
#include "i86priv.h"
#include "i86.h"

extern int i386_dasm_one(char *buffer, UINT32 eip, const UINT8 *oprom, int mode);

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) mame_printf_debug x; } while (0)


/* All pre-i286 CPUs have a 1MB address space */
#define AMASK   0xfffff


/* I86 registers */
union i8086basicregs
{                                      /* eight general registers */
	UINT16 w[8];                       /* viewed as 16 bits registers */
	UINT8 b[16];                       /* or as 8 bit registers */
};

struct i8086_state
{
	i8086basicregs regs;
	UINT32 pc;
	UINT32 prevpc;
	UINT32 base[4];
	UINT16 sregs[4];
	UINT16 flags;
	INT32 AuxVal, OverVal, SignVal, ZeroVal, CarryVal, DirVal;      /* 0 or non-0 valued flags */
	UINT8 ParityVal;
	UINT8 TF, IF;                  /* 0 or 1 valued flags */
	UINT8 int_vector;
	INT8 nmi_state;
	INT8 irq_state;
	INT8 test_state;
	UINT8 rep_in_progress;
	INT32 extra_cycles;       /* extra cycles for interrupts */

	int halted;         /* Is the CPU halted ? */
	int busreq;

	UINT16 ip;
	UINT32 sp;

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
	int icount;

	char seg_prefix;                   /* prefix segment indicator */
	UINT8   prefix_seg;                 /* The prefixed segment */
	unsigned ea;
	UINT16 eo; /* HJB 12/13/98 effective offset of the address (before segment is added) */
	UINT8 ea_seg;   /* effective segment of the address */
};

#include "i86time.c"

/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/


static struct i80x86_timing timing;

static UINT8 parity_table[256];

/* The interrupt number of a pending external interrupt pending NMI is 2.   */
/* For INTR interrupts, the level is caught on the bus during an INTA cycle */

#define PREFIX(name) i8086##name
#define PREFIX86(name) i8086##name

#define I8086
#include "instr86.h"
#include "ea.h"
#include "modrm.h"
#include "table86.h"

#include "instr86.c"
#undef I8086


/***************************************************************************/
static CPU_INIT( i8086 )
{
	i8086_state *cpustate = (i8086_state *)calloc(1, sizeof(i8086_state));
	unsigned int i, j, c;
	static const BREGS reg_name[8] = {AL, CL, DL, BL, AH, CH, DH, BH};
	for (i = 0; i < 256; i++)
	{
		for (j = i, c = 0; j > 0; j >>= 1)
			if (j & 1)
				c++;

		parity_table[i] = !(c & 1);
	}

	for (i = 0; i < 256; i++)
	{
		Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = (WREGS) ((i & 0x38) >> 3);
	}

	for (i = 0xc0; i < 0x100; i++)
	{
		Mod_RM.RM.w[i] = (WREGS) (i & 7);
		Mod_RM.RM.b[i] = (BREGS) reg_name[i & 7];
	}

	return cpustate;
}

static CPU_INIT( i8088 )
{
	return CPU_INIT_CALL(i8086);
}

static CPU_INIT( i80186 )
{
	return CPU_INIT_CALL(i8086);
}

static CPU_RESET( i8086 )
{
	memset(cpustate, 0, sizeof(*cpustate));

	cpustate->sregs[CS] = 0xffff;
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = 0xffff0 & AMASK;
	ExpandFlags(cpustate->flags);

	cpustate->halted = 0;
}

static CPU_RESET( i8088 )
{
	CPU_RESET_CALL(i8086);
}

static CPU_RESET( i80186 )
{
	CPU_RESET_CALL(i8086);
}

/* ASG 971222 -- added these interface functions */

static void set_irq_line(i8086_state *cpustate, int irqline, int state)
{
	if (state != CLEAR_LINE && cpustate->halted)
	{
		cpustate->halted = 0;
	}

	if (irqline == INPUT_LINE_NMI)
	{
		if (cpustate->nmi_state == state)
			return;
		cpustate->nmi_state = state;

		/* on a rising edge, signal the NMI */
		if (state != CLEAR_LINE)
		{
			PREFIX(_interrupt)(cpustate, I8086_NMI_INT_VECTOR);
			cpustate->nmi_state = CLEAR_LINE;
		}
	}
	else
	{
		cpustate->irq_state = state;

		/* if the IF is set, signal an interrupt */
		if (state != CLEAR_LINE && cpustate->IF) {
			PREFIX(_interrupt)(cpustate, (UINT32)-1);
			cpustate->irq_state = CLEAR_LINE;
		}
	}
}

static void set_drq_line(i8086_state *cpustate, int irqline, int state)
{
	// TODO implement me
}

static void set_tmrin_line(i8086_state *cpustate, int irqline, int state)
{
	// TODO implement me
}

/* PJB 03/05 */
static void set_test_line(i8086_state *cpustate, int state)
{
	cpustate->test_state = !state;
}

static CPU_EXECUTE( i8086 )
{
	if (cpustate->halted || cpustate->busreq)
	{
#ifdef SINGLE_MODE_DMA
		if(cpustate->dma != NULL) {
			cpustate->dma->do_dma();
		}
#endif
		int passed_icount = max(1, cpustate->extra_cycles);
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
	if (timing.id != 8086)
		timing = i8086_cycles;

	/* adjust for any interrupts that came in */
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	/* run until we're out */
	while (cpustate->icount > 0 && !cpustate->busreq)
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
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLE86;
			if(now_debugging) {
				if(!cpustate->debugger->now_going) {
					cpustate->debugger->now_suspended = true;
				}
				cpustate->program = cpustate->program_stored;
				cpustate->io = cpustate->io_stored;
			}
		} else {
#endif
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLE86;
#ifdef USE_DEBUGGER
		}
#endif
#ifdef SINGLE_MODE_DMA
		if(cpustate->dma != NULL) {
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

static CPU_EXECUTE( i8088 )
{
	return CPU_EXECUTE_CALL(i8086);
}

#include "i86.h"

#undef PREFIX
#define PREFIX(name) i80186##name
#define PREFIX186(name) i80186##name

#define I80186
#include "instr186.h"
#include "table186.h"

#include "instr86.c"
#include "instr186.c"
#undef I80186

static CPU_EXECUTE( i80186 )
{
	if (cpustate->halted || cpustate->busreq)
	{
#ifdef SINGLE_MODE_DMA
		if (cpustate->dma != NULL) {
			cpustate->dma->do_dma();
		}
#endif
		int passed_icount = max(1, cpustate->extra_cycles);
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
	if (timing.id != 80186)
		timing = i80186_cycles;

	/* adjust for any interrupts that came in */
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	/* run until we're out */
	while (cpustate->icount > 0 && !cpustate->busreq)
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
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLE186;
			if(now_debugging) {
				if(!cpustate->debugger->now_going) {
					cpustate->debugger->now_suspended = true;
				}
				cpustate->program = cpustate->program_stored;
				cpustate->io = cpustate->io_stored;
			}
		} else {
#endif
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLE186;
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

