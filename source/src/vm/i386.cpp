/*
	Skelton for retropc emulator

	Origin : MAME i386 core
	Author : Takeda.Toshiya
	Date   : 2009.06.08-

	[ i386/i486/Pentium/MediaGX ]
*/

#include "i386.h"
//#ifdef USE_DEBUGGER
#include "debugger.h"
#include "i386_dasm.h"
//#endif

/* ----------------------------------------------------------------------------
	MAME i386
---------------------------------------------------------------------------- */
// Note:
// API of bios_int_i86() / bios_caii_i86() has changed.
// regs[8] regs[9] are added.These entries set redirect-address by PSEUDO-BIOS.
// If need, will add more entries for cycle#.
// - 20181126 K.O

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4018 )
#pragma warning( disable : 4065 )
#pragma warning( disable : 4146 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4996 )
#endif

#if 0
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
#endif

#ifndef __BIG_ENDIAN__
#define LSB_FIRST
#endif

#ifndef INLINE
#define INLINE inline
#endif

#define U64(v) UINT64(v)

#define fatalerror(...) exit(1)

#define logdebug(...)						\
	{ \
	  if(cpustate != NULL) {									  \
		  if(cpustate->parent_device != NULL) {							\
			  cpustate->parent_device->out_debug_log(__VA_ARGS__); \
		  }																\
	  }																	\
	}

#define loginfo(...)						\
	{ \
	  if(cpustate != NULL) {									  \
		  if(cpustate->parent_device != NULL) {							\
			  cpustate->parent_device->out_debug_log(__VA_ARGS__); \
		  }																\
	  }																	\
	}

#if 0
#define logerror(...)
#else

#define logerror(...)						\
	{ \
	  if(cpustate != NULL) {									  \
		  if(cpustate->parent_device != NULL) {							\
			  cpustate->parent_device->out_debug_log(__VA_ARGS__); \
		  }																\
	  }																	\
	}

#endif
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
#define CPU_EXECUTE(name)			int __FASTCALL CPU_EXECUTE_NAME(name)(i386_state *cpustate, int cycles)
#define CPU_EXECUTE_CALL(name)			CPU_EXECUTE_NAME(name)(cpustate, cycles)

#define CPU_TRANSLATE_NAME(name)		cpu_translate_##name
#define CPU_TRANSLATE(name)			int CPU_TRANSLATE_NAME(name)(void *cpudevice, address_spacenum space, int intention, offs_t *address)
#define CPU_TRANSLATE_CALL(name)		CPU_TRANSLATE_NAME(name)(cpudevice, space, intention, address)

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
/* src/emu/emumem.h */

// helpers for checking address alignment
#define WORD_ALIGNED(a)                 (((a) & 1) == 0)
#define DWORD_ALIGNED(a)                (((a) & 3) == 0)
#define QWORD_ALIGNED(a)                (((a) & 7) == 0)

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


////#ifdef I386_PSEUDO_BIOS
//#ifdef I86_PSEUDO_BIOS
#define BIOS_INT(num) if(cpustate->bios != NULL) { \
		/*if(((cpustate->cr[0] & 0x0001) == 0) || (cpustate->VM != 0)) */{	/* VM8086 or Not Protected */ \
			uint16_t regs[10], sregs[4];								\
			regs[0] = REG16(AX); regs[1] = REG16(CX); regs[2] = REG16(DX); regs[3] = REG16(BX); \
			regs[4] = REG16(SP); regs[5] = REG16(BP); regs[6] = REG16(SI); regs[7] = REG16(DI); \
			regs[8] = 0x0000; regs[9] = 0x0000;							\
			sregs[0] = cpustate->sreg[ES].selector; sregs[1] = cpustate->sreg[CS].selector; \
			sregs[2] = cpustate->sreg[SS].selector; sregs[3] = cpustate->sreg[DS].selector; \
			int32_t ZeroFlag = cpustate->ZF, CarryFlag = cpustate->CF;	\
if(cpustate->bios->bios_int_i86(num, regs, sregs, &ZeroFlag, &CarryFlag, &(cpustate->cycles), &(cpustate->total_cycles))) { \
				REG16(AX) = regs[0]; REG16(CX) = regs[1]; REG16(DX) = regs[2]; REG16(BX) = regs[3]; \
				REG16(SP) = regs[4]; REG16(BP) = regs[5]; REG16(SI) = regs[6]; REG16(DI) = regs[7]; \
				cpustate->ZF = (UINT8)ZeroFlag; cpustate->CF = (UINT8)CarryFlag; \
				CYCLES(cpustate,CYCLES_IRET);							\
				if((regs[8] != 0x0000) || (regs[9] != 0x0000)) {		\
					uint32_t hi = regs[9];								\
					uint32_t lo = regs[8];								\
					uint32_t addr = (hi << 16) | lo;					\
					cpustate->eip = addr;								\
				}														\
				return;													\
			}															\
		}																\
	}

