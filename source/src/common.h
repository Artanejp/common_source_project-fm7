/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ common header ]
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <tchar.h>

// variable scope of 'for' loop for microsoft visual c++ 6.0 and embedded visual c++ 4.0
#if defined(_MSC_VER) && (_MSC_VER == 1200)
#define for if(0);else for
#endif

// disable warnings C4189, C4995 and C4996 for microsoft visual c++ 2005
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4819 )
#pragma warning( disable : 4995 )
#pragma warning( disable : 4996 )
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

typedef union {
#ifdef _BIG_ENDIAN
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
#define _RGB888

#if defined(_RGB555)
#define RGB_COLOR(r, g, b) ((uint16)(((uint16)(r) & 0xf8) << 7) | (uint16)(((uint16)(g) & 0xf8) << 2) | (uint16)(((uint16)(b) & 0xf8) >> 3))
typedef uint16 scrntype;
#elif defined(_RGB565)
#define RGB_COLOR(r, g, b) ((uint16)(((uint16)(r) & 0xf8) << 8) | (uint16)(((uint16)(g) & 0xfc) << 3) | (uint16)(((uint16)(b) & 0xf8) >> 3))
typedef uint16 scrntype;
#elif defined(_RGB888)
#define RGB_COLOR(r, g, b) (((uint32)(r) << 16) | ((uint32)(g) << 8) | ((uint32)(b) << 0))
typedef uint32 scrntype;
#endif

// misc
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
