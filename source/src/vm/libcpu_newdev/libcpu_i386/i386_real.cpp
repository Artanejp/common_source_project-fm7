
#include "../vm.h"
#include "../debugger.h"
#include "../../emu.h"
#include "./i386_real.h"
#define FAULT(fault,error) {cpustate->ext = 1; i386_trap_with_error(fault,0,0,error); return;}
#define FAULT_EXP(fault,error) {cpustate->ext = 1; i386_trap_with_error(fault,0,trap_level+1,error); return;}
extern "C" {
	#include "../libcpu_softfloat/softfloat.h"
};

int I386_OPS::cpu_translate_i386(void *cpudevice, address_spacenum space, int intention, offs_t *address)
{
	i386_state *cpu_state = (i386_state *)cpudevice;
	int ret = TRUE;
	if(space == AS_PROGRAM)
		ret = i386_translate_address(intention, address, NULL);
	*address &= cpu_state->a20_mask;
	return ret;
}

i386_state *I386_OPS::i386_common_init(int tlbsize)
{
	int i, j;
	static const int regs8[8] = {AL,CL,DL,BL,AH,CH,DH,BH};
	static const int regs16[8] = {AX,CX,DX,BX,SP,BP,SI,DI};
	static const int regs32[8] = {EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI};
	cpustate = &__cpustate;
	//cpustate = (i386_state *)malloc(sizeof(i386_state));
	//x86_cycle_table = _x86_cycle_table_real;

	assert((sizeof(XMM_REG)/sizeof(double)) == 2);

	build_cycle_table();

	for( i=0; i < 256; i++ ) {
		int c=0;
		for( j=0; j < 8; j++ ) {
			if( i & (1 << j) )
				c++;
		}
		i386_parity_table[i] = ~(c & 0x1) & 0x1;
	}

	for( i=0; i < 256; i++ ) {
		i386_MODRM_table[i].reg.b = regs8[(i >> 3) & 0x7];
		i386_MODRM_table[i].reg.w = regs16[(i >> 3) & 0x7];
		i386_MODRM_table[i].reg.d = regs32[(i >> 3) & 0x7];

		i386_MODRM_table[i].rm.b = regs8[i & 0x7];
		i386_MODRM_table[i].rm.w = regs16[i & 0x7];
		i386_MODRM_table[i].rm.d = regs32[i & 0x7];
	}

	cpustate->vtlb = vtlb_alloc((void *)cpustate, AS_PROGRAM, 0, tlbsize);
	cpustate->smi = false;
	cpustate->lock = false;

//	i386_interface *intf = (i386_interface *) device->static_config();
//
//	if (intf != NULL)
//		cpustate->smiact.resolve(intf->smiact, *device);
//	else
//		memset(&cpustate->smiact, 0, sizeof(cpustate->smiact));

	zero_state();
	cpustate->program = d_mem;
	cpustate->io = d_io;
	cpustate->pic = d_pic;
#ifdef I386_PSEUDO_BIOS
	cpustate->bios = d_bios;
#endif
#ifdef SINGLE_MODE_DMA
	cpustate->dma = d_dma;
#endif
	return cpustate;
}

void I386_OPS::i386_decode_opcode()
{
	cpustate->opcode = FETCH();

	if(cpustate->lock && !cpustate->lock_table[0][cpustate->opcode])
		return I386OP(invalid)();

	if( cpustate->operand_size )
		(this->*cpustate->opcode_table1_32[cpustate->opcode])();
	else
		(this->*cpustate->opcode_table1_16[cpustate->opcode])();
}


