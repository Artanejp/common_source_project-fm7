/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ common header ]
*/

#ifndef _COMMON_H_
#define _COMMON_H_

// move shared codes to DLL???
//#ifdef _USE_QT
//	#define USE_SHARED_DLL
//#endif

#ifdef _USE_QT
#include <SDL.h>
#endif

// use zlib to decompress gzip file???
#ifdef _WIN32
	#if defined(_MSC_VER) && (_MSC_VER >= 1500)
		#ifndef _ANY2D88
			#define USE_ZLIB
		#endif
	#endif
#endif
// check environemnt/language
#ifdef _WIN32
	#ifdef _MSC_VER
		// Microsoft Visual C++
		#if _MSC_VER == 1200
			// variable scope of 'for' loop for Microsoft Visual C++ 6.0
			#define for if(0);else for
		#endif
		#if _MSC_VER >= 1200
			// Microsoft Visual C++ 6.0 or later
			#define SUPPORT_TCHAR_TYPE
		#endif
		#if _MSC_VER >= 1400
			// Microsoft Visual C++ 8.0 (2005) or later
			#define SUPPORT_SECURE_FUNCTIONS
			#pragma warning( disable : 4819 )
			//#pragma warning( disable : 4995 )
			#pragma warning( disable : 4996 )
		#endif
		#if _MSC_VER >= 1800
			// Microsoft Visual C++ 12.0 (2013) or later
			#define SUPPORT_CPLUSPLUS_11
		#endif
		#define CSP_OS_WINDOWS
		#define __FASTCALL __fastcall
	#else
		// Win32, but not Microsoft Visual C++
		#define SUPPORT_TCHAR_TYPE
//		#define SUPPORT_SECURE_FUNCTIONS
	#endif
#endif
#ifdef __GNUC__
	#if defined(Q_OS_CYGWIN) 
		#define CSP_OS_GCC_CYGWIN
		#define CSP_OS_WINDOWS
	#elif defined(Q_OS_WIN) || defined(__WIN32) || defined(__WIN64)
		#define CSP_OS_GCC_WINDOWS
		#define CSP_OS_WINDOWS
		#ifdef USE_SHARED_DLL
			#define DLL_PREFIX   __declspec(dllexport)
			#define DLL_PREFIX_I __declspec(dllimport)
		#endif
	#else
		#define CSP_OS_GCC_GENERIC
		#define CSP_OS_GENERIC
		#define DLL_PREFIX
		#define DLL_PREFIX_I
	#endif
	#if defined(__i386__)
		#define __FASTCALL __fastcall
	#else
		#define __FASTCALL
	#endif
	#if defined(__clang__)
		#define __CSP_COMPILER_CLANG
	#else
		#define __CSP_COMPILER_GCC
	#endif
	#define SUPPORT_CPLUSPLUS_11
#else
		#define DLL_PREFIX
		#define DLL_PREFIX_I
#endif

#ifndef SUPPORT_CPLUSPLUS_11
	#if defined(__cplusplus) && (__cplusplus > 199711L)
		#define SUPPORT_CPLUSPLUS_11
	#endif
#endif
#ifndef SUPPORT_TCHAR_TYPE
	// secure functions need tchar type
	#undef SUPPORT_SECURE_FUNCTIONS
#endif
#ifndef DLL_PREFIX
	#define DLL_PREFIX
#endif
#ifndef DLL_PREFIX_I
	#define DLL_PREFIX_I
#endif

// include common header files
#ifdef SUPPORT_TCHAR_TYPE
	#include <tchar.h>
#endif
#ifdef SUPPORT_CPLUSPLUS_11
	#include <stdint.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#include <math.h>
#include <typeinfo.h>
#else
#include <typeinfo>
#endif
#include <assert.h>
#include <errno.h>

// include environment/language dependent header files
#ifdef _WIN32
	#include <windows.h>
	#include <windowsx.h>
	#include <winuser.h>
	#include <mmsystem.h>
	#include <process.h>
#endif
#ifdef __GNUC__
	#include <stdarg.h>
#endif
#ifdef _USE_QT
	#ifdef _USE_QT5
		#include <QString>
		#include <QFile>
		#include <QtEndian>
		#if defined(__MINGW32__) || (__MINGW64__)
			#include <windows.h>
			#include <winbase.h>
		#endif
	#else
		#include <QtCore/QString>
		#include <QtCore/QFile>
	#endif
	#include <sys/param.h>
#endif
#ifndef _MAX_PATH
	#define _MAX_PATH 2048
#endif

// endian
#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
	#if defined(__BYTE_ORDER) && (defined(__LITTLE_ENDIAN) || defined(__BIG_ENDIAN))
		#if __BYTE_ORDER == __LITTLE_ENDIAN
			#define __LITTLE_ENDIAN__
		#elif __BYTE_ORDER == __BIG_ENDIAN
			#define __BIG_ENDIAN__
		#endif
	#elif defined(SDL_BYTEORDER) && (defined(SDL_LIL_ENDIAN) || defined(SDL_BIG_ENDIAN))
		#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			#define __LITTLE_ENDIAN__
		#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
			#define __BIG_ENDIAN__
		#endif
	#elif defined(WORDS_LITTLEENDIAN)
		#define __LITTLE_ENDIAN__
	#elif defined(WORDS_BIGENDIAN)
		#define __BIG_ENDIAN__
	#endif
#endif
#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
	// may be Microsoft Visual C++
	#define __LITTLE_ENDIAN__
#endif

// type definition
#ifndef SUPPORT_TCHAR_TYPE
	#ifndef _TCHAR
		typedef char _TCHAR;
	#endif
#endif

// hint for SIMD
#if defined(__clang__)
	#define __DECL_VECTORIZED_LOOP   _Pragma("clang loop vectorize(enable) distribute(enable)")
#elif defined(__GNUC__)
	#define __DECL_VECTORIZED_LOOP	_Pragma("GCC ivdep")
#else
	#define __DECL_VECTORIZED_LOOP
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
	#define ___assume_aligned(foo, a) std::assume_aligned<a>(foo)
#elif _MSC_VER
	#ifndef __builtin_assume_aligned
		#define ___assume_aligned(foo, a) foo
	#else
		#define ___assume_aligned(foo, a) __builtin_assume_aligned(foo, a)
	#endif
#elif defined(__GNUC__)
	#define ___assume_aligned(foo, a) __builtin_assume_aligned(foo, a)
#else
		#define ___assume_aligned(foo, a) foo
#endif
// hint for branch-optimize. 20210720 K.O
// Usage:
// __LIKELY_IF(expr) : Mostly (expr) will be effected.
// __UNLIKELY_IF(expr) : Mostly (expr) will not be effected.
#undef __LIKELY_IF
#undef __UNLIKELY_IF

#if defined(__cplusplus)
	#if (__cplusplus >= 202000L)
	#define __LIKELY_IF(foo) if(foo) [[likely]]
	#define __UNLIKELY_IF(foo) if(foo) [[unlikely]]
	#endif
#endif

#if !defined(__LIKELY_IF) || !defined(__UNLIKELY_IF)
	#undef __HAS_LIKELY_UNLIKELY_TYPE1__
	#if defined(__clang__)
		#define __HAS_LIKELY_UNLIKELY_TYPE1__
	#elif defined(__GNUC__)
		#if __GNUC__ >= 3
			#define __HAS_LIKELY_UNLIKELY_TYPE1__
		#endif
	#endif
	// ToDo: Implement for other compilers.
	#if defined(__HAS_LIKELY_UNLIKELY_TYPE1__)
	// OK, This compiler seems to have __builtin_expect(foo, bar).
		#define __LIKELY_IF(foo) if(__builtin_expect((foo), 1))
		#define __UNLIKELY_IF(foo) if(__builtin_expect((foo), 0))
	#else
		// Fallthrough: maybe not have __builtin_expect()
		#define __LIKELY_IF(foo) if(foo)
		#define __UNLIKELY_IF(foo) if(foo)
	#endif
	#undef __HAS_LIKELY_UNLIKELY_TYPE1__
#endif

#ifndef SUPPORT_CPLUSPLUS_11
	#ifndef int8_t
		typedef signed char int8_t;
	#endif
	#ifndef int16_t
		typedef signed short int16_t;
	#endif
	#ifndef int32_t
		typedef signed int int32_t;
	#endif
	#ifndef int64_t
		typedef signed long long int64_t;
	#endif
	#ifndef uint8_t
		typedef unsigned char uint8_t;
	#endif
	#ifndef uint16_t
		typedef unsigned short uint16_t;
	#endif
	#ifndef uint32_t
		typedef unsigned int uint32_t;
	#endif
	#ifndef uint64_t
		typedef unsigned long long uint64_t;
	#endif
#endif

