/*
	Skelton for retropc emulator

	Origin : MAME i386 core
	Author : Takeda.Toshiya
	Date  : 2009.06.08-

	[ i386/i486/Pentium/MediaGX ]
*/

#include "i386.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif

/* ----------------------------------------------------------------------------
	MAME i386
---------------------------------------------------------------------------- */

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4018 )
#pragma warning( disable : 4065 )
#pragma warning( disable : 4146 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4996 )
#endif

#if defined(HAS_I386)
	#define CPU_MODEL i386
#elif defined(HAS_I486)
	#define CPU_MODEL i486
#elif defined(HAS_PENTIUM)
	#define CPU_MODEL pentium
#elif defined(HAS_MEDIAGX)
	#define CPU_MODEL mediagx
#elif defined(HAS_PENTIUM_PRO)
	#define CPU_MODEL pentium_pro
#elif defined(HAS_PENTIUM_MMX)
	#define CPU_MODEL pentium_mmx
#elif defined(HAS_PENTIUM2)
	#define CPU_MODEL pentium2
#elif defined(HAS_PENTIUM3)
	#define CPU_MODEL pentium3
#elif defined(HAS_PENTIUM4)
	#define CPU_MODEL pentium4
#endif

#ifndef __BIG_ENDIAN__
#define LSB_FIRST
#endif

#ifndef INLINE
#define INLINE inline
#endif

#define U64(v) UINT64(v)

#define fatalerror(...) exit(1)
#define logerror(...)
#define popmessage(...)

/*****************************************************************************/
/* src/emu/devcpu.h */

// CPU interface functions
#define CPU_INIT_NAME(name)			cpu_init_##name
#define CPU_INIT(name)				void* CPU_INIT_NAME(name)()
#define CPU_INIT_CALL(name)			CPU_INIT_NAME(name)()

#define CPU_RESET_NAME(name)			cpu_reset_##name
#define CPU_RESET(name)				void CPU_RESET_NAME(name)(i386_state *cpustate)
#define CPU_RESET_CALL(name)			CPU_RESET_NAME(name)(cpustate)

#define CPU_EXECUTE_NAME(name)			cpu_execute_##name
#define CPU_EXECUTE(name)			int CPU_EXECUTE_NAME(name)(i386_state *cpustate, int cycles)
#define CPU_EXECUTE_CALL(name)			CPU_EXECUTE_NAME(name)(cpustate, cycles)

#define CPU_TRANSLATE_NAME(name)		cpu_translate_##name
#define CPU_TRANSLATE(name)			int CPU_TRANSLATE_NAME(name)(void *cpudevice, address_spacenum space, int intention, offs_t *address)
#define CPU_TRANSLATE_CALL(name)		CPU_TRANSLATE_NAME(name)(cpudevice, space, intention, address)

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
/* src/emu/dimemory.h */

// Translation intentions
const int TRANSLATE_TYPE_MASK       = 0x03;     // read write or fetch
const int TRANSLATE_USER_MASK       = 0x04;     // user mode or fully privileged
const int TRANSLATE_DEBUG_MASK      = 0x08;     // debug mode (no side effects)

const int TRANSLATE_READ            = 0;        // translate for read
const int TRANSLATE_WRITE           = 1;        // translate for write
const int TRANSLATE_FETCH           = 2;        // translate for instruction fetch
const int TRANSLATE_READ_USER       = (TRANSLATE_READ | TRANSLATE_USER_MASK);
const int TRANSLATE_WRITE_USER      = (TRANSLATE_WRITE | TRANSLATE_USER_MASK);
const int TRANSLATE_FETCH_USER      = (TRANSLATE_FETCH | TRANSLATE_USER_MASK);
const int TRANSLATE_READ_DEBUG      = (TRANSLATE_READ | TRANSLATE_DEBUG_MASK);
const int TRANSLATE_WRITE_DEBUG     = (TRANSLATE_WRITE | TRANSLATE_DEBUG_MASK);
const int TRANSLATE_FETCH_DEBUG     = (TRANSLATE_FETCH | TRANSLATE_DEBUG_MASK);

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

