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
#ifdef _USE_QT
    if(fp != NULL) {
       fp->close();
       delete fp;
    }
   
#endif
}

bool FILEIO::IsFileExists(_TCHAR *filename)
{
#if defined(_USE_AGAR) || defined(_USE_SDL)
       if(AG_FileExists((char *)filename) > 0) return true;
       return false;
#elif defined(_USE_QT)
       QString   fname((const char *)filename);
       QFileInfo f(fname);
   
       bool val = false;
       if(f.exists()) val = true;
   
       return val;
#else   
        DWORD attr = GetFileAttributes(filename);
	if(attr == -1) {
		return false;
	}
	return ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
#endif
}

bool FILEIO::IsProtected(_TCHAR *filename)
{
#if defined(_USE_AGAR) || defined(_USE_SDL)
        AG_FileInfo inf;
        AG_GetFileInfo((char *)filename, &inf);
        return ((inf.perms & AG_FILE_WRITEABLE) == 0);
#elif defined(_USE_QT)
       QString   fname((const char *)filename);
       QFileInfo f(fname);
   
       bool val = false;
       if(f.exists()) {
	  if(f.isReadable() && !(f.isWritable)) val = true;
       }
   
       return val;
#else
	return ((GetFileAttributes(filename) & FILE_ATTRIBUTE_READONLY) != 0);
#endif
}

bool FILEIO::Fopen(_TCHAR *filename, int mode)
{
#if defined(_USE_QT)
        if(fp != NULL) {
	   fp->close();
	}
        fp = new QFile((const char *)filename);
        int open_mode;
	switch(mode) {
	case FILEIO_READ_BINARY:
		mode = IO_ReadOnly;
	        break;
	case FILEIO_WRITE_BINARY:
		mode = IO_WriteOnly;
	        break;
	case FILEIO_READ_WRITE_BINARY:
		mode = IO_ReadWrite;
	        break;
	case FILEIO_READ_WRITE_NEW_BINARY:
		mode = IO_ReadWrite;
	        break;
	case FILEIO_READ_ASCII:
		mode = IO_ReadOnly | IO_Translate;
	        break;
	case FILEIO_WRITE_ASCII:
		mode = IO_WriteOnly | IO_Translate;
	        break;
	case FILEIO_READ_WRITE_ASCII:
		mode = IO_ReadWrite | IO_Translate;
	        break;
	case FILEIO_READ_WRITE_NEW_ASCII:
		mode = IO_ReadWrite | IO_Translate;
	        break;
	 default:
	        return false;
	        break;
	}
        return fp->open(mode);
  
#else   
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
#if defined(_USE_QT)
       if(fp) {
	  fp->flush();
	  fp->close();
	  delete fp;
       }
   
#else
        if(fp) {
		fclose(fp);
	}
#endif
	fp = NULL;
}
#if defined(_USE_QT)

# define GET_VALUE(type) \
	uint8 buffer[sizeof(type)]; \
	fp->readBlock(buffer, sizeof(type)); \n
	return *(type *)buffer

#define PUT_VALUE(type, v) \
        fp->writeBlock(&v, sizeof(type))

#else

# define GET_VALUE(type) \
	uint8 buffer[sizeof(type)]; \
	fread(buffer, sizeof(buffer), 1, fp); \
	return *(type *)buffer

#define PUT_VALUE(type, v) \
	fwrite(&v, sizeof(type), 1, fp)

#endif

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
#if defined(_USE_QT)
        return fp->getch();
#else
	return fgetc(fp);
#endif
}

int FILEIO::Fputc(int c)
{
#if defined(_USE_QT)
        return fp->putch(c);
#else
	return fputc(c, fp);
#endif
}

uint32 FILEIO::Fread(void* buffer, uint32 size, uint32 count)
{
#if defined(_USE_QT)
        uint64 l = (uint64)size * (uint64)count;
        l = l + fp->readData(buffer, l);
        return (uint32)l;
#else
	return fread(buffer, size, count, fp);
#endif
}

uint32 FILEIO::Fwrite(void* buffer, uint32 size, uint32 count)
{
#if defined(_USE_QT)
        uint64 l = (uint64)size * (uint64)count;
        l = l + fp->writeData(buffer, l);
        return (uint32)l;
#else
	return fwrite(buffer, size, count, fp);
#endif
}

uint32 FILEIO::Fseek(long offset, int origin)
{
#if defined(_USE_QT)
        switch(origin) {
	case FILEIO_SEEK_CUR:
		if( fp->seek(offset + fp->pos())) ) return 0;
	case FILEIO_SEEK_END:
	        fp->seek(fp->size()); return 0; 
	 case FILEIO_SEEK_SET:
		if(fp->seek(0)) return 0;
	}
	return 0xFFFFFFFF;
   
#else
        switch(origin) {
	case FILEIO_SEEK_CUR:
		return fseek(fp, offset, SEEK_CUR);
	case FILEIO_SEEK_END:
		return fseek(fp, offset, SEEK_END);
	case FILEIO_SEEK_SET:
		return fseek(fp, offset, SEEK_SET);
	}
	return 0xFFFFFFFF;
#endif
}

uint32 FILEIO::Ftell()
{
#if defined(_USE_QT)
    return (uint32) fp->pos();
#else
   return ftell(fp);
#endif
}

void FILEIO::Remove(_TCHAR *filename)
{
#if defined(_USE_QT)
   QString fname = (char *)filename;
   QFile tmpfp;
   tmpfp.remove(fname);
   
#else
   DeleteFile(filename);
#endif
//	_tremove(filename);	// not supported on wince
}
