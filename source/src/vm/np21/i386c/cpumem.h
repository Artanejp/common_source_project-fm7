#ifndef	NP2_I386C_CPUMEM_H__
#define	NP2_I386C_CPUMEM_H__
#include	"cpucore.h"
#if defined(SUPPORT_IA32_HAXM)
#include	"i386hax/haxfunc.h"
#include	"i386hax/haxcore.h"
#endif

#ifdef NP2_MEMORY_ASM			// アセンブラ版は 必ずfastcallで
#undef	MEMCALL
#define	MEMCALL	FASTCALL
#endif

#if !defined(MEMCALL)
#define	MEMCALL
#endif

//#ifdef __cplusplus
//extern "C" {
//#endif
#include "../../common.h"
#include "../device.h"
//class I386;
#include "../i386_np21.h"

extern I386   *device_cpu;
extern DEVICE *device_mem;
extern DEVICE *device_io;
//#ifdef I386_PSEUDO_BIOS
extern DEVICE *device_bios;
//#endif
//#ifdef SINGLE_MODE_DMA
extern DEVICE *device_dma;
//#endif
extern DEBUGGER *device_debugger;
extern SINT32 __exception_set;
extern UINT32 __exception_pc;
extern UINT64 __exception_code;

inline REG8 MEMCALL memp_read8(UINT32 address);
inline REG16 MEMCALL memp_read16(UINT32 address);
inline UINT32 MEMCALL memp_read32(UINT32 address);
inline void MEMCALL memp_write8(UINT32 address, REG8 value);
inline void MEMCALL memp_write16(UINT32 address, REG16 value);
inline void MEMCALL memp_write32(UINT32 address, UINT32 value);
inline void MEMCALL memp_reads(UINT32 address, void *dat, UINT leng);
inline void MEMCALL memp_writes(UINT32 address, const void *dat, UINT leng);
inline REG8 MEMCALL memp_read8_codefetch(UINT32 address);
inline REG16 MEMCALL memp_read16_codefetch(UINT32 address);
inline UINT32 MEMCALL memp_read32_codefetch(UINT32 address);
inline REG8 MEMCALL memp_read8_paging(UINT32 address);
inline REG16 MEMCALL memp_read16_paging(UINT32 address);
inline UINT32 MEMCALL memp_read32_paging(UINT32 address);
inline void MEMCALL memp_write8_paging(UINT32 address, REG8 value);
inline void MEMCALL memp_write16_paging(UINT32 address, REG16 value);
inline void MEMCALL memp_write32_paging(UINT32 address, UINT32 value);

REG8 MEMCALL meml_read8(UINT32 address);
REG16 MEMCALL meml_read16(UINT32 address);
UINT32 MEMCALL meml_read32(UINT32 address);
void MEMCALL meml_write8(UINT32 address, REG8 dat);
void MEMCALL meml_write16(UINT32 address, REG16 dat);
void MEMCALL meml_write32(UINT32 address, UINT32 dat);
void MEMCALL meml_reads(UINT32 address, void *dat, UINT leng);
void MEMCALL meml_writes(UINT32 address, const void *dat, UINT leng);

REG8 MEMCALL memr_read8(UINT seg, UINT off);
REG16 MEMCALL memr_read16(UINT seg, UINT off);
UINT32 MEMCALL memr_read32(UINT seg, UINT off);
void MEMCALL memr_write8(UINT seg, UINT off, REG8 dat);
void MEMCALL memr_write16(UINT seg, UINT off, REG16 dat);
void MEMCALL memr_write32(UINT seg, UINT off, UINT32 dat);
void MEMCALL memr_reads(UINT seg, UINT off, void *dat, UINT leng);
void MEMCALL memr_writes(UINT seg, UINT off, const void *dat, UINT leng);

inline void IOOUTCALL iocore_out8(UINT port, REG8 dat);
inline REG8 IOINPCALL iocore_inp8(UINT port);