#define BIOS_INT32(num) if(cpustate->bios != NULL) { \
		/*if(((cpustate->cr[0] & 0x0001) == 0) || (cpustate->VM != 0)) */{	/* VM8086 or Not Protected */ \
			uint32_t regs[10];											\
			uint16_t sregs[4];												\
			regs[0] = REG32(EAX); regs[1] = REG32(ECX); regs[2] = REG32(EDX); regs[3] = REG32(EBX); \
			regs[4] = REG32(ESP); regs[5] = REG32(EBP); regs[6] = REG32(ESI); regs[7] = REG32(EDI); \
			regs[8] = 0x0000; regs[9] = 0x0000;							\
			sregs[0] = cpustate->sreg[ES].selector; sregs[1] = cpustate->sreg[CS].selector; \
			sregs[2] = cpustate->sreg[SS].selector; sregs[3] = cpustate->sreg[DS].selector; \
			int32_t ZeroFlag = cpustate->ZF, CarryFlag = cpustate->CF;	\
if(cpustate->bios->bios_int_ia32(num, regs, sregs, &ZeroFlag, &CarryFlag, &(cpustate->cycles), &(cpustate->total_cycles))) { \
				REG32(EAX) = regs[0]; REG32(ECX) = regs[1]; REG32(EDX) = regs[2]; REG32(EBX) = regs[3]; \
				REG32(ESP) = regs[4]; REG32(EBP) = regs[5]; REG32(ESI) = regs[6]; REG32(EDI) = regs[7]; \
				cpustate->ZF = (UINT8)ZeroFlag; cpustate->CF = (UINT8)CarryFlag; \
				CYCLES(cpustate,CYCLES_IRET);							\
				if((regs[8] != 0x0000) || (regs[9] != 0x0000)) {		\
					uint32_t hi = regs[9];								\
					uint32_t lo = regs[8];								\
					uint32_t addr = (hi << 16) | lo;					\
					cpustate->eip = addr;								\
				}														\
				return;													\
			}															\
		}																\
	}

#define BIOS_CALL_FAR(address) if(cpustate->bios != NULL) {				\
		if(((cpustate->cr[0] & 0x0001) == 0) || (cpustate->VM != 0)) {	/* VM8086 or Not Protected */ \
			uint16_t regs[10], sregs[4];								\
			regs[0] = REG16(AX); regs[1] = REG16(CX); regs[2] = REG16(DX); regs[3] = REG16(BX); \
			regs[4] = REG16(SP); regs[5] = REG16(BP); regs[6] = REG16(SI); regs[7] = REG16(DI); \
			regs[8] = 0x0000; regs[9] = 0x0000;							\
			sregs[0] = cpustate->sreg[ES].selector; sregs[1] = cpustate->sreg[CS].selector; \
			sregs[2] = cpustate->sreg[SS].selector; sregs[3] = cpustate->sreg[DS].selector; \
			int32_t ZeroFlag = cpustate->ZF, CarryFlag = cpustate->CF;	\
			if(cpustate->bios->bios_call_far_i86(address, regs, sregs, &ZeroFlag, &CarryFlag, &(cpustate->cycles), &(cpustate->total_cycles))) { \
				REG16(AX) = regs[0]; REG16(CX) = regs[1]; REG16(DX) = regs[2]; REG16(BX) = regs[3]; \
				REG16(SP) = regs[4]; REG16(BP) = regs[5]; REG16(SI) = regs[6]; REG16(DI) = regs[7]; \
				cpustate->ZF = (UINT8)ZeroFlag; cpustate->CF = (UINT8)CarryFlag; \
				CYCLES(cpustate,CYCLES_RET_INTERSEG);					\
				if((regs[8] != 0x0000) || (regs[9] != 0x0000)) {		\
					uint32_t hi = regs[9];								\
					uint32_t lo = regs[8];								\
					uint32_t addr = (hi << 16) | lo;					\
					cpustate->eip = addr;								\
				}														\
				return;													\
			}															\
		}																\
	}

#define BIOS_CALL_FAR32(address) if(cpustate->bios != NULL) {				\
		if(((cpustate->cr[0] & 0x0001) == 0) || (cpustate->VM != 0)) {	/* VM8086 or Not Protected */ \
			uint32_t regs[10];											\
			uint16_t sregs[4];													\
			regs[0] = REG32(EAX); regs[1] = REG32(ECX); regs[2] = REG32(EDX); regs[3] = REG32(EBX); \
			regs[4] = REG32(ESP); regs[5] = REG32(EBP); regs[6] = REG32(ESI); regs[7] = REG32(EDI); \
			regs[8] = 0x0000; regs[9] = 0x0000;							\
			sregs[0] = cpustate->sreg[ES].selector; sregs[1] = cpustate->sreg[CS].selector; \
			sregs[2] = cpustate->sreg[SS].selector; sregs[3] = cpustate->sreg[DS].selector; \
			int32_t ZeroFlag = cpustate->ZF, CarryFlag = cpustate->CF;	\
			if(cpustate->bios->bios_call_far_ia32(address, regs, sregs, &ZeroFlag, &CarryFlag, &(cpustate->cycles), &(cpustate->total_cycles))) { \
				REG32(EAX) = regs[0]; REG32(ECX) = regs[1]; REG32(EDX) = regs[2]; REG32(EBX) = regs[3]; \
				REG32(ESP) = regs[4]; REG32(EBP) = regs[5]; REG32(ESI) = regs[6]; REG32(EDI) = regs[7]; \
				cpustate->ZF = (UINT8)ZeroFlag; cpustate->CF = (UINT8)CarryFlag; \
				CYCLES(cpustate,CYCLES_RET_INTERSEG);					\
				if((regs[8] != 0x0000) || (regs[9] != 0x0000)) {		\
					uint32_t hi = regs[9];								\
					uint32_t lo = regs[8];								\
					uint32_t addr = (hi << 16) | lo;					\
					cpustate->eip = addr;								\
				}														\
				return;													\
			}															\
		}																\
	}

//#endif

static CPU_TRANSLATE(i386);
void terminate()
{
	printf("WARN: unexpected exception\n");
}

