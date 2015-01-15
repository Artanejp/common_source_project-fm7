/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ ram pac 2 (32kbytes) ]
*/

#include "rampac2.h"
#include "../../fileio.h"

static const uint8 header[16] = {
	0xaa, 0x1f, 0x04, 0x00, 0x04, 0x80, 0x00, 0x01, 0x04, 0x04, 0x01, 0x03, 0x08, 0x00, 0x00, 0x00
};

void RAMPAC2::initialize(int id)
{
	opened = modified = false;
	
	// note: rampac2 id must be 1 or 2 !!!
	if(_tcscmp(config.recent_binary_path[id - 1][0], _T("")) != 0) {
		// open last file
		open_file(config.recent_binary_path[id - 1][0]);
	} else {
		// open default rampac2 file
		_TCHAR file_path[_MAX_PATH];
		_stprintf_s(file_path, _MAX_PATH, _T("%sRAMPAC%d.BIN"), emu->application_path(), id);
		open_file(file_path);
	}
}

void RAMPAC2::release()
{
	// save modified data
	if(opened && modified) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(path, FILEIO_WRITE_BINARY)) {
			fio->Fwrite(ram, sizeof(ram), 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void RAMPAC2::reset()
{
	ptr = 0;
}

void RAMPAC2::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0x18:
		ptr = (ptr & 0x7f00) | data;
		break;
	case 0x19:
		ptr = (ptr & 0x00ff) | ((data & 0x7f) << 8);
		break;
	case 0x1a:
		if(ram[ptr & 0x7fff] != data) {
			modified = true;
		}
		ram[ptr & 0x7fff] = data;
		break;
	}
}

uint32 RAMPAC2::read_io8(uint32 addr)
{
	return ram[ptr & 0x7fff];
}

void RAMPAC2::open_file(_TCHAR* file_path)
{
	// save modified data
	release();
	
	// load specified file
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(ram, sizeof(ram), 1);
		fio->Fclose();
	} else {
		// initialize formatted image
		memset(ram, 0, sizeof(ram));
		memcpy(ram, header, sizeof(header));
		memset(ram + 0x20, 0xff, 0x200);
		memset(ram + 0x300, 0xfe, 0x004);
		memset(ram + 0x304, 0xff, 0x0fc);
	}
	delete fio;
	
	_tcscpy_s(path, _MAX_PATH, file_path);
	opened = true;
	modified = false;
}

