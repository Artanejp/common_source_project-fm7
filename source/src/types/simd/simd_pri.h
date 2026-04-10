#pragma once

#include <type_traits>
#include "../optimizer_utils.h"
/*
 * ToDo:
 * - Implement EQUALS, GT, LT...
 * - Implement add, sub, mul, div, mod
 * -- 20260219 K.O
 */
class csp_simd_pri
{
protected:
//	size_t memb;
//	T* m_unaligned_ptr;
//	T* aligned_ptr;
	template <typename __T, typename __D>
		inline ssize_t __load_left_unsafe(__T* src, __D* __dp, const size_t num)
	{
		__T *q = (__T*)__dp;

		//__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < num; i++) {
			q[i] = src[i];
		}
		return (ssize_t)num;
	}
	
	template <typename __T, typename __D>
		inline ssize_t __load_left_safe(__T* src, __D* __dp, const size_t num)
	{
		const size_t num_limit = sizeof(__D) / sizeof(__T);
		__UNLIKELY_IF((src == nullptr) || (__dp == nullptr) || (num > num_limit) || (num_limit == 0)) {
			return (ssize_t)-1;
		}
		return __load_left_unsafe(src, __dp, num);
	}
	
	template <typename __T, typename __D>
		inline ssize_t __load_left(__T* src, __D* __dp, const size_t num)
	{
		return __load_left_safe(src, __dp, num);
	}
	
	template <typename __T, typename __D>
		inline ssize_t __load_right_unsafe(__T* src, __D* __dp, const size_t num)
	{
		const size_t num_limit = sizeof(__D) / sizeof(__T);
		__T *q = (__T*)__dp;
		//__DECL_VECTORIZED_LOOP
		for(size_t i = num; i < num_limit; i++) {
			q[i] = src[i];
		}
		return (ssize_t)(num_limit - num);
	}
	
	template <typename __T, typename __D>
		inline ssize_t __load_right_safe(__T* src, __D* __dp, const size_t num)
	{
		const size_t num_limit = sizeof(__D) / sizeof(__T);
		__UNLIKELY_IF((src == nullptr) || (num >= num_limit) || (num_limit == 0) || (__dp == nullptr) ) {
			return (ssize_t)-1;
		}
		return __load_right_unsafe(src, __dp, num);
	}
	template <typename __T, typename __D>
		inline ssize_t __load_right(__T* src, __D* __dp, const size_t num)
	{
		return __load_right_safe(src, __dp, num);
	}
	
	template <typename __T, typename __D>
		inline void __set_unsafe(size_t pos, __D* __dp, __T data)
	{
		__T* p =(__T*)__dp;
		p[pos] = data;
	}
	
	template <typename __T, typename __D>
		inline void __set_safe(size_t pos, __D* __dp, __T data)
	{
		const size_t num = sizeof(__D) / sizeof(__T);
		__UNLIKELY_IF((__dp == nullptr) || (pos >= num)) {
			return;
		}
		__set_unsafe(pos, __dp, data); 
	}
	template <typename __T, typename __D>
		inline void __set(size_t pos, __D* __dp, __T data)
	{
		__set_safe(pos, __dp, data);
	}
	
	template <typename __T, typename __D>
		inline __T __at_unsafe(size_t pos, __D* __sp)
	{
		__T* p =(__T*)__sp;
		return p[pos];
	}
	
	template <typename __T, typename __D>
		inline __T __at_safe(size_t pos, __D* __sp)
	{
		const size_t num = sizeof(__D) / sizeof(__T);
		__UNLIKELY_IF((__sp == nullptr) || (pos >= num)) {
			return (__T)0;
		}
		return __at_unsafe(pos, __sp);
	}
	template <typename __T, typename __D>
		inline __T __at(size_t pos, __D* __sp)
	{
		return __at_safe(pos, __sp);
	}
	template <typename __T, typename __D>
		inline ssize_t __store_left_unsafe(__T* dst, __D* __sp, const size_t num)
	{
		__T *q = (__T*)__sp;
		
		//__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < num; i++) {
			dst[i] = q[i];
		}
		return (ssize_t)num;
	}
	
	template <typename __T, typename __D>
		inline ssize_t __store_left_safe(__T* dst, __D* __sp, const size_t num)
	{
		const size_t num_limit = sizeof(__D) / sizeof(__T);
		__UNLIKELY_IF((dst == nullptr) || (__sp == nullptr) || (num > num_limit) || (num_limit == 0)) {
			return (ssize_t)-1;
		}
		return __store_left_unsafe(dst, __sp, num);
	}
	
	template <typename __T, typename __D>
		inline ssize_t __store_left(__T* dst, __D* __sp, const size_t num)
	{
		return __store_left_safe(dst, __sp, num);
	}
	
	template <typename __T, typename __D>
		inline ssize_t __store_right_unsafe(__T* dst, __D* __sp, const size_t num)
	{
		const size_t num_limit = sizeof(__D) / sizeof(__T);
		__T *q = (__T*)__sp;
		//__DECL_VECTORIZED_LOOP
		for(size_t i = num; i < num_limit; i++) {
			dst[i] = q[i];
		}
		return (ssize_t)(num_limit - num);
	}
	
	template <typename __T, typename __D>
		inline ssize_t __store_right_safe(__T* dst, __D* __sp, const size_t num)
	{
		const size_t num_limit = sizeof(__D) / sizeof(__T);
		__UNLIKELY_IF((dst == nullptr) || (num >= num_limit) || (num_limit == 0) || (__sp == nullptr) ) {
			return (ssize_t)-1;
		}
		return __store_right_unsafe(dst, __sp, num);
	}
	template <typename __T, typename __D>
		inline ssize_t __store_right(__T* dst, __D* __sp, const size_t num)
	{
		return __store_right_safe(dst, __sp, num);
	}

