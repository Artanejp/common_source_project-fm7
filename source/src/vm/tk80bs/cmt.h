/*
	NEC TK-80BS (COMPO BS/80) Emulator 'eTK-80BS'
	NEC TK-80 Emulator 'eTK-80'

	Author : Takeda.Toshiya
	Date   : 2008.08.26 -

	[ cmt ]
*/

#ifndef _CMT_H_
#define _CMT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_CMT_MIC	0
#define SIG_CMT_EAR	1
#if defined(_TK80BS)
#define SIG_CMT_OUT	2
#endif

// max 256kbytes
#define BUFFER_SIZE	0x40000

class CMT : public DEVICE
{
private:
	DEVICE *d_drec, *d_pio;
#if defined(_TK80BS)
	DEVICE *d_sio;
#endif
	
	bool mic, ear;
	bool pulse;
	int pulse_count;
	
#if defined(_TK80BS)
	FILEIO* fio;
	bool play, rec;
	_TCHAR rec_file_path[_MAX_PATH];
	int bufcnt;
	uint8_t buffer[BUFFER_SIZE];
	
	void release_tape();
#endif
	
public:
	CMT(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("CMT I/F"));
	}
	~CMT() {}
	
	// common functions
	void initialize();
#if defined(_TK80BS)
	void release();
	void reset();
#endif
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
#if defined(_TK80BS)
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
	bool is_tape_inserted()
	{
		return (play || rec);
	}
	bool is_tape_playing()
	{
		return play;
	}
	bool is_tape_recording()
	{
		return rec;
	}
	int get_tape_position()
	{
		return 0;
	}
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
#endif
};

#endif

