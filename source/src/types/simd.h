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

#define HAS_SIMDE
#if defined(HAS_SIMDE)
#include <simde/avx2.h>
#endif
#include "types/basic_types.h"
#include "types/system_endians.h"
#include "types/optimizer_utils.h"

#if !defined(__MINIMUM_ALIGN_LENGTH)
# if defined(SIMDE_ALIGN_PLATFORM_MAXIMUM) && (SIMDE_ALIGN_PLATFORM_MAXIMUM >= 16)
# define __M__MINIMUM_ALIGN_LENGTH 16 /* OK? */
# else
# define __M__MINIMUM_ALIGN_LENGTH 8 /* OK? */
#else
#define __M__MINIMUM_ALIGN_LENGTH __MINIMUM_ALIGN_LENGTH
#endif

#undef  __LOOP_LOAD8
#undef  __LOOP_LOAD8_UNALIGNED
#undef  __LOOP_FILL8
#undef  __LOOP_FILL8_UNALIGNED

#define __LOOP_LOAD8(foo, bar) {		\
		__DECL_VECTORIZED_LOOP			\
		for(size_t i = 0; i < 8; i++) { \
			foo[i] = bar[i];			\
		}								\
	}

#define __LOOP_LOAD8_UNALIGNED(foo, bar) {		\
		for(size_t i = 0; i < 8; i++) {			\
			foo[i] = bar[i];					\
		}										\
	}

#define __LOOP_FILL8(foo, bar) { \
		__DECL_VECTORIZED_LOOP			\
			for(size_t i = 0; i < 8; i++) { \
				foo[i] = bar;				\
			}								\
		}

#define __LOOP_FILL8_UNALIGNED(foo, bar) { \
		for(size_t i = 0; i < 8; i++) {			   \
			foo[i] = bar;						   \
		}										   \
	}											   

