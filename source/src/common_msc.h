/*
 * common.h → common_msc.h
 * Defines for gcc excepts Windows.
 * (c) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * History :
 *     Nov 20,2015 : Initial
 */

#ifndef _COMMON_MSC_H
#define _COMMON_MSC_H


#include <stdarg.h>

#if defined(MSC_VER) 
# define CSP_OS_VISUALC
# define CSP_OS_WINDOWS
#endif

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


#ifndef SUPPORT_TCHAR_TYPE
	#ifndef _TCHAR
		typedef char _TCHAR;
	#endif
#endif

#ifndef _MSC_VER
	#ifndef LPTSTR
		typedef _TCHAR* LPTSTR;
	#endif
	#ifndef LPCTSTR
		typedef const _TCHAR* LPCTSTR;
	#endif
#endif
//static inline char *_tcsncpy(_TCHAR *d, _TCHAR *s, int n) {
//   return strncpy((char *)d, (char *)s, n);
//}

//static inline char *_tcsncat(_TCHAR *d, _TCHAR *s, int n) {
//   return strncat((char *)d, (char *)s, n);
//}
//static int DeleteFile(_TCHAR *path) 
//{
//       QString fpath = (char *)path;
//       QFile tfp(fpath);
//       if(tfp.remove(fpath)) return (int)true;
//       return 0;
//}

//#include <algorithm>

# if !defined(CSP_OS_WINDOWS)
#  define _N(x) _T(x)
# else
#  include <tchar.h>
# endif

// How define for ARMEB?
#define __LITTLE_ENDIAN___
#undef __BIG_ENDIAN___

# if defined(__LITTLE_ENDIAN__)
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

#endif // _COMMON_GCC_H