int I386_OPS::cpu_execute_i386(int cycles)
{
	CHANGE_PC(cpustate->eip);

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
				cpustate->debugger->now_waiting = true;
				while(cpustate->debugger->now_debugging && cpustate->debugger->now_suspended) {
					cpustate->emu->sleep(10);
				}
				cpustate->debugger->now_waiting = false;
			}
			if(cpustate->debugger->now_debugging) {
				cpustate->program = cpustate->io = cpustate->debugger;
			} else {
				now_debugging = false;
			}
			i386_check_irq_line();
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
				i386_decode_opcode();
				if(cpustate->TF && old_tf)
				{
					cpustate->prev_eip = cpustate->eip;
					cpustate->ext = 1;
					i386_trap(1,0,0);
				}
				if(cpustate->lock && (cpustate->opcode != 0xf0))
					cpustate->lock = false;
			}
			catch(UINT64 e)
			{
				cpustate->ext = 1;
				i386_trap_with_error(e&0xffffffff,0,0,e>>32);
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
			i386_check_irq_line();

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
				i386_decode_opcode();
				if(cpustate->TF && old_tf)
				{
					cpustate->prev_eip = cpustate->eip;
					cpustate->ext = 1;
					i386_trap(1,0,0);
				}
				if(cpustate->lock && (cpustate->opcode != 0xf0))
					cpustate->lock = false;
			}
			catch(UINT64 e)
			{
				cpustate->ext = 1;
				i386_trap_with_error(e&0xffffffff,0,0,e>>32);
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

void I386_OPS::i386_trap(int irq, int irq_gate, int trap_level)
{
	/*  I386 Interrupts/Traps/Faults:
	 *
	 *  0x00    Divide by zero
	 *  0x01    Debug exception
	 *  0x02    NMI
	 *  0x03    Int3
	 *  0x04    Overflow
	 *  0x05    Array bounds check
	 *  0x06    Illegal Opcode
	 *  0x07    FPU not available
	 *  0x08    Double fault
	 *  0x09    Coprocessor segment overrun
	 *  0x0a    Invalid task state
	 *  0x0b    Segment not present
	 *  0x0c    Stack exception
	 *  0x0d    General Protection Fault
	 *  0x0e    Page fault
	 *  0x0f    Reserved
	 *  0x10    Coprocessor error
	 */
	UINT32 v1, v2;
	UINT32 offset, oldflags = get_flags();
	UINT16 segment;
	int entry = irq * (PROTECTED_MODE ? 8 : 4);
	int SetRPL = 0;
	cpustate->lock = false;

	if( !(PROTECTED_MODE) )
	{
		/* 16-bit */
		PUSH16(oldflags & 0xffff );
		PUSH16(cpustate->sreg[CS].selector );
		if(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1)
			PUSH16(cpustate->eip );
		else
			PUSH16(cpustate->prev_eip );

		cpustate->sreg[CS].selector = READ16(cpustate->idtr.base + entry + 2 );
		cpustate->eip = READ16(cpustate->idtr.base + entry );

		cpustate->TF = 0;
		cpustate->IF = 0;
	}
	else
	{
		int type;
		UINT16 flags;
		I386_SREG desc;
		UINT8 CPL = cpustate->CPL, DPL = 0; //, RPL = 0;

		/* 32-bit */
		v1 = READ32PL0(cpustate->idtr.base + entry );
		v2 = READ32PL0(cpustate->idtr.base + entry + 4 );
		offset = (v2 & 0xffff0000) | (v1 & 0xffff);
		segment = (v1 >> 16) & 0xffff;
		type = (v2>>8) & 0x1F;
		flags = (v2>>8) & 0xf0ff;

		if(trap_level == 2)
		{
			logerror("IRQ: Double fault.\n");
			FAULT_EXP(FAULT_DF,0);
		}
		if(trap_level >= 3)
		{
			logerror("IRQ: Triple fault. CPU reset.\n");
#if defined(HAS_I386)
			cpu_reset_i386();
#elif defined(HAS_I486)
			cpu_reset_i486();
#elif defined(HAS_PENTIUM)
			cpu_reset_pentium();
#elif defined(HAS_MEDIAGX)
			cpu_reset_mediagx();
#elif defined(HAS_PENTIUM_PRO)
			cpu_reset_pentium_pro();
#elif defined(HAS_PENTIUM_MMX)
			cpu_reset_pentium_mmx();
#elif defined(HAS_PENTIUM2)
			cpu_reset_pentium2();
#elif defined(HAS_PENTIUM3)
			cpu_reset_pentium3();
#elif defined(HAS_PENTIUM4)
			cpu_reset_pentium4();
#endif

			cpustate->shutdown = 1;
			return;
		}

		/* segment privilege checks */
		if(entry >= cpustate->idtr.limit)
		{
			logerror("IRQ (%08x): Vector %02xh is past IDT limit.\n",cpustate->pc,entry);
			FAULT_EXP(FAULT_GP,entry+2)
		}
		/* segment must be interrupt gate, trap gate, or task gate */
		if(type != 0x05 && type != 0x06 && type != 0x07 && type != 0x0e && type != 0x0f)
		{
			logerror("IRQ#%02x (%08x): Vector segment %04x is not an interrupt, trap or task gate.\n",irq,cpustate->pc,segment);
			FAULT_EXP(FAULT_GP,entry+2)
		}

		if(cpustate->ext == 0) // if software interrupt (caused by INT/INTO/INT3)
		{
			if(((flags >> 5) & 0x03) < CPL)
			{
				logerror("IRQ (%08x): Software IRQ - gate DPL is less than CPL.\n",cpustate->pc);
				FAULT_EXP(FAULT_GP,entry+2)
			}
			if(V8086_MODE)
			{
				if((!cpustate->IOP1 || !cpustate->IOP2) && (cpustate->opcode != 0xcc))
				{
					logerror("IRQ (%08x): Is in Virtual 8086 mode and IOPL != 3.\n",cpustate->pc);
					FAULT(FAULT_GP,0)
				}

			}
		}

		if((flags & 0x0080) == 0)
		{
			logerror("IRQ: Vector segment is not present.\n");
			FAULT_EXP(FAULT_NP,entry+2)
		}

		if(type == 0x05)
		{
			/* Task gate */
			memset(&desc, 0, sizeof(desc));
			desc.selector = segment;
			i386_load_protected_mode_segment(&desc,NULL);
			if(segment & 0x04)
			{
				logerror("IRQ: Task gate: TSS is not in the GDT.\n");
				FAULT_EXP(FAULT_TS,segment & ~0x03);
			}
			else
			{
				if(segment > cpustate->gdtr.limit)
				{
					logerror("IRQ: Task gate: TSS is past GDT limit.\n");
					FAULT_EXP(FAULT_TS,segment & ~0x03);
				}
			}
			if((desc.flags & 0x000f) != 0x09 && (desc.flags & 0x000f) != 0x01)
			{
				logerror("IRQ: Task gate: TSS is not an available TSS.\n");
				FAULT_EXP(FAULT_TS,segment & ~0x03);
			}
			if((desc.flags & 0x0080) == 0)
			{
				logerror("IRQ: Task gate: TSS is not present.\n");
				FAULT_EXP(FAULT_NP,segment & ~0x03);
			}
			if(!(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1))
				cpustate->eip = cpustate->prev_eip;
			if(desc.flags & 0x08)
				i386_task_switch(desc.selector,1);
			else
				i286_task_switch(desc.selector,1);
			return;
		}
		else
		{
			/* Interrupt or Trap gate */
			memset(&desc, 0, sizeof(desc));
			desc.selector = segment;
			i386_load_protected_mode_segment(&desc,NULL);
			CPL = cpustate->CPL;  // current privilege level
			DPL = (desc.flags >> 5) & 0x03;  // descriptor privilege level
//          RPL = segment & 0x03;  // requested privilege level

			if((segment & ~0x03) == 0)
			{
				logerror("IRQ: Gate segment is null.\n");
				FAULT_EXP(FAULT_GP,cpustate->ext)
			}
			if(segment & 0x04)
			{
				if((segment & ~0x07) > cpustate->ldtr.limit)
				{
					logerror("IRQ: Gate segment is past LDT limit.\n");
					FAULT_EXP(FAULT_GP,(segment & 0x03)+cpustate->ext)
				}
			}
			else
			{
				if((segment & ~0x07) > cpustate->gdtr.limit)
				{
					logerror("IRQ: Gate segment is past GDT limit.\n");
					FAULT_EXP(FAULT_GP,(segment & 0x03)+cpustate->ext)
				}
			}
			if((desc.flags & 0x0018) != 0x18)
			{
				logerror("IRQ: Gate descriptor is not a code segment.\n");
				FAULT_EXP(FAULT_GP,(segment & 0x03)+cpustate->ext)
			}
			if((desc.flags & 0x0080) == 0)
			{
				logerror("IRQ: Gate segment is not present.\n");
				FAULT_EXP(FAULT_NP,(segment & 0x03)+cpustate->ext)
			}
			if((desc.flags & 0x0004) == 0 && (DPL < CPL))
			{
				/* IRQ to inner privilege */
				I386_SREG stack;
				UINT32 newESP,oldSS,oldESP;

				if(V8086_MODE && DPL)
				{
					logerror("IRQ: Gate to CPL>0 from VM86 mode.\n");
					FAULT_EXP(FAULT_GP,segment & ~0x03);
				}
				/* Check new stack segment in TSS */
				memset(&stack, 0, sizeof(stack));
				stack.selector = i386_get_stack_segment(DPL);
				i386_load_protected_mode_segment(&stack,NULL);
				oldSS = cpustate->sreg[SS].selector;
				if(flags & 0x0008)
					oldESP = REG32(ESP);
				else
					oldESP = REG16(SP);
				if((stack.selector & ~0x03) == 0)
				{
					logerror("IRQ: New stack selector is null.\n");
					FAULT_EXP(FAULT_GP,cpustate->ext)
				}
				if(stack.selector & 0x04)
				{
					if((stack.selector & ~0x07) > cpustate->ldtr.base)
					{
						logerror("IRQ: New stack selector is past LDT limit.\n");
						FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+cpustate->ext)
					}
				}
				else
				{
					if((stack.selector & ~0x07) > cpustate->gdtr.base)
					{
						logerror("IRQ: New stack selector is past GDT limit.\n");
						FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+cpustate->ext)
					}
				}
				if((stack.selector & 0x03) != DPL)
				{
					logerror("IRQ: New stack selector RPL is not equal to code segment DPL.\n");
					FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+cpustate->ext)
				}
				if(((stack.flags >> 5) & 0x03) != DPL)
				{
					logerror("IRQ: New stack segment DPL is not equal to code segment DPL.\n");
					FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+cpustate->ext)
				}
				if(((stack.flags & 0x0018) != 0x10) && (stack.flags & 0x0002) != 0)
				{
					logerror("IRQ: New stack segment is not a writable data segment.\n");
					FAULT_EXP(FAULT_TS,(stack.selector & ~0x03)+cpustate->ext) // #TS(stack selector + EXT)
				}
				if((stack.flags & 0x0080) == 0)
				{
					logerror("IRQ: New stack segment is not present.\n");
					FAULT_EXP(FAULT_SS,(stack.selector & ~0x03)+cpustate->ext) // #TS(stack selector + EXT)
				}
				newESP = i386_get_stack_ptr(DPL);
				if(type & 0x08) // 32-bit gate
				{
					if(((newESP < (V8086_MODE?36:20)) && !(stack.flags & 0x4)) || ((~stack.limit < (~(newESP - 1) + (V8086_MODE?36:20))) && (stack.flags & 0x4)))
					{
						logerror("IRQ: New stack has no space for return addresses.\n");
						FAULT_EXP(FAULT_SS,0)
					}
				}
				else // 16-bit gate
				{
					newESP &= 0xffff;
					if(((newESP < (V8086_MODE?18:10)) && !(stack.flags & 0x4)) || ((~stack.limit < (~(newESP - 1) + (V8086_MODE?18:10))) && (stack.flags & 0x4)))
					{
						logerror("IRQ: New stack has no space for return addresses.\n");
						FAULT_EXP(FAULT_SS,0)
					}
				}
				if(offset > desc.limit)
				{
					logerror("IRQ: New EIP is past code segment limit.\n");
					FAULT_EXP(FAULT_GP,0)
				}
				/* change CPL before accessing the stack */
				cpustate->CPL = DPL;
				/* check for page fault at new stack TODO: check if stack frame crosses page boundary */
				WRITE_TEST(stack.base+newESP-1);
				/* Load new stack segment descriptor */
				cpustate->sreg[SS].selector = stack.selector;
				i386_load_protected_mode_segment(&cpustate->sreg[SS],NULL);
				i386_set_descriptor_accessed(stack.selector);
				REG32(ESP) = newESP;
				if(V8086_MODE)
				{
					//logerror("IRQ (%08x): Interrupt during V8086 task\n",cpustate->pc);
					if(type & 0x08)
					{
						PUSH32(cpustate->sreg[GS].selector & 0xffff);
						PUSH32(cpustate->sreg[FS].selector & 0xffff);
						PUSH32(cpustate->sreg[DS].selector & 0xffff);
						PUSH32(cpustate->sreg[ES].selector & 0xffff);
					}
					else
					{
						PUSH16(cpustate->sreg[GS].selector);
						PUSH16(cpustate->sreg[FS].selector);
						PUSH16(cpustate->sreg[DS].selector);
						PUSH16(cpustate->sreg[ES].selector);
					}
					cpustate->sreg[GS].selector = 0;
					cpustate->sreg[FS].selector = 0;
					cpustate->sreg[DS].selector = 0;
					cpustate->sreg[ES].selector = 0;
					cpustate->VM = 0;
					i386_load_segment_descriptor(GS);
					i386_load_segment_descriptor(FS);
					i386_load_segment_descriptor(DS);
					i386_load_segment_descriptor(ES);
				}
				if(type & 0x08)
				{
					// 32-bit gate
					PUSH32(oldSS);
					PUSH32(oldESP);
				}
				else
				{
					// 16-bit gate
					PUSH16(oldSS);
					PUSH16(oldESP);
				}
				SetRPL = 1;
			}
			else
			{
				int stack_limit;
				if((desc.flags & 0x0004) || (DPL == CPL))
				{
					/* IRQ to same privilege */
					if(V8086_MODE && !cpustate->ext)
					{
						logerror("IRQ: Gate to same privilege from VM86 mode.\n");
						FAULT_EXP(FAULT_GP,segment & ~0x03);
					}
					if(type == 0x0e || type == 0x0f)  // 32-bit gate
						stack_limit = 10;
					else
						stack_limit = 6;
					// TODO: Add check for error code (2 extra bytes)
					if(REG32(ESP) < stack_limit)
					{
						logerror("IRQ: Stack has no space left (needs %i bytes).\n",stack_limit);
						FAULT_EXP(FAULT_SS,0)
					}
					if(offset > desc.limit)
					{
						logerror("IRQ: Gate segment offset is past segment limit.\n");
						FAULT_EXP(FAULT_GP,0)
					}
					SetRPL = 1;
				}
				else
				{
					logerror("IRQ: Gate descriptor is non-conforming, and DPL does not equal CPL.\n");
					FAULT_EXP(FAULT_GP,segment)
				}
			}
		}
		UINT32 tempSP = REG32(ESP);
		try
		{
			// this is ugly but the alternative is worse
			if(type != 0x0e && type != 0x0f)  // if not 386 interrupt or trap gate
			{
				PUSH16(oldflags & 0xffff );
				PUSH16(cpustate->sreg[CS].selector );
				if(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1)
					PUSH16(cpustate->eip );
				else
					PUSH16(cpustate->prev_eip );
			}
			else
			{
				PUSH32(oldflags & 0x00ffffff );
				PUSH32(cpustate->sreg[CS].selector );
				if(irq == 3 || irq == 4 || irq == 9 || irq_gate == 1)
					PUSH32(cpustate->eip );
				else
					PUSH32(cpustate->prev_eip );
			}
		}
		catch(UINT64 e)
		{
			REG32(ESP) = tempSP;
			throw e;
		}
		if(SetRPL != 0)
			segment = (segment & ~0x03) | cpustate->CPL;
		cpustate->sreg[CS].selector = segment;
		cpustate->eip = offset;

		if(type == 0x0e || type == 0x06)
			cpustate->IF = 0;
		cpustate->TF = 0;
		cpustate->NT = 0;
	}

	i386_load_segment_descriptor(CS);
	CHANGE_PC(cpustate->eip);

}

