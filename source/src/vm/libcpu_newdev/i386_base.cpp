/*
	Skelton for retropc emulator

	Origin : MAME i386 core
	Author : Takeda.Toshiya
	Date  : 2009.06.08-

	[ i386/i486/Pentium/MediaGX ]
*/

#include "i386_base.h"
#include "libcpu_i386/i386_opdef.h"

void I386_BASE::initialize()
{
	DEVICE::initialize();
	cpucore = new I386_OPS_BASE;
	cpucore->cpu_init_i386();
	cpucore->set_context_pic(d_pic);
	cpucore->set_context_progmem(d_mem);
	cpucore->set_context_io(d_io);
	cpucore->set_shutdown_flag(0);
}

void I386_BASE::release()
{
	cpucore->i386_vtlb_free();
	cpucore->i386_free_state();
	delete cpucore;
}

void I386_BASE::reset()
{
	cpucore->cpu_reset_i386();
}

int I386_BASE::run(int cycles)
{
	return cpucore->cpu_execute_i386(cycles);
}

void I386_BASE::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CPU_NMI) {
		cpucore->i386_set_irq_line( INPUT_LINE_NMI, (data & mask) ? HOLD_LINE : CLEAR_LINE);
	} else if(id == SIG_CPU_IRQ) {
		cpucore->i386_set_irq_line( INPUT_LINE_IRQ, (data & mask) ? HOLD_LINE : CLEAR_LINE);
	} else if(id == SIG_CPU_BUSREQ) {
		cpucore->set_busreq(((data & mask) != 0));
	} else if(id == SIG_I386_A20) {
		cpucore->i386_set_a20_line( data & mask);
	}
}

void I386_BASE::set_intr_line(bool line, bool pending, uint32_t bit)
{
	cpucore->i386_set_irq_line(INPUT_LINE_IRQ, line ? HOLD_LINE : CLEAR_LINE);
}

void I386_BASE::set_extra_clock(int cycles)
{
	cpucore->set_extra_clock(cycles);
}

int I386_BASE::get_extra_clock()
{
	return cpucore->get_extra_clock();
}

uint32_t I386_BASE::get_pc()
{
	return cpucore->get_prev_pc();
}

uint32_t I386_BASE::get_next_pc()
{
	return cpucore->get_pc();
}

void I386_BASE::set_address_mask(uint32_t mask)
{
	cpucore->set_address_mask(mask);
       
	// TODO: how does A20M and the tlb interact
	cpucore->vtlb_flush_dynamic();
}

uint32_t I386_BASE::get_address_mask()
{
	return cpucore->get_address_mask();
}
void I386_BASE::set_shutdown_flag(int shutdown)
{
	cpucore->set_shutdown_flag(shutdown);
}

int I386_BASE::get_shutdown_flag()
{
	return cpucore->get_shutdown_flag();
}

void I386_BASE::set_context_mem(DEVICE* device)
{
	d_mem = device;
	if(cpucore != NULL) cpucore->set_context_progmem(d_mem);
}

void I386_BASE::set_context_io(DEVICE* device)
{
	d_io = device;
	if(cpucore != NULL) cpucore->set_context_io(d_io);
}

void I386_BASE::set_context_intr(DEVICE* device)
{
	d_pic = device;
	if(cpucore != NULL) cpucore->set_context_pic(d_pic);
}


#include "../../fileio.h"

#define STATE_VERSION	4

void I386_BASE::process_state_SREG(I386_SREG* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->selector);
	state_fio->StateValue(val->flags);
	state_fio->StateValue(val->base);
	state_fio->StateValue(val->limit);
	state_fio->StateValue(val->d);
	state_fio->StateValue(val->valid);
}

void I386_BASE::process_state_SYS_TABLE(I386_SYS_TABLE* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->base);
	state_fio->StateValue(val->limit);
}

void I386_BASE::process_state_SEG_DESC(I386_SEG_DESC* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->segment);
	state_fio->StateValue(val->flags);
	state_fio->StateValue(val->base);
	state_fio->StateValue(val->limit);
}

