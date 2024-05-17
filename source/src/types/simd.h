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

#include "types/basic_types.h"
#include "types/system_endians.h"
#include "types/optimizer_utils.h"

#if !defined(__MINIMUM_ALIGN_LENGTH)
#define __M__MINIMUM_ALIGN_LENGTH 16 /* OK? */
#else
#define __M__MINIMUM_ALIGN_LENGTH __MINIMUM_ALIGN_LENGTH
#endif

template<class T>
	class csp_vector8
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_data[8];
public:
	constexpr csp_vector8(const csp_vector8<T>& __a)
	{
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = __a.at(i);
		}
	}
	constexpr csp_vector8(csp_vector8<T>& __a)
	{
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = __a.at(i);
		}
	}
	constexpr csp_vector8(const T* p)
	{
		__LIKELY_IF(p != nullptr) {
			load(p);
		} else {
			clear();
		}
	}
	constexpr csp_vector8(const T n)
	{
		fill(n);
	}
	constexpr csp_vector8()
	{
		//clear();
	}
	constexpr ~csp_vector8() {}

	constexpr T at(const size_t& n)
	{
		return m_data[n];
	}
	// Pointer may be unaligned, or aligned.
	constexpr void load(T* p)
	{
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = p[i];
		}
	}

	
	constexpr csp_vector8<T>& exchange_endian()
	{
		constexpr size_t __size = sizeof(T);
		if(__size <= 1) return *this;

		if(__size == 2) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				uint16_t n = (uint16_t)(m_data[i]);
				m_data[i] = (T)swapendian_16(n);
			}
			return *this;
		}
		if(__size == 4) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				uint32_t n = (uint32_t)(m_data[i]);
				m_data[i] = (T)swapendian_32(n);
			}
			return *this;
		}
		if(__size == 8) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				uint64_t n = (uint64_t)(m_data[i]);
				m_data[i] = (T)swapendian_64(n);
			}
			return *this;
		}
		#if defined(__HAS_BUILTIN_BSWAP128_X)
		if(__size == 16) {
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
	constexpr void load_with_swapping_endian(uint8_t* p)
	{
		load((T*)p);
		exchange_endian();
	}
	constexpr void store_with_swapping_endian(uint8_t* p)
	{
		// Otherwise...
		csp_vector8<T> pp;
		pp.load_aligned(m_data);
		pp.exchange_endian();
		pp.store((T*)p);
	}
	constexpr void load_from_le(uint8_t* p)
	{
		#if defined(__LITTLE_ENDIAN__)
		load((T*)p);
		#else
		load_with_swapping_endian(p);
		#endif
	}
	constexpr void load_from_be(uint8_t* p)
	{
		#if defined(__LITTLE_ENDIAN__)
		load_with_swapping_endian(p);
		#else
		load((T*)p);
		#endif
	}
	constexpr void store_to_le(uint8_t* p)
	{
		#if defined(__LITTLE_ENDIAN__)
		store((T*)p);
		#else
		store_with_swapping_endian(p);
		#endif
	}
	constexpr void store_to_be(uint8_t* p)
	{
		#if defined(__LITTLE_ENDIAN__)
		store_with_swapping_endian(p);
		#else
		store((T*)p);
		#endif
	}
	constexpr  void load_limited(T* p, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = 0; i < _limit2; i++) {
			m_data[i] = p[i];
		}
	}
	constexpr  void load_offset(T* p, const size_t offset, const size_t _limit = 8)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j++) {
			m_data[i] = p[j];
		}
	}
	constexpr T *load2(T* p)
	{
		size_t j = 0;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i += 2, j++) {
			m_data[i]     = p[j];
			m_data[i + 1] = p[j];
		}
		return &(p[4]);
	}
	constexpr void load4(T* p)
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
		constexpr void load(T2* p)
	{
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (T)(p[i]);
		}
	}
	template <typename T2>
		constexpr  void load_limited(T2* p, const size_t _limit)
	{

		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = 0; i < _limit2; i++) {
			m_data[i] = (T2)(p[i]);
		}
	}
	template <typename T2>
		constexpr  void load_offset(T2* p, const size_t offset, const size_t _limit = 8)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j++) {
			m_data[i] = (T2)(p[j]);
		}
	}
	template <class T2>
		constexpr void load2(T2* p)
	{
		for(size_t i0 = 0, i1 = 1, k = 0; k < 4; i0 += 2, i1 += 2, k++) {
			m_data[i0] = (T)(p[k]);
			m_data[i1] = (T)(p[k]);
		}
	}
	template <class T2>
		constexpr void load4(T2* p)
	{
		for(size_t i0 = 0, i1 = 1, i2 = 2, i3 = 3, k = 0; k < 2; i0 += 4, i1 += 4, i2 += 4, i3 += 4, k++) {
			m_data[i0] = (T)(p[k]);
			m_data[i1] = (T)(p[k]);
			m_data[i2] = (T)(p[k]);
			m_data[i3] = (T)(p[k]);
		}
	}
	// Pointer may be unaligned, or aligned.
	constexpr void store(T* p)
	{
		for(size_t i = 0; i < 8; i++) {
			p[i] = m_data[i];
		}
	}

	constexpr void store_limited(T* p, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = 0; i < _limit2; i++) {
			p[i] = m_data[i];
		}
	}
	constexpr void store_offset(T* p, const size_t offset, const size_t _limit = 8)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j++) {
			p[i] = m_data[j];
		}
	}
	template <class T2>
		constexpr void store(T2* p)
	{
		for(size_t i = 0; i < 8; i++) {
			p[i] = (T2)(m_data[i]);
		}
	}
	template <class T2>
		constexpr void store_limited(T2* p, size_t _limit)
	{
		for(size_t i = 0; (i < 8) && (i < _limit); i++) {
			p[i] = (T2)(m_data[i]);
		}
	}
	template <class T2>
		constexpr void store_offset(T2* p, const size_t offset, const size_t _limit = 8)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j++) {
			p[i] = (T2)(m_data[j]);
		}
	}
	constexpr void store2(T* p)
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
	constexpr void store2_limited(T* p, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = 0, j = 0; i < _limit2; i++, j += 2) {
			T tmpval = m_data[i];
			p[j] = tmpval;
			p[j + 1] = tmpval;
		}
	}
	constexpr void store2_offset(T* p, const size_t offset, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j += 2) {
			T tmpval = m_data[i];
			p[j] = tmpval;
			p[j + 1] = tmpval;
		}
	}
	template <class T2>
		constexpr void store2(T2* p)
	{
		for(size_t i = 0, j = 0; i < 8; i++, j += 2) {
			T2 tmpval = (T2)(m_data[i]);
			p[j] = tmpval;
			p[j + 1] = tmpval;
		}
	}
	template <class T2>
		constexpr void store2_limited(T2* p, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = 0, j = 0; i < _limit2; i++, j += 2) {
			T2 tmpval = (T2)(m_data[i]);
			p[j] = tmpval;
			p[j + 1] = tmpval;
		}
	}
	template <class T2>
		constexpr void store2_offset(T2* p, const size_t offset, const size_t _limit)
	{
		const size_t _limit2 = (_limit >= 8) ? 8 : _limit;
		for(size_t i = offset, j = 0; i < _limit2; i++, j += 2) {
			T2 tmpval = (T2)m_data[i];
			p[j] = tmpval;
			p[j + 1] = tmpval;
		}
	}
	constexpr void store4(T* p)
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
	constexpr void store4_limited(T* p, const size_t _limit)
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
	constexpr void store4_offset(T* p, const size_t offset, const size_t _limit)
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
		constexpr void store4(T2* p)
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
		constexpr void store4_limited(T2* p, size_t _limit)
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
		constexpr void store4_offset(T2* p, const size_t offset, const size_t _limit)
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
	constexpr void store_n(const T* p, const size_t _mag)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			T tmp = m_data[i];
			for(size_t j = 0; j < _mag; j++) {
				*p++ = tmp;
			}
		}
	}
	constexpr void store_n_limited(const T* p, const size_t _mag, const size_t _limit = 8)
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
	constexpr void store_n_limited(const T* p, const size_t _mag, const size_t offset, const size_t _limit = 8)
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
		constexpr void store_n(T2* p, const size_t _mag)
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
		constexpr void store_n_limited(T2* p, const size_t _mag, const size_t _limit)
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
		T* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = q[i];
		}
	}
	template <class T2>
		void load_aligned(T2* p)
	{
		T2* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (T)(q[i]);
		}
	}

	// Pointer must be aligned minimum of 16 bytes.
	inline void store_aligned(T* p) const
	{
		T* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			q[i] = m_data[i];
		}
	}
	template <class T2>
		inline void store_aligned(T2* p) const
	{
		T2* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			q[i] = (T2)(m_data[i]);
		}
	}
	constexpr void store2_aligned(T* p)
	{
		csp_vector8<T> tmpval[2];
		for(size_t k = 0; k < 2; k++) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = (k * 4), j = 0; i < ((k * 4) + 4); i++, j += 2) {
				tmpval[k].set(j    , m_data[i]);
				tmpval[k].set(j + 1, m_data[i]);
			}
		}
		T* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t k = 0; k < 2; k++) {
			tmpval[k].store_aligned(&(q[k * 8]));
		}
	}
	constexpr void store4_aligned(T* p)
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
		T* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t k = 0; k < 4; k++) {
			tmpval[k].store_aligned(&(q[k * 8]));
		}
	}
	
	constexpr void get(const csp_vector8<T>& __b)
	{
		__b.store_aligned(m_data);
	}

	template <class T2>
		constexpr void get(csp_vector8<T2>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = __b.at(i);
		}
	}

	constexpr void put(const csp_vector8<T>& __b)
	{
		__b.load_aligned(m_data);
	}

	template <class T2>
		constexpr void put(csp_vector8<T2>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__b.set(i, (T2)(m_data[i]));
		}
	}
	
	inline void clear()
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (T)0;
		}
	}

	inline void fill(T __val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t __n = 0; __n < 8; __n++) {
			m_data[__n] = __val;
		}
	}
	inline void set(size_t __n, T __val)
	{
		m_data[__n] = __val;
	}
	inline void reset(size_t __n)
	{
		m_data[__n] = (T)0;
	}
	
	
	constexpr void lshift(const size_t pos, const size_t val)
	{
		m_data[pos] <<= val;
	}
	constexpr void lshift(const size_t val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] <<= val;
		}
	}
	
	constexpr void rshift(const size_t pos, const size_t val)
	{
		m_data[pos] >>= val;
	}
	constexpr csp_vector8<T>& rshift(const size_t val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] >>= val;
		}
		return *this;
	}

	template <class T2>
		constexpr csp_vector8<T>& shift(const T2 val)
	{
		constexpr bool _signed = std::is_signed<T2>().value;
		if(_signed) {
			constexpr ssize_t __shift = (ssize_t)val;
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
		constexpr void shift(size_t pos, const T2 val)
	{
		constexpr bool _signed = std::is_signed<T2>().value;
		if(_signed) {
			constexpr ssize_t __shift = (ssize_t)val;
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
	
	constexpr void shift(size_t pos, const ssize_t val)
	{
		if(val < 0) {
			rshift(pos, (-val) % ((sizeof(T) * 8) - 1));
		} else if(val > 0) {
			lshift(pos, val % ((sizeof(T) * 8) - 1));
		}
	}
	constexpr void shift(size_t pos, const size_t val)
	{
		lshift(pos, val % (sizeof(T) * 8));
	}
	constexpr csp_vector8<T>&  shift(csp_vector8<ssize_t>& val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			shift(i, val.at(i));
		}
		return *this;
	}
	
	constexpr csp_vector8<T>& clamp_upper(const T upper_val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (m_data[i] > upper_val) ? upper_val : m_data[i];
		}
		return *this;
	}
	constexpr csp_vector8<T>& clamp_lower(const T lower_val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (m_data[i] < lower_val) ? lower_val : m_data[i];
		}
		return *this;
	}
	constexpr csp_vector8<T>& clamp(const T upper_val, const T lower_val)
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
	constexpr csp_vector8<T>& negate()
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = ~(m_data[i]);
		}
		return *this;
	}
	constexpr csp_vector8<T>& negate(const csp_vector8<T> __b)
	{
		__b.store_aligned(m_data);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = ~(m_data[i]);
		}
		return *this;
	}
	template <typename T2>
		constexpr csp_vector8<T>& lookup(csp_vector8<T2>& __list, T* __table, const size_t count = 8)
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
		constexpr csp_vector8<T>& lookup(csp_vector8<T2>& __list, T2 _limit, T* __table, const size_t count = 8)
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
		constexpr csp_vector8<T>& lookup(csp_vector8<T2>& __list, const T2 _min, const T2 _max, T* __table, const size_t count = 8)
	{
		csp_vector8<T2> rlist(__list);
		size_t _count = ((count > 8) || (count == 0)) ? 8 : count;
		T2 _min2 = _min;
		T2 _max2 = _max;
		if(_min > _max) std::swap(_min2, _max2);

		rlist.clamp(_max2, _min2);

		for(size_t i = 0; i < _count; i++) {
			m_data[i] = __table[rlist[i]];
		}
		return *this;
	}

	constexpr csp_vector8<T>& set_cond(csp_vector8<bool>& __flags, const T __true_val, const T __false_val)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (__flags.at(i)) ? __true_val : __false_val;
		}
		return *this;
	}
	constexpr csp_vector8<T>& set_if_true(csp_vector8<bool>& __flags, const T __val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (__flags.at(i)) ?  m_data[i] : __val;
		}
		return *this;
	}
	constexpr csp_vector8<T>& set_if_false(csp_vector8<bool>& __flags, const T __val)
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

	constexpr T operator[](const size_t& __n)
	{
		return m_data[__n];
	}
	constexpr csp_vector8<T>& operator=(const csp_vector8<T>& __b)
	{
		__b.store_aligned(m_data);
		return *this;
	}
	constexpr csp_vector8<T>& operator~()
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = ~(m_data[i]);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator+=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] += __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator-=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] -= __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator/=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] /= __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator*=(csp_vector8<T>& __b)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] *= __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator&=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] &= __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator|=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] |= __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator^=(csp_vector8<T>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] ^= __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator<<=(const size_t __n)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] <<= __n;
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator>>=(const size_t __n)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] >>= __n;
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator>>=(csp_vector8<size_t>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] >>= (size_t)(__b.at(i));
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator<<=(csp_vector8<size_t>& __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] <<= (size_t)(__b.at(i));
		}
		return *this;
	}

	constexpr bool operator==(csp_vector8<T>& __a)
	{
		bool __f = true;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (__a.at(i) == m_data[i]);
		}
		return __f;
	}
	constexpr bool operator==(const T __a)
	{
		bool __f = true;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (m_data[i] == __a);
		}
		return __f;
	}
	constexpr bool operator!=(csp_vector8<T>& __a)
	{
		bool __f = true;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (__a.at(i) != m_data[i]);
		}
		return __f;
	}
	constexpr bool operator!=(const T __a)
	{
		bool __f = true;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (__a != m_data[i]);
		}
		return __f;
	}

	constexpr void equals(csp_vector8<bool>& __ret, csp_vector8<T>& __a)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (__a.at(i) == m_data[i]));
		}
	}
	constexpr void equals(csp_vector8<bool>& __ret, const T __a)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (m_data[i] == __a));
		}
	}
	constexpr void not_equals(csp_vector8<bool>& __ret, csp_vector8<T>& __a)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (__a.at(i) != m_data[i]));
		}
	}
	constexpr void not_equals(csp_vector8<bool>& __ret, const T __a)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (m_data[i] != __a));
		}
	}
	constexpr void check_bits(csp_vector8<bool>& __ret, const T& _bitmask)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, ((m_data[i] & _bitmask) == _bitmask));
		}
	}
	// Maybe faster than check_bits().
	constexpr void check_any_bits(csp_vector8<bool>& __ret, const T& _bitmask)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, ((m_data[i] & _bitmask) != 0));
		}
	}

};


