/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.12.18-

	[ dummy printer ]
*/

#ifndef _PRNFILE_H_
#define _PRNFILE_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class FILEIO;

class PRNFILE : public DEVICE
{
private:
	outputs_t outputs_busy;
	outputs_t outputs_ack;
	
	_TCHAR file_path[_MAX_PATH];
	FILEIO *fio;
	int value, busy_id, ack_id, wait_frames;
	bool strobe, res, busy, ack;
	
	void set_busy(bool value);
	void set_ack(bool value);
	void open_file();
	void close_file();
	
public:
	PRNFILE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_busy);
		initialize_output_signals(&outputs_ack);
	}
	~PRNFILE() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_frame();
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_signal(int ch);
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_busy(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_busy, device, id, mask);
	}
	void set_context_ack(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_ack, device, id, mask);
	}
};

#endif