void I386_BASE::process_state_GPR(I386_GPR* val, FILEIO* state_fio)
{
	state_fio->StateArray(val->d, sizeof(val->d), 1);
	state_fio->StateArray(val->w, sizeof(val->w), 1);
	state_fio->StateArray(val->b, sizeof(val->b), 1);
}

void I386_BASE::process_state_floatx80(floatx80* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->high);
	state_fio->StateValue(val->low);
}

void I386_BASE::process_state_XMM_REG(XMM_REG* val, FILEIO* state_fio)
{
	state_fio->StateArray(val->b, sizeof(val->b), 1);
	state_fio->StateArray(val->w, sizeof(val->w), 1);
	state_fio->StateArray(val->d, sizeof(val->d), 1);
	state_fio->StateArray(val->q, sizeof(val->q), 1);
	state_fio->StateArray(val->c, sizeof(val->c), 1);
	state_fio->StateArray(val->s, sizeof(val->s), 1);
	state_fio->StateArray(val->i, sizeof(val->i), 1);
	state_fio->StateArray(val->l, sizeof(val->l), 1);
	state_fio->StateArray(val->f, sizeof(val->f), 1);
	state_fio->StateArray(val->f64, sizeof(val->f64), 1);
}

void I386_BASE::process_state_vtlb(vtlb_state* val, FILEIO* state_fio)
{
//	state_fio->StateValue(val->space);
//	state_fio->StateValue(val->dynamic);
//	state_fio->StateValue(val->fixed);
	state_fio->StateValue(val->dynindex);
//	state_fio->StateValue(val->pageshift);
//	state_fio->StateValue(val->addrwidth);
	if(val->live != NULL) {
		state_fio->StateArray(val->live, val->fixed + val->dynamic, 1);
	}
	if(val->fixedpages != NULL) {
		state_fio->StateArray(val->fixedpages, val->fixed, 1);
	}
	if(val->table != NULL) {
		state_fio->StateArray(val->table, (size_t) 1 << (val->addrwidth - val->pageshift), 1);
	}
}

