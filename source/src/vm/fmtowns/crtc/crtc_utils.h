#ifdef _TOWNS_CRTC_H_ /* You must include crtc.h before. */
#ifndef _TOWNS_CRTC_UTILS_H_ /* And this must be included at once. */
#define _TOWNS_CRTC_UTILS_H_ 

#include "../../../common.h"

namespace FMTOWNS {

inline bool TOWNS_CRTC::is_align_scrntype8(void* p)
{
	uintptr_t __p = (uintptr_t)p;
	const uintptr_t __mask = alignof(scrntype8_t) - 1;
	return ((__p & __mask) == 0);
}

constexpr bool TOWNS_CRTC::is_align_scrntype8_constexpr(const void* p)
{
	uintptr_t __p = (uintptr_t)p;
	const uintptr_t __mask = alignof(scrntype8_t) - 1;
	return ((__p & __mask) == 0);
}
	
inline void TOWNS_CRTC::store8_aligned(scrntype_t* dst, scrntype8_t data)
{
	#if defined(_RGB555) || defined(_RGB565)
	simde_mm_store_ps((simde_float32*)dst, data.v);
	#else
	simde_mm256_store_ps((simde_float32*)dst, data.v);
	#endif
}

inline void TOWNS_CRTC::store8_unaligned(scrntype_t* dst, scrntype8_t data)
{
	SIMDE_VECTORIZE
	for(size_t i = 0; i < 8; i++) {
		dst[i] = data.s[i];
	}
}

inline void store8_limited(scrntype_t* dst, scrntype8_t data, const size_t num)
{
	__UNLIKELY_IF(num == 0) {
		return;
	}
	__UNLIKELY_IF(num >= 8) {
		store8_unaligned(dst, data);
		return;
	}
	for(size_t i = 0; i < num; i++) {
		dst[i] = data.s[i];
	}
}

inline scrntype8_t TOWNS_CRTC::zero_scrntype8_t()
{
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _r;
	#if defined(_RGB555) || defined(_RGB565)
	_r.v = simde_mm_xor_ps(_r.v, _r.v);
	#else
	_r.v = simde_mm256_xor_ps(_r.v, _r.v);
	#endif
	return _r;
}

inline void TOWNS_CRTC::store8_pix(const scrntype_t *dst, scrntype8_t data)
{
	if(is_align_scrntype8_constexpr(dst)) {
		store8_aligned(dst, data);
	} else {
		store8_unaligned(dst, data);
	}
}
	
inline void TOWNS_CRTC::store_pix(scrntype_t *dst, scrntype8_t data, const size_t words)
{
	store8_limited(dst, data, words);
}

inline scrntype8_t TOWNS_CRTC::load8_aligned(scrntype_t* dst)
{
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _ret;
	#if defined(_RGB555) || defined(_RGB565)
	_ret.v =  simde_mm_load_ps((const float*)src);
	#else
	_ret.v = simde_mm256_load_ps((const float*)src);
	#endif
	return _ret;
}

inline scrntype8_t TOWNS_CRTC::load8_unaligned(scrntype_t* src)
{
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _ret;
	SIMDE_VECTORIZE
	for(size_t i = 0; i < 8; i++) {
		_ret.s[i] = src[i];
	}
	return _ret;
}

inline scrntype8_t TOWNS_CRTC::load8_limited(scrntype_t* src, size_t num)
{
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _ret;
	__UNLIKELY_IF(num == 0) {	
		_ret = zero_scrntype8_t();
		return _ret;
	}
	__UNLIKELY_IF(num >= 8) {
		return load8_unaligned(src);
		return;
	}
	_ret = zero_scrntype8_t();
	for(size_t i = 0; i < num; i++) {
		_ret.s[i] = src[i];
	}
	return _ret;
}

inline scrntype8_t TOWNS_CRTC::load8_pix(const scrntype_t *src)
{
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _ret;
	
	if(is_align_scrntype8_constexpr(src) {
		_ret = load8_aligned(src);
	} else {
		_ret = load8_unaligned(src);
	}
	return _ret;
}

inline scrntype8_t TOWNS_CRTC::load_pix(scrntype_t *src, size_t words)
{
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _ret;
	_ret = load8_limited(src, words);
	return _ret;
}

inline void TOWNS_CRTC::simd_fill(scrntype_t* dst, scrntype8_t data, size_t words)
{
	const uintptr_t pdst = (uintptr_t)dst;
	const size_t width_8 = sizeof(scrntype_t) * 8;
	const size_t mask_8 = ~width_8;
	__LIKELY_IF(words > 7) {
		__LIKELY_IF((pdst & mask_8) == 0) {
			for(size_t xx = 0; xx < words; xx += 8) {
				store8_aligned(&(dst[xx]), data);
			}
		} else {
			for(size_t xx = 0; xx < words; xx += 8) {
				store8_unaligned(&(dst[xx]), data);
			}
		}
	}
	if((words & 7) != 0) {
		size_t xx = words & (~7);
		store8_limited(&(dst[xx]), data, words & 7);
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
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _tmp;
	__LIKELY_IF(is_align_scrntype8(dst)) {
		for(size_t x = 0; (x < words) && (width >= 8) ; x++) {
			_tmp = load8_pix(&(src[x]));
			store8_aligned(dst, _tmp);
			dst += 8;
			width -= 8;
			pixels_count += 8;
			xx++;
		}
	} else {
		for(size_t x = 0; (x < words) && (width >= 8) ; x++) {
			_tmp = load8_pix(&(src[x]));
			store8_unaligned(dst, _tmp);
			dst += 8;
			width -= 8;
			pixels_count += 8;
			xx++;
		}
	}
	__UNLIKELY_IF((width > 0) && (xx < words)) {
		_tmp = src[xx];
		store8_limited(dst, _tmp, width);
		pixels_count += width;
		width = 0;
	}
	return pixels_count;
}


inline void pix_multiply_x2(scrntype8_t dst[2], const scrntype8_t data)
{
	SIMDE_VECTORIZE
	for(size_t lx = 0; lx < 8; lx++) {
		dst[0].s[lx] = data.s[lx >> 1]; 
	}
	SIMDE_VECTORIZE
	for(size_t rx = 8; rx < 16; rx++) {
		dst[1].s[rx - 8] = data.s[rx >> 1]; 
	}
	
}

inline size_t TOWNS_CRTC::store_x2(scrntype_t *dst, const scrntype8_t *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _s;
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _tmp[2];
	const bool is_dst_aligned = is_align_scrntype8(dst);
	size_t xx = 0;
	for(size_t x = 0; (x < words) && (width >=16) ; x++) {
		_s = load8_pix(&(src[x]));
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
		_s = load8_pix(&(src[xx]));
		pix_multiply_x2(_tmp, _s);
		if(width > 7) {
			store8_aligned(&(dst[0]), _tmp[0]);
			store8_limited(&(dst[8]), _tmp[1], width - 8);
		} else {
			store8_limited(dst, _tmp[0], width);
		}
		pixels_count += width;
		width = 0;
	}
	return pixels_count;
}

inline void pix_multiply_x4(scrntype8_t dst[4], const scrntype8_t data)
{
	SIMDE_VECTORIZE
	for(size_t lx = 0; lx < 8; lx++) {
		dst[0].s[lx] = data.s[lx >> 2]; 
	}
	SIMDE_VECTORIZE
	for(size_t x1 = 8; x1 < 16; x1++) {
		dst[1].s[x1 - 8] = data.s[x1 >> 2]; 
	}
	SIMDE_VECTORIZE
	for(size_t x2 = 16; x2 < 24; x2++) {
		dst[2].s[x2 - 16] = data.s[x2 >> 2]; 
	}
	SIMDE_VECTORIZE
	for(size_t x3 = 24; x3 < 32; x3++) {
		dst[3].s[x3 - 24] = data.s[x3 >> 2]; 
	}
	
}

inline size_t TOWNS_CRTC::store_x4(scrntype_t *dst, const scrntype8_t *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _s;
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _tmp[4];
	size_t xx = 0;
	const bool is_dst_aligned = is_align_scrntype8(dst);
	
	for(size_t x = 0; (x < words) && (width >= 32) ; x++) {
		_s = load8_pix(&(src[x]));
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
		_s = load8_pix(&(src[xx]));
		pix_multiply_x4(_tmp, _s);
		size_t x2 = 0;
		for(size_t w = width; w >= 8; w -= 8) {
			store8_pix(dst, _tmp[x2]);
			dst += 8;
			pixels_count += 8;
			x2++;
		}
		if((width & 7) != 0) {
			store_pix(dst, _tmp[x2], width & 7);
			pixels_count += (width & 7);
		}
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
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _s;
	size_t xx = 0;
	const bool is_dst_aligned = is_align_scrntype8(dst);
	
	size_t dst_lp1 = 0;
	size_t dst_lp2 = 0;
	__LIKELY_IF(mag <= 16) {
		TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _tmp[16];
		const TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _tmp_zero = zero_scrntype8_t();
		for(size_t x = 0; (x < words) && (width > 0) ; x++) {
			_s = load8_pix(&(src[x]));
			for(size_t n = 0; n < mag; n++) {
				_tmp[n] = _tmp_zero;
			}
			// Multiply
			dst_lp1 = 0;
			dst_lp2 = 0;
			for(size_t x2 = 0; x2 < 8; x2++) {
				for(size_t n = 0; n < mag; n++) {
					_tmp[dst_lp1].s[dst_lp2] = _s.s[x2];
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
				} else {
					store_pix(dst, _tmp[xr], width);
					dst += width;
					width = 0;
				}
			}
		}
	} else {
		for(size_t x = 0; (x < words) && (width > 0) ; x++) {
			_s = load8_pix(&(src[x]));
			for(size_t m = 0; m < 8; m++) {
				for(size_t n = 0; n < mag; n++) {
					*dst++ = _s.s[m];
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
	TOWNS_CRTC_SCRNTYPE8_ALIGN scrntype8_t _s;
	size_t pixels_count = 0;
	const size_t width_limit = width;
	size_t rwidth = width & 7;
	size_t __sum = 0;
	SIMDE_VECTORIZE
	for(size_t x = 0; x < 8; x++) {
		__sum += magx_map.u16[x];
	}
	__UNLIKELY_IF(__sum == 0) {
		return 0;
	}
	scrntype_t pix;
	size_t ptr = 0;
	for(size_t x = 0; x < words; x++) {
		__UNLIKELY_IF(pixels_count >= width_limit ) {
			pixels_count = width_limit;
			break;
		}
		__LIKELY_IF(width >= __sum) {
			_s = load8_pix(&(src[x]));
			SIMDE_VECTORIZE
			for(size_t i = 0; i < 8; i++) {
				pix = _s.s[i];
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
			_s = load8_pix(&(src[x]));
			for(size_t i = 0; i < rwidth; i++) {
				pix = _s.s[i];
				for(size_t j = 0; j < magx_map.u16[i]; j++) {
					dst[ptr++] = pix;
					pixels_count++;
					__UNLIKELY_IF(pixels_count >= width_limit) {
						break;
					}
				}
				__UNLIKELY_IF(pixels_count >= width_limit) {
					pixels_count = width_limit;
					break;
				}
			}
			width = 0;
			break;
		}
	}
	return pixels_count;
}

}
#endif /* _TOWNS_CRTC_UTILS_H_ */
#endif /* _TOWNS_CRTC_H_ */
