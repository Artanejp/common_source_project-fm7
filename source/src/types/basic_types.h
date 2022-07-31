#pragma once

// type definition
#ifndef SUPPORT_TCHAR_TYPE
	#ifndef _TCHAR
		typedef char _TCHAR;
	#endif
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

#undef __HAS_BUILTIN_BSWAP16_X
#undef __HAS_BUILTIN_BSWAP32_X
#undef __HAS_BUILTIN_BSWAP64_X

#if defined(__has_builtin)
	#if __has_builtin(__builtin_bswap16)
	#define __HAS_BUILTIN_BSWAP16_X 1
	#endif
	#if __has_builtin(__builtin_bswap32)
	#define __HAS_BUILTIN_BSWAP32_X 1
	#endif
	#if __has_builtin(__builtin_bswap64)
	#define __HAS_BUILTIN_BSWAP64_X 1
	#endif
#endif


#if /*!defined(_WIN32) || */!defined(SOCKET)
	typedef uintptr_t SOCKET;
#endif



