/*
	Skelton for retropc emulator

	Origin : MESS 0.147
	Author : Takeda.Toshiya
	Date   : 2012.10.23-

	[ HuC6280 ]
*/

#include "huc6280.h"
//#ifdef USE_DEBUGGER
#include "debugger.h"
//#endif

/* ----------------------------------------------------------------------------
	MAME h6280
---------------------------------------------------------------------------- */

#define INLINE inline
#define PAIR pair32_t
#define offs_t UINT16

/*****************************************************************************/
/* src/emu/devcpu.h */

// CPU interface functions
#define CPU_INIT_NAME(name)			cpu_init_##name
#define CPU_INIT(name)				void* CPU_INIT_NAME(name)()
#define CPU_INIT_CALL(name)			CPU_INIT_NAME(name)()

#define CPU_RESET_NAME(name)			cpu_reset_##name
#define CPU_RESET(name)				void CPU_RESET_NAME(name)(h6280_Regs *cpustate)
#define CPU_RESET_CALL(name)			CPU_RESET_NAME(name)(cpustate)

#define CPU_EXECUTE_NAME(name)			cpu_execute_##name
#define CPU_EXECUTE(name)			int CPU_EXECUTE_NAME(name)(h6280_Regs *cpustate)
#define CPU_EXECUTE_CALL(name)			CPU_EXECUTE_NAME(name)(cpustate)

#define CPU_DISASSEMBLE_NAME(name)		cpu_disassemble_##name
#define CPU_DISASSEMBLE(name)			int CPU_DISASSEMBLE_NAME(name)(_TCHAR *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, symbol_t *first_symbol)
#define CPU_DISASSEMBLE_CALL(name)		CPU_DISASSEMBLE_NAME(name)(buffer, pc, oprom, opram, d_debugger->first_symbol)

#define READ8_HANDLER(name) 			UINT8 name(h6280_Regs *cpustate, offs_t offset)
#define WRITE8_HANDLER(name)			void name(h6280_Regs *cpustate, offs_t offset, UINT8 data)

/*****************************************************************************/
/* src/emu/didisasm.h */

// Disassembler constants
const UINT32 DASMFLAG_SUPPORTED     = 0x80000000;   // are disassembly flags supported?
const UINT32 DASMFLAG_STEP_OUT      = 0x40000000;   // this instruction should be the end of a step out sequence
const UINT32 DASMFLAG_STEP_OVER     = 0x20000000;   // this instruction should be stepped over by setting a breakpoint afterwards
const UINT32 DASMFLAG_OVERINSTMASK  = 0x18000000;   // number of extra instructions to skip when stepping over
const UINT32 DASMFLAG_OVERINSTSHIFT = 27;           // bits to shift after masking to get the value
const UINT32 DASMFLAG_LENGTHMASK    = 0x0000ffff;   // the low 16-bits contain the actual length

/*****************************************************************************/
/* src/emu/diexec.h */

// I/O line states
enum line_state
{
	CLEAR_LINE = 0,				// clear (a fired or held) line
	ASSERT_LINE,				// assert an interrupt immediately
	HOLD_LINE,				// hold interrupt line until acknowledged
	PULSE_LINE				// pulse interrupt line instantaneously (only for NMI, RESET)
};

enum
{
	INPUT_LINE_IRQ1 = 0,
	INPUT_LINE_IRQ2 = 1,
	INPUT_LINE_TIRQ = 2,
	INPUT_LINE_NMI
};

#define logerror(...)

#include "mame/emu/cpu/h6280/h6280.c"
//#ifdef USE_DEBUGGER
#include "mame/emu/cpu/h6280/6280dasm.c"
//#endif

// main

void HUC6280_BASE::initialize()
{
	DEVICE::initialize();
	opaque = CPU_INIT_CALL(h6280);
	
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	cpustate->program = d_mem;
	cpustate->io = d_io;

}
void HUC6280_BASE::release()
{
}

