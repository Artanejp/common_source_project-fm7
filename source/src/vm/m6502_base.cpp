/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2010.08.10-

	[ M6502 ]
*/

#include "m6502.h"
#include "debugger.h"

// vectors
#define NMI_VEC	0xfffa
#define RST_VEC	0xfffc
#define IRQ_VEC	0xfffe

// flags
#define F_C	0x01
#define F_Z	0x02
#define F_I	0x04
#define F_D	0x08
#define F_B	0x10
#define F_T	0x20
#define F_V	0x40
#define F_N	0x80

// some shortcuts for improved readability
#define A	a
#define X	x
#define Y	y
#define P	p
#define S	sp.b.l
#define SPD	sp.d

#define EAL ea.b.l
#define EAH ea.b.h
#define EAW ea.w.l
#define EAD ea.d

#define ZPL zp.b.l
#define ZPH zp.b.h
#define ZPW zp.w.l
#define ZPD zp.d

#define PCL pc.b.l
#define PCH pc.b.h
#define PCW pc.w.l
#define PCD pc.d

// virtual machine interface
#define RDMEM_ID(addr) d_mem->read_data8(addr)
#define WRMEM_ID(addr, data) d_mem->write_data8(addr, data)

#define RDOP() d_mem->read_data8(PCW++)
#define PEEKOP() d_mem->read_data8(PCW)
#define RDOPARG() d_mem->read_data8(PCW++)

#define RDMEM(addr) d_mem->read_data8(addr)
#define WRMEM(addr, data) d_mem->write_data8(addr, data)

#define CYCLES(c) icount -= (c)

// opcodes
#define PUSH(Rg) WRMEM(SPD, Rg); S--
#define PULL(Rg) S++; Rg = RDMEM(SPD)

void M6502_BASE::OP(uint8_t code)
{
}

inline void M6502_BASE::update_irq()
{
	if(!(P & F_I)) {
		EAD = IRQ_VEC;
		CYCLES(2);
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P |= F_I;
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD + 1);
		// call back the cpuintrf to let it clear the line
		d_pic->notify_intr_reti();
		irq_state = false;
	}
	pending_irq = false;
}

// main

void M6502_BASE::initialize()
{
	DEVICE::initialize();
	A = X = Y = P = 0;
	SPD = EAD = ZPD = PCD = 0;
}

void M6502_BASE::reset()
{
	PCL = RDMEM(RST_VEC);
	PCH = RDMEM(RST_VEC + 1);
	SPD = 0x01ff;
	P = F_T | F_I | F_Z | F_B | (P & F_D);
	
	icount = 0;
	pending_irq = after_cli = false;
	irq_state = nmi_state = so_state = false;
}

int M6502_BASE::run(int clock)
{
	if(clock == -1) {
		if (busreq) {
			// don't run cpu!
			return 1;
		} else {
			// run only one opcode
			icount = 0;
			run_one_opecode();
			return -icount;
		}
	} else {
		icount += clock;
		int first_icount = icount;
		
		// run cpu while given clocks
		while(icount > 0 && !busreq) {
				run_one_opecode();
		}
		// if busreq is raised, spin cpu while remained clock
		if(icount > 0 && busreq) {
			icount = 0;
		}
		return first_icount - icount;
	}
}

void M6502_BASE::run_one_opecode()
{
	int first_icount = icount;
	// if an irq is pending, take it now
	if(nmi_state) {
		EAD = NMI_VEC;
		CYCLES(2);
		PUSH(PCH);
		PUSH(PCL);
		PUSH(P & ~F_B);
		P |= F_I;	// set I flag
		PCL = RDMEM(EAD);
		PCH = RDMEM(EAD + 1);
		nmi_state = false;
	} else if(pending_irq) {
		if(d_debugger != NULL) d_debugger->add_cpu_trace(PCW);
		update_irq();
	}
	prev_pc = PCW;
	uint8_t code = RDOP();
	OP(code);
	
	// check if the I flag was just reset (interrupts enabled)
	if(after_cli) {
		after_cli = false;
		if(irq_state) {
			pending_irq = true;
		}
	} else if(pending_irq) {
		update_irq();
	}
}
void M6502_BASE::write_signal(int id, uint32_t data, uint32_t mask)
{
	bool state = ((data & mask) != 0);
	
	if(id == SIG_CPU_NMI) {
		nmi_state = state;
	} else if(id == SIG_CPU_IRQ) {
		irq_state = state;
		if(state) {
			pending_irq = true;
		}
	} else if(id == SIG_M6502_OVERFLOW) {
		if(so_state && !state) {
			P |= F_V;
		}
		so_state = state;
	} else if(id == SIG_CPU_BUSREQ) {
		busreq = ((data & mask) != 0);
	}
}

