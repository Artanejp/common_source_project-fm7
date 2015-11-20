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
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;
static inline void _stprintf(char *s, const char *fmt, ...) {
   va_list args;
   
   va_start(args, fmt);
   sprintf(s, fmt, args);
   va_end(args);
}
# endif // Generic

# define stricmp(a,b) strcmp(a,b)
# define strnicmp(a,b,n) strncmp(a,b,n)

# define __T(x)      x
 
# define _T(x)       __T(x)
# define _TEXT(x)    __T(x)

typedef char    _TCHAR;
# if defined(CSP_OS_GCC_GENERIC)
typedef _TCHAR* LPCTSTR;
# endif
static inline char *_tcsncpy(_TCHAR *d, _TCHAR *s, int n) {
   return strncpy((char *)d, (char *)s, n);
}

static inline char *_tcsncat(_TCHAR *d, _TCHAR *s, int n) {
   return strncat((char *)d, (char *)s, n);
}
# if defined(CSP_OS_GCC_CYGWIN)
#  define stricmp(a,b) strcmp(a,b)
#  define strnicmp(a,b,n) strncmp(a,b,n)
# endif

static int DeleteFile(_TCHAR *path) 
{
       QString fpath = (char *)path;
       QFile tfp(fpath);
       if(tfp.remove(fpath)) return (int)true;
       return 0;
}

#include <algorithm>

# if !defined(CSP_OS_WINDOWS)
#  define _N(x) _T(x)
# else
#  include <tchar.h>
# endif

#undef __LITTLE_ENDIAN___
#undef __BIG_ENDIAN___

# if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
#  define __LITTLE_ENDIAN__
static inline uint32_t EndianToLittle_DWORD(uint32_t x)
{
   return x;
}

static inline uint16_t EndianToLittle_WORD(uint16_t x)
{
   return x;
}
# else // BIG_ENDIAN
#  define __BIG_ENDIAN__
static inline uint32_t EndianToLittle_DWORD(uint32_t x)
{
   uint32_t y;
   y = ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) |
       ((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24);
   return y;
}

static inline uint16_t EndianToLittle_WORD(uint16_t x)
{
   uint16_t y;
   y = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
   return y;
}
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

//#ifndef max
static inline int max(int a, int b) {
	return (((a) > (b)) ? (a) : (b));
}
static inline uint32_t max(uint32_t a, uint32_t b) {
	return (((a) > (b)) ? (a) : (b));
}

static inline int min(int a, int b) {
	return (((a) < (b)) ? (a) : (b));
}
static inline uint32_t min(uint32_t a, uint32_t b) {
	return (((a) < (b)) ? (a) : (b));
}


#endif // _COMMON_GCC_H
