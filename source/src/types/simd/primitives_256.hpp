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

/*
 * Utilities
 */
template <typename T>
	inline const size_t vector_width(T _eval)
{
	__UNLIKELY_IF(sizeof(T) == 0) {
		return 0;
	}
	return sizeof(simde__m256) / sizeof(T);
}

template <typename T>
	inline const size_t vector_mod(T _eval)
{
	__UNLIKELY_IF(sizeof(T) == 0) {
		return 0;
	}
	return sizeof(simde__m256) % sizeof(T);
}

inline const bool is_aligned(void *p)
{
	const uintptr_t pd = (const uintptr_t)p;
	const uintptr_t mask = sizeof(simde__m256) - 1;  // ToDo: for not 2^n . 20260218 K.O
	return ((pd & mask) == 0) ? true : false;
}

/*
 * Zero ALL bits.
 */
inline simde__m256 op_clear()
{
	return simde_mm256_setzero_si256();
}

/*
 * Set ALL bits.
 */
inline simde__m256 op_setall()
{
	return simde_x_mm256_setone_si256();
}

/*
 * LOAD/STORE
 */
inline simde__m256 load_aligned(simde__m256* p)
{
	return simde_mm256_load_si256((simde__m256i*)p);
}

inline simde__m256 load_unaligned(void* p)
{
	return simde_mm256_loadu_si256(p);
}

inline void store_aligned(simde__m256* p, simde__m256 dat)
{
	simde_mm256_store_si256((simde__m256i*)p, (simde__m256i)dat);
}

inline void store_unaligned(void* p, simde__m256 dat)
{
	simde_mm256_storeu_si256((simde__m256i*)p, (simde__m256i)dat);
}
	
/*
 * Fill all cells by one value (int)
 */	
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

inline simde__m256 op_set_scrntype(const scrntype_t __a)
{
	return op_set32((const uint32_t)__a);
}
/*
 * Fill all cells by one value (float)
 */
inline simde__m256 op_set_float32(const float __a)
{
	__DECL_ALIGNED(32) uint16_16_t __r;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 8; n++) {
		__r._fp32[n] = __a;
	}
	return __r.v;
}

inline simde__m256 op_set_float64(const double __a)
{
	__DECL_ALIGNED(32) uint16_16_t __r;
	SIMDE_VECTORIZE
	for(size_t n = 0; n < 4; n++) {
		__r._fp64[n] = __a;
	}
	return __r.v;
}
	
/*
 * Byte (endian) SWAP OPs
 */
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

/*
  Logical OPs
*/
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

// not(__a) and __b
inline simde__m256 op_andnot(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_andnot_si256(__a, __b);
}

/*
  Integer arithmetical OPs
*/
inline simde__m256 op_add_s8(const simde__m256 __a, const simde__m256 __b)
{
	return simde_mm256_add_epi8(__a, __b);
}

// signed/unsigned saturation add.
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
	
/*
 * Shift OPs
 */
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

	
/* For byte shift: You should call as template.
   This seems to relation of clang ?
   - 20260314 K.O */
template <const int __bytes> 	
	inline simde__m256 op_rshift_bytes(simde__m256 __a)
{
	return simde_mm256_srli_si256(__a, __bytes);
}

template <const int __bytes> 	
	inline simde__m256 op_lshift_bytes(simde__m256 __a)
{
	return simde_mm256_slli_si256(__a, __bytes);
}

// SSSE3
template <const int __bytes> 
	inline simde__m256 op_rshift_packed_bytes(simde__m256 __hi, simde__m256 __lo)
{
	return simde_mm256_alignr_epi8(__hi, __lo, __bytes);
}
	

/*
  Compare (SIGNED INTEGER)
*/
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
inline simde__m256 op_not_equals8(const simde__m256i __a, const simde__m256i __b)
{
	return op_not(simde_mm256_cmpeq_epi8(__a, __b));
}

inline simde__m256 op_not_equals16(const simde__m256i __a, const simde__m256i __b)
{
	return op_not(simde_mm256_cmpeq_epi16(__a, __b));
}

inline simde__m256 op_not_equals32(const simde__m256i __a, const simde__m256i __b)
{
	return op_not(simde_mm256_cmpeq_epi32(__a, __b));
}

