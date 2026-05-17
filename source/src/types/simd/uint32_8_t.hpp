#pragma once

#include "../simd/simd_pri.h"
// Belows are using SIMD Everywhere.
// See https://github.com/simd-everywhere/simde .
#include "../simd/primitives_256.hpp"

class simd_uint32_8 : public csp_simd_pri
{
private:
	
public:
	__DECL_ALIGNED(32) uint32_8_t _d;
	
	simd_uint32_8(uint32_t n = 0)
	{
		fill(n);
	}
	simd_uint32_8(uint32_8_t n)
	{
		_d.v = n.v;
	}
	simd_uint32_8(const simd_uint32_8& obj)
	{
		_d.v = obj._d.v;
	}

	~simd_uint32_8() { }
	
	inline const bool is_aligned(void *p)
	{
		return simd_256bit::is_aligned(p);
	}
	
	inline void align_load(void *p)
	{
		_d.v = simd_256bit::load_aligned((simde__m256 *)p);
		return; 
	}
	inline void unalign_load(void *p)
	{
		_d.v = simd_256bit::load_unaligned(p);
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

	template <typename __T>
		inline ssize_t load_left_unsafe(__T* src, const size_t num)
	{
		return __load_left_unsafe(src, &(_d), num);
	}
	
	template <typename __T>
		inline ssize_t load_left_safe(__T* src, const size_t num)
	{
		return __load_left_safe(src, &(_d), num);
	}
	template <typename __T>
		inline ssize_t load_left(__T* src, const size_t num)
	{
		return __load_left_unsafe(src, &(_d), num);
	}
		
	template <typename __T>
		inline ssize_t load_right_unsafe(__T* src, const size_t num)
	{
		return __load_right_unsafe(src, &(_d), num);
	}
	template <typename __T>
		inline ssize_t load_right_safe(__T* src, const size_t num)
	{
		return __load_right_safe(src, &(_d), num);
	}
	template <typename __T>
		inline ssize_t load_right(__T* src, const size_t num)
	{
		return __load_right_unsafe(src, &(_d), num);
	}
	
	inline void align_store(void *p)
	{
		simd_256bit::store_aligned((simde__m256*)p, _d.v);
		return; 
	}
	inline void unalign_store(void *p)
	{
		simd_256bit::store_unaligned(p, _d.v);
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

	template <typename __T>
		inline ssize_t store_left_unsafe(__T* dst, const size_t num)
	{
		return __store_left_unsafe(dst, &(_d), num);
	}
	
	template <typename __T>
		inline ssize_t store_left_safe(__T* dst, const size_t num)
	{
		return __store_left_safe(dst, &(_d), num);
	}
	template <typename __T>
		inline ssize_t store_left(__T* dst, const size_t num)
	{
		return __store_left_unsafe(dst, &(_d), num);
	}
		
	template <typename __T>
		inline ssize_t store_right_unsafe(__T* dst, const size_t num)
	{
		return __store_right_unsafe(dst, &(_d), num);
	}
	template <typename __T>
		inline ssize_t store_right_safe(__T* dst, const size_t num)
	{
		return __store_right_safe(dst, &(_d), num);
	}
	template <typename __T>
		inline ssize_t store_right(__T* dst, const size_t num)
	{
		return __store_right_unsafe(dst, &(_d), num);
	}
	
	inline void clear()
	{
		_d.v = simd_256bit::op_clear();
	}
	inline void setall()
	{
		_d.v = simd_256bit::op_setall();
	}
	inline void fill(uint8_t val)
	{
		_d.v = simd_256bit::op_set8((const uint8_t)val);
	}
	inline void fill(int8_t val)
	{
		_d.v = simd_256bit::op_set8((const uint8_t)val);
	}
	inline void fill(uint16_t val)
	{
		_d.v = simd_256bit::op_set16((const uint16_t)val);
	}
	inline void fill(int16_t val)
	{
		_d.v = simd_256bit::op_set16((const uint16_t)val);
	}
	inline void fill(uint32_t val)
	{
		_d.v = simd_256bit::op_set32((const uint32_t)val);
	}
	inline void fill(int32_t val)
	{
		_d.v = simd_256bit::op_set32((const uint32_t)val);
	}
	inline void fill(uint64_t val)
	{
		_d.v = simd_256bit::op_set64((const uint32_t)val);
	}
	inline void fill(int64_t val)
	{
		_d.v = simd_256bit::op_set64((const uint32_t)val);
	}
	
	template <typename __T>
		inline __T at_unsafe(size_t pos)
	{
		return __at_unsafe<__T, uint32_8_t>(pos, &(_d));
	}
	template <typename __T>
		inline __T at_safe(size_t pos)
	{
		return __at_safe<__T, uint32_8_t>(pos, &(_d));
	}
	template <typename __T>
		inline __T at(size_t pos)
	{
		return __at_unsafe<__T, uint32_8_t>(pos, &(_d));
	}
	
	template <typename __T>
		inline void set_unsafe(size_t pos, __T data)
	{
		__set_unsafe(pos, (uint32_8_t*)(&(_d)), data);
	}
	template <typename __T>
		inline void set_safe(size_t pos, __T data)
	{
		__set_safe(pos, (uint32_8_t*)(&(_d)), data);
	}
	template <typename __T>
		inline void set(size_t pos, __T data)
	{
		__set_unsafe(pos, (uint32_8_t*)(&(_d)), data);
	}
	
	virtual inline uint32_8_t data()
	{
		return _d;
	}
	virtual inline uint32_8_t* dptr()
	{
		return &(_d);
	}

	constexpr uint32_8_t bswap(uint32_8_t data, const size_t __size = 4)
	{
		__DECL_ALIGNED(32) uint32_8_t __r = data;
		switch(__size) {
		case 1:
			return __r;
		case 2:
			__r.v = simd_256bit::op_bswap16(data.v);
			break;
		case 4:
			__r.v = simd_256bit::op_bswap32(data.v);
			break;
		case 8:
			__r.v = simd_256bit::op_bswap64(data.v);
			break;
		default: /* ERROR */
			__r.v = simd_256bit::op_clear();
			break;
		}
		return __r;
	}
	inline void bswap_self(const size_t __size = 4)
	{
		_d = bswap(_d, __size);
	}
	template <typename _Styp, typename _Dtyp>
		inline simd_uint32_8& from_uint16_8(const simd_uint16_8 src)
	{
		if(std::is_signed<_Styp>::value) {
			switch(sizeof(_Styp)) {
			case 1:
				switch(sizeof(_Dtyp)) {
				case 1:
					__DECL_VECTORIZED_LOOP
					for(size_t ii = 0; ii < 16; ii++) {
						_d.s8[ii] = src._d.s8[ii];
					}
					break;
				case 2:
					_d.v = simd_256bit::op_convert_8bit_to_16bit_signed(src._d.v);
					break;
				case 4:
					_d.v = simd_256bit::op_convert_8bit_to_32bit_signed(src._d.v);
					break;
				case 8:
					_d.v = simd_256bit::op_convert_8bit_to_64bit_signed(src._d.v);
					break;
				default: /* NOP */
					break; 
				}
				break;
			case 2: /* 16bit */
				switch(sizeof(_Dtyp)) {
				case 1:
					__DECL_VECTORIZED_LOOP
					for(size_t ii = 0; ii < 8; ii++) {
						_d.s8[ii] = (int8_t)(src._d.s16[ii]);
					}
					break;
				case 2:
					__DECL_VECTORIZED_LOOP
					for(size_t ii = 0; ii < 8; ii++) {
						_d.s16[ii] = src._d.s16[ii];
					}
					break;
				case 4:
					_d.v = simd_256bit::op_convert_16bit_to_32bit_signed(src._d.v);
					break;
				case 8:
					_d.v = simd_256bit::op_convert_16bit_to_64bit_signed(src._d.v);
					break;
				default: /* NOP */
					break; 
				}
				break;
			case 4: /* 32bit */
				switch(sizeof(_Dtyp)) {
				case 1:
					__DECL_VECTORIZED_LOOP
					for(size_t ii = 0; ii < 8; ii++) {
						_d.s8[ii] = (int8_t)(src._d.s32[ii]);
					}
					break;
				case 2:
					__DECL_VECTORIZED_LOOP
					for(size_t ii = 0; ii < 4; ii++) {
						_d.s16[ii] = (int16_t)(src._d.s32[ii]);
					}
					break;
				case 4:
					__DECL_VECTORIZED_LOOP
					for(size_t ii = 0; ii < 4; ii++) {
						_d.s32[ii] = src._d.s32[ii];
					}
					break;
				case 8:
					_d.v = simd_256bit::op_convert_32bit_to_64bit_signed(src._d.v);
					break;
				default: /* NOP */
					break; 
				}
				break;
			default:
				break;
			}
		} else { /* Unsigned */
			switch(sizeof(_Styp)) {
			case 1:
				switch(sizeof(_Dtyp)) {
				case 1:
					__DECL_VECTORIZED_LOOP
					for(size_t ii = 0; ii < 16; ii++) {
						_d.u8[ii] = src._d.u8[ii];
					}
					break;
				case 2:
					_d.v = simd_256bit::op_convert_8bit_to_16bit_unsigned(src._d.v);
					break;
				case 4:
					_d.v = simd_256bit::op_convert_8bit_to_32bit_unsigned(src._d.v);
					break;
				case 8:
					_d.v = simd_256bit::op_convert_8bit_to_64bit_unsigned(src._d.v);
					break;
				default: /* NOP */
					break; 
				}
				break;
			case 2: /* 16bit */
				switch(sizeof(_Dtyp)) {
				case 1:
					__DECL_VECTORIZED_LOOP
					for(size_t ii = 0; ii < 8; ii++) {
						_d.u8[ii] = src._d.u16[ii];
					}
					break;
				case 2:
					__DECL_VECTORIZED_LOOP
					for(size_t ii = 0; ii < 8; ii++) {
						_d.u16[ii] = src._d.u16[ii];
					}
					break;
				case 4:
					_d.v = simd_256bit::op_convert_16bit_to_32bit_unsigned(src._d.v);
					break;
				case 8:
					_d.v = simd_256bit::op_convert_16bit_to_64bit_unsigned(src._d.v);
					break;
				default: /* NOP */
					break; 
				}
				break;
			case 4: /* 32bit */
				switch(sizeof(_Dtyp)) {
				case 1:
					__DECL_VECTORIZED_LOOP
					for(size_t ii = 0; ii < 4; ii++) {
						_d.u8[ii] = (uint8_t)(src._d.u32[ii]);
					}
					break;
				case 2:
					__DECL_VECTORIZED_LOOP
					for(size_t ii = 0; ii < 4; ii++) {
						_d.u16[ii] = (uint16_t)(src._d.u32[ii]);
					}
					break;
				case 4:
					__DECL_VECTORIZED_LOOP
					for(size_t ii = 0; ii < 4; ii++) {
						_d.u32[ii] = src._d.u32[ii];
					}
					break;
				case 8:
					_d.v = simd_256bit::op_convert_32bit_to_64bit_unsigned(src._d.v);
					break;
				default: /* NOP */
					break; 
				}
				break;
			default:
				break;
			}
		}
		return *this;
	}
	inline simd_uint32_8& operator+()
	{
		return *this;
	}
	inline simd_uint32_8& operator-()
	{
		__DECL_ALIGNED(32) uint32_8_t _r;
		_r.v = simde_mm256_setzero_ps();
		_d.v = simd_256bit::op_sub_s16(_r.v, _d.v);
		return *this;
	}
	inline simd_uint32_8& operator=(const simd_uint32_8& __b)
	{
		//load(&(__b._d.v));
		_d.v = __b._d.v;
		return *this;
	}
	inline simd_uint32_8& operator=(const uint32_8_t& __b)
	{
		_d.v = __b.v;
		return *this;
	}
	inline simd_uint32_8& operator=(const simde__m256& __b)
	{
		_d.v = __b;
		return *this;
	}
	inline simd_uint32_8& operator&=(const simd_uint32_8& __b)
	{
		_d.v = simd_256bit::op_and(_d.v, __b._d.v);
		return *this;
	}
	inline simd_uint32_8& operator&=(const uint32_8_t& __b)
	{
		_d.v = simd_256bit::op_and(_d.v, __b.v);
		return *this;
	}
	inline simd_uint32_8& operator&=(const simde__m256& __b)
	{
		_d.v = simd_256bit::op_and(_d.v, __b);
		return *this;
	}
	
	inline simd_uint32_8& operator|=(const simd_uint32_8& __b)
	{
		_d.v = simd_256bit::op_or(_d.v, __b._d.v);
		return *this;
	}
	inline simd_uint32_8& operator|=(const uint32_8_t __b)
	{
		_d.v = simd_256bit::op_or(_d.v, __b.v);
		return *this;
	}
	inline simd_uint32_8& operator|=(const simde__m256 __b)
	{
		_d.v = simd_256bit::op_or(_d.v, __b);
		return *this;
	}
	
	inline simd_uint32_8& operator^=(const simd_uint32_8& __b)
	{
		_d.v = simd_256bit::op_xor(_d.v, __b._d.v);
		return *this;
	}
	inline simd_uint32_8& operator^=(const uint32_8_t __b)
	{
		_d.v = simd_256bit::op_xor(_d.v, __b.v);
		return *this;
	}
	inline simd_uint32_8& operator^=(const simde__m256 __b)
	{
		_d.v = simd_256bit::op_xor(_d.v, __b);
		return *this;
	}
	
	inline simd_uint32_8& operator+=(const simd_uint32_8& __b)
	{
		_d.v = simd_256bit::op_add_s16_sat(_d.v, __b._d.v);
		return *this;
	}
	inline simd_uint32_8& operator+=(const uint32_8_t __b)
	{
		_d.v = simd_256bit::op_add_s16_sat(_d.v, __b.v);
		return *this;
	}
	inline simd_uint32_8& operator+=(const simde__m256 __b)
	{
		_d.v = simd_256bit::op_add_s16_sat(_d.v, __b);
		return *this;
	}
	
	inline simd_uint32_8& operator-=(const simd_uint32_8& __b)
	{
		_d.v = simd_256bit::op_sub_s16_sat(_d.v, __b._d.v);
		return *this;
	}
	inline simd_uint32_8& operator-=(const uint32_8_t __b)
	{
		_d.v = simd_256bit::op_sub_s16_sat(_d.v, __b.v);
		return *this;
	}
	inline simd_uint32_8& operator-=(const simde__m256 __b)
	{
		_d.v = simd_256bit::op_sub_s16_sat(_d.v, __b);
		return *this;
	}
	
	template <typename _St>
		inline simd_uint32_8& op_lshift16(_St __shift)
	{
		constexpr bool __is_constant = (std::is_const<_St>::value);
		if(__is_constant) {
			_d.v = simd_256bit::op_lshift16_fix(_d.v, (const int)__shift);
		} else {
			_d.v = simd_256bit::op_lshift16(_d.v, (const size_t)__shift);
		}
		return *this;
	}
	template <typename _St>
		inline simd_uint32_8& op_rshift16(_St __shift)
	{
		constexpr bool __is_constant = (std::is_const<_St>::value);
		if(__is_constant) {
			_d.v = simd_256bit::op_rshift16_fix(_d.v, (const int)__shift);
		} else {
			_d.v = simd_256bit::op_rshift16(_d.v, (const size_t)__shift);
		}
		return *this;
	}
	template <typename _St>
		inline simd_uint32_8& op_rshift16_sign(_St __shift)
	{
		constexpr bool __is_constant = (std::is_const<_St>::value);
		if(__is_constant) {
			_d.v = simd_256bit::op_rshift16_sign_fix(_d.v, (const int)__shift);
		} else {
			_d.v = simd_256bit::op_rshift16_sign(_d.v, (const size_t)__shift);
		}
		return *this;
	}
	template <typename _St>
		inline simd_uint32_8& op_lshift32(_St __shift)
	{
		constexpr bool __is_constant = (std::is_const<_St>::value);
		if(__is_constant) {
			_d.v = simd_256bit::op_lshift32_fix(_d.v, (const int)__shift);
		} else {
			_d.v = simd_256bit::op_lshift32(_d.v, (const size_t)__shift);
		}
		return *this;
	}
	template <typename _St>
		inline simd_uint32_8& op_rshift32(_St __shift)
	{
		constexpr bool __is_constant = (std::is_const<_St>::value);
		if(__is_constant) {
			_d.v = simd_256bit::op_rshift32_fix(_d.v, (const int)__shift);
		} else {
			_d.v = simd_256bit::op_rshift32(_d.v, (const size_t)__shift);
		}
		return *this;
	}
	template <typename _St>
		inline simd_uint32_8& op_rshift32_sign(_St __shift)
	{
		constexpr bool __is_constant = (std::is_const<_St>::value);
		if(__is_constant) {
			_d.v = simd_256bit::op_rshift32_sign_fix(_d.v, (const int)__shift);
		} else {
			_d.v = simd_256bit::op_rshift32_sign(_d.v, (const size_t)__shift);
		}
		return *this;
	}
	template <typename _St>
		inline simd_uint32_8& operator<<=(_St __shift)
	{
		return op_lshift16(__shift);
	}
	template <typename _St>
		inline simd_uint32_8& operator>>=(_St __shift)
	{
		return op_rshift16(__shift);
	}

	// Special logic OPs
	inline simd_uint32_8& op_andnot(const simd_uint32_8& mask)
	{
		_d.v = simd_256bit::op_andnot(mask._d.v, _d.v);
		return *this;
	}
	inline simd_uint32_8& op_not()
	{
		_d.v = simd_256bit::op_not(_d.v);
		return *this;
	}
	// Compare ops.
	// per 8bits.
	// this == b
	inline simd_uint32_8& equals_i8(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_equals8(_d.v, b._d.v);
		return *this;
	}
	// this != b
	inline simd_uint32_8& not_equals_i8(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_not_equals8(_d.v, b._d.v);
		return *this;
	}
	// this > b
	inline simd_uint32_8& greater_i8(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_greater8(_d.v, b._d.v);
		return *this;
	}
	inline simd_uint32_8& greater_equals_i8(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_not(simd_256bit::op_lesser8(_d.v, b._d.v));
		return *this;
	}
	// this < b
	inline simd_uint32_8& lesser_i8(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_lesser8(_d.v, b._d.v);
		return *this;
	}
	// this <= b
	inline simd_uint32_8& lesser_equals_i8(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_not(simd_256bit::op_greater8(_d.v, b._d.v));
		return *this;
	}
	
	// per 16bits.
	// this == b
	inline simd_uint32_8& equals_i16(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_equals16(_d.v, b._d.v);
		return *this;
	}
	// this != b
	inline simd_uint32_8& not_equals_i16(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_not_equals16(_d.v, b._d.v);
		return *this;
	}
	// this > b
	inline simd_uint32_8& greater_i16(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_greater16(_d.v, b._d.v);
		return *this;
	}
	inline simd_uint32_8& greater_equals_i16(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_not(simd_256bit::op_lesser16(_d.v, b._d.v));
		return *this;
	}
	// this < b
	inline simd_uint32_8& lesser_i16(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_lesser16(_d.v, b._d.v);
		return *this;
	}
	// this <= b
	inline simd_uint32_8& lesser_equals_i16(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_not(simd_256bit::op_greater16(_d.v, b._d.v));
		return *this;
	}
	// per 32bits.
	inline simd_uint32_8& equals_i32(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_equals32(_d.v, b._d.v);
		return *this;
	}
	// this != b
	inline simd_uint32_8& not_equals_i32(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_not_equals32(_d.v, b._d.v);
		return *this;
	}
	// this > b
	inline simd_uint32_8& greater_i32(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_greater32(_d.v, b._d.v);
		return *this;
	}
	inline simd_uint32_8& greater_equals_i32(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_not(simd_256bit::op_lesser32(_d.v, b._d.v));
		return *this;
	}
	// this < b
	inline simd_uint32_8& lesser_i32(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_lesser32(_d.v, b._d.v);
		return *this;
	}
	// this <= b
	inline simd_uint32_8& lesser_equals_i32(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_not(simd_256bit::op_greater32(_d.v, b._d.v));
		return *this;
	}
	// per 64bits.
	inline simd_uint32_8& equals_i64(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_equals64(_d.v, b._d.v);
		return *this;
	}
	// this != b
	inline simd_uint32_8& not_equals_i64(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_not_equals64(_d.v, b._d.v);
		return *this;
	}
	// this > b
	inline simd_uint32_8& greater_i64(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_greater64(_d.v, b._d.v);
		return *this;
	}
	inline simd_uint32_8& greater_equals_i64(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_not(simd_256bit::op_lesser64(_d.v, b._d.v));
		return *this;
	}
	// this < b
	inline simd_uint32_8& lesser_i64(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_lesser64(_d.v, b._d.v);
		return *this;
	}
	// this <= b
	inline simd_uint32_8& lesser_equals_i64(const simd_uint32_8& b)
	{
		_d.v = simd_256bit::op_not(simd_256bit::op_greater64(_d.v, b._d.v));
		return *this;
	}

	// Test bits.
	inline bool test_zero(const simd_uint32_8& mask)
	{
		int _r = simd_256bit::test_zero(mask._d.v, _d.v);
		return (_r != 0);
	}
	inline bool test_c(const simd_uint32_8& mask)
	{
		int _r = simd_256bit::test_c(mask._d.v, _d.v);
		return (_r != 0);
	}
	inline bool test_nzc(const simd_uint32_8& mask)
	{
		int _r = simd_256bit::test_nzc(mask._d.v, _d.v);
		return (_r != 0);
	}
	
	template <typename _T>
		inline simd_uint32_8 lookup_from_8bitVals(void* srctbl, const uint8_t* srcdat, _T scale)
	{
		__DECL_ALIGNED(32) simd_uint32_8 sd;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			sd._d.u32[i] = srcdat[i];
		}
		return lookup_from_8bitVals<_T>(srctbl, (const simd_uint32_8)sd, scale);
	}
	template <typename _T>
		inline simd_uint32_8 lookup_from_8bitVals(void* srctbl, const uint8_t* srcdat, _T scale, const size_t num)
	{
		__DECL_ALIGNED(32) simd_uint32_8 sd;
		size_t _n = (num > 8) ? 8 : num;
		sd.clear();
		__LIKELY_IF(_n > 0) {
			for(size_t i = 0; i < _n; i++) {
				sd._d.u32[i] = srcdat[i];
			}
		}
		return lookup_from_8bitVals<_T>(srctbl, (const simd_uint32_8)sd, scale, _n);
	}
	template <typename _T>
		inline simd_uint32_8 lookup_from_8bitVals(void* srctbl, const uint8_8_t srcdat, _T scale)
	{
		__DECL_ALIGNED(32) simd_uint32_8 sd;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			sd._d.u32[i] = srcdat.u8[i];
		}
		return lookup_from_8bitVals<_T>(srctbl, (const simd_uint32_8)sd, scale);
	}
	template <typename _T>
		inline simd_uint32_8 lookup_from_8bitVals(void* srctbl, const uint8_8_t srcdat, _T scale, const size_t num)
	{
		__DECL_ALIGNED(32) simd_uint32_8 sd;
		size_t _n = (num > 8) ? 8 : num;
		sd.clear();
		__LIKELY_IF(_n > 0) {
			for(size_t i = 0; i < _n; i++) {
				sd._d.u32[i] = srcdat.u8[i];
			}
		}
		return lookup_from_8bitVals<_T>(srctbl, (const simd_uint32_8)sd, scale, _n);
	}
	template <typename _T>
		inline simd_uint32_8& lookup_from_8bitVals(void* srctbl, const simd_uint32_8 srcdat, _T scale)
	{
		uint8_t* p = (uint8_t *)srctbl;
		switch(scale) {
		case 1:
			_d.v = simde_mm256_i32gather_epi32((const int32_t*)srctbl, srcdat._d.v, 1);
			break;
		case 2:
			_d.v = simde_mm256_i32gather_epi32((const int32_t*)srctbl, srcdat._d.v, 2);
			break;
		case 4:
			_d.v = simde_mm256_i32gather_epi32((const int32_t*)srctbl, srcdat._d.v, 4);
			break;
		case 8:
			_d.v = simde_mm256_i32gather_epi32((const int32_t*)srctbl, srcdat._d.v, 8);
			break;
		default:
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				ssize_t n = (ssize_t)(srcdat._d.s32[i] * scale);
				_d.u32[i] = *((uint32_t*)(&(p[n])));
			}
			break;
		}
		return *this;
	}
	template <typename _T>
		inline simd_uint32_8& lookup_from_8bitVals(void* srctbl, const simd_uint32_8 srcdat, _T scale, const size_t num)
	{
		
		uint8_t* p = (uint8_t* )srctbl;
		size_t __num = (num >= 8) ? 8 : num;
		clear();
		__LIKELY_IF(__num > 0)  {
			for(size_t i = 0; i < __num; i++) {
				ssize_t n = (ssize_t)(srcdat._d.s32[i] * scale);
				_d.u32[i] = *((uint32_t*)(&(p[n])));
			}
		}
		return *this;
	}
	
};


