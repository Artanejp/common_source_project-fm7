/*
	Skelton for retropc emulator

	Author  : Kyuma Ohta <whatisthis.sowhat@gmail.com>
	Date    : 2023.03.13-
	License : GPLv2
	[ simd utils ]

*/

#pragma once

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
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = __a.at(i);
		}
	}
	constexpr csp_vector8(const T* p)
	{
		if(p != nullptr) {
		__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				m_data[i] = p[i];
			}
		} else {
		__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				m_data[i] = 0;
			}
		}
	}
	constexpr csp_vector8(const T n = (const T)0)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = n;
		}
	}
	~csp_vector8() {}

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
	constexpr  void load_limited(T* p, const size_t _limit)
	{

		for(size_t i = 0; (i < 8) && (i < _limit); i++) {
			m_data[i] = p[i];
		}
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

		for(size_t i = 0; (i < 8) && (i < _limit); i++) {
			m_data[i] = (T2)(p[i]);
		}
	}
	// Pointer may be unaligned, or aligned.
	constexpr void store(T* p)
	{
		for(size_t i = 0; i < 8; i++) {
			p[i] = m_data[i];
		}
	}
	constexpr void store_limited(T* p, size_t _limit)
	{
		for(size_t i = 0; (i < 8) && (i < _limit); i++) {
			p[i] = m_data[i];
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
	constexpr void store2(T* p)
	{
		for(size_t i = 0, j = 0; i < 8; i++, j += 2) {
			p[j] = m_data[i];
			p[j + 1] = m_data[i];
		}
	}
	constexpr void store2_limited(T* p, const size_t _limit)
	{
		for(size_t i = 0, j = 0; (i < 8) && (i < _limit); i++, j += 2) {
			p[j] = m_data[i];
			p[j + 1] = m_data[i];
		}
	}
	template <class T2>
		constexpr void store2(T2* p)
	{
		for(size_t i = 0, j = 0; i < 8; i++, j += 2) {
			p[j] = (T2)(m_data[i]);
			p[j + 1] = (T2)(m_data[i]);
		}
	}
	template <class T2>
		constexpr void store2_limited(T2* p, const size_t _limit)
	{
		for(size_t i = 0, j = 0; (i < 8) && (i < _limit); i++, j += 2) {
			p[j] = (T2)(m_data[i]);
			p[j + 1] = (T2)(m_data[i]);
		}
	}
	constexpr void store4(T* p)
	{
		for(size_t i = 0, j = 0; i < 8; i++, j += 4) {
			p[j] = m_data[i];
			p[j + 1] = m_data[i];
			p[j + 2] = m_data[i];
			p[j + 3] = m_data[i];
		}
	}
	constexpr void store4_limited(T* p, const size_t _limit)
	{
		for(size_t i = 0, j = 0; (i < 8) && (i < _limit); i++, j += 4) {
			p[j] = m_data[i];
			p[j + 1] = m_data[i];
			p[j + 2] = m_data[i];
			p[j + 3] = m_data[i];
		}
	}
	template <class T2>
		constexpr void store4(T2* p)
	{
		for(size_t i = 0, j = 0; i < 8; i++, j += 4) {
			p[j] = (T2)(m_data[i]);
			p[j + 1] = (T2)(m_data[i]);
			p[j + 2] = (T2)(m_data[i]);
			p[j + 3] = (T2)(m_data[i]);
		}
	}
	template <class T2>
		constexpr void store4_limited(T2* p, size_t _limit)
	{
		for(size_t i = 0, j = 0; (i < 8) && (i < _limit); i++, j += 4) {
			p[j] = (T2)(m_data[i]);
			p[j + 1] = (T2)(m_data[i]);
			p[j + 2] = (T2)(m_data[i]);
			p[j + 3] = (T2)(m_data[i]);
		}
	}
	constexpr void store_n(const T* p, const size_t _mag)
	{

		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T _tmp[8];
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			_tmp[i] = m_data[i];
		}
		for(size_t i = 0; i < 8; i++) {
			for(size_t j = 0; j < _mag; j++) {
				*p++ = _tmp[i];
			}
		}
	}
	constexpr void store_n_limited(const T* p, const size_t _mag, const size_t _limit)
	{

		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T _tmp[8];
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			_tmp[i] = m_data[i];
		}
		for(size_t i = 0; (i < 8) && (i < _limit); i++) {
			for(size_t j = 0; j < _mag; j++) {
				*p++ = _tmp[i];
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
	constexpr void load_aligned(T* p)
	{
		T* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = q[i];
		}
	}
	template <class T2>
		constexpr void load_aligned(T2* p)
	{
		T2* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (T)(q[i]);
		}
	}

	// Pointer must be aligned minimum of 16 bytes.
	constexpr void store_aligned(T* p)
	{
		T* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			q[i] = m_data[i];
		}
	}
	template <class T2>
		constexpr void store_aligned(T2* p)
	{
		T2* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			q[i] = (T2)(m_data[i]);
		}
	}
	constexpr void store2_aligned(T* p)
	{
		T* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0, j = 0; i < 8; i++, j += 2) {
			q[j] = m_data[i];
			q[j + 1] = m_data[i];
		}
	}
	constexpr void store4_aligned(T* p)
	{
		T* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0, j = 0; i < 8; i++, j += 4) {
			q[j] = m_data[i];
			q[j + 1] = m_data[i];
			q[j + 2] = m_data[i];
			q[j + 3] = m_data[i];
		}
	}
	inline void copy(const csp_vector8<T> __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = __b.at(i);
		}
	}
	template <class T2>
		constexpr void get(csp_vector8<T2> __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (T)(__b.at(i));
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
	inline void  set(size_t __n, T __val)
	{
		m_data[__n] = __val;
	}
	inline void reset(size_t __n)
	{
		m_data[__n] = (T)0;
	}
	template <typename T2>
		constexpr csp_vector8<T>& lookup(csp_vector8<T2>& __list, T* __table, const size_t count = 8)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T2 rlist[8] = {0};
		size_t _count = ((count > 8) || (count == 0)) ? 8 : count;
		constexpr bool _is_signed = std::is_signed<T2>().value;

		for(size_t i = 0; i < _count; i++) {
			rlist[i] = __list.at(i);
		}
		if(_is_signed) {
		__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				rlist[i] = (rlist[i] < 0) ? 0 : rlist[i];
			}
		}

		for(size_t i = 0; i < _count; i++) {
			m_data[i] = __table[rlist[i]];
		}
		return *this;
	}
	template <typename T2>
		constexpr csp_vector8<T>& lookup(csp_vector8<T2>& __list, T2 _limit, T* __table, const size_t count = 8)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T2 rlist[8] = {0};
		constexpr bool _is_signed = std::is_signed<T2>().value;
		size_t _count = ((count > 8) || (count == 0)) ? 8 : count;

		for(size_t i = 0; i < _count; i++) {
			rlist[i] = __list.at(i);
		}
		if(_is_signed) {
		__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				rlist[i] = (rlist[i] < 0) ? 0 : rlist[i];
			}
		}
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			rlist[i] = (rlist[i] > _limit) ? _limit : rlist[i];
		}

		for(size_t i = 0; i < _count; i++) {
			m_data[i] = __table[rlist[i]];
		}
		return *this;
	}
	template <typename T2>
		constexpr csp_vector8<T>& lookup(csp_vector8<T2>& __list, T2 _min, T2 _max, T* __table, const size_t count = 8)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T2 rlist[8] = {0};
		size_t _count = ((count > 8) || (count == 0)) ? 8 : count;
		if(_min > _max) std::swap(_min, _max);

		for(size_t i = 0; i < _count; i++) {
			rlist[i] = __list.at(i);
		}
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			rlist[i] = (rlist[i] < _min) ? _min : rlist[i];
		}
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			rlist[i] = (rlist[i] > _max) ? _max : rlist[i];
		}


		for(size_t i = 0; i < _count; i++) {
			m_data[i] = __table[rlist[i]];
		}
		return *this;
	}
	// Pointer must be aligned minimum of 16 bytes.
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
			if(__flags.at(i)) {
				m_data[i] = __val;
			}
		}
		return *this;
	}
	constexpr csp_vector8<T>& set_if_false(csp_vector8<bool>& __flags, const T __val)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			if(!(__flags.at(i))) {
				m_data[i] = __val;
			}
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
				T2 _tmp = __positions.at(i);
				__p[i] = (_tmp < 0) ? (uint8_t)(_tmp) : (uint8_t)(-_tmp);
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
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			_p.set(i, (T3U)(__positions.at(i)));
		}
		shuffle(_p);
	}

	constexpr T operator[](const size_t& __n)
	{
		return m_data[__n];
	}
	constexpr csp_vector8<T>& operator=(const csp_vector8<T>& __b)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator+=(const csp_vector8<T>& __b)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] += __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator-=(const csp_vector8<T>& __b)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] -= __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator/=(const csp_vector8<T>& __b)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] /= __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator*=(const csp_vector8<T>& __b)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] *= __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator&=(const csp_vector8<T>& __b)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] &= __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator|=(const csp_vector8<T>& __b)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] |= __b.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator^=(const csp_vector8<T>& __b)
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
	constexpr csp_vector8<T>& operator>>=(csp_vector8<size_t>& __n)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] >>= __n.at(i);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator<<=(csp_vector8<size_t>& __n)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] <<= __n.at(i);
		}
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

	constexpr bool operator==(const csp_vector8<T>& __a)
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
	constexpr bool operator!=(const csp_vector8<T>& __a)
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

	constexpr void equals(csp_vector8<bool>& __ret, const csp_vector8<T>& __a)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (__a.at(i) == m_data[i]) ? true : false);
		}
	}
	constexpr void equals(csp_vector8<bool>& __ret, const T __a)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (m_data[i] == __a) ? true : false);
		}
	}
	constexpr void not_equals(csp_vector8<bool>& __ret, const csp_vector8<T>& __a)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (__a.at(i) != m_data[i]) ? true : false);
		}
	}
	constexpr void not_equals(csp_vector8<bool>& __ret, const T __a)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (m_data[i] != __a) ? true : false);
		}
	}
	constexpr void check_bits(csp_vector8<bool>& __ret, const T& _bitmask)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, ((m_data[i] & _bitmask) == _bitmask) ? true : false);
		}
	}
	// Maybe faster than check_bits().
	constexpr void check_any_bits(csp_vector8<bool>& __ret, const T& _bitmask)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, ((m_data[i] & _bitmask) != 0) ? true : false);
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

// Please include type specified (and MPU specified) templates.

#undef __M__MINIMUM_ALIGN_LENGTH
