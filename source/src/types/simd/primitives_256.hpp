/*
 * SIMD Wrapper for 128bit : Primitive operations.
 * ©2026 Kyuma Ohta <whatisthis.sowhat@gmail.com>
 *
 * Pre-Require: SIMD Everywhere a.k.a SIMDE.
 *              See https://github.com/simd-everywhere/simde .
 *
 * History:
 *         Feb 18, 2026 : Initial.
 *
 */
#pragma once

// #include "../simd_types.h"


inline simde__m256 op_clear()
{
	return simde_mm256_setzero_si256();
}

inline simde__m256 op_setall()
{
	return simde_x_mm256_setone_si256();
}

inline simde__m256 op_set8(const uint8_t __a)
{
	__DECL_ALIGNED(32) uint32_8_t __r;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 32; n++) {
		__r.u8[n] = __a;
	}
	return __r.v;
}

inline simde__m256 op_set16(const uint16_t __a)
{
	__DECL_ALIGNED(32) uint32_8_t __r;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 16; n++) {
		__r.u16[n] = __a;
	}
	return __r.v;
}

inline simde__m256 op_set32(const uint32_t __a)
{
	__DECL_ALIGNED(32) uint32_8_t __r;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 8; n++) {
		__r.u32[n] = __a;
	}
	return __r.v;
}
inline simde__m256 op_set64(const uint64_t __a)
{
	__DECL_ALIGNED(32) uint32_8_t __r;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 4; n++) {
		__r.u64[n] = __a;
	}
	return __r.v;
}

inline simde__m256 op_bswap16(const simde__m256 __a)
{
	__DECL_ALIGNED(32) uint32_8_t __r;
	__r.v = __a;
	pair16_t __tmp;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 16; n++) {
		__tmp.w = __r.u16[n];
		__r.u16[n] = __tmp.swap_2bytes(__tmp.w);
	}
	return __r.v;
}

inline simde__m256 op_bswap32(const simde__m256  __a)
{
	__DECL_ALIGNED(32) uint32_8_t __r;
	__r.v = __a;
	pair32_t __tmp;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 8; n++) {
		__tmp.d = __r.u32[n];
		__r.u32[n] = __tmp.swap_4bytes(__tmp.d);
	}
	return __r.v;
}

inline simde__m256 op_bswap64(const simde__m256 __a)
{
	__DECL_ALIGNED(32) uint32_8_t __r;
	__r.v = __a;
	pair64_t __tmp;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 4; n++) {
		__tmp.q = __r.u64[n];
		__r.u64[n] = __tmp.swap_8bytes(__tmp.q);
	}
	return __r.v;
}

// 256bit wide
inline simde__m256 op_and(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_and_si256(__a, __b);
}
inline simde__m256 op_or(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_or_si256(__a, __b);
}
inline simde__m256 op_xor(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_xor_si256(__a, __b);
}

inline simde__m256 op_not(const simde__m256 __a);
{
	return simde_x_mm256_not_ps(__a);
}
inline simde__m256 op_add_s8(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_add_epi8(__a, __b);
}

inline simde__m256 op_add_u8_sat(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_adds_epu8(__a, __b);
}

inline simde__m256 op_add_s8_sat(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_adds_epi8(__a, __b);
}

inline simde__m256 op_add_s16(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_add_epi16(__a, __b);
}

inline simde__m256 op_add_u16_sat(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_adds_epu16(__a, __b);
}

inline simde__m256 op_add_s16_sat(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_adds_epi16(__a, __b);
}

inline simde__m256 op_add_s32(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_add_epi32(__a, __b);
}

inline simde__m256 op_add_s64(const simde__m256& __a, const simde__m256& __b)
{
	return simde_mm256_add_epi64(__a, __b);
}

inline simde__m256 op_sub_s8(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_sub_epi8(__a, __b);
}

inline simde__m256 op_sub_u8_sat(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_subs_epu8(__a, __b);
}

inline simde__m256 op_sub_s8_sat(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_subs_epi8(__a, __b);
}

inline simde__m256 op_sub_s16(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_sub_epi16(__a, __b);
}

inline simde__m256 op_sub_u16_sat(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_subs_epu16(__a, __b);
}

inline simde__m256 op_sub_s16_sat(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_subs_epi16(__a, __b);
}

inline simde__m256 op_sub_s32(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_add_epi32(__a, __b);
}

inline simde__m256 op_sub_s64(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_sub_epi64(__a, __b);
}

// 16bit, signed saturation add.
inline simde__m256& operator+(const simde__m256& __a, const simde__m256& __b)
{
	return op_add_s16_sat(__a, __b);
}

inline simde__m256& operator-(const simde__m256& __a, const simde__m256& __b)
{
	return op_sub_s16_sat(__a, __b);
}

inline simde__m256& operator&(const simde__m256& __a, const simde__m256& __b)
{
	return op_and(__a, __b);
}

inline simde__m256& operator|(const simde__m256& __a, const simde__m256& __b)
{
	return op_or(__a, __b);
}

inline simde__m256& operator^(const simde__m256& __a, const simde__m256& __b)
{
	return op_xor(__a, __b);
}

inline simde__m256& operator~(const simde__m256& __a)
{
	return op_not(__a);
}

inline simde__m256 op_lshift16(const simde__m256 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 16) ? 16 : (uint8_t)__shift;
	return simde_mm256_slli_epi16(__a, __shift);
}
inline simde__m256 op_rshift16(const simde__m256 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 16) ? 16 : (uint8_t)__shift;
	return simde_mm256_srli_epi16(__a, __shift);
}
inline simde__m256 op_rshift16_sign(const simde__m256 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 16) ? 16 : (uint8_t)__shift;
	return simde_mm256_srai_epi16(__a, __shift);
}
inline simde__m256 op_lshift32(const simde__m256 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 32) ? 32 : (uint8_t)__shift;
	return simde_mm256_slli_epi32(__a, __shift);
}
inline simde__m256 op_rshift32(const simde__m256 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 32) ? 32 : (uint8_t)__shift;
	return simde_mm256_srli_epi32(__a, __shift);
}
inline simde__m256 op_rshift32_sign(const simde__m256 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 32) ? 32 : (uint8_t)__shift;
	return simde_mm256_srai_epi32(__a, __shift);
}


inline simde__m256 op_lshift64(const simde__m256 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 64) ? 64 : (uint8_t)__shift;
	return simde_mm256_slli_epi64(__a, __shift);
}
inline simde__m256 op_rshift64(const simde__m256 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 64) ? 64 : (uint8_t)__shift;
	return simde_mm256_srli_epi64(__a, __shift);
}

inline simde__m256 op_byte_lshift(const simde__m256 __a, const size_t __bytes)
{
	const uint8_t _c = (__bytes > 32) ? 32 : (uint8_t)__bytes;
	return simde_mm256_slli_si256(__a, __shift);
}
inline simde__m256 op_byte_rshift(const simde__m256 __a, const size_t __bytes)
{
	const uint8_t _c = (__bytes > 32) ? 32 : (uint8_t)__bytes;
	return simde_mm256_srli_si256(__a, __shift);
}

// Note 16bit x 8.
inline simde__m256& operator<<(const simde__m256 __a, const size_t __shift)
{
	return op_lshift16(__a, __shift);
}

inline simde__m256& operator>>(const simde__m256 __a, const size_t __shift)
{
	return op_rshift16(__a, __shift);
}

// not(__a) and __b
inline simde__m256 op_andnot(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_andnot_si256(__a, __b);
}
