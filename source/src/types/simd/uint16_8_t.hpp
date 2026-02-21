#pragma once

#include "../simd/simd_pri.h"

// Belows are using SIMD Everywhere.
// See https://github.com/simd-everywhere/simde .
#include "../simd/primitives_128.hpp"

class simd_uint16_8 : public csp_simd_pri
{
public:
	
	uint16_8_t _d;
	simd_uint16_8(uint16_t n = 0)
	{
		fill(n);
	}
	simd_uint16_8(uint16_8_t n)
	{
		_d.v = n.v;
	}
	simd_uint16_8(const simd_uint16_8& obj)
	{
		_d.v = obj._d.v;
	}

	~simd_uint16_8() {}
	
	inline const bool is_aligned(void *p)
	{
		const uintptr_t pd = (uintptr_t)p;
		const uintptr_t mask = sizeof(uint16_8_t) - 1;  // ToDo: for not 2^n . 20260218 K.O
		return ((pd & mask) == 0) ? true : false;
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
	inline void load(void *p)
	{
		if(is_aligned(p)) {
			align_load(p);
		} else {
			unalign_load(p);
		}
	}

	template <typename Y>
		size_t load_left(Y *p, const size_t num)
	{
		__UNLIKELY_IF((sizeof(Y) > sizeof(uint16_8_t)) || (p == nullptr)) {
			return 0;
		}
		size_t off = 0;
		size_t i = 0;
		Y* pp = (Y*)p;
		uint8_t* q = (uint8_t*)(&_d);
		Y* qq = (Y*)(&(q[off]));
		for(; (off < sizeof(uint16_8_t)) && (i < num); off += sizeof(Y), i++) {
			Y _t = pp[i];
			qq[i] = _t;
		}
		return i;
	}
	
	template <typename Y>
		size_t load_right(Y *p, const size_t num)
	{
		__UNLIKELY_IF((sizeof(Y) > sizeof(uint16_8_t)) || (p == nullptr)) {
			return 0;
		}
		size_t __num = num;
		__UNLIKELY_IF((sizeof(uint16_8_t) / sizeof(Y)) > __num) {
			__num = sizeof(uint16_8_t) / sizeof(Y);
		}
		size_t off = (sizeof(uint16_8_t) / sizeof(Y) - __num) * sizeof(Y);
		size_t i = 0;
		Y* pp = (Y*)p;
		uint8_t* q = (uint8_t*)(&_d);
		Y* qq = (Y*)(&(q[off]));
		for(; ((off + sizeof(Y)) <= sizeof(uint16_8_t)) && (i < __num); off += sizeof(Y), i++) {
			Y _t = pp[i];
			qq[i] = _t;
		}
		return i;
	}
	
	inline void align_store(void *p)
	{
		simde_mm_store_si128((simde__m128i*)p, _d.v);
		return; 
	}
	inline void unalign_store(void *p)
	{
		simde_mm_storeu_si128(p, _d.v);
		return; 
	}
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
		__UNLIKELY_IF((sizeof(Y) > sizeof(uint16_8_t)) || (p == nullptr)) {
			return 0;
		}
		size_t off = 0;
		size_t i = 0;
		Y* pp = p;
		uint8_t* q = (uint8_t*)(&_d);
		Y* qq = (Y*)(&(q[off]));
		for(; (off < sizeof(uint16_8_t)) && (i < num); off += sizeof(Y), i++) {
			Y _t = qq[i];
			pp[i] = _t;
		}
		return i;
	}
	template <typename Y>
		inline size_t store_right(Y *p, const size_t num)
	{
		__UNLIKELY_IF((sizeof(Y) > sizeof(uint16_8_t)) || (p == nullptr)) {
			return 0;
		}
		size_t __num = num;
		__UNLIKELY_IF((sizeof(uint16_8_t) / sizeof(Y)) > __num) {
			__num = sizeof(uint16_8_t) / sizeof(Y);
		}
		size_t off = (sizeof(uint16_8_t) / sizeof(Y) - __num) * sizeof(Y);
		size_t i = 0;
		Y* pp = p;
		uint8_t* q = (uint8_t*)(&_d);
		Y* qq = (Y*)(&(q[off]));
		for(; ((off + sizeof(Y)) <= sizeof(uint16_8_t)) && (i < __num); off += sizeof(Y), i++) {
			Y _t = qq[i];
			pp[i] = _t;
		}
		return i;
	}
	
