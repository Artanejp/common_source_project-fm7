/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ common header ]
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#define SUPPORT_TCHAR_TYPE
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define SUPPORT_SECURE_FUNCTIONS
#endif
#ifdef SUPPORT_TCHAR_TYPE
 #include <tchar.h>
#endif
#include <stdio.h>



#if defined(_USE_AGAR) || defined(_USE_SDL)
#include <SDL/SDL.h>
#include <agar/core.h>
#include <stdarg.h>
#elif defined(_USE_QT)
#include <SDL2/SDL.h>
#include <stdarg.h>
#include <QtCore/QString>
#include <QtCore/QFile>
#endif

#if defined(_USE_AGAR) || defined(_USE_SDL) || defined(_USE_QT)

#ifndef uint8
   typedef uint8_t uint8;
# endif
# ifndef int8
   typedef int8_t int8;
# endif
# ifndef uint16
   typedef uint16_t uint16;
# endif
# ifndef int16
   typedef int16_t int16;
# endif
# ifndef uint32
   typedef uint32_t uint32;
# endif
# ifndef int32
   typedef int32_t int32;
# endif
# ifndef uint64
   typedef uint64_t uint64;
# endif
# ifndef int64
   typedef int64_t int64;
# endif
# ifndef BOOL
   typedef int BOOL;
# endif
# ifndef BYTE
   typedef uint8_t BYTE;
# endif
# ifndef WORD
   typedef uint16_t WORD;
# endif
# ifndef DWORD
   typedef uint32_t DWORD;
# endif
# ifndef QWORD
   typedef uint64_t QWORD;
# endif

# ifndef UINT8
   typedef uint8_t UINT8;
# endif
# ifndef UINT16
   typedef uint16_t UINT16;
# endif
# ifndef UINT32
   typedef uint32_t UINT32;
# endif
# ifndef UINT64
   typedef uint64_t UINT64;
# endif

# ifndef INT8
   typedef int8_t INT8;
# endif
# ifndef INT16
   typedef int16_t INT16;
# endif
# ifndef INT32
   typedef int32_t INT32;
# endif
# ifndef INT64
   typedef int64_t INT64;
# endif

static inline void _stprintf(char *s, const char *fmt, ...) {
   va_list args;
   
   va_start(args, fmt);
   sprintf(s, fmt, args);
   va_end(args);
}
#define stricmp(a,b) strcmp(a,b)
#define strnicmp(a,b,n) strncmp(a,b,n)


// tchar.h
//#  ifdef  _UNICODE
//#    define __T(x)      L ## x
//#  else
#    define __T(x)      x
//#  endif
 
#  define _T(x)       __T(x)
#  define _TEXT(x)    __T(x)

//#  ifdef _UNICODE
//    typedef wchar_t _TCHAR;
//#  else
    typedef char    _TCHAR;
//#  endif

#  ifndef LPCTSTR
    typedef _TCHAR* LPCTSTR;
#  endif


static inline char *_tcsncpy(_TCHAR *d, _TCHAR *s, int n) {
   return strncpy((char *)d, (char *)s, n);
}

static inline char *_tcsncat(_TCHAR *d, _TCHAR *s, int n) {
   return strncat((char *)d, (char *)s, n);
}


static inline int DeleteFile(_TCHAR *path) 
{
#ifdef _USE_QT
       QString fpath = (char *)path;
       QFile tfp(fpath);
       if(tfp.remove(fpath)) return (int)true;
       return 0;
#else   
   return AG_FileDelete((const char *)path);
#endif
}
#include <algorithm>

#  ifdef USE_GETTEXT
#  include <libintl.h>
#  define _N(x) gettext(x)
# else
#  define _N(x) _T(x)
# endif

#if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
static inline DWORD EndianToLittle_DWORD(DWORD x)
{
   return x;
}

static inline WORD EndianToLittle_WORD(WORD x)
{
   return x;
}
#else // BIG_ENDIAN
static inline DWORD EndianToLittle_DWORD(DWORD x)
{
   DWORD y;
   y = ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) |
       ((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24);
   return y;
}

static inline WORD EndianToLittle_WORD(WORD x)
{
   WORD y;
   y = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
   return y;
}
#endif
#define ZeroMemory(p,s) memset(p,0x00,s)
#define CopyMemory(t,f,s) memcpy(t,f,s)

#ifdef __cplusplus
extern "C" 
{
#endif
//extern void Sleep(uint32_t tick);
//extern uint32_t timeGetTime(void);
#ifdef __cplusplus
}
#endif

#else
#include <tchar.h>