void I386_OPS::i386_trap_with_error(int irq, int irq_gate, int trap_level, UINT32 error)
{
	i386_trap(irq,irq_gate,trap_level);
	if(irq == 8 || irq == 10 || irq == 11 || irq == 12 || irq == 13 || irq == 14)
	{
		// for these exceptions, an error code is pushed onto the stack by the processor.
		// no error code is pushed for software interrupts, either.
		if(PROTECTED_MODE)
		{
			UINT32 entry = irq * 8;
			UINT32 v2,type;
			v2 = READ32PL0(cpustate->idtr.base + entry + 4 );
			type = (v2>>8) & 0x1F;
			if(type == 5)
			{
				v2 = READ32PL0(cpustate->idtr.base + entry);
				v2 = READ32PL0(cpustate->gdtr.base + ((v2 >> 16) & 0xfff8) + 4);
				type = (v2>>8) & 0x1F;
			}
			if(type >= 9)
				PUSH32(error);
			else
				PUSH16(error);
		}
		else
			PUSH16(error);
	}
}

bool I386_OPS::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
#if defined(USE_DEBUGGER)
	if(_tcsicmp(reg, _T("IP")) == 0) {
		cpustate->eip = data & 0xffff;
		CHANGE_PC(cpustate->eip);
	} else if(_tcsicmp(reg, _T("AX")) == 0) {
		REG16(AX) = data;
	} else if(_tcsicmp(reg, _T("BX")) == 0) {
		REG16(BX) = data;
	} else if(_tcsicmp(reg, _T("CX")) == 0) {
		REG16(CX) = data;
	} else if(_tcsicmp(reg, _T("DX")) == 0) {
		REG16(DX) = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		REG16(SP) = data;
	} else if(_tcsicmp(reg, _T("BP")) == 0) {
		REG16(BP) = data;
	} else if(_tcsicmp(reg, _T("SI")) == 0) {
		REG16(SI) = data;
	} else if(_tcsicmp(reg, _T("DI")) == 0) {
		REG16(DI) = data;
	} else if(_tcsicmp(reg, _T("AL")) == 0) {
		REG8(AL) = data;
	} else if(_tcsicmp(reg, _T("AH")) == 0) {
		REG8(AH) = data;
	} else if(_tcsicmp(reg, _T("BL")) == 0) {
		REG8(BL) = data;
	} else if(_tcsicmp(reg, _T("BH")) == 0) {
		REG8(BH) = data;
	} else if(_tcsicmp(reg, _T("CL")) == 0) {
		REG8(CL) = data;
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		REG8(CH) = data;
	} else if(_tcsicmp(reg, _T("DL")) == 0) {
		REG8(DL) = data;
	} else if(_tcsicmp(reg, _T("DH")) == 0) {
		REG8(DH) = data;
	} else {
		return false;
	}
