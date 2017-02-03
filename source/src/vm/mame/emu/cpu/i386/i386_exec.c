#include "i386priv.h"

extern UINT32 i386_load_protected_mode_segment(i386_state *cpustate, I386_SREG *seg, UINT64 *desc );
extern void i386_load_call_gate(i386_state* cpustate, I386_CALL_GATE *gate);
extern void i386_set_descriptor_accessed(i386_state *cpustate, UINT16 selector);
extern void i386_load_segment_descriptor(i386_state *cpustate, int segment );
extern UINT32 i386_get_stack_segment(i386_state* cpustate, UINT8 privilege);
extern UINT32 i386_get_stack_ptr(i386_state* cpustate, UINT8 privilege);
extern void i386_build_cycle_table();

extern void i386_build_opcode_table(i386_state *cpustate, UINT32 features);
static void zero_state(i386_state *cpustate);

extern int x87_inc_stack(i386_state *cpustate);
extern int x87_dec_stack(i386_state *cpustate);
extern int x87_check_exceptions(i386_state *cpustate);
extern void x87_reset(i386_state *cpustate);
extern void build_x87_opcode_table(i386_state *cpustate);

extern void i386_op_decode_opcode_main(i386_state *cpustate);
extern void i386_op_decode_two_byte_main(i386_state *cpustate);
extern void i386_op_decode_three_byte38_main(i386_state *cpustate);
extern void i386_op_decode_three_byte3a_main(i386_state *cpustate);
extern void i386_op_decode_three_byte66_main(i386_state *cpustate);
extern void i386_op_decode_three_bytef2_main(i386_state *cpustate);
extern void i386_op_decode_three_bytef3_main(i386_state *cpustate);
extern void i386_op_decode_four_byte3866_main(i386_state *cpustate);
extern void i386_op_decode_four_byte38f2_main(i386_state *cpustate);
extern void i386_op_decode_four_byte38f3_main(i386_state *cpustate);
extern void i386_op_decode_four_byte3a66_main(i386_state *cpustate);
extern void i386_op_decode_four_byte3af2_main(i386_state *cpustate);

static void I386OP(decode_opcode)(i386_state *cpustate);
static void I386OP(decode_two_byte)(i386_state *cpustate);
static void I386OP(decode_three_byte38)(i386_state *cpustate);
static void I386OP(decode_three_byte3a)(i386_state *cpustate);
static void I386OP(decode_three_byte66)(i386_state *cpustate);
static void I386OP(decode_three_bytef2)(i386_state *cpustate);
static void I386OP(decode_three_bytef3)(i386_state *cpustate);
static void I386OP(decode_four_byte3866)(i386_state *cpustate);
static void I386OP(decode_four_byte3a66)(i386_state *cpustate);
static void I386OP(decode_four_byte38f2)(i386_state *cpustate);
static void I386OP(decode_four_byte3af2)(i386_state *cpustate);
static void I386OP(decode_four_byte38f3)(i386_state *cpustate);

extern void i386_i386_cpu_init_main(i386_state *cpustate);
extern void i386_i486_cpu_init_main(i386_state *cpustate);
extern void i386_pentium_cpu_init_main(i386_state *cpustate);
extern void i386_pentium_pro_cpu_init_main(i386_state *cpustate);
extern void i386_mediagx_cpu_init_main(i386_state *cpustate);
extern void i386_pentium_mmx_cpu_init_main(i386_state *cpustate);
extern void i386_pentium2_cpu_init_main(i386_state *cpustate);
extern void i386_pentium3_cpu_init_main(i386_state *cpustate);
extern void i386_pentium4_cpu_init_main(i386_state *cpustate);


static CPU_RESET( i386 );
static CPU_RESET( i486 );
static CPU_RESET( pentium );
static CPU_RESET( mediagx );
static CPU_RESET( pentium_pro );
static CPU_RESET( pentium_mmx );
static CPU_RESET( pentium2 );
static CPU_RESET( pentium3 );
static CPU_RESET( pentium4 );

extern void i386_pentium_smi_main(i386_state *cpustate);
static void pentium_smi(i386_state *state);
/*************************************************************************/

#include "i386ops.h"

extern void i386_trap(i386_state *cpustate,int irq, int irq_gate, int trap_level);
extern void i386_trap_with_error(i386_state *cpustate,int irq, int irq_gate, int trap_level, UINT32 error);

