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
	bool is_wav, rec, remote;
	_TCHAR rec_file_path[_MAX_PATH];
	int bufcnt;
	uint8_t buffer[BUFFER_SIZE];
	int prev_signal;
	uint32_t prev_clock;
	
	void write_buffer(uint8_t value, int samples);
	void put_signal();
	
public:
	CMT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CMT() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted()
	{
		return rec;
	}
};

#endif

