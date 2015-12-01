/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2013.01.17-

	[ common ]
*/
#if defined(_USE_QT)
#include <string.h>
#include "common.h"
#include "config.h"
#else
# include <windows.h>
# include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include "common.h"
#endif

#ifdef MAX_MACRO_NOT_DEFINED
inline int max(int a, int b)
{
	if(a > b) {
		return a;
	} else {
		return b;
	}
}
inline unsigned int max(unsigned int a, unsigned int b)
{
	if(a > b) {
		return a;
	} else {
		return b;
	}
}
#endif

#ifdef MIN_MACRO_NOT_DEFINED
inline int min(int a, int b)
{
	if(a < b) {
		return a;
	} else {
		return b;
	}
}
inline unsigned int min(unsigned int a, unsigned int b)
{
	if(a < b) {
		return a;
	} else {
		return b;
	}
}
#endif

#ifndef SUPPORT_SECURE_FUNCTIONS
//errno_t my_tfopen_s(FILE** pFile, const _TCHAR *filename, const _TCHAR *mode)
//{
//	if((*pFile = _tfopen(filename, mode)) != NULL) {
//		return 0;
//	} else {
//		return errno;
//	}
//}

errno_t my_strcpy_s(char *strDestination, size_t numberOfElements, const char *strSource)
{
	strcpy(strDestination, strSource);
	return 0;
}

errno_t my_tcscpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource)
{
	_tcscpy(strDestination, strSource);
	return 0;
}

_TCHAR *my_tcstok_s(_TCHAR *strToken, const char *strDelimit, _TCHAR **context)
{
	return _tcstok(strToken, strDelimit);
}

int my_stprintf_s(_TCHAR *buffer, size_t sizeOfBuffer, const _TCHAR *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = _vstprintf(buffer, format, ap);
	va_end(ap);
	return result;
}

int my_vstprintf_s(_TCHAR *buffer, size_t numberOfElements, const _TCHAR *format, va_list argptr)
{
	return _vstprintf(buffer, format, argptr);
}
#endif

#ifndef _MSC_VER
BOOL MyWritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
	// write your compatible function, if possible in standard C/C++ code
	return FALSE;
}

DWORD MyGetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	// write your compatible function, if possible in standard C/C++ code
	return 0;
}

UINT MyGetPrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName)
{
	// write your compatible function, if possible in standard C/C++ code
	return 0;
}
#endif

#if defined(_RGB555)
scrntype RGB_COLOR(uint r, uint g, uint b)
{
	scrntype rr = ((scrntype)r * 0x1f) / 0xff;
	scrntype gg = ((scrntype)g * 0x1f) / 0xff;
	scrntype bb = ((scrntype)b * 0x1f) / 0xff;
	return (rr << 10) | (gg << 5) | bb;
}
scrntype RGBA_COLOR(uint r, uint g, uint b, uint a)
{
	return RGB_COLOR(r, g, b);
}
uint8 R_OF_COLOR(scrntype c)
{
	c = (c >> 10) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8)c;
}
uint8 G_OF_COLOR(scrntype c)
{
	c = (c >>  5) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8)c;
}
uint8 B_OF_COLOR(scrntype c)
{
	c = (c >>  0) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8)c;
}
uint8 A_OF_COLOR(scrntype c)
{
	return 0;
}
#elif defined(_RGB565)
scrntype RGB_COLOR(uint r, uint g, uint b)
{
	scrntype rr = ((scrntype)r * 0x1f) / 0xff;
	scrntype gg = ((scrntype)g * 0x3f) / 0xff;
	scrntype bb = ((scrntype)b * 0x1f) / 0xff;
	return (rr << 11) | (gg << 5) | bb;
}
scrntype RGBA_COLOR(uint r, uint g, uint b, uint a)
{
	return RGB_COLOR(r, g, b);
}
uint8 R_OF_COLOR(scrntype c)
{
	c = (c >> 11) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8)c;
}
uint8 G_OF_COLOR(scrntype c)
{
	c = (c >>  5) & 0x3f;
	c = (c * 0xff) / 0x3f;
	return (uint8)c;
}
uint8 B_OF_COLOR(scrntype c)
{
	c = (c >>  0) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8)c;
}
uint8 A_OF_COLOR(scrntype c)
{
	return 0;
}
#endif


#include "fileio.h"

