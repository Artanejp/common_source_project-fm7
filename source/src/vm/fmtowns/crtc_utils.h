#ifdef _TOWNS_CRTC_H_ /* You must include crtc.h before. */
#ifndef _TOWNS_CRTC_UTILS_H_ /* And this must be included at once. */
#define _TOWNS_CRTC_UTILS_H_ 

inline void TOWNS_CRTC::simd_fill(scrntype_t* dst, csp_vector8<scrntype_t> data, size_t words)
{
	const uintptr_t pdst = (uintptr_t)dst;
	const size_t width_8 = sizeof(scrntype_t) * 8;
	const size_t mask_8 = ~width_8;
	__LIKELY_IF(words > 7) {
		__LIKELY_IF((pdst & mask_8) == 0) {
			for(size_t xx = 0; xx < words; xx += 8) {
				data.store_aligned(&(dst[xx]));
			}
		} else {
			for(size_t xx = 0; xx < words; xx += 8) {
				data.store(&(dst[xx]));
			}
		}
	}
	if((words & 7) != 0) {
		size_t xx = words & (~7);
		data.store_limited(&(dst[xx]), words & 7);
	}
}

inline void TOWNS_CRTC::simd_copy(scrntype_t* dst, scrntype_t* src, size_t words)
{
	const uintptr_t pdst = (uintptr_t)dst;
	const uintptr_t psrc = (uintptr_t)src;
	const size_t width_8 = sizeof(scrntype_t) * 8;
	const size_t mask_8 = ~width_8;
	csp_vector8<scrntype_t> pix;
	__LIKELY_IF(words > 7) {
		__LIKELY_IF((psrc & mask_8) == 0) { // Aligned
			if((pdst & mask_8) == 0) { // Aligned
				for(size_t xx = 0; xx < words; xx += 8) {
					pix.load_aligned(&(src[xx]));
					pix.store_aligned(&(dst[xx]));
				}
			} else {
				for(size_t xx = 0; xx < words; xx += 8) {
					pix.load_aligned(&(src[xx]));
					pix.store(&(dst[xx]));
				}
			}
		} else __LIKELY_IF((pdst & mask_8) == 0) { // DST ONLY ALIGNED
				for(size_t xx = 0; xx < words; xx += 8) {
					pix.load(&(src[xx]));
					pix.store_aligned(&(dst[xx]));
				}
			} else {
			for(size_t xx = 0; xx < words; xx += 8) {
				pix.load(&(src[xx]));
				pix.store(&(dst[xx]));
			}
		}
	}
	if((words & 7) != 0) {
		size_t xx = words & 0xfffffff8;
		size_t w = words & 7;
		pix.load_limited(&(src[xx]), w);
		pix.store_limited(&(dst[xx]), w);
	}
}

