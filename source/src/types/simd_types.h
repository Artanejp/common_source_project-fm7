#pragma once

// Write primitives in simde_ AVX2 primitives,
// but you can build/run without neither X86 or AVX2 feature.
// i.e. i686 (without SIMD anymore),  (basic x86_64; SSE2) ,
//      ARM , RIEC-V (with/without SIMD feature) and sny ARCHs.

#undef USE_SIMD_X86_FALLBACK

#if defined(USE_SIMD_ARM_NEON)
# if !defined(SIMDE_ARM_NEON_H)
# include <simde/arm/neon.h>
# endif
#elif defined(USE_SIMD_ARM_SVE)
# if !defined(SIMDE_ARM_SVE_H)
# include <simde/arm/sve.h>
# endif
#elif defined(USE_SIMD_MIPS)
# if !defined(SIMDE_MIPS_MSA_H)
# include <simde/mips/msa.h>
# endif
#elif (USE_SIMD_WASM)
# if !defined(SIMDE_WASM_RELAXED_SIMD_H)
# include <simde/wasm/relaxed-simd.h>
# endif
#elif !defined(USE_SIMD_X86) /* FALLBACK */
#define USE_SIMD_X86_FALLBACK
#endif

/* FALLBACK and x86 */
#if defined(USE_SIMD_X86) || defined(USE_SIMD_X86_FALLBACK)
# if !defined(SIMDE_X86_AVX2_H)
# include <simde/x86/avx2.h>
# endif
# if !defined(USE_SIMD_X86)
# define USE_SIMD_X86
# endif
#endif

#undef USE_SIMD_X86_FALLBACK

#if !defined(HAS_SIMDE)
#define HAS_SIMDE
#endif

#include "types/basic_types.h"
#include "types/system_endians.h"

#include "types/optimizer_utils.h"

#include "types/scrntype_t.h"
#include "types/pair16_t.h"
#include "types/pair32_t.h"
#include "types/pair64_t.h"

#if !defined(__MINIMUM_ALIGN_LENGTH)
# if defined(SIMDE_ALIGN_PLATFORM_MAXIMUM) && (SIMDE_ALIGN_PLATFORM_MAXIMUM >= 16)
# define __M__MINIMUM_ALIGN_LENGTH 16 /* OK? */
# else
# define __M__MINIMUM_ALIGN_LENGTH 8 /* OK? */
# endif
#else
#define __M__MINIMUM_ALIGN_LENGTH __MINIMUM_ALIGN_LENGTH
#endif

typedef union {
	uint8_t u8[8];
	int8_t  s8[8];
	uint16_t u16[4];
	int16_t  s16[4];
	uint32_t u32[2];
	int32_t  s32[2];
	uint64_t d;
	int64_t  sd;
	pair64_t pair;
} uint8_8_t;

#undef __TMP_V128_TYPE
#undef __TMP_V256_TYPE

#if defined(USE_SIMD_X86)
#define __TMP_V128_TYPE	simde__m128
#define __TMP_V256_TYPE	simde__m256
#elif defined(USE_SIMD_WASM)
#define __TMP_V128_TYPE	simde_v128_t
#elif defined(USE_SIMD_ARM_NEON)
#define __TMP_V128_TYPE	simde_uint16x8_t
#define __TMP_V256_TYPE	simde_uint16x8x2_t
#elif defined(USE_SIMD_MIPS)
#define __TMP_V128_TYPE	simde_v8u16
#endif

#if !defined(__TMP_V128_TYPE)
typedef uint16_t __tmp_simd_v128 SIMDE_VECTOR(16);
#define __TMP_V128_TYPE __tmp_simd_v128;
#else
typedef __TMP_V128_TYPE __tmp_simd_v128;
#endif
#if !defined(__TMP_V256_TYPE)
typedef uint16_t __tmp_simd_v256 SIMDE_VECTOR(32);
#define __TMP_V256_TYPE __tmp_simd_v256
#else
typedef __TMP_V256_TYPE __tmp_simd_v256;
#endif

typedef union {
	uint8_t  u8[16];
	uint8_t  s8[16];	
	uint16_t u16[8];
	int16_t  s16[8];
	uint32_t u32[4];
	int32_t  s32[4];
	uint64_t u64[2];
	int64_t  s64[2];
	union {
		uint8_8_t array[2];
		struct {
		#ifdef __BIG_ENDIAN__
			uint8_8_t h, l;
			#else
			uint8_8_t l, h;
			#endif
		} hl;
	} u8_8;
	union {
		pair64_t array[2];
		struct {
		#ifdef __BIG_ENDIAN__
			pair64_t h, l;
			#else
			pair64_t l, h;
			#endif
		} hl;
	} pair;
	__TMP_V128_TYPE	v;
} uint16_8_t;

typedef union {
	uint8_t  u8[32];
	uint8_t  s8[32];	
	uint16_t u16[16];
	int16_t  s16[16];
	uint32_t u32[8];
	int32_t  s32[8];
	uint64_t u64[4];
	int64_t  s64[4];
	union {
		uint8_8_t array[4];
		struct {
		#ifdef __BIG_ENDIAN__
			uint8_8_t h3, h2, h, l;
			#else
			uint8_8_t l, h, h2, h3;
			#endif
		} hl;
	} u8_8;
	union {
		pair64_t array[4];
		struct {
		#ifdef __BIG_ENDIAN__
			pair64_t h3, h2, h, l;
			#else
			pair64_t l, h, h2, h3;
			#endif
		} hl;
	} pair;
	union {
		uint16_8_t array[2];
		struct {
			#ifdef __BIG_ENDIAN__
			uint16_8_t h, l;
			#else
			uint16_8_t l, h;
			#endif
		} hl;
	} u16_8;		
	union {
		__TMP_V128_TYPE array[2];
		struct {
			#ifdef __BIG_ENDIAN__
			__TMP_V128_TYPE h, l;
			#else
			__TMP_V128_TYPE l, h;
			#endif
		} hl;
	} v128;
	__TMP_V256_TYPE v;
} uint32_8_t;

typedef uint8_8_t simd64_t;
typedef uint16_8_t simd128_t;
typedef uint32_8_t simd256_t;
typedef uint32_8_t uint16_16_t;

#undef SCRNTYPE8_T_WIDTH
#undef SCRNTYPE8_T_ALIGN
typedef union {
	scrntype_t  s[8];
	#if defined(_RGB555) || defined(RGB565)  /* 16bpp */
	#define SCRNTYPE8_T_WIDTH 16
	simd128_t   vp;
	simde__m128 v;
	#else
	/* 32 (24) bpp */
	#define SCRNTYPE8_T_WIDTH 32
	simd256_t   vp;
	simde__m256 v;
	#endif
} scrntype8_t;

#undef __TMP_V128_TYPE
#undef __TMP_V256_TYPE