// address spaces
enum address_spacenum
{
	AS_0,                           // first address space
	AS_1,                           // second address space
	AS_2,                           // third address space
	AS_3,                           // fourth address space
	ADDRESS_SPACES,                 // maximum number of address spaces

	// alternate address space names for common use
	AS_PROGRAM = AS_0,              // program address space
	AS_DATA = AS_1,                 // data address space
	AS_IO = AS_2                    // I/O address space
};

// offsets and addresses are 32-bit (for now...)
typedef UINT32	offs_t;

/*****************************************************************************/
/* src/osd/osdcomm.h */

/* Highly useful macro for compile-time knowledge of an array size */
#define ARRAY_LENGTH(x)     (sizeof(x) / sizeof(x[0]))

#ifdef I386_BIOS_CALL
#define BIOS_INT(num) if(cpustate->bios != NULL) { \
	uint16 regs[8], sregs[4]; \
	regs[0] = REG16(AX); regs[1] = REG16(CX); regs[2] = REG16(DX); regs[3] = REG16(BX); \
	regs[4] = REG16(SP); regs[5] = REG16(BP); regs[6] = REG16(SI); regs[7] = REG16(DI); \
	sregs[0] = cpustate->sreg[ES].selector; sregs[1] = cpustate->sreg[CS].selector; \
	sregs[2] = cpustate->sreg[SS].selector; sregs[3] = cpustate->sreg[DS].selector; \
	int32 ZeroFlag = cpustate->ZF, CarryFlag = cpustate->CF; \
	if(cpustate->bios->bios_int(num, regs, sregs, &ZeroFlag, &CarryFlag)) { \
		REG16(AX) = regs[0]; REG16(CX) = regs[1]; REG16(DX) = regs[2]; REG16(BX) = regs[3]; \
		REG16(SP) = regs[4]; REG16(BP) = regs[5]; REG16(SI) = regs[6]; REG16(DI) = regs[7]; \
		cpustate->ZF = (UINT8)ZeroFlag; cpustate->CF = (UINT8)CarryFlag; \
		return; \
	} \
}
#define BIOS_CALL(address) if(cpustate->bios != NULL) { \
	uint16 regs[8], sregs[4]; \
	regs[0] = REG16(AX); regs[1] = REG16(CX); regs[2] = REG16(DX); regs[3] = REG16(BX); \
	regs[4] = REG16(SP); regs[5] = REG16(BP); regs[6] = REG16(SI); regs[7] = REG16(DI); \
	sregs[0] = cpustate->sreg[ES].selector; sregs[1] = cpustate->sreg[CS].selector; \
	sregs[2] = cpustate->sreg[SS].selector; sregs[3] = cpustate->sreg[DS].selector; \
	int32 ZeroFlag = cpustate->ZF, CarryFlag = cpustate->CF; \
	if(cpustate->bios->bios_call(address, regs, sregs, &ZeroFlag, &CarryFlag)) { \
		REG16(AX) = regs[0]; REG16(CX) = regs[1]; REG16(DX) = regs[2]; REG16(BX) = regs[3]; \
		REG16(SP) = regs[4]; REG16(BP) = regs[5]; REG16(SI) = regs[6]; REG16(DI) = regs[7]; \
		cpustate->ZF = (UINT8)ZeroFlag; cpustate->CF = (UINT8)CarryFlag; \
		return; \
	} \
}
#endif

static CPU_TRANSLATE(i386);

#include "mame/lib/softfloat/softfloat.c"
#include "mame/emu/cpu/vtlb.c"
#include "mame/emu/cpu/i386/i386.c"
#ifdef USE_DEBUGGER
#include "mame/emu/cpu/i386/i386dasm.c"
#endif

