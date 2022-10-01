#pragma once
#include "./osd_sound_mod_consts.h"

#include <limits>
namespace SOUND_MODULE {
/* SOUND_MODULE */
	
	static inline const __BYTEORDER get_system_byteorder()
	{
		#if __LITTLE_ENDIAN__
		return __BYTEORDER::Little;
		#else
		return __BYTEORDER::Big;
		#endif
	}
	inline bool check_attribute(sound_attribute a)
	{
		if((a.word_size == 0) || (a.word_size > 16)) return false;
		if((a.channels == 0) || (a.channels > 8)) return false;
		return true;
	}
	
	inline bool compare_attribute(sound_attribute src, sound_attribute dst)
	{
		bool _b = true;
		_b &= (src.format == dst.format);
		_b &= (src.endian == dst.endian);
		_b &= (src.word_size == dst.word_size);
		_b &= (src.channels == dst.channels);
		return _b;
	}

	
	template <typename T>
	size_t swap_endian(T* src, T* dst, size_t words)
	{
		if(words == 0) return 0;
		const size_t wordsize = sizeof(T);
		
		typedef union _t_pair_t {
			T		data;
			uint8_t	u8[sizeof(T)];
		};
		T* p = src;
		T* q = dst;
		
		__DECL_ALIGNED(16) _t_pair_t tmpbuf[8];
		__DECL_ALIGNED(16) _t_pair_t dstbuf[8];
		const size_t major_words = words / 8;
		const size_t minor_words = words % 8;
		
		for(size_t i = 0; i < major_words; i++) {
		__DECL_VECTORIZED_LOOP
			for(size_t j = 0; j < 8; j++) {
				tmpbuf[j].data = p[j];
			}

		__DECL_VECTORIZED_LOOP
			for(size_t j = 0; j < 8; j++) {
			__DECL_VECTORIZED_LOOP
				for(size_t k = 0; k < sizeof(T); k++) {
					dstbuf[j].u8[k] = tmpbuf[j].u8[sizeof(T) - k - 1];
				}
			}
			
		__DECL_VECTORIZED_LOOP
			for(size_t j = 0; j < 8; j++) {
				q[j] = dstbuf[j].data;
			}
			q += 8;
			p += 8;
		}
		_t_pair_t __tmp;
		_t_pair_t __dst;
		for(size_t i = 0; i < minor_words; i++) {
			__tmp.data = p[i];
			for(size_t k = 0; k < sizeof(T); k++) {
				__dst.u8[k] = __tmp.u8[sizeof(T) - k - 1]; 
			}
			q[i] = __dst.data;
		}
		return words;
	}
	template <>
	size_t swap_endian(uint8_t* src, uint8_t* dst, size_t words)
	{
		if(words == 0) return 0;
		if((uintptr_t)src == (uintptr_t)dst) return words;
		memcpy(dst, src, words);
		return words;
	}	
	template <>
	size_t swap_endian(int8_t* src, int8_t* dst, size_t words)
	{
		if(words == 0) return 0;
		if((uintptr_t)src == (uintptr_t)dst) return words;
		memcpy(dst, src, words);
		return words;
	}
	