static UINT32 get_flags(i386_state *cpustate)
{
	UINT32 f = 0x2;
	f |= cpustate->CF;
	f |= cpustate->PF << 2;
	f |= cpustate->AF << 4;
	f |= cpustate->ZF << 6;
	f |= cpustate->SF << 7;
	f |= cpustate->TF << 8;
	f |= cpustate->IF << 9;
	f |= cpustate->DF << 10;
	f |= cpustate->OF << 11;
	f |= cpustate->IOP1 << 12;
	f |= cpustate->IOP2 << 13;
	f |= cpustate->NT << 14;
	f |= cpustate->RF << 16;
	f |= cpustate->VM << 17;
	f |= cpustate->AC << 18;
	f |= cpustate->VIF << 19;
	f |= cpustate->VIP << 20;
	f |= cpustate->ID << 21;
	return (cpustate->eflags & ~cpustate->eflags_mask) | (f & cpustate->eflags_mask);
}

static void set_flags(i386_state *cpustate, UINT32 f )
{
	cpustate->CF = (f & 0x1) ? 1 : 0;
	cpustate->PF = (f & 0x4) ? 1 : 0;
	cpustate->AF = (f & 0x10) ? 1 : 0;
	cpustate->ZF = (f & 0x40) ? 1 : 0;
	cpustate->SF = (f & 0x80) ? 1 : 0;
	cpustate->TF = (f & 0x100) ? 1 : 0;
	cpustate->IF = (f & 0x200) ? 1 : 0;
	cpustate->DF = (f & 0x400) ? 1 : 0;
	cpustate->OF = (f & 0x800) ? 1 : 0;
	cpustate->IOP1 = (f & 0x1000) ? 1 : 0;
	cpustate->IOP2 = (f & 0x2000) ? 1 : 0;
	cpustate->NT = (f & 0x4000) ? 1 : 0;
	cpustate->RF = (f & 0x10000) ? 1 : 0;
	cpustate->VM = (f & 0x20000) ? 1 : 0;
	cpustate->AC = (f & 0x40000) ? 1 : 0;
	cpustate->VIF = (f & 0x80000) ? 1 : 0;
	cpustate->VIP = (f & 0x100000) ? 1 : 0;
	cpustate->ID = (f & 0x200000) ? 1 : 0;
	cpustate->eflags = f & cpustate->eflags_mask;
}

static void i386_postload(i386_state *cpustate)
{
	int i;
	for (i = 0; i < 6; i++)
		i386_load_segment_descriptor(cpustate,i);
	CHANGE_PC(cpustate,cpustate->eip);
}


extern void i386_op_decode_opcode_main(i386_state *cpustate);
extern void i386_op_decode_two_byte_main(i386_state *cpustate);

static void I386OP(decode_opcode)(i386_state *cpustate)
{
	i386_op_decode_opcode_main(cpustate);
}

/* Two-byte opcode 0f xx */
static void I386OP(decode_two_byte)(i386_state *cpustate)
{
	i386_op_decode_two_byte_main(cpustate);
}

static void I386OP(decode_three_byte38)(i386_state *cpustate)
{
	i386_op_decode_three_byte38_main(cpustate);
}

static void I386OP(decode_three_byte3a)(i386_state *cpustate)
{
	i386_op_decode_three_byte3a_main(cpustate);
}

static void I386OP(decode_three_byte66)(i386_state *cpustate)
{
	i386_op_decode_three_byte66_main(cpustate);
}

static void I386OP(decode_three_bytef2)(i386_state *cpustate)
{
	i386_op_decode_three_bytef2_main(cpustate);
}

static void I386OP(decode_three_bytef3)(i386_state *cpustate)
{
	i386_op_decode_three_bytef3_main(cpustate);
}

static void I386OP(decode_four_byte3866)(i386_state *cpustate)
{
	i386_op_decode_four_byte3866_main(cpustate);
}

