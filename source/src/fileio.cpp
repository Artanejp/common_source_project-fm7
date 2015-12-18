/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ file i/o ]
*/

#ifdef _WIN32
#include <windows.h>
#endif
#include "fileio.h"
#if !defined(MSC_VER)
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#endif

FILEIO::FILEIO()
{
	fp = NULL;
}

FILEIO::~FILEIO(void)
{
	Fclose();
}

bool FILEIO::IsFileExists(const _TCHAR *file_path)
{
#if defined(_USE_QT) || defined(_USE_SDL)
	FILE *f;
	f = fopen(file_path, "r");
	if(f != NULL)  {
		fclose(f);	   
		return true;
	}
	return false;
#else   
# ifdef _WIN32
	DWORD attr = GetFileAttributes(file_path);
 	if(attr == -1) {
 		return false;
 	}
 	return ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
# else
	return (_taccess(file_path, 0) == 0);
# endif
#endif
}
#if defined(_USE_QT)
# include <sys/types.h>
# include <sys/stat.h>
# if !defined(Q_OS_WIN)
#   include <unistd.h>
# endif
#endif
bool FILEIO::IsFileProtected(const _TCHAR *file_path)
{
#if defined(_USE_QT) || defined(_USE_SDL)
	struct stat st;
	if(stat(file_path, &st) == 0) {
		if((st.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) == 0) return true;
	}
    return false;
#else
# ifdef _WIN32
	return ((GetFileAttributes(file_path) & FILE_ATTRIBUTE_READONLY) != 0);
# else
	return (_taccess(file_path, 2) != 0);
# endif
#endif
}

bool FILEIO::RemoveFile(const _TCHAR *file_path)
{
#if defined(_USE_QT) || defined(_USE_SDL)
	return (remove(file_path) == 0);
#else
# ifdef _WIN32
	return (DeleteFile(file_path) != 0);
# else
	return (_tremove(file_path) == 0);	// not supported on wince ???
# endif
#endif
}

bool FILEIO::RenameFile(const _TCHAR *existing_file_path, const _TCHAR *new_file_path)
{
#if defined(_USE_QT)
	return (rename(existing_file_path, new_file_path) == 0);
	#else
		#ifdef _WIN32
			return (MoveFile(existing_file_path, new_file_path) != 0);
		#else
			return (_trename(existing_file_path, new_file_path) == 0);
		#endif
#endif			
}

bool FILEIO::Fopen(const _TCHAR *file_path, int mode)
{
	Fclose();
	
	switch(mode) {
	case FILEIO_READ_BINARY:
		return ((fp = _tfopen(file_path, _T("rb"))) != NULL);
	case FILEIO_WRITE_BINARY:
		return ((fp = _tfopen(file_path, _T("wb"))) != NULL);
	case FILEIO_READ_WRITE_BINARY:
		return ((fp = _tfopen(file_path, _T("r+b"))) != NULL);
	case FILEIO_READ_WRITE_NEW_BINARY:
		return ((fp = _tfopen(file_path, _T("w+b"))) != NULL);
	case FILEIO_READ_ASCII:
		return ((fp = _tfopen(file_path, _T("r"))) != NULL);
	case FILEIO_WRITE_ASCII:
		return ((fp = _tfopen(file_path, _T("w"))) != NULL);
	case FILEIO_WRITE_APPEND_ASCII:
		return ((fp = _tfopen(file_path, _T("a"))) != NULL);
	case FILEIO_READ_WRITE_ASCII:
		return ((fp = _tfopen(file_path, _T("r+"))) != NULL);
	case FILEIO_READ_WRITE_NEW_ASCII:
		return ((fp = _tfopen(file_path, _T("w+"))) != NULL);
	case FILEIO_READ_WRITE_APPEND_ASCII:
		return ((fp = _tfopen(file_path, _T("a+"))) != NULL);
	}
	return false;
}

void FILEIO::Fclose()
{
	if(fp) {
		fclose(fp);
	}
	fp = NULL;
}

uint32 FILEIO::FileLength()
{
	long pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	fseek(fp, pos, SEEK_SET);
	return (uint32)len;
}