#ifndef _WIN32
	#ifndef LPTSTR
		typedef _TCHAR* LPTSTR;
	#endif
	#ifndef LPCTSTR
		typedef const _TCHAR* LPCTSTR;
	#endif
	#ifndef BOOL
		typedef int BOOL;
	#endif
	#ifndef TRUE
		#define TRUE 1
	#endif
	#ifndef FALSE
		#define FALSE 0
	#endif
	#ifndef BYTE
		typedef uint8_t BYTE;
	#endif
	#ifndef WORD
		typedef uint16_t WORD;
	#endif
	#ifndef DWORD
		typedef uint32_t DWORD;
	#endif
	#ifndef QWORD
		typedef uint64_t QWORD;
	#endif
	#ifndef INT8
		typedef int8_t INT8;
	#endif
	#ifndef INT16
		typedef int16_t INT16;
	#endif
	#ifndef INT32
		typedef int32_t INT32;
	#endif
	#ifndef INT64
		typedef int64_t INT64;
	#endif
	#ifndef UINT8
		typedef uint8_t UINT8;
	#endif
	#ifndef UINT16
		typedef uint16_t UINT16;
	#endif
	#ifndef UINT32
		typedef uint32_t UINT32;
	#endif
	#ifndef UINT64
		typedef uint64_t UINT64;
	#endif
	#ifndef INT
		typedef int INT;
	#endif
	#ifndef UINT
		typedef unsigned int UINT;
	#endif
#endif
#if /*!defined(_WIN32) || */!defined(SOCKET)
	typedef uintptr_t SOCKET;
#endif
typedef union pair16_t {
	struct {
#ifdef __BIG_ENDIAN__
		uint8_t h, l;
#else
		uint8_t l, h;
#endif
	} b;
	uint8_t barray[2];
	struct {
#ifdef __BIG_ENDIAN__
		int8_t h, l;
#else
		int8_t l, h;
#endif
	} sb;
	int8_t sbarray[2];
	uint16_t u16; // ToDo: Remove
	int16_t s16; // ToDo: Remove
	uint16_t w;
	int16_t sw;

	inline void __FASTCALL read_2bytes_le_from(uint8_t *t)
	{
		// ToDo: for Unalignment data.
		#ifdef __BIG_ENDIAN__
		b.l = t[0]; b.h = t[1];
		#else
		w = *((uint16_t*)t);
		#endif
	}
	inline void __FASTCALL write_2bytes_le_to(uint8_t *t)
	{
		// ToDo: for Unalignment data.
		#ifdef __BIG_ENDIAN__
		t[0] = b.l; t[1] = b.h;
		#else
		*((uint16_t*)t) = w;
		#endif
	}
	inline void __FASTCALL read_2bytes_be_from(uint8_t *t)
	{
		// ToDo: for Unalignment data.
		#ifdef __BIG_ENDIAN__
		w = *((uint16_t*)t);
		#else
		b.h = t[0]; b.l = t[1];
		#endif
	}
	inline void __FASTCALL write_2bytes_be_to(uint8_t *t)
	{
		// ToDo: for Unalignment data.
		#ifdef __BIG_ENDIAN__
		*((uint16_t*)t) = w;
		#else
		t[0] = b.h; t[1] = b.l;
		#endif
	}
	
	inline void __FASTCALL set_2bytes_be_from(uint16_t n)
	{
		#ifdef __BIG_ENDIAN__
		w = n;
		#else
		union {
			uint16_t w;
			struct {
				uint8_t h, l;
			}b;
		} bigv;
		bigv.w = n;
		b.l = bigv.b.l; b.h = bigv.b.h;
		#endif
	}
	inline void __FASTCALL set_2bytes_le_from(uint16_t n)
	{
		#ifdef __BIG_ENDIAN__
		union {
			uint16_t w;
			struct {
				uint8_t l, h;
			}b;
		} littlev;
		littlev.w = n;
		b.l = littlev.b.l; b.h = littlev.b.h;
		#else
		w = n;
		#endif
	}
	inline uint16_t __FASTCALL get_2bytes_be_to()
	{
		#ifdef __BIG_ENDIAN__
		return w;
		#else
		union {
			uint16_t w;
			struct {
				uint8_t h, l;
			}b;
		} bigv;
		bigv.b.l = b.l; bigv.b.h = b.h;
		return bigv.w;
		#endif
	}
	inline uint16_t __FASTCALL get_2bytes_le_to()
	{
		#ifdef __BIG_ENDIAN__
		union {
			uint16_t w;
			struct {
				uint8_t l, h;
			}b;
		} littlev;
		littlev.b.l = b.l; littlev.b.h = b.h;
		return littlev.w;
		#else
		return w;
		#endif
	}

} pair16_t;

typedef union pair32_t {
	struct {
#ifdef __BIG_ENDIAN__
		uint8_t h3, h2, h, l;
#else
		uint8_t l, h, h2, h3;
#endif
	} b;
	uint8_t barray[4];
	struct {
#ifdef __BIG_ENDIAN__
		int8_t h3, h2, h, l;
#else
		int8_t l, h, h2, h3;
#endif
	} sb;
	int8_t sbarray[4];
	struct {
#ifdef __BIG_ENDIAN__
		uint16_t h, l;
#else
		uint16_t l, h;
#endif
	} w;
	struct {
#ifdef __BIG_ENDIAN__
		int16_t h, l;
#else
		int16_t l, h;
#endif
	} sw;
	struct {
#ifdef __BIG_ENDIAN__
		pair16_t h, l;
#else
		pair16_t l, h;
#endif
	} p16;
	uint32_t d;
	int32_t sd;
	float f; // single float
  
	inline void __FASTCALL read_2bytes_le_from(uint8_t *t)
	{
		d = 0;
		#ifdef __BIG_ENDIAN__
		b.l = t[0]; b.h = t[1];
		#else
		// ToDo: Support for unaligned.
		w.l = *((uint16_t*)t);
		#endif
	}
	inline void __FASTCALL write_2bytes_le_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		t[0] = b.l; t[1] = b.h;
		#else
		// ToDo: Support for unaligned.
		*((uint16_t*)t) = w.l;
		#endif
	}
	inline void __FASTCALL read_2bytes_be_from(uint8_t *t)
	{
		d = 0;
		#ifdef __BIG_ENDIAN__
		// ToDo: Support for unaligned.
		w.l = *((uint16_t*)t);
		#else
		b.h = t[0]; b.l = t[1];
		#endif
	}
	inline void __FASTCALL write_2bytes_be_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		// ToDo: Support for unaligned.
		*((uint16_t*)t) = w.l;
		#else
		t[0] = b.h; t[1] = b.l;
		#endif
	}
	inline void __FASTCALL read_4bytes_le_from(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		b.l = t[0]; b.h = t[1]; b.h2 = t[2]; b.h3 = t[3];
		#else
		// ToDo: Support for unaligned.
		d = *((uint32_t*)t);
		#endif
	}
	inline void __FASTCALL write_4bytes_le_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		t[0] = b.l; t[1] = b.h; t[2] = b.h2; t[3] = b.h3;
		#else
		// ToDo: Support for unaligned.
		*((uint32_t*)t) = d;
		#endif
	}
	inline void __FASTCALL read_4bytes_be_from(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		// ToDo: Support for unaligned.
		d = *((uint32_t*)t);
		#else
		b.h3 = t[0]; b.h2 = t[1]; b.h = t[2]; b.l = t[3];
		#endif
	}
	inline void __FASTCALL write_4bytes_be_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		// ToDo: Support for unaligned.
		*((uint32_t*)t) = d;
		#else
		t[0] = b.h3; t[1] = b.h2; t[2] = b.h; t[3] = b.l;
		#endif
	}

	inline void __FASTCALL set_2bytes_be_from(uint16_t n)
	{
		#ifdef __BIG_ENDIAN__
		w.h = 0;
		w.l = n;
		#else
		union {
			uint16_t w;
			struct {
				uint8_t h, l;
			}b;
		} bigv;
		bigv.w = n;
		b.l = bigv.b.l; b.h = bigv.b.h;
		w.h = 0;
		#endif
	}
	inline void __FASTCALL set_2bytes_le_from(uint16_t n)
	{
		#ifdef __BIG_ENDIAN__
		union {
			uint16_t w;
			struct {
				uint8_t l, h;
			}b;
		} littlev;
		littlev.w = n;
		b.l = littlev.b.l; b.h = littlev.b.h;
		w.h = 0;
		#else
		w.h = 0;
		w.l = n;
		#endif
	}
	inline uint16_t __FASTCALL get_2bytes_be_to()
	{
		#ifdef __BIG_ENDIAN__
		return w.l;
		#else
		union {
			uint16_t w;
			struct {
				uint8_t h, l;
			}b;
		} bigv;
		bigv.b.l = b.l; bigv.b.h = b.h;
		return bigv.w;
		#endif
	}
	inline uint16_t __FASTCALL get_2bytes_le_to()
	{
		#ifdef __BIG_ENDIAN__
		union {
			uint16_t w;
			struct {
				uint8_t l, h;
			}b;
		} littlev;
		littlev.b.l = b.l; littlev.b.h = b.h;
		return littlev.w;
		#else
		return w.l;
		#endif
	}
	
	inline void __FASTCALL set_4bytes_be_from(uint32_t n)
	{
		#ifdef __BIG_ENDIAN__
		d = n;
		#else
		union {
			uint32_t dw;
			struct {
				uint8_t h3, h2, h, l;
			}b;
		} bigv;
		bigv.dw = n;
		b.l = bigv.b.l; b.h = bigv.b.h; b.h2 = bigv.b.h2; b.h3 = bigv.b.h3;
		#endif
	}
	inline void __FASTCALL set_4bytes_le_from(uint32_t n)
	{
		#ifdef __BIG_ENDIAN__
		union {
			uint32_t dw;
			struct {
				uint8_t l, h, h2, h3;
			}b;
		} littlev;
		littlev.dw = n;
		b.l = littlev.b.l; b.h = littlev.b.h; b.h2 = littlev.b.h2; b.h3 = littlev.b.h3;
		#else
		d = n;
		#endif
	}
	inline uint32_t __FASTCALL get_4bytes_be_to()
	{
		#ifdef __BIG_ENDIAN__
		return d;
		#else
		union {
			uint32_t dw;
			struct {
				uint8_t h3, h2, h, l;
			}b;
		} bigv;
		bigv.b.l = b.l; bigv.b.h = b.h; bigv.b.h2 = b.h2; bigv.b.h3 = b.h3;
		return bigv.dw;
		#endif
	}
	inline uint32_t __FASTCALL get_4bytes_le_to()
	{
		#ifdef __BIG_ENDIAN__
		union {
			uint32_t dw;
			struct {
				uint8_t l, h, h2, h3;
			}b;
		} littlev;
		littlev.b.l = b.l; littlev.b.h = b.h; littlev.b.h2 = b.h2; littlev.b.h3 = b.h3;
		return littlev.dw;
		#else
		return d;
		#endif
	}
} pair32_t;