inline simd_uint32_8 operator+(const simd_uint32_8& __a, const simd_uint32_8& __b)
{
	__DECL_ALIGNED(32) simd_uint32_8 __d((simd_uint32_8)__a);
	__d += __b;
	return __d;
}
inline simd_uint32_8 operator-(const simd_uint32_8& __a, const simd_uint32_8& __b)
{
	__DECL_ALIGNED(32) simd_uint32_8 __d((simd_uint32_8)__a);
	__d -= __b;
	return __d;
}

inline simd_uint32_8 operator&(const simd_uint32_8& __a, const simd_uint32_8& __b)
{
	__DECL_ALIGNED(32) simd_uint32_8 __d((simd_uint32_8)__a);
	__d &= __b;
	return __d;
}

inline simd_uint32_8 operator|(const simd_uint32_8& __a, const simd_uint32_8& __b)
{
	__DECL_ALIGNED(32) simd_uint32_8 __d((simd_uint32_8)__a);
	__d |= __b;
	return __d;
}

inline simd_uint32_8 operator^(const simd_uint32_8& __a, const simd_uint32_8& __b)
{
	__DECL_ALIGNED(32) simd_uint32_8 __d((simd_uint32_8)__a);
	__d ^= __b;
	return __d;
}

inline simd_uint32_8 op_andnot(const simd_uint32_8 mask, const simd_uint32_8 src)
{
	__DECL_ALIGNED(32) simd_uint32_8 __d((simd_uint32_8)src);
	__d._d.v = simd_256bit::op_andnot(mask._d.v, __d._d.v);
	return __d;
}

