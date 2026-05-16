/*!
  @todo will move to another directory.
*/

#pragma once

#include "../common.h"
#include "../types/types_video.h"
#include "../types/simd/uint16_8_t.hpp"
#include "../types/simd/uint32_8_t.hpp"
//#include "../types/simd/primitives_128.hpp"
//#include "../types/simd/primitives_256.hpp"

//#include "../types/util_rgbconvert.h"
#if defined(_RGB555) || defined(_RGBA565)
using simd_scrntype8_class = simd_uint16_8;
#else /* RGB888 || RGBA8888 */
using simd_scrntype8_class = simd_uint32_8;
#endif
// Note: table strongly recommend to be aligned by sizeof(uint16_vec8_t).
// This is sizeof(uint16) * 8, some compilers may require to align 16bytes(128)
// when using SIMD128 -- 20181105 K.O
template <typename _TBL_T, typename _VAL_T>
	void PrepareBitTransTable(_TBL_T *tbl, _VAL_T on_val, _VAL_T off_val)
{
	__UNLIKELY_IF(tbl == NULL) return;
	for(_VAL_T i = 0; i < 256; i++) {
		_VAL_T n = i;
__DECL_VECTORIZED_LOOP
		for(size_t j = 0; j < 8; j++) {
			tbl->plane_table[i].w[j] = ((n & ((_VAL_T)0x80)) == 0) ? off_val : on_val;
			n <<= 1;
		}
	}
}

// Prepare reverse byte-order table(s).
template <typename _TBL_T, typename _VAL_T>
	void PrepareReverseBitTransTable(_TBL_T *tbl, _VAL_T on_val, _VAL_T off_val)
{
	__UNLIKELY_IF(tbl == NULL) return;
	for(_VAL_T i = 0; i < 256; i++) {
		_VAL_T n = i;
__DECL_VECTORIZED_LOOP
		for(size_t j = 0; j < 8; j++) {
			tbl->plane_table[i].w[j] = ((n & 0x01) == 0) ? off_val : on_val;
			n >>= 1;
		}
	}
}


inline uint16_8_t GetTwoValues_8x8bit_ltor(const uint8_t odd_base, const uint8_t __on_val, const uint8_t __off_val)
{
	__DECL_ALIGNED(16) uint16_8_t __tmp;
	const uint8_t __base = odd_base & 0xfe;
	__DECL_VECTORIZED_LOOP
	for(uint8_t i = 0; i < 8; i++) {
		uint8_t _v = (((0x80 >> i) & __base) != 0) ? __on_val : __off_val;
		__tmp.u8[i] = _v;
	}
	__DECL_VECTORIZED_LOOP
	for(uint8_t i = 0; i < 8; i++) {
		uint8_t _v = (((0x80 >> i) & (__base + 1)) != 0) ? __on_val : __off_val;
		__tmp.u8[i + 8] = _v;
	}
	return __tmp;
}

inline uint16_8_t GetTwoValues_8x8bit_rtol(const uint8_t odd_base, const uint8_t __on_val, const uint8_t __off_val)
{
	__DECL_ALIGNED(16) uint16_8_t __tmp;
	const uint8_t __base = odd_base & 0xfe;
	__DECL_VECTORIZED_LOOP
	for(uint8_t i = 0; i < 8; i++) {
		uint8_t _v = (((0x01 << i) & __base) != 0) ? __on_val : __off_val;
		__tmp.u8[i] = _v;
	}
	__DECL_VECTORIZED_LOOP
	for(uint8_t i = 0; i < 8; i++) {
		uint8_t _v = (((0x01 << i) & (__base + 1)) != 0) ? __on_val : __off_val;
		__tmp.u8[i + 8] = _v;
	}
	return __tmp;
}


inline void PrepareBitTransTable_8bit_8bitRange(void* tbl, const uint8_t bitshift, const bool is_on_dot, const bool right_to_left, const uint8_t begin_val, const uint16_t numbers)
{
	__UNLIKELY_IF(tbl == NULL) return;
	__DECL_ALIGNED(16) uint16_8_t __tmp;

	simde__m128* p = (simde__m128*)tbl;
	
	const uint8_t __val = 1 << (bitshift & 7);
	const uint8_t __on_val  = (is_on_dot) ? __val : 0;
	const uint8_t __off_val = (is_on_dot) ? 0 : __val;

	const bool __is_aligned = simd_128bit::is_aligned(tbl);
	for(uint16_t i = 0; i < numbers; i += 2)
	{
		const uint8_t __base = (uint8_t)(i + (uint16_t)begin_val);
		if(right_to_left) {
			__tmp = GetTwoValues_8x8bit_rtol(__base, __on_val, __off_val);			
		} else {
			__tmp = GetTwoValues_8x8bit_ltor(__base, __on_val, __off_val);			
		}
		__LIKELY_IF(__is_aligned) {
			simd_128bit::store_aligned(&(p[i / 2]), __tmp.v);
		} else {
			simd_128bit::store_unaligned(&(p[i / 2]), __tmp.v);
		}
	}
}