template<class T>
	class csp_vector8
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_data[8];
public:
	//csp_vector8(const csp_vector8<T>& __a)
	//{
	//	__LOOP_LOAD8(m_data, __a);
	//}
	csp_vector8(csp_vector8<T>& __a)
	{
		__LOOP_LOAD8(m_data, __a);
	}
	csp_vector8(const T* p)
	{
		__LIKELY_IF(p != nullptr) {
			load(p);
		} else {
			clear();
		}
	}
	csp_vector8(const T n)
	{
		__LOOP_FILL8(m_data, n);
	}
	csp_vector8()
	{
		__LOOP_FILL8(m_data, (T)0);
	}
	~csp_vector8() {}

	_CONSTEXPR_FUNC T at(size_t n)
	{
		return m_data[n];
	}
	// Pointer may be unaligned, or aligned.
	_CONSTEXPR_FUNC void load(T* p)
	{
		__LOOP_LOAD8_UNALIGNED(m_data, p);
	}

	
	_CONSTEXPR_FUNC csp_vector8<T>& exchange_endian()
	{
		_CONSTEXPR_IF(sizeof(T) <= 1) return *this;

		_CONSTEXPR_IF(sizeof(T) == 2) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				uint16_t n = (uint16_t)(m_data[i]);
				m_data[i] = (T)swapendian_16(n);
			}
			return *this;
		}
		_CONSTEXPR_IF(sizeof(T) == 4) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				uint32_t n = (uint32_t)(m_data[i]);
				m_data[i] = (T)swapendian_32(n);
			}
			return *this;
		}
		_CONSTEXPR_IF(sizeof(T) == 8) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				uint64_t n = (uint64_t)(m_data[i]);
				m_data[i] = (T)swapendian_64(n);
			}
			return *this;
		}
		#if defined(__HAS_BUILTIN_BSWAP128_X)
		_CONSTEXPR_IF(sizeof(T) == 16) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				__uint128_t n = (__uint128_t)(m_data[i]);
				m_data[i] = (T)swapendian_128(n);
			}
			return *this;
		}
		#endif
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T tmp;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			tmp = m_data[i];
			tmp = swapendian_T(tmp);
			m_data[i] = tmp;
		}
		return *this;
	}		
	_CONSTEXPR_FUNC void load_with_swapping_endian(uint8_t* p)
	{
		load((T*)p);
		exchange_endian();
	}
	_CONSTEXPR_FUNC void store_with_swapping_endian(uint8_t* p)
	{
		// Otherwise...
		csp_vector8<T> pp;
		pp.load_aligned(m_data);
		pp.exchange_endian();
		pp.store((T*)p);
	}
	_CONSTEXPR_FUNC void load_from_le(uint8_t* p)
	{
		#if defined(__LITTLE_ENDIAN__)
		load((T*)p);
		#else
		load_with_swapping_endian(p);
		#endif
	}
	_CONSTEXPR_FUNC void load_from_be(uint8_t* p)
	{
		#if defined(__LITTLE_ENDIAN__)
		load_with_swapping_endian(p);
		#else
		load((T*)p);
		#endif
	}
	_CONSTEXPR_FUNC void store_to_le(uint8_t* p)
	{
		#if defined(__LITTLE_ENDIAN__)
		store((T*)p);
		#else
		store_with_swapping_endian(p);
		#endif
	}
	_CONSTEXPR_FUNC void store_to_be(uint8_t* p)
	{
		#if defined(__LITTLE_ENDIAN__)
		store_with_swapping_endian(p);
		#else
		store((T*)p);
		#endif
	}
	_CONSTEXPR_FUNC  void load_limited(T* p, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = 0; i < _limit2; i++) {
			m_data[i] = p[i];
		}
	}
	_CONSTEXPR_FUNC  void load_offset(T* p, const size_t offset, const size_t _limit = 8)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j++) {
			m_data[i] = p[j];
		}
	}
	_CONSTEXPR_FUNC T *load2(T* p)
	{
		size_t j = 0;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i += 2, j++) {
			m_data[i]     = p[j];
			m_data[i + 1] = p[j];
		}
		return &(p[4]);
	}
	_CONSTEXPR_FUNC void load4(T* p)
	{
		size_t j = 0;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i += 4, j++) {
			m_data[i]     = p[j];
			m_data[i + 1] = p[j];
			m_data[i + 2] = p[j];
			m_data[i + 3] = p[j];
		}
		return &(p[2]);
	}
	template <class T2>
		_CONSTEXPR_FUNC void load(T2* p)
	{
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (T)(p[i]);
		}
	}
	template <typename T2>
		_CONSTEXPR_FUNC  void load_limited(T2* p, const size_t _limit)
	{

		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = 0; i < _limit2; i++) {
			m_data[i] = (T2)(p[i]);
		}
	}
	template <typename T2>
		_CONSTEXPR_FUNC  void load_offset(T2* p, const size_t offset, const size_t _limit = 8)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j++) {
			m_data[i] = (T2)(p[j]);
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void load2(T2* p)
	{
		for(size_t i0 = 0, i1 = 1, k = 0; k < 4; i0 += 2, i1 += 2, k++) {
			m_data[i0] = (T)(p[k]);
			m_data[i1] = (T)(p[k]);
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void load4(T2* p)
	{
		for(size_t i0 = 0, i1 = 1, i2 = 2, i3 = 3, k = 0; k < 2; i0 += 4, i1 += 4, i2 += 4, i3 += 4, k++) {
			m_data[i0] = (T)(p[k]);
			m_data[i1] = (T)(p[k]);
			m_data[i2] = (T)(p[k]);
			m_data[i3] = (T)(p[k]);
		}
	}
	// Pointer may be unaligned, or aligned.
	_CONSTEXPR_FUNC void store(T* p)
	{
		__LOOP_LOAD8(p, m_data);
	}

	_CONSTEXPR_FUNC void store_limited(T* p, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = 0; i < _limit2; i++) {
			p[i] = m_data[i];
		}
	}
	_CONSTEXPR_FUNC void store_offset(T* p, const size_t offset, const size_t _limit = 8)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j++) {
			p[i] = m_data[j];
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void store(T2* p)
	{
		for(size_t i = 0; i < 8; i++) {
			p[i] = (T2)(m_data[i]);
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void store_limited(T2* p, size_t _limit)
	{
		for(size_t i = 0; (i < 8) && (i < _limit); i++) {
			p[i] = (T2)(m_data[i]);
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void store_offset(T2* p, const size_t offset, const size_t _limit = 8)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j++) {
			p[i] = (T2)(m_data[j]);
		}
	}
	_CONSTEXPR_FUNC void store2(T* p)
	{
		csp_vector8<T> tmpval[2];
		for(size_t k = 0; k < 2; k++) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = (k * 4), j = 0; i < ((k * 4) + 4); i++, j += 2) {
				tmpval[k].set(j    , m_data[i]);
				tmpval[k].set(j + 1, m_data[i]);
			}
		}
		__DECL_VECTORIZED_LOOP
		for(size_t k = 0; k < 2; k++) {
			tmpval[k].store(&(p[k * 8]));
		}
	}
	_CONSTEXPR_FUNC void store2_limited(T* p, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = 0, j = 0; i < _limit2; i++, j += 2) {
			T tmpval = m_data[i];
			p[j] = tmpval;
			p[j + 1] = tmpval;
		}
	}
	_CONSTEXPR_FUNC void store2_offset(T* p, const size_t offset, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j += 2) {
			T tmpval = m_data[i];
			p[j] = tmpval;
			p[j + 1] = tmpval;
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void store2(T2* p)
	{
		for(size_t i = 0, j = 0; i < 8; i++, j += 2) {
			T2 tmpval = (T2)(m_data[i]);
			p[j] = tmpval;
			p[j + 1] = tmpval;
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void store2_limited(T2* p, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = 0, j = 0; i < _limit2; i++, j += 2) {
			T2 tmpval = (T2)(m_data[i]);
			p[j] = tmpval;
			p[j + 1] = tmpval;
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void store2_offset(T2* p, const size_t offset, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j += 2) {
			T2 tmpval = (T2)m_data[i];
			p[j] = tmpval;
			p[j + 1] = tmpval;
		}
	}
	_CONSTEXPR_FUNC void store4(T* p)
	{
		csp_vector8<T> tmpval[4];
		for(size_t k = 0; k < 4; k++) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = (k * 2), j = 0; i < ((k * 2) + 2); i++, j += 4) {
				tmpval[k].set(j    , m_data[i]);
				tmpval[k].set(j + 1, m_data[i]);
				tmpval[k].set(j + 2, m_data[i]);
				tmpval[k].set(j + 3, m_data[i]);
			}
		}
		__DECL_VECTORIZED_LOOP
		for(size_t k = 0; k < 4; k++) {
			tmpval[k].store(&(p[k * 8]));
		}
	}
	_CONSTEXPR_FUNC void store4_limited(T* p, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = 0, j = 0; i < _limit2; i++, j += 4) {
			T tmp = m_data[i];
			p[j] = tmp;
			p[j + 1] = tmp;
			p[j + 2] = tmp;
			p[j + 3] = tmp;
		}
	}
	_CONSTEXPR_FUNC void store4_offset(T* p, const size_t offset, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j += 4) {
			T tmpval = m_data[i];
			p[j] = tmpval;
			p[j + 1] = tmpval;
			p[j + 2] = tmpval;
			p[j + 3] = tmpval;
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void store4(T2* p)
	{
		for(size_t i = 0, j = 0; i < 8; i++, j += 4) {
			T2 tmpval = (T2)(m_data[i]);
			p[j] = tmpval;
			p[j + 1] = tmpval;
			p[j + 2] = tmpval;
			p[j + 3] = tmpval;
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void store4_limited(T2* p, size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = 0, j = 0; i < _limit2; i++, j += 4) {
			T2 tmpval = (T2)(m_data[i]);
			p[j] = tmpval;
			p[j + 1] = tmpval;
			p[j + 2] = tmpval;
			p[j + 3] = tmpval;
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void store4_offset(T2* p, const size_t offset, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j += 4) {
			T2 tmpval = (T2)(m_data[i]);
			p[j] = tmpval;
			p[j + 1] = tmpval;
			p[j + 2] = tmpval;
			p[j + 3] = tmpval;
		}
	}
	_CONSTEXPR_FUNC void store_n(const T* p, const size_t _mag)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			T tmp = m_data[i];
			for(size_t j = 0; j < _mag; j++) {
				*p++ = tmp;
			}
		}
	}
	_CONSTEXPR_FUNC void store_n_limited(const T* p, const size_t _mag, const size_t _limit = 8)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < _limit2; i++) {
			T tmpval = m_data[i];
			for(size_t j = 0; j < _mag; j++) {
				*p++ = tmpval;
			}
		}
	}
	_CONSTEXPR_FUNC void store_n_limited(const T* p, const size_t _mag, const size_t offset, const size_t _limit = 8)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < _limit2; i++) {
			T tmpval = m_data[i];
			for(size_t j = 0; j < _mag; j++) {
				*p++ = tmpval;
			}
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void store_n(T2* p, const size_t _mag)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T2 _tmp[8];
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			_tmp[i] = (T2)(m_data[i]);
		}
		for(size_t i = 0; i < 8; i++) {
			for(size_t j = 0; j < _mag; j++) {
				*p++ = _tmp[i];
			}
		}
	}
	template <class T2>
		_CONSTEXPR_FUNC void store_n_limited(T2* p, const size_t _mag, const size_t _limit)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T2 _tmp[8];
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			_tmp[i] = (T2)(m_data[i]);
		}
		for(size_t i = 0; (i < 8) && (i < _limit); i++) {
			for(size_t j = 0; j < _mag; j++) {
				*p++ = _tmp[i];
			}
		}
	}
	// Pointer must be aligned minimum of 16 bytes.
	void load_aligned(T* p)
	{
		T* q = (T*)(___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH));
		__LOOP_LOAD8(m_data, q);
	}
	template <class T2>
		void load_aligned(T2* p)
	{
		T2* q = (T2*)(___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH));
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (T)(q[i]);
		}
	}

	// Pointer must be aligned minimum of 16 bytes.
	inline void store_aligned(T* p) const
	{
		T* q = (T*)(___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH));
		__LOOP_LOAD8(q, m_data);
	}
	template <class T2>
		inline void store_aligned(T2* p) const
	{
		T2* q = (T2*)(___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH));
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			q[i] = (T2)(m_data[i]);
		}
	}
	_CONSTEXPR_FUNC void store2_aligned(T* p)
	{
		csp_vector8<T> tmpval[2];
		for(size_t k = 0; k < 2; k++) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = (k * 4), j = 0; i < ((k * 4) + 4); i++, j += 2) {
				tmpval[k].set(j    , m_data[i]);
				tmpval[k].set(j + 1, m_data[i]);
			}
		}
		T* q = (T*)(___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH));
		__DECL_VECTORIZED_LOOP
		for(size_t k = 0; k < 2; k++) {
			tmpval[k].store_aligned(&(q[k * 8]));
		}
	}
	_CONSTEXPR_FUNC void store4_aligned(T* p)
	{
		csp_vector8<T> tmpval[4];
		for(size_t k = 0; k < 4; k++) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = (k * 2), j = 0; i < ((k * 2) + 2); i++, j += 4) {
				tmpval[k].set(j    , m_data[i]);
				tmpval[k].set(j + 1, m_data[i]);
				tmpval[k].set(j + 2, m_data[i]);
				tmpval[k].set(j + 3, m_data[i]);
			}
		}
		T* q = (T*)(___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH));
		__DECL_VECTORIZED_LOOP
		for(size_t k = 0; k < 4; k++) {
			tmpval[k].store_aligned(&(q[k * 8]));
		}
	}
	
	_CONSTEXPR_FUNC void get(const csp_vector8<T>& __b)
	{
		__b.store_aligned(m_data);
	}

	template <class T2>
		_CONSTEXPR_FUNC void get(csp_vector8<T2>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (T)(__b[i]);
		}
	}

	_CONSTEXPR_FUNC void put(const csp_vector8<T>& __b)
	{
		__b.load_aligned(m_data);
	}

	template <class T2>
		_CONSTEXPR_FUNC void put(csp_vector8<T2>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__b.set(i, (T2)(m_data[i]));
		}
	}
	
	inline void clear()
	{
		__LOOP_FILL8(m_data, 0);
	}

	inline void fill(T __val)
	{
		__LOOP_FILL8(m_data, __val);
	}
	inline void set(size_t __n, T __val)
	{
		m_data[__n] = __val;
	}
	inline void reset(size_t __n)
	{
		m_data[__n] = (T)0;
	}
	
	
	_CONSTEXPR_FUNC void lshift(const size_t pos, const size_t val)
	{
		m_data[pos] <<= val;
	}
	_CONSTEXPR_FUNC void lshift(const size_t val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] <<= val;
		}
	}
	
	_CONSTEXPR_FUNC void rshift(const size_t pos, const size_t val)
	{
		m_data[pos] >>= val;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& rshift(const size_t val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] >>= val;
		}
		return *this;
	}

	template <class T2>
		_CONSTEXPR_FUNC csp_vector8<T>& shift(const T2 val)
	{
		constexpr bool __is_signed = std::is_signed<T2>().value;
		if(__is_signed) {
			const ssize_t __shift = (ssize_t)val;
			if(__shift < 0) {
				const size_t __shift2 = ((const size_t)(-__shift)) % ((sizeof(T) * 8) - 1);
				rshift(__shift2);
			} else if(__shift > 0) {
				const size_t __shift2 = ((const size_t)__shift) % ((sizeof(T) * 8) - 1);
				lshift(__shift2);
			}
		} else {
			const size_t __shift = (const size_t)val % (sizeof(T) * 8);
			lshift(__shift);
		}
		return *this;
	}

	template <class T2>
		_CONSTEXPR_FUNC void shift(size_t pos, const T2 val)
	{
		constexpr bool _signed = std::is_signed<T2>().value;
	    if(_signed) {
			const ssize_t __shift = (ssize_t)val;
			if(__shift < 0) {
				const size_t __shift2 = ((const size_t)(-__shift)) % ((sizeof(T) * 8) - 1);
				rshift(pos, __shift2);
			} else if(__shift > 0) {
				const size_t __shift2 = ((const size_t)__shift) % ((sizeof(T) * 8) - 1);
				lshift(pos, __shift2);
			}
		} else {
			const size_t __shift = (const size_t)val % (sizeof(T) * 8);
			lshift(pos, __shift);
		}
	}
	
	_CONSTEXPR_FUNC void shift(size_t pos, const ssize_t val)
	{
		if(val < 0) {
			rshift(pos, (-val) % ((sizeof(T) * 8) - 1));
		} else if(val > 0) {
			lshift(pos, val % ((sizeof(T) * 8) - 1));
		}
	}
	_CONSTEXPR_FUNC void shift(size_t pos, const size_t val)
	{
		lshift(pos, val % (sizeof(T) * 8));
	}
	_CONSTEXPR_FUNC csp_vector8<T>&  shift(csp_vector8<ssize_t>& val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			shift(i, val.at(i));
		}
		return *this;
	}
	
	_CONSTEXPR_FUNC csp_vector8<T>& clamp_upper(const T upper_val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (m_data[i] > upper_val) ? upper_val : m_data[i];
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& clamp_lower(const T lower_val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (m_data[i] < lower_val) ? lower_val : m_data[i];
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& clamp(const T upper_val, const T lower_val)
	{
		T upper = upper_val;
		T lower = lower_val;
		if(lower > upper) std::swap(upper, lower);

		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (m_data[i] < lower) ? lower : m_data[i];
		}
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (m_data[i] < upper) ? m_data[i] : upper;
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& bitwise_not()
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = ~(m_data[i]);
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& bitwise_not(const csp_vector8<T> __b)
	{
		__b.store_aligned(m_data);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = ~(m_data[i]);
		}
		return *this;
	}
	template <typename T2>
		_CONSTEXPR_FUNC csp_vector8<T>& lookup(csp_vector8<T2>& __list, T* __table, const size_t count = 8)
	{
		csp_vector8<T2> rlist(__list);
		size_t _count = ((count > 8) || (count == 0)) ? 8 : count;
		constexpr bool _is_signed = std::is_signed<T2>().value;

		if(_is_signed) {
			rlist.clamp_lower(0);
		}
		for(size_t i = 0; i < _count; i++) {
			m_data[i] = __table[rlist[i]];
		}
		return *this;
	}
	template <typename T2>
		_CONSTEXPR_FUNC csp_vector8<T>& lookup(csp_vector8<T2>& __list, T2 _limit, T* __table, const size_t count = 8)
	{
		csp_vector8<T2> rlist(__list);
		constexpr bool _is_signed = std::is_signed<T2>().value;
		size_t _count = ((count > 8) || (count == 0)) ? 8 : count;

		if(_is_signed) {
			rlist.clamp_lower((T2)0);
		}
		rlist.clamp_upper(_limit);
		for(size_t i = 0; i < _count; i++) {
			m_data[i] = __table[rlist[i]];
		}
		return *this;
	}
	template <typename T2>
		_CONSTEXPR_FUNC csp_vector8<T>& lookup(csp_vector8<T2>& __list, const T2 _min, const T2 _max, T* __table, const size_t count = 8)
	{
		csp_vector8<T2> rlist(__list);
		const size_t _count = ((count > 8) || (count == 0)) ? 8 : count;
		T2 _min2 = _min;
		T2 _max2 = _max;
		if(_min > _max) std::swap(_min2, _max2);

		rlist.clamp(_max2, _min2);

		for(size_t i = 0; i < _count; i++) {
			m_data[i] = __table[rlist[i]];
		}
		return *this;
	}

	_CONSTEXPR_FUNC csp_vector8<T>& set_cond(csp_vector8<bool>& __flags, const T __true_val, const T __false_val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (__flags.at(i)) ? __true_val : __false_val;
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& set_if_true(csp_vector8<bool>& __flags, const T __val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (__flags.at(i)) ?  m_data[i] : __val;
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& set_if_false(csp_vector8<bool>& __flags, const T __val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (__flags.at(i)) ? __val : m_data[i];
		}
		return *this;
	}
	template <typename T2>
		inline void shuffle(csp_vector8<T2>& __positions)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T __d[8];
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) uint8_t __p[8];
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) const uint8_t __m[8] =
			{7, 7, 7, 7,
			 7, 7, 7, 7};
		constexpr bool __is_signed = std::is_signed<T2>().value;
		if(__is_signed) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				__p[i] = (__positions.at(i) < 0) ? (uint8_t)(-(__positions.at(i))) : (uint8_t)(__positions.at(i));
			}
		} else {
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				__p[i] = (uint8_t)(__positions.at(i));
			}
		}
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__p[i] &= __m[i];
		}

		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__d[i] = m_data[__p[i]];
		}
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = __d[i];
		}
	}
	template <class T3>
		inline void shuffle_force_unsigned(csp_vector8<T3>& __positions)
	{
		typedef typename std::make_unsigned<T3>::type T3U;
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T3U> _p;
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T3U _pos[8];
		__positions.store_aligned(_pos);

		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			_p[i] = (T3U)(_pos[i]);
		}
		shuffle(_p);
	}

	inline T& operator[](size_t __n)
	{
		return m_data[__n];
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator=(const csp_vector8<T>& __b)
	{
		__b.store_aligned(m_data);
		return *this;
	}
	template <typename T2>
		_CONSTEXPR_FUNC csp_vector8<T>& operator=(const csp_vector8<T2>& __b)
	{
		__b.store_aligned(m_data);
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator+=(const T __n)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = m_data[i] + __n;
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator+=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] += __b.at(i);
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator-=(const T __n)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = m_data[i] - __n;
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator-=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] -= __b.at(i);
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator/=(const T __n)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = m_data[i] / __n;
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator/=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] /= __b.at(i);
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator*=(const T __n)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = m_data[i] * __n;
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator*=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] *= __b.at(i);
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator&=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] &= __b.at(i);
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator&=(const T __n)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = m_data[i] & __n;
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator|=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] |= __b.at(i);
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator|=(const T __n)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = m_data[i] | __n;
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator^=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] ^= __b.at(i);
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator^=(const T __n)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = m_data[i] ^ __n;
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator<<=(const size_t __n)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] <<= __n;
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator>>=(const size_t __n)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] >>= __n;
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator>>=(csp_vector8<size_t>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] >>= (size_t)(__b.at(i));
		}
		return *this;
	}
	_CONSTEXPR_FUNC csp_vector8<T>& operator<<=(csp_vector8<size_t>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] <<= (size_t)(__b.at(i));
		}
		return *this;
	}

	_CONSTEXPR_FUNC bool operator==(csp_vector8<T>& __a)
	{
		bool __f = true;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (__a.at(i) == m_data[i]);
		}
		return __f;
	}
	_CONSTEXPR_FUNC bool operator==(const T __a)
	{
		bool __f = true;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (m_data[i] == __a);
		}
		return __f;
	}
	_CONSTEXPR_FUNC bool operator!=(csp_vector8<T>& __a)
	{
		bool __f = true;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (__a.at(i) != m_data[i]);
		}
		return __f;
	}
	_CONSTEXPR_FUNC bool operator!=(const T __a)
	{
		bool __f = true;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (__a != m_data[i]);
		}
		return __f;
	}

	_CONSTEXPR_FUNC void equals(csp_vector8<bool>& __ret, csp_vector8<T>& __a)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (__a.at(i) == m_data[i]));
		}
	}
	_CONSTEXPR_FUNC void equals(csp_vector8<bool>& __ret, T __a)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (m_data[i] == __a));
		}
	}

	
	_CONSTEXPR_FUNC void not_equals(csp_vector8<bool>& __ret, csp_vector8<T>& __a)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (__a.at(i) != m_data[i]));
		}
	}
	_CONSTEXPR_FUNC void not_equals(csp_vector8<bool>& __ret, T __a)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (m_data[i] != __a));
		}
	}
	_CONSTEXPR_FUNC void check_bits(csp_vector8<bool>& __ret, const T& _bitmask)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, ((m_data[i] & _bitmask) == _bitmask));
		}
	}
	// Maybe faster than check_bits().
	_CONSTEXPR_FUNC void check_any_bits(csp_vector8<bool>& __ret, const T& _bitmask)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, ((m_data[i] & _bitmask) != 0));
		}
	}

};


