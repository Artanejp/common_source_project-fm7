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

inline simde__m128 op_clear()
{
	return simde_mm_setzero_si128();
}

inline simde__m128 op_setall()
{
	return simde_x_mm_setone_si128();
}

inline simde__m128 op_set8(const uint8_t __a)
{
	__DECL_ALIGNED(16) uint16_8_t __r;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 16; n++) {
		__r.u8[n] = __a;
	}
	return __r.v;
}

inline simde__m128 op_set16(const uint16_t __a)
{
	__DECL_ALIGNED(16) uint16_8_t __r;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 8; n++) {
		__r.u16[n] = __a;
	}
	return __r.v;
}

inline simde__m128 op_set32(const uint32_t __a)
{
	__DECL_ALIGNED(16) uint16_8_t __r;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 4; n++) {
		__r.u32[n] = __a;
	}
	return __r.v;
}
inline simde__m128 op_set64(const uint64_t __a)
{
	__DECL_ALIGNED(16) uint16_8_t __r;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 2; n++) {
		__r.u64[n] = __a;
	}
	return __r.v;
}

inline simde__m128 op_bswap16(const simde__m128 __a)
{
	__DECL_ALIGNED(16) uint16_8_t __r;
	__r.v = = __a;
	pair16_t __tmp;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 8; n++) {
		__tmp.w = __r.u16[n];
		__r.u16[n] = __tmp.swap_2bytes(__tmp.w);
	}
	return __r.v;
}

inline simde__m128 op_bswap32(const simde__m128 __a)
{
	__DECL_ALIGNED(16) uint16_8_t __r;
	__r.v = __a;
	pair32_t __tmp;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 4; n++) {
		__tmp.d = __r.u32[n];
		__r.u32[n] = __tmp.swap_4bytes(__tmp.d);
	}
	return __r.v;
}

inline simde__m128 op_bswap64(const simde__m128 __a)
{
	__DECL_ALIGNED(16) uint16_8_t __r;
	__r.v = __a;
	pair64_t __tmp;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 2; n++) {
		__tmp.q = __r.u64[n];
		__r.u64[n] = __tmp.swap_8bytes(__tmp.q);
	}
	return __r.v;
}

inline simde__m128 op_and(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_and_si128(__a, __b);
}
inline simde__m128 op_or(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_or_si128(__a, __b);
}
inline simde__m128 op_xor(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_xor_si128(__a, __b);
}

inline simde__m128 op_not(const simde__m128 __a);
{
	return simde_x_mm_not_ps(__a);
}
inline simde__m128 op_add_s8(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_add_epi8(__a, __b);
}

inline simde__m128 op_add_u8_sat(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_adds_epu8(__a, __b);
}

inline simde__m128 op_add_s8_sat(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_adds_epi8(__a, __b);
}

inline simde__m128 op_add_s16(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_add_epi16(__a, __b);
}

inline simde__m128 op_add_u16_sat(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_adds_epu16(__a, __b);
}

inline simde__m128 op_add_s16_sat(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_adds_epi16(__a, __b);
}

inline simde__m128 op_add_s32(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_add_epi32(__a, __b);
}

inline simde__m128 op_add_s64(const simde__m128& __a, const simde__m128& __b)
{
	return simde_mm_add_epi64(__a, __b);
}

inline simde__m128 op_sub_s8(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_sub_epi8(__a, __b);
}

inline simde__m128 op_sub_u8_sat(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_subs_epu8(__a, __b);
}

inline simde__m128 op_sub_s8_sat(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_subs_epi8(__a, __b);
}

inline simde__m128 op_sub_s16(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_sub_epi16(__a, __b);
}

inline simde__m128 op_sub_u16_sat(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_subs_epu16(__a, __b);
}

inline simde__m128 op_sub_s16_sat(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_subs_epi16(__a, __b);
}

inline simde__m128 op_sub_s32(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_add_epi32(__a, __b);
}

inline simde__m128 op_sub_s64(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_sub_epi64(__a, __b);
}

// 16bit, signed saturation add.
inline simde__m128& operator+(const simde__m128& __a, const simde__m128& __b)
{
	return op_add_s16_sat(__a, __b);
}