template <typename __St>
	simd_uint32_8 operator<<(const simd_uint32_8& __a, __St& __shift)
{
	__DECL_ALIGNED(32) simd_uint32_8 __d((simd_uint32_8)__a);
	__d <<= __shift;
	return __d;
}

template <typename __St>
	inline simd_uint32_8 operator>>(const simd_uint32_8& __a, __St& __shift)
{
	__DECL_ALIGNED(32) simd_uint32_8 __d((simd_uint32_8)__a);
	__d >>= __shift;
	return __d;
}

inline simd_uint32_8 cmp_equals_i8(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.equals_i8(b);
}

inline simd_uint32_8 cmp_not_equals_i8(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.not_equals_i8(b);
}

inline simd_uint32_8 cmp_greater_i8(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.greater_i8(b);
}

inline simd_uint32_8 cmp_greater_equals_i8(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.greater_equals_i8(b);
}

inline simd_uint32_8 cmp_lesser_i8(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.lesser_i8(b);
}

inline simd_uint32_8 cmp_lesser_equals_i8(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.lesser_equals_i8(b);
}

// 16bit compare OPs
inline simd_uint32_8 cmp_equals_i16(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.equals_i16(b);
}

inline simd_uint32_8 cmp_not_equals_i16(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.not_equals_i16(b);
}