template <class T>
	 inline csp_vector8<T>& operator+(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret += __b;
	return __ret;
}

// Primitive operators must define outside of class :-(


template <class T>
	inline csp_vector8<T> operator~(const csp_vector8<T>& __a)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret;
	__ret = __a;
	__ret = __ret.bitwise_not();
	return __ret;
}

template <class T>
	 inline csp_vector8<T>& operator-(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret -= __b;
	return __ret;
}

template <class T>
	 inline csp_vector8<T>& operator*(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret *= __b;
	return __ret;
}

template <class T>
	 inline csp_vector8<T>& operator/(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret /= __b;
	return __ret;
}

template <class T>
	 inline csp_vector8<T>& operator&(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret &= __b;
	return __ret;
}


template <class T>
	 inline csp_vector8<T>& operator|(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret |= __b;
	return __ret;
}
template <class T>
	 inline csp_vector8<T>& operator^(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret ^= __b;
	return __ret;
}

template <class T>
	 inline csp_vector8<T>& operator<<(const csp_vector8<T>& __a, const size_t& __shift)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret <<= __shift;
	return __ret;
}

template <class T>
	 inline csp_vector8<T>& operator>>(const csp_vector8<T>& __a, const size_t& __shift)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret >>= __shift;
	return __ret;
}


