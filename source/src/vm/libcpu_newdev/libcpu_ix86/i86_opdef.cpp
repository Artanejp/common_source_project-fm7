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

//extern int i386_dasm_one(_TCHAR *buffer, UINT32 eip, const UINT8 *oprom, int mode);

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) mame_printf_debug x; } while (0)


/* All pre-i286 CPUs have a 1MB address space */
#define AMASK   0xfffff


//#include "i86time.c"

/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/

/* The interrupt number of a pending external interrupt pending NMI is 2.   */
/* For INTR interrupts, the level is caught on the bus during an INTA cycle */

//#define PREFIX(name) i8086##name
//#define PREFIX86(name) i8086##name

//#define I8086
//#include "./instr86.h"
#include "./ea.h"
#include "./modrm.h"
#include "./table86.h"

//#include "instr86.c"
//#undef I8086


/***************************************************************************/
i8086_state *X86_OPS_BASE::cpu_init_i8086()
{
	cpustate = (i8086_state *)calloc(1, sizeof(i8086_state));
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

i8086_state *X86_OPS_BASE::cpu_init_i8088()
{
	return cpu_init_i8086()
}

i8086_state *X86_OPS_BASE::cpu_init_i80186()
{
	return cpu_init_i8086()

}

i8086_state *X86_OPS_BASE::cpu_init_v30()
{
	return cpu_init_i8086()
}

void X86_OPS_BASE::CPU_RESET( i8086 )
{
	memset(cpustate, 0, sizeof(*cpustate));

	cpustate->sregs[CS] = 0xffff;
	cpustate->base[CS] = SegBase(CS);
	cpustate->pc = 0xffff0 & AMASK;
	ExpandFlags(cpustate->flags);

	cpustate->halted = 0;
}

void X86_OPS_BASE::CPU_RESET( i8088 )
{
	CPU_RESET_CALL(i8086);
}

void X86_OPS_BASE::CPU_RESET( i80186 )
{
	CPU_RESET_CALL(i8086);
}

void X86_OPS_BASE::CPU_RESET( v30 )
{
	CPU_RESET_CALL(i8086);
	SetMD(1);
}

/* ASG 971222 -- added these interface functions */

void X86_OPS_BASE::set_irq_line_86(int irqline, int state)
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
			PREFIX86(_interrupt)(cpustate, I8086_NMI_INT_VECTOR);
			cpustate->nmi_state = CLEAR_LINE;
		}
	}
	else
	{
		cpustate->irq_state = state;

		/* if the IF is set, signal an interrupt */
		if (state != CLEAR_LINE && cpustate->IF) {
			PREFIXV30(_interrupt)(cpustate, (UINT32)-1);
			cpustate->irq_state = CLEAR_LINE;
		}
	}
}

void X86_OPS_BASE::set_drq_line_86(int irqline, int state)
{
	// TODO implement me
}

void X86_OPS_BASE::set_tmrin_line_86(i8086_state *cpustate, int irqline, int state)
{
	// TODO implement me
}

/* PJB 03/05 */
void X86_OPS_BASE::set_test_line_86(i8086_state *cpustate, int state)
{
	cpustate->test_state = !state;
}

void X86_OPS_BASE::CPU_EXECUTE( i8086 )
{
	if (cpustate->halted || cpustate->busreq)
	{
		if (icount == -1) {
			int passed_icount = max(1, cpustate->extra_cycles);
			// this is main cpu, cpustate->icount is not used
			/*cpustate->icount = */cpustate->extra_cycles = 0;
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
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	/* run until we're out */
	while (cpustate->icount > 0 && !cpustate->busreq)
	{
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLE86;
		/* adjust for any interrupts that came in */
		cpustate->icount -= cpustate->extra_cycles;
		cpustate->extra_cycles = 0;
	}

	/* if busreq is raised, spin cpu while remained clock */
	if (cpustate->icount > 0 && cpustate->busreq) {
		cpustate->icount = 0;
	}
	return base_icount - cpustate->icount;
}

void X86_OPS_BASE::CPU_EXECUTE( i8088 )
{
	return CPU_EXECUTE_CALL(i8086);
}

//#include "i86.h"

//#undef PREFIX
//#define PREFIX(name) i80186##name
//#define PREFIX186(name) i80186##name

//#define I80186
//#include "instr186.h"
#include "table186.h"

//#include "instr86.c"
//#include "instr186.c"
//#undef I80186

void X86_OPS_BASE::CPU_EXECUTE( i80186 )
{
	if (cpustate->halted || cpustate->busreq)
	{
		if (icount == -1) {
			int passed_icount = max(1, cpustate->extra_cycles);
			// this is main cpu, cpustate->icount is not used
			/*cpustate->icount = */cpustate->extra_cycles = 0;
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
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	/* run until we're out */
	while (cpustate->icount > 0 && !cpustate->busreq)
	{
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLE186;
		/* adjust for any interrupts that came in */
		cpustate->icount -= cpustate->extra_cycles;
		cpustate->extra_cycles = 0;
	}

	/* if busreq is raised, spin cpu while remained clock */
	if (cpustate->icount > 0 && cpustate->busreq) {
		cpustate->icount = 0;
	}
	return base_icount - cpustate->icount;
}
void X86_OPS_BASE::set_irq_line_186(int irqline, int state)
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
			PREFIX186(_interrupt)(cpustate, I8086_NMI_INT_VECTOR);
			cpustate->nmi_state = CLEAR_LINE;
		}
	}
	else
	{
		cpustate->irq_state = state;

		/* if the IF is set, signal an interrupt */
		if (state != CLEAR_LINE && cpustate->IF) {
			PREFIX186(_interrupt)(cpustate, (UINT32)-1);
			cpustate->irq_state = CLEAR_LINE;
		}
	}
}


//#include "i86.h"

//#undef PREFIX
//#define PREFIX(name) v30##name
//#define PREFIXV30(name) v30##name

//#define I80186
//#include "instrv30.h"
#include "tablev30.h"

//#include "instr86.c"
//#include "instrv30.c"
//#undef I80186

void X86_OPS_BASE::CPU_EXECUTE( v30 )
{
	if (cpustate->halted || cpustate->busreq)
	{
		if (icount == -1) {
			int passed_icount = max(1, cpustate->extra_cycles);
			// this is main cpu, cpustate->icount is not used
			/*cpustate->icount = */cpustate->extra_cycles = 0;
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
	cpustate->icount -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	/* run until we're out */
	while (cpustate->icount > 0 && !cpustate->busreq)
	{
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLEV30;
		/* adjust for any interrupts that came in */
		cpustate->icount -= cpustate->extra_cycles;
		cpustate->extra_cycles = 0;
	}

	/* if busreq is raised, spin cpu while remained clock */
	if (cpustate->icount > 0 && cpustate->busreq) {
		cpustate->icount = 0;
	}
	return base_icount - cpustate->icount;
}

void X86_OPS_BASE::set_irq_line_v30(int irqline, int state)
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
			PREFIXV30(_interrupt)(cpustate, I8086_NMI_INT_VECTOR);
			cpustate->nmi_state = CLEAR_LINE;
		}
	}
	else
	{
		cpustate->irq_state = state;

		/* if the IF is set, signal an interrupt */
		if (state != CLEAR_LINE && cpustate->IF) {
			PREFIXV30(_interrupt)(cpustate, (UINT32)-1);
			cpustate->irq_state = CLEAR_LINE;
		}
	}
}