#include "mame/lib/softfloat/softfloat.c"
#include "mame/lib/softfloat/fsincos.c"
#include "mame/emu/cpu/vtlb.c"
#include "mame/emu/cpu/i386/i386.c"

void I386::initialize()
{
	DEVICE::initialize();
	uint32_t n_cpu_type = N_CPU_TYPE_I386;
	if(osd->check_feature("HAS_I386")) {
		n_cpu_type = N_CPU_TYPE_I386;
	} else if(osd->check_feature("HAS_I486")) {
		n_cpu_type = N_CPU_TYPE_I486;
	} else if(osd->check_feature("HAS_PENTIUM")) {
		n_cpu_type = N_CPU_TYPE_PENTIUM;
	} else if(osd->check_feature("HAS_MEDIAGX")) {
		n_cpu_type = N_CPU_TYPE_MEDIAGX;
	} else if(osd->check_feature("HAS_PENTIUM_PRO")) {
		n_cpu_type = N_CPU_TYPE_PENTIUM_PRO;
	} else if(osd->check_feature("HAS_PENTIUM_MMX")) {
		n_cpu_type = N_CPU_TYPE_PENTIUM_MMX;
	} else if(osd->check_feature("HAS_PENTIUM2")) {
		n_cpu_type = N_CPU_TYPE_PENTIUM2;
	} else if(osd->check_feature("HAS_PENTIUM3")) {
		n_cpu_type = N_CPU_TYPE_PENTIUM3;
	} else if(osd->check_feature("HAS_PENTIUM4")) {
		n_cpu_type = N_CPU_TYPE_PENTIUM4;
	}
	switch(n_cpu_type) {
	case N_CPU_TYPE_I386:
		set_device_name(_T("i80386 CPU"));
		opaque = CPU_INIT_CALL( i386 );
		break;
	case N_CPU_TYPE_I486:
		set_device_name(_T("i80486 CPU"));
		opaque = CPU_INIT_CALL( i486 );
		break;
	case N_CPU_TYPE_PENTIUM:
		set_device_name(_T("Pentium CPU"));
		opaque = CPU_INIT_CALL( pentium );
		break;
	case N_CPU_TYPE_MEDIAGX:
		set_device_name(_T("Media GX CPU"));
		opaque = CPU_INIT_CALL( mediagx );
		break;
	case N_CPU_TYPE_PENTIUM_PRO:
		set_device_name(_T("Pentium PRO CPU"));
		opaque = CPU_INIT_CALL( pentium_pro );
		break;
	case N_CPU_TYPE_PENTIUM_MMX:
		set_device_name(_T("Pentium MMX CPU"));
		opaque = CPU_INIT_CALL( pentium_mmx );
		break;
	case N_CPU_TYPE_PENTIUM2:
		set_device_name(_T("Pentium2 CPU"));
		opaque = CPU_INIT_CALL( pentium2 );
		break;
	case N_CPU_TYPE_PENTIUM3:
		set_device_name(_T("Pentium3 CPU"));
		opaque = CPU_INIT_CALL( pentium3 );
		break;
	case N_CPU_TYPE_PENTIUM4:
		set_device_name(_T("Pentium4 CPU"));
		opaque = CPU_INIT_CALL( pentium4 );
		break;
	default: // ???
		set_device_name(_T("i80386 CPU"));
		opaque = CPU_INIT_CALL( i386 );
		break;
	}
	
	i386_state *cpustate = (i386_state *)opaque;
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
	
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
//#endif
	cpustate->parent_device = this; // This aims to log.
	cpustate->cpu_type = n_cpu_type; // check cpu type
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
	logerror(_T("I386::reset()"));
	cpu_reset_generic(cpustate);
	write_signals(&outputs_extreset, 0xffffffff);
}

int I386::run(int cycles)
{	i386_state *cpustate = (i386_state *)opaque;
	return CPU_EXECUTE_CALL(i386); // OK?
}

uint32_t I386::read_signal(int id)
{
	if((id == SIG_CPU_TOTAL_CYCLE_HI) || (id == SIG_CPU_TOTAL_CYCLE_LO)) {
		i386_state *cpustate = (i386_state *)opaque;
		pair64_t n;
		if(cpustate != NULL) {
			n.q = cpustate->total_cycles;
		} else {
			n.q = 0;
		}
		if(id == SIG_CPU_TOTAL_CYCLE_HI) {
			return n.d.h;
		} else {
			return n.d.l;
		}
	} else if(id == SIG_I386_A20) {
		i386_state *cpustate = (i386_state *)opaque;
		return cpustate->a20_mask;
	}
	return 0;
}

void I386::write_signal(int id, uint32_t data, uint32_t mask)
{
	i386_state *cpustate = (i386_state *)opaque;
	
	if(id == SIG_CPU_NMI) {
		i386_set_irq_line(cpustate, INPUT_LINE_NMI, (data & mask) ? HOLD_LINE : CLEAR_LINE);
	} else if(id == SIG_CPU_IRQ) {
		i386_set_irq_line(cpustate, INPUT_LINE_IRQ, (data & mask) ? HOLD_LINE : CLEAR_LINE);
	} else if(id == SIG_CPU_BUSREQ) {
		cpustate->busreq = (data & mask) ? 1 : 0;
	} else if(id == SIG_CPU_HALTREQ) {
		cpustate->haltreq = (data & mask) ? 1 : 0;
	} else if(id == SIG_I386_A20) {
		i386_set_a20_line(cpustate, data & mask);
	} else if(id == SIG_I386_NOTIFY_RESET) {
		write_signals(&outputs_extreset, (((data & mask) == 0) ? 0x00000000 : 0xffffffff));
	} else if(id == SIG_CPU_WAIT_FACTOR) {
		cpustate->waitfactor = data; // 65536.
		cpustate->waitcount = 0; // 65536.
	}
}