	inline void clear()
	{
		_d.v = simde_mm_setzero_ps();
	}
	inline void fill(uint8_t val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 16; i++) {
			_d.u8[i] = val;
		}
	}
	inline void fill(int8_t val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 16; i++) {
			_d.s8[i] = val;
		}
	}
	inline void fill(uint16_t val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			_d.u16[i] = val;
		}
	}
	inline void fill(int16_t val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			_d.s16[i] = val;
		}
	}
	inline void fill(uint32_t val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 4; i++) {
			_d.u32[i] = val;
		}
	}
	inline void fill(int32_t val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 4; i++) {
			_d.s32[i] = val;
		}
	}
	inline void fill(uint64_t val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 2; i++) {
			_d.u32[i] = val;
		}
	}
	inline void fill(int64_t val)
	{
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 2; i++) {
			_d.s32[i] = val;
		}
	}

	inline void set(size_t pos, uint8_t val)
	{
		__UNLIKELY_IF(pos >= 16) {
			return;
		}
		_d.u8[pos] = val;
	}
	inline void set(size_t pos, int8_t val)
	{
		__UNLIKELY_IF(pos >= 16) {
			return;
		}
		_d.s8[pos] = val;
	}
	inline void set(size_t pos, uint16_t val)
	{
		__UNLIKELY_IF(pos >= 8) {
			return;
		}
		_d.u16[pos] = val;
	}
	inline void set(size_t pos, int16_t val)
	{
		__UNLIKELY_IF(pos >= 16) {
			return;
		}
		_d.s16[pos] = val;
	}
	inline void set(size_t pos, uint32_t val)
	{
		__UNLIKELY_IF(pos >= 4) {
			return;
		}
		_d.u32[pos] = val;
	}
	inline void set(size_t pos, int32_t val)
	{
		__UNLIKELY_IF(pos >= 4) {
			return;
		}
		_d.s32[pos] = val;
	}
	inline void set(size_t pos, uint64_t val)
	{
		__UNLIKELY_IF(pos >= 2) {
			return;
		}
		_d.u64[pos] = val;
	}
	inline void set(size_t pos, int64_t val)
	{
		__UNLIKELY_IF(pos >= 2) {
			return;
		}
		_d.s64[pos] = val;
	}

	template <typename T>
		T at(size_t pos)
	{
		if(sizeof(T) > sizeof(_d)) {
			return (T)0;
		}
		__UNLIKELY_IF(pos >= (sizeof(T) / sizeof(_d))) {
			return (T)0;
		}
		uint8_t *p = (uint8_t*)(&_d);
		size_t rpos = sizeof(T) * pos;
		p = &(p[rpos]);
		T* q = (T*)p;
		return *q;
	}
	
	virtual inline uint16_8_t data()
	{
		return _d;
	}
	virtual inline uint16_8_t* dptr()
	{
		return &(_d);
	}

	constexpr uint16_8_t bswap(uint16_8_t data, const size_t __size = 4)
	{
		__DECL_ALIGNED(16) uint16_8_t __r = data;
		switch(__size) {
		case 1:
			return __r;
		case 2:
			__r.v = simd_128bit::op_bswap16(data.v);
			break;
		case 4:
			__r.v = simd_128bit::op_bswap32(data.v);
			break;
		case 8:
			__r.v = simd_128bit::op_bswap64(data.v);
			break;
		default: /* ERROR */
			__r.v = simd_128bit::op_clear();
			break;
		}
		return __r;
	}
	inline void bswap_self(const size_t __size = 4)
	{
		_d = bswap(_d, __size);
	}
	
	inline simd_uint16_8& operator+()
	{
		return *this;
	}
	inline simd_uint16_8& operator-()
	{
		__DECL_ALIGNED(16) uint16_8_t _r;
		_r.v = simde_mm_setzero_ps();
		_d.v = simd_128bit::op_sub_s16(_r.v, _d.v);
		return *this;
	}
	inline simd_uint16_8& operator=(const simd_uint16_8& __b)
	{
		//load(&(__b._d.v));
		_d.v = __b._d.v;
		return *this;
	}
	inline simd_uint16_8& operator=(const uint16_8_t& __b)
	{
		_d.v = __b.v;
		return *this;
	}
	inline simd_uint16_8& operator=(const simde__m128& __b)
	{
		_d.v = __b;
		return *this;
	}
	inline simd_uint16_8& operator&=(const simd_uint16_8& __b)
	{
		_d.v = simd_128bit::op_and(_d.v, __b._d.v);
		return *this;
	}
	inline simd_uint16_8& operator&=(const uint16_8_t& __b)
	{
		_d.v = simd_128bit::op_and(_d.v, __b.v);
		return *this;
	}
	inline simd_uint16_8& operator&=(const simde__m128& __b)
	{
		_d.v = simd_128bit::op_and(_d.v, __b);
		return *this;
	}
	
	inline simd_uint16_8& operator|=(const simd_uint16_8& __b)
	{
		_d.v = simd_128bit::op_or(_d.v, __b._d.v);
		return *this;
	}
	inline simd_uint16_8& operator|=(const uint16_8_t __b)
	{
		_d.v = simd_128bit::op_or(_d.v, __b.v);
		return *this;
	}
	inline simd_uint16_8& operator|=(const simde__m128 __b)
	{
		_d.v = simd_128bit::op_or(_d.v, __b);
		return *this;
	}
	
	inline simd_uint16_8& operator^=(const simd_uint16_8& __b)
	{
		_d.v = simd_128bit::op_xor(_d.v, __b._d.v);
		return *this;
	}
	inline simd_uint16_8& operator^=(const uint16_8_t __b)
	{
		_d.v = simd_128bit::op_xor(_d.v, __b.v);
		return *this;
	}
	inline simd_uint16_8& operator^=(const simde__m128 __b)
	{
		_d.v = simd_128bit::op_xor(_d.v, __b);
		return *this;
	}
	
	inline simd_uint16_8& operator+=(const simd_uint16_8& __b)
	{
		_d.v = simd_128bit::op_add_s16_sat(_d.v, __b._d.v);
		return *this;
	}
	inline simd_uint16_8& operator+=(const uint16_8_t __b)
	{
		_d.v = simd_128bit::op_add_s16_sat(_d.v, __b.v);
		return *this;
	}
	inline simd_uint16_8& operator+=(const simde__m128 __b)
	{
		_d.v = simd_128bit::op_add_s16_sat(_d.v, __b);
		return *this;
	}
	
	inline simd_uint16_8& operator-=(const simd_uint16_8& __b)
	{
		_d.v = simd_128bit::op_sub_s16_sat(_d.v, __b._d.v);
		return *this;
	}
	inline simd_uint16_8& operator-=(const uint16_8_t __b)
	{
		_d.v = simd_128bit::op_sub_s16_sat(_d.v, __b.v);
		return *this;
	}
	inline simd_uint16_8& operator-=(const simde__m128 __b)
	{
		_d.v = simd_128bit::op_sub_s16_sat(_d.v, __b);
		return *this;
	}

	
	inline simd_uint16_8& operator<<=(const size_t __shift)
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = simd_128bit::op_lshift16(_d.v, __shift);
		_d.v = _r;
		return *this;
	}
	inline simd_uint16_8& operator>>=(const size_t __shift)
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = simd_128bit::op_rshift16(_d.v, __shift);
		_d.v = _r;
		return *this;
	}

};


