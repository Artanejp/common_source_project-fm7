#pragma once

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

// max/min
#ifndef _MSC_VER
//#undef max
//	#undef min
//	#define max(a,b) std::max(a,b)
//	#define min(a,b) std::min(a,b)
	#include <algorithm>
	using std::min;
	using std::max;
#endif


#ifndef _MAX_PATH
	#define _MAX_PATH 2048
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