template <class T>
	 inline csp_vector8<T>& operator<<(const csp_vector8<T>& __a, csp_vector8<size_t>& __shift)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret <<= __shift;
	return __ret;
}

template <class T>
	 inline csp_vector8<T>& operator>>(const csp_vector8<T>& __a, csp_vector8<size_t>& __shift)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret >>= __shift;
	return __ret;
}

template <class T>
	 inline csp_vector8<bool> operator==(csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<bool> __ret;
	__a.equals(__ret, __b);
	return __ret;
}

template <class T>
	 inline csp_vector8<bool> operator==(csp_vector8<T>& __a, const T __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<bool> __ret;
	__a.equals(__ret, __b);
	return __ret;
}

template <class T>
	 inline csp_vector8<bool> operator!=(csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<bool> __ret;
	__a.not_equals(__ret, __b);
	return __ret;
}

template <class T>
	 inline csp_vector8<bool> operator!=(csp_vector8<bool>& __a, const T __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<bool> __ret;
	__a.not_equals(__ret, __b);
	return __ret;
}


template <class T>
	inline void make_rgba_vec8(csp_vector8<scrntype_t>& dst, csp_vector8<T> r ,csp_vector8<T> g , csp_vector8<T> b, csp_vector8<T> a)
{
	// Caution: r, g, b, a assumes 8bit value.
	csp_vector8<scrntype_t> r_data;
	csp_vector8<scrntype_t> g_data;
	csp_vector8<scrntype_t> b_data;
	csp_vector8<scrntype_t> a_data;
	r_data = r;
	g_data = g;
	b_data = b;
	a_data = a;
	
	#if defined(_RGB555) || defined(_RGB565)
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		a_data[i] = (a_data[i] == (T)0) ? 0 : (a_data[i] + (T)1);
	}
	// Apply alpha value.
	const size_t __shiftval = 8;
	r_data *= a_data;
	g_data *= a_data;
	b_data *= a_data;
	r_data >>= __shiftval;
	g_data >>= __shiftval;
	b_data >>= __shiftval;
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		dst.set(i, RGB_COLOR(r_data[i], g_data[i], b_data[i]));
	}
	#else /* RGBA32 */
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		dst.set(i, RGBA_COLOR(r_data[i], g_data[i], b_data[i], a_data[i]));
	}
	#endif

}