#define GET_VALUE(type) \
	uint8 buffer[sizeof(type)]; \
	fread(buffer, sizeof(buffer), 1, fp); \
	return *(type *)buffer

#define PUT_VALUE(type, v) \
	fwrite(&v, sizeof(type), 1, fp)

bool FILEIO::FgetBool()
{
	GET_VALUE(bool);
}

void FILEIO::FputBool(bool val)
{
	PUT_VALUE(bool, val);
}

uint8 FILEIO::FgetUint8()
{
	GET_VALUE(uint8);
}

void FILEIO::FputUint8(uint8 val)
{
	PUT_VALUE(uint8, val);
}

uint16 FILEIO::FgetUint16()
{
	GET_VALUE(uint16);
}

void FILEIO::FputUint16(uint16 val)
{
	PUT_VALUE(uint16, val);
}

uint32 FILEIO::FgetUint32()
{
	GET_VALUE(uint32);
}

void FILEIO::FputUint32(uint32 val)
{
	PUT_VALUE(uint32, val);
}

uint64 FILEIO::FgetUint64()
{
	GET_VALUE(uint64);
}

void FILEIO::FputUint64(uint64 val)
{
	PUT_VALUE(uint64, val);
}

int8 FILEIO::FgetInt8()
{
	GET_VALUE(int8);
}

void FILEIO::FputInt8(int8 val)
{
	PUT_VALUE(int8, val);
}

int16 FILEIO::FgetInt16()
{
	GET_VALUE(int16);
}

void FILEIO::FputInt16(int16 val)
{
	PUT_VALUE(int16, val);
}

int32 FILEIO::FgetInt32()
{
	GET_VALUE(int32);
}

void FILEIO::FputInt32(int32 val)
{
	PUT_VALUE(int32, val);
}

int64 FILEIO::FgetInt64()
{
	GET_VALUE(int64);
}

void FILEIO::FputInt64(int64 val)
{
	PUT_VALUE(int64, val);
}

float FILEIO::FgetFloat()
{
	GET_VALUE(float);
}

void FILEIO::FputFloat(float val)
{
	PUT_VALUE(float, val);
}

double FILEIO::FgetDouble()
{
	GET_VALUE(double);
}

void FILEIO::FputDouble(double val)
{
	PUT_VALUE(double, val);
}

typedef union {
	struct {
#ifdef __BIG_ENDIAN__
		uint8 h, l;
#else
		uint8 l, h;
#endif
	} b;
	uint16 u16;
	int16 s16;
} pair16;

typedef union {
	struct {
#ifdef __BIG_ENDIAN__
		uint8 h3, h2, h, l;
#else
		uint8 l, h, h2, h3;
#endif
	} b;
	uint32 u32;
	int32 s32;
} pair32;

typedef union {
	struct {
#ifdef __BIG_ENDIAN__
		uint8 h7, h6, h5, h4, h3, h2, h, l;
#else
		uint8 l, h, h2, h3, h4, h5, h6, h7;
#endif
	} b;
	uint64 u64;
	int64 s64;
} pair64;

uint16 FILEIO::FgetUint16_LE()
{
	pair16 tmp;
	tmp.b.l = FgetUint8();
	tmp.b.h = FgetUint8();
	return tmp.u16;
}

void FILEIO::FputUint16_LE(uint16 val)
{
	pair16 tmp;
	tmp.u16 = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
}

uint32 FILEIO::FgetUint32_LE()
{
	pair32 tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	return tmp.u32;
}

void FILEIO::FputUint32_LE(uint32 val)
{
	pair32 tmp;
	tmp.u32 = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
}

uint64 FILEIO::FgetUint64_LE()
{
	pair64 tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h7 = FgetUint8();
	return tmp.u64;
}

void FILEIO::FputUint64_LE(uint64 val)
{
	pair64 tmp;
	tmp.u64 = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h7);
}

int16 FILEIO::FgetInt16_LE()
{
	pair16 tmp;
	tmp.b.l = FgetUint8();
	tmp.b.h = FgetUint8();
	return tmp.s16;
}

void FILEIO::FputInt16_LE(int16 val)
{
	pair16 tmp;
	tmp.s16 = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
}

