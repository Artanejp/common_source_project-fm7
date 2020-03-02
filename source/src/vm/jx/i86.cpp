/*
	Skelton for retropc emulator

	Origin : MAME i286 core
	Author : Takeda.Toshiya
	Date   : 2012.10.18-

	[ i86 ]
*/

#include "i86.h"
#ifdef USE_DEBUGGER
#include "../debugger.h"
#include "../i386_dasm.h"
//#include "../v30_dasm.h"
#endif

/* ----------------------------------------------------------------------------
	MAME i286
---------------------------------------------------------------------------- */

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4018 )
#pragma warning( disable : 4146 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4996 )
#endif

#ifndef __BIG_ENDIAN__
#define LSB_FIRST
#endif

#ifndef INLINE
#define INLINE inline
#endif

#define logerror(...)

/*****************************************************************************/
/* src/emu/devcpu.h */

// CPU interface functions
#define CPU_INIT_NAME(name)			cpu_init_##name
#define CPU_INIT(name)				void* CPU_INIT_NAME(name)()
#define CPU_INIT_CALL(name)			CPU_INIT_NAME(name)()

#define CPU_RESET_NAME(name)			cpu_reset_##name
#define CPU_RESET(name)				void CPU_RESET_NAME(name)(cpu_state *cpustate)
#define CPU_RESET_CALL(name)			CPU_RESET_NAME(name)(cpustate)

#define CPU_EXECUTE_NAME(name)			cpu_execute_##name
#define CPU_EXECUTE(name)			int CPU_EXECUTE_NAME(name)(cpu_state *cpustate, int icount)
#define CPU_EXECUTE_CALL(name)			CPU_EXECUTE_NAME(name)(cpustate, icount)

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
	INPUT_LINE_IRQ = 0,
	INPUT_LINE_NMI
};

/*****************************************************************************/
/* src/emu/emucore.h */

// constants for expression endianness
enum endianness_t
{
	ENDIANNESS_LITTLE,
	ENDIANNESS_BIG
};

// declare native endianness to be one or the other
#ifdef LSB_FIRST
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_LITTLE;
#else
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_BIG;
#endif
// endian-based value: first value is if 'endian' is little-endian, second is if 'endian' is big-endian
#define ENDIAN_VALUE_LE_BE(endian,leval,beval)	(((endian) == ENDIANNESS_LITTLE) ? (leval) : (beval))
// endian-based value: first value is if native endianness is little-endian, second is if native is big-endian
#define NATIVE_ENDIAN_VALUE_LE_BE(leval,beval)	ENDIAN_VALUE_LE_BE(ENDIANNESS_NATIVE, leval, beval)
// endian-based value: first value is if 'endian' matches native, second is if 'endian' doesn't match native
#define ENDIAN_VALUE_NE_NNE(endian,leval,beval)	(((endian) == ENDIANNESS_NATIVE) ? (neval) : (nneval))

/*****************************************************************************/
/* src/emu/memory.h */

// offsets and addresses are 32-bit (for now...)
typedef UINT32	offs_t;

#define cpu_state i8086_state
#include "../mame/emu/cpu/i86/i86.c"

