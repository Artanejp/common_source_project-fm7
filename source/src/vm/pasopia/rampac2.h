/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ ram pac 2 (32kbytes) ]
*/

#ifndef _RAMPAC2_H_
#define _RAMPAC2_H_

#include "../vm.h"
#include "../../emu.h"
#include "pac2dev.h"

class RAMPAC2 : public PAC2DEV
{
private:
	_TCHAR path[_MAX_PATH];
	uint8 ram[32*1024];
	uint32 ptr;
	bool opened, modified;
	
public:
	RAMPAC2(VM* parent_vm, EMU* parent_emu) : PAC2DEV(parent_vm, parent_emu) {}
	~RAMPAC2() {}
	
	// common functions
	void initialize(int id);
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void open_file(_TCHAR* file_path);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

