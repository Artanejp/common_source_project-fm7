/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.16 -

	[ cmt ]
*/

#ifndef _CMT_H_
#define _CMT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_CMT_REMOTE	0
#define SIG_CMT_OUT	1

// max 256kbytes
#define BUFFER_SIZE	0x40000

class CMT : public DEVICE
{
private:
	DEVICE* d_sio;
	
	FILEIO* fio;
	bool play, rec, remote;
	_TCHAR rec_file_path[_MAX_PATH];
	int bufcnt;
	uint8_t buffer[BUFFER_SIZE];
	
	void release_tape();
	
public:
	CMT(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("CMT I/F"));
	}
	~CMT() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted()
	{
		return (play || rec);
	}
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
};

#endif