public:
	//T _d;
	virtual ~csp_simd_pri() = default;

	virtual inline const bool is_aligned(void *p) = 0;
	// Read
	virtual inline void align_load(void* p) = 0;
	virtual inline void unalign_load(void* p) = 0;
	virtual inline void load(void *p) = 0;

	virtual inline void align_store(void* p) = 0;
	virtual inline void unalign_store(void* p) = 0;
	virtual inline void store(void *p) = 0;

	virtual inline void clear() = 0;
	virtual inline void setall() = 0;
	virtual inline void fill(uint8_t val) = 0;
	virtual inline void fill(int8_t val) = 0;
	virtual inline void fill(uint16_t val) = 0;
	virtual inline void fill(int16_t val) = 0;
	virtual inline void fill(uint32_t val) = 0;
	virtual inline void fill(int32_t val) = 0;
	virtual inline void fill(uint64_t val) = 0;
	virtual inline void fill(int64_t val) = 0;

	
	
	//template <typename _T>
	//	inline _T data()
	//{
	//	return (_T)0;
	//}
	//template <typename _T>
	//	_T* dptr()
	//{
	//	return NULL;
	//}
	
	//virtual inline __T bswap(_T data, const size_t size = 4) = 0;
	//virtual inline void bswap_self(const size_t size = 4) = 0;
	
	//virtual inline csp_simd_pri<T>& operator-(const csp_simd_pri<T>& __b) = 0;
	//virtual inline csp_simd_pri<T>& operator+(const csp_simd_pri<T>& __b) = 0;
	
	//virtual inline csp_simd_pri<T>& operator=(const csp_simd_pri<T>& __b) = 0;
	//virtual inline csp_simd_pri<T>& operator=(const T& __b) = 0;
	
	//virtual inline csp_simd_pri<T>& operator&=(const csp_simd_pri<T>& __b) = 0;
	//virtual inline csp_simd_pri<T>& operator&=(const T __b) = 0;
	//virtual inline csp_simd_pri<T>& operator|=(const csp_simd_pri<T>& __b) = 0;
	//virtual inline csp_simd_pri<T>& operator|=(const T __b) = 0;
	//virtual inline csp_simd_pri<T>& operator^=(const csp_simd_pri<T>& __b) = 0;
	//virtual inline csp_simd_pri<T>& operator^=(const T __b) = 0;
	
	
	//virtual inline csp_simd_pri<T>& operator+=(const csp_simd_pri<T>& __b) = 0;
	//virtual inline csp_simd_pri<T>& operator+=(const T& __b) = 0;
	//virtual inline csp_simd_pri<T>& operator-=(const csp_simd_pri<T>& __b) = 0;
	//virtual inline csp_simd_pri<T>& operator-=(const T& __b) = 0;
	
	//virtual inline csp_simd_pri<T>& operator<<=(const size_t __b) = 0;
	//virtual inline csp_simd_pri<T>& operator>>=(const size_t __b) = 0;
	
};

// Base Template
template <typename __T>
	inline const bool is_aligned(__T* p, const size_t min_align)
{
	constexpr size_t check_align = (min_align == 0) ? sizeof(__T) : min_align;
	if(check_align <= 1) {
		return true;
	}
	const uintptr_t p_p = (const uintptr_t)p;
	const uintptr_t align_mask = (const uintptr_t)(check_align - 1);
	return ((align_mask & p_p) == 0) ? true : false;
}

template <typename __T, typename __U>
	inline size_t copy_multiple(const __T* dst, const __U* src, const size_t words = 0)
{
	__UNLIKELY_IF((dst == nullptr) || (src == nullptr) || (words == 0)) {
		return 0;
	}
	__DECL_VECTORIZED_LOOP /* OK? */
	for(size_t i = 0; i < words; i++) {
		dst[i] = (__T)(src[i]); 
	}
	return words;
}

template <typename __T, typename __U, typename __M>
	__U simd_lookup(__T* table, const __M pos, const size_t table_length)
{
	if(sizeof(__U) > sizeof(__T)) return (__U)0;
	__UNLIKELY_IF(table == nullptr) {
		return (__U)0;
	}
	__UNLIKELY_IF((table_length == 0) || (pos >= table_length)) return (__U)0;
	return (__U)(table[pos]);
}