inline simd_uint32_8 cmp_greater_i16(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.greater_i16(b);
}

inline simd_uint32_8 cmp_greater_equals_i16(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.greater_equals_i16(b);
}

inline simd_uint32_8 cmp_lesser_i16(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.lesser_i16(b);
}

inline simd_uint32_8 cmp_lesser_equals_i16(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.lesser_equals_i16(b);
}

// 32bit cmps.
inline simd_uint32_8 cmp_equals_i32(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.equals_i32(b);
}

inline simd_uint32_8 cmp_not_equals_i32(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.not_equals_i32(b);
}

inline simd_uint32_8 cmp_greater_i32(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.greater_i32(b);
}

inline simd_uint32_8 cmp_greater_equals_i32(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.greater_equals_i32(b);
}

inline simd_uint32_8 cmp_lesser_i32(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.lesser_i32(b);
}

inline simd_uint32_8 cmp_lesser_equals_i32(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.lesser_equals_i32(b);
}

// 64bit cmps
inline simd_uint32_8 cmp_equals_i64(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.equals_i64(b);
}

inline simd_uint32_8 cmp_not_equals_i64(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.not_equals_i64(b);
}

inline simd_uint32_8 cmp_greater_i64(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.greater_i64(b);
}

inline simd_uint32_8 cmp_greater_equals_i64(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.greater_equals_i64(b);
}

inline simd_uint32_8 cmp_lesser_i64(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.lesser_i64(b);
}

inline simd_uint32_8 cmp_lesser_equals_i64(simd_uint32_8& a, simd_uint32_8& b)
{
	__DECL_ALIGNED(32) simd_uint32_8  _r(a);
	return _r.lesser_equals_i64(b);
}