inline uint16_8_t GetValue_8x16bit_ltor(const uint8_t __base, const uint16_t __on_val, const uint16_t __off_val)
{
	__DECL_ALIGNED(16) uint16_8_t __tmp;
	__DECL_VECTORIZED_LOOP
	for(uint8_t i = 0; i < 8; i++) {
		uint16_t _v = (((0x80 >> i) & __base) != 0) ? __on_val : __off_val;
		__tmp.u16[i] = _v;
	}
	return __tmp;
}

inline uint16_8_t GetValue_8x16bit_rtol(const uint8_t __base, const uint16_t __on_val, const uint16_t __off_val)
{
	__DECL_ALIGNED(16) uint16_8_t __tmp;
	__DECL_VECTORIZED_LOOP
	for(uint8_t i = 0; i < 8; i++) {
		uint16_t _v = (((0x01 << i) & __base) != 0) ? __on_val : __off_val;
		__tmp.u16[i] = _v;
	}
	return __tmp;
}

inline void PrepareBitTransTable_16bit_8bitRange(void* tbl, const uint8_t bitshift, const bool is_on_dot, const bool right_to_left, const uint8_t begin_val, const uint16_t numbers)
{
	__UNLIKELY_IF(tbl == NULL) return;
	simde__m128* p = (simde__m128*)tbl;
	__DECL_ALIGNED(16) uint16_8_t __tmp;
	const uint16_t __val = 1 << (bitshift & 15);
	const uint16_t __on_val  = (is_on_dot) ? __val : 0;
	const uint16_t __off_val = (is_on_dot) ? 0 : __val;

	const bool __is_aligned = simd_128bit::is_aligned(tbl);
	for(uint16_t i = 0; i < numbers; i++)
	{
		const uint8_t __base = (uint8_t)(((uint16_t)begin_val) + i);
		if(right_to_left) {
			__tmp = GetValue_8x16bit_rtol(__base, __on_val, __off_val);			
		} else {
			__tmp = GetValue_8x16bit_ltor(__base, __on_val, __off_val);			
		}
		__LIKELY_IF(__is_aligned) {
			simd_128bit::store_aligned(&(p[i]), __tmp.v);
		} else {
			simd_128bit::store_unaligned(&(p[i]), __tmp.v);
		}
	}
}

inline void PrepareBitTransTable16_4bitRange(void* tbl, const uint8_t bitshift)
{
	PrepareBitTransTable_16bit_8bitRange(tbl, bitshift, true, true, 0, 16);
}

inline void PrepareBitTransTable16_4bitRange_Reverse(void* tbl, const uint8_t bitshift)
{
	PrepareBitTransTable_16bit_8bitRange(tbl, bitshift, true, false, 0, 16);
}

inline void PrepareBitTransTable16_8bitRange(void* tbl, const uint8_t bitshift)
{
	PrepareBitTransTable_16bit_8bitRange(tbl, bitshift, true, true, 0, 256);
}

inline void PrepareBitTransTable16_8bitRange_Reverse(void* tbl, const uint8_t bitshift)
{
	PrepareBitTransTable_16bit_8bitRange(tbl, bitshift, true, false, 0, 256);
}

inline void PrepareBitTransTable8_4bitRange(void* tbl, const uint8_t bitshift)
{
	PrepareBitTransTable_8bit_8bitRange(tbl, bitshift, true, true, 0, 16);
}

inline void PrepareBitTransTable8_4bitRange_Reverse(void* tbl, const uint8_t bitshift)
{
	PrepareBitTransTable_8bit_8bitRange(tbl, bitshift, true, false, 0, 16);
}

inline void PrepareBitTransTable8_8bitRange(void* tbl, const uint8_t bitshift)
{
	PrepareBitTransTable_8bit_8bitRange(tbl, bitshift, true, true, 0, 256);
}

inline void PrepareBitTransTable8_8bitRange_Reverse(void* tbl, const uint8_t bitshift)
{
	PrepareBitTransTable_8bit_8bitRange(tbl, bitshift, true, false, 0, 256);
}