static void I386OP(decode_four_byte3a66)(i386_state *cpustate)
{
	i386_op_decode_four_byte3a66_main(cpustate);
}
static void I386OP(decode_four_byte38f2)(i386_state *cpustate)
{
	i386_op_decode_four_byte38f2_main(cpustate);
}
static void I386OP(decode_four_byte3af2)(i386_state *cpustate)
{
	i386_op_decode_four_byte3af2_main(cpustate);
}
static void I386OP(decode_four_byte38f3)(i386_state *cpustate)
{
	i386_op_decode_four_byte38f3_main(cpustate);
}


static i386_state *i386_common_init(int tlbsize)
{
	int i, j;
	static const int regs8[8] = {AL,CL,DL,BL,AH,CH,DH,BH};
	static const int regs16[8] = {AX,CX,DX,BX,SP,BP,SI,DI};
	static const int regs32[8] = {EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI};
	i386_state *cpustate = (i386_state *)malloc(sizeof(i386_state));

	assert((sizeof(XMM_REG)/sizeof(double)) == 2);

	i386_build_cycle_table();

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

	cpustate->vtlb = vtlb_alloc(cpustate, AS_PROGRAM, 0, tlbsize);
	cpustate->smi = false;
	cpustate->lock = false;

//	i386_interface *intf = (i386_interface *) device->static_config();
//
//	if (intf != NULL)
//		cpustate->smiact.resolve(intf->smiact, *device);
//	else
//		memset(&cpustate->smiact, 0, sizeof(cpustate->smiact));

	zero_state(cpustate);

	return cpustate;
}
static void i386_check_irq_line(i386_state *cpustate)
{
	if(!cpustate->smm && cpustate->smi)
	{
		pentium_smi(cpustate);
		return;
	}

	/* Check if the interrupts are enabled */
	if ( (cpustate->irq_state) && cpustate->IF )
	{
		cpustate->cycles -= 2;
		i386_trap(cpustate, cpustate->pic->get_intr_ack(), 1, 0);
		cpustate->irq_state = 0;
	}
}

static void zero_state(i386_state *cpustate)
{
	memset( &cpustate->reg, 0, sizeof(cpustate->reg) );
	memset( cpustate->sreg, 0, sizeof(cpustate->sreg) );
	cpustate->eip = 0;
	cpustate->pc = 0;
	cpustate->prev_eip = 0;
	cpustate->eflags = 0;
	cpustate->eflags_mask = 0;
	cpustate->CF = 0;
	cpustate->DF = 0;
	cpustate->SF = 0;
	cpustate->OF = 0;
	cpustate->ZF = 0;
	cpustate->PF = 0;
	cpustate->AF = 0;
	cpustate->IF = 0;
	cpustate->TF = 0;
	cpustate->IOP1 = 0;
	cpustate->IOP2 = 0;
	cpustate->NT = 0;
	cpustate->RF = 0;
	cpustate->VM = 0;
	cpustate->AC = 0;
	cpustate->VIF = 0;
	cpustate->VIP = 0;
	cpustate->ID = 0;
	cpustate->CPL = 0;
	cpustate->performed_intersegment_jump = 0;
	cpustate->delayed_interrupt_enable = 0;
	memset( cpustate->cr, 0, sizeof(cpustate->cr) );
	memset( cpustate->dr, 0, sizeof(cpustate->dr) );
	memset( cpustate->tr, 0, sizeof(cpustate->tr) );
	memset( &cpustate->gdtr, 0, sizeof(cpustate->gdtr) );
	memset( &cpustate->idtr, 0, sizeof(cpustate->idtr) );
	memset( &cpustate->task, 0, sizeof(cpustate->task) );
	memset( &cpustate->ldtr, 0, sizeof(cpustate->ldtr) );
	cpustate->ext = 0;
	cpustate->halted = 0;
	cpustate->operand_size = 0;
	cpustate->xmm_operand_size = 0;
	cpustate->address_size = 0;
	cpustate->operand_prefix = 0;
	cpustate->address_prefix = 0;
	cpustate->segment_prefix = 0;
	cpustate->segment_override = 0;
	cpustate->cycles = 0;
	cpustate->base_cycles = 0;
	cpustate->opcode = 0;
	cpustate->irq_state = 0;
	cpustate->a20_mask = 0;
	cpustate->cpuid_max_input_value_eax = 0;
	cpustate->cpuid_id0 = 0;
	cpustate->cpuid_id1 = 0;
	cpustate->cpuid_id2 = 0;
	cpustate->cpu_version = 0;
	cpustate->feature_flags = 0;
	cpustate->tsc = 0;
	cpustate->perfctr[0] = cpustate->perfctr[1] = 0;
	memset( cpustate->x87_reg, 0, sizeof(cpustate->x87_reg) );
	cpustate->x87_cw = 0;
	cpustate->x87_sw = 0;
	cpustate->x87_tw = 0;
	cpustate->x87_data_ptr = 0;
	cpustate->x87_inst_ptr = 0;
	cpustate->x87_opcode = 0;
	memset( cpustate->sse_reg, 0, sizeof(cpustate->sse_reg) );
	cpustate->mxcsr = 0;
	cpustate->smm = false;
	cpustate->smi = false;
	cpustate->smi_latched = false;
	cpustate->nmi_masked = false;
	cpustate->nmi_latched = false;
	cpustate->smbase = 0;
#ifdef DEBUG_MISSING_OPCODE
	memset( cpustate->opcode_bytes, 0, sizeof(cpustate->opcode_bytes) );
	cpustate->opcode_pc = 0;
	cpustate->opcode_bytes_length = 0;
#endif
}


