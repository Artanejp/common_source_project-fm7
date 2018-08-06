/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2013.01.17-

	[ common ]
*/

#if defined(_USE_QT)
	#include <string.h>
	#include <fcntl.h>
	#if !defined(__WIN32) && !defined(__WIN64)
		#include <unistd.h>
	#else
		#include <io.h>
		#include <direct.h>
	#endif
	#include <sys/types.h>
	#include <sys/stat.h>
	#include "csp_logger.h"
	#include <string>
	#include <algorithm>
	#include <cctype>
	#include <QDir>
	#include <QFileInfo>
#elif defined(_WIN32)
	#include <shlwapi.h>
	#pragma comment(lib, "shlwapi.lib")
#else
	#include <time.h>
#endif
#include <math.h>
#include "common.h"
#include "fileio.h"

#if defined(__MINGW32__) || defined(__MINGW64__)
	extern DWORD GetLongPathName(LPCTSTR lpszShortPath, LPTSTR lpszLongPath, DWORD cchBuffer);
#endif
#if defined(_USE_QT)
	std::string DLL_PREFIX cpp_homedir;
	std::string DLL_PREFIX my_procname;
	std::string DLL_PREFIX sRssDir;
#endif

uint32_t DLL_PREFIX EndianToLittle_DWORD(uint32_t x)
{
#if defined(__LITTLE_ENDIAN__)
	return x;
#else
	uint32_t y;
	y = ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) |
	    ((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24);
	return y;
#endif
}

uint16_t DLL_PREFIX EndianToLittle_WORD(uint16_t x)
{
#if defined(__LITTLE_ENDIAN__)
	return x;
#else
	uint16_t y;
	y = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
	return y;
#endif
}

uint32_t DLL_PREFIX EndianFromLittle_DWORD(uint32_t x)
{
#if defined(__LITTLE_ENDIAN__)
	return x;
#else
	uint32_t y;
	y = ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) |
	    ((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24);
	return y;
#endif
}

uint16_t DLL_PREFIX EndianFromLittle_WORD(uint16_t x)
{
#if defined(__LITTLE_ENDIAN__)
	return x;
#else
	uint16_t y;
	y = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
	return y;
#endif
}


uint32_t DLL_PREFIX EndianToBig_DWORD(uint32_t x)
{
#if defined(__BIG_ENDIAN__)
	return x;
#else
	uint32_t y;
	y = ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) |
	    ((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24);
	return y;
#endif
}

uint16_t DLL_PREFIX EndianToBig_WORD(uint16_t x)
{
#if defined(__BIG_ENDIAN__)
	return x;
#else
	uint16_t y;
	y = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
	return y;
#endif
}

uint32_t DLL_PREFIX EndianFromBig_DWORD(uint32_t x)
{
#if defined(__BIG_ENDIAN__)
	return x;
#else
	uint32_t y;
	y = ((x & 0x000000ff) << 24) | ((x & 0x0000ff00) << 8) |
	    ((x & 0x00ff0000) >> 8)  | ((x & 0xff000000) >> 24);
	return y;
#endif
}

uint16_t DLL_PREFIX EndianFromBig_WORD(uint16_t x)
{
#if defined(__BIG_ENDIAN__)
	return x;
#else
	uint16_t y;
	y = ((x & 0x00ff) << 8) | ((x & 0xff00) >> 8);
	return y;
#endif
}

#ifndef _MSC_VER
int DLL_PREFIX max(int a, int b)
{
	if(a > b) {
		return a;
	} else {
		return b;
	}
}

unsigned DLL_PREFIX int max(unsigned int a, unsigned int b)
{
	if(a > b) {
		return a;
	} else {
		return b;
	}
}

unsigned DLL_PREFIX int max(unsigned int a, int b)
{
	if(b < 0) return a;
	if(a > (unsigned int)b) {
		return a;
	} else {
		return b;
	}
}

unsigned DLL_PREFIX int max(int a, unsigned int b)
{
	if(a < 0) return b;
	if((unsigned int)a > b) {
		return a;
	} else {
		return b;
	}
}

int DLL_PREFIX min(int a, int b)
{
	if(a < b) {
		return a;
	} else {
		return b;
	}
}

int DLL_PREFIX min(unsigned int a, int b)
{
	if(b < 0) return b;
	if(a > INT_MAX) return b;
	
	if((int)a < b) {
		return (int)a;
	} else {
		return b;
	}
}

int DLL_PREFIX min(int a, unsigned int b)
{
	if(a < 0) return a;
	if(b > INT_MAX) return a;
	
	if(a < (int)b) {
		return a;
	} else {
		return (int)b;
	}
}

unsigned int DLL_PREFIX min(unsigned int a, unsigned int b)
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

errno_t DLL_PREFIX my_tcscat_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource)
{
	_tcscat(strDestination, strSource);
	return 0;
}

errno_t DLL_PREFIX my_strcpy_s(char *strDestination, size_t numberOfElements, const char *strSource)
{
	strcpy(strDestination, strSource);
	return 0;
}

errno_t DLL_PREFIX my_tcscpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource)
{
	_tcscpy(strDestination, strSource);
	return 0;
}

errno_t DLL_PREFIX my_strncpy_s(char *strDestination, size_t numberOfElements, const char *strSource, size_t count)
{
	strncpy(strDestination, strSource, count);
	return 0;
}

errno_t DLL_PREFIX my_tcsncpy_s(_TCHAR *strDestination, size_t numberOfElements, const _TCHAR *strSource, size_t count)
{
	_tcsncpy(strDestination, strSource, count);
	return 0;
}

char *DLL_PREFIX my_strtok_s(char *strToken, const char *strDelimit, char **context)
{
	return strtok(strToken, strDelimit);
}

_TCHAR *DLL_PREFIX my_tcstok_s(_TCHAR *strToken, const char *strDelimit, _TCHAR **context)
{
	return _tcstok(strToken, strDelimit);
}

int DLL_PREFIX my_sprintf_s(char *buffer, size_t sizeOfBuffer, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = vsnprintf(buffer, sizeOfBuffer, format, ap);
	va_end(ap);
	return result;
}

int DLL_PREFIX my_swprintf_s(wchar_t *buffer, size_t sizeOfBuffer, const wchar_t *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = vswprintf(buffer, sizeOfBuffer, format, ap);
	va_end(ap);
	return result;
}

int DLL_PREFIX my_stprintf_s(_TCHAR *buffer, size_t sizeOfBuffer, const _TCHAR *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int result = vsnprintf(buffer, sizeOfBuffer, format, ap);
	va_end(ap);
	return result;
}

int DLL_PREFIX my_vsprintf_s(char *buffer, size_t numberOfElements, const char *format, va_list argptr)
{
	return vsnprintf(buffer, numberOfElements * sizeof(char), format, argptr);
}

int DLL_PREFIX my_vstprintf_s(_TCHAR *buffer, size_t numberOfElements, const _TCHAR *format, va_list argptr)
{
	return vsnprintf(buffer, numberOfElements * sizeof(_TCHAR), format, argptr);
}
#endif