void I386::set_intr_line(bool line, bool pending, uint32_t bit)
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

uint32_t I386::get_pc()
{
	i386_state *cpustate = (i386_state *)opaque;
	return cpustate->prev_pc;
}

uint32_t I386::get_next_pc()
{
	i386_state *cpustate = (i386_state *)opaque;
	return cpustate->pc;
}

//#ifdef USE_DEBUGGER
void I386::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem->write_data8w(addr, data, &wait);
}

uint32_t I386::translate_address(int segment, uint32_t offset)
{
	i386_state *cpustate = (i386_state *)opaque;
	uint32_t addr = 0;
	if((segment >= 0) && (segment <= GS)) {
		addr = cpustate->sreg[segment].base + offset;
		// addr = (((uint32_t)(cpustate->sreg[segment].selector)) << 4) + offset;
	}
	return addr;
}

uint32_t I386::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem->read_data8w(addr, &wait);
}

void I386::write_debug_data16(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem->write_data16w(addr, data, &wait);
}

uint32_t I386::read_debug_data16(uint32_t addr)
{
	int wait;
	return d_mem->read_data16w(addr, &wait);
}

void I386::write_debug_data32(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem->write_data32w(addr, data, &wait);
}

uint32_t I386::read_debug_data32(uint32_t addr)
{
	int wait;
	return d_mem->read_data32w(addr, &wait);
}

void I386::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	d_io->write_io8w(addr, data, &wait);
}

uint32_t I386::read_debug_io8(uint32_t addr)
{
	int wait;
	return d_io->read_io8w(addr, &wait);
}

void I386::write_debug_io16(uint32_t addr, uint32_t data)
{
	int wait;
	d_io->write_io16w(addr, data, &wait);
}

uint32_t I386::read_debug_io16(uint32_t addr)
{
	int wait;
	return d_io->read_io16w(addr, &wait);
}

void I386::write_debug_io32(uint32_t addr, uint32_t data)
{
	int wait;
	d_io->write_io32w(addr, data, &wait);
}

uint32_t I386::read_debug_io32(uint32_t addr)
{
	int wait;
	return d_io->read_io32w(addr, &wait);
}

bool I386::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	i386_state *cpustate = (i386_state *)opaque;
	if(_tcsicmp(reg, _T("IP")) == 0) {
		cpustate->eip = data & 0xffff;
		CHANGE_PC(cpustate, cpustate->eip);
	} else if(_tcsicmp(reg, _T("EIP")) == 0) {
		cpustate->eip = data;
		CHANGE_PC(cpustate, cpustate->eip);
	} else if(_tcsicmp(reg, _T("EAX")) == 0) {
		REG32(EAX) = data;
	} else if(_tcsicmp(reg, _T("EBX")) == 0) {
		REG32(EBX) = data;
	} else if(_tcsicmp(reg, _T("ECX")) == 0) {
		REG32(ECX) = data;
	} else if(_tcsicmp(reg, _T("EDX")) == 0) {
		REG32(EDX) = data;
	} else if(_tcsicmp(reg, _T("ESP")) == 0) {
		REG32(ESP) = data;
	} else if(_tcsicmp(reg, _T("EBP")) == 0) {
		REG32(EBP) = data;
	} else if(_tcsicmp(reg, _T("ESI")) == 0) {
		REG32(ESI) = data;
	} else if(_tcsicmp(reg, _T("EDI")) == 0) {
		REG32(EDI) = data;
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
	return true;
}

uint32_t I386::read_debug_reg(const _TCHAR *reg)
{
	i386_state *cpustate = (i386_state *)opaque;
	if(_tcsicmp(reg, _T("EIP")) == 0) {
		return cpustate->eip;
	} else if(_tcsicmp(reg, _T("IP")) == 0) {
		return cpustate->eip & 0xffff;
	}  else if(_tcsicmp(reg, _T("EAX")) == 0) {
		return REG32(EAX);
	} else if(_tcsicmp(reg, _T("EBX")) == 0) {
		return REG32(EBX);
	} else if(_tcsicmp(reg, _T("ECX")) == 0) {
		return REG32(ECX);
	} else if(_tcsicmp(reg, _T("EDX")) == 0) {
		return REG32(EDX);
	} else if(_tcsicmp(reg, _T("ESP")) == 0) {
		return REG32(ESP);
	} else if(_tcsicmp(reg, _T("EBP")) == 0) {
		return REG32(EBP);
	} else if(_tcsicmp(reg, _T("ESI")) == 0) {
		return REG32(ESI);
	} else if(_tcsicmp(reg, _T("EDI")) == 0) {
		return REG32(EDI);
	} else if(_tcsicmp(reg, _T("AX")) == 0) {
		return REG16(AX);
	} else if(_tcsicmp(reg, _T("BX")) == 0) {
		return REG16(BX);
	} else if(_tcsicmp(reg, _T("CX")) == 0) {
		return REG16(CX);
	} else if(_tcsicmp(reg, _T("DX")) == 0) {
		return REG16(DX);
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		return REG16(SP);
	} else if(_tcsicmp(reg, _T("BP")) == 0) {
		return REG16(BP);
	} else if(_tcsicmp(reg, _T("SI")) == 0) {
		return REG16(SI);
	} else if(_tcsicmp(reg, _T("DI")) == 0) {
		return REG16(DI);
	} else if(_tcsicmp(reg, _T("AL")) == 0) {
		return REG8(AL);
	} else if(_tcsicmp(reg, _T("AH")) == 0) {
		return REG8(AH);
	} else if(_tcsicmp(reg, _T("BL")) == 0) {
		return REG8(BL);
	} else if(_tcsicmp(reg, _T("BH")) == 0) {
		return REG8(BH);
	} else if(_tcsicmp(reg, _T("CL")) == 0) {
		return REG8(CL);
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		return REG8(CH);
	} else if(_tcsicmp(reg, _T("DL")) == 0) {
		return REG8(DL);
	} else if(_tcsicmp(reg, _T("DH")) == 0) {
		return REG8(DH);
	}
	return 0;
}

