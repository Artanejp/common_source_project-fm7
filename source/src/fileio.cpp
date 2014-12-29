/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ file i/o ]
*/

#include "fileio.h"

FILEIO::FILEIO()
{
	fp = NULL;
}

FILEIO::~FILEIO(void)
{
	Fclose();
}

bool FILEIO::IsFileExists(_TCHAR *filename)
{
	DWORD attr = GetFileAttributes(filename);
	if(attr == -1) {
		return false;
	}
	return ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

bool FILEIO::IsProtected(_TCHAR *filename)
{
	return ((GetFileAttributes(filename) & FILE_ATTRIBUTE_READONLY) != 0);
}

bool FILEIO::Fopen(_TCHAR *filename, int mode)
{
	Fclose();
	
	switch(mode) {
	case FILEIO_READ_BINARY:
		return ((fp = _tfopen(filename, _T("rb"))) != NULL);
	case FILEIO_WRITE_BINARY:
		return ((fp = _tfopen(filename, _T("wb"))) != NULL);
	case FILEIO_READ_WRITE_BINARY:
		return ((fp = _tfopen(filename, _T("r+b"))) != NULL);
	case FILEIO_READ_WRITE_NEW_BINARY:
		return ((fp = _tfopen(filename, _T("w+b"))) != NULL);
	case FILEIO_READ_ASCII:
		return ((fp = _tfopen(filename, _T("r"))) != NULL);
	case FILEIO_WRITE_ASCII:
		return ((fp = _tfopen(filename, _T("w"))) != NULL);
	case FILEIO_READ_WRITE_ASCII:
		return ((fp = _tfopen(filename, _T("r+"))) != NULL);
	case FILEIO_READ_WRITE_NEW_ASCII:
		return ((fp = _tfopen(filename, _T("w+"))) != NULL);
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

int FILEIO::Fgetc()
{
	return fgetc(fp);
}

int FILEIO::Fputc(int c)
{
	return fputc(c, fp);
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

void FILEIO::Remove(_TCHAR *filename)
{
	DeleteFile(filename);
//	_tremove(filename);	// not supported on wince
}