template <class T>
	constexpr csp_vector8<T>& operator+(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret += __b;
	return __ret;
}

// Primitive operators must define outside of class :-(
template <class T>
	constexpr csp_vector8<T>& operator-(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret -= __b;
	return __ret;
}

template <class T>
	constexpr csp_vector8<T>& operator*(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret *= __b;
	return __ret;
}

template <class T>
	constexpr csp_vector8<T>& operator/(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret /= __b;
	return __ret;
}

template <class T>
	constexpr csp_vector8<T>& operator&(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret &= __b;
	return __ret;
}

template <class T>
	constexpr csp_vector8<T>& operator|(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret |= __b;
	return __ret;
}
template <class T>
	constexpr csp_vector8<T>& operator^(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret ^= __b;
	return __ret;
}

template <class T>
	constexpr csp_vector8<T>& operator<<(const csp_vector8<T>& __a, const size_t& __shift)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret <<= __shift;
	return __ret;
}

template <class T>
	constexpr csp_vector8<T>& operator>>(const csp_vector8<T>& __a, const size_t& __shift)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret >>= __shift;
	return __ret;
}

template <class T>
	constexpr csp_vector8<T>& operator<<(const csp_vector8<T>& __a, csp_vector8<size_t>& __shift)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret <<= __shift;
	return __ret;
}

