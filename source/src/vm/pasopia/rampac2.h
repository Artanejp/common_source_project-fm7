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
	uint8_t ram[32*1024];
	uint32_t ptr;
	bool opened, modified;
	
public:
	RAMPAC2(VM_TEMPLATE* parent_vm, EMU* parent_emu) : PAC2DEV(parent_vm, parent_emu)
	{
		set_device_name(_T("RAM PAC2"));
	}
	~RAMPAC2() {}
	
	// common functions
	void initialize(int id);
	void release();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void open_file(const _TCHAR* file_path);
	bool process_state(FILEIO* state_fio, bool loading);
};

#endif