void i80386_cpu_reset_main(i386_state *cpustate)
{
   CPU_RESET_CALL(CPU_MODEL);
}

static CPU_EXECUTE( i386 )
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

static CPU_INIT( i386 )
{
	i386_state *cpustate = i386_common_init(32);
	i386_i386_cpu_init_main(cpustate);
	return cpustate;
}
/*************************************************************************/

static CPU_RESET( i386 )
{
	zero_state(cpustate);
	vtlb_flush_dynamic(cpustate->vtlb);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base     = 0xffff0000;
	cpustate->sreg[CS].limit    = 0xffff;
	cpustate->sreg[CS].flags    = 0x9b;
	cpustate->sreg[CS].valid    = true;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;
	cpustate->sreg[DS].valid = cpustate->sreg[ES].valid = cpustate->sreg[FS].valid = cpustate->sreg[GS].valid = cpustate->sreg[SS].valid =true;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;
	cpustate->smm = false;
	cpustate->smi_latched = false;
	cpustate->nmi_masked = false;
	cpustate->nmi_latched = false;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x7fffffe0; // reserved bits set to 1
	cpustate->eflags = 0;
	cpustate->eflags_mask = 0x00037fd7;
	cpustate->eip = 0xfff0;

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 3 (386), Model 0 (DX), Stepping 8 (D1)
	REG32(EAX) = 0;
	REG32(EDX) = (3 << 8) | (0 << 4) | (8);

	cpustate->CPL = 0;

	CHANGE_PC(cpustate,cpustate->eip);
}
/*****************************************************************************/
/* Intel 486 */

static CPU_RESET( i486 )
{
	zero_state(cpustate);
	vtlb_flush_dynamic(cpustate->vtlb);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base     = 0xffff0000;
	cpustate->sreg[CS].limit    = 0xffff;
	cpustate->sreg[CS].flags    = 0x009b;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x00000010;
	cpustate->eflags = 0;
	cpustate->eflags_mask = 0x00077fd7;
	cpustate->eip = 0xfff0;
	cpustate->smm = false;
	cpustate->smi_latched = false;
	cpustate->nmi_masked = false;
	cpustate->nmi_latched = false;

	x87_reset(cpustate);

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 4 (486), Model 0/1 (DX), Stepping 3
	REG32(EAX) = 0;
	REG32(EDX) = (4 << 8) | (0 << 4) | (3);

	CHANGE_PC(cpustate,cpustate->eip);
}


static CPU_INIT( i486 )
{
	i386_state *cpustate = i386_common_init(32);
	i386_i486_cpu_init_main(cpustate);
	return cpustate;
}

/*****************************************************************************/
/* Pentium */


static CPU_INIT( pentium )
{
	// 64 dtlb small, 8 dtlb large, 32 itlb
	i386_state *cpustate = i386_common_init(96);
	i386_pentium_cpu_init_main(cpustate);
	return cpustate;
}