void I86::initialize()
{
//	switch(device_model) {
//	case INTEL_8086:
//		opaque = CPU_INIT_CALL(i8086);
//		set_device_name(_T("8086 CPU"));
//		break;
//	case INTEL_8088:
		opaque = CPU_INIT_CALL(i8088);
		set_device_name(_T("8088 CPU"));
//		break;
//	case INTEL_80186:
//		opaque = CPU_INIT_CALL(i80186);
//		set_device_name(_T("80186 CPU"));
//		break;
//	case NEC_V30:
//		opaque = CPU_INIT_CALL(v30);
//		set_device_name(_T("V30 CPU"));
//		break;
//	}
	cpu_state *cpustate = (cpu_state *)opaque;
	cpustate->pic = d_pic;
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef I86_PSEUDO_BIOS
	cpustate->bios = d_bios;
#endif
#ifdef SINGLE_MODE_DMA
	cpustate->dma = d_dma;
#endif
#ifdef USE_DEBUGGER
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
	
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void I86::release()
{
	free(opaque);
}

void I86::reset()
{
	cpu_state *cpustate = (cpu_state *)opaque;
	int busreq = cpustate->busreq;
	
//	switch(device_model) {
//	case INTEL_8086:
//		CPU_RESET_CALL(i8086);
//		break;
//	case INTEL_8088:
		CPU_RESET_CALL(i8088);
//		break;
//	case INTEL_80186:
//		CPU_RESET_CALL(i80186);
//		break;
//	case NEC_V30:
//		CPU_RESET_CALL(v30);
//		break;
//	}
	cpustate->pic = d_pic;
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef I86_PSEUDO_BIOS
	cpustate->bios = d_bios;
#endif
#ifdef SINGLE_MODE_DMA
	cpustate->dma = d_dma;
#endif
#ifdef USE_DEBUGGER
	cpustate->emu = emu;
	cpustate->debugger = d_debugger;
	cpustate->program_stored = d_mem;
	cpustate->io_stored = d_io;
#endif
	cpustate->busreq = busreq;
}

int I86::run(int icount)
{
	cpu_state *cpustate = (cpu_state *)opaque;
#ifdef _JX
	// ugly patch for PC/JX hardware diagnostics :-(
#ifdef TIMER_HACK
	if(cpustate->pc == 0xff040) cpustate->pc = 0xff04a;
	if(cpustate->pc == 0xff17d) cpustate->pc = 0xff18f;
#endif
#ifdef KEYBOARD_HACK
	if(cpustate->pc == 0xfa909) { cpustate->regs.b[BH] = read_port_byte(0xa1); cpustate->pc = 0xfa97c; }
	if(cpustate->pc == 0xff6e1) { cpustate->regs.b[AL] = 0x0d; cpustate->pc += 2; }
#endif
#endif
//	switch(device_model) {
//	case INTEL_8086:
//		return CPU_EXECUTE_CALL(i8086);
//	case INTEL_8088:
		return CPU_EXECUTE_CALL(i8088);
//	case INTEL_80186:
//		return CPU_EXECUTE_CALL(i80186);
//	case NEC_V30:
//		return CPU_EXECUTE_CALL(v30);
//	}
//	return 0;
}

void I86::write_signal(int id, uint32_t data, uint32_t mask)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	
	if(id == SIG_CPU_NMI) {
		set_irq_line(cpustate, INPUT_LINE_NMI, (data & mask) ? HOLD_LINE : CLEAR_LINE);
	} else if(id == SIG_CPU_IRQ) {
		set_irq_line(cpustate, INPUT_LINE_IRQ, (data & mask) ? HOLD_LINE : CLEAR_LINE);
	} else if(id == SIG_CPU_BUSREQ) {
		cpustate->busreq = (data & mask) ? 1 : 0;
	} else if(id == SIG_I86_TEST) {
		cpustate->test_state = (data & mask) ? 1 : 0;
	}
}

void I86::set_intr_line(bool line, bool pending, uint32_t bit)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	set_irq_line(cpustate, INPUT_LINE_IRQ, line ? HOLD_LINE : CLEAR_LINE);
}

void I86::set_extra_clock(int icount)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	cpustate->extra_cycles += icount;
}

int I86::get_extra_clock()
{
	cpu_state *cpustate = (cpu_state *)opaque;
	return cpustate->extra_cycles;
}

uint32_t I86::get_pc()
{
	cpu_state *cpustate = (cpu_state *)opaque;
	return cpustate->prevpc;
}

uint32_t I86::get_next_pc()
{
	cpu_state *cpustate = (cpu_state *)opaque;
	return cpustate->pc;
}

#ifdef USE_DEBUGGER
void I86::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem->write_data8w(addr, data, &wait);
}

uint32_t I86::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem->read_data8w(addr, &wait);
}

void I86::write_debug_data16(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem->write_data16w(addr, data, &wait);
}

uint32_t I86::read_debug_data16(uint32_t addr)
{
	int wait;
	return d_mem->read_data16w(addr, &wait);
}

void I86::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	d_io->write_io8w(addr, data, &wait);
}

uint32_t I86::read_debug_io8(uint32_t addr)
{
	int wait;
	return d_io->read_io8w(addr, &wait);
}