inline simde__m128& operator-(const simde__m128& __a, const simde__m128& __b)
{
	return simde_mm_subs_epi16(__a, __b);
}

inline simde__m128& operator&(const simde__m128& __a, const simde__m128& __b)
{
	return simde_mm_and_si128(__a, __b);
}

inline simde__m128& operator|(const simde__m128& __a, const simde__m128& __b)
{
	return simde_mm_or_si128(__a, __b);
}

inline simde__m128& operator^(const simde__m128& __a, const simde__m128& __b)
{
	return simde_mm_xor_si128(__a, __b);
}

inline simde__m128& operator~(const simde__m128& __a)
{
	return simde_x_mm_not_ps(__a);
}

inline simde__m128 op_lshift16(const simde__m128 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 16) ? 16 : (uint8_t)__shift;
	return simde_mm_slli_epi16(__a, __shift);
}
inline simde__m128 op_rshift16(const simde__m128 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 16) ? 16 : (uint8_t)__shift;
	return simde_mm_srli_epi16(__a, __shift);
}
inline simde__m128 op_rshift16_sign(const simde__m128 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 16) ? 16 : (uint8_t)__shift;
	return simde_mm_srai_epi16(__a, __shift);
}
inline simde__m128 op_lshift32(const simde__m128 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 32) ? 32 : (uint8_t)__shift;
	return simde_mm_slli_epi32(__a, __shift);
}
inline simde__m128 op_rshift32(const simde__m128 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 32) ? 32 : (uint8_t)__shift;
	return simde_mm_srli_epi32(__a, __shift);
}
inline simde__m128 op_rshift32_sign(const simde__m128 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 32) ? 32 : (uint8_t)__shift;
	return simde_mm_srai_epi32(__a, __shift);
}


inline simde__m128 op_lshift64(const simde__m128 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 64) ? 64 : (uint8_t)__shift;
	return simde_mm_slli_epi64(__a, __shift);
}
inline simde__m128 op_rshift64(const simde__m128 __a, const size_t __shift)
{
	const uint8_t _c = (__shift > 64) ? 64 : (uint8_t)__shift;
	return simde_mm_srli_epi64(__a, __shift);
}

inline simde__m128 op_byte_lshift(const simde__m128 __a, const size_t __bytes)
{
	const uint8_t _c = (__bytes > 16) ? 16 : (uint8_t)__bytes;
	return simde_mm_slli_si128(__a, __shift);
}
inline simde__m128 op_byte_rshift(const simde__m128 __a, const size_t __bytes)
{
	const uint8_t _c = (__bytes > 16) ? 16 : (uint8_t)__bytes;
	return simde_mm_srli_si128(__a, __shift);
}

inline simde__m128 op_equals8(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_cmpeq_epi8(__a, __b);
}

inline simde__m128 op_equals16(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_cmpeq_epi16(__a, __b);
}

inline simde__m128 op_equals32(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_cmpeq_epi32(__a, __b);
}

inline simde__m128 op_equals64(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_cmpeq_epi64(__a, __b);
}

// __a > __b (signed)
inline simde__m128 op_greater8(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_cmpgt_epi8(__a, __b);
}
inline simde__m128 op_greater16(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_cmpgt_epi16(__a, __b);
}
inline simde__m128 op_greater32(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_cmpgt_epi32(__a, __b);
}
inline simde__m128 op_greater64(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_cmpgt_epi8(__a, __b);
}

// __a < __b (signed)
inline simde__m128 op_lesser8(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_cmpgt_epi8(__b, __a);
}
inline simde__m128 op_lesser16(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_cmpgt_epi16(__b, __a);
}
inline simde__m128 op_lesser32(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_cmpgt_epi32(__b, __a);
}
inline simde__m128 op_lesser64(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_cmpgt_epi64(__b, __a);
}

// Note 16bit x 8.
inline simde__m128& operator<<(const simde__m128 __a, const size_t __shift)
{
	return op_lshift16(__a, __shift);
}

inline simde__m128& operator>>(const simde__m128 __a, const size_t __shift)
{
	return op_rshift16(__a, __shift);
}

// not(__a) and __b
inline simde__m128& op_andnot(const simde__m128& __a, const simde__m128& __b)
{
	return simde_mm_andnot_si128(__a, __b);
}
