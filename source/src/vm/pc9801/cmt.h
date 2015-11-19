/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'

	Author : Takeda.Toshiya
	Date   : 2011.04.08-

	[ cmt ]
*/

#ifndef _CMT_H_
#define _CMT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_CMT_OUT	0
#define SIG_CMT_TXRDY	1
#define SIG_CMT_RXRDY	2
#define SIG_CMT_TXEMP	3
#define SIG_CMT_MIX     4
#define SIG_CMT_VOLUME  5

// max 256kbytes
#define BUFFER_SIZE	0x40000

class CMT : public DEVICE
{
private:
	DEVICE* d_sio;
	
	FILEIO* fio;
	int bufcnt;
	int buffer_size;
	uint8 buffer[BUFFER_SIZE];
   
	bool play, rec, remote;
#ifdef DATAREC_SOUND
	bool cmt_mix;
	int  cmt_volume;
#endif
	void release_tape();
	
public:
	CMT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {
#ifdef DATAREC_SOUND
		cmt_mix = false;
		cmt_volume = 0x1800;
#endif
	}
	~CMT() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	void write_signal(int id, uint32 data, uint32 mask);
#ifdef DATAREC_SOUND
	void mix(int32 *buffer, int cnt);
#endif
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool tape_inserted()
	{
		return (play || rec);
	}
};

#endif

