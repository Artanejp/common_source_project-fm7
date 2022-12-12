//#include	"compiler.h"

#if 1
#undef	TRACEOUT
//#define USE_TRACEOUT_VS
//#define MEM_BDA_TRACEOUT
//#define MEM_D8_TRACEOUT
#ifdef USE_TRACEOUT_VS
static void trace_fmt_ex(const char *fmt, ...)
{
	char stmp[2048];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(stmp, fmt, ap);
	strcat(stmp, "\n");
	va_end(ap);
	OutputDebugStringA(stmp);
}
#define	TRACEOUT(s)	trace_fmt_ex s
#else
#define	TRACEOUT(s)	(void)(s)
#endif
#endif	/* 1 */

#ifndef NP2_MEMORY_ASM

#include	"cpucore.h"
#if defined(SUPPORT_IA32_HAXM)
#include	"i386hax/haxfunc.h"
#include	"i386hax/haxcore.h"
#endif
#include	"ia32/ia32.mcr"

DEVICE *device_cpu;
DEVICE *device_mem;
DEVICE *device_io;
#ifdef I86_PSEUDO_BIOS
DEVICE *device_bios = NULL;
#endif
#ifdef SINGLE_MODE_DMA
DEVICE *device_dma = NULL;
#endif
#ifdef USE_DEBUGGER
DEBUGGER *device_debugger;
UINT32 codefetch_address;
#endif

// ----
REG8 MEMCALL memp_read8(UINT32 address) {
	
	int wait = 0;
	address = address & CPU_ADRSMASK;
	REG8 value = device_mem->read_data8w(address, &wait);
	CPU_WORKCLOCK(wait);
	return value;
}

REG16 MEMCALL memp_read16(UINT32 address) {

	int wait = 0;
	address = address & CPU_ADRSMASK;
	REG16 value = device_mem->read_data16w(address, &wait);
	CPU_WORKCLOCK(wait);
	return value;
}

UINT32 MEMCALL memp_read32(UINT32 address) {

	int wait = 0;
	address = address & CPU_ADRSMASK;
	UINT32 value = device_mem->read_data32w(address, &wait);
	CPU_WORKCLOCK(wait);
	return value;
}

// ----
REG8 MEMCALL memp_read8_codefetch(UINT32 address) {
	
	int wait = 0;
#ifdef USE_DEBUGGER
	codefetch_address = address & CPU_ADRSMASK;
	REG8 value = device_mem->read_data8w(codefetch_address, &wait);
#else
	address = address & CPU_ADRSMASK;
	REG8 value = device_mem->read_data8w(address, &wait);
#endif
	CPU_WORKCLOCK(wait);
	return value;
}

REG16 MEMCALL memp_read16_codefetch(UINT32 address) {

	int wait = 0;
	address = address & CPU_ADRSMASK;
	REG16 value = device_mem->read_data16w(address, &wait);
	CPU_WORKCLOCK(wait);
	return value;
}

UINT32 MEMCALL memp_read32_codefetch(UINT32 address) {

	int wait = 0;
	address = address & CPU_ADRSMASK;
	UINT32 value = device_mem->read_data32w(address, &wait);
	CPU_WORKCLOCK(wait);
	return value;
}

// ----
REG8 MEMCALL memp_read8_paging(UINT32 address) {
	
	return memp_read8_codefetch(address);
}
REG16 MEMCALL memp_read16_paging(UINT32 address) {
	
	return memp_read16_codefetch(address);
}

UINT32 MEMCALL memp_read32_paging(UINT32 address) {
	
	return memp_read32_codefetch(address);
}

void MEMCALL memp_write8(UINT32 address, REG8 value) {
	
	int wait = 0;
	address = address & CPU_ADRSMASK;
	device_mem->write_data8w(address, value, &wait);
	CPU_WORKCLOCK(wait);
}

void MEMCALL memp_write16(UINT32 address, REG16 value) {
	
	int wait = 0;
	address = address & CPU_ADRSMASK;
	device_mem->write_data16w(address, value, &wait);
	CPU_WORKCLOCK(wait);
}

