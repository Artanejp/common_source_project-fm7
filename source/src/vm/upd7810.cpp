/*
	Skelton for retropc emulator

	Origin : MESS 0.152
	Author : Takeda.Toshiya
	Date   : 2016.03.17-

	[ uPD7810 ]
*/

#include "upd7810.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

#if defined(HAS_UPD7810)
	#define CPU_MODEL upd7810
#elif defined(HAS_UPD7807)
	#define CPU_MODEL upd7807
#elif defined(HAS_UPD7801)
	#define CPU_MODEL upd7801
#elif defined(HAS_UPD78C05)
	#define CPU_MODEL upd78c05
#elif defined(HAS_UPD78C06)
	#define CPU_MODEL upd78c06
#elif defined(HAS_UPD7907)
	#define CPU_MODEL upd7907
#endif

/* ----------------------------------------------------------------------------
	MAME uPD7810
---------------------------------------------------------------------------- */

#define PAIR pair32_t
#define offs_t UINT16

/*****************************************************************************/
/* src/emu/devcpu.h */

// CPU interface functions
#define CPU_INIT_NAME(name)			cpu_init_##name
#define CPU_INIT(name)				void* CPU_INIT_NAME(name)()
#define CPU_INIT_CALL(name)			CPU_INIT_NAME(name)()

#define CPU_RESET_NAME(name)			cpu_reset_##name
#define CPU_RESET(name)				void CPU_RESET_NAME(name)(upd7810_state *cpustate)
#define CPU_RESET_CALL(name)			CPU_RESET_NAME(name)(cpustate)

#define CPU_EXECUTE_NAME(name)			cpu_execute_##name
#define CPU_EXECUTE(name)			int CPU_EXECUTE_NAME(name)(upd7810_state *cpustate)
#define CPU_EXECUTE_CALL(name)			CPU_EXECUTE_NAME(name)(cpustate)

#define CPU_DISASSEMBLE_NAME(name)		cpu_disassemble_##name
#define CPU_DISASSEMBLE(name)			int CPU_DISASSEMBLE_NAME(name)(_TCHAR *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, symbol_t *first_symbol)
#define CPU_DISASSEMBLE_CALL(name)		CPU_DISASSEMBLE_NAME(name)(buffer, pc, oprom, oprom, d_debugger->first_symbol)

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
	ASSERT_LINE				// assert an interrupt immediately
};

enum
{
	UPD7810_INTF1  = 0,
	UPD7810_INTF2  = 1,
	UPD7810_INTF0  = 2,
	UPD7810_INTFE1 = 4,
	INPUT_LINE_NMI
};

#define logerror(...)
#define fatalerror(...)

#undef IN
#undef OUT

#include "mame/emu/cpu/upd7810/upd7810.c"
#ifdef USE_DEBUGGER
#undef _DOFF
#include "mame/emu/cpu/upd7810/7810dasm.c"
#endif

// main

void UPD7810::initialize()
{
	opaque = CPU_INIT_CALL(upd7810);
	
	upd7810_state *cpustate = (upd7810_state *)opaque;
#if defined(HAS_UPD7810)
	cpustate->config.type = TYPE_7810;
#elif defined(HAS_UPD7807)
	cpustate->config.type = TYPE_7807;
#elif defined(HAS_UPD7801)
	cpustate->config.type = TYPE_7801;
#elif defined(HAS_UPD78C05)
	cpustate->config.type = TYPE_78C05;
#elif defined(HAS_UPD78C06)
	cpustate->config.type = TYPE_78C06;
#elif defined(HAS_UPD7907)
	cpustate->config.type = TYPE_78C06;
#endif
	cpustate->program = d_mem;
	cpustate->io = d_io;
	cpustate->outputs_to = (void*)&outputs_to;
	cpustate->outputs_txd = (void*)&outputs_txd;
#ifdef USE_DEBUGGER
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
	
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void UPD7810::release()
{
	free(opaque);
}

void UPD7810::reset()
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	
	CPU_RESET_CALL(CPU_MODEL);
	
	cpustate->program = d_mem;
	cpustate->io = d_io;
	cpustate->outputs_to = (void*)&outputs_to;
	cpustate->outputs_txd = (void*)&outputs_txd;
#ifdef USE_DEBUGGER
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
#endif
	icount = 0;
	busreq = false;
}

int UPD7810::run(int clock)
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	
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

int UPD7810::run_one_opecode()
{
#ifdef USE_DEBUGGER
	upd7810_state *cpustate = (upd7810_state *)opaque;
	d_debugger->add_cpu_trace(cpustate->pc.w.l);
#endif
	int passed_icount = CPU_EXECUTE_CALL(upd7810);
#ifdef USE_DEBUGGER
	total_icount += passed_icount;
#endif
	return passed_icount;
}

