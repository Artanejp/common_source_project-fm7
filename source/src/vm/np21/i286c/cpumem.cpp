//#include	"compiler.h"

#ifndef NP2_MEMORY_ASM

#include	"cpucore.h"

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
#endif

// ----

REG8 MEMCALL memp_read8(UINT32 address) {

	address = address & CPU_ADRSMASK;
	return device_mem->read_data8(address);
}

REG16 MEMCALL memp_read16(UINT32 address) {

	address = address & CPU_ADRSMASK;
	return device_mem->read_data16(address);
}

UINT32 MEMCALL memp_read32(UINT32 address) {

	address = address & CPU_ADRSMASK;
	return device_mem->read_data32(address);
}

void MEMCALL memp_write8(UINT32 address, REG8 value) {

	address = address & CPU_ADRSMASK;
	device_mem->write_data8(address, value);
}

void MEMCALL memp_write16(UINT32 address, REG16 value) {

	address = address & CPU_ADRSMASK;
	device_mem->write_data16(address, value);
}

void MEMCALL memp_write32(UINT32 address, UINT32 value) {

	address = address & CPU_ADRSMASK;
	device_mem->write_data32(address, value);
}


void MEMCALL memp_reads(UINT32 address, void *dat, UINT leng) {

	UINT8 *out = (UINT8 *)dat;

	while(leng--) {
		*out++ = memp_read8(address++);
	}
}

void MEMCALL memp_writes(UINT32 address, const void *dat, UINT leng) {

	const UINT8 *out = (UINT8 *)dat;

	while(leng--) {
		memp_write8(address++, *out++);
	}
}


// ---- Logical Space (BIOS)

REG8 MEMCALL memr_read8(UINT seg, UINT off) {

	UINT32	address;

	address = (seg << 4) + LOW16(off);
	return(memp_read8(address));

}

REG16 MEMCALL memr_read16(UINT seg, UINT off) {

	UINT32	address;

	address = (seg << 4) + LOW16(off);
	return(memp_read16(address));

}

void MEMCALL memr_write8(UINT seg, UINT off, REG8 value) {

	UINT32	address;

	address = (seg << 4) + LOW16(off);
	memp_write8(address, value);
}

void MEMCALL memr_write16(UINT seg, UINT off, REG16 value) {

	UINT32	address;

	address = (seg << 4) + LOW16(off);
	memp_write16(address, value);
}

void MEMCALL memr_reads(UINT seg, UINT off, void *dat, UINT leng) {

	UINT8	*out;
	UINT32	adrs;

	out = (UINT8 *)dat;
	adrs = seg << 4;
	off = LOW16(off);

	while(leng--) {
		*out++ = memp_read8(adrs + off);
		off = LOW16(off + 1);
	}
}

void MEMCALL memr_writes(UINT seg, UINT off, const void *dat, UINT leng) {

	UINT8	*out;
	UINT32	adrs;

	out = (UINT8 *)dat;
	adrs = seg << 4;
	off = LOW16(off);

	while(leng--) {
		memp_write8(adrs + off, *out++);
		off = LOW16(off + 1);
	}
}

void IOOUTCALL iocore_out8(UINT port, REG8 dat)
{
	device_io->write_io8(port, dat);
}

REG8 IOINPCALL iocore_inp8(UINT port)
{
	return device_io->read_io8(port);
}

void IOOUTCALL iocore_out16(UINT port, REG16 dat)
{
	device_io->write_io16(port, dat);
}

REG16 IOINPCALL iocore_inp16(UINT port)
{
	return device_io->read_io16(port);
}

void IOOUTCALL iocore_out32(UINT port, UINT32 dat)
{
	device_io->write_io32(port, dat);
}

UINT32 IOINPCALL iocore_inp32(UINT port)
{
	return device_io->read_io32(port);
}

void dmax86(void)
{
#ifdef SINGLE_MODE_DMA
	if(device_dma != NULL) device_dma->do_dma();
#endif
}

void dmav30(void)
{
#ifdef SINGLE_MODE_DMA
	if(device_dma != NULL) device_dma->do_dma();
#endif
}

#endif

