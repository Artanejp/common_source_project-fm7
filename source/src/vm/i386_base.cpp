/*
	Skelton for retropc emulator

	Origin : MAME i386 core
	Author : Takeda.Toshiya
	Date  : 2009.06.08-

	[ i386/i486/Pentium/MediaGX ]
*/

#include "i386_base.h"

void I386_BASE::initialize()
{
//	i386_state *cpustate = (i386_state *)opaque;
//	cpustate->pic = d_pic;
//	cpustate->program = d_mem;
//	cpustate->io = d_io;
//	cpustate->bios = d_bios;
//	cpustate->dma = d_dma;
//	cpustate->shutdown = 0;
}

void I386_BASE::release()
{
	i386_state *cpustate = (i386_state *)opaque;
	vtlb_free(cpustate->vtlb);
	free(opaque);
}

void I386_BASE::reset()
{
	//i386_state *cpustate = (i386_state *)opaque;
	//CPU_RESET_CALL(CPU_MODEL);
}

int I386_BASE::run(int cycles)
{
	//i386_state *cpustate = (i386_state *)opaque;
	return 0;
	//return CPU_EXECUTE_CALL(i386);
}

void I386_BASE::write_signal(int id, uint32_t data, uint32_t mask)
{
	i386_state *cpustate = (i386_state *)opaque;
	
	if(id == SIG_CPU_NMI) {
		i386_set_irq_line(cpustate, INPUT_LINE_NMI, (data & mask) ? HOLD_LINE : CLEAR_LINE);
	} else if(id == SIG_CPU_IRQ) {
		i386_set_irq_line(cpustate, INPUT_LINE_IRQ, (data & mask) ? HOLD_LINE : CLEAR_LINE);
	} else if(id == SIG_CPU_BUSREQ) {
		cpustate->busreq = (data & mask) ? 1 : 0;
	} else if(id == SIG_I386_A20) {
		i386_set_a20_line(cpustate, data & mask);
	}
}

void I386_BASE::set_intr_line(bool line, bool pending, uint32_t bit)
{
	i386_state *cpustate = (i386_state *)opaque;
	i386_set_irq_line(cpustate, INPUT_LINE_IRQ, line ? HOLD_LINE : CLEAR_LINE);
}

void I386_BASE::set_extra_clock(int cycles)
{
	i386_state *cpustate = (i386_state *)opaque;
	cpustate->extra_cycles += cycles;
}

int I386_BASE::get_extra_clock()
{
	i386_state *cpustate = (i386_state *)opaque;
	return cpustate->extra_cycles;
}

uint32_t I386_BASE::get_pc()
{
	i386_state *cpustate = (i386_state *)opaque;
	return cpustate->prev_pc;
}

uint32_t I386_BASE::get_next_pc()
{
	i386_state *cpustate = (i386_state *)opaque;
	return cpustate->pc;
}

void I386_BASE::set_address_mask(uint32_t mask)
{
	i386_state *cpustate = (i386_state *)opaque;
	cpustate->a20_mask = mask;
}

uint32_t I386_BASE::get_address_mask()
{
	i386_state *cpustate = (i386_state *)opaque;
	return cpustate->a20_mask;
}

void I386_BASE::set_shutdown_flag(int shutdown)
{
	i386_state *cpustate = (i386_state *)opaque;
	cpustate->shutdown = shutdown;
}

int I386_BASE::get_shutdown_flag()
{
	i386_state *cpustate = (i386_state *)opaque;
	return cpustate->shutdown;
}

#define STATE_VERSION	1

void I386_BASE::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(opaque, sizeof(i386_state), 1);
}

bool I386_BASE::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(opaque, sizeof(i386_state), 1);
	
	// post process
	i386_state *cpustate = (i386_state *)opaque;
	cpustate->pic = d_pic;
	cpustate->program = d_mem;
	cpustate->io = d_io;

	cpustate->bios = d_bios;
	cpustate->dma = d_dma;

	return true;
}

