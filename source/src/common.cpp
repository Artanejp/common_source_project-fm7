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
#include "agar_logger.h"
#include <string>
#include <algorithm>
#include <cctype>
#elif defined(_WIN32)
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#else
#include <time.h>
#endif

#if defined(MAX_MACRO_NOT_DEFINED)
int max(int a, int b)
{
	if(a > b) {
		return a;
	} else {
		return b;
	}
}

unsigned int max(unsigned int a, unsigned int b)
{
	if(a > b) {
		return a;
	} else {
		return b;
	}
}
#endif

#ifdef MIN_MACRO_NOT_DEFINED
int min(int a, int b)
{
	if(a < b) {
		return a;
	} else {
		return b;
	}
}

unsigned int min(unsigned int a, unsigned int b)
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

errno_t my_strncpy_s(char *strDestination, size_t numberOfElements, const char *strSource, size_t count)
{
	strncpy(strDestination, strSource, count);
	return 0;
}

errno_t my_tcsncpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource, size_t count)
{
	_tcsncpy(strDestination, strSource, count);
	return 0;
}

char *my_strtok_s(char *strToken, const char *strDelimit, char **context)
{
	return strtok(strToken, strDelimit);
}

_TCHAR *my_tcstok_s(_TCHAR *strToken, const char *strDelimit, _TCHAR **context)
{
	return _tcstok(strToken, strDelimit);
}

int my_sprintf_s(char *buffer, size_t sizeOfBuffer, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = vsprintf(buffer, format, ap);
	va_end(ap);
	return result;
}

int my_stprintf_s(_TCHAR *buffer, size_t sizeOfBuffer, const _TCHAR *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = _vstprintf(buffer, format, ap);
	va_end(ap);
	return result;
}

int my_vsprintf_s(char *buffer, size_t numberOfElements, const char *format, va_list argptr)
{
	return vsprintf(buffer, format, argptr);
}

int my_vstprintf_s(_TCHAR *buffer, size_t numberOfElements, const _TCHAR *format, va_list argptr)
{
	return _vstprintf(buffer, format, argptr);
}
#endif

 
#ifndef _WIN32
BOOL MyWritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
{
	BOOL result = FALSE;
	FILEIO* fio_i = new FILEIO();
	if(fio_i->Fopen(lpFileName, FILEIO_READ_ASCII)) {
		char tmp_path[_MAX_PATH];
		my_sprintf_s(tmp_path, _MAX_PATH, "%s.$$$", lpFileName);
		FILEIO* fio_o = new FILEIO();
		if(fio_o->Fopen(tmp_path, FILEIO_WRITE_ASCII)) {
			bool in_section = false;
			char section[1024], line[1024], *equal;
			my_sprintf_s(section, 1024, "[%s]", lpAppName);
			while(fio_i->Fgets(line, 1024) != NULL && strlen(line) > 0) {
				if(line[strlen(line) - 1] == '\n') {
					line[strlen(line) - 1] = '\0';
				}
				if(!result) {
					if(line[0] == '[') {
						if(in_section) {
							fio_o->Fprintf("%s=%s\n", lpKeyName, lpString);
							result = TRUE;
						} else if(strcmp(line, section) == 0) {
							in_section = true;
						}
					} else if(in_section && (equal = strstr(line, "=")) != NULL) {
						*equal = '\0';
						if(strcmp(line, lpKeyName) == 0) {
							fio_o->Fprintf("%s=%s\n", lpKeyName, lpString);
							result = TRUE;
							continue;
						}
						*equal = '=';
					}
				}
				fio_o->Fprintf("%s\n", line);
			}
			if(!result) {
				if(!in_section) {
					fio_o->Fprintf("[%s]\n", lpAppName);
				}
				fio_o->Fprintf("%s=%s\n", lpKeyName, lpString);
				result = TRUE;
			}
			fio_o->Fclose();
		}
		delete fio_o;
		fio_i->Fclose();
		if(result) {
			if(!(FILEIO::RemoveFile(lpFileName) && FILEIO::RenameFile(tmp_path, lpFileName))) {
				result = FALSE;
			}
		}
	} else {
		FILEIO* fio_o = new FILEIO();
		if(fio_o->Fopen(lpFileName, FILEIO_WRITE_ASCII)) {
			fio_o->Fprintf("[%s]\n", lpAppName);
			fio_o->Fprintf("%s=%s\n", lpKeyName, lpString);
			fio_o->Fclose();
		}
		delete fio_o;
	}
	delete fio_i;
	return result;
//	if((lpKeyName == NULL) || (lpAppName == NULL) || (lpFileName == NULL)) {
//		delete path;
//		return FALSE;
//	}
//	if(path->Fopen(lpFileName, FILEIO_WRITE_APPEND_ASCII) != true) {
//		delete path;
//		return FALSE;
//	}
//	
//	if(lpString == NULL) {
//		v = "";
//	} else {
//		v = lpString;
//	}
//	s = lpAppName;
//	s.append(".");
//	s.append(lpKeyName);
//	s.append("=");
//	s.append(v);
//	s.append("\n");
//	path->Fwrite((void *)s.c_str(), s.length(), 1);
//	path->Fclose();
//	delete path;
//	return TRUE;
}