void I386::get_debug_gregs_info(_TCHAR *buffer, size_t buffer_len)
{
	i386_state *cpustate = (i386_state *)opaque;
	_TCHAR gprstr[64][256]; // Reserve for Rxx etc
	static const int    gprlist_i[] = {EAX, EBX, ECX, EDX, ESP, EBP, ESI, EDI, -1};
	static const _TCHAR *gprlist_s[] = {_T("EAX"), _T("EBX"), _T("ECX"), _T("EDX"), _T("ESP"), _T("EBP"), _T("ESI"), _T("EDI"), NULL};
	int regnum = 0;
	while(gprlist_i[regnum] >= 0) {
		memset(gprstr[regnum], 0x00, sizeof(_TCHAR) * 256);
		my_stprintf_s(gprstr[regnum], 255, _T("%s=%08X "), gprlist_s[regnum], REG32(gprlist_i[regnum]));
		regnum++;
		if(regnum >= 64) break;
	}
	regnum = 0;
	if(buffer != NULL) {
		while(gprlist_i[regnum] >= 0) {
			my_tcscat_s(buffer, buffer_len, gprstr[regnum]);
			regnum++;
			if(regnum >= 64) break;
		}
		my_tcscat_s(buffer, buffer_len, _T("\n"));
	}
}

void I386::get_debug_sregs_info(_TCHAR *buffer, size_t buffer_len)
{
	i386_state *cpustate = (i386_state *)opaque;
	_TCHAR segstr[8][256];
	static const int    seglist_i[] = {CS, SS, DS, ES, FS, GS, -1};
	static const _TCHAR *seglist_s[] = {_T("CS"), _T("SS"), _T("DS"), _T("ES"), _T("FS"), _T("GS"), NULL};
	int segnum = 0;
	while(seglist_i[segnum] >= 0) {
		memset(segstr[segnum], 0x00, sizeof(_TCHAR) * 256);
		int realseg = seglist_i[segnum];
		my_stprintf_s(segstr[segnum], 255,
					_T("%s : SELECTOR=%04X BASE=%08X LIMIT=%08X FLAGS=%04X D=%d %s\n")
					,
					seglist_s[segnum],
					cpustate->sreg[realseg].selector,
					cpustate->sreg[realseg].base,
					cpustate->sreg[realseg].limit,
					cpustate->sreg[realseg].flags,
					cpustate->sreg[realseg].d,
					(cpustate->sreg[realseg].valid) ? _T("VALID") : _T("INVALID")        
			);
		segnum++;
		if(segnum >= 8) break;
	}
	segnum = 0;
	if(buffer != NULL) {
		while(seglist_i[segnum] >= 0) {
			my_tcscat_s(buffer, buffer_len, segstr[segnum]);
			segnum++;
			if(segnum >= 8) break;
		}
	}
}

