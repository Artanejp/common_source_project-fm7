#pragma once

#include "../simd_types.h"

/*
 * ToDo:
 * - Implement EQUALS, GT, LT...
 * - Implement add, sub, mul, div, mod
 * -- 20260219 K.O
 */
template <typename T>
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
	virtual inline void align_load(void* p) const = 0;
	virtual inline void unalign_load(void* p) const = 0;
	inline void load(void *p)
	{
		if(is_aligned(p)) {
			align_load(p);
		} else {
			unalign_load(p);
		}
	}
	template <typename Y>
		size_t load_left(void *p, const size_t num)
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
	template <typename Y>
		size_t load_right(void *p, const size_t num)
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
	
	virtual inline void align_store(void* p) const = 0;
	virtual inline void unalign_store(void* p) const = 0;
	inline void store(void *p)
	{
		if(is_aligned(p)) {
			align_store(p);
		} else {
			unalign_store(p);
		}
	}	
	template <typename Y>
		size_t store_left(Y *p, const size_t num)
	{
		__UNLIKELY_IF((sizeof(Y) > sizeof(T)) || (p == nullptr)) {
			return 0;
		}
		size_t off = 0;
		size_t i = 0;
		Y* pp = p;
		uint8_t* q = (uint8_t*)(&_d);
		Y* qq = (Y*)(&(q[off]));
		for(; (off < sizeof(T)) && (i < num); off += sizeof(Y), i++) {
			Y _t = qq[i];
			pp[i] = _t;
		}
		return _i;
	}
	template <typename Y>
		inline size_t store_right(Y *p, const size_t num)
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
		Y* pp = p;
		uint8_t* q = (uint8_t*)(&_d);
		Y* qq = (Y*)(&(q[off]));
		for(; ((off + sizeof(Y)) <= sizeof(T)) && (i < __num); off += sizeof(Y), i++) {
			Y _t = qq[i];
			pp[i] = _t;
		}
		return _i;
	}
	
	virtual inline void clear() const = 0;
	template <typename Y>
		void fill(Y val)
	{
		size_t off = 0;
		uint8_t* p = (uint8_t*)(&_d);
		Y* pp = (Y*)p;
		for(size_t i = 0; off < sizeof(T); off += sizeof(Y), i++) {
			pp[i] = val;
		}
	}
	template <typename Y>
		void set(size_t pos, Y val)
	{
		__UNLIKELY_IF((sizeof(T) / sizeof(Y)) <= pos) {
			return;
		}
		uint8_t* p = (uint8_t*)(&_d);
		Y* pp = (Y*)p;
		pp[pos] = val; // OK?
	}
	template <typename Y>
		Y at(size_t pos)
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
	template <typename Y>
		Y lookup(Y* tbl, uint16_t pos, constexpr size_t elements = 256) const = 0;
	template <typename Y>
		Y lookup(const csp_simd_pri<T>* tbl, uint16_t pos, constexpr size_t elements = 256) const = 0;
	template <typename Y>
		Y lookup(const csp_simd_pri<T>* tbl, const uint16_t[] pos_tbl, const uint16_t tbl_length, constexpr size_t elements = 256) const = 0;

	virtual inline T bswap(T data, constexpr size_t size = 4) const = 0;
	virtual inline void bswap_self(constexpr size_t size = 4) const = 0;
	
	virtual inline csp_simd_pri<T>& operator-(const csp_simd_pri<T>& __b) const = 0;
	virtual inline csp_simd_pri<T>& operator+(const csp_simd_pri<T>& __b) const = 0;
	
	virtual inline csp_simd_pri<T>& operator=(const csp_simd_pri<T>& __b) const = 0;
	virtual inline csp_simd_pri<T>& operator=(const T& __b) const = 0;
	
	virtual inline csp_simd_pri<T>& operator&=(const csp_simd_pri<T>& __b) const = 0;
	virtual inline csp_simd_pri<T>& operator&=(const T __b) const = 0;
	virtual inline csp_simd_pri<T>& operator|=(const csp_simd_pri<T>& __b) const = 0;
	virtual inline csp_simd_pri<T>& operator|=(const T __b) const = 0;
	virtual inline csp_simd_pri<T>& operator^=(const csp_simd_pri<T>& __b) const = 0;
	virtual inline csp_simd_pri<T>& operator^=(const T __b) const = 0;
	
	// SELF = NOT(__a) & self
    virtual inline csp_simd_pri<T>& op_andnot(const csp_simd_prim<T>& __a) const = 0;
	// RET = NOT(__a) & self
    virtual inline csp_simd_pri<T> op_andnot(const T __a) const = 0;
	// RET = NOT(__a) & __b	
    virtual inline T op_andnot(const T __a, const T __b) const = 0;
	
	virtual inline csp_simd_pri<T>& operator+=(const csp_simd_pri<T>& __b) const = 0;
	virtual inline csp_simd_pri<T>& operator+=(const T& __b) const = 0;
	virtual inline csp_simd_pri<T>& operator-=(const csp_simd_pri<T>& __b) const = 0;
	virtual inline csp_simd_pri<T>& operator-=(const T& __b) const = 0;


	virtual inline csp_simd_pri<T>& operator<<=(const size_t __b) const = 0;
	virtual inline csp_simd_pri<T>& operator>>=(const size_t __b) const = 0;
	
	virtual inline csp_simd_pri<T>& op_and(csp_simd_pri<T>& __a) const = 0;
	virtual inline csp_simd_pri<T>& op_and(T __a) const = 0;
	virtual inline T op_and(T __a, T __b) const = 0;
	
	virtual inline csp_simd_pri<T>& op_or(csp_simd_pri<T>& __a) const = 0;
	virtual inline csp_simd_pri<T>& op_or(T __a) const = 0;
	virtual inline T op_or(T __a, T __b) const = 0;
	
	virtual inline csp_simd_pri<T>& op_xor(csp_simd_pri<T>& __a) const = 0;
	virtual inline csp_simd_pri<T>& op_xor(T __a) const = 0;
	virtual inline T op_xor(T __a, T __b) const = 0;
	
	virtual inline csp_simd_pri<T>& op_not(csp_simd_pri<T>& __a) const = 0;
	virtual inline csp_simd_pri<T>& op_not(T __a) const = 0;
	virtual inline T op_not(T __a) const = 0;

	inline csp_simd_pri<T>& op_lshift16(const size_t __shift) const = 0;
	inline csp_simd_pri<T>& op_lshift32(const size_t __shift) const = 0;
	inline csp_simd_pri<T>& op_lshift64(const size_t __shift) const = 0;
	inline csp_simd_pri<T>& op_byte_lshift(const size_t __shift) const = 0;

	inline csp_simd_pri<T>& op_rshift16(const size_t __shift) const = 0;
	inline csp_simd_pri<T>& op_rshift32(const size_t __shift) const = 0;
	inline csp_simd_pri<T>& op_rshift64(const size_t __shift) const = 0;
	inline csp_simd_pri<T>& op_byte_lshift(const size_t __shift) const = 0;

};