void I386::initialize()
{
	opaque = CPU_INIT_CALL(CPU_MODEL);
	
	i386_state *cpustate = (i386_state *)opaque;
	cpustate->pic = d_pic;
	cpustate->program = d_mem;
	cpustate->io = d_io;
#ifdef I386_BIOS_CALL
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
	cpustate->shutdown = 0;
}

void I386::release()
{
	i386_state *cpustate = (i386_state *)opaque;
	vtlb_free(cpustate->vtlb);
	free(opaque);
}

void I386::reset()
{
	i386_state *cpustate = (i386_state *)opaque;
	CPU_RESET_CALL(CPU_MODEL);
}

int I386::run(int cycles)
{
	i386_state *cpustate = (i386_state *)opaque;
	return CPU_EXECUTE_CALL(i386);
}

void I386::write_signal(int id, uint32 data, uint32 mask)
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

void I386::set_intr_line(bool line, bool pending, uint32 bit)
{
	i386_state *cpustate = (i386_state *)opaque;
	i386_set_irq_line(cpustate, INPUT_LINE_IRQ, line ? HOLD_LINE : CLEAR_LINE);
}

void I386::set_extra_clock(int cycles)
{
	i386_state *cpustate = (i386_state *)opaque;
	cpustate->extra_cycles += cycles;
}

int I386::get_extra_clock()
{
	i386_state *cpustate = (i386_state *)opaque;
	return cpustate->extra_cycles;
}

uint32 I386::get_pc()
{
	i386_state *cpustate = (i386_state *)opaque;
	return cpustate->prev_pc;
}

uint32 I386::get_next_pc()
{
	i386_state *cpustate = (i386_state *)opaque;
	return cpustate->pc;
}

#ifdef USE_DEBUGGER
void I386::debug_write_data8(uint32 addr, uint32 data)
{
	int wait;
	d_mem->write_data8w(addr, data, &wait);
}

uint32 I386::debug_read_data8(uint32 addr)
{
	int wait;
	return d_mem->read_data8w(addr, &wait);
}

void I386::debug_write_data16(uint32 addr, uint32 data)
{
	int wait;
	d_mem->write_data16w(addr, data, &wait);
}

uint32 I386::debug_read_data16(uint32 addr)
{
	int wait;
	return d_mem->read_data16w(addr, &wait);
}

void I386::debug_write_data32(uint32 addr, uint32 data)
{
	int wait;
	d_mem->write_data32w(addr, data, &wait);
}

uint32 I386::debug_read_data32(uint32 addr)
{
	int wait;
	return d_mem->read_data32w(addr, &wait);
}

void I386::debug_write_io8(uint32 addr, uint32 data)
{
	int wait;
	d_io->write_io8w(addr, data, &wait);
}

uint32 I386::debug_read_io8(uint32 addr) {
	int wait;
	return d_io->read_io8w(addr, &wait);
}

void I386::debug_write_io16(uint32 addr, uint32 data)
{
	int wait;
	d_io->write_io16w(addr, data, &wait);
}

uint32 I386::debug_read_io16(uint32 addr) {
	int wait;
	return d_io->read_io16w(addr, &wait);
}

void I386::debug_write_io32(uint32 addr, uint32 data)
{
	int wait;
	d_io->write_io32w(addr, data, &wait);
}

uint32 I386::debug_read_io32(uint32 addr) {
	int wait;
	return d_io->read_io32w(addr, &wait);
}