inline size_t TOWNS_CRTC::store1_aligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= 8) {
			src[x].store_aligned(dst);
			dst += 8;
			width -= 8;
			pixels_count += 8;
		} else {
			src[x].store_limited(dst, width);
			dst += width;
			pixels_count += width;
			width = 0;
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::store2_aligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= 16) {
			src[x].store2_aligned(dst);
			dst += 16;
			width -= 16;
			pixels_count += 16;
		} else {
			src[x].store2_limited(dst, width);
			dst += (2 * width);
			pixels_count += (2 * width);
			width = 0;
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::store4_aligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= 32) {
			src[x].store4_aligned(dst);
			dst += 32;
			width -= 32;
			pixels_count += 32;
		} else {
			src[x].store4_limited(dst, width);
			dst += (width * 4);
			pixels_count += (width * 4);
			width = 0;
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::store_n_any(scrntype_t *dst, csp_vector8<scrntype_t> *src, const int mag, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= (8 * mag)) {
			src[x].store_n(dst, mag);
			dst += (8 * mag);
			width -= (8 * mag);
			pixels_count += (8 * mag);
		} else {
			src[x].store_n_limited(dst, mag, width);
			dst += (width * mag);
			pixels_count += (width * mag);
			width = 0;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::store1_unaligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= 8) {
			src[x].store(dst);
			dst += 8;
			width -= 8;
			pixels_count += 8;
		} else {
			src[x].store_limited(dst, width);
			dst += width;
			pixels_count += width;
			width = 0;
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::store2_unaligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= 16) {
			src[x].store2(dst);
			dst += 16;
			width -= 16;
			pixels_count += 16;
		} else {
			src[x].store2_limited(dst, width);
			dst += (2 * width);
			pixels_count += (2 * width);
			width = 0;
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::store4_unaligned(scrntype_t *dst, csp_vector8<scrntype_t> *src, const size_t words, size_t& width)
{
	size_t pixels_count = 0;
	for(size_t x = 0; (x < words) && (width > 0) ; x++) {
		__LIKELY_IF(width >= 32) {
			src[x].store4(dst);
			dst += 32;
			width -= 32;
			pixels_count += 32;
		} else {
			src[x].store4_limited(dst, width);
			dst += (width * 4);
			pixels_count += (width * 4);
			width = 0;
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::scaling_store(scrntype_t *dst, csp_vector8<scrntype_t> *src, const int mag, const size_t words, size_t& width)
{
	__UNLIKELY_IF((dst == NULL) || (src == NULL)) return 0;

	uintptr_t dstval = (uintptr_t)dst;
	size_t pixels_count = 0;
	const uintptr_t as = alignof(csp_vector8<scrntype_t>) - 1;
	__LIKELY_IF((dstval & as) == 0) { // ALIGNED
		switch(mag) {
		case 1:
			pixels_count = store1_aligned(dst, src, words, width);
			break;
		case 2:
			pixels_count = store2_aligned(dst, src, words, width);
			break;
		case 4:
			pixels_count = store4_aligned(dst, src, words, width);
			break;
		default:
			pixels_count = store_n_any(dst, src, mag, words, width);
			break;
		}
	} else { // Not aligned
		switch(mag) {
		case 1:
			pixels_count = store1_unaligned(dst, src, words, width);
			break;
		case 2:
			pixels_count = store2_unaligned(dst, src, words, width);
			break;
		case 4:
			pixels_count = store4_unaligned(dst, src, words, width);
			break;
		default:
			pixels_count = store_n_any(dst, src, mag, words, width);
			break;
		}
	}
	return pixels_count;
}

inline size_t TOWNS_CRTC::scaling_store_by_map(scrntype_t *dst, csp_vector8<scrntype_t> *src, csp_vector8<uint16_t> magx_map, const size_t words, size_t& width)
{
	if((words == 0) || (width == 0)) return 0;
	size_t pixels_count = 0;
	const size_t width_limit = width;
	size_t rwidth = width & 7;
	size_t __sum = 0;
	__DECL_VECTORIZED_LOOP
	for(size_t x = 0; x < 8; x++) {
		__sum += magx_map.at(x);
	}
	scrntype_t pix;
	size_t ptr = 0;
	for(size_t x = 0; x < words; x++) {
		__UNLIKELY_IF(pixels_count >= width_limit ) {
			pixels_count = width_limit;
			break;
		}
		__LIKELY_IF(width >= __sum) {
			__DECL_VECTORIZED_LOOP
			for(size_t i = 0; i < 8; i++) {
				pix = src[x].at(i);
				for(size_t j = 0; j < magx_map.at(i); j++) {
					dst[ptr++] = pix; 
				}
			}
			width -= __sum;
			pixels_count += __sum;
		} else {
			__UNLIKELY_IF(rwidth == 0) {
				rwidth = 8; // Temporally value
			}
			for(size_t i = 0; i < rwidth; i++) {
				pix = src[x].at(i);
				for(size_t j = 0; j < magx_map.at(i); j++) {
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
	
#endif /* _TOWNS_CRTC_UTILS_H_ */
#endif /* _TOWNS_CRTC_H_ */