inline simde__m256 op_not_equals64(const simde__m256i __a, const simde__m256i __b)
{
	return op_not(simde_mm256_cmpeq_epi64(__a, __b));
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



constexpr size_t copy_multiple(const void* dst, const void* src, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (src == nullptr) || (vec8_words == 0)) {
		return 0;
	}
	const bool src_aligned = is_aligned((void *)src);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* src2 = (simde__m256*)src;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256i tmp;
	for(size_t n = 0; n < vec8_words; n++) {
		if(src_aligned) {
			tmp = load_aligned(&(src2[n]));
		} else {
			tmp = load_unaligned(&(src2[n]));
		}
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec8_words;
}

// __dst &= src
constexpr size_t op_and_multiple(const void* dst, const void* src, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (src == nullptr) || (vec8_words == 0)) {
		return 0;
	}
	const bool src_aligned = is_aligned((void *)src);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* src2 = (simde__m256*)src;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_src;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_dst;
	for(size_t n = 0; n < vec8_words; n++) {
		if(src_aligned) {
			tmp_src = load_aligned(&(src2[n]));
		} else {
			tmp_src = load_unaligned(&(src2[n]));
		}
		if(dst_aligned) {
			tmp_dst = load_aligned(&(dst2[n]));
			store_aligned(&(dst2[n]), op_and(tmp_src, tmp_dst));
		} else {
			tmp_dst = load_unaligned(&(dst2[n]));
			store_unaligned(&(dst2[n]), op_and(tmp_src, tmp_dst));
		}
	}
	return vec8_words;
}
	
// __dst |= src
constexpr size_t op_or_multiple(const void* dst, const void* src, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (src == nullptr) || (vec8_words == 0)) {
		return 0;
	}
	const bool src_aligned = is_aligned((void *)src);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* src2 = (simde__m256*)src;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_src;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_dst;
	for(size_t n = 0; n < vec8_words; n++) {
		if(src_aligned) {
			tmp_src = load_aligned(&(src2[n]));
		} else {
			tmp_src = load_unaligned(&(src2[n]));
		}
		if(dst_aligned) {
			tmp_dst = load_aligned(&(dst2[n]));
			store_aligned(&(dst2[n]), op_or(tmp_src, tmp_dst));
		} else {
			tmp_dst = load_unaligned(&(dst2[n]));
			store_unaligned(&(dst2[n]), op_or(tmp_src, tmp_dst));
		}
	}
	return vec8_words;
}

// __dst ^= src
constexpr size_t op_xor_multiple(const void* dst, const void* src, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (src == nullptr) || (vec8_words == 0)) {
		return 0;
	}
	const bool src_aligned = is_aligned((void *)src);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* src2 = (simde__m256*)src;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_src;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_dst;
	for(size_t n = 0; n < vec8_words; n++) {
		if(src_aligned) {
			tmp_src = load_aligned(&(src2[n]));
		} else {
			tmp_src = load_unaligned(&(src2[n]));
		}
		if(dst_aligned) {
			tmp_dst = load_aligned(&(dst2[n]));
			store_aligned(&(dst2[n]), op_xor(tmp_src, tmp_dst));
		} else {
			tmp_dst = load_unaligned(&(dst2[n]));
			store_unaligned(&(dst2[n]), op_xor(tmp_src, tmp_dst));
		}
	}
	return vec8_words;
}

// __dst = src & not(mask)
constexpr size_t op_andnot_multiple(const void* dst, const void* src, const void* mask, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (src == nullptr) || (mask == nullptr)  || (vec8_words == 0)) {
		return 0;
	}
	const bool src_aligned = is_aligned((void *)src);
	const bool dst_aligned = is_aligned((void *)dst);
	const bool mask_aligned = is_aligned((void *)mask);
	simde__m256* src2 = (simde__m256*)src;
	simde__m256* dst2 = (simde__m256*)dst;
	simde__m256* mask2 = (simde__m256*)mask;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_src;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_mask;
	for(size_t n = 0; n < vec8_words; n++) {
		if(src_aligned) {
			tmp_src = load_aligned(&(src2[n]));
		} else {
			tmp_src = load_unaligned(&(src2[n]));
		}
		if(mask_aligned) {
			tmp_mask = load_aligned(&(mask2[n]));
		} else {
			tmp_mask = load_unaligned(&(mask2[n]));
		}
		if(dst_aligned) {
			store_aligned(&(dst2[n]), op_andnot(tmp_mask, tmp_src));
		} else {
			store_unaligned(&(dst2[n]), op_andnot(tmp_mask, tmp_src));
		}
	}
	return vec8_words;
}