static std::string MyGetPrivateProfileStr(const _TCHAR *lpAppName, const _TCHAR *lpKeyName, _TCHAR *lpFileName)
{
   std::string key;
   char ibuf[4096 + 102];
   int64_t i;
   int l_len;
   int c = '\0';
   std::string::size_type  pos;
   std::string key_str;
   std::string got_str;
   FILEIO *pf = new FILEIO;
   
   key = lpAppName;
   key = key + ".";
   key = key + lpKeyName;
   got_str = "";
   if(pf->Fopen(lpFileName, FILEIO_READ_ASCII) != true) {
	   delete pf;
	   return got_str;
   }
   AGAR_DebugLog(AGAR_LOG_DEBUG, "Try App: %s Key: %s", lpAppName, lpKeyName);
   pf->Fseek(0, FILEIO_SEEK_SET);
   do {
	   key_str = key;
	   ibuf[0] = '\0';
	   i = 0;
	   l_len = 0;
	   while(1) {
		   if(l_len > (4096 + 100)) { // Too long, read dummy.
			   c = (char)pf->Fgetc();
			   if((c != EOF) && (c != '\n') && (c != '\0')) continue;
			   break;
		   }
		   c = (char)pf->Fgetc();
		   if((c == EOF) || (c == '\n') || (c == '\0')) break;
		   ibuf[i] = (char)c;
		   i++;
		   l_len++;
	   }
	   l_len = 0;
	   ibuf[i] = '\0';
	   got_str = ibuf;
	   key_str = key_str + "=";
	   pos = got_str.find(key_str);
	   if(pos != std::string::npos) break;
	   if(c == EOF) return "";
   } while(c != EOF);
   pf->Fclose();
   delete pf;
   
   got_str.erase(0, pos + key_str.length());
   AGAR_DebugLog(AGAR_LOG_DEBUG, "Got: %s Length: %d", got_str.c_str(), got_str.length());
   return got_str;
}


DWORD MyGetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPCTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	if(lpDefault != NULL) {
		my_strcpy_s(lpReturnedString, nSize, lpDefault);
	} else {
		lpReturnedString[0] = '\0';
	}
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(lpFileName, FILEIO_READ_ASCII)) {
		bool in_section = false;
		char section[1024], line[1024], *equal;
		my_sprintf_s(section, 1024, "[%s]", lpAppName);
		while(fio->Fgets(line, 1024) != NULL && strlen(line) > 0) {
			if(line[strlen(line) - 1] == '\n') {
				line[strlen(line) - 1] = '\0';
			}
			if(line[0] == '[') {
				if(in_section) {
					break;
				} else if(strcmp(line, section) == 0) {
					in_section = true;
				}
			} else if(in_section && (equal = strstr(line, "=")) != NULL) {
				*equal = '\0';
				if(strcmp(line, lpKeyName) == 0) {
					my_strcpy_s(lpReturnedString, nSize, equal + 1);
					break;
				}
			}
		}
		fio->Fclose();
	}
	delete fio;
	return strlen(lpReturnedString);
//	std::string sp = MyGetPrivateProfileStr(lpAppName, lpKeyName, lpFileName);
//	if((!sp.empty()) && (nSize > 1)){
//		strncpy(lpReturnedString, sp.c_str(), nSize);
//	} else {
//		strncpy(lpReturnedString, lpDefault, nSize);
//	}
//	return  strlen(lpReturnedString);
}

