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
	#include <valarray>
	#include <algorithm>
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

/*!
  @todo will move to another directory.
*/
#include "./types/optimizer_utils.h"

#include "./types/basic_types.h"

/*!
  @todo will move to another directory.
*/
#include "./types/util_endians.h"

// max/min
#ifndef _MSC_VER
//#undef max
//	#undef min
//	#define max(a,b) std::max(a,b)
//	#define min(a,b) std::min(a,b)
	using std::min;
	using std::max;
#endif

// string
/*!
  @todo will move to another directory.
*/
#include "./types/util_strings.h"

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

// rgb color
#if !defined(_RGB555) && !defined(_RGB565) && !defined(_RGB888)
	#define _RGB888
#endif

/*!
  @todo will move to another directory.
*/
#include "./types/util_video.h"
#include "./types/util_sound.h"
#include "./types/util_system.h"
#include "./types/util_disasm.h"

// time
#define LEAP_YEAR(y)	(((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

#define dll_cur_time_t DLL_PREFIX_I struct cur_time_s

typedef struct cur_time_s {
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

#endif
