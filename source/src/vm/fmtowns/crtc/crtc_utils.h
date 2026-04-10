#ifdef _TOWNS_CRTC_H_ /* You must include crtc.h before. */
#ifndef _TOWNS_CRTC_UTILS_H_ /* And this must be included at once. */
#define _TOWNS_CRTC_UTILS_H_ 

#include "../../../common.h"
#include "../types/types_video.h"
#include "../types/simd/uint16_8_t.hpp"
#include "../types/simd/uint32_8_t.hpp"
//#include "../../../types/simd_types.h"
//#include "../../../types/simd/primitives_128.hpp"
//#include "../../../types/simd/primitives_256.hpp"

namespace FMTOWNS {

	
inline bool TOWNS_CRTC::is_align_scrntype8(void* p)
{
	return SCRNTYPE8_SIMD::is_aligned(p);
}

	
inline void TOWNS_CRTC::store8_aligned(scrntype_t* dst, scrntype8_t data)
{
	SCRNTYPE8_SIMD::store_aligned((simd_scrntype8_t*)dst, data.v);
}

inline void TOWNS_CRTC::store8_unaligned(scrntype_t* dst, scrntype8_t data)
{
	SCRNTYPE8_SIMD::store_unaligned((simd_scrntype8_t*)dst, data.v);
}

inline void TOWNS_CRTC::store8_limited(scrntype_t* dst, scrntype8_t data, const size_t num)
{
	__UNLIKELY_IF(num == 0) {
		return;
	}
	__UNLIKELY_IF(num >= 8) {
		store8_unaligned(dst, data);
		return;
	}
	for(size_t i = 0; i < num; i++) {
		dst[i] = simd_element(data, i);
	}
}

inline scrntype8_t TOWNS_CRTC::zero_scrntype8_t()
{
	__DECL_SCRNTYPE8_ALIGNED scrntype8_t r;
	r.v = SCRNTYPE8_SIMD::op_clear();
	return r;
}

	
inline void TOWNS_CRTC::store8_pix(const scrntype_t *dst, scrntype8_t data)
{
	if(is_align_scrntype8((void *)dst)) {
		store8_aligned((scrntype_t*)dst, data);
	} else {
		store8_unaligned((scrntype_t*)dst, data);
	}
}
	
inline void TOWNS_CRTC::store_pix(scrntype_t *dst, scrntype8_t data, const size_t words)
{
	store8_limited(dst, data, words);
}

inline scrntype8_t TOWNS_CRTC::load8_aligned(scrntype_t* src)
{
	__DECL_SCRNTYPE8_ALIGNED scrntype8_t _ret;
	_ret.v = SCRNTYPE8_SIMD::load_aligned((simd_scrntype8_t*)src);
	return _ret;
}

inline scrntype8_t TOWNS_CRTC::load8_unaligned(scrntype_t* src)
{
	__DECL_SCRNTYPE8_ALIGNED  scrntype8_t _ret;
	_ret.v = SCRNTYPE8_SIMD::load_unaligned((simd_scrntype8_t*)src);
	return _ret;
}

inline scrntype8_t TOWNS_CRTC::load8_limited(scrntype_t* src, size_t num)
{
	__DECL_SCRNTYPE8_ALIGNED scrntype8_t _ret;
	__UNLIKELY_IF(num == 0) {	
		return zero_scrntype8_t();
	}
	__UNLIKELY_IF(num >= 8) {
		return load8_unaligned(src);
	}
	_ret = zero_scrntype8_t();
	for(size_t i = 0; i < num; i++) {
		simd_element_raw(_ret, i) = (simd_element_cast)(src[i]);
	}
	return _ret;
}

inline scrntype8_t TOWNS_CRTC::load8_pix(const scrntype_t *src)
{
	__DECL_SCRNTYPE8_ALIGNED scrntype8_t _ret;
	if(is_align_scrntype8((void *)src)) {
		_ret = load8_aligned((scrntype_t*)src);
	} else {
		_ret = load8_unaligned((scrntype_t*)src);
	}
	return _ret;
}

inline scrntype8_t TOWNS_CRTC::load_pix(scrntype_t *src, size_t words)
{
	__DECL_SCRNTYPE8_ALIGNED scrntype8_t _ret;
	_ret = load8_limited(src, words);
	return _ret;
}

inline void TOWNS_CRTC::simd_fill(scrntype_t* dst, scrntype8_t data, size_t words)
{
	__UNLIKELY_IF(dst == nullptr) {
		return;
	}
	__UNLIKELY_IF(words == 0) {
		return;
	}
	__DECL_SCRNTYPE8_ALIGNED scrntype8_t _d;
	_d.v = data.v;
	size_t ptr = 0;
	const bool __aligned = is_align_scrntype8((void *)dst);
	for(size_t i = 0; i < words / 8; i++) {
		if(__aligned) {
			store8_aligned(&(dst[ptr]), _d);
		} else {
			store8_unaligned(&(dst[ptr]), _d);
		}
		ptr += 8;
	}
	if((words & 7) != 0) {
		__UNLIKELY_IF(ptr < 8) {
			ptr = 8;
		}
		ptr -= 8;
		store8_limited(&(dst[ptr]), _d, words & 7);
	}
}

inline void TOWNS_CRTC::simd_copy(scrntype_t* dst, scrntype_t* src, size_t words)
{
	const uintptr_t pdst = (uintptr_t)dst;
	const uintptr_t psrc = (uintptr_t)src;
	__UNLIKELY_IF(words == 0) {
		return;
	}
	__UNLIKELY_IF((dst == nullptr) || (src == nullptr)) {
		return;
	}
	memcpy(dst, src, words * sizeof(scrntype_t));
}

inline size_t TOWNS_CRTC::store_x1(scrntype_t *dst, const scrntype8_t *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	size_t xx = 0;
	__DECL_SCRNTYPE8_ALIGNED scrntype8_t _tmp;
	__LIKELY_IF(is_align_scrntype8(dst)) {
		for(size_t x = 0; (x < words) && (width >= 8) ; x++) {
			_tmp = load8_pix((const scrntype_t*)(&(src[x])));
			store8_aligned(dst, _tmp);
			dst += 8;
			width -= 8;
			pixels_count += 8;
			xx++;
		}
	} else {
		for(size_t x = 0; (x < words) && (width >= 8) ; x++) {
			_tmp = load8_pix((const scrntype_t*)(&(src[x])));
			store8_unaligned(dst, _tmp);
			dst += 8;
			width -= 8;
			pixels_count += 8;
			xx++;
		}
	}
	__UNLIKELY_IF((width > 0) && (xx < words)) {
		_tmp = load8_pix((const scrntype_t*)(&(src[xx])));
		for(size_t i = 0; (i < 8) && (width > 0); i++) {
			dst[i] = simd_element(_tmp, i);
			pixels_count++;
			width--;
		}
	}
	width = 0;
	return pixels_count;
}


inline void TOWNS_CRTC::pix_multiply_x2(scrntype8_t dst[2], const scrntype8_t data)
{
	__DECL_VECTORIZED_LOOP
	for(size_t lx = 0; lx < 8; lx++) {
		simd_element_raw(dst[0], lx) = simd_element_raw(data, lx >> 1);
	}
	__DECL_VECTORIZED_LOOP
	for(size_t rx = 8; rx < 16; rx++) {
		simd_element_raw(dst[1], rx - 8) = simd_element_raw(data, rx >> 1);
	}
}

inline size_t TOWNS_CRTC::store_x2(scrntype_t *dst, const scrntype8_t *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	__DECL_SCRNTYPE8_ALIGNED  scrntype8_t _s;
	__DECL_SCRNTYPE8_ALIGNED  scrntype8_t _tmp[2];
	const bool is_dst_aligned = is_align_scrntype8(dst);
	size_t xx = 0;
	for(size_t x = 0; (x < words) && (width >=16) ; x++) {
		_s = load8_pix((const scrntype_t*)(&(src[x])));
		pix_multiply_x2(_tmp, _s);
		
		__LIKELY_IF(is_dst_aligned) {
			store8_aligned(&(dst[0]), _tmp[0]);
			store8_aligned(&(dst[8]), _tmp[1]);
		} else {
			store8_unaligned(&(dst[0]), _tmp[0]);
			store8_unaligned(&(dst[8]), _tmp[1]);
		}
		dst += 16;
		width -= 16;
		pixels_count += 16;
		xx++;
	}
	__UNLIKELY_IF((width > 0) && (xx < words)) {
		_s = load8_pix((const scrntype_t*)(&(src[xx])));
		pix_multiply_x2(_tmp, _s);
		for(size_t i = 0; (i < 8) && (width > 0); i++) {
			dst[i] = simd_element(_tmp[0], i);
			pixels_count++;
			width--;
		}
		dst += 8;
		if(width > 0) {
			for(size_t i = 0; (i < 8) && (width > 0); i++) {
				dst[i] = simd_element(_tmp[1], i);
				pixels_count++;
				width--;
			}
		}
//		if(width > 7) {
//			store8_unaligned(&(dst[0]), _tmp[0]);
//			if(width > 8) {
//				store8_limited(&(dst[8]), _tmp[1], width - 8);
//			}
//		} else {
//			store8_limited(dst, _tmp[0], width);
//		}
//		pixels_count += width;
	}
	width = 0;
	return pixels_count;
}

inline void TOWNS_CRTC::pix_multiply_x4(scrntype8_t dst[4], const scrntype8_t data)
{
	__DECL_VECTORIZED_LOOP
	for(size_t lx = 0; lx < 8; lx++) {
		simd_element_raw(dst[0], lx) = simd_element_raw(data, lx >> 2);
	}
	__DECL_VECTORIZED_LOOP
	for(size_t x1 = 8; x1 < 16; x1++) {
		simd_element_raw(dst[1], x1 - 8) = simd_element_raw(data, x1 >> 2);
	}
	__DECL_VECTORIZED_LOOP
	for(size_t x2 = 16; x2 < 24; x2++) {
		simd_element_raw(dst[2], x2 - 16) = simd_element_raw(data, x2 >> 2);
	}
	__DECL_VECTORIZED_LOOP
	for(size_t x3 = 24; x3 < 32; x3++) {
		simd_element_raw(dst[3], x3 - 24) = simd_element_raw(data, x3 >> 2);
	}
}

inline size_t TOWNS_CRTC::store_x4(scrntype_t *dst, const scrntype8_t *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	__DECL_SCRNTYPE8_ALIGNED  scrntype8_t _s;
	__DECL_SCRNTYPE8_ALIGNED  scrntype8_t _tmp[4];
	size_t xx = 0;
	const bool is_dst_aligned = is_align_scrntype8(dst);
	
	for(size_t x = 0; (x < words) && (width >= 32) ; x++) {
		_s = load8_pix((const scrntype_t*)(&(src[x])));
		pix_multiply_x4(_tmp, _s);
		__LIKELY_IF(is_dst_aligned) {
			store8_aligned(&(dst[0]),  _tmp[0]);
			store8_aligned(&(dst[8]),  _tmp[1]);
			store8_aligned(&(dst[16]), _tmp[2]);
			store8_aligned(&(dst[24]), _tmp[3]);
		} else {
			store8_unaligned(&(dst[0]),  _tmp[0]);
			store8_unaligned(&(dst[8]),  _tmp[1]);
			store8_unaligned(&(dst[16]), _tmp[2]);
			store8_unaligned(&(dst[24]), _tmp[3]);
		}
		dst += 32;
		width -= 32;
		pixels_count += 32;
		xx++;
	}

	__UNLIKELY_IF((width > 0) && (xx < words)) {
		_s = load8_pix((const scrntype_t*)(&(src[xx])));
		pix_multiply_x4(_tmp, _s);
		for(size_t x0 = 0; (width > 0) && (x0 < 4); x0++) {
			for(size_t j = 0; (j < 8) && (width > 0); j++) {
				dst[j] = simd_element(_tmp[x0], j);
				pixels_count++;
				width--;
			}
		}
//		size_t x2 = 0;
//		for(size_t w = width; (w >= 8) && (x2 < 4); w -= 8) {
//			store8_pix(dst, _tmp[x2]);
//			dst += 8;
//			pixels_count += 8;
//			x2++;
//		}
//		if(((width & 7) != 0) && (x2 < 4)){
//			store_pix(dst, _tmp[x2], width & 7);
//			pixels_count += (width & 7);
//		}
	}
	width = 0;
	return pixels_count;
}

inline size_t TOWNS_CRTC::store_n(scrntype_t *dst, const scrntype8_t *src, const int mag, const size_t words, size_t& width)
{
	__UNLIKELY_IF(mag <= 0) {
		return 0;
	}
	size_t pixels_count = 0;
	__DECL_SCRNTYPE8_ALIGNED  scrntype8_t _s;
	size_t xx = 0;
	const bool is_dst_aligned = is_align_scrntype8(dst);
	
	size_t dst_lp1 = 0;
	size_t dst_lp2 = 0;
	#if 1
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		_s = load8_pix((const scrntype_t*)(&(src[x])));
		for(size_t i = 0; (i < 8) && (width > 0) ; i++) {
			for(size_t j = 0; (j < mag) && (width > 0); j++) {
				dst[xx++] = simd_element(_s, i);
				pixels_count++;
				width--;
			}
		}
	}
	#else
	__LIKELY_IF(mag <= 16) {
		__DECL_SCRNTYPE8_ALIGNED  scrntype8_t _tmp[16];
		__DECL_SCRNTYPE8_ALIGNED const scrntype8_t _tmp_zero = zero_scrntype8_t();
		for(size_t x = 0; (x < words) && (width > 0) ; x++) {
			_s = load8_pix((const scrntype_t*)(&(src[x])));
			for(size_t n = 0; n < mag; n++) {
				_tmp[n] = _tmp_zero;
			}
			// Multiply
			dst_lp1 = 0;
			dst_lp2 = 0;
			for(size_t x2 = 0; x2 < 8; x2++) {
				for(size_t n = 0; n < mag; n++) {
					simd_element_raw(_tmp[dst_lp1], dst_lp2) = simd_element_raw(_s, x2);
					dst_lp2++;
					__UNLIKELY_IF(dst_lp2 > 7) {
						dst_lp1++;
						dst_lp2 = 0;
					}
				}
			}
			for(size_t xr = 0; xr < mag; xr++) {
				__LIKELY_IF(width > 7) {
					store8_pix(dst, _tmp[xr]);
					dst += 8;
					width -= 8;
					pixels_count += 8;
				} else {
					store_pix(dst, _tmp[xr], width);
					dst += width;
					pixels_count += width;
					width = 0;
				}
			}
		}
	} else {
		for(size_t x = 0; (x < words) && (width > 0) ; x++) {
			_s = load8_pix((const scrntype_t*)(&(src[x])));
			for(size_t m = 0; m < 8; m++) {
				for(size_t n = 0; n < mag; n++) {
					*dst++ = simd_element(_s, m);
					width--;
					pixels_count++;
					__UNLIKELY_IF(width == 0) {
						break;
					}
				}
				__UNLIKELY_IF(width == 0) {
					break;
				}
			}
		}
	}
	#endif
	return pixels_count;
}