void UPD7810::write_signal(int id, uint32_t data, uint32_t mask)
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	
	switch(id) {
	case SIG_UPD7810_INTF1:
		set_irq_line(cpustate, UPD7810_INTF1, (data & mask) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case SIG_UPD7810_INTF2:
		set_irq_line(cpustate, UPD7810_INTF2, (data & mask) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case SIG_UPD7810_INTF0:
		set_irq_line(cpustate, UPD7810_INTF0, (data & mask) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case SIG_UPD7810_INTFE1:
		set_irq_line(cpustate, UPD7810_INTFE1, (data & mask) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case SIG_UPD7810_NMI:
		set_irq_line(cpustate, INPUT_LINE_NMI, (data & mask) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case SIG_CPU_BUSREQ:
		busreq = ((data & mask) != 0);
		break;
	}
}

uint32_t UPD7810::get_pc()
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	return cpustate->ppc.w.l;
}

uint32_t UPD7810::get_next_pc()
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	return cpustate->pc.w.l;
}

#ifdef USE_DEBUGGER
void UPD7810::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem->write_data8w(addr, data, &wait);
}

uint32_t UPD7810::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem->read_data8w(addr, &wait);
}

void UPD7810::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	d_io->write_io8w(addr, data, &wait);
}

uint32_t UPD7810::read_debug_io8(uint32_t addr) {
	int wait;
	return d_io->read_io8w(addr, &wait);
}

bool UPD7810::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	
	if(_tcsicmp(reg, _T("PC")) == 0) {
		PC = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		SP = data;
	} else if(_tcsicmp(reg, _T("VA")) == 0) {
		VA = data;
	} else if(_tcsicmp(reg, _T("BC")) == 0) {
		BC = data;
	} else if(_tcsicmp(reg, _T("DE")) == 0) {
		DE = data;
	} else if(_tcsicmp(reg, _T("HL")) == 0) {
		HL = data;
	} else if(_tcsicmp(reg, _T("V")) == 0) {
		V = data;
	} else if(_tcsicmp(reg, _T("A")) == 0) {
		A = data;
	} else if(_tcsicmp(reg, _T("B")) == 0) {
		B = data;
	} else if(_tcsicmp(reg, _T("C")) == 0) {
		C = data;
	} else if(_tcsicmp(reg, _T("D")) == 0) {
		D = data;
	} else if(_tcsicmp(reg, _T("E")) == 0) {
		E = data;
	} else if(_tcsicmp(reg, _T("H")) == 0) {
		H = data;
	} else if(_tcsicmp(reg, _T("L")) == 0) {
		L = data;
	} else if(_tcsicmp(reg, _T("VA'")) == 0) {
		VA2 = data;
	} else if(_tcsicmp(reg, _T("BC'")) == 0) {
		BC2 = data;
	} else if(_tcsicmp(reg, _T("DE'")) == 0) {
		DE2 = data;
	} else if(_tcsicmp(reg, _T("HL'")) == 0) {
		HL2 = data;
	} else if(_tcsicmp(reg, _T("V'")) == 0) {
		V2 = data;
	} else if(_tcsicmp(reg, _T("A'")) == 0) {
		A2 = data;
	} else if(_tcsicmp(reg, _T("B'")) == 0) {
		B2 = data;
	} else if(_tcsicmp(reg, _T("C'")) == 0) {
		C2 = data;
	} else if(_tcsicmp(reg, _T("D'")) == 0) {
		D2 = data;
	} else if(_tcsicmp(reg, _T("E'")) == 0) {
		E2 = data;
	} else if(_tcsicmp(reg, _T("H'")) == 0) {
		H2 = data;
	} else if(_tcsicmp(reg, _T("L'")) == 0) {
		L2 = data;
	} else {
		return false;
	}
	return true;
}

bool UPD7810::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
/*
VA = 0000  BC = 0000  DE = 0000 HL = 0000  PSW= 00 [Z SK HC L1 L0 CY]
VA'= 0000  BC'= 0000  DE'= 0000 HL'= 0000  SP = 0000  PC = 0000
          (BC)= 0000 (DE)=0000 (HL)= 0000 (SP)= 0000 <DI>
Clocks = 0 (0)  Since Scanline = 0/0 (0/0)
*/
	upd7810_state *cpustate = (upd7810_state *)opaque;
	int wait;
	my_stprintf_s(buffer, buffer_len,
	_T("VA = %04X  BC = %04X  DE = %04X HL = %04X  PSW= %02x [%s %s %s %s %s %s]\n")
	_T("VA'= %04X  BC'= %04X  DE'= %04X HL'= %04X  SP = %04X  PC = %04X\n")
	_T("          (BC)= %04X (DE)=%04X (HL)= %04X (SP)= %04X <%s>\n")
	_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
	VA, BC, DE, HL, PSW,
	(PSW & Z) ? _T("Z") : _T("-"), (PSW & SK) ? _T("SK") : _T("--"), (PSW & HC) ? _T("HC") : _T("--"), (PSW & L1) ? _T("L1") : _T("--"), (PSW & L0) ? _T("L0") : _T("--"), (PSW & CY) ? _T("CY") : _T("--"),
	VA2, BC2, DE2, HL2, SP, PC,
	d_mem->read_data16w(BC, &wait), d_mem->read_data16w(DE, &wait), d_mem->read_data16w(HL, &wait), d_mem->read_data16w(SP, &wait),
	IFF ? _T("EI") : _T("DI"),
	total_icount, total_icount - prev_total_icount,
	get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
	prev_total_icount = total_icount;
	return true;
}

