/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.02.28 -

	[ EPSON TF-20 (pseudo) ]
*/

#ifndef _PTF20_H_
#define _PTF20_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIGNAL_TF20_SIO	0

class DISK;

class PTF20 : public DEVICE
{
private:
	outputs_t outputs_sio;
	DISK* disk[MAX_DRIVE];
	uint8 bufr[256], bufs[256];
	int buflen, phase;
	
	bool process_cmd();
	bool disk_protected(int drv);
	uint8* get_sector(int drv, int trk, int sec);
	
public:
	PTF20(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs_sio);
	}
	~PTF20() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_signal(int id, uint32 data, uint32 mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_sio(DEVICE* device, int id)
	{
		register_output_signal(&outputs_sio, device, id, 0xff);
	}
	void open_disk(int drv, _TCHAR path[], int bank);
	void close_disk(int drv);
	bool disk_inserted(int drv);
};

#endif

