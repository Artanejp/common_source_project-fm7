/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.02.28 -

	[ EPSON TF-20 ]
*/

#ifndef _TF20_H_
#define _TF20_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIGNAL_TF20_SIO	0

class DISK;

class TF20 : public DEVICE
{
private:
	DEVICE *d_sio;
	int did_sio;
	
	DISK* disk[MAX_DRIVE];
	uint8 bufr[256], bufs[256];
	int buflen, phase;
	
	bool process_cmd();
	bool disk_protected(int drv);
	uint8* get_sector(int drv, int trk, int sec);
	
public:
	TF20(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~TF20() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unitque function
	void set_context_sio(DEVICE* device, int id)
	{
		d_sio = device;
		did_sio = id;
	}
	void open_disk(int drv, _TCHAR path[], int offset);
	void close_disk(int drv);
	bool disk_inserted(int drv);
};

#endif