static CPU_RESET( pentium )
{
	zero_state(cpustate);
	vtlb_flush_dynamic(cpustate->vtlb);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base     = 0xffff0000;
	cpustate->sreg[CS].limit    = 0xffff;
	cpustate->sreg[CS].flags    = 0x009b;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x00000010;
	cpustate->eflags = 0x00200000;
	cpustate->eflags_mask = 0x003f7fd7;
	cpustate->eip = 0xfff0;
	cpustate->mxcsr = 0x1f80;
	cpustate->smm = false;
	cpustate->smi_latched = false;
	cpustate->smbase = 0x30000;
	cpustate->nmi_masked = false;
	cpustate->nmi_latched = false;

	x87_reset(cpustate);

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 5 (Pentium), Model 2 (75 - 200MHz), Stepping 5
	REG32(EAX) = 0;
	REG32(EDX) = (5 << 8) | (2 << 4) | (5);

	cpustate->cpuid_id0 = 0x756e6547;   // Genu
	cpustate->cpuid_id1 = 0x49656e69;   // ineI
	cpustate->cpuid_id2 = 0x6c65746e;   // ntel

	cpustate->cpuid_max_input_value_eax = 0x01;
	cpustate->cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	cpustate->feature_flags = 0x000001bf;

	CHANGE_PC(cpustate,cpustate->eip);
}

/*****************************************************************************/
/* Cyrix MediaGX */


static CPU_INIT( mediagx )
{
	// probably 32 unified
	i386_state *cpustate = i386_common_init(32);
	i386_mediagx_cpu_init_main(cpustate);
	return cpustate;
}

static CPU_RESET( mediagx )
{
	zero_state(cpustate);
	vtlb_flush_dynamic(cpustate->vtlb);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base     = 0xffff0000;
	cpustate->sreg[CS].limit    = 0xffff;
	cpustate->sreg[CS].flags    = 0x009b;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x00000010;
	cpustate->eflags = 0x00200000;
	cpustate->eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	cpustate->eip = 0xfff0;
	cpustate->smm = false;
	cpustate->smi_latched = false;
	cpustate->nmi_masked = false;
	cpustate->nmi_latched = false;

	x87_reset(cpustate);

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 4, Model 4 (MediaGX)
	REG32(EAX) = 0;
	REG32(EDX) = (4 << 8) | (4 << 4) | (1); /* TODO: is this correct? */

	cpustate->cpuid_id0 = 0x69727943;   // Cyri
	cpustate->cpuid_id1 = 0x736e4978;   // xIns
	cpustate->cpuid_id2 = 0x6d616574;   // tead

	cpustate->cpuid_max_input_value_eax = 0x01;
	cpustate->cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	cpustate->feature_flags = 0x00000001;

	CHANGE_PC(cpustate,cpustate->eip);
}

/*****************************************************************************/
/* Intel Pentium Pro */

static CPU_INIT( pentium_pro )
{
	// 64 dtlb small, 32 itlb
	i386_state *cpustate = i386_common_init(96);
	i386_pentium_pro_cpu_init_main(cpustate);
	return cpustate;
}

static CPU_RESET( pentium_pro )
{
	zero_state(cpustate);
	vtlb_flush_dynamic(cpustate->vtlb);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base     = 0xffff0000;
	cpustate->sreg[CS].limit    = 0xffff;
	cpustate->sreg[CS].flags    = 0x009b;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x60000010;
	cpustate->eflags = 0x00200000;
	cpustate->eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	cpustate->eip = 0xfff0;
	cpustate->mxcsr = 0x1f80;
	cpustate->smm = false;
	cpustate->smi_latched = false;
	cpustate->smbase = 0x30000;
	cpustate->nmi_masked = false;
	cpustate->nmi_latched = false;

	x87_reset(cpustate);

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 6, Model 1 (Pentium Pro)
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (1 << 4) | (1); /* TODO: is this correct? */

	cpustate->cpuid_id0 = 0x756e6547;   // Genu
	cpustate->cpuid_id1 = 0x49656e69;   // ineI
	cpustate->cpuid_id2 = 0x6c65746e;   // ntel

	cpustate->cpuid_max_input_value_eax = 0x02;
	cpustate->cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	// [15:15] CMOV and FCMOV
	// No MMX
	cpustate->feature_flags = 0x000081bf;

	CHANGE_PC(cpustate,cpustate->eip);
}

/*****************************************************************************/
/* Intel Pentium MMX */

