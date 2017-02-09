#include "./i86priv.h"
#include "./i86.h"

//#ifdef I80286
//#undef GetMemB
//#undef GetMemW
//#undef PutMemB
//#undef PutMemW
//#undef PUSH
//#undef POP
//#undef IOPL
//#undef NT
//#undef xF

//#define GetMemB(Seg,Off)        (read_mem_byte(GetMemAddr(cpustate,Seg,Off,1,I80286_READ)))
//#define GetMemW(Seg,Off)        (read_mem_word(GetMemAddr(cpustate,Seg,Off,2,I80286_READ)))
//#define PutMemB(Seg,Off,x)      write_mem_byte(GetMemAddr(cpustate,Seg,Off,1,I80286_WRITE), (x))
//#define PutMemW(Seg,Off,x)      write_mem_word(GetMemAddr(cpustate,Seg,Off,2,I80286_WRITE), (x))

//#define PUSH(val)               { if(PM) i80286_check_permission(cpustate, SS, cpustate->regs.w[SP]-2, I80286_WORD, I80286_WRITE); cpustate->regs.w[SP] -= 2; WriteWord(((cpustate->base[SS] + cpustate->regs.w[SP]) & AMASK), val); }
//#define POP(var)                { if(PM) i80286_check_permission(cpustate, SS, cpustate->regs.w[SP], I80286_WORD, I80286_READ); cpustate->regs.w[SP] += 2; var = ReadWord(((cpustate->base[SS] + ((cpustate->regs.w[SP]-2) & 0xffff)) & AMASK)); }

//#define IOPL ((cpustate->flags&0x3000)>>12)
//#define NT ((cpustate->flags&0x4000)>>14)
//#define xF (0)
//#endif
//#include "./instr86.h"
#include "./table86.h"
//#include "./instr186.h"
#include "./table186.h"
#include "./instr286.h"
#include "./table286.h"
//#include "./instrv30.h"
#include "./tablev30.h"

#include "ix86_real.h"

void X86_OPS::CPU_EXECUTE( i8086 )
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
#ifdef USE_DEBUGGER
		bool now_debugging = cpustate->debugger->now_debugging;
		if(now_debugging) {
			cpustate->debugger->check_break_points(cpustate->pc);
			if(cpustate->debugger->now_suspended) {
				cpustate->emu->mute_sound();
				while(cpustate->debugger->now_debugging && cpustate->debugger->now_suspended) {
					cpustate->emu->sleep(10);
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

	/* if busreq is raised, spin cpu while remained clock */
	if (cpustate->icount > 0 && cpustate->busreq) {
		cpustate->icount = 0;
	}
	return base_icount - cpustate->icount;
}

void X86_OPS::CPU_EXECUTE( i80186 )
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
#ifdef USE_DEBUGGER
		bool now_debugging = cpustate->debugger->now_debugging;
		if(now_debugging) {
			cpustate->debugger->check_break_points(cpustate->pc);
			if(cpustate->debugger->now_suspended) {
				cpustate->emu->mute_sound();
				while(cpustate->debugger->now_debugging && cpustate->debugger->now_suspended) {
					cpustate->emu->sleep(10);
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

	/* if busreq is raised, spin cpu while remained clock */
	if (cpustate->icount > 0 && cpustate->busreq) {
		cpustate->icount = 0;
	}
	return base_icount - cpustate->icount;
}

void X86_OPS::CPU_EXECUTE( i80286 )
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
					cpustate->emu->sleep(10);
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

	/* if busreq is raised, spin cpu while remained clock */
	if (cpustate->icount > 0 && cpustate->busreq) {
		cpustate->icount = 0;
	}
	return base_icount - cpustate->icount;
}

void X86_OPS::CPU_EXECUTE( v30 )
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
#ifdef USE_DEBUGGER
		bool now_debugging = cpustate->debugger->now_debugging;
		if(now_debugging) {
			cpustate->debugger->check_break_points(cpustate->pc);
			if(cpustate->debugger->now_suspended) {
				cpustate->emu->mute_sound();
				while(cpustate->debugger->now_debugging && cpustate->debugger->now_suspended) {
					cpustate->emu->sleep(10);
				}
			}
			if(cpustate->debugger->now_debugging) {
				cpustate->program = cpustate->io = cpustate->debugger;
			} else {
				now_debugging = false;
			}
			cpustate->seg_prefix = FALSE;
			cpustate->prevpc = cpustate->pc;
			TABLEV30;
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
			TABLEV30;
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

	/* if busreq is raised, spin cpu while remained clock */
	if (cpustate->icount > 0 && cpustate->busreq) {
		cpustate->icount = 0;
	}
	return base_icount - cpustate->icount;
}

void X86_OPS::set_irq_line_v30(int irqline, int state)
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

