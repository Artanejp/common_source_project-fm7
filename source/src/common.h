/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ common header ]
*/

#ifndef _COMMON_H_
#define _COMMON_H_

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
		#define DLL_PREFIX   __declspec(dllexport)
		#define DLL_PREFIX_I __declspec(dllimport)
	#else
		#define CSP_OS_GCC_GENERIC
		#define CSP_OS_GENERIC
		#define DLL_PREFIX
		#define DLL_PREFIX_I
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

typedef union {
	struct {
#ifdef __BIG_ENDIAN__
		uint8_t h3, h2, h, l;
#else
		uint8_t l, h, h2, h3;
#endif
	} b;
	struct {
#ifdef __BIG_ENDIAN__
		int8_t h3, h2, h, l;
#else
		int8_t l, h, h2, h3;
#endif
	} sb;
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
	uint32_t d;
	int32_t sd;
	inline void read_2bytes_le_from(uint8_t *t)
	{
		b.l = t[0]; b.h = t[1]; b.h2 = b.h3 = 0;
	}
	inline void write_2bytes_le_to(uint8_t *t)
	{
		t[0] = b.l; t[1] = b.h;
	}
	inline void read_2bytes_be_from(uint8_t *t)
	{
		b.h3 = b.h2 = 0; b.h = t[0]; b.l = t[1];
	}
	inline void write_2bytes_be_to(uint8_t *t)
	{
		t[0] = b.h; t[1] = b.l;
	}
	inline void read_4bytes_le_from(uint8_t *t)
	{
		b.l = t[0]; b.h = t[1]; b.h2 = t[2]; b.h3 = t[3];
	}
	inline void write_4bytes_le_to(uint8_t *t)
	{
		t[0] = b.l; t[1] = b.h; t[2] = b.h2; t[3] = b.h3;
	}
	inline void read_4bytes_be_from(uint8_t *t)
	{
		b.h3 = t[0]; b.h2 = t[1]; b.h = t[2]; b.l = t[3];
	}
	inline void write_4bytes_be_to(uint8_t *t)
	{
		t[0] = b.h3; t[1] = b.h2; t[2] = b.h; t[3] = b.l;
	}
} pair_t;

uint32_t DLL_PREFIX EndianToLittle_DWORD(uint32_t x);
uint16_t DLL_PREFIX EndianToLittle_WORD(uint16_t x);

// max/min
#ifndef _MSC_VER
	#undef max
	#undef min
	int max(int a, int b);
	unsigned int max(unsigned int a, unsigned int b);
	int min(int a, int b);
	unsigned int min(unsigned int a, unsigned int b);
#endif

// string
#if defined(__GNUC__) || defined(__CYGWIN__) || defined(Q_OS_CYGWIN)
	#define stricmp(a,b) strcasecmp(a,b)
	#define strnicmp(a,b,n) strncasecmp(a,b,n)
#endif

#ifndef SUPPORT_TCHAR_TYPE
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
//	errno_t my_tfopen_s(FILE** pFile, const _TCHAR *filename, const _TCHAR *mode);
	errno_t DLL_PREFIX my_strcpy_s(char *strDestination, size_t numberOfElements, const char *strSource);
	errno_t DLL_PREFIX my_tcscpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource);
	errno_t DLL_PREFIX my_strncpy_s(char *strDestination, size_t numberOfElements, const char *strSource, size_t count);
	errno_t DLL_PREFIX my_tcsncpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource, size_t count);
	char * DLL_PREFIX my_strtok_s(char *strToken, const char *strDelimit, char **context);
	_TCHAR * DLL_PREFIX my_tcstok_s(_TCHAR *strToken, const char *strDelimit, _TCHAR **context);
	#define my_fprintf_s fprintf
	int DLL_PREFIX my_sprintf_s(char *buffer, size_t sizeOfBuffer, const char *format, ...);
	int DLL_PREFIX my_stprintf_s(_TCHAR *buffer, size_t sizeOfBuffer, const _TCHAR *format, ...);
	int DLL_PREFIX my_vsprintf_s(char *buffer, size_t numberOfElements, const char *format, va_list argptr);
	int DLL_PREFIX my_vstprintf_s(_TCHAR *buffer, size_t numberOfElements, const _TCHAR *format, va_list argptr);
#else
//	#define my_tfopen_s _tfopen_s
	#define my_strcpy_s strcpy_s
	#define my_tcscpy_s _tcscpy_s
	#define my_strncpy_s strncpy_s
	#define my_tcsncpy_s _tcsncpy_s
	#define my_strtok_s strtok_s
	#define my_tcstok_s _tcstok_s
	#define my_fprintf_s fprintf_s
	#define my_sprintf_s sprintf_s
	#define my_stprintf_s _stprintf_s
	#define my_vsprintf_s vsprintf_s
	#define my_vstprintf_s _vstprintf_s
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

#if defined(_RGB555) || defined(_RGB565)
	typedef uint16_t scrntype_t;
	scrntype_t RGB_COLOR(uint32_t r, uint32_t g, uint32_t b);
	scrntype_t RGBA_COLOR(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
	uint8_t R_OF_COLOR(scrntype_t c);
	uint8_t G_OF_COLOR(scrntype_t c);
	uint8_t B_OF_COLOR(scrntype_t c);
	uint8_t A_OF_COLOR(scrntype_t c);
#elif defined(_RGB888)
	typedef uint32_t scrntype_t;
	#define RGB_COLOR(r, g, b)	(((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | ((uint32_t)(b) << 0))
	#define RGBA_COLOR(r, g, b, a)	(((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | ((uint32_t)(b) << 0) | ((uint32_t)(a) << 24))
	#define R_OF_COLOR(c)		(((c) >> 16) & 0xff)
	#define G_OF_COLOR(c)		(((c) >>  8) & 0xff)
	#define B_OF_COLOR(c)		(((c)      ) & 0xff)
	#define A_OF_COLOR(c)		(((c) >> 24) & 0xff)
#endif

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

// file path
const _TCHAR *get_application_path();
const _TCHAR *create_local_path(const _TCHAR *format, ...);
void create_local_path(_TCHAR *file_path, int length, const _TCHAR *format, ...);
const _TCHAR *create_date_file_path(const _TCHAR *extension);
void create_date_file_path(_TCHAR *file_path, int length, const _TCHAR *extension);
bool check_file_extension(const _TCHAR *file_path, const _TCHAR *ext);
const _TCHAR *get_file_path_without_extensiton(const _TCHAR *file_path);
void get_long_full_path_name(const _TCHAR* src, _TCHAR* dst, size_t dst_len);
const _TCHAR* get_parent_dir(const _TCHAR* file);

// misc
const _TCHAR * DLL_PREFIX create_string(const _TCHAR* format, ...);
uint32_t DLL_PREFIX get_crc32(uint8_t data[], int size);
uint16_t DLL_PREFIX jis_to_sjis(uint16_t jis);

int DLL_PREFIX decibel_to_volume(int decibel);
int32_t DLL_PREFIX apply_volume(int32_t sample, int volume);

#define array_length(array) (sizeof(array) / sizeof(array[0]))

#define FROM_BCD(v)	(((v) & 0x0f) + (((v) >> 4) & 0x0f) * 10)
#define TO_BCD(v)	((int)(((v) % 100) / 10) << 4) | ((v) % 10)
#define TO_BCD_LO(v)	((v) % 10)
#define TO_BCD_HI(v)	(int)(((v) % 100) / 10)

#define LEAP_YEAR(y)	(((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

typedef DLL_PREFIX struct cur_time_s {
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

void DLL_PREFIX get_host_time(cur_time_t* cur_time);

#endif