typedef union pair64_t {
	struct {
#ifdef __BIG_ENDIAN__
		uint8_t h7, h6, h5, h4, h3, h2, h, l;
#else
		uint8_t l, h, h2, h3, h4, h5, h6, h7;
#endif
	} b;
	uint8_t barray[8];
	struct {
#ifdef __BIG_ENDIAN__
		int8_t h7, h6, h5, h4, h3, h2, h, l;
#else
		int8_t l, h, h2, h3, h4, h5, h6, h7;
#endif
	} sb;
	uint8_t sbarray[8];
	struct {
#ifdef __BIG_ENDIAN__
		uint16_t h3, h2, h, l;
#else
		uint16_t l, h, h2, h3;
#endif
	} w;
	struct {
#ifdef __BIG_ENDIAN__
		int16_t h3, h2, h, l;
#else
		int16_t l, h, h2, h3;
#endif
	} sw;
	struct {
#ifdef __BIG_ENDIAN__
		pair16_t h3, h2, h, l;
#else
		pair16_t l, h, h2, h3;
#endif
	} p16;
	struct {
#ifdef __BIG_ENDIAN__
		uint32_t h, l;
#else
		uint32_t l, h;
#endif
	} d;
	struct {
#ifdef __BIG_ENDIAN__
		int32_t h, l;
#else
		int32_t l, h;
#endif
	} sd;
	struct {
#ifdef __BIG_ENDIAN__
		pair32_t h, l;
#else
		pair32_t l, h;
#endif
	} p32;
	struct {
#ifdef __BIG_ENDIAN__
		float h, l;
#else
		float l, h;
#endif
	} f32;
	uint64_t q;
	int64_t sq;
	double df; // double float
	inline void __FASTCALL read_2bytes_le_from(uint8_t *t)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		b.l = t[0]; b.h = t[1]; //b.h2 = b.h3 = 0;
//		b.h4 = 0; b.h5 = 0; b.h6 = 0; b.h7 = 0;
		#else
		// ToDo: Support unaligned.
		w.l = *((uint16_t*)t);
		#endif
	}
	inline void __FASTCALL write_2bytes_le_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		t[0] = b.l; t[1] = b.h;
		#else
		// ToDo: Support unaligned.
		*((uint16_t*)t) = w.l;
		#endif
	}
	inline void __FASTCALL read_2bytes_be_from(uint8_t *t)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		// ToDo: Support unaligned.
		w.l = *((uint16_t*)t);
		#else
		b.h = t[0]; b.l = t[1];
		#endif
	}
	inline void __FASTCALL write_2bytes_be_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		// ToDo: Support unaligned.
		*((uint16_t*)t) = w.l;
		#else
		t[0] = b.h; t[1] = b.l;
		#endif
	}
	inline void __FASTCALL read_4bytes_le_from(uint8_t *t)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		b.l = t[0]; b.h = t[1]; b.h2 = t[2]; b.h3 = t[3];
		#else
		// ToDo: Support unaligned.
		d.l = *((uint32_t*)t);
		#endif
	}
	inline void __FASTCALL write_4bytes_le_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		t[0] = b.l; t[1] = b.h; t[2] = b.h2; t[3] = b.h3;
		#else
		// ToDo: Support unaligned.
		*((uint32_t*)t) = d.l;
		#endif
	}
	inline void __FASTCALL read_4bytes_be_from(uint8_t *t)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		// ToDo: Support unaligned.
		d.l = *((uint32_t*)t);
		#else
		b.h3 = t[0]; b.h2 = t[1]; b.h = t[2]; b.l = t[3];
		#endif
	}
	inline void __FASTCALL write_4bytes_be_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		// ToDo: Support unaligned.
		*((uint32_t*)t) = d.l;
		#else
		t[0] = b.h3; t[1] = b.h2; t[2] = b.h; t[3] = b.l;
		#endif
	}

	// Note: Expect to optimize by SIMD when aligned this value. 20210811 K.O
	inline void __FASTCALL read_8bytes_le_from(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
			int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = t[ij];
				ij--;
			}
		#else
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = t[ii];
			}
		#endif
//		b.l = t[0];  b.h = t[1];  b.h2 = t[2]; b.h3 = t[3];
//		b.h4 = t[4]; b.h5 = t[5]; b.h6 = t[6]; b.h7 = t[7];
	}
	inline void __FASTCALL write_8bytes_le_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
			int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				t[ij] = barray[ii];
				ij--;
			}
		#else
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				t[ii] = barray[ii];
			}
		#endif
//		t[0] = b.l;  t[1] = b.h;  t[2] = b.h2; t[3] = b.h3;
//		t[4] = b.h4; t[5] = b.h5; t[6] = b.h6; t[7] = b.h7;
	}
	inline void __FASTCALL read_8bytes_be_from(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = t[ii];
			}
		#else
			int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = t[ij];
				ij--;
			}
		#endif
//		b.h7 = t[0]; b.h6 = t[1]; b.h5 = t[2]; b.h4 = t[3];
//		b.h3 = t[4]; b.h2 = t[5]; b.h = t[6];  b.l = t[7];
	}
	inline void __FASTCALL write_8bytes_be_to(uint8_t *t)
	{
		#ifdef __BIG_ENDIAN__
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				t[ii] = barray[ii];
			}
		#else
			int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				t[ij] = barray[ii];
				ij--;
			}
		#endif
//		t[0] = b.h7; t[1] = b.h6; t[2] = b.h5; t[3] = b.h4;
//		t[4] = b.h3; t[5] = b.h2; t[6] = b.h;  t[7] = b.l;
	}

	inline void __FASTCALL set_2bytes_be_from(uint16_t n)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		w.l = n;
		#else
		union {
			uint16_t w;
			struct {
				uint8_t h, l;
			}b;
		} bigv;
		bigv.w = n;
		b.l = bigv.b.l; b.h = bigv.b.h;
//		b.h2 = 0; b.h3 = 0;
//		b.h4 = 0; b.h5 = 0; b.h6 = 0; b.h7 = 0;
		#endif
	}
	inline void __FASTCALL set_2bytes_le_from(uint16_t n)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		union {
			uint16_t w;
			struct {
				uint8_t l, h;
			}b;
		} littlev;
		littlev.w = n;
		b.l = littlev.b.l; b.h = littlev.b.h;
