/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.09.16-

	[ d88 image handler ]
*/

#ifndef _HARDDISK_H_
#define _HARDDISK_H_

#include "../common.h"
//#include "vm.h"
//#include "../emu.h"

class FILEIO;
class EMU;
class HARDDISK
{
protected:
	EMU* emu;
private:
	_TCHAR image_path[_MAX_PATH];
	
	int buffer_size;
	uint8_t *buffer;
	
	int header_size;
	
public:
	HARDDISK(EMU* parent_emu)
	{
		emu = parent_emu;
		buffer = NULL;
		bytes_per_sec = 5 * 1024 * 1024;
		static int num = 0;
		drive_num = num++;
		memset(image_path, 0x00, sizeof(image_path));
		set_device_name(_T("Hard Disk Drive #%d"), drive_num + 1);
	}
	~HARDDISK()
	{
		close();
	}
	
	void open(const _TCHAR* file_path);
	void close();
	
	int cylinders;
	int surfaces;
	int sectors;
	int sector_size;
	int bytes_per_sec;
	int drive_num;
	
	bool mounted()
	{
		return (buffer != NULL);
	}
	bool read_buffer(int position, int length, uint8_t *buf);
	bool write_buffer(int position, int length, uint8_t *buf);
	
	// state
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// device name
	void set_device_name(const _TCHAR* format, ...)
	{
		if(format != NULL) {
			va_list ap;
			_TCHAR buffer[1024];
			
			va_start(ap, format);
			my_vstprintf_s(buffer, 1024, format, ap);
			va_end(ap);
			
			my_tcscpy_s(this_device_name, 128, buffer);
		}
	}
	const _TCHAR *get_device_name()
	{
		return (const _TCHAR *)this_device_name;
	}
	_TCHAR this_device_name[128];
};

#endif