template <class T>
	inline void make_rgb_vec8(csp_vector8<scrntype_t>& dst, csp_vector8<T> r ,csp_vector8<T> g , csp_vector8<T> b)
{
	// Caution: r, g, b, a assumes 8bit value.
	csp_vector8<scrntype_t> r_data;
	csp_vector8<scrntype_t> g_data;
	csp_vector8<scrntype_t> b_data;
	r_data = r;
	g_data = g;
	b_data = b;
	#if defined(_RGB555) || defined(_RGB565)
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		dst.set(i, RGB_COLOR(r_data[i], g_data[i], b_data[i]));
	}
	#else /* _RGBA8888 */
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		dst.set(i, RGBA_COLOR(r_data[i], g_data[i], b_data[i], 255));
	}
	#endif

}

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
} uint8_8t;

typedef union {
	uint8_t u8[16];
	int8_t  s8[16];
	uint16_t u16[8];
	int16_t  s16[8];
	uint32_t u32[4];
	int32_t  s32[4];
	uint64_t d[2];
	int64_t  sd[2];
	union {
		#ifdef __BIG_ENDIAN__
		uint8_8_t h, l;
		#else
		uint8_8_t l, h;
		#endif
		uint8_8_t pairs[2];
	} pair;
	simde__m128 v;
} uint16_8t;

