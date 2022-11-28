
#pragma once

#ifndef MEMCALL
#define	MEMCALL
#endif

//#ifdef __cplusplus
//extern "C" {
//#endif

REG8 MEMCALL memp_read8(UINT32 address);
REG16 MEMCALL memp_read16(UINT32 address);
UINT32 MEMCALL memp_read32(UINT32 address);
void MEMCALL memp_write8(UINT32 address, REG8 value);
void MEMCALL memp_write16(UINT32 address, REG16 value);
void MEMCALL memp_write32(UINT32 address, UINT32 value);

void MEMCALL memp_reads(UINT32 address, void *dat, UINT leng);
void MEMCALL memp_writes(UINT32 address, const void *dat, UINT leng);

REG8 MEMCALL memr_read8(UINT seg, UINT off);
REG16 MEMCALL memr_read16(UINT seg, UINT off);
void MEMCALL memr_write8(UINT seg, UINT off, REG8 value);
void MEMCALL memr_write16(UINT seg, UINT off, REG16 value);
void MEMCALL memr_reads(UINT seg, UINT off, void *dat, UINT leng);
void MEMCALL memr_writes(UINT seg, UINT off, const void *dat, UINT leng);

void IOOUTCALL iocore_out8(UINT port, REG8 dat);
REG8 IOINPCALL iocore_inp8(UINT port);

void IOOUTCALL iocore_out16(UINT port, REG16 dat);
REG16 IOINPCALL iocore_inp16(UINT port);

void IOOUTCALL iocore_out32(UINT port, UINT32 dat);
UINT32 IOINPCALL iocore_inp32(UINT port);

void dmax86(void);
void dmav30(void);

//#ifdef __cplusplus
//}
//#endif


// ---- Physical Space (DMA)

#define	MEMP_READ8(addr)					\
			memp_read8((addr))
#define	MEMP_WRITE8(addr, dat)				\
			memp_write8((addr), (dat))


// ---- Logical Space (BIOS)

#define	MEML_READ8(addr)					\
			memp_read8((addr))
#define	MEML_READ16(addr)					\
			memp_read16((addr))
#define	MEML_WRITE8(addr, dat)				\
			memp_write8((addr), (dat))
#define	MEML_WRITE16(addr, dat)				\
			memp_write16((addr), (dat))
#define MEML_READS(addr, dat, leng)			\
			memp_reads((addr), (dat), (leng))
#define MEML_WRITES(addr, dat, leng)		\
			memp_writes((addr), (dat), (leng))

#define	MEMR_READ8(seg, off)				\
			memr_read8((seg), (off))
#define	MEMR_READ16(seg, off)				\
			memr_read16((seg), (off))
#define	MEMR_WRITE8(seg, off, dat)			\
			memr_write8((seg), (off), (dat))
#define	MEMR_WRITE16(seg, off, dat)			\
			memr_write16((seg), (off), (dat))
#define MEMR_READS(seg, off, dat, leng)		\
			memr_reads((seg), (off), (dat), (leng))
#define MEMR_WRITES(seg, off, dat, leng)	\
			memr_writes((seg), (off), (dat), (leng))

