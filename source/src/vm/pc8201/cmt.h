/*
	NEC PC-8201 Emulator 'ePC-8201'

	Author : Takeda.Toshiya
	Date   : 2013.04.22-

	[ cmt (record only) ]
*/

#ifndef _CMT_H_
#define _CMT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_CMT_REMOTE	0
#define SIG_CMT_SOD	1

// max 256kbytes
#define BUFFER_SIZE	0x40000

class CMT : public DEVICE
{
private:
	FILEIO* fio;
	int bufcnt;
	uint8 buffer[BUFFER_SIZE];
	int prev_signal;
	uint32 prev_clock;
	bool is_wav, rec, remote;
	
	void write_buffer(uint8 value, int samples);
	void put_signal();
	
public:
	CMT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CMT() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted()
	{
		return rec;
	}
};

#endif

