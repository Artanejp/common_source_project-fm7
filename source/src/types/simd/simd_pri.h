#pragma once

#include "../optimizer_utils.h"
/*
 * ToDo:
 * - Implement EQUALS, GT, LT...
 * - Implement add, sub, mul, div, mod
 * -- 20260219 K.O
 */
class csp_simd_pri
{
//protected:
//	size_t memb;
//	T* m_unaligned_ptr;
//	T* aligned_ptr;
public:
	//T _d;
	virtual ~csp_simd_pri() = default;

	virtual inline const bool is_aligned(void *p) = 0;
	// Read
	virtual inline void align_load(void* p) = 0;
	virtual inline void unalign_load(void* p) = 0;
	virtual inline void load(void *p) = 0;
	//template <typename Y>
	//	size_t load_left(void *p, const size_t num) = 0;
	//template <typename Y>
	//	size_t load_right(void *p, const size_t num) = 0;
	
	virtual inline void align_store(void* p) = 0;
	virtual inline void unalign_store(void* p) = 0;
	virtual inline void store(void *p) = 0;
	//	template <typename Y>
	//	size_t store_left(Y *p, const size_t num) = 0;
	//	template <typename Y>
	//	size_t store_right(Y *p, const size_t num) = 0;

	
	virtual inline void clear() = 0;
	virtual inline void fill(uint8_t val) = 0;
	virtual inline void fill(int8_t val) = 0;
	virtual inline void fill(uint16_t val) = 0;
	virtual inline void fill(int16_t val) = 0;
	virtual inline void fill(uint32_t val) = 0;
	virtual inline void fill(int32_t val) = 0;
	virtual inline void fill(uint64_t val) = 0;
	virtual inline void fill(int64_t val) = 0;

	virtual inline void set(size_t pos, uint8_t val) = 0;
	virtual inline void set(size_t pos, int8_t val) = 0;
	virtual inline void set(size_t pos, uint16_t val) = 0;
	virtual inline void set(size_t pos, int16_t val) = 0;
	virtual inline void set(size_t pos, uint32_t val) = 0;
	virtual inline void set(size_t pos, int32_t val) = 0;
	virtual inline void set(size_t pos, uint64_t val) = 0;
	virtual inline void set(size_t pos, int64_t val) = 0;

	//virtual inline uint8_t    at(size_t pos) = 0;
	//virtual inline int8_t     at(size_t pos) = 0;
	//virtual inline uint16_t   at(size_t pos) = 0;
	//virtual inline int16_t    at(size_t pos) = 0;
	//virtual inline uint32_t   at(size_t pos) = 0;
	//virtual inline int32_t    at(size_t pos) = 0;
	//virtual inline uint64_t   at(size_t pos) = 0;
	//virtual inline int64_t    at(size_t pos) = 0;
	
	//virtual inline T data() = 0;
	//virtual inline T* dptr() = 0;
	
	//virtual inline T bswap(T data, const size_t size = 4) = 0;
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