bool I386::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	i386_state *cpustate = (i386_state *)opaque;
	_TCHAR segstr[256 * 12] = {0};
	_TCHAR gprstr[256 * 64] = {0};
	get_debug_sregs_info(segstr, sizeof(segstr) / sizeof(_TCHAR));
	get_debug_gregs_info(gprstr, sizeof(gprstr) / sizeof(_TCHAR));
	
	if(cpustate->operand_size) {
		my_stprintf_s(buffer, buffer_len,
					  _T("MODE=%s PC=%08X PREV_PC=%08X SP(REAL)=%08X\n")
					  _T("CR[0-4]=%08X %08X %08X %08X %08X IOPL=%d CPL=%d\n")	  
					  _T("%s")
					  _T("%s")
					  _T("A20_MASK=%08X EIP=%08X  EFLAGS=%08X FLAG=[%c%c%c%c%c%c%c%c%c]\n")
					  _T("GDTR: BASE=%08X LIMIT=%08X LDTR: BASE=%08X LIMIT=%08X\n")
					  _T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
					  (PROTECTED_MODE != 0) ? ((V8086_MODE) ? _T("PROTECTED V8086(32bit)") : _T("PROTECTED 32bit")) : ((V8086_MODE) ? _T("V8086(32bit)") : _T("32bit")),
					  cpustate->pc, cpustate->prev_pc, cpustate->sreg[SS].base + ((uint32_t)REG32(ESP)),
					  cpustate->cr[0] ,cpustate->cr[1], cpustate->cr[2], cpustate->cr[3], cpustate->cr[4], (cpustate->IOP1) | (cpustate->IOP2 << 1), cpustate->CPL,
					  gprstr,
					  segstr,
					  cpustate->a20_mask, cpustate->eip, cpustate->eflags,
					  cpustate->OF ? _T('O') : _T('-'), cpustate->DF ? _T('D') : _T('-'), cpustate->IF ? _T('I') : _T('-'), cpustate->TF ? _T('T') : _T('-'),
					  cpustate->SF ? _T('S') : _T('-'), cpustate->ZF ? _T('Z') : _T('-'), cpustate->AF ? _T('A') : _T('-'), cpustate->PF ? _T('P') : _T('-'), cpustate->CF ? _T('C') : _T('-'),
					  cpustate->gdtr.base, cpustate->gdtr.limit,
					  cpustate->ldtr.base, cpustate->ldtr.limit,
					  cpustate->total_cycles, cpustate->total_cycles - cpustate->prev_total_cycles,
					  get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame()
			);
	} else {
		if((PROTECTED_MODE) != 0) {
			if((V8086_MODE)) {
				my_stprintf_s(buffer, buffer_len,
					  _T("MODE=V8086 (PROTECTED) PC=%08X PREV_PC=%08X SP(REAL)=%08X\n")
					  _T("CR[0-4]=%08X %08X %08X %08X %08X IOP=%d CPL=%d\n")	  
					  _T("%s")
					  _T("%s")
 					  _T("A20_MASK=%08X IP=%04X  FLAG=[%c%c%c%c%c%c%c%c%c]\n")
					  _T("GDTR: BASE=%08X LIMIT=%08X LDTR: BASE=%08X LIMIT=%08X\n")
					  _T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
					  cpustate->pc, cpustate->prev_pc, cpustate->sreg[SS].base + ((uint32_t)REG32(ESP) & 0xffff),
					  cpustate->cr[0] ,cpustate->cr[1], cpustate->cr[2], cpustate->cr[3], cpustate->cr[4], (cpustate->IOP1) | (cpustate->IOP2 << 1), cpustate->CPL,
					  gprstr,
					  segstr,
					  cpustate->a20_mask, cpustate->eip,
					  cpustate->OF ? _T('O') : _T('-'), cpustate->DF ? _T('D') : _T('-'), cpustate->IF ? _T('I') : _T('-'), cpustate->TF ? _T('T') : _T('-'),
					  cpustate->SF ? _T('S') : _T('-'), cpustate->ZF ? _T('Z') : _T('-'), cpustate->AF ? _T('A') : _T('-'), cpustate->PF ? _T('P') : _T('-'), cpustate->CF ? _T('C') : _T('-'),
					  cpustate->gdtr.base, cpustate->gdtr.limit,
					  cpustate->ldtr.base, cpustate->ldtr.limit,
					  cpustate->total_cycles, cpustate->total_cycles - cpustate->prev_total_cycles,
					  get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
			} else {
				my_stprintf_s(buffer, buffer_len,
							  _T("MODE=PROTECTED 16bit PC=%08X PREV_PC=%08X SP(REAL)=%08X\n")
							  _T("CR[0-4]=%08X %08X %08X %08X %08X IOP=%d CPL=%d\n")	  
							  _T("%s")
							  _T("%s")
							  _T("A20_MASK=%08X IP=%04X  EFLAGS=%08X FLAG=[%c%c%c%c%c%c%c%c%c]\n")
							  _T("GDTR: BASE=%08X LIMIT=%08X LDTR: BASE=%08X LIMIT=%08X\n")
							  _T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
							  cpustate->pc, cpustate->prev_pc, cpustate->sreg[SS].base + ((uint32_t)REG32(ESP)),
							  cpustate->cr[0] ,cpustate->cr[1], cpustate->cr[2], cpustate->cr[3], cpustate->cr[4], (cpustate->IOP1) | (cpustate->IOP2 << 1), cpustate->CPL,
							  gprstr,
							  segstr,
							  cpustate->a20_mask, cpustate->eip, cpustate->eflags,
							  cpustate->OF ? _T('O') : _T('-'), cpustate->DF ? _T('D') : _T('-'), cpustate->IF ? _T('I') : _T('-'), cpustate->TF ? _T('T') : _T('-'),
							  cpustate->SF ? _T('S') : _T('-'), cpustate->ZF ? _T('Z') : _T('-'), cpustate->AF ? _T('A') : _T('-'), cpustate->PF ? _T('P') : _T('-'), cpustate->CF ? _T('C') : _T('-'),
							  cpustate->gdtr.base, cpustate->gdtr.limit,
							  cpustate->ldtr.base, cpustate->ldtr.limit,
							  cpustate->total_cycles, cpustate->total_cycles - cpustate->prev_total_cycles,
							  get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
			}
		} else {
				my_stprintf_s(buffer, buffer_len,
							  _T("MODE=16bit PC=%08X PREV_PC=%08X SP(REAL)=%08X\n")
							  _T("CR[0-4]=%08X %08X %08X %08X %08X IOP=%d CPL=%d\n")	  
							  _T("%s")
							  _T("%s")
							  _T("A20_MASK=%08X IP=%04X EFLAGS=%08X FLAG=[%c%c%c%c%c%c%c%c%c]\n")
							  _T("GDTR: BASE=%08X LIMIT=%08X LDTR: BASE=%08X LIMIT=%08X\n")
							  _T("Clocks = %llu (%llu) Since Scanline = %d/%d (%d/%d)"),
							  cpustate->pc, cpustate->prev_pc, cpustate->sreg[SS].base + ((uint32_t)REG16(SP) & 0xffff),
							  cpustate->cr[0] ,cpustate->cr[1], cpustate->cr[2], cpustate->cr[3], cpustate->cr[4], (cpustate->IOP1) | (cpustate->IOP2 << 1), cpustate->CPL,
							  gprstr,
							  segstr,
							  cpustate->a20_mask, cpustate->eip, cpustate->eflags,
							  cpustate->OF ? _T('O') : _T('-'), cpustate->DF ? _T('D') : _T('-'), cpustate->IF ? _T('I') : _T('-'), cpustate->TF ? _T('T') : _T('-'),
							  cpustate->SF ? _T('S') : _T('-'), cpustate->ZF ? _T('Z') : _T('-'), cpustate->AF ? _T('A') : _T('-'), cpustate->PF ? _T('P') : _T('-'), cpustate->CF ? _T('C') : _T('-'),
							  cpustate->gdtr.base, cpustate->gdtr.limit,
							  cpustate->ldtr.base, cpustate->ldtr.limit,
							  cpustate->total_cycles, cpustate->total_cycles - cpustate->prev_total_cycles,
							  get_passed_clock_since_vline(), get_cur_vline_clocks(), get_cur_vline(), get_lines_per_frame());
		}			
	}
	cpustate->prev_total_cycles = cpustate->total_cycles;
	return true;
}
int I386::debug_dasm_with_userdata(uint32_t pc, _TCHAR *buffer, size_t buffer_len, uint32_t userdata)
{
	i386_state *cpustate = (i386_state *)opaque;
	UINT64 eip = pc - cpustate->sreg[CS].base;
	UINT8 oprom[16];
	for(int i = 0; i < 16; i++) {
		int wait;
		oprom[i] = d_mem->read_data8w((pc + i) & cpustate->a20_mask, &wait);
	}
	bool __op32 = (userdata & I386_TRACE_DATA_BIT_USERDATA_SET) ? ((userdata & I386_TRACE_DATA_BIT_OP32) ? true : false) : ((cpustate->operand_size != 0) ? true : false);
	if(__op32) {
		return i386_dasm(oprom, eip, true,  buffer, buffer_len);
	} else {
		return i386_dasm(oprom, eip, false,  buffer, buffer_len);
	}
}
//#endif

void I386::set_address_mask(uint32_t mask)
{
	i386_state *cpustate = (i386_state *)opaque;
	cpustate->a20_mask = mask;
	
	// TODO: how does A20M and the tlb interact
	vtlb_flush_dynamic(cpustate->vtlb);
}

uint32_t I386::get_address_mask()
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

#define STATE_VERSION	8

void process_state_SREG(I386_SREG* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->selector);
	state_fio->StateValue(val->flags);
	state_fio->StateValue(val->base);
	state_fio->StateValue(val->limit);
	state_fio->StateValue(val->d);
	state_fio->StateValue(val->valid);
}