typedef union {
	uint8_t u8[32];
	int8_t  s8[32];
	uint16_t u16[16];
	int16_t  s16[16];
	uint32_t u32[8];
	int32_t  s32[8];
	uint64_t d[4];
	int64_t  sd[4];
	union {
		#ifdef __BIG_ENDIAN__
		uint8_8_t h3, h2, h, l;
		#else
		uint8_8_t l, h, h2, h3;
		#endif
		uint8_8_t pairs[4];
	} pairs;
	union {
		#ifdef __BIG_ENDIAN__
		simde__m128 h, l;
		#else
		simde__m128 l, h;
		#endif
		simde__m128 v[2];
	} v128;
	simde__m256 v256;
	simde__m256 v;
} uint16_16t;

typedef simd64_t uint8_8_t;
typedef simd128_t uint16_8_t;
typedef simd256_t uint16_16_t;

#undef SCRNTYPE8_T_WIDTH
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

template <typename T, typename Y>
	class csp_simd_pri
{
//protected:
//	size_t memb;
//	T* m_unaligned_ptr;
//	T* aligned_ptr;
public:
	T _d;
	virtual ~csp_simd_pri() {	}

	virtual inline const bool is_aligned(void *p)
	{
		const uintptr_t pd = (uintptr_t)p;
		const uintptr_t mask = sizeof(T) - 1;  // ToDo: for not 2^n . 20260218 K.O
		return ((pd & mask) == 0) ? true : false;
	}
	// Read
	virtual inline void align_load(void* p) = 0;
	virtual inline void unalign_load(void* p) = 0;
	inline void load(void *p)
	{
		if(is_aligned(p)) {
			align_load(p);
		} else {
			unalign_load(p);
		}
	}

	inline size_t load_left(void *p, const size_t num)
	{
		__UNLIKELY_IF((sizeof(Y) > sizeof(T)) || (p == nullptr)) {
			return 0;
		}
		size_t off = 0;
		size_t i = 0;
		Y* pp = (Y*)p;
		uint8_t* q = (uint8_t*)(&_d);
		Y* qq = (Y*)(&(q[off]));
		for(; (off < sizeof(T)) && (i < num); off += sizeof(Y), i++) {
			Y _t = pp[i];
			qq[i] = _t;
		}
		return _i;
	}
	virtual inline size_t load_right(void *p, const size_t num)
	{
		__UNLIKELY_IF((sizeof(Y) > sizeof(T)) || (p == nullptr)) {
			return 0;
		}
		size_t __num = num;
		__UNLIKELY_IF((sizeof(T) / sizeof(Y)) > __num) {
			__num = sizeof(T) / sizeof(Y);
		}
		size_t off = (sizeof(T) / sizeof(Y) - __num) * sizeof(Y);
		size_t i = 0;
		Y* pp = (Y*)p;
		uint8_t* q = (uint8_t*)(&_d);
		Y* qq = (Y*)(&(q[off]));
		for(; ((off + sizeof(Y)) <= sizeof(T)) && (i < __num); off += sizeof(Y), i++) {
			Y _t = pp[i];
			qq[i] = _t;
		}
		return _i;
	}
	
	virtual inline void align_store(void* p) = 0;
	virtual inline void unalign_store(void* p) = 0;
	inline void store(void *p)
	{
		if(is_aligned(p)) {
			align_store(p);
		} else {
			unalign_store(p);
		}
	}	
	inline size_t store_left(void *p, const size_t num)
	{
		__UNLIKELY_IF((sizeof(Y) > sizeof(T)) || (p == nullptr)) {
			return 0;
		}
		size_t off = 0;
		size_t i = 0;
		Y* pp = (Y*)p;
		uint8_t* q = (uint8_t*)(&_d);
		Y* qq = (Y*)(&(q[off]));
		for(; (off < sizeof(T)) && (i < num); off += sizeof(Y), i++) {
			Y _t = qq[i];
			pp[i] = _t;
		}
		return _i;
	}
	virtual inline size_t store_right(void *p, const size_t num)
	{
		__UNLIKELY_IF((sizeof(Y) > sizeof(T)) || (p == nullptr)) {
			return 0;
		}
		size_t __num = num;
		__UNLIKELY_IF((sizeof(T) / sizeof(Y)) > __num) {
			__num = sizeof(T) / sizeof(Y);
		}
		size_t off = (sizeof(T) / sizeof(Y) - __num) * sizeof(Y);
		size_t i = 0;
		Y* pp = (Y*)p;
		uint8_t* q = (uint8_t*)(&_d);
		Y* qq = (Y*)(&(q[off]));
		for(; ((off + sizeof(Y)) <= sizeof(T)) && (i < __num); off += sizeof(Y), i++) {
			Y _t = qq[i];
			pp[i] = _t;
		}
		return _i;
	}
	
	virtual inline void clear() = 0;
	virtual inline void fill(Y val)
	{
		size_t off = 0;
		uint8_t* p = (uint8_t*)(&_d);
		Y* pp = (Y*)p;
		for(size_t i = 0; off < sizeof(T); off += sizeof(Y), i++) {
			pp[i] = val;
		}
	}
	virtual inline void set(size_t pos, Y val)
	{
		__UNLIKELY_IF((sizeof(T) / sizeof(Y)) <= pos) {
			return;
		}
		uint8_t* p = (uint8_t*)(&_d);
		Y* pp = (Y*)p;
		pp[pos] = val; // OK?
	}
	virtual inline Y at(size_t pos)
	{
		__UNLIKELY_IF((sizeof(T) / sizeof(Y)) <= pos) {
			return (Y)0;
		}
		uint8_t* p = (uint8_t*)(&_d);
		Y* pp = (Y*)p;
		return pp[pos]; // OK?
	}
	virtual inline csp_simd_prim<T> me()
	{
		retutn *this;
	}
	virtual inline csp_simd_prim<T>* ptr()
	{
		retutn this;
	}
	virtual inline T data()
	{
		retutn _d;
	}
	virtual inline T* dptr()
	{
		retutn &(_d);
	}
	