bool check_file_extension(const _TCHAR* file_path, const _TCHAR* ext)
{
#if defined(_USE_AGAR)
   	int nam_len = strlen(file_path);
	int ext_len = strlen(ext);
	
	return (nam_len >= ext_len && strncmp(&file_path[nam_len - ext_len], ext, ext_len) == 0);
#elif defined(_USE_QT)
        QString s_fpath = file_path;
        QString s_ext = ext;
        bool f = false;
        s_fpath = s_fpath.toUpper();
        s_ext = s_ext.toUpper();
        if(s_fpath.length() < s_ext.length()) return false;
        s_fpath = s_fpath.right(s_ext.length());
        if(s_fpath == s_ext) return true;
        return false;
#else
   	int nam_len = _tcslen(file_path);
	int ext_len = _tcslen(ext);
	
	return (nam_len >= ext_len && _tcsncicmp(&file_path[nam_len - ext_len], ext, ext_len) == 0);
#endif
}

_TCHAR *get_file_path_without_extensiton(const _TCHAR* file_path)
{
	static _TCHAR path[_MAX_PATH];
	
#if defined(_USE_AGAR) || defined(_USE_QT)
        _TCHAR *p1,  *p2;
        static _TCHAR p3[_MAX_PATH];
        strcpy(path, file_path);
        p1 = (_TCHAR *)strrchr(path, '/');
        p2 = (_TCHAR *)strrchr(path, '.');
        
        if(p2 == NULL) {
		return path;
	} else if(p1 == NULL) {
		strncpy(p3, path, p2 - path);
	   	return p3;
	} else if(p1 > p2) {
		return path;
	} else {
		strncpy(p3, path, p2 - path);
	   	return p3;
	}
   
   
   
#else
	my_tcscpy_s(path, _MAX_PATH, file_path);
        PathRemoveExtension(path);
	return path;
#endif
}

uint32 getcrc32(uint8 data[], int size)
{
	static bool initialized = false;
	static uint32 table[256];
	
	if(!initialized) {
		for(int i = 0; i < 256; i++) {
			uint32 c = i;
			for(int j = 0; j < 8; j++) {
				if(c & 1) {
					c = (c >> 1) ^ 0xedb88320;
				} else {
					c >>= 1;
				}
			}
			table[i] = c;
		}
		initialized = true;
	}
	
	uint32 c = ~0;
	for(int i = 0; i < size; i++) {
		c = table[(c ^ data[i]) & 0xff] ^ (c >> 8);
	}
	return ~c;
}

void cur_time_t::increment()
{
	if(++second >= 60) {
		second = 0;
		if(++minute >= 60) {
			minute = 0;
			if(++hour >= 24) {
				hour = 0;
				// days in this month
				int days = 31;
				if(month == 2) {
					days = LEAP_YEAR(year) ? 29 : 28;
				} else if(month == 4 || month == 6 || month == 9 || month == 11) {
					days = 30;
				}
				if(++day > days) {
					day = 1;
					if(++month > 12) {
						month = 1;
						year++;
					}
				}
				if(++day_of_week >= 7) {
					day_of_week = 0;
				}
			}
		}
	}
}

void cur_time_t::update_year()
{
	// 1970-2069
	if(year < 70) {
		year += 2000;
	} else if(year < 100) {
		year += 1900;
	}
}

void cur_time_t::update_day_of_week()
{
	static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	int y = year - (month < 3);
	day_of_week = (y + y / 4 - y / 100 + y / 400 + t[month - 1] + day) % 7;
}

#define STATE_VERSION	1

void cur_time_t::save_state(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;
	
	state_fio->FputUint32(STATE_VERSION);
	
	state_fio->FputInt32(year);
	state_fio->FputInt32(month);
	state_fio->FputInt32(day);
	state_fio->FputInt32(day_of_week);
	state_fio->FputInt32(hour);
	state_fio->FputInt32(minute);
	state_fio->FputInt32(second);
	state_fio->FputBool(initialized);
}

bool cur_time_t::load_state(void *f)
{
	FILEIO *state_fio = (FILEIO *)f;
	
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	year = state_fio->FgetInt32();
	month = state_fio->FgetInt32();
	day = state_fio->FgetInt32();
	day_of_week = state_fio->FgetInt32();
	hour = state_fio->FgetInt32();
	minute = state_fio->FgetInt32();
	second = state_fio->FgetInt32();
	initialized = state_fio->FgetBool();
	return true;
}

