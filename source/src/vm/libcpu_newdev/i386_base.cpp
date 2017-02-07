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

#define STATE_VERSION	1

void I386_BASE::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	cpucore->save_state(state_fio);
}

bool I386_BASE::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	cpucore->load_state(state_fio);
	
	// post process
	cpucore->set_context_pic(d_pic);
	cpucore->set_context_progmem(d_mem);
	cpucore->set_context_io(d_io);

	return true;
}

