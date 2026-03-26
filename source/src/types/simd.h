/*
	Skelton for retropc emulator

	Author  : Kyuma Ohta <whatisthis.sowhat@gmail.com>
	Date    : 2023.03.13-
	License : GPLv2
	[ simd utils ]

*/

#pragma once

#include <type_traits>
#include <utility>

// Write primitives in simde_ AVX2 primitives,
// but you can build/run without neither X86 or AVX2 feature.
// i.e. i686 (without SIMD anymore),  (basic x86_64; SSE2) ,
//      ARM , RIEC-V (with/without SIMD feature) and sny ARCHs.


#define USE_SIMD_X86
#include "./simd_types.h"

// Belows are using SIMD Everywhere.
// See https://github.com/simd-everywhere/simde .
//#include "./simd/primitives_128.hpp"
//#include "./simd/primitives_256.hpp"

//#include "./simd/simd_pri.h"
#include "./simd/uint16_8_t.hpp"
#include "./simd/uint32_8_t.hpp"


inline scrntype8_t make_rgba_scrntype8(uint16_8_t r, uint16_8_t g, uint16_8_t b, uint16_8_t a)
{
	__DECL_SCRNTYPE8_ALIGNED scrntype8_t tmp;
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		#if defined(_RGB555) || defined(_RGB565)
		tmp.u16[i] = RGBA_COLOR((uint8_t)(r.u16[i]), (uint8_t)(g.u16[i]), (uint8_t)(b.u16[i]), (uint8_t)(a.u16[i]));
		#else
		tmp.u32[i] = RGBA_COLOR((uint8_t)(r.u16[i]), (uint8_t)(g.u16[i]), (uint8_t)(b.u16[i]), (uint8_t)(a.u16[i]));
		#endif
	}
	return tmp;
}
inline scrntype8_t make_rgb_scrntype8(uint16_8_t r, uint16_8_t g, uint16_8_t b)
{
	__DECL_SCRNTYPE8_ALIGNED scrntype8_t tmp;
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		#if defined(_RGB555) || defined(_RGB565)
		tmp.u16[i] = RGBA_COLOR((uint8_t)(r.u16[i]), (uint8_t)(g.u16[i]), (uint8_t)(b.u16[i]), 255);
		#else
		tmp.u32[i] = RGBA_COLOR((uint8_t)(r.u16[i]), (uint8_t)(g.u16[i]), (uint8_t)(b.u16[i]), 255);
		#endif
	}
	return tmp;
}
// Please include type specified (and MPU specified) templates.


#undef  __LOOP_LOAD8
#undef  __LOOP_LOAD8_UNALIGNED
#undef  __LOOP_FILL8
#undef  __LOOP_FILL8_UNALIGNED
#undef __M__MINIMUM_ALIGN_LENGTH