void process_state_SYS_TABLE(I386_SYS_TABLE* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->base);
	state_fio->StateValue(val->limit);
}

void process_state_SEG_DESC(I386_SEG_DESC* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->segment);
	state_fio->StateValue(val->flags);
	state_fio->StateValue(val->base);
	state_fio->StateValue(val->limit);
}

void process_state_GPR(I386_GPR* val, FILEIO* state_fio)
{
	state_fio->StateArray(val->d, sizeof(val->d), 1);
	state_fio->StateArray(val->w, sizeof(val->w), 1);
	state_fio->StateArray(val->b, sizeof(val->b), 1);
}

void process_state_floatx80(floatx80* val, FILEIO* state_fio)
{
	state_fio->StateValue(val->high);
	state_fio->StateValue(val->low);
}

void process_state_XMM_REG(XMM_REG* val, FILEIO* state_fio)
{
	state_fio->StateArray(val->b, sizeof(val->b), 1);
	state_fio->StateArray(val->w, sizeof(val->w), 1);
	state_fio->StateArray(val->d, sizeof(val->d), 1);
	state_fio->StateArray(val->q, sizeof(val->q), 1);
	state_fio->StateArray(val->c, sizeof(val->c), 1);
	state_fio->StateArray(val->s, sizeof(val->s), 1);
	state_fio->StateArray(val->i, sizeof(val->i), 1);
	state_fio->StateArray(val->l, sizeof(val->l), 1);
	state_fio->StateArray(val->f, sizeof(val->f), 1);
	state_fio->StateArray(val->f64, sizeof(val->f64), 1);
}

void process_state_vtlb(vtlb_state* val, FILEIO* state_fio)
{
//	state_fio->StateValue(val->space);
//	state_fio->StateValue(val->dynamic);
//	state_fio->StateValue(val->fixed);
	state_fio->StateValue(val->dynindex);
//	state_fio->StateValue(val->pageshift);
//	state_fio->StateValue(val->addrwidth);
	if(val->live != NULL) {
		state_fio->StateArray(val->live, val->fixed + val->dynamic, 1);
	}
	if(val->fixedpages != NULL) {
		state_fio->StateArray(val->fixedpages, val->fixed, 1);
	}
	if(val->table != NULL) {
		state_fio->StateArray(val->table, (size_t) 1 << (val->addrwidth - val->pageshift), 1);
	}
}