template <class T>
	constexpr csp_vector8<T>& operator>>(const csp_vector8<T>& __a, csp_vector8<size_t>& __shift)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) csp_vector8<T> __ret(__a);
	__ret >>= __shift;
	return __ret;
}

template <class T>
	constexpr csp_vector8<bool>& operator==(csp_vector8<bool>& __a, const csp_vector8<T>& __b)
{
	equals(__a, __b);
	return __a;
}

template <class T>
	constexpr csp_vector8<bool>& operator==(csp_vector8<bool>& __a, const T __b)
{
	equals(__a, __b);
	return __a;
}

template <class T>
	constexpr csp_vector8<bool>& operator!=(csp_vector8<bool>& __a, const T __b)
{
	not_equals(__a, __b);
	return __a;
}

template <class T>
	constexpr csp_vector8<T>& operator~(csp_vector8<T>& __a)
{
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		__a.set(i, ~(__a.at(i)));
	}
	return __a;
}

template <class T>
	void make_rgba_vec8(csp_vector8<scrntype_t>& dst, csp_vector8<T> r ,csp_vector8<T> g , csp_vector8<T> b, csp_vector8<T> a)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) scrntype_t r_data[8];
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) scrntype_t g_data[8];
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) scrntype_t b_data[8];
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) scrntype_t tmpdata[8];
	r.store_aligned(r_data);
	g.store_aligned(g_data);
	b.store_aligned(b_data);

	#if defined(_RGB555) || defined(_RGB565)
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		dst.set(i, RGBA_COLOR(r_data[i], g_data[i], b_data[i], 255));
	}
	#else /* RGBA32 */
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) scrntype_t a_data[8];
	a.store_aligned(a_data);
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		dst.set(i, RGBA_COLOR(r_data[i], g_data[i], b_data[i], a_data[i]));
	}
	#endif

}

template <class T>
	void make_rgb_vec8(csp_vector8<scrntype_t>& dst, csp_vector8<T> r ,csp_vector8<T> g , csp_vector8<T> b)
{
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) scrntype_t r_data[8];
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) scrntype_t g_data[8];
	__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) scrntype_t b_data[8];

	r.store_aligned(r_data);
	g.store_aligned(g_data);
	b.store_aligned(b_data);

	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		dst.set(i, RGBA_COLOR(r_data[i], g_data[i], b_data[i], 255));
	}

}

// Please include type specified (and MPU specified) templates.

#undef __M__MINIMUM_ALIGN_LENGTH