inline void IOOUTCALL iocore_out16(UINT port, REG16 dat);
inline REG16 IOINPCALL iocore_inp16(UINT port);

inline void IOOUTCALL iocore_out32(UINT port, UINT32 dat);
inline UINT32 IOINPCALL iocore_inp32(UINT port);

inline void dmax86(void);

#ifndef	IA32_CPU_IA32_MCR__
#include "./ia32/ia32.mcr"
#endif
//#ifdef __cplusplus
//}
//#endif


// ---- Physical Space (DMA)

#define	MEMP_READ8(addr)					\
			memp_read8((addr))
#define	MEMP_READ16(addr)					\
			memp_read16((addr))
#define	MEMP_READ32(addr)					\
			memp_read32((addr))
#define	MEMP_WRITE8(addr, dat)				\
			memp_write8((addr), (dat))
#define	MEMP_WRITE16(addr, dat)				\
			memp_write16((addr), (dat))
#define	MEMP_WRITE32(addr, dat)				\
			memp_write32((addr), (dat))
#define MEMP_READS(addr, dat, leng)			\
			memp_reads((addr), (dat), (leng))
#define MEMP_WRITES(addr, dat, leng)		\
			memp_writes((addr), (dat), (leng))


// ---- Logical Space (BIOS)

#define MEML_READ8(addr)					\
			meml_read8((addr))
#define MEML_READ16(addr)					\
			meml_read16((addr))
#define MEML_READ32(addr)					\
			meml_read32((addr))
#define MEML_WRITE8(addr, dat)				\
			meml_write8((addr), (dat))
#define MEML_WRITE16(addr, dat)				\
			meml_write16((addr), (dat))
#define MEML_WRITE32(addr, dat)				\
			meml_write32((addr), (dat))
#define MEML_READS(addr, dat, leng)			\
			meml_reads((addr), (dat), (leng))
#define MEML_WRITES(addr, dat, leng)		\
			meml_writes((addr), (dat), (leng))

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

inline REG8 MEMCALL memp_read8(UINT32 address) {
	address = address & CPU_ADRSMASK;
	int wait = 0;
	REG8 val;
	val = device_mem->read_data8w(address, &wait);
	CPU_WORKCLOCK(wait);
	return val;
}

inline REG16 MEMCALL memp_read16(UINT32 address) {
	address = address & CPU_ADRSMASK;
	int wait = 0;
	REG16 val;
	val = device_mem->read_data16w(address, &wait);
	CPU_WORKCLOCK(wait);
	return val;
}

inline UINT32 MEMCALL memp_read32(UINT32 address) {
	address = address & CPU_ADRSMASK;
	int wait = 0;
	UINT32 val;
	val = device_mem->read_data32w(address, &wait);
	CPU_WORKCLOCK(wait);
	return val;
}

// ----
inline REG8 MEMCALL memp_read8_codefetch(UINT32 address) {

	address = address & CPU_ADRSMASK;
	int wait = 0;
	REG8 val;
	CPU_FETCHADR = address;
	val = device_mem->read_data8w(CPU_FETCHADR, &wait);
	CPU_WORKCLOCK(wait);
	return val;
}

inline REG16 MEMCALL memp_read16_codefetch(UINT32 address) {
	address = address & CPU_ADRSMASK;
	int wait = 0;
	REG16 val;
	CPU_FETCHADR = address;
	val = device_mem->read_data16w(CPU_FETCHADR, &wait);
	CPU_WORKCLOCK(wait);
	return val;
}

inline UINT32 MEMCALL memp_read32_codefetch(UINT32 address) {

	address = address & CPU_ADRSMASK;
	int wait = 0;
	UINT32 val;
	CPU_FETCHADR = address;
	val = device_mem->read_data32w(CPU_FETCHADR, &wait);
	CPU_WORKCLOCK(wait);
	return val;
}

// ----
inline REG8 MEMCALL memp_read8_paging(UINT32 address) {

	return memp_read8_codefetch(address);
}
inline REG16 MEMCALL memp_read16_paging(UINT32 address) {

	return memp_read16_codefetch(address);
}

