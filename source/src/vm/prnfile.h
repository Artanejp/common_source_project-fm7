/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.12.18-

	[ dummy printer ]
*/

#ifndef _PRNFILE_H_
#define _PRNFILE_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class FILEIO;

class PRNFILE : public DEVICE
{
private:
	_TCHAR file_path[_MAX_PATH];
	FILEIO *fio;
	int value, wait_frames;
	bool strobe;
	
	void open_file();
	void close_file();
	
public:
	PRNFILE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~PRNFILE() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_frame();
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_signal(int ch);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

