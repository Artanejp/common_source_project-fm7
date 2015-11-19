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

#if defined(_MSC_VER)
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <process.h>
#endif

#if defined(_USE_QT)
#include <SDL.h>
#include <stdarg.h>
#include "qt_input.h"
# if defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
#  include <tchar.h>
# endif
# if defined(_USE_QT5)
#  include <QString>
#  include <QFile>
#  include <QtEndian>
# else
#  include <QtCore/QString>
#  include <QtCore/QFile>
# endif
#endif



#if defined(_USE_AGAR) || defined(_USE_SDL) || defined(_USE_QT)

#  ifndef uint8
   typedef uint8_t uint8;
#  endif
#  ifndef int8
   typedef int8_t int8;
#  endif
#  ifndef uint16
   typedef uint16_t uint16;
#  endif
#  ifndef int16
   typedef int16_t int16;
#  endif
#  ifndef uint32
   typedef uint32_t uint32;
#  endif
#  ifndef int32
   typedef int32_t int32;
#  endif
#  ifndef uint64
   typedef uint64_t uint64;
#  endif
#  ifndef int64
   typedef int64_t int64;
#  endif
#  ifndef BOOL
   typedef int BOOL;
#  endif
# if !defined(Q_OS_CYGWIN) && !defined(Q_OS_WIN)
#  ifndef BYTE
   typedef uint8_t BYTE;
#  endif
#  ifndef WORD
   typedef uint16_t WORD;
#  endif
#  ifndef DWORD
   typedef uint32_t DWORD;
#  endif
#  ifndef QWORD
   typedef uint64_t QWORD;
#  endif
# endif
# if !defined(Q_OS_CYGWIN)
#  ifndef UINT8
   typedef uint8_t UINT8;
#  endif
#  ifndef UINT16
   typedef uint16_t UINT16;
#  endif
#  ifndef UINT32
   typedef uint32_t UINT32;
#  endif
#  ifndef UINT64
   typedef uint64_t UINT64;
#  endif

#  ifndef INT8
   typedef int8_t INT8;
#  endif
#  ifndef INT16
   typedef int16_t INT16;
#  endif
#  ifndef INT32
   typedef int32_t INT32;
#  endif
#  ifndef INT64
   typedef int64_t INT64;
#  endif

# if !defined(Q_OS_CYGWIN) && !defined(Q_OS_WIN)
static inline void _stprintf(char *s, const char *fmt, ...) {
   va_list args;
   
   va_start(args, fmt);
   sprintf(s, fmt, args);
   va_end(args);
}
# endif
# define stricmp(a,b) strcmp(a,b)
# define strnicmp(a,b,n) strncmp(a,b,n)


#    define __T(x)      x
 
#  define _T(x)       __T(x)
#  define _TEXT(x)    __T(x)

    typedef char    _TCHAR;

# if !defined(Q_OS_CYGWIN) && !defined(Q_OS_WIN)
#  ifndef LPCTSTR
    typedef _TCHAR* LPCTSTR;
#  endif
static inline char *_tcsncpy(_TCHAR *d, _TCHAR *s, int n) {
   return strncpy((char *)d, (char *)s, n);
}

static inline char *_tcsncat(_TCHAR *d, _TCHAR *s, int n) {
   return strncat((char *)d, (char *)s, n);
}
# endif

# endif
#endif

#if defined(_USE_QT)
# if defined(Q_OS_CYGWIN)
# define stricmp(a,b) strcmp(a,b)
# define strnicmp(a,b,n) strncmp(a,b,n)
# endif
# if defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)

   typedef char    _TCHAR;
# endif
static int DeleteFile(_TCHAR *path) 
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

# ifndef Q_OS_WIN
#  define _N(x) _T(x)
# endif
#endif

#if defined(_USE_QT)
# if defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
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
#endif
#define ZeroMemory(p,s) memset(p,0x00,s)
#define CopyMemory(t,f,s) memcpy(t,f,s)


# if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
# if SDL_BYTEORDER == SDL_LIL_ENDIAN
#  define __LITTLE_ENDIAN__
# else
#  define __BIG_ENDIAN__
# endif
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
#pragma warning( disable : 4996 )
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
	struct {
#ifdef __BIG_ENDIAN__
		uint8 h3, h2, h, l;
#else
		uint8 l, h, h2, h3;
#endif
	} b;
	struct {
#ifdef __BIG_ENDIAN__
		int8 h3, h2, h, l;
#else
		int8 l, h, h2, h3;
#endif
	} sb;
	struct {
#ifdef __BIG_ENDIAN__
		uint16 h, l;
#else
		uint16 l, h;
#endif
	} w;
	struct {
#ifdef __BIG_ENDIAN__
		int16 h, l;
#else
		int16 l, h;
#endif
	} sw;
	uint32 d;
	int32 sd;
	inline void read_2bytes_le_from(uint8 *t)
	{
		b.l = t[0]; b.h = t[1]; b.h2 = b.h3 = 0;
	}
	inline void write_2bytes_le_to(uint8 *t)
	{
		t[0] = b.l; t[1] = b.h;
	}
	inline void read_2bytes_be_from(uint8 *t)
	{
		b.h3 = b.h2 = 0; b.h = t[0]; b.l = t[1];
	}
	inline void write_2bytes_be_to(uint8 *t)
	{
		t[0] = b.h; t[1] = b.l;
	}
	inline void read_4bytes_le_from(uint8 *t)
	{
		b.l = t[0]; b.h = t[1]; b.h2 = t[2]; b.h3 = t[3];
	}
	inline void write_4bytes_le_to(uint8 *t)
	{
		t[0] = b.l; t[1] = b.h; t[2] = b.h2; t[3] = b.h3;
	}
	inline void read_4bytes_be_from(uint8 *t)
	{
		b.h3 = t[0]; b.h2 = t[1]; b.h = t[2]; b.l = t[3];
	}
	inline void write_4bytes_be_to(uint8 *t)
	{
		t[0] = b.h3; t[1] = b.h2; t[2] = b.h; t[3] = b.l;
	}

} pair;