//#ifdef USE_FAST_MEMCPY

void DLL_PREFIX *my_memcpy(void *dst, void *src, size_t len)
{
	size_t len1;
	register size_t len2;
	register uint32_t s_align = (uint32_t)(((size_t)src) & 0x1f);
	register uint32_t d_align = (uint32_t)(((size_t)dst) & 0x1f);
	int i;
	
	if(len == 0) return dst;
	if(len < 8) {
		return memcpy(dst, src, len);
	}
	len1 = len;

#if defined(WITHOUT_UNALIGNED_SIMD)
// Using SIMD without un-aligned instructions.
	switch(s_align) {
	case 0: // Align 256
		{
			uint64_t b64[4];
			register uint64_t *s64 = (uint64_t *)src;
			register uint64_t *d64 = (uint64_t *)dst;
			switch(d_align) {
			case 0: // 256 vs 256
				{
					len2 = len1 >> 5;
					while(len2 > 0) {
						for(i = 0; i < 4; i++) b64[i] = s64[i];
						for(i = 0; i < 4; i++) d64[i] = b64[i];
						s64 += 4;
						d64 += 4;
						--len2;
					}
					len1 = len1 & 0x1f;
					if(len1 != 0) return memcpy(d64, s64, len1);
					return dst;
				}
				break;
			case 0x10: // 256 vs 128
				{
					len2 = len1 >> 5;
					while(len2 > 0) {
						for(i = 0; i < 4; i++) b64[i] = s64[i];
						for(i = 0; i < 2; i++) d64[i] = b64[i];
						d64 += 2;
						for(i = 2; i < 4; i++) d64[i - 2] = b64[i];
						d64 += 2;
						s64 += 4;
						--len2;
					}
					len1 = len1 & 0x1f;
					if(len1 != 0) return memcpy(d64, s64, len1);
					return dst;
				}
				break;
			case 0x08:
			case 0x18: // 256 vs 64
				{
					len2 = len1 >> 5;
					while(len2 > 0) {
						for(i = 0; i < 4; ++i) b64[i] = s64[i];
						for(i = 0; i < 4; ++i) {
							*d64 = b64[i];
							++d64;
						}
						s64 += 4;
						--len2;
					}
					len1 = len1 & 0x1f;
					if(len1 != 0) return memcpy(d64, s64, len1);
					return dst;
				}
				break;
			case 0x04:
			case 0x0c: 
			case 0x14:
			case 0x1c: // 256 vs 32
				{
					uint32_t b32[8];
					register uint32_t *s32 = (uint32_t *)src;
					register uint32_t *d32 = (uint32_t *)dst;
					len2 = len1 >> 5;
					while(len2 > 0) {
						for(i = 0; i < 8; ++i) b32[i] = s32[i];
						*d32 = b32[0];
						++d32;
						*d32 = b32[1];
						++d32;
						*d32 = b32[2];
						++d32;
						*d32 = b32[3];
						++d32;
						*d32 = b32[4];
						++d32;
						*d32 = b32[5];
						++d32;
						*d32 = b32[6];
						++d32;
						*d32 = b32[7];
						++d32;
						s32 += 8;
						--len2;
					}
					len1 = len1 & 0x1f;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			default:
				return memcpy(dst, src, len1);
				break;
			}
		}
		break;
	case 0x10: // Src alignn to 16.
		{
			uint32_t b32[4];
			register uint32_t *s32 = (uint32_t *)src;
			register uint32_t *d32 = (uint32_t *)dst;
			switch(d_align) {
			case 0: // 128 vs 256/128
			case 0x10:
				{
					len2 = len1 >> 4;
					while(len2 > 0) {
						for(i = 0; i < 4; i++) b32[i] = s32[i];
						for(i = 0; i < 4; i++) d32[i] = b32[i];
						s32 += 4;
						d32 += 4;
						--len2;
					}
					len1 = len1 & 0x0f;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			case 0x08:
			case 0x18: // 128 vs 64
				{
					len2 = len1 >> 4;
					while(len2 > 0) {
						for(i = 0; i < 4; ++i) b32[i] = s32[i];
						for(i = 0; i < 2; ++i) {
							d32[i] = b32[i];
						}
						d32 += 2;
						for(i = 2; i < 4; ++i) {
							d32[i - 2] = b32[i];
						}
						d32 += 2;
						s32 += 4;
						--len2;
					}
					len1 = len1 & 0x0f;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			case 0x04:
			case 0x0c:
			case 0x14:
			case 0x1c: // 128 vs 32
				{
					len2 = len1 >> 4;
					while(len2 > 0) {
						for(i = 0; i < 4; ++i) b32[i] = s32[i];
						*d32 = b32[0];
						++d32;
						*d32 = b32[1];
						++d32;
						*d32 = b32[2];
						++d32;
						*d32 = b32[3];
						++d32;
						s32 += 4;
						--len2;
					}
					len1 = len1 & 0x0f;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			default:
				return memcpy(dst, src, len1);
				break;
			}
		}
		break;
	case 0x08:
	case 0x18: // Src alignn to 64.
		{
			register uint32_t *s32 = (uint32_t *)src;
			register uint32_t *d32 = (uint32_t *)dst;
			register uint64_t *s64 = (uint64_t *)src;
			register uint64_t *d64 = (uint64_t *)dst;
			switch(d_align) {
			case 0:
			case 0x10: // 64 vs 128
				{
					uint64_t b128[2];
					len2 = len1 >> 4;
					while(len2 > 0) {
						b128[0] = *s64;
						++s64;
						b128[1] = *s64;
						++s64;
						for(i = 0; i < 2; i++) d64[i] = b128[i];
						d64 += 2;
						--len2;
					}
					len1 = len1 & 0x0f;
					if(len1 != 0) return memcpy(d64, s64, len1);
					return dst;
				}
				break;
			case 0x08:
			case 0x18: // 64 vs 64
				{
					len2 = len1 >> 3;
					while(len2 > 0) {
						*d64 = *s64;
						++s64;
						++d64;
						--len2;
					}
					len1 = len1 & 0x07;
					if(len1 != 0) return memcpy(d64, s64, len1);
					return dst;
				}
				break;
			case 0x04:
			case 0x0c: // 64 vs 32
			case 0x14:
			case 0x1c: // 64 vs 32
				{
					uint32_t b32[2];
					len2 = len1 >> 3;
					while(len2 > 0) {
						for(i = 0; i < 2; ++i) b32[i] = s32[i];
						d32[0] = b32[0];
						d32[1] = b32[1];
						s32 += 2;
						d32 += 2;
						--len2;
					}
					len1 = len1 & 0x07;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			default:
				return memcpy(dst, src, len1);
				break;
			}
		}
	case 0x04:
	case 0x0c:
	case 0x14:
	case 0x1c:  // Src align 32
		{
			register uint32_t *s32 = (uint32_t *)src;
			register uint32_t *d32 = (uint32_t *)dst;
			register uint64_t *d64 = (uint64_t *)dst;
			switch(d_align) {
			case 0:
			case 0x10: // 32 vs 128
				{
					uint32_t b128[4];
					len2 = len1 >> 4;
					while(len2 > 0) {
						b128[0] = *s32;
						++s32;
						b128[1] = *s32;
						++s32;
						b128[3] = *s32;
						++s32;
						b128[4] = *s32;
						++s32;
						for(i = 0; i < 4; i++) d32[i] = b128[i];
						d32 += 4;
						--len2;
					}
					len1 = len1 & 0x0f;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			case 0x08:
			case 0x18: // 32 vs 64
				{
					uint32_t b64[2];
					len2 = len1 >> 3;
					while(len2 > 0) {
						b64[0] = *s32;
						++s32;
						b64[1] = *s32;
						++s32;

						for(i = 0; i < 2; i++) d32[i] = b64[i];
						d32 += 2;
						--len2;
					}
					len1 = len1 & 0x07;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			case 0x04:
			case 0x0c: 
			case 0x14:
			case 0x1c: // 32 vs 32
				{
					len2 = len1 >> 2;
					while(len2 > 0) {
						*d32 = *s32;
						++s32;
						++d32;
						--len2;
					}
					len1 = len1 & 0x03;
					if(len1 != 0) return memcpy(d32, s32, len1);
					return dst;
				}
				break;
			default:
				return memcpy(dst, src, len1);
				break;
			}
		}
		break;
	default:
		if(len1 != 0) return memcpy(dst, src, len1);
		break;
	}

#else
// Using SIMD *with* un-aligned instructions.
	register uint32_t *s32 = (uint32_t *)src;
	register uint32_t *d32 = (uint32_t *)dst;
	if(((s_align & 0x07) != 0x0) && ((d_align & 0x07) != 0x0)) { // None align.
		return memcpy(dst, src, len);
	}
	if((s_align == 0x0) || (d_align == 0x0)) { // Align to 256bit
		uint32_t b256[8];
		len2 = len1 >> 5;
		while(len2 > 0) {
			for(i = 0; i < 8; i++) b256[i] = s32[i];
			for(i = 0; i < 8; i++) d32[i] = b256[i];
			s32 += 8;
			d32 += 8;
			--len2;
		}
		len1 = len1 & 0x1f;
		if(len1 != 0) return memcpy(d32, s32, len1);
		return dst;
	}
	if(((s_align & 0x0f) == 0x0) || ((d_align & 0x0f) == 0x0)) { // Align to 128bit
		uint32_t b128[4];
		len2 = len1 >> 4;
		while(len2 > 0) {
			for(i = 0; i < 4; i++) b128[i] = s32[i];
			for(i = 0; i < 4; i++) d32[i] = b128[i];
			s32 += 4;
			d32 += 4;
			--len2;
		}
		len1 = len1 & 0x0f;
		if(len1 != 0) return memcpy(d32, s32, len1);
		return dst;
	}		
	if(((s_align & 0x07) == 0x0) || ((d_align & 0x07) == 0x0)) { // Align to 64bit
		uint32_t b64[2];
		len2 = len1 >> 3;
		while(len2 > 0) {
			for(i = 0; i < 2; i++) b64[i] = s32[i];
			for(i = 0; i < 2; i++) d32[i] = b64[i];
			s32 += 2;
			d32 += 2;
			--len2;
		}
		len1 = len1 & 0x07;
		if(len1 != 0) return memcpy(d32, s32, len1);
		return dst;
	}		
	//if(len1 != 0) return memcpy(dst, src, len1);
#endif
	// Trap
	return dst;
}
//#endif


#ifndef _WIN32
BOOL DLL_PREFIX MyWritePrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpString, LPCTSTR lpFileName)
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
}


DWORD DLL_PREFIX MyGetPrivateProfileString(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize, LPCTSTR lpFileName)
{
	_TCHAR *lpp = (_TCHAR *)lpReturnedString;
	if(lpDefault != NULL) {
		my_strcpy_s(lpp, nSize, lpDefault);
	} else {
		lpp[0] = '\0';
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
					my_strcpy_s(lpp, nSize, equal + 1);
					break;
				}
			}
		}
		fio->Fclose();
	}
	delete fio;
	//csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Try App: %s Key: %s", lpAppName, lpKeyName);
	return strlen(lpp);
}

UINT DLL_PREFIX MyGetPrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName)
{
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
	//csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Got Int: %d\n", i);
	return i;
}
#endif

#if defined(_RGB555)
scrntype_t DLL_PREFIX RGB_COLOR(uint32_t r, uint32_t g, uint32_t b)
{
	scrntype_t rr = ((scrntype_t)r * 0x1f) / 0xff;
	scrntype_t gg = ((scrntype_t)g * 0x1f) / 0xff;
	scrntype_t bb = ((scrntype_t)b * 0x1f) / 0xff;
	return (rr << 10) | (gg << 5) | bb;
}

scrntype_t DLL_PREFIX RGBA_COLOR(uint32_t r, uint32_t g, uint b, uint32_t a)
{
	return RGB_COLOR(r, g, b);
}

uint8_t DLL_PREFIX R_OF_COLOR(scrntype_t c)
{
	c = (c >> 10) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8_t)c;
}

