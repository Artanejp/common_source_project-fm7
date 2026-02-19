#pragma once

#include "../simd/simd_pri.h"
// Belows are using SIMD Everywhere.
// See https://github.com/simd-everywhere/simde .
#include "../simd/primitives_128.hpp"

template <uint16_8_t>
	class csp_simd_pri {
	csp_simd_pri(uint16_t n = 0)
	{
		fill(n);
	}
	csp_simd_pri(uint16_8_t n)
	{
		_d.v = n.v;
	}
	csp_simd_pri(csp_simd_pri<uint16_8_t> obj)
	{
		_d.v = obj._d.v;
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
		_d.u16[pos] = val;
	}
	inline void set(size_t pos, uint16_t val)
	{
		__UNLIKELY_IF(pos >= 8) {
			return;
		}
		_d.u16[pos] = val;
	}
	inline void set(size_t pos, uint32_t val)
	{
		__UNLIKELY_IF(pos >= 4) {
			return;
		}
		_d.u32[pos] = val;
	}
	inline uint8_t at(size_t pos)
	{
		__UNLIKELY_IF(pos >= 16) {
			return 0;
		}
		return _d.u8[pos];
	}
	inline int8_t at(size_t pos)
	{
		__UNLIKELY_IF(pos >= 16) {
			return 0;
		}
		return _d.s8[pos];
	}
	inline uint16_t at(size_t pos)
	{
		__UNLIKELY_IF(pos >= 8) {
			return 0;
		}
		return _d.u16[pos];
	}
	inline int16_t at(size_t pos)
	{
		__UNLIKELY_IF(pos >= 8) {
			return 0;
		}
		return _d.s16[pos];
	}
	inline uint32_t at(size_t pos)
	{
		__UNLIKELY_IF(pos >= 4) {
			return 0;
		}
		return _d.u32[pos];
	}
	inline int32_t at(size_t pos)
	{
		__UNLIKELY_IF(pos >= 4) {
			return 0;
		}
		return _d.s32[pos];
	}

	inline uint16_8_t bswap(uint16_8_t data, constexpr size = 4)
	{
		__DECL_ALIGNED(16) uint16_8_t __r = data;
		switch(size) {
		case 1:
			return __r;
		case 2:
			__r.v = op_bswap16(data);
			break;
		case 4:
			__r.v = op_bswap32(data);
			break;
		case 8:
			__r.v = op_bswap64(data);
			break;
		default: /* ERROR */
			__r = op_clear();
			break;
		}
		return __r
	}
	inline void bswap_self(constexpr size = 4)
	{
		_d = bswap(_d);
	}
	
	inline csp_simd_pri<uint16_8_t>& operator+() const
	{
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator-() const
	{
		__DECL_ALIGNED(16) uint16_8_t _r;
		_r.v = simde_mm_setzero_ps();
		_d.v = op_sub_s16(_r.v, _d.v);
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
    inline csp_simd_prim<uint16_8_t>& op_andnot(const csp_simd_prim<uint16_8_t>& __a) const
	{
		__DECL_ALIGNED(16) csp_simd_prim<uint16_8_t> _r(__a);
		_d.v = op_andnot(_r._d.v, _d.v);
		return *this;
	}
    inline csp_simd_prim<uint16_8_t>& op_andnot(const uint16_8_t __a) const
	{
		_d.v = op_andnot(__a.v, _d.v);
		return *this;
	}
    inline uint16_8_t op_andnot(uint16_8_t __a, uint16_8_t __b) const
	{
		__DECL_ALIGNED(16) uint16_8_t __r;
		__r = op_andnot(__a.v, __b.v);
		return __r;
	}
	
	inline csp_simd_prim<uint16_8_t>& operator+=(csp_simd_prim<uint16_8_t>& __b) const
	{
		_d.v = op_add_s16_sat(_d.v, __b._d.v);
		return *this;
	}
	inline csp_simd_prim<uint16_8_t>& operator+=(uint16_8_t __b) const
	{
		_d.v = op_add_s16_sat(_d.v, __b.v);
		return *this;
	}
	inline csp_simd_prim<uint16_8_t>& operator+=(simde__m128 __b) const
	{
		_d.v = op_add_s16_sat(_d.v, __b);
		return *this;
	}
	
	inline csp_simd_prim<uint16_8_t>& operator-=(csp_simd_prim<uint16_8_t>& __b) const
	{
		_d.v = op_sub_s16_sat(_d.v, __b._d.v);
		return *this;
	}
	inline csp_simd_prim<uint16_8_t>& operator-=(uint16_8_t __b) const
	{
		_d.v = op_sub_s16_sat(_d.v, __b.v);
		return *this;
	}
	inline csp_simd_prim<uint16_8_t>& operator-=(simde__m128 __b) const
	{
		_d.v = op_sub_s16_sat(_d.v, __b);
		return *this;
	}

	
	inline csp_simd_pri<uint16_8_t>& operator<<=(const size_t __shift) const
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = op_lshift16(_d.v, __shift);
		_d.v = _r;
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& operator>>=(const size_t __shift) const
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = op_rshift16(_d.v, __shift);
		_d.v = _r;
		return *this;
	}

    inline csp_simd_prim<uint16_8_t>& op_and(const csp_simd_prim<uint16_8_t>& __a) const
	{
		_d.v = op_and(__a._d.v, _d.v);
		return *this;
	}
    inline csp_simd_prim<uint16_8_t>& op_and(const uint16_8_t __a) const
	{
		_d.v = op_and(__a.v, _d.v);
		return *this;
	}
    inline uint16_8_t op_and(uint16_8_t __a, uint16_8_t __b) const
	{
		__DECL_ALIGNED(16) uint16_8_t __r;
		__r.v = op_and(__a.v, __b.v);
		return __r;
	}
	
    inline csp_simd_prim<uint16_8_t>& op_or(const csp_simd_prim<uint16_8_t>& __a) const
	{
		_d.v = op_or(__a._d.v, _d.v);
		return *this;
	}
    inline csp_simd_prim<uint16_8_t>& op_or(const uint16_8_t __a) const
	{
		_d.v = op_or(__a.v, _d.v);
		return *this;
	}
    inline uint16_8_t op_or(uint16_8_t __a, uint16_8_t __b) const
	{
		__DECL_ALIGNED(16) uint16_8_t __r;
		__r.v = op_or(__a.v, __b.v);
		return __r;
	}
    inline csp_simd_prim<uint16_8_t>& op_xor(const csp_simd_prim<uint16_8_t>& __a) const
	{
		_d.v = op_xor(__a._d.v, _d.v);
		return *this;
	}
    inline csp_simd_prim<uint16_8_t>& op_xor(const uint16_8_t __a) const
	{
		_d.v = op_xor(__a.v, _d.v);
		return *this;
	}
    inline uint16_8_t op_xor(uint16_8_t __a, uint16_8_t __b) const
	{
		__DECL_ALIGNED(16) uint16_8_t __r;
		__r.v = op_xor(__a.v, __b.v);
		return __r;
	}
	inline csp_simd_pri<uint16_8_t>& op_not(csp_simd_pri<uint16_8_t>& __a) const
	{
		_d.v = op_not(__a._d.v);
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& op_not(csp_simd_priuint16_8_t __a) const
	{
		_d.v = op_not(__a.v);
		return *this;
	}
	inline uint16_8_t op_not(csp_simd_priuint16_8_t __a) const
	{
		_d.v = op_not(__a.v);
		return _d;
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
	inline csp_simd_pri<uint16_8_t>& op_lshift64(const size_t __shift) const
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = op_lshift64(_d.v, __shift);
		_d.v = _r;
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& op_rshift64(const size_t __shift) const
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = op_rshift16(_d.v, __shift);
		_d.v = _r;
		return *this;
	}
	inline csp_simd_pri<uint16_8_t>& op_byte_lshift(const size_t __shift) const
	{
		__DECL_ALIGNED(16) simde__m128 _r;
		_r = op_byte_lshift16(_d.v, __shift);
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
