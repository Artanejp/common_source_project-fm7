#pragma once

/*!
  @todo will move to another directory.
*/
#include "./basic_types.h"

// hint for SIMD
#if defined(__clang__)
	#define __DECL_VECTORIZED_LOOP   _Pragma("clang loop vectorize(enable) distribute(enable)")
#elif defined(__GNUC__)
	#define __DECL_VECTORIZED_LOOP	_Pragma("GCC ivdep")
#else
	#define __DECL_VECTORIZED_LOOP
#endif

#if defined(__cplusplus) && (__cplusplus >= 202002L)
	#include <version>
#endif
// 20181104 K.O:
// Below routines aim to render common routine.
#if defined(__cplusplus) && (__cplusplus >= 201103L)
	#define __DECL_ALIGNED(foo) alignas(foo)
#elif defined(_MSC_VER)
	#define __DECL_ALIGNED(foo) __declspec(align(foo))
#elif defined(__GNUC__)
	// C++ >= C++11
	#define __DECL_ALIGNED(foo) __attribute__((aligned(foo)))
#else
	// ToDo
	#define __DECL_ALIGNED(foo)
#endif

#if defined(__cplusplus) && (__cplusplus >= 202002L)
	#include <memory>
	#if __cpp_lib_assume_aligned
		#define ___assume_aligned(foo, a) std::assume_aligned<a>(foo)
	#else
		#define ___assume_aligned(foo, a) __builtin_assume_aligned(foo, a)
	#endif
#elif _MSC_VER
	#ifndef __builtin_assume_aligned
		#define ___assume_aligned(foo, a) foo
	#else
		#define ___assume_aligned(foo, a) __builtin_assume_aligned(foo, a)
	#endif
#else
	#if defined(__has_builtin)
		#if (__has_builtin(__builtin_assume_aligned))
			#define ___assume_aligned(foo, a) __builtin_assume_aligned(foo, a)
		#else
			#define ___assume_aligned(foo, a) foo
		#endif
	#else
			#define ___assume_aligned(foo, a) foo
	#endif
#endif

// hint for branch-optimize. 20210720 K.O
// Usage:
// __LIKELY_IF(expr) : Mostly (expr) will be effected.
// __UNLIKELY_IF(expr) : Mostly (expr) will not be effected.
#undef __LIKELY_IF
#undef __UNLIKELY_IF

#if defined(__cplusplus) && (__cplusplus >= 202000L) && defined(__has_cpp_attribute)
	#if (__has_cpp_attribute(likely))
		#define __LIKELY_IF(foo) if(foo) [[likely]]
	#endif
	#if (__has_cpp_attribute(unlikely))
		#define __UNLIKELY_IF(foo) if(foo) [[unlikely]]
	#endif
#endif

#if !defined(__LIKELY_IF) || !defined(__UNLIKELY_IF)
	#if defined(__has_builtin)
		#if (__has_builtin(__builtin_expect))
			#define __LIKELY_IF(foo) if(__builtin_expect((foo), 1))
			#define __UNLIKELY_IF(foo) if(__builtin_expect((foo), 0))
		#else
			#define __LIKELY_IF(foo) if(foo)
			#define __UNLIKELY_IF(foo) if(foo)
		#endif
	#else
			// Fallthrough: maybe not have __builtin_expect()
			#define __LIKELY_IF(foo) if(foo)
			#define __UNLIKELY_IF(foo) if(foo)
	#endif
#endif


#undef __TMP_HAS_PREFETCH
#undef __TMP_HAS_CLEAR_CACHE
#undef __TMP_CACHE_SIZE

#if !defined(__CACHE_LINE_SIZE)
	#define __TMP_CACHE_SIZE 32
#else
	#define __TMP_CACHE_SIZE __CACHE_LINE_SIZE
