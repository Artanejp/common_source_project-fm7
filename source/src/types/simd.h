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
		for(size_t i = 0, j = 0; i < 8; i++, j += 2) {
			T tmpval = m_data[i];
			p[j] = tmpval;
			p[j + 1] = tmpval;
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
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0, j = 0; i < 8; i++, j += 4) {
			T tmp = m_data[i];
			p[j] = tmp;
			p[j + 1] = tmp;
			p[j + 2] = tmp;
			p[j + 3] = tmp;
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
		T* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0, j = 0; i < 8; i++, j += 2) {
			T tmp = m_data[i];
			q[j] = tmp;
			q[j + 1] = tmp;
		}
	}
	constexpr void store4_aligned(T* p)
	{
		T* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0, j = 0; i < 8; i++, j += 4) {
			T tmp = m_data[i];
			q[j] = tmp;
			q[j + 1] = tmp;
			q[j + 2] = tmp;
			q[j + 3] = tmp;
		}
	}
	inline void copy(const csp_vector8<T> __b)
	{
		__b.store_aligned(m_data);
	}

	template <class T2>
		constexpr void get(csp_vector8<T2> __b)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T dst[8];
		__b.store_aligned(dst);

		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = dst[i];
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
	// Pointer must be aligned minimum of 16 bytes.
	constexpr csp_vector8<T>& set_cond(csp_vector8<bool>& __flags, const T __true_val, const T __false_val)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) bool __p[8];
		__flags.store_aligned(__p);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (__p[i]) ? __true_val : __false_val;
		}
		return *this;
	}
	constexpr csp_vector8<T>& set_if_true(csp_vector8<bool>& __flags, const T __val)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) bool __p[8];
		__flags.store_aligned(__p);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (__p[i]) ?  m_data[i] : __val;
		}
		return *this;
	}
	constexpr csp_vector8<T>& set_if_false(csp_vector8<bool>& __flags, const T __val)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) bool __p[8];
		__flags.store_aligned(__p);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (__p[i]) ? __val : m_data[i];
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
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = ~(m_data[i]);
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator+=(const csp_vector8<T>& __b)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__b.store_aligned(m_shadow);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] += m_shadow[i];
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator-=(const csp_vector8<T>& __b)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__b.store_aligned(m_shadow);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] -= m_shadow[i];
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator/=(const csp_vector8<T>& __b)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__b.store_aligned(m_shadow);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] /= m_shadow[i];
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator*=(const csp_vector8<T>& __b)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__b.store_aligned(m_shadow);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] *= m_shadow[i];
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator&=(const csp_vector8<T>& __b)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__b.store_aligned(m_shadow);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] &= m_shadow[i];
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator|=(const csp_vector8<T>& __b)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__b.store_aligned(m_shadow);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] |= m_shadow[i];
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator^=(const csp_vector8<T>& __b)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__b.store_aligned(m_shadow);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] ^= m_shadow[i];
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
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__b.store_aligned(m_shadow);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] >>= m_shadow[i];
		}
		return *this;
	}
	constexpr csp_vector8<T>& operator<<=(csp_vector8<size_t>& __b)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__b.store_aligned(m_shadow);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] <<= m_shadow[i];
		}
		return *this;
	}

	constexpr bool operator==(const csp_vector8<T>& __a)
	{
		bool __f = true;
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__a.store_aligned(m_shadow);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (m_shadow[i] == m_data[i]);
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
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__a.store_aligned(m_shadow);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (m_shadow[i] != m_data[i]);
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
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) bool m_result[8];
		__a.store_aligned(m_shadow);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_result[i] = (m_shadow[i] == m_data[i]);
		}
		__ret.load_aligned(m_result);
	}
	constexpr void equals(csp_vector8<bool>& __ret, const T __a)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) bool m_result[8];
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_result[i]  = (m_data[i] == __a);
		}
		__ret.load_aligned(m_result);
	}
	constexpr void not_equals(csp_vector8<bool>& __ret, const csp_vector8<T>& __a)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T m_shadow[8];
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) bool m_result[8];
		__a.store_aligned(m_shadow);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_result[i]  = (m_data[i] != m_shadow[i]);
		}
		__ret.load_aligned(m_result);
	}
	constexpr void not_equals(csp_vector8<bool>& __ret, const T __a)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) bool m_result[8];
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_result[i]  = (m_data[i] != __a);
		}
		__ret.load_aligned(m_result);
	}
	constexpr void check_bits(csp_vector8<bool>& __ret, const T& _bitmask)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) bool m_result[8];
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_result[i] = ((m_data[i] & _bitmask) == _bitmask);
		}
		__ret.load_aligned(m_result);
	}
	// Maybe faster than check_bits().
	constexpr void check_any_bits(csp_vector8<bool>& __ret, const T& _bitmask)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) bool m_result[8];
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_result[i] = ((m_data[i] & _bitmask) != 0);
		}
		__ret.load_aligned(m_result);
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