	virtual inline Y lookup(Y* tbl, uint16_t pos, constexpr size_t elements = 256) = 0;
	virtual inline Y lookup(const csp_simd_pri<T>* tbl, uint16_t pos, constexpr size_t elements = 256) = 0;
	virtual inline T lookup(const csp_simd_pri<T>* tbl, const uint16_t[] pos_tbl, const uint16_t tbl_length, constexpr size_t elements = 256) = 0;

	virtual inline T bswap(constexpr size_t size = 4) = 0;
	virtual inline void bswap_self(constexpr size_t size = 4) = 0;
	
	virtual inline csp_simd_pri<T>& operator=(const csp_simd_pri<T>& __b) = 0;
	
	virtual inline csp_simd_pri<T>& operator+=(const csp_simd_pri<T>& __b) const = 0;
	virtual inline csp_simd_pri<T>& operator-=(const csp_simd_pri<T>& __b) const = 0;

	
	virtual inline csp_simd_pri<T>& operator|=(const csp_simd_pri<T>& __b) = 0;
	virtual inline csp_simd_pri<T>& operator&=(const T& __b) = 0;
	virtual inline csp_simd_pri<T>& operator^=(const T& __b) = 0;

	virtual inline csp_simd_pri<T> operator~() const = 0;
	virtual inline csp_simd_pri<T> operator|(const csp_simd_pri<T>& __b) const = 0;
	virtual inline csp_simd_pri<T> operator&(const csp_simd_pri<T>& __b) const = 0;
	virtual inline csp_simd_pri<T> operator^(const csp_simd_pri<T>& __b) const = 0;

	virtual inline csp_simd_pri<T>& operator<<=(const size_t __b) const = 0;
	virtual inline csp_simd_pri<T>& operator>>=(const size_t __b) const = 0;
	virtual inline csp_simd_pri<T> operator<<(const ssize_t& __b) const = 0;
	virtual inline csp_simd_pri<T> operator>>(const ssize_t& __b) const = 0;
	
	virtual inline bool operator==(csp_simd_pri<T>& __b) = 0;
	virtual inline bool operator!=(csp_simd_pri<T>& __b) = 0;
	virtual inline bool operator>=(csp_simd_pri<T>& __b) = 0;
	virtual inline bool operator<=(csp_simd_pri<T>& __b) = 0;
	virtual inline bool operator>(csp_simd_pri<T>& __b) = 0;
	virtual inline bool operator<(csp_simd_pri<T>& __b) = 0;
	
	virtual inline csp_simd_pri<T> op_add(csp_simd_pri<T> __a, csp_simd_pri<T> __b) = 0;
	virtual inline csp_simd_pri<T> op_sub(csp_simd_pri<T> __a, csp_simd_pri<T> __b) = 0;
	virtual inline csp_simd_pri<T> op_mul(csp_simd_pri<T> __a, csp_simd_pri<T> __b) = 0;
	virtual inline csp_simd_pri<T> op_div(csp_simd_pri<T> __a, csp_simd_pri<T> __b) = 0;
	virtual inline csp_simd_pri<T> op_mod(csp_simd_pri<T> __a, csp_simd_pri<T> __b) = 0;

	virtual inline csp_simd_pri<T> op_and(csp_simd_pri<T> __a, csp_simd_pri<T> __b) = 0;
	virtual inline csp_simd_pri<T> op_or(csp_simd_pri<T> __a, csp_simd_pri<T> __b) = 0;
	virtual inline csp_simd_pri<T> op_xor(csp_simd_pri<T> __a, csp_simd_pri<T> __b) = 0;
	virtual inline csp_simd_pri<T> op_not(csp_simd_pri<T> __a) = 0;
	// Return = (not __a) and __b
	virtual inline csp_simd_pri<T> op_notand(csp_simd_pri<T> __a, csp_simd_pri<T> __b) = 0;

	virtual inline csp_simd_pri<T>& lshift(const ssize_t __b) = 0;
	virtual inline csp_simd_pri<T>& rshift(const ssize_t __b) = 0;

};

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