#endif
#if defined(__cplusplus) && (__cplusplus >= 202002L)
	#if defined(__has_builtin)
		#if __has_builtin(__builtin_prefetch)
			#define __TMP_HAS_PREFETCH 1
		#endif
		#if __has_builtin(__builtin___clear_cache)
		#define __TMP_HAS_CLEAR_CACHE 1
		#endif
	#endif
	#if defined(__TMP_HAS_PREFETCH)
		inline void make_prefetch(void *p, const size_t bytes)
		{
			uint8_t *q = (uint8_t*)p;
			for(size_t i = 0; i <= (bytes / __TMP_CACHE_SIZE); i += __TMP_CACHE_SIZE) {
				__builtin_prefetch(&(q[i]));
			}
		}
		inline void make_prefetch_local(void *p, const size_t bytes)
		{
			uint8_t *q = (uint8_t*)p;
			for(size_t i = 0; i <= (bytes / __TMP_CACHE_SIZE); i += __TMP_CACHE_SIZE) {
				__builtin_prefetch(&(q[i]), 0, 2);
			}
		}
		inline void make_prefetch_volatile(void *p, const size_t bytes)
		{
			uint8_t *q = (uint8_t*)p;
			for(size_t i = 0; i <= (bytes / __TMP_CACHE_SIZE); i += __TMP_CACHE_SIZE) {
				__builtin_prefetch(&(q[i]), 0, 0);
			}
		}
		inline void make_prefetch_read_local(void *p, const size_t bytes)
		{
			uint8_t *q = (uint8_t*)p;
			for(size_t i = 0; i <= (bytes / __TMP_CACHE_SIZE); i += __TMP_CACHE_SIZE) {
				__builtin_prefetch(&(q[i]), 0, 2);
			}
		}
		inline void make_prefetch_write_local(void *p, const size_t bytes)
		{
			uint8_t *q = (uint8_t*)p;
			for(size_t i = 0; i <= (bytes / __TMP_CACHE_SIZE); i += __TMP_CACHE_SIZE) {
				__builtin_prefetch(&(q[i]), 1, 2);
			}
		}
		inline void make_prefetch_read_volatile(void *p, const size_t bytes)
		{
			uint8_t *q = (uint8_t*)p;
			for(size_t i = 0; i <= (bytes / __TMP_CACHE_SIZE); i += __TMP_CACHE_SIZE) {
				__builtin_prefetch(&(q[i]), 0, 2);
			}
		}
		inline void make_prefetch_write_volatile(void *p, const size_t bytes)
		{
			uint8_t *q = (uint8_t*)p;
			for(size_t i = 0; i <= (bytes / __TMP_CACHE_SIZE); i += __TMP_CACHE_SIZE) {
				__builtin_prefetch(&(q[i]), 1, 0);
			}
		}
	#else
		inline void make_prefetch(void *p, const size_t bytes) {}
		inline void make_prefetch_local(void *p, const size_t bytes) {}
		inline void make_prefetch_volatile(void *p, const size_t bytes) {}
		inline void make_prefetch_read_local(void *p, const size_t bytes) {}
		inline void make_prefetch_write_local(void *p, const size_t bytes) {}
		inline void make_prefetch_read_volatile(void *p, const size_t bytes) {}
		inline void make_prefetch_write_volatile(void *p, const size_t bytes) {}
	#endif
	#if defined(__TMP_HAS_CLEAR_CACHE)
		inline void flush_cache(void *p, const size_t bytes)
		{
			__LIKELY_IF(bytes > 0) {
				uint8_t* q = (uint8_t*)p;
				__builtin___clear_cache((char *)p, (char *)(&(q[bytes - 1])));
			}
		}
	#else
		inline void flush_cache(void *p, const size_t bytes) {}
	#endif
#else /* ToDo: Cache feature with MSVC */
	inline void make_prefetch(void *p, const size_t bytes) {}
	inline void make_prefetch_local(void *p, const size_t bytes) {}
	inline void make_prefetch_volatile(void *p, const size_t bytes) {}
	inline void make_prefetch_read_local(void *p, const size_t bytes) {}
	inline void make_prefetch_write_local(void *p, const size_t bytes) {}
	inline void make_prefetch_read_volatile(void *p, const size_t bytes) {}
	inline void make_prefetch_write_volatile(void *p, const size_t bytes) {}
	inline void flush_cache(void *p, const size_t bytes) {}
#endif

#undef __TMP_CACHE_SIZE
#undef __TMP_HAS_PREFETCH
#undef __TMP_HAS_CLEAR_CACHE

	
inline uint16_t swapendian_16(uint16_t src)
{
#if defined(__HAS_BUILTIN_BSWAP16_X)
	uint16_t n = __builtin_bswap16((uint16_t)src);
	return n;
#else
	union {
		uint16_t w;
		uint8_t  b[2];
	} n;
	n.w = src;
	std::swap(n.b[0], n.b[1]);
	return n.w;
#endif
}

inline uint32_t swapendian_32(uint32_t src)
{
#if defined(__HAS_BUILTIN_BSWAP32_X)
	uint32_t n = __builtin_bswap32(src);
	return n;
#else
	union {
		uint32_t d;
		uint8_t  b[4];
	} n;
	n.d = src;
	std::swap(n.b[0], n.b[3]);
	std::swap(n.b[1], n.b[2]);
	return n.d;
#endif
}

inline uint64_t swapendian_64(uint64_t src)
{
#if defined(__HAS_BUILTIN_BSWAP64_X)
	uint64_t n = __builtin_bswap64((uint64_t)src);
	return n;
#else
	__DECL_ALIGNED(8) union {
		uint64_t l;
		uint8_t  b[8];
	} n;
	n.l = src;
	std::swap(n.b[0], n.b[7]);
	std::swap(n.b[1], n.b[6]);
	std::swap(n.b[2], n.b[5]);
	std::swap(n.b[3], n.b[4]);
	return n.l;
#endif
}

#if defined(__HAS_BUILTIN_BSWAP128_X)
inline __uint128_t swapendian_128(__uint128_t src)
{
	__uint128_t n = __builtin_bswap128(src);
	return n;
}
#endif

template <class T>
	inline T swapendian_T(T src)
{
	constexpr size_t __size = sizeof(T);

	if(__size <= 1) {
		return src;
	}
	if(__size == 16) {
	#if defined(__HAS_BUILTIN_BSWAP128_X)
		return (T)swapendian_128((__uint128_t)src);
	#else
		__DECL_ALIGNED(16) union {
			T        dat;
			uint8_t  b[16];
		} n;
		n.dat = src;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0, j = 15; i < 8; i++, j--) {
			std::swap(n.b[i], n.b[j]);
		}
		return n.dat;
	#endif
	}
	if(__size == 32) {
		__DECL_ALIGNED(16) union {
			T        dat;
			uint8_t  b[32];
		} n;
		n.dat = src;
		__DECL_VECTORIZED_LOOP
		for(size_t i = 0, j = 31; i < 16; i++, j--) {
			std::swap(n.b[i], n.b[j]);
		}
		return (T)(n.dat);
	}
	__DECL_ALIGNED(32) T tmp = src;
	uint8_t *p = (uint8_t*)(&tmp);
	__DECL_VECTORIZED_LOOP
	for(size_t i = 0, j = __size - 1; i < (__size / 2); i++, j--) {
		std::swap(p[i], p[j]);
	}
	return tmp;
}
	