//		b.h2 = 0; b.h3 = 0;
//		b.h4 = 0; b.h5 = 0; b.h6 = 0; b.h7 = 0;
		#else
		w.l = n;
		#endif
	}
	inline uint16_t __FASTCALL get_2bytes_be_to()
	{
		#ifdef __BIG_ENDIAN__
		return w.l;
		#else
		union {
			uint16_t w;
			struct {
				uint8_t h, l;
			}b;
		} bigv;
		bigv.b.l = b.l; bigv.b.h = b.h;
		return bigv.w;
		#endif
	}
	inline uint16_t __FASTCALL get_2bytes_le_to()
	{
		#ifdef __BIG_ENDIAN__
		union {
			uint16_t w;
			struct {
				uint8_t l, h;
			}b;
		} littlev;
		littlev.b.l = b.l; littlev.b.h = b.h;
		return littlev.w;
		#else
		return w.l;
		#endif
	}
	
	inline void __FASTCALL set_4bytes_be_from(uint32_t n)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		d.l = n;
		#else
		union {
			uint32_t dw;
			struct {
				uint8_t h3, h2, h, l;
			}b;
		} bigv;
		bigv.dw = n;
		b.l = bigv.b.l; b.h = bigv.b.h; b.h2 = bigv.b.h2; b.h3 = bigv.b.h3;
//		b.h4 = 0;       b.h5 = 0;       b.h6 = 0;         b.h7 = 0;
		#endif
	}
	inline void __FASTCALL set_4bytes_le_from(uint32_t n)
	{
		q = 0;
		#ifdef __BIG_ENDIAN__
		union {
			uint32_t dw;
			struct {
				uint8_t l, h, h2, h3;
			}b;
		} littlev;
		littlev.dw = n;
		b.l = littlev.b.l; b.h = littlev.b.h; b.h2 = littlev.b.h2; b.h3 = littlev.b.h3;
		//b.h4 = 0;          b.h5 = 0;          b.h6 = 0;            b.h7 = 0;
		#else
		d.l = n;
		#endif
	}
	inline uint32_t __FASTCALL  get_4bytes_be_to()
	{
		#ifdef __BIG_ENDIAN__
		return d.l;
		#else
		union {
			uint32_t dw;
			struct {
				uint8_t h3, h2, h, l;
			}b;
		} bigv;
		bigv.b.l = b.l; bigv.b.h = b.h; bigv.b.h2 = b.h2; bigv.b.h3 = b.h3;
		return bigv.dw;
		#endif
	}
	inline uint32_t __FASTCALL get_4bytes_le_to()
	{
		#ifdef __BIG_ENDIAN__
		union {
			uint32_t dw;
			struct {
				uint8_t l, h, h2, h3;
			}b;
		} littlev;
		littlev.b.l = b.l; littlev.b.h = b.h; littlev.b.h2 = b.h2; littlev.b.h3 = b.h3;
		return littlev.dw;
		#else
		return d.l;
		#endif
	}

	inline void __FASTCALL set_8bytes_be_from(uint64_t n)
	{
		__DECL_ALIGNED(16) union {
			uint64_t qw;
			struct {
				uint8_t h7, h6, h5, h4, h3, h2, h, l;
			}b;
			uint8_t barray[8];
		} bigv;
		bigv.qw = n;
		#ifdef __BIG_ENDIAN__
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = bigv.barray[ii];
			}
		#else
		int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = bigv.barray[ij];
				ij--;
			}
		#endif
		//b.l = bigv.b.l;   b.h = bigv.b.h;   b.h2 = bigv.b.h2; b.h3 = bigv.b.h3;
		//b.h4 = bigv.b.h4; b.h5 = bigv.b.h5; b.h6 = bigv.b.h6; b.h7 = bigv.b.h7;
	}
	inline void __FASTCALL set_8bytes_le_from(uint64_t n)
	{
		__DECL_ALIGNED(16) union {
			uint64_t qw;
			struct {
				uint8_t l, h, h2, h3, h4, h5, h6, h7;
			}b;
			uint8_t barray[8];
		} littlev;
		littlev.qw = n;
		#ifdef __BIG_ENDIAN__
		int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = littlev.barray[ij];
				ij--;
			}
		#else
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				barray[ii] = littlev.barray[ii];
			}
		#endif
//		b.l = littlev.b.l;   b.h = littlev.b.h;   b.h2 = littlev.b.h2; b.h3 = littlev.b.h3;
//		b.h4 = littlev.b.h4; b.h5 = littlev.b.h5; b.h6 = littlev.b.h6; b.h7 = littlev.b.h7;
	}
	inline uint64_t __FASTCALL get_8bytes_be_to()
	{
		__DECL_ALIGNED(16) union {
			uint64_t qw;
			struct {
				uint8_t h7, h6, h5, h4, h3, h2, h, l;
			}b;
			uint8_t barray[8];
		} bigv;
		#ifdef __BIG_ENDIAN__
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				bigv.barray[ii] = barray[ii];
			}
		#else
		int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				bigv.barray[ii] = barray[ij];
				ij--;
			}
		#endif
		//bigv.b.l = b.l;   bigv.b.h = b.h;   bigv.b.h2 = b.h2; bigv.b.h3 = b.h3;
		//bigv.b.h4 = b.h4; bigv.b.h5 = b.h5; bigv.b.h6 = b.h6; bigv.b.h7 = b.h7;
		return bigv.qw;
	}
	inline uint64_t __FASTCALL get_8bytes_le_to()
	{
		__DECL_ALIGNED(16) union {
			uint64_t qw;
			struct {
				uint8_t l, h, h2, h3, h4, h5, h6, h7;
			}b;
			uint8_t barray[8];
		} littlev;
		#ifdef __BIG_ENDIAN__
		int ij = 7;
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				littlev.barray[ii] = barray[ij];
				ij--;
			}
		#else
		__DECL_VECTORIZED_LOOP
			for(int ii = 0; ii < 8; ii++) {
				littlev.barray[ii] = barray[ii];
			}
		#endif
		//littlev.b.l = b.l;   littlev.b.h = b.h;   littlev.b.h2 = b.h2; littlev.b.h3 = b.h3;
		//littlev.b.h4 = b.h4; littlev.b.h5 = b.h5; littlev.b.h6 = b.h6; littlev.b.h7 = b.h7;
		return littlev.qw;
	}

} pair64_t;


inline uint32_t __FASTCALL EndianToLittle_DWORD(uint32_t x)
{
	pair32_t xx;
	xx.d = x;
	return xx.get_4bytes_le_to();
}

inline uint16_t  __FASTCALL EndianToLittle_WORD(uint16_t x)
{
	pair16_t xx;
	xx.w = x;
	return xx.get_2bytes_le_to();
}

inline uint32_t  __FASTCALL EndianFromLittle_DWORD(uint32_t x)
{
	pair32_t xx;
	xx.set_4bytes_le_from(x);
	return xx.d;
}

inline uint16_t  __FASTCALL EndianFromLittle_WORD(uint16_t x)
{
	pair16_t xx;
	xx.set_2bytes_le_from(x);
	return xx.w;
}


inline uint32_t  __FASTCALL  EndianToBig_DWORD(uint32_t x)
{
	pair32_t xx;
	xx.d = x;
	return xx.get_4bytes_be_to();
}

inline uint16_t  __FASTCALL EndianToBig_WORD(uint16_t x)
{
	pair16_t xx;
	xx.w = x;
	return xx.get_2bytes_be_to();
}

inline uint32_t  __FASTCALL EndianFromBig_DWORD(uint32_t x)
{
	pair32_t xx;
	xx.set_4bytes_be_from(x);
	return xx.d;
}

inline uint16_t  __FASTCALL EndianFromBig_WORD(uint16_t x)
{
	pair16_t xx;
	xx.set_2bytes_be_from(x);
	return xx.w;
}
// max/min
#ifndef _MSC_VER
	#include <algorithm>
//#undef max
//	#undef min
//	#define max(a,b) std::max(a,b)
//	#define min(a,b) std::min(a,b)
	using std::min;
	using std::max;
#endif

// string
#if defined(__GNUC__) || defined(__CYGWIN__) || defined(Q_OS_CYGWIN)
	#define stricmp(a,b) strcasecmp(a,b)
	#define strnicmp(a,b,n) strncasecmp(a,b,n)
#endif