void MEMCALL memp_write32(UINT32 address, UINT32 value) {
	
	int wait = 0;
	address = address & CPU_ADRSMASK;
	device_mem->write_data32w(address, value, &wait);
	CPU_WORKCLOCK(wait);
}

void MEMCALL memp_write8_paging(UINT32 address, REG8 value) {
	
	int wait = 0;
	address = address & CPU_ADRSMASK;
	device_mem->write_data8w(address, value, &wait);
	CPU_WORKCLOCK(wait);
}

void MEMCALL memp_write16_paging(UINT32 address, REG16 value) {
	
	int wait = 0;
	address = address & CPU_ADRSMASK;
	device_mem->write_data16w(address, value, &wait);
	CPU_WORKCLOCK(wait);
}

void MEMCALL memp_write32_paging(UINT32 address, UINT32 value) {
	
	int wait = 0;
	address = address & CPU_ADRSMASK;
	device_mem->write_data32w(address, value, &wait);
	CPU_WORKCLOCK(wait);
}


void MEMCALL memp_reads(UINT32 address, void *dat, UINT leng) {

	UINT8 *out = (UINT8 *)dat;
	
	//address = address & CPU_ADRSMASK;

	/* slow memory access */
	while (leng-- > 0) {
		*out++ = memp_read8(address++);
	}
}

void MEMCALL memp_writes(UINT32 address, const void *dat, UINT leng) {

	const UINT8 *out = (UINT8 *)dat;

	//address = address & CPU_ADRSMASK;

	/* slow memory access */
	while (leng-- > 0) {
		memp_write8(address++, *out++);
	}
}


// ---- Logical Space (BIOS)

static UINT32 MEMCALL physicaladdr(UINT32 addr, BOOL wr) {

	UINT32	a;
	UINT32	pde;
	UINT32	pte;

	a = CPU_STAT_PDE_BASE + ((addr >> 20) & 0xffc);
	pde = memp_read32(a);
	if (!(pde & CPU_PDE_PRESENT)) {
		goto retdummy;
	}
	if (!(pde & CPU_PDE_ACCESS)) {
		memp_write8(a, (UINT8)(pde | CPU_PDE_ACCESS));
	}
	a = (pde & CPU_PDE_BASEADDR_MASK) + ((addr >> 10) & 0xffc);
	pte = cpu_memoryread_d(a);
	if (!(pte & CPU_PTE_PRESENT)) {
		goto retdummy;
	}
	if (!(pte & CPU_PTE_ACCESS)) {
		memp_write8(a, (UINT8)(pte | CPU_PTE_ACCESS));
	}
	if ((wr) && (!(pte & CPU_PTE_DIRTY))) {
		memp_write8(a, (UINT8)(pte | CPU_PTE_DIRTY));
	}
	addr = (pte & CPU_PTE_BASEADDR_MASK) + (addr & 0x00000fff);
	return(addr);

 retdummy:
	return(0x01000000);	/* XXX */
}


void MEMCALL meml_reads(UINT32 address, void *dat, UINT leng) {

	UINT	size;

	if (!CPU_STAT_PAGING) {
		memp_reads(address, dat, leng);
	}
	else {
		while(leng) {
			size = 0x1000 - (address & 0xfff);
			size = min(size, leng);
			memp_reads(physicaladdr(address, FALSE), dat, size);
			address += size;
			dat = ((UINT8 *)dat) + size;
			leng -= size;
		}
	}
}

void MEMCALL meml_writes(UINT32 address, const void *dat, UINT leng) {

	UINT	size;

	if (!CPU_STAT_PAGING) {
		memp_writes(address, dat, leng);
	}
	else {
		while(leng) {
			size = 0x1000 - (address & 0xfff);
			size = min(size, leng);
			memp_writes(physicaladdr(address, TRUE), dat, size);
			address += size;
			dat = ((UINT8 *)dat) + size;
			leng -= size;
		}
	}
}


REG8 MEMCALL memr_read8(UINT seg, UINT off) {

	UINT32	addr;

	addr = (seg << 4) + LOW16(off);
	if (CPU_STAT_PAGING) {
		addr = physicaladdr(addr, FALSE);
	}
	return(memp_read8(addr));
}

