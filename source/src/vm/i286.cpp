/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date  : 2012.10.18-

	[ i286 ]
*/

#include "i286.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif
#include "../fileio.h"

/* ----------------------------------------------------------------------------
	MAME i286
---------------------------------------------------------------------------- */

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4018 )
#pragma warning( disable : 4146 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4996 )
#endif

#if defined(HAS_I86)
	#define CPU_MODEL i8086
#elif defined(HAS_I88)
	#define CPU_MODEL i8088
#elif defined(HAS_I186)
	#define CPU_MODEL i80186
#elif defined(HAS_I286)
	#define CPU_MODEL i80286
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

#define CPU_DISASSEMBLE_NAME(name)		cpu_disassemble_##name
#define CPU_DISASSEMBLE(name)			int CPU_DISASSEMBLE_NAME(name)(char *buffer, offs_t eip, const UINT8 *oprom)
#define CPU_DISASSEMBLE_CALL(name)		CPU_DISASSEMBLE_NAME(name)(buffer, eip, oprom)

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

/*****************************************************************************/
/* src/osd/osdcomm.h */

/* Highly useful macro for compile-time knowledge of an array size */
#define ARRAY_LENGTH(x)     (sizeof(x) / sizeof(x[0]))

#if defined(HAS_I86) || defined(HAS_I88) || defined(HAS_I186)
#define cpu_state i8086_state
#include "mame/emu/cpu/i86/i86.c"
#elif defined(HAS_I286)
#define cpu_state i80286_state
#include "mame/emu/cpu/i86/i286.c"
#endif
#ifdef USE_DEBUGGER
#include "mame/emu/cpu/i386/i386dasm.c"
#endif

void I286::initialize()
{
	opaque = CPU_INIT_CALL(CPU_MODEL);
	
	cpu_state *cpustate = (cpu_state *)opaque;
	cpustate->pic = d_pic;
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef I86_BIOS_CALL
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

void I286::release()
{
	free(opaque);
}

void I286::reset()
{
	cpu_state *cpustate = (cpu_state *)opaque;
	int busreq = cpustate->busreq;
	
	CPU_RESET_CALL(CPU_MODEL);
	
	cpustate->pic = d_pic;
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef I86_BIOS_CALL
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

int I286::run(int icount)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	return CPU_EXECUTE_CALL(CPU_MODEL);
}

void I286::write_signal(int id, uint32 data, uint32 mask)
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
#ifdef HAS_I286
	} else if(id == SIG_I86_A20) {
		i80286_set_a20_line(cpustate, data & mask);
#endif
	}
}

void I286::set_intr_line(bool line, bool pending, uint32 bit)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	set_irq_line(cpustate, INPUT_LINE_IRQ, line ? HOLD_LINE : CLEAR_LINE);
}

void I286::set_extra_clock(int icount)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	cpustate->extra_cycles += icount;
}

int I286::get_extra_clock()
{
	cpu_state *cpustate = (cpu_state *)opaque;
	return cpustate->extra_cycles;
}

uint32 I286::get_pc()
{
	cpu_state *cpustate = (cpu_state *)opaque;
	return cpustate->prevpc;
}

uint32 I286::get_next_pc()
{
	cpu_state *cpustate = (cpu_state *)opaque;
	return cpustate->pc;
}

#ifdef USE_DEBUGGER
void I286::debug_write_data8(uint32 addr, uint32 data)
{
	int wait;
	d_mem->write_data8w(addr, data, &wait);
}

uint32 I286::debug_read_data8(uint32 addr)
{
	int wait;
	return d_mem->read_data8w(addr, &wait);
}

void I286::debug_write_data16(uint32 addr, uint32 data)
{
	int wait;
	d_mem->write_data16w(addr, data, &wait);
}

uint32 I286::debug_read_data16(uint32 addr)
{
	int wait;
	return d_mem->read_data16w(addr, &wait);
}

void I286::debug_write_io8(uint32 addr, uint32 data)
{
	int wait;
	d_io->write_io8w(addr, data, &wait);
}

uint32 I286::debug_read_io8(uint32 addr) {
	int wait;
	return d_io->read_io8w(addr, &wait);
}

void I286::debug_write_io16(uint32 addr, uint32 data)
{
	int wait;
	d_io->write_io16w(addr, data, &wait);
}

uint32 I286::debug_read_io16(uint32 addr) {
	int wait;
	return d_io->read_io16w(addr, &wait);
}

bool I286::debug_write_reg(_TCHAR *reg, uint32 data)
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

void I286::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	_stprintf_s(buffer, buffer_len,
	_T("AX=%04X  BX=%04X CX=%04X DX=%04X SP=%04X  BP=%04X  SI=%04X  DI=%04X\nDS=%04X  ES=%04X SS=%04X CS=%04X IP=%04X  FLAG=[%c%c%c%c%c%c%c%c%c]"),
	cpustate->regs.w[AX], cpustate->regs.w[BX], cpustate->regs.w[CX], cpustate->regs.w[DX], cpustate->regs.w[SP], cpustate->regs.w[BP], cpustate->regs.w[SI], cpustate->regs.w[DI],
	cpustate->sregs[DS], cpustate->sregs[ES], cpustate->sregs[SS], cpustate->sregs[CS], cpustate->pc - cpustate->base[CS],
	OF ? _T('O') : _T('-'), DF ? _T('D') : _T('-'), cpustate->IF ? _T('I') : _T('-'), cpustate->TF ? _T('T') : _T('-'),
	SF ? _T('S') : _T('-'), ZF ? _T('Z') : _T('-'), AF ? _T('A') : _T('-'), PF ? _T('P') : _T('-'), CF ? _T('C') : _T('-'));
}

int I286::debug_dasm(uint32 pc, _TCHAR *buffer, size_t buffer_len)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	UINT64 eip = pc - cpustate->base[CS];
	UINT8 ops[16];
	for(int i = 0; i < 16; i++) {
		int wait;
		ops[i] = d_mem->read_data8w(pc + i, &wait);
	}
	UINT8 *oprom = ops;
	
	return CPU_DISASSEMBLE_CALL(x86_16) & DASMFLAG_LENGTHMASK;
}
#endif

#ifdef HAS_I286
void I286::set_address_mask(uint32 mask)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	cpustate->amask = mask;
}

uint32 I286::get_address_mask()
{
	cpu_state *cpustate = (cpu_state *)opaque;
	return cpustate->amask;
}

void I286::set_shutdown_flag(int shutdown)
{
	cpu_state *cpustate = (cpu_state *)opaque;
	cpustate->shutdown = shutdown;
}

int I286::get_shutdown_flag()
{
	cpu_state *cpustate = (cpu_state *)opaque;
	return cpustate->shutdown;
}
#endif


#define STATE_VERSION	1

void I286::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(opaque, sizeof(cpu_state), 1);
}

bool I286::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(opaque, sizeof(cpu_state), 1);
	
	cpu_state *cpustate = (cpu_state *)opaque;
	cpustate->pic = d_pic;
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef I86_BIOS_CALL
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
	return true;
}

