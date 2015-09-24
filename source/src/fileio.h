/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ file i/o ]
*/

#ifndef _FILEIO_H_
#define _FILEIO_H_

#ifdef _USE_AGAR
#include <agar/core.h>
#elif defined(_USE_QT)
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#else
#include <windows.h>
#endif
#include <stdio.h>
#include "common.h"

#define FILEIO_READ_BINARY		1
#define FILEIO_WRITE_BINARY		2
#define FILEIO_READ_WRITE_BINARY	3
#define FILEIO_READ_WRITE_NEW_BINARY	4
#define FILEIO_READ_ASCII		5
#define FILEIO_WRITE_ASCII		6
#define FILEIO_READ_WRITE_ASCII		7
#define FILEIO_READ_WRITE_NEW_ASCII	8
#define FILEIO_SEEK_SET			0
#define FILEIO_SEEK_CUR			1
#define FILEIO_SEEK_END			2

class FILEIO
{
private:
	FILE* fp;
	
public:
	FILEIO();
	~FILEIO();

	static bool IsFileExists(const _TCHAR *filename);
	static bool IsFileProtected(const _TCHAR *filename);
	static void RemoveFile(const _TCHAR *filename);

	bool Fopen(const _TCHAR *filename, int mode);
	void Fclose();
	bool IsOpened() { return (fp != NULL); }
	uint32 FileLength();
	
	bool FgetBool();
	void FputBool(bool val);
	uint8 FgetUint8();
	void FputUint8(uint8 val);
	uint16 FgetUint16();
	void FputUint16(uint16 val);
	uint32 FgetUint32();
	void FputUint32(uint32 val);
	uint64 FgetUint64();
	void FputUint64(uint64 val);
	int8 FgetInt8();
	void FputInt8(int8 val);
	int16 FgetInt16();
	void FputInt16(int16 val);
	int32 FgetInt32();
	void FputInt32(int32 val);
	int64 FgetInt64();
	void FputInt64(int64 val);
	float FgetFloat();
	void FputFloat(float val);
	double FgetDouble();
	void FputDouble(double val);
	
	uint16 FgetUint16_LE();
	void FputUint16_LE(uint16 val);
	uint32 FgetUint32_LE();
	void FputUint32_LE(uint32 val);
	uint64 FgetUint64_LE();
	void FputUint64_LE(uint64 val);
	int16 FgetInt16_LE();
	void FputInt16_LE(int16 val);
	int32 FgetInt32_LE();
	void FputInt32_LE(int32 val);
	int64 FgetInt64_LE();
	void FputInt64_LE(int64 val);
	
	uint16 FgetUint16_BE();
	void FputUint16_BE(uint16 val);
	uint32 FgetUint32_BE();
	void FputUint32_BE(uint32 val);
	uint64 FgetUint64_BE();
	void FputUint64_BE(uint64 val);
	int16 FgetInt16_BE();
	void FputInt16_BE(int16 val);
	int32 FgetInt32_BE();
	void FputInt32_BE(int32 val);
	int64 FgetInt64_BE();
	void FputInt64_BE(int64 val);
	
	int Fgetc();
	int Fputc(int c);
	char *Fgets(char *str, int n);
	int Fprintf(const char* format, ...);
	
	uint32 Fread(void* buffer, uint32 size, uint32 count);
	uint32 Fwrite(void* buffer, uint32 size, uint32 count);
	uint32 Fseek(long offset, int origin);
	uint32 Ftell();
};

#endif
