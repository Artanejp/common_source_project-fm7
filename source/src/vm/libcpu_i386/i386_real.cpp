
#include "../vm.h"
#include "../../emu.h"
#include "./i386opdef_real.h"

I386_OPS::I386_OPS(void) : I386_OPS_BASE()
{
}

I386_OPS::~I386_OPS_BASE()
{
}

int I386_OPS::cpu_translate_i386(void *cpudevice, address_spacenum space, int intention, offs_t *address)
{
	i386_state *cpu_state = (i386_state *)cpudevice;
	int ret = TRUE;
	if(space == AS_PROGRAM)
		ret = i386_translate_address(cpu_state, intention, address, NULL);
	*address &= cpu_state->a20_mask;
	return ret;
}

int I386_OPS::cpu_execute_i386(int cycles)
{
	CHANGE_PC(cpustate,cpustate->eip);

	if (cpustate->halted || cpustate->busreq)
	{
#ifdef SINGLE_MODE_DMA
		if(cpustate->dma != NULL) {
			cpustate->dma->do_dma();
		}
#endif
		if (cycles == -1) {
			int passed_cycles = max(1, cpustate->extra_cycles);
			// this is main cpu, cpustate->cycles is not used
			/*cpustate->cycles = */cpustate->extra_cycles = 0;
			cpustate->tsc += passed_cycles;
			return passed_cycles;
		} else {
			cpustate->cycles += cycles;
			cpustate->base_cycles = cpustate->cycles;

			/* adjust for any interrupts that came in */
			cpustate->cycles -= cpustate->extra_cycles;
			cpustate->extra_cycles = 0;

			/* if busreq is raised, spin cpu while remained clock */
			if (cpustate->cycles > 0) {
				cpustate->cycles = 0;
			}
			int passed_cycles = cpustate->base_cycles - cpustate->cycles;
			cpustate->tsc += passed_cycles;
			return passed_cycles;
		}
	}

	if (cycles == -1) {
		cpustate->cycles = 1;
	} else {
		cpustate->cycles += cycles;
	}
	cpustate->base_cycles = cpustate->cycles;

	/* adjust for any interrupts that came in */
	cpustate->cycles -= cpustate->extra_cycles;
	cpustate->extra_cycles = 0;

	while( cpustate->cycles > 0 && !cpustate->busreq )
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
			i386_check_irq_line(cpustate);
			cpustate->operand_size = cpustate->sreg[CS].d;
			cpustate->xmm_operand_size = 0;
			cpustate->address_size = cpustate->sreg[CS].d;
			cpustate->operand_prefix = 0;
			cpustate->address_prefix = 0;

			cpustate->ext = 1;
			int old_tf = cpustate->TF;

			cpustate->segment_prefix = 0;
			cpustate->prev_eip = cpustate->eip;
			cpustate->prev_pc = cpustate->pc;

			if(cpustate->delayed_interrupt_enable != 0)
			{
				cpustate->IF = 1;
				cpustate->delayed_interrupt_enable = 0;
			}
#ifdef DEBUG_MISSING_OPCODE
			cpustate->opcode_bytes_length = 0;
			cpustate->opcode_pc = cpustate->pc;
#endif
			try
			{
				I386OP(decode_opcode)(cpustate);
				if(cpustate->TF && old_tf)
				{
					cpustate->prev_eip = cpustate->eip;
					cpustate->ext = 1;
					i386_trap(cpustate,1,0,0);
				}
				if(cpustate->lock && (cpustate->opcode != 0xf0))
					cpustate->lock = false;
			}
			catch(UINT64 e)
			{
				cpustate->ext = 1;
				i386_trap_with_error(cpustate,e&0xffffffff,0,0,e>>32);
			}
#ifdef SINGLE_MODE_DMA
			if(cpustate->dma != NULL) {
				cpustate->dma->do_dma();
			}
#endif
			/* adjust for any interrupts that came in */
			cpustate->cycles -= cpustate->extra_cycles;
			cpustate->extra_cycles = 0;
			
			if(now_debugging) {
				if(!cpustate->debugger->now_going) {
					cpustate->debugger->now_suspended = true;
				}
				cpustate->program = cpustate->program_stored;
				cpustate->io = cpustate->io_stored;
			}
		} else {
#endif
			i386_check_irq_line(cpustate);
			cpustate->operand_size = cpustate->sreg[CS].d;
			cpustate->xmm_operand_size = 0;
			cpustate->address_size = cpustate->sreg[CS].d;
			cpustate->operand_prefix = 0;
			cpustate->address_prefix = 0;

			cpustate->ext = 1;
			int old_tf = cpustate->TF;

			cpustate->segment_prefix = 0;
			cpustate->prev_eip = cpustate->eip;
			cpustate->prev_pc = cpustate->pc;

			if(cpustate->delayed_interrupt_enable != 0)
			{
				cpustate->IF = 1;
				cpustate->delayed_interrupt_enable = 0;
			}
#ifdef DEBUG_MISSING_OPCODE
			cpustate->opcode_bytes_length = 0;
			cpustate->opcode_pc = cpustate->pc;
#endif
			try
			{
				I386OP(decode_opcode)(cpustate);
				if(cpustate->TF && old_tf)
				{
					cpustate->prev_eip = cpustate->eip;
					cpustate->ext = 1;
					i386_trap(cpustate,1,0,0);
				}
				if(cpustate->lock && (cpustate->opcode != 0xf0))
					cpustate->lock = false;
			}
			catch(UINT64 e)
			{
				cpustate->ext = 1;
				i386_trap_with_error(cpustate,e&0xffffffff,0,0,e>>32);
			}
#ifdef SINGLE_MODE_DMA
			if(cpustate->dma != NULL) {
				cpustate->dma->do_dma();
			}
#endif
			/* adjust for any interrupts that came in */
			cpustate->cycles -= cpustate->extra_cycles;
			cpustate->extra_cycles = 0;
#ifdef USE_DEBUGGER
		}
#endif
	}

	/* if busreq is raised, spin cpu while remained clock */
	if (cpustate->cycles > 0 && cpustate->busreq) {
		cpustate->cycles = 0;
	}
	int passed_cycles = cpustate->base_cycles - cpustate->cycles;
	cpustate->tsc += passed_cycles;
	return passed_cycles;
}


void I386_OPS::I386OP(int)(i386_state *cpustate)               // Opcode 0xcd
{
	int interrupt = FETCH(cpustate);
	CYCLES(cpustate,CYCLES_INT);
#ifdef I386_PSEUDO_BIOS
	BIOS_INT(interrupt)
#endif
	cpustate->ext = 0; // not an external interrupt
	i386_trap(cpustate,interrupt, 1, 0);
	cpustate->ext = 1;
}
