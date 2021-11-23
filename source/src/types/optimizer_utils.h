#pragma once

/*!
  @todo will move to another directory.
*/

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