static inline scrntype_vec8_t ConvertByteToMonochromePackedPixel(uint8_t src, _bit_trans_table_t *tbl,scrntype_t on_val, scrntype_t off_val)
{
	__DECL_ALIGNED(16) uint16_vec8_t  tmpd;
	__DECL_ALIGNED(32) scrntype_vec8_t tmpdd;

	uint16_t* vt = (uint16_t*)___assume_aligned(&(tbl->plane_table[src]), sizeof(uint16_vec8_t));
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpd.w[i] = vt[i];
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpdd.w[i] = (tmpd.w[i] == 0) ? off_val: on_val;
	}
	return tmpdd;
}


// Table must be (ON_VAL_COLOR : OFF_VAL_COLOR)[256].
static inline scrntype_vec8_t ConvertByteToPackedPixel_PixelTbl(uint8_t src, _bit_trans_table_scrn_t *tbl)
{
	__DECL_ALIGNED(32) scrntype_vec8_t tmpdd;
	scrntype_t* vt = (scrntype_t*)___assume_aligned(&(tbl->plane_table[src]), sizeof(scrntype_vec8_t)); 
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpdd.w[i] = vt[i];
	}
	return tmpdd;
}

inline simd_uint16_8 Get4PixelsFromRGB(uint8_t r, uint8_t g, uint8_t b, uint16_8_t* rtbl, uint16_8_t* gtbl, uint16_8_t* btbl, constexpr bool is_left)
{
	__DECL_ALIGNED(16) simd_uint16_8 tmpr;
	__DECL_ALIGNED(16) simd_uint16_8 tmpg;
	__DECL_ALIGNED(16) simd_uint16_8 tmpb;
	uint16_8_t*  rvt = (uint16_8_t*)___assume_aligned(rtbl, sizeof(uint16_8_t));
	uint16_8_t*  gvt = (uint16_8_t*)___assume_aligned(gtbl, sizeof(uint16_8_t));
	uint16_8_t*  bvt = (uint16_8_t*)___assume_aligned(btbl, sizeof(uint16_8_t));

	if(is_left) { // Bit7 to 4
		r >>= 4;
		g >>= 4;
		b >>= 4;
	} else {
		r &= 0x0f;
		g &= 0x0f;
		b &= 0x0f;
	}
	tmpr.align_load(&(rvt[r]));
	tmpg.align_load(&(gvt[g]));
	tmpb.align_load(&(bvt[b]));
	tmpr |= tmpg;
	tmpr |= tmpb;
	if(is_left) { // Bit7 to 4
		tmpr.v = simd_128bit::op_lshift_bytes<8>(tmpr.v);
	}
	return tmpr;
}

inline simd_uint16_8 Get4PixelsFromRGBI(uint8_t r, uint8_t g, uint8_t b, uint8_t i, uint16_8_t* rtbl, uint16_8_t* gtbl, uint16_8_t* btbl, uint16_8_t* itbl, constexpr bool is_left)
{
	__DECL_ALIGNED(16) simd_uint16_8 tmpr;
	__DECL_ALIGNED(16) simd_uint16_8 tmpg;
	__DECL_ALIGNED(16) simd_uint16_8 tmpb;
	__DECL_ALIGNED(16) simd_uint16_8 tmpi;
	uint16_8_t*  rvt = (uint16_8_t*)___assume_aligned(rtbl, sizeof(uint16_8_t));
	uint16_8_t*  gvt = (uint16_8_t*)___assume_aligned(gtbl, sizeof(uint16_8_t));
	uint16_8_t*  bvt = (uint16_8_t*)___assume_aligned(btbl, sizeof(uint16_8_t));
	uint16_8_t*  ivt = (uint16_8_t*)___assume_aligned(itbl, sizeof(uint16_8_t));

	if(is_left) { // Bit7 to 4
		r >>= 4;
		g >>= 4;
		b >>= 4;
		i >>= 4;
	} else {
		r &= 0x0f;
		g &= 0x0f;
		b &= 0x0f;
		i &= 0x0f;
	}
	tmpr.align_load(&(rvt[r]));
	tmpg.align_load(&(gvt[g]));
	tmpb.align_load(&(bvt[b]));
	tmpi.align_load(&(ivt[i]));
	tmpr |= tmpg;
	tmpr |= tmpb;
	tmpr |= tmpi;
	if(is_left) { // Bit7 to 4
		tmpr.v = simd_128bit::op_lshift_bytes<8>(tmpr.v);
	}
	return tmpr;
}

