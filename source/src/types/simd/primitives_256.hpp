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

namespace simd_256bit {
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

inline simde__m256 op_bswap16(simde__m256 __a)
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

inline simde__m256 op_bswap32(simde__m256  __a)
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

inline simde__m256 op_bswap64(simde__m256 __a)
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

inline simde__m256 op_not(const simde__m256 __a)
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

inline simde__m256 op_add_s64(const simde__m256 __a, const simde__m256 __b)
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

inline simde__m256 op_lshift16(const simde__m256 __a, const simde__m128i __shift)
{
	return simde_mm256_sll_epi16(__a, __shift);
}

inline simde__m256 op_lshift16(const simde__m256 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_lshift16(__a, _s);
}

inline simde__m256 op_lshift16_fix(const simde__m256 __a, const int __shift)
{
	return simde_mm256_slli_epi16(__a, __shift);
}
inline simde__m256 op_rshift16_fix(const simde__m256 __a, const int __shift)
{
	return simde_mm256_srli_epi16(__a, __shift);
}
inline simde__m256 op_rshift16_sign_fix(const simde__m256 __a, const int __shift)
{
	return simde_mm256_srai_epi16(__a, __shift);
}

inline simde__m256 op_rshift16(const simde__m256 __a, const simde__m128i __shift)
{
	return simde_mm256_srl_epi16(__a, __shift);
}

inline simde__m256 op_rshift16(const simde__m256 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_rshift16(__a, _s);
}

inline simde__m256 op_rshift16_sign(const simde__m256 __a, const simde__m128i __shift)
{
	return simde_mm256_sra_epi16(__a, __shift);
}

inline simde__m256 op_rshift16_sign(const simde__m256 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_rshift16_sign(__a, _s);
}

inline simde__m256 op_lshift32_fix(const simde__m256 __a, const int __shift)
{
	return simde_mm256_slli_epi32(__a, __shift);
}
inline simde__m256 op_rshift32_fix(const simde__m256 __a, const int __shift)
{
	return simde_mm256_srli_epi32(__a, __shift);
}
inline simde__m256 op_rshift32_sign_fix(const simde__m256 __a, const int __shift)
{
	return simde_mm256_srai_epi32(__a, __shift);
}

inline simde__m256 op_lshift32(const simde__m256 __a, const simde__m128i __shift)
{
	return simde_mm256_sll_epi32(__a, __shift);
}

inline simde__m256 op_lshift32(const simde__m256 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_lshift32(__a, _s);
}

inline simde__m256 op_rshift32(const simde__m256 __a, const simde__m128i __shift)
{
	return simde_mm256_srl_epi32(__a, __shift);
}

inline simde__m256 op_rshift32(const simde__m256 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_rshift32(__a, _s);
}

inline simde__m256 op_lshift64_fix(const simde__m256 __a, const int __shift)
{
	return simde_mm256_slli_epi64(__a, __shift);
}
inline simde__m256 op_rshift64_fix(const simde__m256 __a, const int __shift)
{
	return simde_mm256_srli_epi64(__a, __shift);
}
inline simde__m256 op_rshift32_sign(const simde__m256 __a, const simde__m128i __shift)
{
	return simde_mm256_sra_epi32(__a, __shift);
}

inline simde__m256 op_rshift32_sign(const simde__m256 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_rshift32_sign(__a, _s);
}

inline simde__m256 op_lshift64(const simde__m256 __a, const simde__m128i __shift)
{
	return simde_mm256_sll_epi64(__a, __shift);
}

inline simde__m256 op_lshift64(const simde__m256 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_lshift64(__a, _s);
}

inline simde__m256 op_rshift64(const simde__m256 __a, const simde__m128i __shift)
{
	return simde_mm256_srl_epi64(__a, __shift);
}

inline simde__m256 op_rshift64(const simde__m256 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_rshift64(__a, _s);
}

inline simde__m256 op_equals8(const simde__m256i __a, const simde__m256i __b)
{
	return simde_mm256_cmpeq_epi8(__a, __b);
}

inline simde__m256 op_equals16(const simde__m256i __a, const simde__m256i __b)
{
	return simde_mm256_cmpeq_epi16(__a, __b);
}

inline simde__m256 op_equals32(const simde__m256i __a, const simde__m256i __b)
{
	return simde_mm256_cmpeq_epi32(__a, __b);
}

inline simde__m256 op_equals64(const simde__m256i __a, const simde__m256i __b)
{
	return simde_mm256_cmpeq_epi64(__a, __b);
}

// __a > __b (signed)
inline simde__m256 op_greater8(const simde__m256i __a, const simde__m256i __b)
{
	return simde_mm256_cmpgt_epi8(__a, __b);
}
inline simde__m256 op_greater16(const simde__m256i __a, const simde__m256i __b)
{
	return simde_mm256_cmpgt_epi16(__a, __b);
}
inline simde__m256 op_greater32(const simde__m256i __a, const simde__m256i __b)
{
	return simde_mm256_cmpgt_epi32(__a, __b);
}
inline simde__m256 op_greater64(const simde__m256i __a, const simde__m256i __b)
{
	return simde_mm256_cmpgt_epi8(__a, __b);
}

// __a < __b (signed)
inline simde__m256 op_lesser8(const simde__m256i __a, const simde__m256i __b)
{
	return simde_mm256_cmpgt_epi8(__b, __a);
}
inline simde__m256 op_lesser16(const simde__m256i __a, const simde__m256i __b)
{
	return simde_mm256_cmpgt_epi16(__b, __a);
}
inline simde__m256 op_lesser32(const simde__m256i __a, const simde__m256i __b)
{
	return simde_mm256_cmpgt_epi32(__b, __a);
}
inline simde__m256 op_lesser64(const simde__m256i __a, const simde__m256 __b)
{
	return simde_mm256_cmpgt_epi64(__b, __a);
}


// not(__a) and __b
inline simde__m256 op_andnot(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_andnot_si256(__a, __b);
}

} /* namespace simd_256bit */