bool I386::process_state(FILEIO* state_fio, bool loading)
{
	i386_state *cpustate = (i386_state *)opaque;
	
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	process_state_GPR(&cpustate->reg, state_fio);
	for(int i = 0; i < array_length(cpustate->sreg); i++) {
		process_state_SREG(&cpustate->sreg[i], state_fio);
	}
	state_fio->StateValue(cpustate->eip);
	state_fio->StateValue(cpustate->pc);
	state_fio->StateValue(cpustate->prev_eip);
	state_fio->StateValue(cpustate->prev_pc);
	state_fio->StateValue(cpustate->eflags);
	state_fio->StateValue(cpustate->eflags_mask);
	state_fio->StateValue(cpustate->CF);
	state_fio->StateValue(cpustate->DF);
	state_fio->StateValue(cpustate->SF);
	state_fio->StateValue(cpustate->OF);
	state_fio->StateValue(cpustate->ZF);
	state_fio->StateValue(cpustate->PF);
	state_fio->StateValue(cpustate->AF);
	state_fio->StateValue(cpustate->IF);
	state_fio->StateValue(cpustate->TF);
	state_fio->StateValue(cpustate->IOP1);
	state_fio->StateValue(cpustate->IOP2);
	state_fio->StateValue(cpustate->NT);
	state_fio->StateValue(cpustate->RF);
	state_fio->StateValue(cpustate->VM);
	state_fio->StateValue(cpustate->AC);
	state_fio->StateValue(cpustate->VIF);
	state_fio->StateValue(cpustate->VIP);
	state_fio->StateValue(cpustate->ID);
	state_fio->StateValue(cpustate->CPL);
	state_fio->StateValue(cpustate->performed_intersegment_jump);
	state_fio->StateValue(cpustate->delayed_interrupt_enable);
	state_fio->StateArray(cpustate->cr, sizeof(cpustate->cr), 1);
	state_fio->StateArray(cpustate->dr, sizeof(cpustate->dr), 1);
	state_fio->StateArray(cpustate->tr, sizeof(cpustate->tr), 1);
	process_state_SYS_TABLE(&cpustate->gdtr, state_fio);
	process_state_SYS_TABLE(&cpustate->idtr, state_fio);
	process_state_SEG_DESC(&cpustate->task, state_fio);
	process_state_SEG_DESC(&cpustate->ldtr, state_fio);
	state_fio->StateValue(cpustate->ext);
	state_fio->StateValue(cpustate->halted);
	state_fio->StateValue(cpustate->haltreq);
	state_fio->StateValue(cpustate->busreq);
	state_fio->StateValue(cpustate->shutdown);
	state_fio->StateValue(cpustate->operand_size);
	state_fio->StateValue(cpustate->xmm_operand_size);
	state_fio->StateValue(cpustate->address_size);
	state_fio->StateValue(cpustate->operand_prefix);
	state_fio->StateValue(cpustate->address_prefix);
	state_fio->StateValue(cpustate->segment_prefix);
	state_fio->StateValue(cpustate->segment_override);
//#ifdef USE_DEBUGGER
	state_fio->StateValue(cpustate->total_cycles);
//#endif
	state_fio->StateValue(cpustate->cycles);
	state_fio->StateValue(cpustate->extra_cycles);
	state_fio->StateValue(cpustate->base_cycles);
	state_fio->StateValue(cpustate->opcode);
	state_fio->StateValue(cpustate->irq_state);
	state_fio->StateValue(cpustate->a20_mask);
	state_fio->StateValue(cpustate->cpuid_max_input_value_eax);
	state_fio->StateValue(cpustate->cpuid_id0);
	state_fio->StateValue(cpustate->cpuid_id1);
	state_fio->StateValue(cpustate->cpuid_id2);
	state_fio->StateValue(cpustate->cpu_version);
	state_fio->StateValue(cpustate->feature_flags);
	state_fio->StateValue(cpustate->tsc);
	state_fio->StateArray(cpustate->perfctr, sizeof(cpustate->perfctr), 1);
	for(int i = 0; i < array_length(cpustate->x87_reg); i++) {
		process_state_floatx80(&cpustate->x87_reg[i], state_fio);
	}
	state_fio->StateValue(cpustate->x87_cw);
	state_fio->StateValue(cpustate->x87_sw);
	state_fio->StateValue(cpustate->x87_tw);
	state_fio->StateValue(cpustate->x87_data_ptr);
	state_fio->StateValue(cpustate->x87_inst_ptr);
	state_fio->StateValue(cpustate->x87_opcode);
	for(int i = 0; i < array_length(cpustate->sse_reg); i++) {
		process_state_XMM_REG(&cpustate->sse_reg[i], state_fio);
	}
	state_fio->StateValue(cpustate->mxcsr);
	state_fio->StateArray(&cpustate->lock_table[0][0], sizeof(cpustate->lock_table), 1);
	if(cpustate->vtlb != NULL) {
		process_state_vtlb(cpustate->vtlb, state_fio);
	}
	state_fio->StateValue(cpustate->smm);
	state_fio->StateValue(cpustate->smi);
	state_fio->StateValue(cpustate->smi_latched);
	state_fio->StateValue(cpustate->nmi_masked);
	state_fio->StateValue(cpustate->nmi_latched);
	state_fio->StateValue(cpustate->smbase);
//	state_fio->StateValue(cpustate->smiact);
	state_fio->StateValue(cpustate->lock);
	
	state_fio->StateValue(cpustate->waitcount);
	state_fio->StateValue(cpustate->waitfactor);
	state_fio->StateValue(cpustate->memory_wait);
#ifdef DEBUG_MISSING_OPCODE
	state_fio->StateArray(cpustate->opcode_bytes, sizeof(cpustate->opcode_bytes), 1);
	state_fio->StateValue(cpustate->opcode_pc);
	state_fio->StateValue(cpustate->opcode_bytes_length);
#endif
	
//#ifdef USE_DEBUGGER
	// post process
	if(loading) {
		cpustate->prev_total_cycles = cpustate->total_cycles;
	}
//#endif
	return true;
}