bool I386::debug_write_reg(_TCHAR *reg, uint32 data)
{
	i386_state *cpustate = (i386_state *)opaque;
	if(_tcsicmp(reg, _T("IP")) == 0) {
		cpustate->eip = data & 0xffff;
		CHANGE_PC(cpustate, cpustate->eip);
	} else if(_tcsicmp(reg, _T("AX")) == 0) {
		REG16(AX) = data;
	} else if(_tcsicmp(reg, _T("BX")) == 0) {
		REG16(BX) = data;
	} else if(_tcsicmp(reg, _T("CX")) == 0) {
		REG16(CX) = data;
	} else if(_tcsicmp(reg, _T("DX")) == 0) {
		REG16(DX) = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		REG16(SP) = data;
	} else if(_tcsicmp(reg, _T("BP")) == 0) {
		REG16(BP) = data;
	} else if(_tcsicmp(reg, _T("SI")) == 0) {
		REG16(SI) = data;
	} else if(_tcsicmp(reg, _T("DI")) == 0) {
		REG16(DI) = data;
	} else if(_tcsicmp(reg, _T("AL")) == 0) {
		REG8(AL) = data;
	} else if(_tcsicmp(reg, _T("AH")) == 0) {
		REG8(AH) = data;
	} else if(_tcsicmp(reg, _T("BL")) == 0) {
		REG8(BL) = data;
	} else if(_tcsicmp(reg, _T("BH")) == 0) {
		REG8(BH) = data;
	} else if(_tcsicmp(reg, _T("CL")) == 0) {
		REG8(CL) = data;
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		REG8(CH) = data;
	} else if(_tcsicmp(reg, _T("DL")) == 0) {
		REG8(DL) = data;
	} else if(_tcsicmp(reg, _T("DH")) == 0) {
		REG8(DH) = data;
	} else {
		return false;
	}
	return false;
}

void I386::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	i386_state *cpustate = (i386_state *)opaque;
	_stprintf_s(buffer, buffer_len,
	_T("AX=%04X  BX=%04X CX=%04X DX=%04X SP=%04X  BP=%04X  SI=%04X  DI=%04X\nDS=%04X  ES=%04X SS=%04X CS=%04X IP=%04X  FLAG=[%c%c%c%c%c%c%c%c%c]"),
	REG16(AX), REG16(BX), REG16(CX), REG16(DX), REG16(SP), REG16(BP), REG16(SI), REG16(DI),
	cpustate->sreg[DS].selector, cpustate->sreg[ES].selector, cpustate->sreg[SS].selector, cpustate->sreg[CS].selector, cpustate->eip,
	cpustate->OF ? _T('O') : _T('-'), cpustate->DF ? _T('D') : _T('-'), cpustate->IF ? _T('I') : _T('-'), cpustate->TF ? _T('T') : _T('-'),
	cpustate->SF ? _T('S') : _T('-'), cpustate->ZF ? _T('Z') : _T('-'), cpustate->AF ? _T('A') : _T('-'), cpustate->PF ? _T('P') : _T('-'), cpustate->CF ? _T('C') : _T('-'));
}

int I386::debug_dasm(uint32 pc, _TCHAR *buffer, size_t buffer_len)
{
	i386_state *cpustate = (i386_state *)opaque;
	UINT64 eip = cpustate->eip;
	UINT8 ops[16];
	for(int i = 0; i < 16; i++) {
		int wait;
		ops[i] = d_mem->read_data8w(pc + i, &wait);
	}
	UINT8 *oprom = ops;
	
	if(cpustate->operand_size) {
		return CPU_DISASSEMBLE_CALL(x86_32) & DASMFLAG_LENGTHMASK;
	} else {
		return CPU_DISASSEMBLE_CALL(x86_16) & DASMFLAG_LENGTHMASK;
	}
}
#endif

void I386::set_address_mask(uint32 mask)
{
	i386_state *cpustate = (i386_state *)opaque;
	cpustate->a20_mask = mask;
}

uint32 I386::get_address_mask()
{
	i386_state *cpustate = (i386_state *)opaque;
	return cpustate->a20_mask;
}

void I386::set_shutdown_flag(int shutdown)
{
	i386_state *cpustate = (i386_state *)opaque;
	cpustate->shutdown = shutdown;
}

int I386::get_shutdown_flag()
{
	i386_state *cpustate = (i386_state *)opaque;
	return cpustate->shutdown;
}
