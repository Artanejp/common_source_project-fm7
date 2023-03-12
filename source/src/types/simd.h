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
	csp_vector8(const csp_vector8 __a)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = __a.at(i);
		}
	}
	csp_vector8(const T* p)
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
	csp_vector8(const T n = (const T)0)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = n;
		}
	}
	~csp_vector8() {}

	constexpr T at(size_t n)
	{
		return m_data[n];
	}
	// Pointer may be unaligned, or aligned.
	inline void load(T* p)
	{
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = p[i];
		}
	}
	// Pointer may be unaligned, or aligned.
	inline void store(T* p)
	{
		for(size_t i = 0; i < 8; i++) {
			p[i] = m_data[i];
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

	// Pointer must be aligned minimum of 16 bytes.
	constexpr void store_aligned(T* p)
	{
		T* q = ___assume_aligned(p, __M__MINIMUM_ALIGN_LENGTH);
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			q[i] = m_data[i];
		}
	}
	inline void copy(const csp_vector8<T> __b)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = __b.at(i);
		}
	}
	inline void clear()
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (T)0;
		}
	}
	constexpr T set(size_t __n, T __val)
	{
		m_data[__n] = __val;
	}
	constexpr T reset(size_t __n)
	{
		m_data[__n] = (T)0;
	}

	// Pointer must be aligned minimum of 16 bytes.
	constexpr csp_vector8<T>& set_cond(const csp_vector8<bool>& __flags, const T __true_val, const T __false_val)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (__flags.at(i)) ? __true_val : __false_val;
		}
		return *this;
	}
	constexpr void set_cond(const csp_vector8<bool>& __flags, const T __true_val, const T __false_val)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = (__flags.at(i)) ? __true_val : __false_val;
		}
	}
	constexpr void set_if_true(const csp_vector8<bool>& __flags, const T __val)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			if(__flags.at(i)) {
				m_data[i] = __val;
			}
		}
	}
	inline void shuffle(const csp_vector8<uint8_t>& __positions)
	{
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) T __d[8];
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) uint8_t __p[8];
		__DECL_ALIGNED(__M__MINIMUM_ALIGN_LENGTH) const uint8_t __m[8] =
			{7, 7, 7, 7,
			 7, 7, 7, 7};

		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__p[i] = __positions.at(i);
		}
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__p[i] &= __m[i];
		}

		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__d[i] = m_data[__n[i]];
		}
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = __d[i];
		}
	}

	constexpr T operator[](size_type __n)
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
		retrurn *this;
	}
	constexpr csp_vector8<T>& operator>>=(const size_t __n)
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] >>= __n;
		}
		retrurn *this;
	}
	constexpr csp_vector8<T>& operator~()
	{
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			m_data[i] = ~(m_data[i]);
		}
		return *this;
	}

	constexpr csp_vector8<T>& operator+(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
	{
		copy(__a);
		*this += __b;
		return *this;
	}
	constexpr csp_vector8<T>& operator-(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
	{
		copy(__a);
		*this -= __b;
		return *this;
	}
	constexpr csp_vector8<T>& operator*(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
	{
		copy(__a);
		*this *= __b;
		return *this;
	}
	constexpr csp_vector8<T>& operator/(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
	{
		copy(__a);
		*this /= __b;
		return *this;
	}
	constexpr csp_vector8<T>& operator&(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
	{
		copy(__a);
		*this &= __b;
		return *this;
	}
	constexpr csp_vector8<T>& operator|(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
	{
		copy(__a);
		*this |= __b;
		return *this;
	}
	constexpr csp_vector8<T>& operator^(const csp_vector8<T>& __a, const csp_vector8<T>& __b)
	{
		copy(__a);
		*this ^= __b;
		return *this;
	}
	constexpr csp_vector8<T>& operator>>(const csp_vector8<T>& __a, size_t __n)
	{
		copy(__a);
		*this >>= __n;
		return *this;
	}
	constexpr csp_vector8<T>& operator<<(const csp_vector8<T>& __a, size_t __n)
	{
		copy(__a);
		*this <<= __n;
		return *this;
	}
	constexpr bool& operator==(const csp_vector8<T>& __a)
	{
		bool __f = true;
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (__a.at(i) == m_data[i]);
		}
		return __f;
	}
	constexpr bool& operator==(const T __a)
	{
		bool __f = true;
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (m_data[i] == __a);
		}
		return __f;
	}
	constexpr bool& operator!=(const csp_vector8<T>& __a)
	{
		bool __f = true;
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (__a.at(i) != m_data[i]);
		}
		return __f;
	}
	constexpr bool& operator!=(const T __a)
	{
		bool __f = true;
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__f &= (__a != m_data[i]);
		}
		return __f;
	}

	constexpr csp_vector8<bool>& operator==(const csp_vector8<T>& __a)
	{
		csp_vector8<bool> __ret(false);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (__a.at(i) == m_data[i]) ? true : false);
		}
		return __ret;
	}
	constexpr csp_vector8<bool>& operator==(const T __a)
	{
		csp_vector8<bool> __ret(false);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (m_data[i] == __a) ? true : false);
		}
		return __ret;
	}
	constexpr csp_vector8<bool>& operator!=(const csp_vector8<T>& __a)
	{
		csp_vector8<bool> __ret(false);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (__a.at(i) != m_data[i]) ? true : false);
		}
		return __ret;
	}
	constexpr csp_vector8<bool>& operator!=(const T __a)
	{
		csp_vector8<bool> __ret(false);
	__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			__ret.set(i, (m_data[i] != __a) ? true : false);
		}
		return __ret;
	}

};
// Please include type specified (and MPU specified) templates.

#undef __M__MINIMUM_ALIGN_LENGTH