template <typename _St>
	inline void ConvertRGBTo8ColorsUint8(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, uint16_8_t* rtbl, uint16_8_t* gtbl, uint16_8_t* btbl, _St shift)
{
	__DECL_ALIGNED(16) simd_uint16_8 tmpd;

	tmpd  = Get4PixelsFromRGB(r, g, b, rtbl, gtbl, btbl, true);
	tmpd |= Get4PixelsFromRGB(r, g, b, rtbl, gtbl, btbl, false);
	if(shift > 0) {
		tmpd >>= shift;
	} else if(shift < 0) {
		tmpd <<= -shift;
	}
	__DECL_ALIGNED(8) uint8_8_t tmpdd;
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		tmpdd.b[i] = (uint8_t)(tmpd.at<uint16_t>(i));
	}
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		dst[i] = tmpdd.b[i];
	}
}

template <typename _St>
	inline void ConvertRGBTo8ColorsUint8_Zoom2Left(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, uint16_8_t* rtbl, uint16_8_t* gtbl,uint16_8_t* btbl, _St shift)
{
	__DECL_ALIGNED(16) simd_uint16_8 tmpd;

	tmpd  = Get4PixelsFromRGB(r >> 4, g >> 4, b >> 4, rtbl, gtbl, btbl, false); // left nibble, but reduce to shift-cost.
	
	if(shift > 0) {
		tmpd >>= shift;
	} else if(shift < 0) {
		tmpd <<= -shift;
	}

	__DECL_ALIGNED(8) uint8_8_t tmpdd;
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0, j = 0; i < 8; i += 2, j++) {
		uint8_t __tmp = (uint8_t)(tmpd.at<uint16_t>(j));
		tmpdd.b[i    ] = __tmp;
		tmpdd.b[i + 1] = __tmp;
	}
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		dst[i] = tmpdd.b[i];
	}
}

template <typename _St>
	inline void ConvertRGBTo8ColorsUint8_Zoom2Right(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, uint16_8_t* rtbl, uint16_8_t* gtbl,uint16_8_t* btbl, _St shift)
{
	__DECL_ALIGNED(16) simd_uint16_8 tmpd;

	tmpd = Get4PixelsFromRGB(r, g, b, rtbl, gtbl, btbl, false);
	
	if(shift > 0) {
		tmpd >>= shift;
	} else if(shift < 0) {
		tmpd <<= -shift;
	}
	__DECL_ALIGNED(8) uint8_8_t tmpdd;
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0, j = 0; i < 8; i += 2, j++) {
		uint8_t __tmp = (uint8_t)(tmpd.at<uint16_t>(j));
		tmpdd.b[i    ] = __tmp;
		tmpdd.b[i + 1] = __tmp;
	}
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		dst[i] = tmpdd.b[i];
	}
}