// max/min from WinDef.h
#if !defined(MSC_VER)
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
//#endif
#endif
// rgb color
#if !defined(_USE_QT)
#define _RGB888
#else
#define _RGBA888
#endif

#if defined(__BIG_ENDIAN__)
# if defined(_RGB555)
#  define RGB_COLOR(r, g, b) ((uint16)(((uint16)(b) & 0xf8) >>4) | (uint16)(((uint16)(g) & 0xf8) << 2) | (uint16)(((uint16)(r) & 0xf8) << 8))
typedef uint16 scrntype;
# elif defined(_RGB565)
#  define RGB_COLOR(r, g, b) ((uint16)(((uint16)(b) & 0xf8) >>3) | (uint16)(((uint16)(g) & 0xfc) << 2) | (uint16)(((uint16)(r) & 0xf8) << 8))
typedef uint16 scrntype;
# elif defined(_RGB888) 
#  define RGB_COLOR(r, g, b) (((uint32)(r) << 24) | ((uint32)(g) << 16) | ((uint32)(b) << 8))
typedef uint32 scrntype;
# elif defined(_RGBA888)
//#   define RGB_COLOR(r, g, b) (((uint32)(r) << 24) | ((uint32)(g) << 16) | ((uint32)(b) << 8)) | ((uint32)0xff << 0)
#   define RGB_COLOR(r, g, b) (((uint32)(b) << 24) | ((uint32)(g) << 16) | ((uint32)(r) << 8)) | ((uint32)0xff << 0)
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
#  define RGB_COLOR(r, g, b) (((uint32)(r) << 16) | ((uint32)(g) << 8) | ((uint32)(b) << 0))
typedef uint32 scrntype;
# elif defined(_RGBA888)
//#   define RGB_COLOR(r, g, b) (((uint32)(r) << 16) | ((uint32)(g) << 8) | ((uint32)(b) << 0)) | ((uint32)0xff << 24)
#   define RGB_COLOR(r, g, b) (((uint32)(b) << 16) | ((uint32)(g) << 8) | ((uint32)(r) << 0)) | ((uint32)0xff << 24)
typedef uint32 scrntype;
# endif

#endif  // ENDIAN


// _TCHAR
#ifndef SUPPORT_TCHAR_TYPE
typedef char _TCHAR;
//#define _T(s) (s)
#define _tfopen fopen
#define _tcscmp strcmp
#define _tcscpy strcpy
# if !defined(_tcsicmp)
# define _tcsicmp stricmp
# endif
#define _tcslen strlen
#define _tcsncat strncat
#define _tcsncpy strncpy
# if !defined(_tcsncicmp)
#define _tcsncicmp strnicmp
# endif
#define _tcsstr strstr
#define _tcstok strtok
#define _tcstol strtol
#define _tcstoul strtoul
#define _stprintf sprintf
#define _vstprintf vsprintf
#endif

#if !defined(_MSC_VER)
#include <errno.h>
typedef int errno_t;
#endif
// secture functions
#if !defined(SUPPORT_SECURE_FUNCTIONS) || defined(Q_OS_WIN)
# ifndef errno_t
typedef int errno_t;
# endif
# if defined(Q_OS_WIN)
#  define strcpy_s _strcpy_s
#  define tcscpy_s _tcscpy_s
#  define tcstok_s _tcstok_s
#  define stprintf_s _stprintf_s
#  define vstprintf_s _vstprintf_s
#  define vsprintf_s _vsprintf_s
# endif
errno_t _strcpy_s(char *strDestination, size_t numberOfElements, const char *strSource);
# if !defined(Q_OS_WIN) //&& !defined(Q_OS_CYGWIN)
errno_t _tcscpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource);
_TCHAR *_tcstok_s(_TCHAR *strToken, const char *strDelimit, _TCHAR **context);
# endif
int _stprintf_s(_TCHAR *buffer, size_t sizeOfBuffer, const _TCHAR *format, ...);
int _vstprintf_s(_TCHAR *buffer, size_t numberOfElements, const _TCHAR *format, va_list argptr);
#else
# define _strcpy_s strcpy_s
#endif

// wav file header
#pragma pack(1)
typedef struct {
	char id[4];
	uint32 size;
} wav_chunk_t;
#pragma pack()

#pragma pack(1)
typedef struct {
	wav_chunk_t riff_chunk;
	char wave[4];
	wav_chunk_t fmt_chunk;
	uint16 format_id;
	uint16 channels;
	uint32 sample_rate;
	uint32 data_speed;
	uint16 block_size;
	uint16 sample_bits;
} wav_header_t;
#pragma pack()


// misc
#ifdef __cplusplus
bool check_file_extension(const _TCHAR* file_path, const _TCHAR* ext);
_TCHAR *get_file_path_without_extensiton(const _TCHAR* file_path);
uint32 getcrc32(uint8 data[], int size);


#define array_length(array) (sizeof(array) / sizeof(array[0]))

#define FROM_BCD(v)	(((v) & 0x0f) + (((v) >> 4) & 0x0f) * 10)
#define TO_BCD(v)	((int)(((v) % 100) / 10) << 4) | ((v) % 10)
#define TO_BCD_LO(v)	((v) % 10)
#define TO_BCD_HI(v)	(int)(((v) % 100) / 10)

#define LEAP_YEAR(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

typedef struct cur_time_s {
	int year, month, day, day_of_week, hour, minute, second;
	bool initialized;
	cur_time_s()
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

//#endif

#endif