static CPU_INIT( pentium_mmx )
{
	// 64 dtlb small, 8 dtlb large, 32 itlb small, 2 itlb large
	i386_state *cpustate = i386_common_init(96);
	i386_pentium_mmx_cpu_init_main(cpustate);
	return cpustate;
}

static CPU_RESET( pentium_mmx )
{
	zero_state(cpustate);
	vtlb_flush_dynamic(cpustate->vtlb);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base     = 0xffff0000;
	cpustate->sreg[CS].limit    = 0xffff;
	cpustate->sreg[CS].flags    = 0x009b;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x60000010;
	cpustate->eflags = 0x00200000;
	cpustate->eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	cpustate->eip = 0xfff0;
	cpustate->mxcsr = 0x1f80;
	cpustate->smm = false;
	cpustate->smi_latched = false;
	cpustate->smbase = 0x30000;
	cpustate->nmi_masked = false;
	cpustate->nmi_latched = false;

	x87_reset(cpustate);

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 5, Model 4 (P55C)
	REG32(EAX) = 0;
	REG32(EDX) = (5 << 8) | (4 << 4) | (1);

	cpustate->cpuid_id0 = 0x756e6547;   // Genu
	cpustate->cpuid_id1 = 0x49656e69;   // ineI
	cpustate->cpuid_id2 = 0x6c65746e;   // ntel

	cpustate->cpuid_max_input_value_eax = 0x01;
	cpustate->cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 2:2] I/O breakpoints
	// [ 4:4] Time Stamp Counter
	// [ 5:5] Pentium CPU style model specific registers
	// [ 7:7] Machine Check Exception
	// [ 8:8] CMPXCHG8B instruction
	// [23:23] MMX instructions
	cpustate->feature_flags = 0x008001bf;

	CHANGE_PC(cpustate,cpustate->eip);
}

/*****************************************************************************/
/* Intel Pentium II */

static CPU_INIT( pentium2 )
{
	// 64 dtlb small, 8 dtlb large, 32 itlb small, 2 itlb large
	i386_state *cpustate = i386_common_init(96);
	i386_pentium2_cpu_init_main(cpustate);
	return cpustate;
}

static CPU_RESET( pentium2 )
{
	zero_state(cpustate);
	vtlb_flush_dynamic(cpustate->vtlb);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base     = 0xffff0000;
	cpustate->sreg[CS].limit    = 0xffff;
	cpustate->sreg[CS].flags    = 0x009b;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x60000010;
	cpustate->eflags = 0x00200000;
	cpustate->eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	cpustate->eip = 0xfff0;
	cpustate->mxcsr = 0x1f80;
	cpustate->smm = false;
	cpustate->smi_latched = false;
	cpustate->smbase = 0x30000;
	cpustate->nmi_masked = false;
	cpustate->nmi_latched = false;

	x87_reset(cpustate);

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 6, Model 3 (Pentium II / Klamath)
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (3 << 4) | (1); /* TODO: is this correct? */

	cpustate->cpuid_id0 = 0x756e6547;   // Genu
	cpustate->cpuid_id1 = 0x49656e69;   // ineI
	cpustate->cpuid_id2 = 0x6c65746e;   // ntel

	cpustate->cpuid_max_input_value_eax = 0x02;
	cpustate->cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	cpustate->feature_flags = 0x008081bf;       // TODO: enable relevant flags here

	CHANGE_PC(cpustate,cpustate->eip);
}

/*****************************************************************************/
/* Intel Pentium III */

static CPU_INIT( pentium3 )
{
	// 64 dtlb small, 8 dtlb large, 32 itlb small, 2 itlb large
	i386_state *cpustate = i386_common_init(96);
	i386_pentium3_cpu_init_main(cpustate);
	return cpustate;
}