// variable scope of 'for' loop for Microsoft Visual C++ 6.0

#if defined(_MSC_VER) && (_MSC_VER == 1200)
#define for if(0);else for
#endif

// disable warnings  for microsoft visual c++ 2005 or later
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4819 )
//#pragma warning( disable : 4995 )
//#pragma warning( disable : 4996 )
#endif

// endian
#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
	#if defined(__BYTE_ORDER) && (defined(__LITTLE_ENDIAN) || defined(__BIG_ENDIAN))
		#if __BYTE_ORDER == __LITTLE_ENDIAN
			#define __LITTLE_ENDIAN__
		#elif __BYTE_ORDER == __BIG_ENDIAN
			#define __BIG_ENDIAN__
		#endif
	#elif defined(WORDS_LITTLEENDIAN)
		#define __LITTLE_ENDIAN__
	#elif defined(WORDS_BIGENDIAN)
		#define __BIG_ENDIAN__
	#endif
#endif
#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
	// Microsoft Visual C++
	#define __LITTLE_ENDIAN__
 #endif


// type definition
#ifndef uint8
typedef unsigned char uint8;
#endif
#ifndef uint16
typedef unsigned short uint16;
#endif
#ifndef uint32
typedef unsigned int uint32;
#endif
#ifndef uint64
#ifdef _MSC_VER
typedef unsigned __int64 uint64;
#else
typedef unsigned long long uint64;
#endif
#endif

#ifndef int8
typedef signed char int8;
#endif
#ifndef int16
typedef signed short int16;
#endif
#ifndef int32
typedef signed int int32;
#endif
#ifndef int64
#ifdef _MSC_VER
typedef signed __int64 int64;
#else
typedef signed long long int64;
#endif
#endif

static inline DWORD EndianToLittle_DWORD(DWORD x)
{
   return x;
}

static inline WORD EndianToLittle_WORD(WORD x)
{
   return x;
}


#endif


typedef union {
#ifdef __BIG_ENDIAN__
	struct {
		uint8 h3, h2, h, l;
	} b;
	struct {
		int8 h3, h2, h, l;
	} sb;
	struct {
		uint16 h, l;
	} w;
	struct {
		int16 h, l;
	} sw;
#else
	struct {
		uint8 l, h, h2, h3;
	} b;
	struct {
		uint16 l, h;
	} w;
	struct {
		int8 l, h, h2, h3;
	} sb;
	struct {
		int16 l, h;
	} sw;
#endif
	uint32 d;
	int32 sd;
} pair;

// rgb color
//#define _RGB888
#define _RGBA888

#if defined(_USE_AGAR)

# if AG_BYTEORDER == AG_BIG_ENDIAN
# if defined(_RGB555)
//#define RGB_COLOR(r, g, b) ((uint16)(((uint16)(b) & 0xf8) << 7) | (uint16)(((uint16)(g) & 0xf8) << 2) | (uint16)(((uint16)(r) & 0xf8) >> 3))
#  define RGB_COLOR(r, g, b) ((uint16)(((uint16)(b) & 0xf8) >>4) | (uint16)(((uint16)(g) & 0xf8) << 2) | (uint16)(((uint16)(r) & 0xf8) << 8))
typedef uint16 scrntype;
# elif defined(_RGB565)
//#define RGB_COLOR(r, g, b) ((uint16)(((uint16)(b) & 0xf8) << 8) | (uint16)(((uint16)(g) & 0xfc) << 3) | (uint16)(((uint16)(r) & 0xf8) >> 3))
#  define RGB_COLOR(r, g, b) ((uint16)(((uint16)(b) & 0xf8) >>3) | (uint16)(((uint16)(g) & 0xfc) << 2) | (uint16)(((uint16)(r) & 0xf8) << 8))
typedef uint16 scrntype;
# elif defined(_RGB888) 
#  define RGB_COLOR(r, g, b) (((uint32)(r) << 24) | ((uint32)(g) << 16) | ((uint32)(b) << 8))
typedef uint32 scrntype;
# elif defined(_RGBA888) 
#  define RGB_COLOR(r, g, b) (((uint32)(r) << 24) | ((uint32)(g) << 16) | ((uint32)(b) << 8)) | ((uint32)0xff << 0)
typedef uint32 scrntype;
# endif

#else // LITTLE ENDIAN