bool I386_BASE::process_state(FILEIO* state_fio, bool loading)
{
	i386_state *cpustate = cpucore->get_cpu_state();
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	process_state_GPR(&cpustate->reg, state_fio);
	for(int i = 0; i < array_length(cpustate->sreg); i++) {
		process_state_SREG(&cpustate->sreg[i], state_fio);
	}
	state_fio->StateValue(cpustate->eip);
	state_fio->StateValue(cpustate->pc);
	state_fio->StateValue(cpustate->prev_eip);
	state_fio->StateValue(cpustate->prev_pc);
	state_fio->StateValue(cpustate->eflags);
	state_fio->StateValue(cpustate->eflags_mask);
	state_fio->StateValue(cpustate->CF);
	state_fio->StateValue(cpustate->DF);
	state_fio->StateValue(cpustate->SF);
	state_fio->StateValue(cpustate->OF);
	state_fio->StateValue(cpustate->ZF);
	state_fio->StateValue(cpustate->PF);
	state_fio->StateValue(cpustate->AF);
	state_fio->StateValue(cpustate->IF);
	state_fio->StateValue(cpustate->TF);
	state_fio->StateValue(cpustate->IOP1);
	state_fio->StateValue(cpustate->IOP2);
	state_fio->StateValue(cpustate->NT);
	state_fio->StateValue(cpustate->RF);
	state_fio->StateValue(cpustate->VM);
	state_fio->StateValue(cpustate->AC);
	state_fio->StateValue(cpustate->VIF);
	state_fio->StateValue(cpustate->VIP);
	state_fio->StateValue(cpustate->ID);
	state_fio->StateValue(cpustate->CPL);
	state_fio->StateValue(cpustate->performed_intersegment_jump);
	state_fio->StateValue(cpustate->delayed_interrupt_enable);
	state_fio->StateArray(cpustate->cr, sizeof(cpustate->cr), 1);
	state_fio->StateArray(cpustate->dr, sizeof(cpustate->dr), 1);
	state_fio->StateArray(cpustate->tr, sizeof(cpustate->tr), 1);
	process_state_SYS_TABLE(&cpustate->gdtr, state_fio);
	process_state_SYS_TABLE(&cpustate->idtr, state_fio);
	process_state_SEG_DESC(&cpustate->task, state_fio);
	process_state_SEG_DESC(&cpustate->ldtr, state_fio);
	state_fio->StateValue(cpustate->ext);
	state_fio->StateValue(cpustate->halted);
	state_fio->StateValue(cpustate->busreq);
	state_fio->StateValue(cpustate->shutdown);
	state_fio->StateValue(cpustate->operand_size);
	state_fio->StateValue(cpustate->xmm_operand_size);
	state_fio->StateValue(cpustate->address_size);
	state_fio->StateValue(cpustate->operand_prefix);
	state_fio->StateValue(cpustate->address_prefix);
	state_fio->StateValue(cpustate->segment_prefix);
	state_fio->StateValue(cpustate->segment_override);

	state_fio->StateValue(cpustate->total_cycles);

	state_fio->StateValue(cpustate->cycles);
	state_fio->StateValue(cpustate->extra_cycles);
	state_fio->StateValue(cpustate->base_cycles);
	state_fio->StateValue(cpustate->opcode);
	state_fio->StateValue(cpustate->irq_state);
	state_fio->StateValue(cpustate->a20_mask);
	state_fio->StateValue(cpustate->cpuid_max_input_value_eax);
	state_fio->StateValue(cpustate->cpuid_id0);
	state_fio->StateValue(cpustate->cpuid_id1);
	state_fio->StateValue(cpustate->cpuid_id2);
	state_fio->StateValue(cpustate->cpu_version);
	state_fio->StateValue(cpustate->feature_flags);
	state_fio->StateValue(cpustate->tsc);
	state_fio->StateArray(cpustate->perfctr, sizeof(cpustate->perfctr), 1);
	for(int i = 0; i < array_length(cpustate->x87_reg); i++) {
		process_state_floatx80(&cpustate->x87_reg[i], state_fio);
	}
	state_fio->StateValue(cpustate->x87_cw);
	state_fio->StateValue(cpustate->x87_sw);
	state_fio->StateValue(cpustate->x87_tw);
	state_fio->StateValue(cpustate->x87_data_ptr);
	state_fio->StateValue(cpustate->x87_inst_ptr);
	state_fio->StateValue(cpustate->x87_opcode);
	for(int i = 0; i < array_length(cpustate->sse_reg); i++) {
		process_state_XMM_REG(&cpustate->sse_reg[i], state_fio);
	}
	state_fio->StateValue(cpustate->mxcsr);
	state_fio->StateArray(&cpustate->lock_table[0][0], sizeof(cpustate->lock_table), 1);
	if(cpustate->vtlb != NULL) {
		process_state_vtlb(cpustate->vtlb, state_fio);
	}
	state_fio->StateValue(cpustate->smm);
	state_fio->StateValue(cpustate->smi);
	state_fio->StateValue(cpustate->smi_latched);
	state_fio->StateValue(cpustate->nmi_masked);
	state_fio->StateValue(cpustate->nmi_latched);
	state_fio->StateValue(cpustate->smbase);
//	state_fio->StateValue(cpustate->smiact);
	state_fio->StateValue(cpustate->lock);
#ifdef DEBUG_MISSING_OPCODE
	state_fio->StateArray(cpustate->opcode_bytes, sizeof(cpustate->opcode_bytes), 1);
	state_fio->StateValue(cpustate->opcode_pc);
	state_fio->StateValue(cpustate->opcode_bytes_length);
#endif
	
	// post process
	if(loading) {
		cpustate->prev_total_cycles = cpustate->total_cycles;
	}
	return true;
}