	/* convert format for both of integer variants */
	template <typename S, typename D>
	size_t convert_format(D* dst, S* src, size_t words)
	{
		if(dst == nullptr) return 0;
		if(src == nullptr) return 0;
		if(words == 0) return 0;

		std::numeric_limits<S> src_limit;
		std::numeric_limits<D> dst_limit;
		size_t major_nwords = words / 8;
		size_t minor_words  = words % 8;
		
		enum {
			src_false = 0,
			src_true  = 1,
			dst_false = 0,
			dst_true  = 2
		};

		uint8_t type_is_int =
			(std::numeric_limits<S>::is_exact() ? src_true : src_false) |
			(std::numeric_limits<D>::is_exact() ? dst_true : dst_false);
		
		S* p = src;
		D* q = dst;
		__DECL_ALIGNED(16) S srcbuf[8];
		__DECL_ALIGNED(16) D dstbuf[8];
		
		switch(type_is_int) {
		case (src_false | dst_false):
			// Both float or double
			{
				for(size_t i = 0; i < major_nwords; i++) {
				__DECL_VECTORIZED_LOOP
					for(size_t j = 0; j < 8; j++) {
						srcbuf[j] = p[j];
					}
				__DECL_VECTORIZED_LOOP
					for(size_t j = 0; j < 8; j++) {
						dstbuf[j] = (D)(srcbuf[j]);
					}
				__DECL_VECTORIZED_LOOP
					for(size_t j = 0; j < 8; j++) {
						q[j] = dstbuf[j];
					}
					p += 8;
					q += 8;
				}
				for(size_t j = 0; j < minor_words; j++) {
					q[j] = (D)(p[j]);
				}
			}
			break;
		case (src_true | dst_false):
			// src is signed or unsigned int, dst is float or double
			{
				const D tmp_src_max = (D)(src_limit.max());
				__DECL_ALIGNED(16) const D src_max[8] = {tmp_src_max};
				__DECL_ALIGNED(16) D srcbuf_2[8];
				
				if(std::numeric_limits<S>::is_signed()) {
					// Signed
					for(size_t i = 0; i < major_nwords; i++) {
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							srcbuf[j] = p[j];
						}
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							srcbuf_2[j] = (D)(srcbuf[j]);
						}
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							dstbuf[j] = srcbuf_2[j] / src_max[j];
						}
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							q[j] = dstbuf[j];
						}
						p += 8;
						q += 8;
					}
					for(size_t j = 0; j < minor_words; j++) {
						D _tmp = (D)(p[j]);
						_tmp = _tmp / tmp_src_max;
						q[j] = _tmp;
					}
				} else {
					// Unsigned
					for(size_t i = 0; i < major_nwords; i++) {
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							srcbuf[j] = p[j];
						}
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							srcbuf_2[j] = (D)(srcbuf[j]);
						}
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							dstbuf[j] = (srcbuf_2[j] / src_max[j]) - 0.5;
						}
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							q[j] = dstbuf[j];
						}
						p += 8;
						q += 8;
					}
					for(size_t j = 0; j < minor_words; j++) {
						D _tmp = (D)(p[j]);
						_tmp = (_tmp / tmp_src_max) - 0.5;
						q[j] = _tmp;
					}
				}
			}
			break;
		case (src_false | dst_true):
			//  src is float or double, dst is signed or unsigned int
			{
				const S tmp_dst_max = (S)(dst_limit.max()) + 1.0;
				__DECL_ALIGNED(16) const S dst_max[8] = {tmp_dst_max};
		
				if(std::numeric_limits<S>::is_signed()) {
					// Signed
					for(size_t i = 0; i < major_nwords; i++) {
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							srcbuf[j] = p[j];
						}
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							srcbuf[j] = srcbuf[j] * dst_max[j];
						}
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							dstbuf[j] = (D)(srcbuf[j]);
						}
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							q[j] = dstbuf[j];
						}
						p += 8;
						q += 8;
					}
					for(size_t j = 0; j < minor_words; j++) {
						S _tmp = p[j];
						_tmp = _tmp * tmp_dst_max;
						q[j] = (D)_tmp;
					}
				} else {
					// Unsigned
					for(size_t i = 0; i < major_nwords; i++) {
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							srcbuf[j] = p[j];
						}
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							srcbuf[j] = (srcbuf[j] + 0.5)  * dst_max[j];
						}
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							dstbuf[j] = (D)(srcbuf[j]);
						}
					__DECL_VECTORIZED_LOOP
						for(size_t j = 0; j < 8; j++) {
							q[j] = dstbuf[j];
						}
						p += 8;
						q += 8;
					}
					for(size_t j = 0; j < minor_words; j++) {
						S _tmp = p[j];
						_tmp = (_tmp + 0.5) * tmp_dst_max;
						q[j] = (D)_tmp;
					}
				}
			}
			break;
		default:
			//  both src and dst are signed or unsigned int
			{
				uint8_t type_is_signed =
					(std::numeric_limits<S>::is_signed() ? src_true : src_false) |
					(std::numeric_limits<D>::is_signed() ? dst_true : dst_false);
				
				const ssize_t src_bit_width = sizeof(S) << 3; 
				const ssize_t dst_bit_width = sizeof(D) << 3;
				const ssize_t bit_width_diff = dst_bit_width - src_bit_width;
				
				const S tmp_src_max = src_limit.max();
				const D tmp_dst_max = dst_limit.max();
				const D tmp_bitfill = (bit_width_diff > 0) ? ((((D)1) << src_bit_width) - 1) & tmp_dst_max : 0;

				D tmp_offset;
				switch(type_is_signed) {
				case (src_true | dst_false): // signed -> unsigned
					tmp_offset = (D)tmp_src_max + 1;
					break;
				case (src_false | dst_true): // unsigned -> signed
					tmp_offset = -((D)tmp_src_max + 1);
					break;
				default:
					// unsigned -> unsigned
					// signed -> signed
					tmp_offset = 0;
					break;
				}
				if(bit_width_diff <= 0) {
					tmp_offset = 0;
				}
				if((tmp_offset == 0) && (bit_width_diff == 0)) {
					// Same signess && Same bitwidth
					memdpy(dst, src, words * sizeof(D));
					return words;
				}
				__DECL_ALIGNED(16) const D bitfill[8] = {tmp_bitfill};
				__DECL_ALIGNED(16) const D _offset[8] = {tmp_offset};
				
				for(size_t i = 0; i < major_nwords; i++) {
				__DECL_VECTORIZED_LOOP
					for(size_t j = 0; j < 8; j++) {
						srcbuf[j] = p[j];
					}
				__DECL_VECTORIZED_LOOP
					for(size_t j = 0; j < 8; j++) {
						dstbuf[j] = (D)(srcbuf[j]);
					}
				__DECL_VECTORIZED_LOOP
					for(size_t j = 0; j < 8; j++) {
						dstbuf[j] += _offset[j];
					}
				__DECL_VECTORIZED_LOOP
					for(size_t j = 0; j < 8; j++) {
						dstbuf[j] <<= bit_width_diff;
					}
				__DECL_VECTORIZED_LOOP
					for(size_t j = 0; j < 8; j++) {
						dstbuf[j] |= bitfill[j];
					}
				__DECL_VECTORIZED_LOOP
					for(size_t j = 0; j < 8; j++) {
						q[j] = dstbuf[j];
					}
					p += 8;
					q += 8;
				}
				for(size_t j = 0; j < minor_words; j++) {
					D tmp = (D)(p[j]);
					tmp += tmp_offset;
					tmp <<= bit_width_diff;
					tmp |= tmp_bitfill;
					q[j] = tmp;
				}
			}
			break;
		}
		return words;
	}
	
/* SOUND_MODULE */
}