int32 FILEIO::FgetInt32_LE()
{
	pair32 tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	return tmp.s32;
}

void FILEIO::FputInt32_LE(int32 val)
{
	pair32 tmp;
	tmp.s32 = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
}

int64 FILEIO::FgetInt64_LE()
{
	pair64 tmp;
	tmp.b.l  = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h7 = FgetUint8();
	return tmp.s64;
}

void FILEIO::FputInt64_LE(int64 val)
{
	pair64 tmp;
	tmp.s64 = val;
	FputUint8(tmp.b.l);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h7);
}

uint16 FILEIO::FgetUint16_BE()
{
	pair16 tmp;
	tmp.b.h = FgetUint8();
	tmp.b.l = FgetUint8();
	return tmp.u16;
}

void FILEIO::FputUint16_BE(uint16 val)
{
	pair16 tmp;
	tmp.u16 = val;
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

uint32 FILEIO::FgetUint32_BE()
{
	pair32 tmp;
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.u32;
}

void FILEIO::FputUint32_BE(uint32 val)
{
	pair32 tmp;
	tmp.u32 = val;
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

uint64 FILEIO::FgetUint64_BE()
{
	pair64 tmp;
	tmp.b.h7 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.u64;
}

void FILEIO::FputUint64_BE(uint64 val)
{
	pair64 tmp;
	tmp.u64 = val;
	FputUint8(tmp.b.h7);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

int16 FILEIO::FgetInt16_BE()
{
	pair16 tmp;
	tmp.b.h = FgetUint8();
	tmp.b.l = FgetUint8();
	return tmp.s16;
}

void FILEIO::FputInt16_BE(int16 val)
{
	pair16 tmp;
	tmp.s16 = val;
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

int32 FILEIO::FgetInt32_BE()
{
	pair32 tmp;
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.s32;
}

void FILEIO::FputInt32_BE(int32 val)
{
	pair32 tmp;
	tmp.s32 = val;
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

int64 FILEIO::FgetInt64_BE()
{
	pair64 tmp;
	tmp.b.h7 = FgetUint8();
	tmp.b.h6 = FgetUint8();
	tmp.b.h5 = FgetUint8();
	tmp.b.h4 = FgetUint8();
	tmp.b.h3 = FgetUint8();
	tmp.b.h2 = FgetUint8();
	tmp.b.h  = FgetUint8();
	tmp.b.l  = FgetUint8();
	return tmp.s64;
}

void FILEIO::FputInt64_BE(int64 val)
{
	pair64 tmp;
	tmp.s64 = val;
	FputUint8(tmp.b.h7);
	FputUint8(tmp.b.h6);
	FputUint8(tmp.b.h5);
	FputUint8(tmp.b.h4);
	FputUint8(tmp.b.h3);
	FputUint8(tmp.b.h2);
	FputUint8(tmp.b.h);
	FputUint8(tmp.b.l);
}

int FILEIO::Fgetc()
{
	return fgetc(fp);
}

int FILEIO::Fputc(int c)
{
	return fputc(c, fp);
}

char *FILEIO::Fgets(char *str, int n)
{
	return fgets(str, n, fp);
}

int FILEIO::Fprintf(const char* format, ...)
{
	va_list ap;
	char buffer[1024];
	
	va_start(ap, format);
	my_vsprintf_s(buffer, 1024, format, ap);
	va_end(ap);
	
	return my_fprintf_s(fp, "%s", buffer);
}

uint32 FILEIO::Fread(void* buffer, uint32 size, uint32 count)
{
	return fread(buffer, size, count, fp);
}

uint32 FILEIO::Fwrite(void* buffer, uint32 size, uint32 count)
{
	return fwrite(buffer, size, count, fp);
}

uint32 FILEIO::Fseek(long offset, int origin)
{
	switch(origin) {
	case FILEIO_SEEK_CUR:
		return fseek(fp, offset, SEEK_CUR);
	case FILEIO_SEEK_END:
		return fseek(fp, offset, SEEK_END);
	case FILEIO_SEEK_SET:
		return fseek(fp, offset, SEEK_SET);
	}
	return 0xFFFFFFFF;
}

uint32 FILEIO::Ftell()
{
	return ftell(fp);
}