#endif
	return true;
}

void I386_OPS::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
#if defined(USE_DEBUGGER)
	my_stprintf_s(buffer, buffer_len,
				  _T("AX=%04X  BX=%04X CX=%04X DX=%04X SP=%04X  BP=%04X  SI=%04X  DI=%04X\nDS=%04X  ES=%04X SS=%04X CS=%04X IP=%04X  FLAG=[%c%c%c%c
%c%c%c%c%c]\nTotal CPU Clocks = %llu (%llu)"),
				  cpustate->sreg[DS].selector, cpustate->sreg[ES].selector, cpustate->sreg[SS].selector, cpustate->sreg[CS].selector, cpustate->eip,
				  cpustate->OF ? _T('O') : _T('-'), cpustate->DF ? _T('D') : _T('-'), cpustate->IF ? _T('I') : _T('-'), cpustate->TF ? _T('T') : _T('-'),
				  cpustate->SF ? _T('S') : _T('-'), cpustate->ZF ? _T('Z') : _T('-'), cpustate->AF ? _T('A') : _T('-'), cpustate->PF ? _T('P') : _T('-'),
				  cpustate->CF ? _T('C') : _T('-'),
				  cpustate->total_cycles, cpustate->total_cycles - cpustate->prev_total_cycles);
	cpustate->prev_total_cycles = cpustate->total_cycles;
#endif
}

int I386_OPS::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
#if defined(USE_DEBUGGER)
	UINT64 eip = cpustate->eip;
	UINT8 ops[16];
	for(int i = 0; i < 16; i++) {
		int wait;
		ops[i] = d_mem->read_data8w(pc + i, &wait);
	}
	UINT8 *oprom = ops;
	
	if(cpustate->operand_size) {
		return CPU_DISASSEMBLE_CALL(x86_32) & DASMFLAG_LENGTHMASK;
	} else {
		return CPU_DISASSEMBLE_CALL(x86_16) & DASMFLAG_LENGTHMASK;
	}
#else
	return 0;
#endif
}
