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

static int i386_dasm_one(_TCHAR *buffer, UINT32 eip, const UINT8 *oprom, int mode);

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) mame_printf_debug x; } while (0)


#include "i86time.c"

/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/
/* I86 registers */
/* ADD DUMMY ENTRY 20181126 K.O */
union i80286basicregs
{                   /* eight general registers */
	UINT16 w[10];    /* viewed as 16 bits registers */
	UINT8  b[20];   /* or as 8 bit registers */
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
//#ifdef I86_PSEUDO_BIOS
	DEVICE *bios;
//#endif
//#ifdef SINGLE_MODE_DMA
	DEVICE *dma;
//#endif
//#ifdef USE_DEBUGGER
	EMU_TEMPLATE *emu;
	DEBUGGER *debugger;
	DEVICE *program_stored;
	DEVICE *io_stored;
//#endif
	INT32   AuxVal, OverVal, SignVal, ZeroVal, CarryVal, DirVal; /* 0 or non-0 valued flags */
	UINT8   ParityVal;
	UINT8   TF, IF;     /* 0 or 1 valued flags */
	UINT8   MF;         /* V30 mode flag */
	INT8    nmi_state;
	INT8    irq_state;
	INT8    test_state;
	UINT8 rep_in_progress;
	INT32   extra_cycles;       /* extra cycles for interrupts */

	int halted;         /* Is the CPU halted ? */
	int busreq;
	int haltreq;
	int trap_level;
	int shutdown;
	uint32_t waitfactor;
	uint32_t waitcount;
	int32_t memory_wait;

//#ifdef USE_DEBUGGER
	uint64_t total_icount;
	uint64_t prev_total_icount;
//#endif
	int icount;
	//char seg_prefix;
	UINT8 seg_prefix;
	UINT8   prefix_seg;
	unsigned ea;
	UINT16 eo; /* HJB 12/13/98 effective offset of the address (before segment is added) */
	UINT8 ea_seg;   /* effective segment of the address */
};

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

static UINT8 parity_table[256];

static struct i80x86_timing timing;

#ifdef I80286
extern void i80286_code_descriptor(i80286_state *cpustate, UINT16 selector, UINT16 offset, int gate);
#endif
bool i286_call_pseudo_bios(i80286_state *cpustate, uint32_t PC)
{
//#ifdef I86_PSEUDO_BIOS
	if(cpustate->bios != NULL) {
		cpustate->regs.w[8] = 0x0000;
		cpustate->regs.w[9] = 0x0000;
		if(cpustate->bios->bios_call_far_i86(PC & AMASK, cpustate->regs.w, cpustate->sregs, &cpustate->ZeroVal, &cpustate->CarryVal, &(cpustate->icount), &(cpustate->total_icount))) {
			return true;

		}
	}
//#endif
	return false;
}

inline UINT8 __FASTCALL read_mem_byte_(i80286_state *cpustate, unsigned a)
{
	int wait = 0;
	UINT8 d = cpustate->program->read_data8w(a, &wait);
	cpustate->icount -= wait;
	return d;
}

inline UINT16 __FASTCALL read_mem_word_(i80286_state *cpustate, unsigned a)
{
	int wait = 0;
	UINT16 d = cpustate->program->read_data16w(a, &wait);
	cpustate->icount -= wait;
	return d;
}

inline void __FASTCALL write_mem_byte_(i80286_state *cpustate, unsigned a, UINT8 d)
{
	int wait = 0;
	cpustate->program->write_data8w(a, d, &wait);
	cpustate->icount -= wait;
}

inline void __FASTCALL write_mem_word_(i80286_state *cpustate, unsigned a, UINT16 d)
{
	int wait = 0;
	cpustate->program->write_data16w(a, d, &wait);
	cpustate->icount -= wait;
}

inline UINT8 __FASTCALL read_port_byte_(i80286_state *cpustate, unsigned a)
{
	int wait = 0;
	UINT8 d = cpustate->io->read_io8w(a, &wait);
	cpustate->icount -= wait;
	return d;
}

inline UINT16 __FASTCALL read_port_word_(i80286_state *cpustate, unsigned a)
{
	int wait = 0;
	UINT16 d = cpustate->io->read_io16w(a, &wait);
	cpustate->icount -= wait;
	return d;
}

inline void __FASTCALL write_port_byte_(i80286_state *cpustate, unsigned a, UINT8 d)
{
	int wait = 0;
	cpustate->io->write_io8w(a, d, &wait);
	cpustate->icount -= wait;
}

inline void __FASTCALL write_port_word_(i80286_state *cpustate, unsigned a, UINT16 d)
{
	int wait = 0;
	cpustate->io->write_io16w(a, d, &wait);
	cpustate->icount -= wait;
}

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

	int halted = cpustate->halted;
	int busreq = cpustate->busreq;
	int haltreq = cpustate->haltreq;

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
	cpustate->rights[CS]=0x93;
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
	cpustate->waitcount = 0;
	cpustate->memory_wait = 0;

	CHANGE_PC(cpustate->pc);

//	cpustate->icount = cpustate->extra_cycles = 0;
	cpustate->halted = halted;
	cpustate->busreq = busreq;
	cpustate->haltreq = haltreq;
}

/****************************************************************************/

/* ASG 971222 -- added these interface functions */