// __a == __b => dst
constexpr size_t eval_equals8_multiple(const void* dst, const void* __a, const void* __b, const size_t vec32_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (__b == nullptr)  || (vec32_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool __b_aligned = is_aligned((void *)__b);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* b2 = (simde__m256*)__b;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_b;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec32_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		if(__b_aligned) {
			tmp_b = load_aligned(&(b2[n]));
		} else {
			tmp_b = load_unaligned(&(b2[n]));
		}
		tmp = op_equals8(tmp_a, tmp_b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec32_words;
}

constexpr size_t eval_not_equals8_multiple(const void* dst, const void* __a, const void* __b, const size_t vec32_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (__b == nullptr)  || (vec32_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool __b_aligned = is_aligned((void *)__b);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* b2 = (simde__m256*)__b;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_b;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec32_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		if(__b_aligned) {
			tmp_b = load_aligned(&(b2[n]));
		} else {
			tmp_b = load_unaligned(&(b2[n]));
		}
		tmp = op_not_equals8(tmp_a, tmp_b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec32_words;
}

constexpr size_t eval_equals8(const void* dst, const void* __a, const simde__m256 __b, const size_t vec32_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec32_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec32_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_equals8(tmp_a, __b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec32_words;
}
constexpr size_t eval_not_equals8(const void* dst, const void* __a, const simde__m256 __b, const size_t vec32_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec32_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec32_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_not_equals8(tmp_a, __b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec32_words;
}

constexpr size_t eval_equals16_multiple(const void* dst, const void* __a, const void* __b, const size_t vec16_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (__b == nullptr)  || (vec16_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool __b_aligned = is_aligned((void *)__b);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* b2 = (simde__m256*)__b;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_b;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec16_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		if(__b_aligned) {
			tmp_b = load_aligned(&(b2[n]));
		} else {
			tmp_b = load_unaligned(&(b2[n]));
		}
		tmp = op_equals16(tmp_a, tmp_b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec16_words;
}

constexpr size_t eval_not_equals16_multiple(const void* dst, const void* __a, const void* __b, const size_t vec16_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (__b == nullptr)  || (vec16_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool __b_aligned = is_aligned((void *)__b);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* b2 = (simde__m256*)__b;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_b;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec16_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		if(__b_aligned) {
			tmp_b = load_aligned(&(b2[n]));
		} else {
			tmp_b = load_unaligned(&(b2[n]));
		}
		tmp = op_not_equals16(tmp_a, tmp_b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec16_words;
}

constexpr size_t eval_equals16(const void* dst, const void* __a, const simde__m256 __b, const size_t vec16_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec16_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec16_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_equals16(tmp_a, __b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec16_words;
}

constexpr size_t eval_not_equals16(const void* dst, const void* __a, const simde__m256 __b, const size_t vec16_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec16_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec16_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_not_equals16(tmp_a, __b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec16_words;
}

constexpr size_t eval_equals32_multiple(const void* dst, const void* __a, const void* __b, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (__b == nullptr)  || (vec8_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool __b_aligned = is_aligned((void *)__b);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* b2 = (simde__m256*)__b;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_b;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec8_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		if(__b_aligned) {
			tmp_b = load_aligned(&(b2[n]));
		} else {
			tmp_b = load_unaligned(&(b2[n]));
		}
		tmp = op_equals32(tmp_a, tmp_b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec8_words;
}

constexpr size_t eval_not_equals32_multiple(const void* dst, const void* __a, const void* __b, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (__b == nullptr)  || (vec8_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool __b_aligned = is_aligned((void *)__b);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* b2 = (simde__m256*)__b;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_b;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec8_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		if(__b_aligned) {
			tmp_b = load_aligned(&(b2[n]));
		} else {
			tmp_b = load_unaligned(&(b2[n]));
		}
		tmp = op_not_equals32(tmp_a, tmp_b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec8_words;
}

constexpr size_t eval_equals32(const void* dst, const void* __a, const simde__m256 __b, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec8_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec8_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_equals32(tmp_a, __b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec8_words;
}

constexpr size_t eval_not_equals32(const void* dst, const void* __a, const simde__m256 __b, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec8_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec8_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_not_equals32(tmp_a, __b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec8_words;
}

// __a > __b => dst [signed]
constexpr size_t eval_greater8_multiple(const void* dst, const void* __a, const void* __b, const size_t vec32_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (__b == nullptr)  || (vec32_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool __b_aligned = is_aligned((void *)__b);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* b2 = (simde__m256*)__b;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_b;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec32_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		if(__b_aligned) {
			tmp_b = load_aligned(&(b2[n]));
		} else {
			tmp_b = load_unaligned(&(b2[n]));
		}
		tmp = op_greater8(tmp_a, tmp_b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec32_words;
}
constexpr size_t eval_greater8(const void* dst, const void* __a, const simde__m256 __b, const size_t vec32_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec32_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec32_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_greater8(tmp_a, __b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec32_words;
}

constexpr size_t eval_lesser8(const void* dst, const void* __a, const simde__m256 __b, const size_t vec32_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec32_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec32_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_greater8(__b, tmp_a);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec32_words;
}
	
constexpr size_t eval_greater16_multiple(const void* dst, const void* __a, const void* __b, const size_t vec16_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (__b == nullptr)  || (vec16_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool __b_aligned = is_aligned((void *)__b);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* b2 = (simde__m256*)__b;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_b;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec16_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		if(__b_aligned) {
			tmp_b = load_aligned(&(b2[n]));
		} else {
			tmp_b = load_unaligned(&(b2[n]));
		}
		tmp = op_greater16(tmp_a, tmp_b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec16_words;
}

constexpr size_t eval_greater16(const void* dst, const void* __a, const simde__m256 __b, const size_t vec16_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec16_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec16_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_greater16(tmp_a, __b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec16_words;
}

constexpr size_t eval_lesser16(const void* dst, const void* __a, const simde__m256 __b, const size_t vec16_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec16_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec16_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_greater16(__b, tmp_a);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec16_words;
}
	
constexpr size_t eval_greater32_multiple(const void* dst, const void* __a, const void* __b, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (__b == nullptr)  || (vec8_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool __b_aligned = is_aligned((void *)__b);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* b2 = (simde__m256*)__b;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_b;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec8_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		if(__b_aligned) {
			tmp_b = load_aligned(&(b2[n]));
		} else {
			tmp_b = load_unaligned(&(b2[n]));
		}
		tmp = op_greater32(tmp_a, tmp_b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec8_words;
}

constexpr size_t eval_greater32(const void* dst, const void* __a, const simde__m256 __b, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec8_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec8_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_greater32(tmp_a, __b);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec8_words;
}

constexpr size_t eval_lesser32(const void* dst, const void* __a, const simde__m256 __b, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec8_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec8_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_greater32(__b, tmp_a);
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec8_words;
}

constexpr size_t eval_lesser_equals8(const void* dst, const void* __a, const simde__m256 __b, const size_t vec32_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec32_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec32_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_not(op_greater8(tmp_a, __b));
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec32_words;
}

constexpr size_t eval_greater_equals8(const void* dst, const void* __a, const simde__m256 __b, const size_t vec32_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec32_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec32_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_not(op_greater8(__b, tmp_a));
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec32_words;
}

constexpr size_t eval_lesser_equals16(const void* dst, const void* __a, const simde__m256 __b, const size_t vec16_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec16_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec16_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_not(op_greater16(tmp_a, __b));
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec16_words;
}

constexpr size_t eval_greater_equals16(const void* dst, const void* __a, const simde__m256 __b, const size_t vec16_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec16_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec16_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_not(op_greater16(__b, tmp_a));
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec16_words;
}

constexpr size_t eval_lesser_equals32(const void* dst, const void* __a, const simde__m256 __b, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec8_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec8_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_not(op_greater32(tmp_a, __b));
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec8_words;
}

constexpr size_t eval_greater_equals32(const void* dst, const void* __a, const simde__m256 __b, const size_t vec8_words = 1)
{
	__UNLIKELY_IF((dst == nullptr) || (__a == nullptr) || (vec8_words == 0)) {
		return 0;
	}
	const bool __a_aligned = is_aligned((void *)__a);
	const bool dst_aligned = is_aligned((void *)dst);
	simde__m256* a2 = (simde__m256*)__a;
	simde__m256* dst2 = (simde__m256*)dst;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp_a;
	__DECL_ALIGNED(sizeof(simde__m256)) simde__m256 tmp;
	for(size_t n = 0; n < vec8_words; n++) {
		if(__a_aligned) {
			tmp_a = load_aligned(&(a2[n]));
		} else {
			tmp_a = load_unaligned(&(a2[n]));
		}
		tmp = op_not(op_greater32(__b, tmp_a));
		if(dst_aligned) {
			store_aligned(&(dst2[n]), tmp);
		} else {
			store_unaligned(&(dst2[n]), tmp);
		}
	}
	return vec8_words;
}
	
inline size_t eval_eq8(const void* dst, const void* __a, const simde__m256 __b, const size_t vec32_words = 1)
{
	return eval_equals8(dst, __a, __b, vec32_words);
}

inline size_t eval_eq16(const void* dst, const void* __a, const simde__m256 __b, const size_t vec16_words = 1)
{
	return eval_equals16(dst, __a, __b, vec16_words);
}

inline size_t eval_eq32(const void *dst, const void *__a, const simde__m256 __b, const size_t vec8_words = 1)
{
	return eval_equals32(dst, __a, __b, vec8_words);
}
inline size_t eval_ne8(const void* dst, const void* __a, const simde__m256 __b, const size_t vec32_words = 1)
{
	return eval_not_equals8(dst, __a, __b, vec32_words);
}

inline size_t eval_ne16(const void* dst, const void* __a, const simde__m256 __b, const size_t vec16_words = 1)
{
	return eval_not_equals16(dst, __a, __b, vec16_words);
}

inline size_t eval_ne32(const void *dst, const void *__a, const simde__m256 __b, const size_t vec8_words = 1)
{
	return eval_not_equals32(dst, __a, __b, vec8_words);
}
	
inline size_t eval_gt8(const void* dst, const void* __a, const simde__m256 __b, const size_t vec32_words = 1)
{
	return eval_greater8(dst, __a, __b, vec32_words);
}

// __a < __b => dst	
inline size_t eval_lt8(const void* dst, const void* __a, const simde__m256 __b, const size_t vec32_words = 1)
{
	return eval_lesser8(dst, __a, __b, vec32_words);
}

inline size_t eval_ge8(const void* dst, const void* __a, const simde__m256 __b, const size_t vec32_words = 1)
{
	return eval_greater_equals8(dst, __a, __b, vec32_words);
}

inline size_t eval_le8(const void* dst, const void* __a, const simde__m256 __b, const size_t vec32_words = 1)
{
	return eval_lesser_equals8(dst, __a, __b, vec32_words);
}
//
inline size_t eval_gt16(const void* dst, const void* __a, const simde__m256 __b, const size_t vec16_words = 1)
{
	return eval_greater16(dst, __a, __b, vec16_words);
}
inline size_t eval_lt16(const void* dst, const void* __a, const simde__m256 __b, const size_t vec16_words = 1)
{
	return eval_lesser16(dst, __a, __b, vec16_words);
}

inline size_t eval_ge16(const void* dst, const void* __a, const simde__m256 __b, const size_t vec16_words = 1)
{
	return eval_greater_equals16(dst, __a, __b, vec16_words);
}

inline size_t eval_le16(const void* dst, const void* __a, const simde__m256 __b, const size_t vec16_words = 1)
{
	return eval_lesser_equals16(dst, __a, __b, vec16_words);
}
//
inline size_t eval_gt32(const void* dst, const void* __a, const simde__m256 __b, const size_t vec8_words = 1)
{
	return eval_greater32(dst, __a, __b, vec8_words);
}
inline size_t eval_lt32(const void* dst, const void* __a, const simde__m256 __b, const size_t vec8_words = 1)
{
	return eval_lesser32(dst, __a, __b, vec8_words);
}

inline size_t eval_ge32(const void* dst, const void* __a, const simde__m256 __b, const size_t vec8_words = 1)
{
	return eval_greater_equals32(dst, __a, __b, vec8_words);
}

inline size_t eval_le32(const void* dst, const void* __a, const simde__m256 __b, const size_t vec8_words = 1)
{
	return eval_lesser_equals32(dst, __a, __b, vec8_words);
}
	
/*
 * Lookup tables (a.k.a Gather) OPs.
 */

inline simde__m256i make_table32x4_from_uint8(uint8_t* p)
{
	__DECL_ALIGNED(32) uint16_16_t _tbl;
	SIMDE_VECTORIZE
	for(size_t i = 0; i < 8; i++) {
		_tbl.u32[i] = p[i];
	}
	return _tbl.v;
}

inline simde__m256i make_table64x4_from_uint8(uint8_t* p)
{
	__DECL_ALIGNED(32) uint16_16_t _tbl;
	SIMDE_VECTORIZE
	for(size_t i = 0; i < 4; i++) {
		_tbl.u64[i] = p[i];
	}
	return _tbl.v;
}

inline simde__m256i make_table32x8_from_uint16(uint16_t* p)
{
	__DECL_ALIGNED(32) uint16_16_t  _tbl;
	SIMDE_VECTORIZE
	for(size_t i = 0; i < 8; i++) {
		_tbl.u32[i] = p[i];
	}
	return _tbl.v;
}

inline simde__m256i make_table64x4_from_uint16(uint16_t* p)
{
	__DECL_ALIGNED(32) uint16_16_t _tbl;
	//_src.v = op_clear();
	SIMDE_VECTORIZE
	for(size_t i = 0; i < 4; i++) {
		_tbl.u64[i] = p[i];
	}
	return _tbl.v;
}

// Get from 8x32bit elements from table.
template <int tbl_scale>
	inline simde__m256i op_lookup_int32(const int32_t* tbl, const simde__m256i __index)
{
	return simde_mm256_i32gather_epi32(tbl, __index, tbl_scale);
}

template <int tbl_scale>
	inline simde__m256i op_lookup_selectable_int32(const simde__m256i src_vec, const int32_t* tbl, const simde__m256i __index, const simde__m256i mask_vec)
{
	return simde_mm256_mask_i32gather_epi32(src_vec, tbl, __index, mask_vec, tbl_scale);
}

template <int tbl_scale>
	inline simde__m256i op_lookup_selectable_int32(const simde__m256i src_vec, const int32_t* tbl, const simde__m256i __index, const bool masks[8])
{
	__DECL_ALIGNED(32) uint16_16_t mask_vec;
	SIMDE_VECTORIZE
	for(size_t i = 0; i < 8; i++) {
		mask_vec.u32[i] = (masks[i]) ? UINT32_MAX : 0;
	}
	return simde_mm256_mask_i32gather_epi32(src_vec, tbl, __index, mask_vec.v, tbl_scale);
}

// Get from 4x64bit elements from table.
template <int tbl_scale>
	inline simde__m256i op_lookup_int64(const int64_t* tbl, const simde__m128i __index)
{
	return simde_mm256_i32gather_epi64(tbl, __index, tbl_scale);
}

// Get from 4x64bit elements from table with selectable mask.
template <int tbl_scale>
	inline simde__m256i op_lookup_selectable_int64(const simde__m256i src_vec, const int64_t* tbl, const simde__m128i __index, const simde__m256i mask_vec)
{
	return simde_mm256_mask_i32gather_epi64(src_vec, tbl, __index, mask_vec, tbl_scale);
}

template <int tbl_scale>
	inline simde__m256i op_lookup_selectable_int64(const simde__m256i src_vec, const int64_t* tbl, const simde__m128i __index, const bool masks[4])
{
	__DECL_ALIGNED(32) uint16_16_t mask_vec;
	SIMDE_VECTORIZE
	for(size_t i = 0; i < 4; i++) {
		mask_vec.u64[i] = (masks[i]) ? UINT64_MAX : 0;
	}
	return simde_mm256_mask_i32gather_epi64(src_vec, tbl, __index, mask_vec.v, tbl_scale);
}

} /* namespace simd_256bit */