#ifndef SUPPORT_TCHAR_TYPE
	#ifndef _fgetts
		#define _fgetts fgets
	#endif
	#ifndef _ftprintf
		#define _ftprintf printf
	#endif
	#ifndef _tfopen
		#define _tfopen fopen
	#endif
	#ifndef _tcscmp
		#define _tcscmp strcmp
	#endif
	#ifndef _tcscpy
		#define _tcscpy strcpy
	#endif
	#ifndef _tcsicmp
		#define _tcsicmp stricmp
	#endif
	#ifndef _tcslen
		#define _tcslen strlen
	#endif
	#ifndef _tcscat
		#define _tcscat strcat
	#endif
	#ifndef _tcsncat
		#define _tcsncat strncat
	#endif
	#ifndef _tcsncpy
		#define _tcsncpy strncpy
	#endif
	#ifndef _tcsncicmp
		#define _tcsncicmp strnicmp
	#endif
	#ifndef _tcschr
		#define _tcschr strchr
	#endif
	#ifndef _tcsrchr
		#define _tcsrchr strrchr
	#endif
	#ifndef _tcsstr
		#define _tcsstr strstr
	#endif
	#ifndef _tcstok
		#define _tcstok strtok
	#endif
	#ifndef _tstoi
		#define _tstoi atoi
	#endif
	#ifndef _tcstol
		#define _tcstol strtol
	#endif
	#ifndef _tcstoul
		#define _tcstoul strtoul
	#endif
	#ifndef _stprintf
		#define _stprintf sprintf
	#endif
	#ifndef _vstprintf
		#define _vstprintf vsprintf
	#endif
	#ifndef _taccess
		#define _taccess access
	#endif
	#ifndef _tremove
		#define _tremove remove
	#endif
	#ifndef _trename
		#define _trename rename
	#endif
	#define __T(x) x
	#define _T(x) __T(x)
	#define _TEXT(x) __T(x)
#endif

#ifndef SUPPORT_SECURE_FUNCTIONS
	#ifndef errno_t
		typedef int errno_t;
	#endif
//	errno_t DLL_PREFIX my_tfopen_s(FILE** pFile, const _TCHAR *filename, const _TCHAR *mode);
	errno_t DLL_PREFIX my_tcscat_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource);
	errno_t DLL_PREFIX my_strcpy_s(char *strDestination, size_t numberOfElements, const char *strSource);
	errno_t DLL_PREFIX my_tcscpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource);
	errno_t DLL_PREFIX my_strncpy_s(char *strDestination, size_t numberOfElements, const char *strSource, size_t count);
	errno_t DLL_PREFIX my_tcsncpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource, size_t count);
	char * DLL_PREFIX my_strtok_s(char *strToken, const char *strDelimit, char **context);
	_TCHAR *DLL_PREFIX my_tcstok_s(_TCHAR *strToken, const char *strDelimit, _TCHAR **context);
	#define my_fprintf_s fprintf
	#define my_ftprintf_s fprintf
	int DLL_PREFIX my_sprintf_s(char *buffer, size_t sizeOfBuffer, const char *format, ...);
	int DLL_PREFIX my_swprintf_s(wchar_t *buffer, size_t sizeOfBuffer, const wchar_t *format, ...);
	int DLL_PREFIX my_stprintf_s(_TCHAR *buffer, size_t sizeOfBuffer, const _TCHAR *format, ...);
	int DLL_PREFIX my_vsprintf_s(char *buffer, size_t numberOfElements, const char *format, va_list argptr);
	int DLL_PREFIX my_vstprintf_s(_TCHAR *buffer, size_t numberOfElements, const _TCHAR *format, va_list argptr);
#else
//	#define my_tfopen_s _tfopen_s
	#define my_tcscat_s _tcscat_s
	#define my_strcpy_s strcpy_s
	#define my_tcscpy_s _tcscpy_s
	#define my_strncpy_s strncpy_s
	#define my_tcsncpy_s _tcsncpy_s
	#define my_strtok_s strtok_s
	#define my_tcstok_s _tcstok_s
	#define my_fprintf_s fprintf_s
	#define my_ftprintf_s _ftprintf_s
	#define my_sprintf_s sprintf_s
	#define my_swprintf_s swprintf_s
	#define my_stprintf_s _stprintf_s
	#define my_vsprintf_s vsprintf_s
	#define my_vstprintf_s _vstprintf_s
#endif

// memory
#define my_memcpy memcpy


// C99 math functions
#ifdef _MSC_VER
	#define my_isfinite  _finite
	#define my_log2(v) (log((double)(v)) / log(2.0))
#else
	#include <cmath>
	#define my_isfinite std::isfinite
	#define my_log2 log2
#endif

// win32 api
#ifndef _WIN32
	BOOL MyWritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName);
	DWORD MyGetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName);
	UINT MyGetPrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName);
	// used only in winmain and win32 osd class
//	#define ZeroMemory(p,s) memset(p,0x00,s)
//	#define CopyMemory(t,f,s) memcpy(t,f,s)
#else
	#define MyWritePrivateProfileString WritePrivateProfileString
	#define MyGetPrivateProfileString GetPrivateProfileString
	#define MyGetPrivateProfileInt GetPrivateProfileInt
#endif

inline uint64_t __FASTCALL ExchangeEndianU64(uint64_t __in)
{
	__DECL_ALIGNED(16) pair64_t __i, __o;
	__i.q = __in;
	int ij = 7;
__DECL_VECTORIZED_LOOP
	for(int ii = 0; ii < 8; ii++) {
		__o.barray[ij] = __i.barray[ii];
		ij--;
	}
//	__o.b.h7  = __i.b.l;
//	__o.b.h6  = __i.b.h;
//	__o.b.h5  = __i.b.h2;
//	__o.b.h4  = __i.b.h3;
//	__o.b.h3  = __i.b.h4;
//	__o.b.h2  = __i.b.h5;
//	__o.b.h   = __i.b.h6;
//	__o.b.l   = __i.b.h7;
	return __o.q;
}

inline int64_t __FASTCALL ExchangeEndianS64(uint64_t __in)
{
	__DECL_ALIGNED(16) pair64_t __i, __o;
	__i.q = __in;
	int ij = 7;
__DECL_VECTORIZED_LOOP
	for(int ii = 0; ii < 8; ii++) {
		__o.barray[ij] = __i.barray[ii];
		ij--;
	}
	return __o.sq;
}
inline uint32_t __FASTCALL ExchangeEndianU32(uint32_t __in)
{
	__DECL_ALIGNED(4) pair32_t __i, __o;
	__i.d = __in;
	int ij = 3;
__DECL_VECTORIZED_LOOP
	for(int ii = 0; ii < 4; ii++) {
		__o.barray[ij] = __i.barray[ii];
		ij--;
	}
	
//	__o.b.h3 = __i.b.l;
//	__o.b.h2 = __i.b.h;
//	__o.b.h  = __i.b.h2;
//	__o.b.l  = __i.b.h3;
	return __o.d;
}

inline int32_t __FASTCALL ExchangeEndianS32(uint32_t __in)
{
	__DECL_ALIGNED(4) pair32_t __i, __o;
	__i.d = __in;
	int ij = 3;
__DECL_VECTORIZED_LOOP
	for(int ii = 0; ii < 4; ii++) {
		__o.barray[ij] = __i.barray[ii];
		ij--;
	}
	return __o.sd;
}

inline uint16_t __FASTCALL ExchangeEndianU16(uint16_t __in)
{
	pair16_t __i, __o;
	__i.u16 = __in;
	__o.b.h = __i.b.l;
	__o.b.l  = __i.b.h;
	return __o.u16;
}

inline int16_t __FASTCALL ExchangeEndianS16(uint16_t __in)
{
	pair16_t __i, __o;
	__i.u16 = __in;
	__o.b.h = __i.b.l;
	__o.b.l = __i.b.h;
	return __o.s16;
}

// rgb color
#if !defined(_RGB555) && !defined(_RGB565) && !defined(_RGB888)
	#define _RGB888
#endif

#define swap_endian_u16(foo) ExchangeEndianU16(foo)