template <typename _St>
	void __FASTCALL Render8Colors_Line(scrntype_t *dst, scrntype_t *dst2, uint8_t *src,
										   uint32_t startx, uint32_t x_width,
										   scrntype8_t* palette,
										   uint16_8_t *r_table, uint16_8_t *g_table, uint16_8_t* b_table,
										   const bool scan_line,
										   uint32_t base_address_r, uint32_t base_address_g, uint32_t base_address_b,
										   uint32_t voffset, const uint32_t address_mask, const uint32_t offset_mask,
										   const bool is_render_rgb[4], _St bitshift, size_t bytes)
{
	__UNLIKELY_IF(src == NULL) return;
	__UNLIKELY_IF(dst == NULL) return;

	__UNLIKELY_IF(r_table == NULL) return;
	__UNLIKELY_IF(g_table == NULL) return;
	__UNLIKELY_IF(b_table == NULL) return;


	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class palette_cache;
	__UNLIKELY_IF(palette == NULL) {
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			palette_cache.set(i, 
							  (scrntype_t)(RGBA_COLOR(((i & 2) != 0) ? 0xff : 0, 
													  ((i & 4) != 0) ? 0xff : 0,
													  ((i & 1) != 0) ? 0xff : 0,
													  255)));
		}
		palette_cache.set(0, RGBA_COLOR(0, 0, 0, 0)); // OK?
	} else {
		palette_cache.unalign_load(palette);
	}

   
	uint32_t x = startx;

	uint8_t *rp = &(src[base_address_r]);
	uint8_t *gp = &(src[base_address_g]);
	uint8_t *bp = &(src[base_address_b]);

	bool is_render_r = is_render_rgb[0];
	bool is_render_g = is_render_rgb[1];
	bool is_render_b = is_render_rgb[2];


	uint32_t n = x;
	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class sline;
	sline.fill((scrntype_t)RGBA_COLOR(31, 31, 31, 255));

	__DECL_ALIGNED(16) simd_uint16_8 r_array;
	__DECL_ALIGNED(16) simd_uint16_8 g_array;
	__DECL_ALIGNED(16) simd_uint16_8 b_array;
	__DECL_ALIGNED(16) simd_uint16_8 tmpd;
	__DECL_ALIGNED(16) const simd_uint16_8 maskd((uint16_t)0x07);
	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class tmpdd;
	
	uint16_8_t *vpb = (uint16_8_t*)___assume_aligned(b_table, sizeof(uint16_8_t));
	uint16_8_t *vpr = (uint16_8_t*)___assume_aligned(r_table, sizeof(uint16_8_t));
	uint16_8_t *vpg = (uint16_8_t*)___assume_aligned(g_table, sizeof(uint16_8_t));

	for(uint32_t xx = 0; xx < bytes; xx++) {
		const uint32_t abs_offset = (voffset + n) & address_mask;
		uint8_t _r = (is_render_r) ? rp[abs_offset] : 0;
		uint8_t _g = (is_render_g) ? gp[abs_offset] : 0;
		uint8_t _b = (is_render_b) ? bp[abs_offset] : 0;

		// Note: Should pre-allocate valarrays to improbe speed.
		tmpd  = Get4PixelsFromRGB(_r, _g, _b, r_table, g_table, b_table, true);
		tmpd |= Get4PixelsFromRGB(_r, _g, _b, r_table, g_table, b_table, false);
	   
		__LIKELY_IF(bitshift > 0) {
			tmpd >>= bitshift;
		} else if(bitshift < 0) {
			tmpd <<= -bitshift;
		}
		tmpd &= maskd;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			tmpdd.set_unsafe(i, palette_cache.at<scrntype_t>(tmpd.at<uint16_t>(i))); 
		}
		tmpdd.store(dst);
		dst += 8;
		__LIKELY_IF(dst2 != nullptr) {
			if(scan_line) {
#if defined(_RGB555) || defined(_RGBA565)
				tmpdd >>= 2;
#else // 24bit
				tmpdd >>= 3;
#endif
				tmpdd &= sline;
			}
			tmpdd.store(dst2);
			dst2 += 8;
		}
		n = (n + 1) & offset_mask;
	}
}


