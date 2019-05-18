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

extern int i386_dasm_one(_TCHAR *buffer, UINT32 eip, const UINT8 *oprom, int mode);

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) mame_printf_debug x; } while (0)


/* All pre-i286 CPUs have a 1MB address space */
#define AMASK   0xfffff


/* I86 registers */
union i8086basicregs
{                                      /* eight general registers */
	UINT16 w[10];                       /* viewed as 16 bits registers */
	UINT8 b[20];                       /* or as 8 bit registers */
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
	UINT8 MF;                      /* V30 mode flag */

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
#ifdef I86_PSEUDO_BIOS
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
	uint64_t total_icount;
	uint64_t prev_total_icount;

	int icount;
	uint32_t waitfactor;
	uint32_t waitcount;

	//char seg_prefix;                   /* prefix segment indicator */
	UINT8 seg_prefix;                   /* prefix segment indicator */
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


#ifdef I80286
extern void i80286_code_descriptor(i80286_state *cpustate, UINT16 selector, UINT16 offset, int gate);
#endif
bool i86_call_pseudo_bios(i8086_state *cpustate, uint32_t PC)
{
#ifdef I86_PSEUDO_BIOS
	if(cpustate->bios != NULL) {
		cpustate->regs.w[8] = 0x0000;
		cpustate->regs.w[9] = 0x0000;
		if(cpustate->bios->bios_call_far_i86(PC, cpustate->regs.w, cpustate->sregs, &cpustate->ZeroVal, &cpustate->CarryVal, &(cpustate->icount), &(cpustate->total_icount))) {
#if 0			
			if((cpustate->regs.w[8] != 0x0000) || (cpustate->regs.w[9] != 0x0000)) {
				UINT32 hi = cpustate->regs.w[9];
				UINT32 lo = cpustate->regs.w[8];
				UINT32 addr = (hi << 16) | lo;
#ifdef I80286
				i80286_code_descriptor(cpustate, cpustate->sregs[1],lo, 1);
#else
				cpustate->base[1] = (cpustate->sregs[1] << 4);
				cpustate->pc = (cpustate->base[1] + addr) & AMASK;
#endif
				
			}
#endif
			return true;
		}
	}
#endif
	return false;
}

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

static CPU_INIT( v30 )
{
	return CPU_INIT_CALL(i8086);
}

static CPU_RESET( i8086 )
{
#ifdef USE_DEBUGGER
	uint64_t total_icount = cpustate->total_icount;
	uint64_t prev_total_icount = cpustate->prev_total_icount;
#endif
	int icount = cpustate->icount;
	int extra_cycles = cpustate->extra_cycles;

	memset(cpustate, 0, sizeof(*cpustate));

	cpustate->sregs[CS] = 0xffff;
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = 0xffff0 & AMASK;
	ExpandFlags(cpustate->flags);

#ifdef USE_DEBUGGER
	cpustate->total_icount = total_icount;
	cpustate->prev_total_icount = prev_total_icount;
#endif
	cpustate->waitcount = 0;
	cpustate->icount = icount;
	cpustate->extra_cycles = extra_cycles;
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

static CPU_RESET( v30 )
{
	CPU_RESET_CALL(i8086);
	SetMD(1);
}

/* ASG 971222 -- added these interface functions */

static void set_irq_line(i8086_state *cpustate, int irqline, int state)
{
	int first_icount = cpustate->icount;

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
	cpustate->extra_cycles += first_icount - cpustate->icount;
	cpustate->icount = first_icount;
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

static void cpu_wait_i86(i8086_state *cpustate,int clocks)
{
	uint32_t ncount = 0;
	if(clocks < 0) return;
	if(cpustate->waitfactor == 0) return;
	uint32_t wcount = cpustate->waitcount;
	wcount += (cpustate->waitfactor * (uint32_t)clocks);
	if(wcount >= 65536) {
		ncount = wcount >> 16;
		wcount = wcount - (ncount << 16);
	}
	cpustate->extra_cycles += ncount;
	cpustate->waitcount = wcount;
}

CPU_EXECUTE( i8086 )
{
	if (cpustate->halted || cpustate->busreq)
	{
#ifdef SINGLE_MODE_DMA
		if(cpustate->dma != NULL) {
			cpustate->dma->do_dma();
		}
#endif
		if (icount == -1) {
			int passed_icount = max(1, cpustate->extra_cycles);
			// this is main cpu, cpustate->icount is not used
			/*cpustate->icount = */cpustate->extra_cycles = 0;
#ifdef USE_DEBUGGER
			cpustate->total_icount += passed_icount;
#endif
			cpu_wait_i86(cpustate, passed_icount);
			return passed_icount;
		} else {
			cpustate->icount += icount;
			int base_icount = cpustate->icount;

			/* adjust for any interrupts that came in */
			cpustate->icount -= cpustate->extra_cycles;
			cpustate->extra_cycles = 0;

			/* if busreq is raised, spin cpu while remained clock */
			if (cpustate->icount > 0) {
				cpustate->icount = 0;
			}
#ifdef USE_DEBUGGER
			cpustate->total_icount += base_icount - cpustate->icount;
#endif
			cpu_wait_i86(cpustate, base_icount - cpustate->icount);
			return base_icount - cpustate->icount;
		}
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
#ifdef USE_DEBUGGER
	cpustate->total_icount += cpustate->extra_cycles;
#endif
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
			cpustate->debugger->add_cpu_trace(cpustate->pc);
			int first_icount = cpustate->icount;
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLE86;
			cpustate->total_icount += first_icount - cpustate->icount;
#ifdef SINGLE_MODE_DMA
			if(cpustate->dma != NULL) {
				cpustate->dma->do_dma();
			}
#endif
			if(now_debugging) {
				if(!cpustate->debugger->now_going) {
					cpustate->debugger->now_suspended = true;
				}
				cpustate->program = cpustate->program_stored;
				cpustate->io = cpustate->io_stored;
			}
		} else {
			cpustate->debugger->add_cpu_trace(cpustate->pc);
			int first_icount = cpustate->icount;
#endif
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLE86;
#ifdef USE_DEBUGGER
			cpustate->total_icount += first_icount - cpustate->icount;
#endif
#ifdef SINGLE_MODE_DMA
			if(cpustate->dma != NULL) {
				cpustate->dma->do_dma();
			}
#endif
#ifdef USE_DEBUGGER
		}
#endif
		/* adjust for any interrupts that came in */
#ifdef USE_DEBUGGER
		cpustate->total_icount += cpustate->extra_cycles;
#endif
		cpustate->icount -= cpustate->extra_cycles;
		cpustate->extra_cycles = 0;
	}

	/* if busreq is raised, spin cpu while remained clock */
	if (cpustate->icount > 0 && cpustate->busreq) {
#ifdef USE_DEBUGGER
		cpustate->total_icount += cpustate->icount;
#endif
		cpustate->icount = 0;
	}
	cpu_wait_i86(cpustate, base_icount - cpustate->icount);
	return base_icount - cpustate->icount;
}

CPU_EXECUTE( i8088 )
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

static void cpu_wait_i186(cpu_state *cpustate,int clocks)
{
	uint32_t ncount = 0;
	if(clocks < 0) return;
	if(cpustate->waitfactor == 0) return;
	uint32_t wcount = cpustate->waitcount;
	wcount += (cpustate->waitfactor * (uint32_t)clocks);
	if(wcount >= 65536) {
		ncount = wcount >> 16;
		wcount = wcount - (ncount << 16);
	}
	cpustate->extra_cycles += ncount;
	cpustate->waitcount = wcount;
}

CPU_EXECUTE( i80186 )
{
	if (cpustate->halted || cpustate->busreq)
	{
#ifdef SINGLE_MODE_DMA
		if (cpustate->dma != NULL) {
			cpustate->dma->do_dma();
		}
#endif
		if (icount == -1) {
			int passed_icount = max(1, cpustate->extra_cycles);
			// this is main cpu, cpustate->icount is not used
			/*cpustate->icount = */cpustate->extra_cycles = 0;
#ifdef USE_DEBUGGER
			cpustate->total_icount += passed_icount;
#endif
			cpu_wait_i186(cpustate, passed_icount);
			return passed_icount;
		} else {
			cpustate->icount += icount;
			int base_icount = cpustate->icount;

			/* adjust for any interrupts that came in */
			cpustate->icount -= cpustate->extra_cycles;
			cpustate->extra_cycles = 0;

			/* if busreq is raised, spin cpu while remained clock */
			if (cpustate->icount > 0) {
				cpustate->icount = 0;
			}
#ifdef USE_DEBUGGER
			cpustate->total_icount += base_icount - cpustate->icount;
#endif
			cpu_wait_i186(cpustate, base_icount - cpustate->icount);
			return base_icount - cpustate->icount;
		}
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
#ifdef USE_DEBUGGER
	cpustate->total_icount += cpustate->extra_cycles;
#endif
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
			cpustate->debugger->add_cpu_trace(cpustate->pc);
			int first_icount = cpustate->icount;
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLE186;
			cpustate->total_icount += first_icount - cpustate->icount;
#ifdef SINGLE_MODE_DMA
			if (cpustate->dma != NULL) {
				cpustate->dma->do_dma();
			}
#endif
			if(now_debugging) {
				if(!cpustate->debugger->now_going) {
					cpustate->debugger->now_suspended = true;
				}
				cpustate->program = cpustate->program_stored;
				cpustate->io = cpustate->io_stored;
			}
		} else {
			cpustate->debugger->add_cpu_trace(cpustate->pc);
			int first_icount = cpustate->icount;
#endif
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLE186;
#ifdef USE_DEBUGGER
			cpustate->total_icount += first_icount - cpustate->icount;
#endif
#ifdef SINGLE_MODE_DMA
			if (cpustate->dma != NULL) {
				cpustate->dma->do_dma();
			}
#endif
#ifdef USE_DEBUGGER
		}
#endif
		/* adjust for any interrupts that came in */
#ifdef USE_DEBUGGER
		cpustate->total_icount += cpustate->extra_cycles;
#endif
		cpustate->icount -= cpustate->extra_cycles;
		cpustate->extra_cycles = 0;
	}

	/* if busreq is raised, spin cpu while remained clock */
	if (cpustate->icount > 0 && cpustate->busreq) {
#ifdef USE_DEBUGGER
		cpustate->total_icount += cpustate->icount;
#endif
		cpustate->icount = 0;
	}
	cpu_wait_i186(cpustate, base_icount - cpustate->icount);
	return base_icount - cpustate->icount;
}

#include "i86.h"

#undef PREFIX
#define PREFIX(name) v30##name
#define PREFIXV30(name) v30##name

#define I80186
#include "instrv30.h"
#include "tablev30.h"

#include "instr86.c"
#include "instrv30.c"
#undef I80186

static void cpu_wait_v30(cpu_state *cpustate,int clocks)
{
	uint32_t ncount = 0;
	if(clocks < 0) return;
	if(cpustate->waitfactor == 0) return;
	uint32_t wcount = cpustate->waitcount;
	wcount += (cpustate->waitfactor * (uint32_t)clocks);
	if(wcount >= 65536) {
		ncount = wcount >> 16;
		wcount = wcount - (ncount << 16);
	}
	cpustate->extra_cycles += ncount;
	cpustate->waitcount = wcount;
}

CPU_EXECUTE( v30 )
{
	if (cpustate->halted || cpustate->busreq)
	{
#ifdef SINGLE_MODE_DMA
		if (cpustate->dma != NULL) {
			cpustate->dma->do_dma();
		}
#endif
		if (icount == -1) {
			int passed_icount = max(1, cpustate->extra_cycles);
			// this is main cpu, cpustate->icount is not used
			/*cpustate->icount = */cpustate->extra_cycles = 0;
#ifdef USE_DEBUGGER
			cpustate->total_icount += passed_icount;
#endif
			cpu_wait_v30(cpustate, passed_icount);
			return passed_icount;
		} else {
			cpustate->icount += icount;
			int base_icount = cpustate->icount;

			/* adjust for any interrupts that came in */
			cpustate->icount -= cpustate->extra_cycles;
			cpustate->extra_cycles = 0;

			/* if busreq is raised, spin cpu while remained clock */
			if (cpustate->icount > 0) {
				cpustate->icount = 0;
			}
#ifdef USE_DEBUGGER
			cpustate->total_icount += base_icount - cpustate->icount;
#endif
			cpu_wait_v30(cpustate, base_icount - cpustate->icount);
			return base_icount - cpustate->icount;
		}
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
#ifdef USE_DEBUGGER
	cpustate->total_icount += cpustate->extra_cycles;
#endif
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
			cpustate->debugger->add_cpu_trace(cpustate->pc);
			int first_icount = cpustate->icount;
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLEV30;
			cpustate->total_icount += first_icount - cpustate->icount;
#ifdef SINGLE_MODE_DMA
			if (cpustate->dma != NULL) {
				cpustate->dma->do_dma();
			}
#endif
			if(now_debugging) {
				if(!cpustate->debugger->now_going) {
					cpustate->debugger->now_suspended = true;
				}
				cpustate->program = cpustate->program_stored;
				cpustate->io = cpustate->io_stored;
			}
		} else {
			cpustate->debugger->add_cpu_trace(cpustate->pc);
			int first_icount = cpustate->icount;
#endif
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLEV30;
#ifdef USE_DEBUGGER
			cpustate->total_icount += first_icount - cpustate->icount;
#endif
#ifdef SINGLE_MODE_DMA
			if (cpustate->dma != NULL) {
				cpustate->dma->do_dma();
			}
#endif
#ifdef USE_DEBUGGER
		}
#endif
		/* adjust for any interrupts that came in */
#ifdef USE_DEBUGGER
		cpustate->total_icount += cpustate->extra_cycles;
#endif
		cpustate->icount -= cpustate->extra_cycles;
		cpustate->extra_cycles = 0;
	}

	/* if busreq is raised, spin cpu while remained clock */
	if (cpustate->icount > 0 && cpustate->busreq) {
#ifdef USE_DEBUGGER
		cpustate->total_icount += cpustate->icount;
#endif
		cpustate->icount = 0;
	}
	cpu_wait_v30(cpustate, base_icount - cpustate->icount);
	return base_icount - cpustate->icount;
}