#if defined(_RGB555) || defined(_RGB565)
	typedef uint16_t scrntype_t;
	scrntype_t DLL_PREFIX  __FASTCALL RGB_COLOR(uint32_t r, uint32_t g, uint32_t b);
	scrntype_t DLL_PREFIX  __FASTCALL RGBA_COLOR(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
	uint8_t DLL_PREFIX  __FASTCALL R_OF_COLOR(scrntype_t c);
	uint8_t DLL_PREFIX  __FASTCALL G_OF_COLOR(scrntype_t c);
	uint8_t DLL_PREFIX  __FASTCALL B_OF_COLOR(scrntype_t c);
	uint8_t DLL_PREFIX  __FASTCALL A_OF_COLOR(scrntype_t c);
	#if defined(_RGB565)
inline scrntype_t __FASTCALL rgb555le_to_scrntype_t(uint16_t n)
{
	#if !defined(__LITTLE_ENDIAN__)
	n = swap_endian_u16(n);
	#endif
	scrntype r;
	r = n & 0x7c00; // r
	r = r | (n & 0x03e0); // g
	r <<= 1;
	r = r | (n & 0x001f); // b
	return r;
}
	#else // RGB555
inline scrntype_t __FASTCALL rgb555le_to_scrntype_t(uint16_t n)
{
	#if !defined(__LITTLE_ENDIAN__)
	n = swap_endian_u16(n);
	#endif
	return n;
}
	#endif

inline scrntype_t __FASTCALL msb_to_mask_u16le(uint16_t n)
{
	// bit15: '0' = NOT TRANSPARENT
	//        '1' = TRANSPARENT
	#if !defined(__LITTLE_ENDIAN__)
	n = swap_endian_u16(n);
	#endif
	scrntype_t _n = ((n & 0x8000) != 0) ? 0x0000 : 0xffff;
	return _n;
}

inline scrntype_t __FASTCALL msb_to_alpha_mask_u16le(uint16_t n)
{
	// bit15: '0' = NOT TRANSPARENT
	//        '1' = TRANSPARENT
	#if !defined(__LITTLE_ENDIAN__)
	n = swap_endian_u16(n);
	#endif
	scrntype_t _n = ((n & 0x8000) != 0) ? 0x0000 : 0xffff;
	return _n; // Not ALPHA
}

#elif defined(_RGB888)
	typedef uint32_t scrntype_t;
#if defined(__LITTLE_ENDIAN__)
	#define RGB_COLOR(r, g, b)	(((uint32_t)(b) << 16) | ((uint32_t)(g) << 8) | ((uint32_t)(r) << 0) | (0xff << 24))
	#define RGBA_COLOR(r, g, b, a)	(((uint32_t)(b) << 16) | ((uint32_t)(g) << 8) | ((uint32_t)(r) << 0) | ((uint32_t)(a) << 24))
	#define R_OF_COLOR(c)		(((c)      ) & 0xff)
	#define G_OF_COLOR(c)		(((c) >>  8) & 0xff)
	#define B_OF_COLOR(c)		(((c) >> 16) & 0xff)
	#define A_OF_COLOR(c)		(((c) >> 24) & 0xff)
#else
	#define RGB_COLOR(r, g, b)	(((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | ((uint32_t)(b) << 0) | (0xff << 24))
	#define RGBA_COLOR(r, g, b, a)	(((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | ((uint32_t)(b) << 0) | ((uint32_t)(a) << 24))
	#define R_OF_COLOR(c)		(((c) >> 16) & 0xff)
	#define G_OF_COLOR(c)		(((c) >>  8) & 0xff)
	#define B_OF_COLOR(c)		(((c)      ) & 0xff)
	#define A_OF_COLOR(c)		(((c) >> 24) & 0xff)
#endif

inline scrntype_t __FASTCALL rgb555le_to_scrntype_t(uint16_t n)
{
	scrntype_t r, g, b;
	#if defined(__LITTLE_ENDIAN__)
	r = (n & 0x7c00) << (16 + 1);
	g = (n & 0x03e0) << (8 + 4 + 2);
	b = (n & 0x001f) << (8 + 3);
	return (r | g | b | 0x000000ff);
	#else
	scrntype_t g2;
	r = (n & 0x007c) << (16 + 1 + 8);
	g = (n & 0x0e00) << (8 + 2)
	g2= (n & 0x0030) << (8 + 4 + 2);
	b = (n & 0x1f00) << 3;
	return (r | g | g2 | b | 0x000000ff);
	#endif
}

inline scrntype_t __FASTCALL msb_to_mask_u16le(uint16_t n)
{
	// bit15: '0' = NOT TRANSPARENT
	//        '1' = TRANSPARENT
	scrntype_t _n;
	#if defined(__LITTLE_ENDIAN__)
	_n = ((n & 0x8000) != 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
	#else
	_n = ((n & 0x0080) != 0) ? RGBA_COLOR(0, 0, 0, 0) : RGBA_COLOR(255, 255, 255, 255);
	#endif
	return _n;
}

inline scrntype_t __FASTCALL msb_to_alpha_mask_u16le(uint16_t n)
{
	// bit15: '0' = NOT TRANSPARENT
	//        '1' = TRANSPARENT
	scrntype_t _n;
	#if defined(__LITTLE_ENDIAN__)
	_n = ((n & 0x8000) != 0) ? RGBA_COLOR(255, 255, 255, 0) : RGBA_COLOR(255, 255, 255, 255);
	#else
	_n = ((n & 0x0080) != 0) ? RGBA_COLOR(255, 255, 255, 0) : RGBA_COLOR(255, 255, 255, 255);
	#endif
	return _n;
}
#endif

// ToDo: for MSVC
#if defined(_RGB555) || defined(_RGBA565)
typedef	union {
	scrntype_t w[8];
	__v8hi v;
} scrntype_vec8_t;
typedef	union {
	scrntype_t w[16];
	__v8hi v[2];
} scrntype_vec16_t;
#else
typedef	 union {
	scrntype_t w[8];
	__v16hi v;
} scrntype_vec8_t;
typedef	union {
	scrntype_t w[16];
	__v16hi v[2];
} scrntype_vec16_t;
#endif

typedef  union {
	__v4hi v;
	uint8_t w[8];
} uint8_vec8_t;

typedef union {
	__v8hi v;
	uint16_t w[8];
} uint16_vec8_t;

typedef union {
	__v16hi v;
	uint32_t w[8];
} uint32_vec8_t;

typedef struct {
	uint16_vec8_t plane_table[256];
} _bit_trans_table_t;

typedef struct {
	scrntype_vec8_t plane_table[256];
} _bit_trans_table_scrn_t;

typedef struct {
	scrntype_t* palette; // Must be 2^planes entries. If NULL, assume RGB.
	_bit_trans_table_t* bit_trans_table[16]; // Must be exist >= planes. Must be aligned with sizeof(uint16_vec8_t).
	int xzoom; // 1 - 4?
	bool is_render[16];
	int shift;
	uint8_t* data[16];
	uint32_t baseaddress[16];
	uint32_t voffset[16];
	uint32_t addrmask;  // For global increment.
	uint32_t addrmask2; // For local increment.
	uint32_t begin_pos;
	uint32_t render_width;
} _render_command_data_t;


inline scrntype_vec8_t ConvertByteToMonochromePackedPixel(uint8_t src, _bit_trans_table_t *tbl,scrntype_t on_val, scrntype_t off_val)
{
	__DECL_ALIGNED(16) uint16_vec8_t   tmpd;
	__DECL_ALIGNED(32) scrntype_vec8_t tmpdd;
	_bit_trans_table_t*  vt = (_bit_trans_table_t*)___assume_aligned(tbl, sizeof(uint16_vec8_t));

	tmpd.v = vt->plane_table[src].v;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		tmpdd.w[i] = (tmpd.w[i] == 0) ? off_val: on_val;
	}
	return tmpdd;
}

// Note: Pls. read Note(s) of common.cpp -- 20181105 K.Ohta.
// Tables for below functions must be aligned by 16 (_bit_trans_table_t) or 32(_bit_trans_table_scrn_t).  
void DLL_PREFIX ConvertByteToPackedPixelByColorTable(uint8_t *src, scrntype_t* dst, int bytes, _bit_trans_table_t *tbl, scrntype_t *on_color_table, scrntype_t* off_color_table);
void DLL_PREFIX ConvertByteToPackedPixelByColorTable2(uint8_t *src, scrntype_t* dst, int bytes, _bit_trans_table_scrn_t *tbl, scrntype_t *on_color_table, scrntype_t* off_color_table);
void DLL_PREFIX ConvertByteToSparceUint16(uint8_t *src, uint16_t* dst, int bytes, _bit_trans_table_t *tbl, uint16_t mask);
void DLL_PREFIX ConvertByteToSparceUint8(uint8_t *src, uint16_t* dst, int bytes, _bit_trans_table_t *tbl, uint16_t mask);

// Table must be (ON_VAL_COLOR : OFF_VAL_COLOR)[256].
inline scrntype_vec8_t ConvertByteToPackedPixel_PixelTbl(uint8_t src, _bit_trans_table_scrn_t *tbl)
{
	__DECL_ALIGNED(32) scrntype_vec8_t tmpdd;
	_bit_trans_table_scrn_t*  vt = (_bit_trans_table_scrn_t*)___assume_aligned(tbl, sizeof(uint16_vec8_t));

	tmpdd.v = vt->plane_table[src].v;
	return tmpdd;
}

// Table must be (ON_VAL_COLOR : OFF_VAL_COLOR)[256].
inline scrntype_vec16_t ConvertByteToDoublePackedPixel_PixelTbl(uint8_t src, _bit_trans_table_scrn_t *tbl)
{
	__DECL_ALIGNED(32) scrntype_vec16_t tmpdd;
	__DECL_ALIGNED(32) scrntype_vec8_t tmpd;
	_bit_trans_table_scrn_t*  vt = (_bit_trans_table_scrn_t*)___assume_aligned(tbl, sizeof(uint16_vec8_t));
	tmpd.v = vt->plane_table[src].v;
	int j = 0;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i += 2) {
		tmpdd.w[i]     = tmpd.w[j];
		tmpdd.w[i + 1] = tmpd.w[j];
		j++;
	}
	return tmpdd;
}