UINT MyGetPrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName)
{
	// write your compatible function, if possible in standard C/C++ code
	int i;
	char sstr[128];
	char sval[128];
	std::string s;
	memset(sstr, 0x00, sizeof(sstr));
	memset(sval, 0x00, sizeof(sval));
	snprintf(sval, 128, "%d", nDefault); 
	MyGetPrivateProfileString(lpAppName,lpKeyName, sval, sstr, 128, lpFileName);
	s = sstr;
	
	if(s.empty()) {
		i = nDefault;
	} else {
		i = strtol(s.c_str(), NULL, 10);
	}
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "Got Int: %d\n", i);
	return i;
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
struct to_upper {  // Refer from documentation of libstdc++, GCC5.
	char operator() (char c) const { return std::toupper(c); }
};

const _TCHAR *application_path()
{
	static _TCHAR app_path[_MAX_PATH];
	static bool initialized = false;
	
	if(!initialized) {
#ifdef _WIN32
		_TCHAR tmp_path[_MAX_PATH], *ptr = NULL;
		if(GetModuleFileName(NULL, tmp_path, _MAX_PATH) != 0 && GetFullPathName(tmp_path, _MAX_PATH, app_path, &ptr) != 0 && ptr != NULL) {
			*ptr = _T('\0');
		} else {
			my_tcscpy_s(app_path, _MAX_PATH, _T(".\\"));
		}
#else
		// write code for your environment
#endif
		initialized = true;
	}
	return (const _TCHAR *)app_path;
}

const _TCHAR *create_local_path(const _TCHAR* format, ...)
{
	static _TCHAR file_path[_MAX_PATH];
	_TCHAR file_name[_MAX_PATH];
	va_list ap;
	
	va_start(ap, format);
	my_vstprintf_s(file_name, _MAX_PATH, format, ap);
	va_end(ap);
	my_stprintf_s(file_path, _MAX_PATH, _T("%s%s"), application_path(), file_name);
	return (const _TCHAR *)file_path;
}

void create_local_path(_TCHAR *file_path, int length, const _TCHAR* format, ...)
{
	_TCHAR file_name[_MAX_PATH];
	va_list ap;
	
	va_start(ap, format);
	my_vstprintf_s(file_name, _MAX_PATH, format, ap);
	va_end(ap);
	my_stprintf_s(file_path, length, _T("%s%s"), application_path(), file_name);
}

const _TCHAR *create_date_file_path(const _TCHAR *extension)
{
	static _TCHAR file_path[_MAX_PATH];
	cur_time_t cur_time;
	
	get_host_time(&cur_time);
	return create_local_path(_T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.%s"), cur_time.year, cur_time.month, cur_time.day, cur_time.hour, cur_time.minute, cur_time.second, extension);
}

void create_date_file_path(_TCHAR *file_path, int length, const _TCHAR *extension)
{
	my_tcscpy_s(file_path, length, create_date_file_path(extension));
}

bool check_file_extension(const _TCHAR* file_path, const _TCHAR* ext)
{
#if defined(_USE_QT)
	std::string s_fpath = file_path;
	std::string s_ext = ext;
	bool f = false;
	int pos;
	std::transform(s_fpath.begin(), s_fpath.end(), s_fpath.begin(), to_upper());
	std::transform(s_ext.begin(), s_ext.end(), s_ext.begin(), to_upper());
	if(s_fpath.length() < s_ext.length()) return false;
	pos = s_fpath.rfind(s_ext.c_str(), s_fpath.length());
	if((pos != std::string::npos) && (pos >= (s_fpath.length() - s_ext.length()))) return true; 
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
	
	my_tcscpy_s(path, _MAX_PATH, file_path);
#ifdef _WIN32
 	PathRemoveExtension(path);
#else
	_TCHAR *p = _tcsrchr(path, _T('.'));
	if(p != NULL) {
		*p = _T('\0');
	}
#endif
	return path;
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

void get_host_time(cur_time_t* cur_time)
{
#ifdef _WIN32
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);
	cur_time->year = sTime.wYear;
	cur_time->month = sTime.wMonth;
	cur_time->day = sTime.wDay;
	cur_time->day_of_week = sTime.wDayOfWeek;
	cur_time->hour = sTime.wHour;
	cur_time->minute = sTime.wMinute;
	cur_time->second = sTime.wSecond;
#else
	time_t timer = time(NULL);
	struct tm *local = localtime(&timer);
	cur_time->year = local->tm_year + 1900;
	cur_time->month = local->tm_mon + 1;
	cur_time->day = local->tm_mday;
	cur_time->day_of_week = local->tm_wday;
	cur_time->hour = local->tm_hour;
	cur_time->minute = local->tm_min;
	cur_time->second = local->tm_sec;
#endif
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