REG16 MEMCALL memr_read16(UINT seg, UINT off) {

	UINT32	addr;

	addr = (seg << 4) + LOW16(off);
	if (!CPU_STAT_PAGING) {
		return(memp_read16(addr));
	}
	else if ((addr + 1) & 0xfff) {
		return(memp_read16(physicaladdr(addr, FALSE)));
	}
	return(memr_read8(seg, off) + (memr_read8(seg, off + 1) << 8));
}

void MEMCALL memr_write8(UINT seg, UINT off, REG8 dat) {

	UINT32	addr;

	addr = (seg << 4) + LOW16(off);
	if (CPU_STAT_PAGING) {
		addr = physicaladdr(addr, TRUE);
	}
	memp_write8(addr, dat);
}

void MEMCALL memr_write16(UINT seg, UINT off, REG16 dat) {

	UINT32	addr;

	addr = (seg << 4) + LOW16(off);
	if (!CPU_STAT_PAGING) {
		memp_write16(addr, dat);
	}
	else if ((addr + 1) & 0xfff) {
		memp_write16(physicaladdr(addr, TRUE), dat);
	}
	else {
		memr_write8(seg, off, (REG8)dat);
		memr_write8(seg, off + 1, (REG8)(dat >> 8));
	}
}

void MEMCALL memr_reads(UINT seg, UINT off, void *dat, UINT leng) {

	UINT32	addr;
	UINT	rem;
	UINT	size;

	while(leng) {
		off = LOW16(off);
		addr = (seg << 4) + off;
		rem = 0x10000 - off;
		size = min(leng, rem);
		if (CPU_STAT_PAGING) {
			rem = 0x1000 - (addr & 0xfff);
			size = min(size, rem);
			addr = physicaladdr(addr, FALSE);
		}
		memp_reads(addr, dat, size);
		off += size;
		dat = ((UINT8 *)dat) + size;
		leng -= size;
	}
}

void MEMCALL memr_writes(UINT seg, UINT off, const void *dat, UINT leng) {

	UINT32	addr;
	UINT	rem;
	UINT	size;

	while(leng) {
		off = LOW16(off);
		addr = (seg << 4) + off;
		rem = 0x10000 - off;
		size = min(leng, rem);
		if (CPU_STAT_PAGING) {
			rem = 0x1000 - (addr & 0xfff);
			size = min(size, rem);
			addr = physicaladdr(addr, TRUE);
		}
		memp_writes(addr, dat, size);
		off += size;
		dat = ((UINT8 *)dat) + size;
		leng -= size;
	}
}

void IOOUTCALL iocore_out8(UINT port, REG8 dat)
{
	int wait = 0;
	device_io->write_io8w(port, dat, &wait);
	CPU_WORKCLOCK(wait);
}

REG8 IOINPCALL iocore_inp8(UINT port)
{
	int wait = 0;
	REG8 dat = device_io->read_io8w(port, &wait);
	CPU_WORKCLOCK(wait);
	return dat;
}

void IOOUTCALL iocore_out16(UINT port, REG16 dat)
{
	int wait = 0;
	device_io->write_io16w(port, dat, &wait);
	CPU_WORKCLOCK(wait);
}

REG16 IOINPCALL iocore_inp16(UINT port)
{
	int wait = 0;
	REG16 dat = device_io->read_io16w(port, &wait);
	CPU_WORKCLOCK(wait);
	return dat;
}

void IOOUTCALL iocore_out32(UINT port, UINT32 dat)
{
	int wait = 0;
	device_io->write_io32w(port, dat, &wait);
	CPU_WORKCLOCK(wait);
}

UINT32 IOINPCALL iocore_inp32(UINT port)
{
	int wait = 0;
	UINT32 dat = device_io->read_io32w(port, &wait);
	CPU_WORKCLOCK(wait);
	return dat;
}

void dmax86(void)
{
#ifdef SINGLE_MODE_DMA
	if(device_dma != NULL) device_dma->do_dma();
#endif
}

#endif