void I86::write_debug_io16(uint32_t addr, uint32_t data)
{
	int wait;
	d_io->write_io16w(addr, data, &wait);
}

uint32_t I86::read_debug_io16(uint32_t addr)
{
	int wait;
	return d_io->read_io16w(addr, &wait);
}

bool I86::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	if(_tcsicmp(reg, _T("IP")) == 0) {
		cpustate->pc = ((data & 0xffff) + cpustate->base[CS]) & AMASK;
		CHANGE_PC(cpustate->pc);
	} else if(_tcsicmp(reg, _T("AX")) == 0) {
		cpustate->regs.w[AX] = data;
	} else if(_tcsicmp(reg, _T("BX")) == 0) {
		cpustate->regs.w[BX] = data;
	} else if(_tcsicmp(reg, _T("CX")) == 0) {
		cpustate->regs.w[CX] = data;
	} else if(_tcsicmp(reg, _T("DX")) == 0) {
		cpustate->regs.w[DX] = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		cpustate->regs.w[SP] = data;
	} else if(_tcsicmp(reg, _T("BP")) == 0) {
		cpustate->regs.w[BP] = data;
	} else if(_tcsicmp(reg, _T("SI")) == 0) {
		cpustate->regs.w[SI] = data;
	} else if(_tcsicmp(reg, _T("DI")) == 0) {
		cpustate->regs.w[DI] = data;
	} else if(_tcsicmp(reg, _T("AL")) == 0) {
		cpustate->regs.b[AL] = data;
	} else if(_tcsicmp(reg, _T("AH")) == 0) {
		cpustate->regs.b[AH] = data;
	} else if(_tcsicmp(reg, _T("BL")) == 0) {
		cpustate->regs.b[BL] = data;
	} else if(_tcsicmp(reg, _T("BH")) == 0) {
		cpustate->regs.b[BH] = data;
	} else if(_tcsicmp(reg, _T("CL")) == 0) {
		cpustate->regs.b[CL] = data;
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		cpustate->regs.b[CH] = data;
	} else if(_tcsicmp(reg, _T("DL")) == 0) {
		cpustate->regs.b[DL] = data;
	} else if(_tcsicmp(reg, _T("DH")) == 0) {
		cpustate->regs.b[DH] = data;
	} else {
		return false;
	}
	return true;
}

uint32_t I86::read_debug_reg(const _TCHAR *reg)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	if(_tcsicmp(reg, _T("IP")) == 0) {
		return cpustate->pc - cpustate->base[CS];
	} else if(_tcsicmp(reg, _T("AX")) == 0) {
		return cpustate->regs.w[AX];
	} else if(_tcsicmp(reg, _T("BX")) == 0) {
		return cpustate->regs.w[BX];
	} else if(_tcsicmp(reg, _T("CX")) == 0) {
		return cpustate->regs.w[CX];
	} else if(_tcsicmp(reg, _T("DX")) == 0) {
		return cpustate->regs.w[DX];
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		return cpustate->regs.w[SP];
	} else if(_tcsicmp(reg, _T("BP")) == 0) {
		return cpustate->regs.w[BP];
	} else if(_tcsicmp(reg, _T("SI")) == 0) {
		return cpustate->regs.w[SI];
	} else if(_tcsicmp(reg, _T("DI")) == 0) {
		return cpustate->regs.w[DI];
	} else if(_tcsicmp(reg, _T("AL")) == 0) {
		return cpustate->regs.b[AL];
	} else if(_tcsicmp(reg, _T("AH")) == 0) {
		return cpustate->regs.b[AH];
	} else if(_tcsicmp(reg, _T("BL")) == 0) {
		return cpustate->regs.b[BL];
	} else if(_tcsicmp(reg, _T("BH")) == 0) {
		return cpustate->regs.b[BH];
	} else if(_tcsicmp(reg, _T("CL")) == 0) {
		return cpustate->regs.b[CL];
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		return cpustate->regs.b[CH];
	} else if(_tcsicmp(reg, _T("DL")) == 0) {
		return cpustate->regs.b[DL];
	} else if(_tcsicmp(reg, _T("DH")) == 0) {
		return cpustate->regs.b[DH];
	}
	return 0;
}