template <uint16_8_t, typename Y>
	class csp_simd_pri {
	csp_simd_pri(uint16_t n = 0)
	{
		fill(n);
	}
	csp_simd_pri(uint16_8_t n)
	{
		data.v = n.v;
	}

	
	inline void align_load(void *p)
	{
		_d.v = simde_mm_load_ps((const float*)p);
		return; 
	}
	inline void unalign_load(void *p)
	{
		_d.v = simde_mm_loadu_ps((const float*)p);
		return; 
	}
	inline void align_store(void *p)
	{
		simde_mm_store_ps((const float*)p, _d.v);
		return; 
	}
	inline void unalign_load(void *p)
	{
		simde_mm_storeu_ps((const float*)src, _d.v);
		return; 
	}
	inline void clear()
	{
		_d.v = simde_mm_zero_ps();
	}
	inline void fill(uint8_t val) override
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 16; i++) {
			_d.u8[i] = val;
		}
	}
	inline void fill(int8_t val) override
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 16; i++) {
			_d.s8[i] = val;
		}
	}
	inline void fill(uint16_t val) override
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			_d.u16[i] = val;
		}
	}
	inline void fill(int16_t val) override
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			_d.s16[i] = val;
		}
	}
	inline void fill(uint32_t val) override
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 4; i++) {
			_d.u32[i] = val;
		}
	}
	inline void fill(int32_t val) override
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 4; i++) {
			_d.s32[i] = val;
		}
	}
	inline void fill(uint64_t val) override
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 2; i++) {
			_d.u32[i] = val;
		}
	}
	inline void fill(int64_t val) override
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 2; i++) {
			_d.s32[i] = val;
		}
	}
	inline csp_simd_pri<uint16_8_t>& operator-() const
	{
		__DECL_ALIGNED(16) uint16_8_t _r;
		_r.v = simde_mm_setzero_ps();
		_d.v = op_sib_s16(_r.v, _d.v);
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator=(const csp_simd_pri<uint16_8_t>& __b) const
	{
		//load(&(__b._d.v));
		_d.v = __b._d.v;
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator=(const uint16_8_t& __b) const
	{
		_d.v = __b.v;
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator=(const simde__m128& __b) const
	{
		_d.v = __b;
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator&=(const csp_simd_pri<uint16_8_t>& __b) const
	{
		_d.v = op_and(_d.v, __b._d.v)
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator&=(const uint16_8_t& __b) const
	{
		_d.v = op_and(_d.v, __b.v);
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator&=(const simde__m128& __b) const
	{
		_d.v = op_and(_d.v, __b);
		return *this;
	}
	
	inline csp_simd_pri<uint16_8_t>& operator|=(const csp_simd_pri<uint16_8_t>& __b) const
	{
		_d.v = op_or(_d.v, __b._d.v)
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator|=(const uint16_8_t __b) const
	{
		_d.v = op_or(_d.v, __b.v);
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator|=(const simde__m128 __b) const
	{
		_d.v = op_or(_d.v, __b);
		return *this;
	}
	
	inline csp_simd_pri<uint16_8_t>& operator^=(const csp_simd_pri<uint16_8_t>& __b) const
	{
		_d.v = op_xor(_d.v, __b._d.v)
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator^=(const uint16_8_t __b) const
	{
		_d.v = op_xor(_d.v, __b.v);
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator^=(const simde__m128 __b) const
	{
		_d.v = op_xor(_d.v, __b);
		return *this;
	}
	
	inline csp_simd_pri<uint16_8_t>& operator~() const
	{
		_d.v = op_not(_d.v);
		return *this;
	}
	inline csp_simd_prim<uint16_8_t>& operator+=(csp_simd_prim<uint16_8_t>& __b) const
	{
		_d.v = op_add_s16_sat(_d.v, __b._d.v);
		return *this;
	}
	inline csp_simd_prim<uint16_8_t>& operator+=(uint16_8_t& __b) const
	{
		_d.v = op_add_s16_sat(_d.v, __b.v);
		return *this;
	}
	inline csp_simd_prim<uint16_8_t>& operator+=(simde__m128& __b) const
	{
		_d.v = op_add_s16_sat(_d.v, __b);
		return *this;
	}
	
	inline csp_simd_prim<uint16_8_t>& operator-=(csp_simd_prim<uint16_8_t>& __b) const
	{
		_d.v = op_sub_s16_sat(_d.v, __b._d.v);
		return *this;
	}
	inline csp_simd_prim<uint16_8_t>& operator-=(uint16_8_t& __b) const
	{
		_d.v = op_sub_s16_sat(_d.v, __b.v);
		return *this;
	}
	inline csp_simd_prim<uint16_8_t>& operator-=(simde__m128& __b) const
	{
		_d.v = op_sub_s16_sat(_d.v, __b);
		return *this;
	}
    inline csp_simd_prim<uint16_8_t>& op_andnot(const csp_simd_prim<uint16_8_t>& __a) const
	{
		__DECL_ALIGNED(16) simde__m128 _r(__a);
		_d.v = simde_mm_andnot_si128(_r._d.v, _d.v);
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator<<=(const size_t& __shift) const
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = op_lshift16(_d.v, __shift);
		_d.v = _r;
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator>>=(const size_t& __shift) const
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = op_rshift16(_d.v, __shift);
		_d.v = _r;
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& op_lshift32(const size_t __shift) const
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = op_lshift32(_d.v, __shift);
		_d.v = _r;
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& op_rshift32(const size_t __shift) const
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = op_rshift32(_d.v, __shift);
		_d.v = _r;
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& op_lshift16(const size_t __shift) const
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = op_lshift16(_d.v, __shift);
		_d.v = _r;
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& op_rshift16(const size_t __shift) const
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = op_rshift16(_d.v, __shift);
		_d.v = _r;
		return *this;
	}
	
};

inline csp_simd_pri<uint16_8_t>& operator+(const csp_simd_pri<uint16_8_t>& __a, const csp_simd_pri<uint16_8_t>& __b)
{
	__DECL_ALIGNED(16) csp_simd_pri<uint16_8_t> __d(__a);
	__d += __b;
	return __d;
}
inline csp_simd_pri<uint16_8_t>& operator-(const csp_simd_pri<uint16_8_t>& __a, const csp_simd_pri<uint16_8_t>& __b)
{
	__DECL_ALIGNED(16) csp_simd_pri<uint16_8_t> __d(__a);
	__d -= __b;
	return __d;
}

inline csp_simd_pri<uint16_8_t>& operator&(const csp_simd_pri<uint16_8_t>& __a, const csp_simd_pri<uint16_8_t>& __b)
{
	__DECL_ALIGNED(16) csp_simd_pri<uint16_8_t> __d(__a);
	__d &= __b;
	return __d;
}

inline csp_simd_pri<uint16_8_t>& operator|(const csp_simd_pri<uint16_8_t>& __a, const csp_simd_pri<uint16_8_t>& __b)
{
	__DECL_ALIGNED(16) csp_simd_pri<uint16_8_t> __d(__a);
	__d |= __b;
	return __d;
}

inline csp_simd_pri<uint16_8_t>& operator^(const csp_simd_pri<uint16_8_t>& __a, const csp_simd_pri<uint16_8_t>& __b)
{
	__DECL_ALIGNED(16) csp_simd_pri<uint16_8_t> __d(__a);
	__d ^= __b;
	return __d;
}

inline csp_simd_pri<uint16_8_t>& op_andnot(const csp_simd_pri<uint16_8_t>& __a, const csp_simd_pri<uint16_8_t>& __b)
{
	__DECL_ALIGNED(16) csp_simd_pri<uint16_8_t> __d(__b);
	__d.op_andnot(__a);
	return __d;
}

inline csp_simd_pri<uint16_8_t>& operator<<(const csp_simd_pri<uint16_8_t>& __a, const size_t& __shift)
{
	__DECL_ALIGNED(16) csp_simd_pri<uint16_8_t> __d(__a);
	__d <<= __shift;
	return __d;
}

inline csp_simd_pri<uint16_8_t>& operator>>(const csp_simd_pri<uint16_8_t>& __a, const size_t& __shift)
{
	__DECL_ALIGNED(16) csp_simd_pri<uint16_8_t> __d(__a);
	__d >>= __shift;
	return __d;
}

// Please include type specified (and MPU specified) templates.


#undef  __LOOP_LOAD8
#undef  __LOOP_LOAD8_UNALIGNED
#undef  __LOOP_FILL8
#undef  __LOOP_FILL8_UNALIGNED
#undef __M__MINIMUM_ALIGN_LENGTH
