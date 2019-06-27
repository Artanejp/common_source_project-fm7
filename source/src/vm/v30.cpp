/*
	Skelton for retropc emulator

	Origin : MAME i286 core
	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date  : 2019.06.27-
	History: 2019.06.27 Split from i286.cpp.

	[ NEC V30 ]
*/

#include "v30.h"
#include "debugger.h"
#include "i80x86_commondefs.h"

/* ----------------------------------------------------------------------------
	MAME i86
---------------------------------------------------------------------------- */

// Note:
// API of bios_int_i86() / bios_caii_i86() has changed.
// regs[8] regs[9] are added.These entries set redirect-address by PSEUDO-BIOS.
// If need, will add more entries for cycle#.
// - 20181126 K.O

#define cpu_state i8086_state
#include "mame/emu/cpu/i86/v30.c"
#include "mame/emu/cpu/nec/necdasm.c"


void V30::initialize()
{
	DEVICE::initialize();
	n_cpu_type = N_CPU_TYPE_V30;
	_HAS_i80286 = false;
	_HAS_v30 = true;
	opaque = CPU_INIT_CALL( v30 );
	
	cpu_state *cpustate = (cpu_state *)opaque;
	cpustate->pic = d_pic;
	cpustate->program = d_mem;
	cpustate->io = d_io;
//#ifdef I86_PSEUDO_BIOS
	cpustate->bios = d_bios;
//#endif
//#ifdef SINGLE_MODE_DMA
	cpustate->dma = d_dma;
//#endif
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
	
	if(d_debugger != NULL) {
		d_debugger->set_context_mem(d_mem);
		d_debugger->set_context_io(d_io);
	}
	cpustate->waitfactor = 0;
	cpustate->waitcount = 0;
}

void V30::release()
{
	free(opaque);
}
void V30::reset()
{
	cpu_state *cpustate = (cpu_state *)opaque;
	int busreq = cpustate->busreq;

	CPU_RESET_CALL( v30 );
	
	cpustate->pic = d_pic;
	cpustate->program = d_mem;
	cpustate->io = d_io;
//#ifdef I86_PSEUDO_BIOS
	cpustate->bios = d_bios;
//#endif
//#ifdef SINGLE_MODE_DMA
	cpustate->dma = d_dma;
//#endif
//#ifdef USE_DEBUGGER
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
//#endif
	cpustate->busreq = busreq;
}

int V30::run(int icount)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	int ret = 0;
	ret = CPU_EXECUTE_CALL( v30 );
	return ret;
}
// Belows are common to V30:: .
//uint32_t V30::read_signal(int id)
//void V30::write_signal(int id, uint32_t data, uint32_t mask)
//void V30::set_intr_line(bool line, bool pending, uint32_t bit)
//void V30::set_extra_clock(int icount)
//int V30::get_extra_clock()
//uint32_t V30::get_pc()
//uint32_t V30::get_next_pc()
//uint32_t V30::translate_address(int segment, uint32_t offset)
//void V30::write_debug_data8(uint32_t addr, uint32_t data)
//uint32_t V30::read_debug_data8(uint32_t addr)
//void V30::write_debug_data16(uint32_t addr, uint32_t data)
//uint32_t V30::read_debug_data16(uint32_t addr)
//void V30::write_debug_io8(uint32_t addr, uint32_t data)
//uint32_t V30::read_debug_io8(uint32_t addr) {
//void V30::write_debug_io16(uint32_t addr, uint32_t data)
//uint32_t V30::read_debug_io16(uint32_t addr) {
//bool V30::write_debug_reg(const _TCHAR *reg, uint32_t data)
//uint32_t V30::read_debug_reg(const _TCHAR *reg)
//bool V30::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
int V30::debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	UINT64 eip = pc - cpustate->base[CS];
	UINT8 ops[16];
	for(int i = 0; i < 16; i++) {
		int wait;
		ops[i] = d_mem->read_data8w(pc + i, &wait);
	}
	UINT8 *oprom = ops;
	
	return CPU_DISASSEMBLE_CALL(nec_generic) & DASMFLAG_LENGTHMASK;
}