uint8_t DLL_PREFIX G_OF_COLOR(scrntype_t c)
{
	c = (c >>  5) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8_t)c;
}

uint8_t DLL_PREFIX B_OF_COLOR(scrntype_t c)
{
	c = (c >>  0) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8_t)c;
}

uint8_t DLL_PREFIX A_OF_COLOR(scrntype_t c)
{
	return 0;
}
#elif defined(_RGB565)
scrntype_t DLL_PREFIX RGB_COLOR(uint32_t r, uint32_t g, uint32_t b)
{
	scrntype_t rr = ((scrntype_t)r * 0x1f) / 0xff;
	scrntype_t gg = ((scrntype_t)g * 0x3f) / 0xff;
	scrntype_t bb = ((scrntype_t)b * 0x1f) / 0xff;
	return (rr << 11) | (gg << 5) | bb;
}

scrntype_t DLL_PREFIX RGBA_COLOR(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
	return RGB_COLOR(r, g, b);
}

uint8_t DLL_PREFIX R_OF_COLOR(scrntype_t c)
{
	c = (c >> 11) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8_t)c;
}

uint8_t DLL_PREFIX G_OF_COLOR(scrntype_t c)
{
	c = (c >>  5) & 0x3f;
	c = (c * 0xff) / 0x3f;
	return (uint8_t)c;
}

uint8_t DLL_PREFIX B_OF_COLOR(scrntype_t c)
{
	c = (c >>  0) & 0x1f;
	c = (c * 0xff) / 0x1f;
	return (uint8_t)c;
}

uint8_t DLL_PREFIX A_OF_COLOR(scrntype_t c)
{
	return 0;
}
#endif

#ifndef _MSC_VER
struct to_upper {  // Refer from documentation of libstdc++, GCC5.
	char operator() (char c) const { return std::toupper(c); }
};
#endif

