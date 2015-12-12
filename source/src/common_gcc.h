/*
 * common.h → common_gcc.h
 * Defines for gcc excepts Windows.
 * (c) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History :
 *     Nov 20,2015 : Initial
 */

#ifndef _COMMON_GCC_H
#define _COMMON_GCC_H


#include <stdarg.h>
#include <string>
#include <iostream>
#include <fstream>

#include <SDL.h>

#if defined(Q_OS_CYGWIN) 
# define CSP_OS_GCC_CYGWIN
# define CSP_OS_WINDOWS
#elif defined(Q_OS_WIN) || defined(__WIN32) || defined(__WIN64)
# define CSP_OS_GCC_WINDOWS
# define CSP_OS_WINDOWS
#else
# define CSP_OS_GCC_GENERIC
# define CSP_OS_GENERIC
#endif
#if defined(__clang__)
# define __CSP_COMPILER_CLANG
#else
# define __CSP_COMPILER_GCC
#endif

#include <stdarg.h>
#include "qt_input.h"
# if defined(CSP_OS_GCC_WINDOWS) || defined(CSP_OS_GCC_CYGWIN)
#  include <tchar.h>
# endif
#include <errno.h>

typedef int errno_t;
typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int64_t int64;
typedef int BOOL;

# if defined(CSP_OS_GCC_GENERIC)
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;
//# if !defined(Q_OS_CYGWIN)
typedef uint32_t UINT;
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef int32_t INT;
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;
# if !defined(TRUE)
#  define TRUE 1
# endif
# if !defined(FALSE)
#  define FALSE 0
# endif
static inline void _stprintf(char *s, const char *fmt, ...) {
   va_list args;
   
   va_start(args, fmt);
   sprintf(s, fmt, args);
   va_end(args);
}
# endif // Generic

//# define stricmp(a,b) strcmp(a,b)
//# define strnicmp(a,b,n) strncmp(a,b,n)

# define __T(x)      x
 
# define _T(x)       __T(x)
# define _TEXT(x)    __T(x)

# if defined(CSP_OS_GCC_GENERIC)
typedef char _TCHAR;
typedef _TCHAR* LPCTSTR;
# endif
//# if defined(CSP_OS_GCC_CYGWIN)
//#  define stricmp(a,b) strcmp(a,b)
//#  define strnicmp(a,b,n) strncmp(a,b,n)
//# endif

static int DeleteFile(_TCHAR *path) 
{
	if(std::remove(path) == 0) return (int)true;
	return 0;
}

#include <algorithm>

# if !defined(CSP_OS_WINDOWS)
#  define _N(x) _T(x)
# else
#  include <tchar.h>
# endif


# if !defined(CSP_OS_WINDOWS)
#  define ZeroMemory(p,s) memset(p,0x00,s)
#  define CopyMemory(t,f,s) memcpy(t,f,s)
# endif

#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
# if SDL_BYTEORDER == SDL_LIL_ENDIAN
#  define __LITTLE_ENDIAN__
# else
#  define __BIG_ENDIAN__
# endif
#endif

#undef max
#undef min

#endif // _COMMON_GCC_H