inline size_t TOWNS_CRTC::scaling_store(scrntype_t *dst, scrntype8_t *src, const int mag, const size_t words, size_t& width)
{
	__UNLIKELY_IF((dst == NULL) || (src == NULL)) return 0;

	uintptr_t dstval = (uintptr_t)dst;
	size_t pixels_count = 0;

	switch(mag) {
	case 1:
		pixels_count = store_x1(dst, src, words, width);
		break;
	case 2:
		pixels_count = store_x2(dst, src, words, width);
		break;
	case 4:
		pixels_count = store_x4(dst, src, words, width);
		break;
	default:
		pixels_count = store_n(dst, src, mag, words, width);
		break;
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::scaling_store_by_map(scrntype_t *dst, scrntype8_t *src, uint16_8_t magx_map, const size_t words, size_t& width)
{
	if((words == 0) || (width == 0)) return 0;
	__DECL_SCRNTYPE8_ALIGNED scrntype8_t _s;
	size_t pixels_count = 0;
	const size_t width_limit = width;
	size_t rwidth = width & 7;
	size_t __sum = 0;
	SIMDE_VECTORIZE
	for(size_t x = 0; x < 8; x++) {
		__sum += magx_map.u16[x];
	}
	__UNLIKELY_IF(__sum == 0) {
		width = 0;
		return 0;
	}
	scrntype_t pix;
	size_t ptr = 0;
	for(size_t x = 0; (x < words) && (width > 0); x++) {
		__UNLIKELY_IF(pixels_count >= width_limit ) {
			width = 0;
			pixels_count = width_limit;
			break;
		}
		__LIKELY_IF(width >= __sum) {
			_s = load8_pix((const scrntype_t*)(&(src[x])));
			SIMDE_VECTORIZE
			for(size_t i = 0; i < 8; i++) {
				pix = simd_element(_s, i);
				for(size_t j = 0; j < magx_map.u16[i]; j++) {
					dst[ptr++] = pix; 
				}
			}
			width -= __sum;
			pixels_count += __sum;
		} else {
			__UNLIKELY_IF(rwidth == 0) {
				rwidth = 8; // Temporally value
			}
			_s = load8_pix((const scrntype_t*)(&(src[x])));
			for(size_t i = 0; (i < rwidth) && (width > 0); i++) {
				pix = simd_element(_s, i);
				for(size_t j = 0; (j < magx_map.u16[i]) && (width > 0); j++) {
					dst[ptr++] = pix;
					pixels_count++;
					width--;
					__UNLIKELY_IF(pixels_count >= width_limit) {
						width = 0;
						break;
					}
				}
				__UNLIKELY_IF(pixels_count >= width_limit) {
					width = 0;
					pixels_count = width_limit;
					break;
				}
			}
			width = 0;
			break;
		}
	}
	width = 0;
	return pixels_count;
}
	
// Note: RGB range limits 0 to 31 (0x1f) .
inline simd_scrntype8_class TOWNS_CRTC::make_rgb_32768(const simd_uint16_8 r, const simd_uint16_8 g, const simd_uint16_8 b)
{
	__DECL_SCRNTYPE8_ALIGNED simd_scrntype8_class _ret;
	__DECL_ALIGNED(16) simd_uint16_8 rmask(r);
	__DECL_ALIGNED(16) simd_uint16_8 gmask(g);
	__DECL_ALIGNED(16) simd_uint16_8 bmask(b);
	__DECL_ALIGNED(16) simd_uint16_8 rbuf(r);
	__DECL_ALIGNED(16) simd_uint16_8 gbuf(g);
	__DECL_ALIGNED(16) simd_uint16_8 bbuf(b);
	
	__DECL_ALIGNED(16) simd_uint16_8 tmp_zero;
	tmp_zero.clear(); // OK?
	__DECL_ALIGNED(16) simd_uint16_8 tmp_pixmask0((uint16_t)0x001f); /* R, B */
	rmask.greater_i16(tmp_zero);
	gmask.greater_i16(tmp_zero);
	bmask.greater_i16(tmp_zero);
	
	rbuf &= rmask;
	gbuf &= gmask;
	bbuf &= bmask;
	rbuf &= tmp_pixmask0;
	gbuf &= tmp_pixmask0;
	bbuf &= tmp_pixmask0;
	
#if defined(_RGB555)
	rbuf <<= 10;
	gbuf <<= 5;
	_ret  = rbuf;
	_ret |= gbuf;
	_ret |= bbuf;
	#if defined(__BIG_ENDIAN__)
	_ret.bswap_self(sizeof(uint16_t));
	#endif
#elif defined(_RGB565)
	__DECL_ALIGNED(16) simd_uint16_8 __glastbit((uint16_t)(1 << 5));
	__glastbit &= gmask;
	gbuf <<= 6;
	gbuf |= __glastbit;
	rbuf <<= 11;
	
	_ret = rbuf;
	_ret |= gbuf;
	_ret |= bbuf;
	#if defined(__BIG_ENDIAN__)
	_ret.bswap_self(sizeof(uint16_t));
	#endif
#else /* _RGB565 */
	/* _RGB888 || _RGBA8888 */
	__DECL_ALIGNED(32) simd_scrntype8_class rbuf32;
	__DECL_ALIGNED(32) simd_scrntype8_class gbuf32;
	__DECL_ALIGNED(32) simd_scrntype8_class bbuf32;
	__DECL_ALIGNED(32) simd_scrntype8_class rmask32;
	__DECL_ALIGNED(32) simd_scrntype8_class gmask32;
	__DECL_ALIGNED(32) simd_scrntype8_class bmask32;
	//__DECL_ALIGNED(32) simd_scrntype8_class mask32;

	rbuf32.from_uint16_8<uint16_t, uint32_t>(rbuf);
	gbuf32.from_uint16_8<uint16_t, uint32_t>(gbuf);
	bbuf32.from_uint16_8<uint16_t, uint32_t>(bbuf);
	rmask32.from_uint16_8<int16_t, int32_t>(rmask);
	gmask32.from_uint16_8<int16_t, int32_t>(gmask);
	bmask32.from_uint16_8<int16_t, int32_t>(bmask);

	__DECL_ALIGNED(32) simd_scrntype8_class rbase;
	__DECL_ALIGNED(32) simd_scrntype8_class gbase;
	__DECL_ALIGNED(32) simd_scrntype8_class bbase;
	rbase.fill((scrntype_t)RGBA_COLOR(0x07, 0, 0, 0));
	gbase.fill((scrntype_t)RGBA_COLOR(0, 0x07, 0, 0));
	bbase.fill((scrntype_t)RGBA_COLOR(0, 0, 0x07, 0));
	// Delete Zero value.
	rbase &= rmask32;
	gbase &= gmask32;
	bbase &= bmask32;
	#if 0
	rbuf32 <<= 3;
	gbuf32 <<= 3;
	bbuf32 <<= 3;
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0; i < 8; i++) {
		_ret._d.u32[i] = RGBA_COLOR(rbuf32._d.u32[i] , gbuf32._d.u32[i] , bbuf32._d.u32[i] , 255);
	}
	#else
	_ret.fill((uint32_t)RGBA_COLOR(0, 0, 0, 255));
	#if defined(__LITTLE_ENDIAN__)
	rbuf32.op_lshift32((const size_t)3);
	gbuf32.op_lshift32((const size_t)(8 + 3));
	bbuf32.op_lshift32((const size_t)(16 + 3));
	#else /* __BIG_ENDIAN__ */
	rbuf32.op_lshift32((const size_t)(24 + 3));
	gbuf32.op_lshift32((const size_t)(16 + 3));
	bbuf32.op_lshift32((const size_t)(8 + 3));
	#endif
	_ret |= rbuf32;
	_ret |= gbuf32;
	_ret |= bbuf32;
	#endif
	_ret |= rbase;
	_ret |= gbase;
	_ret |= bbase;
	
	//__DECL_VECTORIZED_LOOP
	//for(size_t i = 0; i < 8; i++) {
	//	_ret.u32[i] = RGBA_COLOR((((rbuf.u16[i] << 3) & 0xf8) | 0x07), (((gbuf.u16[i] << 3) & 0xf8) | 0x07), (((bbuf.u16[i] << 3) & 0xf8) | 0x07), 255);
	//}

#endif /* _RGB888 || _RGBA888 */
	return _ret;
}
}
#endif /* _TOWNS_CRTC_UTILS_H_ */
#endif /* _TOWNS_CRTC_H_ */