#if defined(_USE_QT)
static void _my_mkdir(std::string t_dir)
{
	struct stat st;
//#if !defined(__WIN32) && !defined(__WIN64)
//	if(fstatat(AT_FDCWD, csppath.c_str(), &st, 0) != 0) {
//		mkdirat(AT_FDCWD, t_dir.c_str(), 0700); // Not found
//	}
#if defined(_USE_QT)
	if(stat(t_dir.c_str(), &st) != 0) {
		QDir dir = QDir::current();
		dir.mkdir(QString::fromStdString(t_dir));
		//dir.mkpath(QString::fromUtf8(app_path));
	}
#else
	if(stat(csppath.c_str(), &st) != 0) {
		_mkdir(t_dir.c_str()); // Not found
	}
#endif
}
#endif

const _TCHAR *DLL_PREFIX get_application_path()
{
	static _TCHAR app_path[_MAX_PATH];
	static bool initialized = false;
	
	if(!initialized) {
#if defined(_WIN32) && !defined(_USE_QT)
		_TCHAR tmp_path[_MAX_PATH], *ptr = NULL;
		if(GetModuleFileName(NULL, tmp_path, _MAX_PATH) != 0 && GetFullPathName(tmp_path, _MAX_PATH, app_path, &ptr) != 0 && ptr != NULL) {
			*ptr = _T('\0');
		} else {
			my_tcscpy_s(app_path, _MAX_PATH, _T(".\\"));
		}
#else
#if defined(Q_OS_WIN)
		std::string delim = "\\";
#else
		std::string delim = "/";
#endif
		std::string csppath = cpp_homedir + "CommonSourceCodeProject" + delim ;
		_my_mkdir(csppath);
	   
		std::string cpath = csppath + my_procname + delim;
		_my_mkdir(cpath);
		strncpy(app_path, cpath.c_str(), _MAX_PATH - 1);
#endif
		initialized = true;
	}
	return (const _TCHAR *)app_path;
}

const _TCHAR *DLL_PREFIX create_local_path(const _TCHAR *format, ...)
{
	static _TCHAR file_path[8][_MAX_PATH];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	_TCHAR file_name[_MAX_PATH];
	//printf("%d %d\n", table_index, output_index);
	va_list ap;
	
	va_start(ap, format);
	my_vstprintf_s(file_name, _MAX_PATH, format, ap);
	va_end(ap);
	my_stprintf_s(file_path[output_index], _MAX_PATH, _T("%s%s"), get_application_path(), file_name);
	return (const _TCHAR *)file_path[output_index];
}

void DLL_PREFIX create_local_path(_TCHAR *file_path, int length, const _TCHAR *format, ...)
{
	_TCHAR file_name[_MAX_PATH];
	va_list ap;
	
	va_start(ap, format);
	my_vstprintf_s(file_name, _MAX_PATH, format, ap);
	va_end(ap);
	my_stprintf_s(file_path, length, _T("%s%s"), get_application_path(), file_name);
}

bool DLL_PREFIX is_absolute_path(const _TCHAR *file_path)
{
#ifdef _WIN32
	if(_tcslen(file_path) > 2 && ((file_path[0] >= _T('A') && file_path[0] <= _T('Z')) || (file_path[0] >= _T('a') && file_path[0] <= _T('z'))) && file_path[1] == _T(':')) {
		return true;
	}
#endif
	return (_tcslen(file_path) > 1 && (file_path[0] == _T('/') || file_path[0] == _T('\\')));
}

const _TCHAR *DLL_PREFIX create_absolute_path(const _TCHAR *file_name)
{
	static _TCHAR file_path[8][_MAX_PATH];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	
	if(is_absolute_path(file_name)) {
		my_tcscpy_s(file_path[output_index], _MAX_PATH, file_name);
	} else {
		my_tcscpy_s(file_path[output_index], _MAX_PATH, create_local_path(file_name));
	}
	return (const _TCHAR *)file_path[output_index];
}

void DLL_PREFIX create_absolute_path(_TCHAR *file_path, int length, const _TCHAR *file_name)
{
	my_tcscpy_s(file_path, length, create_absolute_path(file_name));
}