template <typename _St>
	void __FASTCALL Render16Colors_Line2(scrntype_t *dst, scrntype_t *dst2, uint8_t *src,
										 uint32_t startx, uint32_t x_width,
										 scrntype8_t* palette,
										 uint16_8_t *r_table, uint16_8_t *g_table, uint16_8_t* b_table, uint16_8_t* i_table,
										 const bool scan_line,
										 uint32_t base_address_r, uint32_t base_address_g,
										 uint32_t base_address_b, uint32_t base_address_i,
										 uint32_t voffset, const uint32_t address_mask, const uint32_t offset_mask,
										 const bool is_render_rgb[4], _St bitshift, size_t bytes)
{
	__UNLIKELY_IF(src == NULL) return;
	__UNLIKELY_IF(dst == NULL) return;

	__UNLIKELY_IF(r_table == NULL) return;
	__UNLIKELY_IF(g_table == NULL) return;
	__UNLIKELY_IF(b_table == NULL) return;
	__UNLIKELY_IF(i_table == NULL) return;


	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class palette_cache;
	__UNLIKELY_IF(palette == NULL) {
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			scrntype_t boost_val = (i < 8) ? 0 : RGBA_COLOR(0x80, 0x80, 0x80, 255);
			palette_cache.set(i,
							  (scrntype_t)(RGBA_COLOR(((i & 2) != 0) ? 0x7f : 0, 
													  ((i & 4) != 0) ? 0x7f : 0,
													  ((i & 1) != 0) ? 0x7f : 0,
													  255)) | boost_val
				);
		}
		palette_cache.set(0, RGBA_COLOR(0, 0, 0, 0)); // OK?
	} else {
		palette_cache.unalign_load(palette);
	}
	uint32_t x = startx;

	uint8_t *rp = &(src[base_address_r]);
	uint8_t *gp = &(src[base_address_g]);
	uint8_t *bp = &(src[base_address_b]);
	uint8_t *ip = &(src[base_address_i]);

	bool is_render_r = is_render_rgb[0];
	bool is_render_g = is_render_rgb[1];
	bool is_render_b = is_render_rgb[2];
	bool is_render_i = is_render_rgb[3];

	uint32_t n = x;
	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class sline;
	sline.fill((scrntype_t)RGBA_COLOR(31, 31, 31, 255));

	__DECL_ALIGNED(16) simd_uint16_8 r_array;
	__DECL_ALIGNED(16) simd_uint16_8 g_array;
	__DECL_ALIGNED(16) simd_uint16_8 b_array;
	__DECL_ALIGNED(16) simd_uint16_8 i_array;
	__DECL_ALIGNED(16) simd_uint16_8 tmpd;
	__DECL_ALIGNED(16) const simd_uint16_8 maskd((uint16_t)0x0f);
	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class tmpdd;
	

	for(uint32_t xx = 0; xx < bytes; xx++) {
		const uint32_t abs_offset = (voffset + n) & address_mask;
		uint8_t _r = (is_render_r) ? rp[abs_offset] : 0;
		uint8_t _g = (is_render_g) ? gp[abs_offset] : 0;
		uint8_t _b = (is_render_b) ? bp[abs_offset] : 0;
		uint8_t _i = (is_render_i) ? ip[abs_offset] : 0;

		// Note: Should pre-allocate valarrays to improbe speed.
		tmpd  = Get4PixelsFromRGBI(_r, _g, _b, _i, r_table, g_table, b_table, i_table, true);
		tmpd |= Get4PixelsFromRGBI(_r, _g, _b, _i, r_table, g_table, b_table, i_table, false);
		
	   
		__LIKELY_IF(bitshift > 0) {
			tmpd >>= bitshift;
		} else if(bitshift < 0) {
			tmpd <<= -bitshift;
		}
		tmpd &= maskd;
		
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			tmpdd.set_unsafe(i, palette_cache.at<scrntype_t>(tmpd.at<uint16_t>(i))); 
		}
		tmpdd.store(dst);
		dst += 8;
		__LIKELY_IF(dst2 != nullptr) {
			if(scan_line) {
#if defined(_RGB555) || defined(_RGBA565)
				tmpdd >>= 2;
#else // 24bit
				tmpdd >>= 3;
#endif
				tmpdd &= sline;
			}
			tmpdd.store(dst2);
			dst2 += 8;
		}
		n = (n + 1) & offset_mask;
	}
}




template <typename _St>
	void __FASTCALL Convert8ColorsToByte_Line(uint8_t *dst, uint8_t *src,
										   uint32_t startx, uint32_t x_width,
										   uint16_8_t *r_table, uint16_8_t *g_table, uint16_8_t* b_table,
										   uint32_t base_address_r, uint32_t base_address_g, uint32_t base_address_b,
										   uint32_t voffset, const uint32_t address_mask, const uint32_t offset_mask,
										   const bool is_render_rgb[4], _St bitshift)
{
	__DECL_ALIGNED(16) simd_uint16_8 tmpd;
	__DECL_ALIGNED(16) const simd_uint16_8 maskd((uint16_t)0x07);

	uint32_t noffset = startx & offset_mask;
	uint8_t b, r, g;
	uint8_t *bp = &(src[base_address_b]);
	uint8_t *rp = &(src[base_address_r]);
	uint8_t *gp = &(src[base_address_g]);

	for(int x = 0; x < x_width; x++) {
		b = (is_render_rgb[2]) ? bp[(noffset + voffset) & address_mask] : 0;
		r = (is_render_rgb[0]) ? rp[(noffset + voffset) & address_mask] : 0;
		g = (is_render_rgb[1]) ? gp[(noffset + voffset) & address_mask] : 0;

		noffset = (noffset + 1) & offset_mask;

		tmpd  = Get4PixelsFromRGB(r, g, b, r_table, g_table, b_table, true);
		tmpd |= Get4PixelsFromRGB(r, g, b, r_table, g_table, b_table, false);

		if(bitshift > 0) {
			tmpd = tmpd >> bitshift;
		} else if(bitshift < 0) {
			tmpd = tmpd << -bitshift;
		}
		tmpd &= maskd;
		
		__DECL_ALIGNED(8) uint8_t_t tmpdd;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			tmpdd.b[i] = (uint8_t)(tmpd.at<uint16_t>(i));
		}
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0; i < 8; i++) {
			dst[i] = tmpdd.b[i];
		}
		dst += 8;
	}
}


