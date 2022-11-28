/*
	Skelton for retropc emulator

	Origin : MESS 0.147
	Author : Takeda.Toshiya
	Date   : 2012.10.23-

	[ HuC6280 ]
*/

#include "huc6280.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

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
#define CPU_DISASSEMBLE_CALL(name)		CPU_DISASSEMBLE_NAME(name)(buffer, pc, oprom, oprom, d_debugger->first_symbol)

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
#ifdef USE_DEBUGGER
#include "mame/emu/cpu/h6280/6280dasm.c"
#endif

// main

void HUC6280::initialize()
{
	opaque = CPU_INIT_CALL(h6280);
	
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef USE_DEBUGGER
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
	
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void HUC6280::release()
{
	free(opaque);
}

void HUC6280::reset()
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	
	CPU_RESET_CALL(h6280);
	
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef USE_DEBUGGER
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
#endif
	icount = 0;
	busreq = false;
}

int HUC6280::run(int clock)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	
	if(clock == -1) {
		if(busreq) {
			// don't run cpu!
#ifdef USE_DEBUGGER
			total_icount += 1;
#endif
			return 1;
		} else {
			// run only one opcode
			return run_one_opecode();
		}
	} else {
		icount += clock;
		int first_icount = icount;
		
		// run cpu while given clocks
		while(icount > 0 && !busreq) {
			icount -= run_one_opecode();
		}
		// if busreq is raised, spin cpu while remained clock
		if(icount > 0 && busreq) {
#ifdef USE_DEBUGGER
			total_icount += icount;
#endif
			icount = 0;
		}
		return first_icount - icount;
	}
}

int HUC6280::run_one_opecode()
{
#ifdef USE_DEBUGGER
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	d_debugger->add_cpu_trace(cpustate->pc.w.l);
#endif
	int passed_icount = CPU_EXECUTE_CALL(h6280);
#ifdef USE_DEBUGGER
	total_icount += passed_icount;
#endif
	return passed_icount;
}

void HUC6280::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CPU_BUSREQ) {
		busreq = ((data & mask) != 0);
	} else {
		h6280_Regs *cpustate = (h6280_Regs *)opaque;
		set_irq_line(cpustate, id, data);
	}
}

uint32_t HUC6280::get_pc()
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	return cpustate->ppc.w.l;
}

uint32_t HUC6280::get_next_pc()
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	return cpustate->pc.w.l;
}

uint8_t HUC6280::irq_status_r(uint16_t offset)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	return h6280_irq_status_r(cpustate, offset);
}

void HUC6280::irq_status_w(uint16_t offset, uint8_t data)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	h6280_irq_status_w(cpustate, offset, data);
}

uint8_t HUC6280::timer_r(uint16_t offset)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	return h6280_timer_r(cpustate, offset);
}

void HUC6280::timer_w(uint16_t offset, uint8_t data)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	h6280_timer_w(cpustate, offset, data);
}

#ifdef USE_DEBUGGER
void HUC6280::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem->write_data8w(addr, data, &wait);
}

uint32_t HUC6280::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem->read_data8w(addr, &wait);
}

void HUC6280::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	d_io->write_io8w(addr, data, &wait);
}

uint32_t HUC6280::read_debug_io8(uint32_t addr) {
	int wait;
	return d_io->read_io8w(addr, &wait);
}

bool HUC6280::write_debug_reg(const _TCHAR *reg, uint32_t data)
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

bool HUC6280::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
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

int HUC6280::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	uint8_t oprom[8];
	uint8_t *opram = oprom;
	
	for(int i = 0; i < 8; i++) {
		int wait;
		oprom[i] = d_mem->read_data8w(pc + i, &wait);
	}
	return CPU_DISASSEMBLE_CALL(h6280) & DASMFLAG_LENGTHMASK;
}
#endif

#define STATE_VERSION	5

bool HUC6280::process_state(FILEIO* state_fio, bool loading)
{
	h6280_Regs *cpustate = (h6280_Regs *)opaque;
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(cpustate->ICount);
	state_fio->StateValue(cpustate->ppc);
	state_fio->StateValue(cpustate->pc);
	state_fio->StateValue(cpustate->sp);
	state_fio->StateValue(cpustate->zp);
	state_fio->StateValue(cpustate->ea);
	state_fio->StateValue(cpustate->a);
	state_fio->StateValue(cpustate->x);
	state_fio->StateValue(cpustate->y);
	state_fio->StateValue(cpustate->p);
	state_fio->StateArray(cpustate->mmr, sizeof(cpustate->mmr), 1);
	state_fio->StateValue(cpustate->irq_mask);
	state_fio->StateValue(cpustate->timer_status);
	state_fio->StateValue(cpustate->timer_ack);
	state_fio->StateValue(cpustate->clocks_per_cycle);
	state_fio->StateValue(cpustate->timer_value);
	state_fio->StateValue(cpustate->timer_load);
	state_fio->StateValue(cpustate->nmi_state);
	state_fio->StateArray(cpustate->irq_state, sizeof(cpustate->irq_state), 1);
	state_fio->StateValue(cpustate->irq_pending);
#if LAZY_FLAGS
	state_fio->StateValue(cpustate->NZ);
#endif
#ifdef USE_DEBUGGER
	state_fio->StateValue(total_icount);
#endif
	state_fio->StateValue(icount);
	state_fio->StateValue(busreq);
	
#ifdef USE_DEBUGGER
	// post process
	if(loading) {
		prev_total_icount = total_icount;
	}
#endif
	return true;
}

