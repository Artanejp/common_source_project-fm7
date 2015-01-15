/*
	CASIO PV-2000 Emulator 'EmuGaki'

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ cmt ]
*/

#ifndef _CMT_H_
#define _CMT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class FILEIO;

#define BUFFER_SIZE 0x10000

class CMT : public DEVICE
{
private:
	FILEIO* fio;
	int bufcnt;
	uint8 buffer[BUFFER_SIZE];
	bool play, rec;
	uint8 start, bit;
	
public:
	CMT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CMT() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted()
	{
		return (play || rec);
	}
	bool skip() { return (start & 1) && (play || rec); }
};

#endif