static CPU_RESET( pentium3 )
{
	zero_state(cpustate);
	vtlb_flush_dynamic(cpustate->vtlb);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base     = 0xffff0000;
	cpustate->sreg[CS].limit    = 0xffff;
	cpustate->sreg[CS].flags    = 0x009b;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x60000010;
	cpustate->eflags = 0x00200000;
	cpustate->eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	cpustate->eip = 0xfff0;
	cpustate->mxcsr = 0x1f80;
	cpustate->smm = false;
	cpustate->smi_latched = false;
	cpustate->smbase = 0x30000;
	cpustate->nmi_masked = false;
	cpustate->nmi_latched = false;

	x87_reset(cpustate);

	// [11:8] Family
	// [ 7:4] Model
	// [ 3:0] Stepping ID
	// Family 6, Model 8 (Pentium III / Coppermine)
	REG32(EAX) = 0;
	REG32(EDX) = (6 << 8) | (8 << 4) | (10);

	cpustate->cpuid_id0 = 0x756e6547;   // Genu
	cpustate->cpuid_id1 = 0x49656e69;   // ineI
	cpustate->cpuid_id2 = 0x6c65746e;   // ntel

	cpustate->cpuid_max_input_value_eax = 0x03;
	cpustate->cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	// [ 4:4] Time Stamp Counter
	// [ D:D] PTE Global Bit
	cpustate->feature_flags = 0x00002011;       // TODO: enable relevant flags here

	CHANGE_PC(cpustate,cpustate->eip);
}

/*****************************************************************************/
/* Intel Pentium 4 */

static CPU_INIT( pentium4 )
{
	// 128 dtlb, 64 itlb
	i386_state *cpustate = i386_common_init(196);
	i386_pentium4_cpu_init_main(cpustate);
	return cpustate;
}

static CPU_RESET( pentium4 )
{
	zero_state(cpustate);
	vtlb_flush_dynamic(cpustate->vtlb);

	cpustate->sreg[CS].selector = 0xf000;
	cpustate->sreg[CS].base     = 0xffff0000;
	cpustate->sreg[CS].limit    = 0xffff;
	cpustate->sreg[CS].flags    = 0x009b;

	cpustate->sreg[DS].base = cpustate->sreg[ES].base = cpustate->sreg[FS].base = cpustate->sreg[GS].base = cpustate->sreg[SS].base = 0x00000000;
	cpustate->sreg[DS].limit = cpustate->sreg[ES].limit = cpustate->sreg[FS].limit = cpustate->sreg[GS].limit = cpustate->sreg[SS].limit = 0xffff;
	cpustate->sreg[DS].flags = cpustate->sreg[ES].flags = cpustate->sreg[FS].flags = cpustate->sreg[GS].flags = cpustate->sreg[SS].flags = 0x0092;

	cpustate->idtr.base = 0;
	cpustate->idtr.limit = 0x3ff;

	cpustate->a20_mask = ~0;

	cpustate->cr[0] = 0x60000010;
	cpustate->eflags = 0x00200000;
	cpustate->eflags_mask = 0x00277fd7; /* TODO: is this correct? */
	cpustate->eip = 0xfff0;
	cpustate->mxcsr = 0x1f80;
	cpustate->smm = false;
	cpustate->smi_latched = false;
	cpustate->smbase = 0x30000;
	cpustate->nmi_masked = false;
	cpustate->nmi_latched = false;

	x87_reset(cpustate);

	// [27:20] Extended family
	// [19:16] Extended model
	// [13:12] Type
	// [11: 8] Family
	// [ 7: 4] Model
	// [ 3: 0] Stepping ID
	// Family 15, Model 0 (Pentium 4 / Willamette)
	REG32(EAX) = 0;
	REG32(EDX) = (0 << 20) | (0xf << 8) | (0 << 4) | (1);

	cpustate->cpuid_id0 = 0x756e6547;   // Genu
	cpustate->cpuid_id1 = 0x49656e69;   // ineI
	cpustate->cpuid_id2 = 0x6c65746e;   // ntel

	cpustate->cpuid_max_input_value_eax = 0x02;
	cpustate->cpu_version = REG32(EDX);

	// [ 0:0] FPU on chip
	cpustate->feature_flags = 0x00000001;       // TODO: enable relevant flags here

	CHANGE_PC(cpustate,cpustate->eip);
}

static void pentium_smi(i386_state *cpustate)
{
	i386_pentium_smi_main(cpustate);
}

void i386_call_pseudo_bios_int_main(i386_state *cpustate, int interrupt)
{
#ifdef I386_PSEUDO_BIOS
	BIOS_INT(interrupt)
#endif
}

void i386_call_pseudo_bios_call_main(i386_state *cpustate, UINT16 addr)
{
#ifdef I386_PSEUDO_BIOS
	BIOS_CALL(addr);
#endif

}
