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

namespace simd_128bit {
inline simde__m128 op_clear()
{
	return simde_mm_setzero_si128();
}

inline simde__m128 op_setall()
{
	return simde_x_mm_setone_si128();
}

inline simde__m128 load_aligned(simde__m128* p)
{
	return simde_mm_load_si128((simde__128i*)p);
}

inline simde__m128 load_unaligned(void* p)
{
	return simde_mm_loadu_si128(p);
}

inline void store_aligned(simde__m128* p, simde__m128 dat)
{
	simde_mm_store_si128((simde__128i*)p, (simde__128i)dat);
}

inline void store_unaligned(void* p, simde__m128 dat)
{
	simde_mm_storeu_si128((simde__128i*)p, (simde__128i)dat);
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

inline simde__m128 op_bswap16(simde__m128 __a)
{
	__DECL_ALIGNED(16) uint16_8_t __r;
	__r.v = __a;
	pair16_t __tmp;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 8; n++) {
		__tmp.w = __r.u16[n];
		__r.u16[n] = __tmp.swap_2bytes(__tmp.w);
	}
	return __r.v;
}

inline simde__m128 op_bswap32(simde__m128 __a)
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

inline simde__m128 op_bswap64(simde__m128 __a)
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

inline simde__m128 op_not(const simde__m128 __a)
{
	return simde_x_mm_not_ps(__a);
}

inline simde__m128 op_add_s8(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_add_epi8(__a, __b);
}

// signed/unsigned saturation add.
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

inline simde__m128 op_add_s64(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_add_epi64(__a, __b);
}

inline simde__m128 op_sub_s8(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_sub_epi8(__a, __b);
}

// signed/unsigned saturation add.
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


inline simde__m128 op_lshift16_fix(const simde__m128 __a, const int __shift)
{
	return simde_mm_slli_epi16(__a, __shift);
}
inline simde__m128 op_rshift16_fix(const simde__m128 __a, const int __shift)
{
	return simde_mm_srli_epi16(__a, __shift);
}
inline simde__m128 op_rshift16_sign_fix(const simde__m128i __a, const int __shift)
{
	return simde_mm_srai_epi16(__a, __shift);
}

inline simde__m128 op_lshift16(const simde__m128 __a, const simde__m128i __shift)
{
	return simde_mm_sll_epi16(__a, __shift);
}

inline simde__m128 op_lshift16(const simde__m128 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_lshift16(__a, _s);
}

inline simde__m128 op_rshift16(const simde__m128 __a, const simde__m128i __shift)
{
	return simde_mm_srl_epi16(__a, __shift);
}

inline simde__m128 op_rshift16(const simde__m128 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_rshift16(__a, _s);
}

inline simde__m128 op_rshift16_sign(const simde__m128 __a, const simde__m128i __shift)
{
	return simde_mm_sra_epi16(__a, __shift);
}

inline simde__m128 op_rshift16_sign(const simde__m128 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_rshift16_sign(__a, _s);
}


inline simde__m128 op_lshift32_fix(const simde__m128 __a, const int __shift)
{
	return simde_mm_slli_epi32(__a, __shift);
}
inline simde__m128 op_rshift32_fix(const simde__m128 __a, const int __shift)
{
	return simde_mm_srli_epi32(__a, __shift);
}
inline simde__m128 op_rshift32_sign_fix(const simde__m128i __a, const int __shift)
{
	return simde_mm_srai_epi32(__a, __shift);
}

inline simde__m128 op_lshift32(const simde__m128 __a, const simde__m128i __shift)
{
	return simde_mm_sll_epi32(__a, __shift);
}

inline simde__m128 op_lshift32(const simde__m128 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_lshift32(__a, _s);
}

inline simde__m128 op_rshift32(const simde__m128 __a, const simde__m128i __shift)
{
	return simde_mm_srl_epi32(__a, __shift);
}

inline simde__m128 op_rshift32(const simde__m128 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_rshift32(__a, _s);
}

inline simde__m128 op_rshift32_sign(const simde__m128 __a, const simde__m128i __shift)
{
	return simde_mm_sra_epi32(__a, __shift);
}

inline simde__m128 op_rshift32_sign(const simde__m128 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_rshift32_sign(__a, _s);
}

inline simde__m128 op_lshift64_fix(const simde__m128 __a, const int __shift)
{
	return simde_mm_slli_epi64(__a, __shift);
}
inline simde__m128 op_rshift64_fix(const simde__m128 __a, const int __shift)
{
	return simde_mm_srli_epi64(__a, __shift);
}
inline simde__m128 op_lshift64(const simde__m128 __a, const simde__m128i __shift)
{
	return simde_mm_sll_epi64(__a, __shift);
}

inline simde__m128 op_lshift64(const simde__m128 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_lshift64(__a, _s);
}

inline simde__m128 op_rshift64(const simde__m128 __a, const simde__m128i __shift)
{
	return simde_mm_srl_epi64(__a, __shift);
}

inline simde__m128 op_rshift64(const simde__m128 __a, const size_t __shift)
{
	__DECL_ALIGNED(16) simde__m128i _s;
	_s = simde_mm_cvtsi64_si128((int64_t)__shift);
	return op_rshift64(__a, _s);
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


// not(__a) and __b
inline simde__m128 op_andnot(const simde__m128 __a, const simde__m128 __b)
{
	return simde_mm_andnot_si128(__a, __b);
}

// Saturation ADD.
inline void op_saturation_add8_vec16(int16_t* dst, int16_t* src, simde__m128i tmp_zero)
{
	__DECL_ALIGNED(16) simde__m128i tmp_high;
	__DECL_ALIGNED(16) simde__m128i tmp_low;
	__DECL_ALIGNED(16) simde__m128i tmp_calc;
	__DECL_ALIGNED(16) simde__m128i dst_words_hi;
	tmp_high = load_unaligned(src);
	tmp_low  = load_unaligned(&(src[8]));
	tmp_src  = simde_mm_packs_epi32(tmp_low, tmp_high);
	
	tmp_high = load_unaligned(dst);
	tmp_low  = load_unaligned(&(dst[8]));
	tmp_calc = simde_mm_packs_epi32(tmp_low, tmp_high);
	tmp_low = op_add_s8_sat(tmp_calc, tmp_src);
	
	// 8bit -> 16bit
	// See https://officedaytime.com/tips/simd.html#tip3 .
	dst_words_hi = op_greater8(tmp_zero, tmp_calc);
	tmp_high = simde_mm_unpackhi_epi16(tmp_calc, dst_words_hi);
	tmp_low  = simde_mm_unpacklo_epi16(tmp_calc, dst_words_hi);
	store_unaligned(dst, tmp_high);
	store_unaligned(&(dst[8]), tmp_low);
}
	
inline void op_saturation_add16_vec8(int32_t* dst, int32_t* src, simde__m128i tmp_zero)
{
	__DECL_ALIGNED(16) simde__m128i tmp_high;
	__DECL_ALIGNED(16) simde__m128i tmp_low;
	__DECL_ALIGNED(16) simde__m128i tmp_calc;
	__DECL_ALIGNED(16) simde__m128i dst_words_hi;
	tmp_high = load_unaligned(src);
	tmp_low  = load_unaligned(&(src[4]));
	tmp_src  = simde_mm_packs_epi32(tmp_low, tmp_high);
	
	tmp_high = load_unaligned(dst);
	tmp_low  = load_unaligned(&(dst[4]));
	tmp_calc = simde_mm_packs_epi32(tmp_low, tmp_high);
	tmp_low = op_add_s16_sat(tmp_calc, tmp_src);
	
	// 16bit -> 32bit
	// See https://officedaytime.com/tips/simd.html#tip3 .
	dst_words_hi = op_greater16(tmp_zero, tmp_calc);
	tmp_high = simde_mm_unpackhi_epi16(tmp_calc, dst_words_hi);
	tmp_low  = simde_mm_unpacklo_epi16(tmp_calc, dst_words_hi);
	store_unaligned(dst, tmp_high);
	store_unaligned(&(dst[4]), tmp_low);
}


} /* namespace simd_128bit */

inline size_t op_saturation_add8_multiple(int16_t* dst, int16_t* src, size_t words)
{
	__UNLIKELY_IF((words == 0) || (dst == nullptr) || (src == nullptr)) {
		return 0;
	}
	size_t np;
	__DECL_ALIGNED(16) simde__m128i tmp_zero;
	tmp_zero = op_clear();
	for(np = 0; np < words; np += 16) {
		simd_128bit::op_saturation_add8_vec16(&(dst[np]), &(src[np]), tmp_zero);
	}
	if((words & 15) != 0) {
		if(np < 16) {
			np = 16;
		}
		int16_t* p = &(src[np - 16]);
		int16_t* q = &(src[np - 16]);
		size_t rwords = words & 7;
		for(size_t n = 0; n < rwords; n++) {
			int16_t tmp = p[n];
			tmp += q[n];
			__UNLIKELY_IF(tmp < -128) {
				tmp = -128;
			} else if(tmp > 127) {
				tmp = 127;
			}
			q[n] = tmp;
		}
	}
	return words;
}

inline size_t op_saturation_add16_multiple(int32_t* dst, int32_t* src, size_t words)
{
	__UNLIKELY_IF((words == 0) || (dst == nullptr) || (src == nullptr)) {
		return 0;
	}
	size_t np;
	__DECL_ALIGNED(16) simde__m128i tmp_zero;
	tmp_zero = op_clear();
	for(np = 0; np < words; np += 8) {
		simd_128bit::op_saturation_add16_vec8(&(dst[np]), &(src[np]), tmp_zero);
	}
	if((words & 7) != 0) {
		if(np < 8) {
			np = 8;
		}
		int32_t* p = &(src[np - 8]);
		int32_t* q = &(src[np - 8]);
		size_t rwords = words & 7;
		for(size_t n = 0; n < rwords; n++) {
			int32_t tmp = p[n];
			tmp += q[n];
			__UNLIKELY_IF(tmp < -32768) {
				tmp = -32768;
			} else if(tmp > 32767) {
				tmp = 32767;
			}
			q[n] = tmp;
		}
	}
	return words;
}