static void set_irq_line(i80286_state *cpustate, int irqline, int state)
{
	int first_icount = cpustate->icount;
	if (cpustate->haltreq != 0) {
		cpustate->extra_cycles += first_icount - cpustate->icount;
		cpustate->icount = first_icount;
		return;
	}

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
				i80286_interrupt_descriptor(cpustate, cpustate->pic->get_intr_ack(), 2, -1);
				cpustate->irq_state = CLEAR_LINE;
			}
		}
	}
	catch (UINT32 e)
	{
		i80286_trap2(cpustate, e);
	}
	cpustate->extra_cycles += first_icount - cpustate->icount;
	cpustate->icount = first_icount;
}

static void __FASTCALL cpu_wait_i286(cpu_state *cpustate,int clocks)
{
	if(clocks <= 0) clocks = 1;
	int64_t wfactor = cpustate->waitfactor;
	int64_t wcount = cpustate->waitcount;
	int64_t mwait = cpustate->memory_wait;
	int64_t ncount;
	if(cpustate->waitfactor >= 65536) {
		wcount += ((wfactor - 65536) * clocks); // Append wait due to be slower clock.
	}
	wcount += (wfactor * mwait);  // memory wait
	if(wcount >= 65536) {
		ncount = wcount >> 16;
		wcount = wcount - (ncount << 16);
		cpustate->extra_cycles += (int)ncount;
	} else if(wcount < 0) {
		wcount = 0;
	}
	cpustate->waitcount = wcount;
	cpustate->memory_wait = 0;
}

static CPU_EXECUTE( i80286 )
{
	if (cpustate->halted || cpustate->busreq || cpustate->haltreq)
	{
//#ifdef SINGLE_MODE_DMA
		if(!cpustate->haltreq) {
			if (cpustate->dma != NULL) {
				cpustate->dma->do_dma();
			}
		}
//#endif
		bool now_debugging = false;
		if(cpustate->debugger != NULL) {
			now_debugging = cpustate->debugger->now_debugging;
		}
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
			if(now_debugging) {
				if(!cpustate->debugger->now_going) {
					cpustate->debugger->now_suspended = true;
				}
				cpustate->program = cpustate->program_stored;
				cpustate->io = cpustate->io_stored;
			}
		}
		if (icount == -1) {
			int passed_icount = max(5, cpustate->extra_cycles); // 80286 CPI: 4.8
			// this is main cpu, cpustate->icount is not used
			/*cpustate->icount = */cpustate->extra_cycles = 0;
//#ifdef USE_DEBUGGER
			cpustate->total_icount += passed_icount;
//#endif
			cpu_wait_i286(cpustate,passed_icount);
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
//#ifdef USE_DEBUGGER
			cpustate->total_icount += base_icount - cpustate->icount;
//#endif
			cpu_wait_i286(cpustate, base_icount - cpustate->icount);
			int passed_icount = base_icount - cpustate->icount;
			cpustate->icount = 0;
			return passed_icount;

		}
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
//#ifdef USE_DEBUGGER
	cpustate->total_icount += cpustate->extra_cycles;
//#endif
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	/* run until we're out */
	while(cpustate->icount > 0 && !cpustate->busreq && !cpustate->haltreq)
	{
//#ifdef USE_DEBUGGER
		bool now_debugging = false;
		if(cpustate->debugger != NULL) {
			now_debugging = cpustate->debugger->now_debugging;
		}
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
			cpustate->total_icount += first_icount - cpustate->icount;
//#ifdef SINGLE_MODE_DMA
			if(!cpustate->haltreq) {
				if (cpustate->dma != NULL) {
					cpustate->dma->do_dma();
				}
			}
//#endif
			if(now_debugging) {
				if(!cpustate->debugger->now_going) {
					cpustate->debugger->now_suspended = true;
				}
				cpustate->program = cpustate->program_stored;
				cpustate->io = cpustate->io_stored;
			}
		} else {
			if(cpustate->debugger != NULL) {
				cpustate->debugger->add_cpu_trace(cpustate->pc);
			}
			int first_icount = cpustate->icount;
//#endif
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
//#ifdef USE_DEBUGGER
			cpustate->total_icount += first_icount - cpustate->icount;
//#endif
//#ifdef SINGLE_MODE_DMA
			if(!cpustate->haltreq) {
				if (cpustate->dma != NULL) {
					cpustate->dma->do_dma();
				}
			}
//#endif
//#ifdef USE_DEBUGGER
		}
//#endif
		/* adjust for any interrupts that came in */
//#ifdef USE_DEBUGGER
		cpustate->total_icount += cpustate->extra_cycles;
//#endif
		cpustate->icount -= cpustate->extra_cycles;
		cpustate->extra_cycles = 0;
	}

	/* if busreq is raised, spin cpu while remained clock */
	if (cpustate->icount > 0 && (cpustate->busreq || cpustate->haltreq)) {
//#ifdef USE_DEBUGGER
		cpustate->total_icount += cpustate->icount;
//#endif
		cpustate->icount = 0;
	}
	cpu_wait_i286(cpustate, base_icount - cpustate->icount);
	int passed_icount = base_icount - cpustate->icount;
	cpustate->icount = 0;
	return passed_icount;

}

static CPU_INIT( i80286 )
{
	i80286_state *cpustate = (i80286_state *)calloc(1, sizeof(i80286_state));

	cpustate->amask = 0xfffff;

	i80286_urinit();
	cpustate->waitfactor = 65536;
	return cpustate;
}

#undef I80286