// disassembler

int UPD7810::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	uint8_t oprom[8];
	uint8_t *opram = oprom;
	
	for(int i = 0; i < 8; i++) {
		int wait;
		oprom[i] = d_mem->read_data8w(pc + i, &wait);
	}
	return CPU_DISASSEMBLE_CALL(CPU_MODEL) & DASMFLAG_LENGTHMASK;
}
#endif

#define STATE_VERSION	5

bool UPD7810::process_state(FILEIO* state_fio, bool loading)
{
	upd7810_state *cpustate = (upd7810_state *)opaque;
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(cpustate->ppc);
	state_fio->StateValue(cpustate->pc);
	state_fio->StateValue(cpustate->sp);
	state_fio->StateValue(cpustate->op);
	state_fio->StateValue(cpustate->op2);
	state_fio->StateValue(cpustate->iff);
	state_fio->StateValue(cpustate->softi);
	state_fio->StateValue(cpustate->psw);
	state_fio->StateValue(cpustate->ea);
	state_fio->StateValue(cpustate->va);
	state_fio->StateValue(cpustate->bc);
	state_fio->StateValue(cpustate->de);
	state_fio->StateValue(cpustate->hl);
	state_fio->StateValue(cpustate->ea2);
	state_fio->StateValue(cpustate->va2);
	state_fio->StateValue(cpustate->bc2);
	state_fio->StateValue(cpustate->de2);
	state_fio->StateValue(cpustate->hl2);
	state_fio->StateValue(cpustate->cnt);
	state_fio->StateValue(cpustate->tm);
	state_fio->StateValue(cpustate->ecnt);
	state_fio->StateValue(cpustate->etm);
	state_fio->StateValue(cpustate->ma);
	state_fio->StateValue(cpustate->mb);
	state_fio->StateValue(cpustate->mcc);
	state_fio->StateValue(cpustate->mc);
	state_fio->StateValue(cpustate->mm);
	state_fio->StateValue(cpustate->mf);
	state_fio->StateValue(cpustate->tmm);
	state_fio->StateValue(cpustate->etmm);
	state_fio->StateValue(cpustate->eom);
	state_fio->StateValue(cpustate->sml);
	state_fio->StateValue(cpustate->smh);
	state_fio->StateValue(cpustate->anm);
	state_fio->StateValue(cpustate->mkl);
	state_fio->StateValue(cpustate->mkh);
	state_fio->StateValue(cpustate->zcm);
	state_fio->StateValue(cpustate->pa_in);
	state_fio->StateValue(cpustate->pb_in);
	state_fio->StateValue(cpustate->pc_in);
	state_fio->StateValue(cpustate->pd_in);
	state_fio->StateValue(cpustate->pf_in);
	state_fio->StateValue(cpustate->pa_out);
	state_fio->StateValue(cpustate->pb_out);
	state_fio->StateValue(cpustate->pc_out);
	state_fio->StateValue(cpustate->pd_out);
	state_fio->StateValue(cpustate->pf_out);
	state_fio->StateValue(cpustate->cr0);
	state_fio->StateValue(cpustate->cr1);
	state_fio->StateValue(cpustate->cr2);
	state_fio->StateValue(cpustate->cr3);
	state_fio->StateValue(cpustate->txb);
	state_fio->StateValue(cpustate->rxb);
	state_fio->StateValue(cpustate->txd);
	state_fio->StateValue(cpustate->rxd);
	state_fio->StateValue(cpustate->sck);
	state_fio->StateValue(cpustate->ti);
	state_fio->StateValue(cpustate->to);
	state_fio->StateValue(cpustate->ci);
	state_fio->StateValue(cpustate->co0);
	state_fio->StateValue(cpustate->co1);
	state_fio->StateValue(cpustate->irr);
	state_fio->StateValue(cpustate->itf);
	state_fio->StateValue(cpustate->int1);
	state_fio->StateValue(cpustate->int2);
	state_fio->StateValue(cpustate->txs);
	state_fio->StateValue(cpustate->rxs);
	state_fio->StateValue(cpustate->txcnt);
	state_fio->StateValue(cpustate->rxcnt);
	state_fio->StateValue(cpustate->txbuf);
	state_fio->StateValue(cpustate->ovc0);
	state_fio->StateValue(cpustate->ovc1);
	state_fio->StateValue(cpustate->ovce);
	state_fio->StateValue(cpustate->ovcf);
	state_fio->StateValue(cpustate->ovcs);
	state_fio->StateValue(cpustate->ovcsio);
	state_fio->StateValue(cpustate->edges);
	state_fio->StateValue(cpustate->icount);
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