# if defined(_RGB555)
#  define RGB_COLOR(r, g, b) ((uint16)(((uint16)(b) & 0xf8) << 7) | (uint16)(((uint16)(g) & 0xf8) << 2) | (uint16)(((uint16)(r) & 0xf8) >> 3))
typedef uint16 scrntype;
# elif defined(_RGB565)
#  define RGB_COLOR(r, g, b) ((uint16)(((uint16)(b) & 0xf8) << 8) | (uint16)(((uint16)(g) & 0xfc) << 3) | (uint16)(((uint16)(r) & 0xf8) >> 3))
typedef uint16 scrntype;
# elif defined(_RGB888)
#  define RGB_COLOR(r, g, b) (((uint32)(r) << 0) | ((uint32)(g) << 8) | ((uint32)(b) << 16))
typedef uint32 scrntype;
# elif defined(_RGBA888)
#  define RGB_COLOR(r, g, b) (((uint32)(r) << 0) | ((uint32)(g) << 8) | ((uint32)(b) << 16)) | ((uint32)0xff << 24)
typedef uint32 scrntype;
# endif

#endif  // ENDIAN

#else // NOT USE AGAR

# if defined(_RGB555)
#define RGB_COLOR(r, g, b) ((uint16)(((uint16)(r) & 0xf8) << 7) | (uint16)(((uint16)(g) & 0xf8) << 2) | (uint16)(((uint16)(b) & 0xf8) >> 3))
typedef uint16 scrntype;
#elif defined(_RGB565)
#define RGB_COLOR(r, g, b) ((uint16)(((uint16)(r) & 0xf8) << 8) | (uint16)(((uint16)(g) & 0xfc) << 3) | (uint16)(((uint16)(b) & 0xf8) >> 3))
typedef uint16 scrntype;
#elif defined(_RGB888)
#define RGB_COLOR(r, g, b) (((uint32)(r) << 16) | ((uint32)(g) << 8) | ((uint32)(b) << 0))
typedef uint32 scrntype;
#elif defined(_RGBA888)
#define RGB_COLOR(r, g, b) (((uint32)(r) << 16) | ((uint32)(g) << 8) | ((uint32)(b) << 0)) | ((uint32)0xff << 24)
typedef uint32 scrntype;
#endif

#endif

// _TCHAR
#ifndef SUPPORT_TCHAR_TYPE
typedef char _TCHAR;
//#define _T(s) (s)
#define _tfopen fopen
#define _tcscmp strcmp
#define _tcscpy strcpy
#define _tcsicmp stricmp
#define _tcslen strlen
#define _tcsncat strncat
#define _tcsncpy strncpy
#define _tcsncicmp strnicmp
#define _tcsstr strstr
#define _tcstok strtok
#define _tcstol strtol
#define _stprintf sprintf
#define _vstprintf vsprintf
#endif

#if !defined(_MSC_VER)
#include <errno.h>
typedef int errno_t;
#endif
// secture functions
#ifndef SUPPORT_SECURE_FUNCTIONS
errno_t _tfopen_s(FILE** pFile, const _TCHAR *filename, const _TCHAR *mode);
errno_t _strcpy_s(char *strDestination, size_t numberOfElements, const char *strSource);
errno_t _tcscpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource);
_TCHAR *_tcstok_s(_TCHAR *strToken, const char *strDelimit, _TCHAR **context);
int _stprintf_s(_TCHAR *buffer, size_t sizeOfBuffer, const _TCHAR *format, ...);
int _vstprintf_s(_TCHAR *buffer, size_t numberOfElements, const _TCHAR *format, va_list argptr);
#else
#define _strcpy_s strcpy_s
#endif


#if defined(_USE_SDL) || defined(_USE_AGAR) || defined(_USE_QT)
// misc
#ifdef __cplusplus
bool check_file_extension(_TCHAR* file_path, _TCHAR* ext);
_TCHAR *get_file_path_without_extensiton(_TCHAR* file_path);
uint32 getcrc32(uint8 data[], int size);


#define array_length(array) (sizeof(array) / sizeof(array[0]))

#define FROM_BCD(v)	(((v) & 0x0f) + (((v) >> 4) & 0x0f) * 10)
#define TO_BCD(v)	((int)(((v) % 100) / 10) << 4) | ((v) % 10)
#define TO_BCD_LO(v)	((v) % 10)
#define TO_BCD_HI(v)	(int)(((v) % 100) / 10)

#define LEAP_YEAR(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

typedef struct cur_time_t {
	int year, month, day, day_of_week, hour, minute, second;
	bool initialized;
	cur_time_t()
	{
		initialized = false;
	}
	void increment();
	void update_year();
	void update_day_of_week();
	void save_state(void *f);
	bool load_state(void *f);
} cur_time_t;
#endif

#endif

#endif