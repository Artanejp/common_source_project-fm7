/*
	BANDAI RX-78 Emulator 'eRX-78'

	Author : Takeda.Toshiya
	Date   : 2006.08.21 -

	[ cmt ]
*/

#ifndef _CMT_H_
#define _CMT_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_CMT_IN	0

class CMT : public DEVICE
{
private:
	DEVICE* d_drec;
	
	// data recorder
	bool in, out, remote, now_acc;
	int framecnt;
	
public:
	CMT(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~CMT() {}
	
	// common functions
	void initialize();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique function
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
};

#endif