inline simd_uint16_8 operator+(const simd_uint16_8& __a, const simd_uint16_8& __b)
{
	__DECL_ALIGNED(16) simd_uint16_8 __d((simd_uint16_8)__a);
	__d += __b;
	return __d;
}


inline simd_uint16_8 operator-(const simd_uint16_8& __a, const simd_uint16_8& __b)
{
	__DECL_ALIGNED(16) simd_uint16_8 __d((simd_uint16_8)__a);
	__d -= __b;
	return __d;
}


inline simd_uint16_8 operator&(const simd_uint16_8& __a, const simd_uint16_8& __b)
{
	__DECL_ALIGNED(16) simd_uint16_8 __d((simd_uint16_8)__a);
	__d &= __b;
	return __d;
}

inline simd_uint16_8 operator|(const simd_uint16_8& __a, const simd_uint16_8& __b)
{
	__DECL_ALIGNED(16) simd_uint16_8 __d((simd_uint16_8)__a);
	__d |= __b;
	return __d;
}


inline simd_uint16_8 operator^(const simd_uint16_8& __a, const simd_uint16_8& __b)
{
	__DECL_ALIGNED(16) simd_uint16_8 __d((simd_uint16_8)__a);
	__d ^= __b;
	return __d;
}


inline simd_uint16_8 operator<<(const simd_uint16_8& __a, const size_t& __shift)
{
	__DECL_ALIGNED(16) simd_uint16_8 __d((simd_uint16_8)__a);
	__d <<= __shift;
	return __d;
}

inline simd_uint16_8 operator>>(const simd_uint16_8& __a, const size_t& __shift)
{
	__DECL_ALIGNED(16) simd_uint16_8 __d((simd_uint16_8)__a);
	__d >>= __shift;
	return __d;
}