void HUC6280_BASE::reset()
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	
	CPU_RESET_CALL(h6280);
	
	cpustate->program = d_mem;
	cpustate->io = d_io;
	icount = 0;
	busreq = false;
}

int HUC6280_BASE::run(int clocks)
{
	return 0;
}

int HUC6280_BASE::run_one_opecode()
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	int passed_icount;
	if(cpustate->debugger != NULL) {
		if(cpustate->debugger->now_debugging) {
			passed_icount = CPU_EXECUTE_CALL(h6280_debug);
		} else {
			passed_icount = CPU_EXECUTE_CALL(h6280);
		}
	} else {
		passed_icount = CPU_EXECUTE_CALL(h6280);
	}
	return passed_icount;
}


void HUC6280_BASE::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CPU_BUSREQ) {
		busreq = ((data & mask) != 0);
	} else {
		h6280_Regs *cpustate = (h6280_Regs *)opaque;
		set_irq_line(cpustate, id, data);
	}
}

uint32_t HUC6280_BASE::get_pc()
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	return cpustate->ppc.w.l;
}

uint32_t HUC6280_BASE::get_next_pc()
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	return cpustate->pc.w.l;
}

uint8_t HUC6280_BASE::irq_status_r(uint16_t offset)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	return h6280_irq_status_r(cpustate, offset);
}

void HUC6280_BASE::irq_status_w(uint16_t offset, uint8_t data)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	h6280_irq_status_w(cpustate, offset, data);
}

uint8_t HUC6280_BASE::timer_r(uint16_t offset)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	return h6280_timer_r(cpustate, offset);
}

void HUC6280_BASE::timer_w(uint16_t offset, uint8_t data)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	h6280_timer_w(cpustate, offset, data);
}

//#ifdef USE_DEBUGGER
void HUC6280_BASE::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem->write_data8w(addr, data, &wait);
}

uint32_t HUC6280_BASE::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem->read_data8w(addr, &wait);
}

void HUC6280_BASE::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	d_io->write_io8w(addr, data, &wait);
}

uint32_t HUC6280_BASE::read_debug_io8(uint32_t addr) {
	int wait;
	return d_io->read_io8w(addr, &wait);
}

bool HUC6280_BASE::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	if(_tcsicmp(reg, _T("PC")) == 0) {
		cpustate->pc.w.l = data;
	} if(_tcsicmp(reg, _T("SP")) == 0) {
		cpustate->sp.w.l = data;
	} if(_tcsicmp(reg, _T("ZP")) == 0) {
		cpustate->zp.w.l = data;
	} if(_tcsicmp(reg, _T("EA")) == 0) {
		cpustate->ea.w.l = data;
	} if(_tcsicmp(reg, _T("A")) == 0) {
		cpustate->a = data;
	} if(_tcsicmp(reg, _T("X")) == 0) {
		cpustate->x = data;
	} if(_tcsicmp(reg, _T("Y")) == 0) {
		cpustate->y = data;
	} if(_tcsicmp(reg, _T("P")) == 0) {
		cpustate->p = data;
	} else {
		return false;
	}
	return true;
}

bool HUC6280_BASE::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	my_stprintf_s(buffer, buffer_len,
	_T("PC = %04X SP = %04X ZP = %04X EA = %04X A = %02X X = %02X Y = %02X P = %02X\n")
	_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
	cpustate->pc.w.l, cpustate->sp.w.l, cpustate->zp.w.l, cpustate->ea.w.l, cpustate->a, cpustate->x, cpustate->y, cpustate->p,
	total_icount, total_icount - prev_total_icount,
	get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());

	prev_total_icount = total_icount;
	return true;
}

// disassembler

int HUC6280_BASE::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	uint8_t oprom[8];
	uint8_t *opram = oprom;
	
	for(int i = 0; i < 8; i++) {
		int wait;
		oprom[i] = d_mem->read_data8w(pc + i, &wait);
	}
	if(d_debugger != NULL) {
		return CPU_DISASSEMBLE_CALL(h6280) & DASMFLAG_LENGTHMASK;
	} else {
		return 0;
	}
}
//#endif