bool I86::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	my_stprintf_s(buffer, buffer_len,
	_T("AX=%04X  BX=%04X CX=%04X DX=%04X SP=%04X  BP=%04X  SI=%04X  DI=%04X\n")
	_T("DS=%04X  ES=%04X SS=%04X CS=%04X IP=%04X  FLAG=[%c%c%c%c%c%c%c%c%c]\n")
	_T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
	cpustate->regs.w[AX], cpustate->regs.w[BX], cpustate->regs.w[CX], cpustate->regs.w[DX], cpustate->regs.w[SP], cpustate->regs.w[BP], cpustate->regs.w[SI], cpustate->regs.w[DI],
	cpustate->sregs[DS], cpustate->sregs[ES], cpustate->sregs[SS], cpustate->sregs[CS], cpustate->pc - cpustate->base[CS],
	OF ? _T('O') : _T('-'), DF ? _T('D') : _T('-'), cpustate->IF ? _T('I') : _T('-'), cpustate->TF ? _T('T') : _T('-'),
	SF ? _T('S') : _T('-'), ZF ? _T('Z') : _T('-'), AF ? _T('A') : _T('-'), PF ? _T('P') : _T('-'), CF ? _T('C') : _T('-'),
	cpustate->total_icount, cpustate->total_icount - cpustate->prev_total_icount,
	get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
	cpustate->prev_total_icount = cpustate->total_icount;
	return true;
}

int I86::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	uint32_t eip = pc - cpustate->base[CS];
	uint8_t oprom[16];

	for(int i = 0; i < 16; i++) {
		int wait;
		oprom[i] = d_mem->read_data8w((pc + i) & AMASK, &wait);
	}
//	switch(device_model) {
//	case NEC_V30:
//		return v30_dasm(cpustate->debugger, oprom, eip, (cpustate->MF == 0), buffer, buffer_len);
//	default:
		return i386_dasm(oprom, eip, false, buffer, buffer_len);
//	}
}
#endif

#define STATE_VERSION	1

bool I86::process_state(FILEIO* state_fio, bool loading)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(cpustate->regs.w, sizeof(cpustate->regs.w), 1);
	state_fio->StateValue(cpustate->pc);
	state_fio->StateValue(cpustate->prevpc);
	state_fio->StateArray(cpustate->base, sizeof(cpustate->base), 1);
	state_fio->StateArray(cpustate->sregs, sizeof(cpustate->sregs), 1);
	state_fio->StateValue(cpustate->flags);
	state_fio->StateValue(cpustate->AuxVal);
	state_fio->StateValue(cpustate->OverVal);
	state_fio->StateValue(cpustate->SignVal);
	state_fio->StateValue(cpustate->ZeroVal);
	state_fio->StateValue(cpustate->CarryVal);
	state_fio->StateValue(cpustate->DirVal);
	state_fio->StateValue(cpustate->ParityVal);
	state_fio->StateValue(cpustate->TF);
	state_fio->StateValue(cpustate->IF);
	state_fio->StateValue(cpustate->MF);
	state_fio->StateValue(cpustate->MF_WriteDisabled);
	state_fio->StateValue(cpustate->NF);
	state_fio->StateValue(cpustate->int_vector);
	state_fio->StateValue(cpustate->nmi_state);
	state_fio->StateValue(cpustate->irq_state);
	state_fio->StateValue(cpustate->test_state);
	state_fio->StateValue(cpustate->rep_in_progress);
	state_fio->StateValue(cpustate->extra_cycles);
	state_fio->StateValue(cpustate->halted);
	state_fio->StateValue(cpustate->busreq);
	state_fio->StateValue(cpustate->ip);
	state_fio->StateValue(cpustate->sp);
#ifdef USE_DEBUGGER
	state_fio->StateValue(cpustate->total_icount);
#endif
	state_fio->StateValue(cpustate->icount);
	state_fio->StateValue(cpustate->seg_prefix);
	state_fio->StateValue(cpustate->prefix_seg);
	state_fio->StateValue(cpustate->ea);
	state_fio->StateValue(cpustate->eo);
	state_fio->StateValue(cpustate->ea_seg);
	
#ifdef USE_DEBUGGER
	// post process
	if(loading) {
		cpustate->prev_total_icount = cpustate->total_icount;
	}
#endif
	return true;
}