// Table must be initialize ON_COLOR : OFF_COLOR
inline void ConvertByteToDoubleMonochromeUint8(uint8_t src, uint8_t* dst, _bit_trans_table_t* tbl)
{
	__DECL_ALIGNED(16) uint16_vec8_t   tmpd;
	uint16_vec8_t*  vt = (uint16_vec8_t*)___assume_aligned(&(tbl->plane_table[0]), sizeof(uint16_vec8_t));

	__DECL_ALIGNED(16) uint8_t d[16];
	tmpd = vt[src];
	int j = 0;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i += 2) {
		d[i]     = (uint8_t)(tmpd.w[j]);
		d[i + 1] = (uint8_t)(tmpd.w[j]);
		j++;
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i++) {
		dst[i] = d[i];
	}
}

inline void ConvertByteToMonochromeUint8(uint8_t src, uint8_t* dst, _bit_trans_table_t* tbl)
{
	__DECL_ALIGNED(16) uint16_vec8_t   tmpd;
	uint16_vec8_t*  vt = (uint16_vec8_t*)___assume_aligned(&(tbl->plane_table[0]), sizeof(uint16_vec8_t));

	tmpd = vt[src];
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		dst[i] = (uint8_t)(tmpd.w[i]);
	}
}

inline void ConvertRGBTo8ColorsUint8(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, _bit_trans_table_t* rtbl, _bit_trans_table_t* gtbl, _bit_trans_table_t* btbl, int shift)
{
	__DECL_ALIGNED(16) uint16_vec8_t   tmpd;
	uint16_vec8_t*  rvt = (uint16_vec8_t*)___assume_aligned(&(rtbl->plane_table[0]), sizeof(uint16_vec8_t));
	uint16_vec8_t*  gvt = (uint16_vec8_t*)___assume_aligned(&(gtbl->plane_table[0]), sizeof(uint16_vec8_t));
	uint16_vec8_t*  bvt = (uint16_vec8_t*)___assume_aligned(&(btbl->plane_table[0]), sizeof(uint16_vec8_t));

	tmpd.v = rvt[r].v;
	tmpd.v = tmpd.v | gvt[g].v;
	tmpd.v = tmpd.v | bvt[b].v;
	tmpd.v = tmpd.v >> shift;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		dst[i] = (uint8_t)(tmpd.w[i]);
	}
}

inline void ConvertRGBTo8ColorsUint8_Zoom2Left(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, _bit_trans_table_t* rtbl, _bit_trans_table_t* gtbl, _bit_trans_table_t* btbl, int shift)
{
	__DECL_ALIGNED(16) uint16_vec8_t   tmpd;
	uint16_vec8_t*  rvt = (uint16_vec8_t*)___assume_aligned(&(rtbl->plane_table[0]), sizeof(uint16_vec8_t));
	uint16_vec8_t*  gvt = (uint16_vec8_t*)___assume_aligned(&(gtbl->plane_table[0]), sizeof(uint16_vec8_t));
	uint16_vec8_t*  bvt = (uint16_vec8_t*)___assume_aligned(&(btbl->plane_table[0]), sizeof(uint16_vec8_t));

	tmpd.v = rvt[r].v;
	tmpd.v = tmpd.v | gvt[g].v;
	tmpd.v = tmpd.v | bvt[b].v;
	tmpd.v = tmpd.v >> shift;
__DECL_VECTORIZED_LOOP
	for(int i = 0, j = 0; i < 8; i += 2, j++) {
		dst[i]     = (uint8_t)(tmpd.w[j]);
		dst[i + 1] = (uint8_t)(tmpd.w[j]);
	}
}

inline void ConvertRGBTo8ColorsUint8_Zoom2Right(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, _bit_trans_table_t* rtbl, _bit_trans_table_t* gtbl, _bit_trans_table_t* btbl, int shift)
{
	__DECL_ALIGNED(16) uint16_vec8_t   tmpd;
	uint16_vec8_t*  rvt = (uint16_vec8_t*)___assume_aligned(&(rtbl->plane_table[0]), sizeof(uint16_vec8_t));
	uint16_vec8_t*  gvt = (uint16_vec8_t*)___assume_aligned(&(gtbl->plane_table[0]), sizeof(uint16_vec8_t));
	uint16_vec8_t*  bvt = (uint16_vec8_t*)___assume_aligned(&(btbl->plane_table[0]), sizeof(uint16_vec8_t));

	tmpd.v = rvt[r].v;
	tmpd.v = tmpd.v | gvt[g].v;
	tmpd.v = tmpd.v | bvt[b].v;
	tmpd.v = tmpd.v >> shift;
__DECL_VECTORIZED_LOOP
	for(int i = 0, j = 4; i < 8; i += 2, j++) {
		dst[i]     = (uint8_t)(tmpd.w[j]);
		dst[i + 1] = (uint8_t)(tmpd.w[j]);
	}
}

inline void ConvertRGBTo8ColorsUint8_Zoom2Double(uint8_t r, uint8_t g, uint8_t b, uint8_t* dst, _bit_trans_table_t* rtbl, _bit_trans_table_t* gtbl, _bit_trans_table_t* btbl, int shift)
{
	__DECL_ALIGNED(16) uint16_vec8_t   tmpd;
	uint16_vec8_t*  rvt = (uint16_vec8_t*)___assume_aligned(&(rtbl->plane_table[0]), sizeof(uint16_vec8_t));
	uint16_vec8_t*  gvt = (uint16_vec8_t*)___assume_aligned(&(gtbl->plane_table[0]), sizeof(uint16_vec8_t));
	uint16_vec8_t*  bvt = (uint16_vec8_t*)___assume_aligned(&(btbl->plane_table[0]), sizeof(uint16_vec8_t));

	tmpd.v = rvt[r].v;
	tmpd.v = tmpd.v | gvt[g].v;
	tmpd.v = tmpd.v | bvt[b].v;
	tmpd.v = tmpd.v >> shift;
__DECL_VECTORIZED_LOOP
	for(int i = 0, j = 0; i < 16; i += 2, j++) {
		dst[i]     = (uint8_t)(tmpd.w[j]);
		dst[i + 1] = (uint8_t)(tmpd.w[j]);
	}
}

inline void ConvertByteToMonochromeUint8Cond_Zoom2(uint8_t src, uint8_t* dst, _bit_trans_table_t* tbl, uint8_t on_color, uint8_t off_color)
{
	__DECL_ALIGNED(16) uint16_vec8_t   tmpd;
	uint16_vec8_t*  vt = (uint16_vec8_t*)___assume_aligned(&(tbl->plane_table[0]), sizeof(uint16_vec8_t));

	__DECL_ALIGNED(16) uint8_t d[16];
	tmpd = vt[src];
	int j = 0;
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i += 2) {
		d[i]     = (tmpd.w[j] == 0) ? off_color : on_color;
		d[i + 1] = (tmpd.w[j] == 0) ? off_color : on_color;
		j++;
	}
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 16; i++) {
		dst[i] = d[i];
	}
}

inline void ConvertByteToMonochromeUint8Cond(uint8_t src, uint8_t* dst, _bit_trans_table_t* tbl, uint8_t on_color, uint8_t off_color)
{
	__DECL_ALIGNED(16) uint16_vec8_t   tmpd;
	uint16_vec8_t*  vt = (uint16_vec8_t*)___assume_aligned(&(tbl->plane_table[0]), sizeof(uint16_vec8_t));

	tmpd = vt[src];
__DECL_VECTORIZED_LOOP
	for(int i = 0; i < 8; i++) {
		dst[i]     = (tmpd.w[i] == 0) ? off_color : on_color;
	}
}

void DLL_PREFIX PrepareBitTransTableUint16(_bit_trans_table_t *tbl, uint16_t on_val, uint16_t off_val);
void DLL_PREFIX PrepareBitTransTableScrnType(_bit_trans_table_scrn_t *tbl, scrntype_t on_val, scrntype_t off_val);
void DLL_PREFIX PrepareReverseBitTransTableUint16(_bit_trans_table_t *tbl, uint16_t on_val, uint16_t off_val);
void DLL_PREFIX PrepareReverseBitTransTableScrnType(_bit_trans_table_scrn_t *tbl, scrntype_t on_val, scrntype_t off_val);

void DLL_PREFIX Render8Colors_Line(_render_command_data_t *src, scrntype_t *dst, scrntype_t *dst2, bool scan_line);

