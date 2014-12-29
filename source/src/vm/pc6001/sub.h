/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : Takeda.Toshiya
	Date   : 2014.05.24-

	[ sub cpu ]
*/

#ifndef _SUB_H_
#define _SUB_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SUB_DATAREC	0
#define SIG_SUB_RXRDY	1

class FILEIO;

class SUB : public DEVICE
{
private:
	DEVICE *d_pio, *d_drec, *d_timer;
	uint8* key_stat;
	int p1_out, p2_in;
	bool drec_in, rxrdy_in;
	bool changed;
	void update_key();
	
	FILEIO* fio;
	bool rec, is_wav;
	int prev_command, baud, index;
	uint8 buffer[0x10000];
	
public:
	SUB(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SUB() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	uint32 intr_ack();
	void event_frame();
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique functions
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void set_context_timer(DEVICE* device)
	{
		d_timer = device;
	}
	bool rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted()
	{
		return rec;
	}
};
#endif