const _TCHAR *DLL_PREFIX create_date_file_path(const _TCHAR *extension)
{
	cur_time_t cur_time;
	
	get_host_time(&cur_time);
	return create_local_path(_T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.%s"), cur_time.year, cur_time.month, cur_time.day, cur_time.hour, cur_time.minute, cur_time.second, extension);
}

void DLL_PREFIX create_date_file_path(_TCHAR *file_path, int length, const _TCHAR *extension)
{
	my_tcscpy_s(file_path, length, create_date_file_path(extension));
}

bool DLL_PREFIX check_file_extension(const _TCHAR *file_path, const _TCHAR *ext)
{
#if defined(_USE_QT)
	std::string s_fpath = file_path;
	std::string s_ext = ext;
	//bool f = false;
	int pos;
	std::transform(s_fpath.begin(), s_fpath.end(), s_fpath.begin(), to_upper());
	std::transform(s_ext.begin(), s_ext.end(), s_ext.begin(), to_upper());
	if(s_fpath.length() < s_ext.length()) return false;
	pos = s_fpath.rfind(s_ext.c_str(), s_fpath.length());
	if((pos != (int)std::string::npos) && (pos >= ((int)s_fpath.length() - (int)s_ext.length()))) return true; 
	return false;
#else
	int nam_len = _tcslen(file_path);
	int ext_len = _tcslen(ext);
	
	return (nam_len >= ext_len && _tcsncicmp(&file_path[nam_len - ext_len], ext, ext_len) == 0);
#endif
}

const _TCHAR *DLL_PREFIX get_file_path_without_extensiton(const _TCHAR *file_path)
{
	static _TCHAR path[8][_MAX_PATH];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	
	my_tcscpy_s(path[output_index], _MAX_PATH, file_path);
#if defined(_WIN32) && defined(_MSC_VER)
	PathRemoveExtension(path[output_index]);
#elif defined(_USE_QT)
	QString delim;
	delim = QString::fromUtf8(".");
	QString tmp_path = QString::fromUtf8(file_path);
	int n = tmp_path.lastIndexOf(delim);
	if(n > 0) {
		tmp_path = tmp_path.left(n);
	}
	//printf("%s\n", tmp_path.toUtf8().constData());
	memset(path[output_index], 0x00, sizeof(_TCHAR) * _MAX_PATH);
	strncpy(path[output_index], tmp_path.toUtf8().constData(), _MAX_PATH - 1);
#else
#endif
	return (const _TCHAR *)path[output_index];
}

void DLL_PREFIX get_long_full_path_name(const _TCHAR* src, _TCHAR* dst, size_t dst_len)
{
#ifdef _WIN32
	_TCHAR tmp[_MAX_PATH];
	if(GetFullPathName(src, _MAX_PATH, tmp, NULL) == 0) {
		my_tcscpy_s(dst, dst_len, src);
	} else if(GetLongPathName(tmp, dst, _MAX_PATH) == 0) {
		my_tcscpy_s(dst, dst_len, tmp);
	}
#elif defined(_USE_QT)
	QString tmp_path = QString::fromUtf8(src);
	QFileInfo info(tmp_path);
	my_tcscpy_s(dst, dst_len, info.absoluteFilePath().toLocal8Bit().constData());
#else
	// write code for your environment
	
#endif
}

const _TCHAR *DLL_PREFIX get_parent_dir(const _TCHAR* file)
{
	static _TCHAR path[8][_MAX_PATH];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	
#ifdef _WIN32
	_TCHAR *ptr;
	GetFullPathName(file, _MAX_PATH, path[output_index], &ptr);
	if(ptr != NULL) {
		*ptr = _T('\0');
	}
#elif defined(_USE_QT)
	QString delim;
#if defined(Q_OS_WIN)
	delim = QString::fromUtf8("\\");
#else
	delim = QString::fromUtf8("/");
#endif
	QString tmp_path = QString::fromUtf8(file);
	int n = tmp_path.lastIndexOf(delim);
	if(n > 0) {
		tmp_path = tmp_path.left(n);
		tmp_path.append(delim);
	}
	//printf("%s\n", tmp_path.toUtf8().constData());
	memset(path[output_index], 0x00, sizeof(_TCHAR) * _MAX_PATH);
	strncpy(path[output_index], tmp_path.toUtf8().constData(), _MAX_PATH - 1);
#else
	// write code for your environment
#endif
	return path[output_index];
}

const wchar_t *DLL_PREFIX char_to_wchar(const char *cs)
{
	// char to wchar_t
	static wchar_t ws[4096];
	
#if defined(_WIN32) || defined(_USE_QT)
	mbstowcs(ws, cs, strlen(cs));
#else
	// write code for your environment
#endif
	return ws;
}

const char *DLL_PREFIX wchar_to_char(const wchar_t *ws)
{
	// wchar_t to char
	static char cs[4096];
	
#ifdef _WIN32
	wcstombs(cs, ws, wcslen(ws));
#elif defined(_USE_QT)
	wcstombs(cs, ws, wcslen(ws));
#else
	// write code for your environment
#endif
	return cs;
}

const _TCHAR *DLL_PREFIX char_to_tchar(const char *cs)
{
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
	// char to wchar_t
	return char_to_wchar(cs);
#else
	// char to char
	return cs;
#endif
}

const char *DLL_PREFIX tchar_to_char(const _TCHAR *ts)
{
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
	// wchar_t to char
	return wchar_to_char(ts);
#else
	// char to char
	return ts;
#endif
}

const _TCHAR *DLL_PREFIX wchar_to_tchar(const wchar_t *ws)
{
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
	// wchar_t to wchar_t
	return ws;
#else
	// wchar_t to char
	return wchar_to_char(ws);
#endif
}

const wchar_t *DLL_PREFIX tchar_to_wchar(const _TCHAR *ts)
{
#if defined(_UNICODE) && defined(SUPPORT_TCHAR_TYPE)
	// wchar_t to wchar_t
	return ts;
#else
	// char to wchar_t
	return char_to_wchar(ts);
#endif
}

const _TCHAR *DLL_PREFIX create_string(const _TCHAR* format, ...)
{
	static _TCHAR buffer[8][1024];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	va_list ap;
	
	va_start(ap, format);
	my_vstprintf_s(buffer[output_index], 1024, format, ap);
	va_end(ap);
	return (const _TCHAR *)buffer[output_index];
}

int32_t DLL_PREFIX muldiv_s32(int32_t nNumber, int32_t nNumerator, int32_t nDenominator)
{
	try {
		int64_t tmp;
		tmp  = (int64_t)nNumber;
		tmp *= (int64_t)nNumerator;
		tmp /= (int64_t)nDenominator;
		return (int32_t)tmp;
	} catch(...) {
		double tmp;
		tmp  = (double)nNumber;
		tmp *= (double)nNumerator;
		tmp /= (double)nDenominator;
		if(tmp < 0) {
			return (int32_t)(tmp - 0.5);
		} else {
			return (int32_t)(tmp + 0.5);
		}
	}
}

uint32_t DLL_PREFIX muldiv_u32(uint32_t nNumber, uint32_t nNumerator, uint32_t nDenominator)
{
	try {
		uint64_t tmp;
		tmp  = (uint64_t)nNumber;
		tmp *= (uint64_t)nNumerator;
		tmp /= (uint64_t)nDenominator;
		return (uint32_t)tmp;
	} catch(...) {
		double tmp;
		tmp  = (double)nNumber;
		tmp *= (double)nNumerator;
		tmp /= (double)nDenominator;
		return (uint32_t)(tmp + 0.5);
	}
}

static bool _crc_initialized = false;
static uint32_t _crc_table[256] = {0};
static void init_crc32_table(void)
{
	for(int i = 0; i < 256; i++) {
		uint32_t c = i;
		for(int j = 0; j < 8; j++) {
			if(c & 1) {
				c = (c >> 1) ^ 0xedb88320;
			} else {
				c >>= 1;
			}
		}
		_crc_table[i] = c;
	}
	_crc_initialized = true;
}

uint32_t DLL_PREFIX get_crc32(uint8_t data[], int size)
{
	const uint32_t *table = (const uint32_t *)_crc_table;
	if(!_crc_initialized) {
		init_crc32_table();
	}
	
	uint32_t c = ~0;
	for(int i = 0; i < size; i++) {
		c = table[(c ^ data[i]) & 0xff] ^ (c >> 8);
	}
	return ~c;
}

uint32_t DLL_PREFIX calc_crc32(uint32_t seed, uint8_t data[], int size)
{
#if 0
	if(!_crc_initialized) {
		init_crc32_table();
	}
	const uint32_t *table = (const uint32_t *)_crc_table;

	uint32_t c = ~seed;
	for(int i = 0; i < size; i++) {
		c = table[(c ^ data[i]) & 0xff] ^ (c >> 8);
	}
	return ~c;
#else
	// Calculate CRC32
	// Refer to : https://qiita.com/mikecat_mixc/items/e5d236e3a3803ef7d3c5
	static const uint32_t CRC_MAGIC_WORD = 0x04C11DB7;
	uint32_t crc = seed;
	uint8_t *ptr = data;
	uint8_t d;
	int bytes = size;
	bool is_overflow;
	for(int i = 0; i < bytes; i++) {
		d = *ptr++;
		for(int bit = 0; bit < 8; bit++) {
			is_overflow = ((crc & 0x1) != 0);
			crc = crc >> 1;
			if((d & 0x01) != 0) crc = crc | 0x80000000;
			if(is_overflow) crc = crc ^ ((uint32_t)~CRC_MAGIC_WORD);
			d >>= 1;
		}
	}
	return crc;
#endif
}

uint16_t DLL_PREFIX jis_to_sjis(uint16_t jis)
{
	pair_t tmp;
	
	tmp.w.l = jis - 0x2121;
	if(tmp.w.l & 0x100) {
		tmp.w.l += 0x9e;
	} else {
		tmp.w.l += 0x40;
	}
	if(tmp.b.l > 0x7f) {
		tmp.w.l += 0x01;
	}
	tmp.b.h = (tmp.b.h >> 1) + 0x81;
	if(tmp.w.l >= 0xa000) {
		tmp.w.l += 0x4000;
	}
	return tmp.w.l;
}

int DLL_PREFIX decibel_to_volume(int decibel)
{
	// +1 equals +0.5dB (same as fmgen)
	return (int)(1024.0 * pow(10.0, decibel / 40.0) + 0.5);
}

int32_t DLL_PREFIX apply_volume(int32_t sample, int volume)
{
//	int64_t output;
	int32_t output;
	if(sample < 0) {
		output = -sample;
		output *= volume;
		output >>= 10;
		output = -output;
	} else {
		output = sample;
		output *= volume;
		output >>= 10;
	}
//	if(output > 2147483647) {
//		return 2147483647;
//	} else if(output < (-2147483647 - 1)) {
//		return (-2147483647 - 1);
//	} else {
//		return (int32_t)output;
//	}
	return output;
}

void DLL_PREFIX get_host_time(cur_time_t* cur_time)
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



void DLL_PREFIX cur_time_t::increment()
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

void DLL_PREFIX cur_time_t::update_year()
{
	// 1970-2069
	if(year < 70) {
		year += 2000;
	} else if(year < 100) {
		year += 1900;
	}
}

void DLL_PREFIX cur_time_t::update_day_of_week()
{
	static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	int y = year - (month < 3);
	day_of_week = (y + y / 4 - y / 100 + y / 400 + t[month - 1] + day) % 7;
}

#define STATE_VERSION	1

#include "./state_data.h"

void DLL_PREFIX cur_time_t::save_state_helper(void *f, uint32_t *sumseed, bool *__stat)
{
	csp_state_data_saver *state_saver = (csp_state_data_saver *)f;
	const _TCHAR *_ns = "cur_time_t::BEGIN";
	const _TCHAR *_ne = "cur_time_t::END";
	
	if(f == NULL) return;
	
	state_saver->save_string_data(_ns, sumseed, strlen(_ns) + 1, __stat);
	state_saver->put_dword(STATE_VERSION, sumseed, __stat);

	state_saver->put_int32(year, sumseed, __stat);
	state_saver->put_int8((int8_t)month, sumseed, __stat);
	state_saver->put_int8((int8_t)day, sumseed, __stat);
	state_saver->put_int8((int8_t)day_of_week, sumseed, __stat);
	state_saver->put_int8((int8_t)hour, sumseed, __stat);
	state_saver->put_int8((int8_t)minute, sumseed, __stat);
	state_saver->put_int16((int16_t)second, sumseed, __stat);
	state_saver->put_bool(initialized, sumseed, __stat);
	
	state_saver->save_string_data(_ne, sumseed, strlen(_ne) + 1, __stat);
}

bool DLL_PREFIX cur_time_t::load_state_helper(void *f, uint32_t *sumseed, bool *__stat)
{
	csp_state_data_saver *state_saver = (csp_state_data_saver *)f;
	const _TCHAR *_ns = "cur_time_t::BEGIN";
	const _TCHAR *_ne = "cur_time_t::END";
	_TCHAR sbuf[128];
	uint32_t tmpvar;

	if(f == NULL) return false;
	memset(sbuf, 0x00, sizeof(sbuf));
	state_saver->load_string_data(sbuf, sumseed, strlen(_ns) + 1, __stat);
	tmpvar = state_saver->get_dword(sumseed, __stat);
	if(strncmp(sbuf, _ns, strlen(_ns) + 1) != 0) {
		if(__stat != NULL) *__stat = false;
		return false;
	}
	if(tmpvar != STATE_VERSION) {
		if(__stat != NULL) *__stat = false;
		return false;
	}
	year =              state_saver->get_int32(sumseed, __stat);
	month =       (int)(state_saver->get_int8(sumseed, __stat));
	day =         (int)(state_saver->get_int8(sumseed, __stat));
	day_of_week = (int)(state_saver->get_int8(sumseed, __stat));
	hour =        (int)(state_saver->get_int8(sumseed, __stat));
	minute =      (int)(state_saver->get_int8(sumseed, __stat));
	second =      (int)(state_saver->get_int16(sumseed, __stat));
	initialized = state_saver->get_bool(sumseed, __stat);
	
	memset(sbuf, 0x00, sizeof(sbuf));
	state_saver->load_string_data(sbuf, sumseed, strlen(_ne) + 1, __stat);
	if(strncmp(_ne, sbuf, strlen(_ne) + 1) != 0) {
		if(__stat != NULL) *__stat = false;
		return false;
	}
	
	if(__stat != NULL) {
		if(*__stat == false) return false;
		//*__stat = true;
	}
	return true;
}

void DLL_PREFIX cur_time_t::save_state(void *f)
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

bool DLL_PREFIX cur_time_t::load_state(void *f)
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

const _TCHAR *DLL_PREFIX get_symbol(symbol_t *first_symbol, uint32_t addr)
{
	static _TCHAR name[8][1024];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	
	if(first_symbol != NULL) {
		for(symbol_t* symbol = first_symbol; symbol; symbol = symbol->next_symbol) {
			if(symbol->addr == addr) {
				my_tcscpy_s(name[output_index], 1024, symbol->name);
				return name[output_index];
			}
		}
	}
	return NULL;
}

const _TCHAR *DLL_PREFIX get_value_or_symbol(symbol_t *first_symbol, const _TCHAR *format, uint32_t addr)
{
	static _TCHAR name[8][1024];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	
	if(first_symbol != NULL) {
		for(symbol_t* symbol = first_symbol; symbol; symbol = symbol->next_symbol) {
			if(symbol->addr == addr) {
				my_tcscpy_s(name[output_index], 1024, symbol->name);
				return name[output_index];
			}
 		}
 	}
	my_stprintf_s(name[output_index], 1024, format, addr);
	return name[output_index];
}

const _TCHAR *DLL_PREFIX get_value_and_symbol(symbol_t *first_symbol, const _TCHAR *format, uint32_t addr)
{
	static _TCHAR name[8][1024];
	static unsigned int table_index = 0;
	unsigned int output_index = (table_index++) & 7;
	
	my_stprintf_s(name[output_index], 1024, format, addr);
	
	if(first_symbol != NULL) {
		for(symbol_t* symbol = first_symbol; symbol; symbol = symbol->next_symbol) {
			if(symbol->addr == addr) {
				_TCHAR temp[1024];
//				my_stprintf_s(temp, 1024, _T(" (%s)"), symbol->name);
				my_stprintf_s(temp, 1024, _T(";%s"), symbol->name);
				my_tcscat_s(name[output_index], 1024, temp);
				return name[output_index];
			}
		}
	}
	return name[output_index];
}

// Use this before writing wav_data.
bool DLL_PREFIX write_dummy_wav_header(void *__fio)
{
	if(__fio == NULL) return false;

	FILEIO *fio = (FILEIO *)__fio;
	uint8_t dummy[sizeof(wav_header_t) + sizeof(wav_chunk_t)];

	if(!fio->IsOpened()) return false;
	
	memset(dummy, 0, sizeof(dummy));
	fio->Fwrite(dummy, sizeof(dummy), 1);
	return true;
}
// Use this after writing wav_data.
bool DLL_PREFIX set_wav_header(wav_header_t *header, wav_chunk_t *first_chunk, uint16_t channels, uint32_t rate,
							   uint16_t bits, size_t file_length)
{
	uint32_t length = (uint32_t) file_length;
	
	if(header == NULL) return false;
	if(first_chunk == NULL) return false;

	pair_t __riff_chunk_size;
	pair_t __fmt_chunk_size;
	pair_t __wav_chunk_size;
	pair16_t __fmt_id;
	pair16_t __channels;
	pair_t __sample_rate;
	pair_t __data_speed;
	pair16_t __block_size;
	pair16_t __sample_bits;

	__riff_chunk_size.d = length - 8;
	__fmt_chunk_size.d = 16;
	__fmt_id.w = 1;
	__channels.w = channels;
	__sample_rate.d = rate;
	__block_size.w = (uint16_t)((channels * bits) / 8);
	__sample_bits.w = bits;
	__data_speed.d = rate * (uint32_t)(__block_size.w);

	memcpy(&(header->riff_chunk.id), "RIFF", 4);
	header->riff_chunk.size = __riff_chunk_size.get_4bytes_le_to();
	
	memcpy(&(header->wave), "WAVE", 4);
	memcpy(&(header->fmt_chunk.id), "fmt ", 4);
	header->fmt_chunk.size = __fmt_chunk_size.get_4bytes_le_to();
	header->format_id = __fmt_id.get_2bytes_le_to();
	header->channels = __channels.get_2bytes_le_to();
	header->sample_rate = __sample_rate.get_4bytes_le_to();
	header->data_speed =  __data_speed.get_4bytes_le_to();
	header->block_size = __block_size.get_2bytes_le_to();
	header->sample_bits = __sample_bits.get_2bytes_le_to();

	memcpy(&(first_chunk->id), "data", 4);
	__wav_chunk_size.d = length - sizeof(wav_header_t) - sizeof(wav_chunk_t);
	first_chunk->size = __wav_chunk_size.get_4bytes_le_to();

	return true;
}
// Note: buffers are allocated by this, You should free() within user class.
bool DLL_PREFIX load_wav_to_stereo(void *__fio, int16_t **left_buf, int16_t **right_buf, uint32_t *rate, int *got_samples)
{

	if(__fio == NULL) return false;
	if(left_buf == NULL) return false;
	if(right_buf == NULL) return false;
	if(rate == NULL) return false;
	if(got_samples == NULL) return false;
	//if((bits != 8) && (bits != 16) && (bits != 32)) return false;

	FILEIO *fio = (FILEIO *)__fio;
	if(!fio->IsOpened()) return false;

	
	int16_t *left_buffer = NULL;
	int16_t *right_buffer = NULL;
	size_t samples = 0;
	uint32_t sample_rate = 0;
	
	wav_header_t header;
	wav_chunk_t  chunk;

	pair16_t __fmt_id;
	pair16_t __sample_bits;
	pair16_t __channels;
	pair_t __sample_rate;
	pair_t __chunk_size;

	fio->Fread(&header, sizeof(header), 1);
	__fmt_id.set_2bytes_le_from(header.format_id);
	__sample_bits.set_2bytes_le_from(header.sample_bits);
	__chunk_size.set_4bytes_le_from(header.fmt_chunk.size);
	__channels.set_2bytes_le_from(header.channels);
	__sample_rate.set_4bytes_le_from(header.sample_rate);

	if((__fmt_id.w == 1) && ((__sample_bits.w == 8) || (__sample_bits.w == 16) || (__sample_bits.w == 32))) {
		fio->Fseek(__chunk_size.d - 16, FILEIO_SEEK_CUR);
		bool is_eof = false;
		while(1) {
			if(fio->Fread(&chunk, sizeof(chunk), 1) != 1) {
				is_eof = true;
				break;
			}
			__chunk_size.set_4bytes_le_from(chunk.size);
			if(strncmp(chunk.id, "data", 4) == 0) {
				break;
			}
			fio->Fseek(__chunk_size.d, FILEIO_SEEK_CUR);
		}
		__chunk_size.set_4bytes_le_from(chunk.size);
		if(is_eof) {
			fio->Fclose();
			delete fio;
			return false;
		}
		
		samples = (size_t)(__chunk_size.d / __channels.w);
		int16_t data_l, data_r;
		union {
			int16_t s16;
			struct {
				uint8_t l, h;
			} b;
		} pair16;
		union {
			int32_t s32;
			struct {
				uint8_t l, h, h2, h3;
			} b;
		} pair32;
		
		if(samples > 0) {
			if(__sample_bits.w == 16) {
				samples /= 2;
			} else if(__sample_bits.w == 32) {
				samples /= 4;
			}
			if(samples == 0) return false;
			sample_rate = __sample_rate.d;

			left_buffer = (int16_t *)malloc(samples * sizeof(int16_t));
			right_buffer = (int16_t *)malloc(samples * sizeof(int16_t));
			if(left_buffer == NULL) {
				if(right_buffer != NULL) free(right_buffer);
				return false;
			}
			if(right_buffer == NULL) {
				if(left_buffer != NULL) free(left_buffer);
				return false;
			}
			switch(__sample_bits.w) {
			case 8:
				if(__channels.sw == 1) {
					for(int i = 0; i < samples; i++) {
						data_l = (int16_t)(fio->FgetUint8());
						data_l = (data_l - 128) * 256;
						left_buffer[i] = data_l;
						right_buffer[i] = data_l;
					}
				} else if(__channels.sw == 2) {
					for(int i = 0; i < samples; i++) {
						data_l = (int16_t)(fio->FgetUint8());
						data_l = (data_l - 128) * 256;
						data_r = (int16_t)(fio->FgetUint8());
						data_r = (data_r - 128) * 256;
						left_buffer[i] = data_l;
						right_buffer[i] = data_r;
					}
				}
				break;
			case 16:
				if(__channels.sw == 1) {
					for(int i = 0; i < samples; i++) {
						pair16.b.l = fio->FgetUint8();
						pair16.b.h = fio->FgetUint8();
						data_l = pair16.s16;
						
						left_buffer[i] = data_l;
						right_buffer[i] = data_l;
					}
				} else if(__channels.sw == 2) {
					for(int i = 0; i < samples; i++) {
						pair16.b.l = fio->FgetUint8();
						pair16.b.h = fio->FgetUint8();
						data_l = pair16.s16;
						
						pair16.b.l = fio->FgetUint8();
						pair16.b.h = fio->FgetUint8();
						data_r = pair16.s16;
						left_buffer[i] = data_l;
						right_buffer[i] = data_r;
					}
				}
				break;
			case 32:
				if(__channels.sw == 1) {
					for(int i = 0; i < samples; i++) {
						pair32.b.l = fio->FgetUint8();
						pair32.b.h = fio->FgetUint8();
						pair32.b.h2 = fio->FgetUint8();
						pair32.b.h3 = fio->FgetUint8();
						data_l = (int16_t)(pair32.s32 / 65536);
						
						left_buffer[i] = data_l;
						right_buffer[i] = data_l;
					}
				} else if(__channels.sw == 2) {
					for(int i = 0; i < samples; i++) {
						pair32.b.l = fio->FgetUint8();
						pair32.b.h = fio->FgetUint8();
						pair32.b.h2 = fio->FgetUint8();
						pair32.b.h3 = fio->FgetUint8();
						data_l = (int16_t)(pair32.s32 / 65536);
						
						pair32.b.l = fio->FgetUint8();
						pair32.b.h = fio->FgetUint8();
						pair32.b.h2 = fio->FgetUint8();
						pair32.b.h3 = fio->FgetUint8();
						data_r = (int16_t)(pair32.s32 / 65536);
						
						left_buffer[i] = data_l;
						right_buffer[i] = data_r;
					}
				}
				break;
			default:
				break;
			}
		}
	} else {
		return false;
	}
	*left_buf = left_buffer;
	*right_buf = right_buffer;
	*rate = sample_rate;
	*got_samples = (int)samples;
	return true;
}

bool DLL_PREFIX load_wav_to_monoral(void *__fio, int16_t **buffer, uint32_t *rate, int *got_samples)
{

	if(__fio == NULL) return false;
	if(buffer == NULL) return false;
	if(rate == NULL) return false;
	if(got_samples == NULL) return false;
	//if((bits != 8) && (bits != 16) && (bits != 32)) return false;

	FILEIO *fio = (FILEIO *)__fio;
	if(!fio->IsOpened()) return false;

	
	int16_t *left_buffer = NULL;
	size_t samples = 0;
	uint32_t sample_rate = 0;
	
	wav_header_t header;
	wav_chunk_t  chunk;

	pair16_t __fmt_id;
	pair16_t __sample_bits;
	pair16_t __channels;
	pair_t __sample_rate;
	pair_t __chunk_size;

	fio->Fread(&header, sizeof(header), 1);
	__fmt_id.set_2bytes_le_from(header.format_id);
	__sample_bits.set_2bytes_le_from(header.sample_bits);
	__chunk_size.set_4bytes_le_from(header.fmt_chunk.size);
	__channels.set_2bytes_le_from(header.channels);
	__sample_rate.set_4bytes_le_from(header.sample_rate);

	if((__fmt_id.w == 1) && ((__sample_bits.w == 8) || (__sample_bits.w == 16) || (__sample_bits.w == 32))) {
		fio->Fseek(__chunk_size.d - 16, FILEIO_SEEK_CUR);
		bool is_eof = false;
		while(1) {
			if(fio->Fread(&chunk, sizeof(chunk), 1) != 1) {
				is_eof = true;
				break;
			}
			__chunk_size.set_4bytes_le_from(chunk.size);
			if(strncmp(chunk.id, "data", 4) == 0) {
				break;
			}
			fio->Fseek(__chunk_size.d, FILEIO_SEEK_CUR);
		}
		__chunk_size.set_4bytes_le_from(chunk.size);
		if(is_eof) {
			fio->Fclose();
			delete fio;
			return false;
		}
		
		samples = (size_t)(__chunk_size.d / __channels.w);
		int16_t data_l, data_r;
		int32_t data32_l, data32_r;
		union {
			int16_t s16;
			struct {
				uint8_t l, h;
			} b;
		} pair16;
		union {
			int32_t s32;
			struct {
				uint8_t l, h, h2, h3;
			} b;
		} pair32;
		
		if(samples > 0) {
			if(__sample_bits.w == 16) {
				samples /= 2;
			} else if(__sample_bits.w == 32) {
				samples /= 4;
			}
			if(samples == 0) return false;
			sample_rate = __sample_rate.d;

			left_buffer = (int16_t *)malloc(samples * sizeof(int16_t));
			if(left_buffer == NULL) {
				return false;
			}
			switch(__sample_bits.w) {
			case 8:
				if(__channels.sw == 1) {
					for(int i = 0; i < samples; i++) {
						data_l = (int16_t)(fio->FgetUint8());
						data_l = (data_l - 128) * 256;
						left_buffer[i] = data_l;
					}
				} else if(__channels.sw == 2) {
					for(int i = 0; i < samples; i++) {
						data_l = (int16_t)(fio->FgetUint8());
						data_l = (data_l - 128) * 256;
						data_r = (int16_t)(fio->FgetUint8());
						data_r = (data_r - 128) * 256;
						left_buffer[i] = (data_l + data_r) / 2;
					}
				}
				break;
			case 16:
				if(__channels.sw == 1) {
					for(int i = 0; i < samples; i++) {
						pair16.b.l = fio->FgetUint8();
						pair16.b.h = fio->FgetUint8();
						data_l = pair16.s16;
						
						left_buffer[i] = data_l;
					}
				} else if(__channels.sw == 2) {
					for(int i = 0; i < samples; i++) {
						pair16.b.l = fio->FgetUint8();
						pair16.b.h = fio->FgetUint8();
						data_l = pair16.s16;
						
						pair16.b.l = fio->FgetUint8();
						pair16.b.h = fio->FgetUint8();
						data_r = pair16.s16;
						left_buffer[i] = (data_l + data_r) / 2;
					}
				}
				break;
			case 32:
				if(__channels.sw == 1) {
					for(int i = 0; i < samples; i++) {
						pair32.b.l = fio->FgetUint8();
						pair32.b.h = fio->FgetUint8();
						pair32.b.h2 = fio->FgetUint8();
						pair32.b.h3 = fio->FgetUint8();
						data_l = (int16_t)(pair32.s32 / 65536);
						
						left_buffer[i] = data_l;
					}
				} else if(__channels.sw == 2) {
					for(int i = 0; i < samples; i++) {
						pair32.b.l = fio->FgetUint8();
						pair32.b.h = fio->FgetUint8();
						pair32.b.h2 = fio->FgetUint8();
						pair32.b.h3 = fio->FgetUint8();
						data32_l = pair32.s32 / 65536;
						
						pair32.b.l = fio->FgetUint8();
						pair32.b.h = fio->FgetUint8();
						pair32.b.h2 = fio->FgetUint8();
						pair32.b.h3 = fio->FgetUint8();
						data32_r = pair32.s32 / 65536;
						
						left_buffer[i] = (int16_t)((data32_l + data32_r) / 2);
					}
				}
				break;
			default:
				break;
			}
		}
	} else {
		return false;
	}
	*buffer = left_buffer;
	*rate = sample_rate;
	*got_samples = (int)samples;
	return true;
}

DLL_PREFIX const _TCHAR *get_lib_common_version()
{
#if defined(__LIBEMU_UTIL_VERSION)
	return (const _TCHAR *)__LIBEMU_UTIL_VERSION;
#else
	return (const _TCHAR *)"\0";
#endif
}