//#ifdef USE_DEBUGGER
void M6502_BASE::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem_stored->write_data8w(addr, data, &wait);
}

uint32_t M6502_BASE::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem_stored->read_data8w(addr, &wait);
}

bool M6502_BASE::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if(_tcsicmp(reg, _T("PC")) == 0) {
		PCW = data;
	} else if(_tcsicmp(reg, _T("A")) == 0) {
		A = data;
	} else if(_tcsicmp(reg, _T("X")) == 0) {
		X = data;
	} else if(_tcsicmp(reg, _T("Y")) == 0) {
		Y = data;
	} else if(_tcsicmp(reg, _T("S")) == 0) {
		S = data;
	} else if(_tcsicmp(reg, _T("P")) == 0) {
		P = data;
	} else {
		return false;
	}
	return true;
}

void M6502_BASE::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	my_stprintf_s(buffer, buffer_len,
	_T("PC = %04X  A = %02X  X = %02X  Y = %02X  S = %02X  P = %02X [%c%c%c%c%c%c%c%c]\nClocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
 	PCW, A, X, Y, S, P,
 	(P & F_N) ? _T('N') : _T('-'), (P & F_V) ? _T('V') : _T('-'), (P & F_T) ? _T('T') : _T('-'), (P & F_B) ? _T('B') : _T('-'), 
	(P & F_D) ? _T('D') : _T('-'), (P & F_I) ? _T('I') : _T('-'), (P & F_Z) ? _T('Z') : _T('-'), (P & F_C) ? _T('C') : _T('-'),
	total_icount, total_icount - prev_total_icount,
	get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
	prev_total_icount = total_icount;
}

// disassembler

#define offs_t UINT16

/*****************************************************************************/
/* src/emu/devcpu.h */

// CPU interface functions
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
//const UINT32 DASMFLAG_LENGTHMASK    = 0x0000ffff;   // the low 16-bits contain the actual length

#include "mame/emu/cpu/m6502/6502dasm.c"

int M6502_BASE::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
}
//#endif

void M6502_BASE::save_state_regs(FILEIO* state_fio)
{
	state_fio->FputUint32(pc.d);
	state_fio->FputUint32(sp.d);
	state_fio->FputUint32(zp.d);
	state_fio->FputUint32(ea.d);
	state_fio->FputUint16(prev_pc);
	state_fio->FputUint8(a);
	state_fio->FputUint8(x);
	state_fio->FputUint8(y);
	state_fio->FputUint8(p);
	state_fio->FputBool(pending_irq);
	state_fio->FputBool(after_cli);
	state_fio->FputBool(nmi_state);
	state_fio->FputBool(irq_state);
	state_fio->FputBool(so_state);
}

void M6502_BASE::load_state_regs(FILEIO* state_fio)
{
	pc.d = state_fio->FgetUint32();
	sp.d = state_fio->FgetUint32();
	zp.d = state_fio->FgetUint32();
	ea.d = state_fio->FgetUint32();
	prev_pc = state_fio->FgetUint16();
	a = state_fio->FgetUint8();
	x = state_fio->FgetUint8();
	y = state_fio->FgetUint8();
	p = state_fio->FgetUint8();
	pending_irq = state_fio->FgetBool();
	after_cli = state_fio->FgetBool();
	nmi_state = state_fio->FgetBool();
	irq_state = state_fio->FgetBool();
	so_state = state_fio->FgetBool();
	icount = state_fio->FgetInt32();
	busreq = state_fio->FgetBool();
}