inline UINT32 MEMCALL memp_read32_paging(UINT32 address) {

	return memp_read32_codefetch(address);
}

inline void MEMCALL memp_write8(UINT32 address, REG8 value) {

	address = address & CPU_ADRSMASK;
	int wait = 0;
	device_mem->write_data8w(address, value, &wait);
	CPU_WORKCLOCK(wait);

}

inline void MEMCALL memp_write16(UINT32 address, REG16 value) {

	address = address & CPU_ADRSMASK;
	int wait = 0;
	device_mem->write_data16w(address, value, &wait);
	CPU_WORKCLOCK(wait);
}

inline void MEMCALL memp_write32(UINT32 address, UINT32 value) {

	address = address & CPU_ADRSMASK;
	int wait = 0;
	device_mem->write_data32w(address, value, &wait);
	CPU_WORKCLOCK(wait);
}

inline void MEMCALL memp_write8_paging(UINT32 address, REG8 value) {

	address = address & CPU_ADRSMASK;
	int wait = 0;
	device_mem->write_data8w(address, value, &wait);
	CPU_WORKCLOCK(wait);
}

inline void MEMCALL memp_write16_paging(UINT32 address, REG16 value) {

	address = address & CPU_ADRSMASK;
	int wait = 0;
	device_mem->write_data16w(address, value, &wait);
	CPU_WORKCLOCK(wait);
}

inline void MEMCALL memp_write32_paging(UINT32 address, UINT32 value) {

	address = address & CPU_ADRSMASK;
	int wait = 0;
	device_mem->write_data32w(address, value, &wait);
	CPU_WORKCLOCK(wait);

}


inline void MEMCALL memp_reads(UINT32 address, void *dat, UINT leng) {

	UINT8 *out = (UINT8 *)dat;

	//address = address & CPU_ADRSMASK;

	/* slow memory access */
	while (leng-- > 0) {
		address = address & CPU_ADRSMASK;
		*out++ = memp_read8(address++);
	}
}

inline void MEMCALL memp_writes(UINT32 address, const void *dat, UINT leng) {

	const UINT8 *out = (UINT8 *)dat;

	/* slow memory access */
	while (leng-- > 0) {
		address = address & CPU_ADRSMASK;
		memp_write8(address++, *out++);
	}
}

inline void dmax86(void)
{
//#ifdef SINGLE_MODE_DMA
	__LIKELY_IF(device_dma != NULL) device_dma->do_dma();
//#endif
}

inline void IOOUTCALL iocore_out8(UINT port, REG8 dat)
{
	int wait = 0;
	device_io->write_io8w(port, dat, &wait);
	CPU_WORKCLOCK(wait);
}

inline REG8 IOINPCALL iocore_inp8(UINT port)
{
	int wait = 0;
	REG8 val = device_io->read_io8w(port, &wait);
	CPU_WORKCLOCK(wait);
	return val;
}

inline void IOOUTCALL iocore_out16(UINT port, REG16 dat)
{
	int wait = 0;
	device_io->write_io16w(port, dat, &wait);
	CPU_WORKCLOCK(wait);
}

inline REG16 IOINPCALL iocore_inp16(UINT port)
{
	int wait = 0;
	REG16 val = device_io->read_io16w(port, &wait);
	CPU_WORKCLOCK(wait);
	return val;
}

inline void IOOUTCALL iocore_out32(UINT port, UINT32 dat)
{
	int wait = 0;
	device_io->write_io32w(port, dat, &wait);
	CPU_WORKCLOCK(wait);
}

inline UINT32 IOINPCALL iocore_inp32(UINT port)
{
	int wait = 0;
	UINT32 val = device_io->read_io32w(port, &wait);
	CPU_WORKCLOCK(wait);
	return val;
}

#endif	/* !NP2_I386C_CPUMEM_H__ */