void DLL_PREFIX Render16Colors_Line(_render_command_data_t *src, scrntype_t *dst, scrntype_t *dst2, bool scan_line);
void DLL_PREFIX Render2NColors_Line(_render_command_data_t *src, scrntype_t *dst, scrntype_t* dst2, bool scan_line, int planes);

void DLL_PREFIX Convert8ColorsToByte_Line(_render_command_data_t *src, uint8_t *dst);
void DLL_PREFIX Convert2NColorsToByte_Line(_render_command_data_t *src, uint8_t *dst, int planes);
void DLL_PREFIX Convert2NColorsToByte_LineZoom2(_render_command_data_t *src, uint8_t *dst, int planes);

// wav file header
#pragma pack(1)
typedef struct {
	char id[4];
	uint32_t size;
} wav_chunk_t;
#pragma pack()

#pragma pack(1)
typedef struct {
	wav_chunk_t riff_chunk;
	char wave[4];
	wav_chunk_t fmt_chunk;
	uint16_t format_id;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t data_speed;
	uint16_t block_size;
	uint16_t sample_bits;
} wav_header_t;
#pragma pack()

//  See http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html.
#pragma pack(1)
typedef struct {
	wav_chunk_t riff_chunk;
	char wave[4];
	wav_chunk_t fmt_chunk;
	uint16_t format_id;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t data_speed;
	uint16_t block_size;
	uint16_t sample_bits;
	uint16_t cbsize; // Extension size.Normaly set to 0.
	wav_chunk_t fact_chunk; // "fact", 4.
} wav_header_float_t;
#pragma pack()

// Use this before writing wav_data.
bool DLL_PREFIX write_dummy_wav_header(void *__fio);
// Use this after writng wav_data.
bool DLL_PREFIX set_wav_header(wav_header_t *header, wav_chunk_t *first_chunk, uint16_t channels, uint32_t rate,
							   uint16_t bits, size_t file_length);
bool DLL_PREFIX load_wav_to_stereo(void *__fio, int16_t **left_buf, int16_t **right_buf, uint32_t *rate, int *got_samples);
bool DLL_PREFIX load_wav_to_monoral(void *__fio, int16_t **buffer, uint32_t *rate, int *got_samples);

// file path
const _TCHAR *DLL_PREFIX get_application_path();
const _TCHAR *DLL_PREFIX get_initial_current_path();
const _TCHAR *DLL_PREFIX create_local_path(const _TCHAR *format, ...);
void DLL_PREFIX create_local_path(_TCHAR *file_path, int length, const _TCHAR *format, ...);
const _TCHAR *DLL_PREFIX create_absolute_path(const _TCHAR *format, ...);
void DLL_PREFIX create_absolute_path(_TCHAR *file_path, int length, const _TCHAR *format, ...);
bool DLL_PREFIX is_absolute_path(const _TCHAR *file_path);
const _TCHAR *DLL_PREFIX create_date_file_path(const _TCHAR *extension);
void DLL_PREFIX create_date_file_path(_TCHAR *file_path, int length, const _TCHAR *extension);
const _TCHAR *DLL_PREFIX create_date_file_name(const _TCHAR *extension);
void DLL_PREFIX create_date_file_name(_TCHAR *file_path, int length, const _TCHAR *extension);
bool DLL_PREFIX check_file_extension(const _TCHAR *file_path, const _TCHAR *ext);
const _TCHAR *DLL_PREFIX get_file_path_without_extensiton(const _TCHAR *file_path);
void DLL_PREFIX get_long_full_path_name(const _TCHAR* src, _TCHAR* dst, size_t dst_len);
const _TCHAR* DLL_PREFIX get_parent_dir(const _TCHAR* file);

// string
const _TCHAR *DLL_PREFIX  create_string(const _TCHAR* format, ...);
const wchar_t *DLL_PREFIX char_to_wchar(const char *cs);
const char *DLL_PREFIX  wchar_to_char(const wchar_t *ws);
const _TCHAR *DLL_PREFIX  char_to_tchar(const char *cs);
const char *DLL_PREFIX  tchar_to_char(const _TCHAR *ts);
const _TCHAR *DLL_PREFIX  wchar_to_tchar(const wchar_t *ws);
const wchar_t *DLL_PREFIX  tchar_to_wchar(const _TCHAR *ts);

// Convert Zenkaku KATAKANA/HIRAGANA/ALPHABET to Hankaku.Data must be UCS-4 encoding Unicode (UTF-32).
int DLL_PREFIX ucs4_kana_zenkaku_to_hankaku(const uint32_t in, uint32_t *buf, int bufchars);

// for disassedmbler
uint32_t DLL_PREFIX get_relative_address_8bit(uint32_t base, uint32_t mask, int8_t offset);
uint32_t DLL_PREFIX get_relative_address_16bit(uint32_t base, uint32_t mask, int16_t offset);
uint32_t DLL_PREFIX get_relative_address_32bit(uint32_t base, uint32_t mask, int32_t offset);
// misc
void DLL_PREFIX common_initialize();

int32_t DLL_PREFIX  __FASTCALL muldiv_s32(int32_t nNumber, int32_t nNumerator, int32_t nDenominator);
uint32_t DLL_PREFIX __FASTCALL muldiv_u32(uint32_t nNumber, uint32_t nNumerator, uint32_t nDenominator);

uint32_t DLL_PREFIX  get_crc32(uint8_t data[], int size);
uint32_t DLL_PREFIX  calc_crc32(uint32_t seed, uint8_t data[], int size);

inline uint16_t __FASTCALL jis_to_sjis(uint16_t jis)
{
	pair32_t tmp;
	
	tmp.w.l = jis - 0x2121;
	if(tmp.w.l & 0x100) {
		tmp.w.l += 0x9e;
	} else {
		tmp.w.l += 0x40;
	}
	if(tmp.b.l > 0x7f) {
		tmp.w.l += 0x01;
	}
	tmp.b.h = (tmp.b.h >> 1) + 0x81;
	if(tmp.w.l >= 0xa000) {
		tmp.w.l += 0x4000;
	}
	return tmp.w.l;
}

inline int __FASTCALL decibel_to_volume(int decibel)
{
	// +1 equals +0.5dB (same as fmgen)
	return (int)(1024.0 * pow(10.0, decibel / 40.0) + 0.5);
}

inline int32_t __FASTCALL apply_volume(int32_t sample, int volume)
{
//	int64_t output;
	int32_t output;
	if(sample < 0) {
		output = -sample;
		output *= volume;
		output >>= 10;
		output = -output;
	} else {
		output = sample;
		output *= volume;
		output >>= 10;
	}
//	if(output > 2147483647) {
//		return 2147483647;
//	} else if(output < (-2147483647 - 1)) {
//		return (-2147483647 - 1);
//	} else {
//		return (int32_t)output;
//	}
	return output;
}

// High pass filter and Low pass filter.
void DLL_PREFIX calc_high_pass_filter(int32_t* dst, int32_t* src, int sample_freq, int hpf_freq, int samples, double quality = 1.0, bool is_add = true);
void DLL_PREFIX calc_low_pass_filter(int32_t* dst, int32_t* src, int sample_freq, int lpf_freq, int samples, double quality = 1.0, bool is_add = true);


#define array_length(array) (sizeof(array) / sizeof(array[0]))

#define FROM_BCD(v)	(((v) & 0x0f) + (((v) >> 4) & 0x0f) * 10)
#define TO_BCD(v)	((int)(((v) % 100) / 10) << 4) | ((v) % 10)
#define TO_BCD_LO(v)	((v) % 10)
#define TO_BCD_HI(v)	(int)(((v) % 100) / 10)

// time
#define LEAP_YEAR(y)	(((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

#define dll_cur_time_t DLL_PREFIX_I struct cur_time_s

typedef DLL_PREFIX struct cur_time_s {
	int year, month, day, day_of_week, hour, minute, second;
	bool initialized;
	DLL_PREFIX cur_time_s()
	{
		initialized = false;
	}
	void DLL_PREFIX increment();
	void DLL_PREFIX update_year();
	void DLL_PREFIX update_day_of_week();
	bool DLL_PREFIX process_state(void *f, bool loading);
} cur_time_t;

void DLL_PREFIX get_host_time(cur_time_t* cur_time);
const _TCHAR DLL_PREFIX *get_lib_common_version();

// symbol
typedef struct symbol_s {
	uint32_t addr;
	_TCHAR *name;
	struct symbol_s *next_symbol;
} symbol_t;

const _TCHAR* DLL_PREFIX get_symbol(symbol_t *first_symbol, uint32_t addr);
const _TCHAR* DLL_PREFIX get_value_or_symbol(symbol_t *first_symbol, const _TCHAR *format, uint32_t addr);
const _TCHAR* DLL_PREFIX get_value_and_symbol(symbol_t *first_symbol, const _TCHAR *format, uint32_t addr);

#endif
