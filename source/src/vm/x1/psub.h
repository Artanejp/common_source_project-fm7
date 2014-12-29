/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2009.03.15-

	[ pseudo sub cpu ]
*/

#ifndef _PSUB_H_
#define _PSUB_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_PSUB_TAPE_REMOTE	0
#define SIG_PSUB_TAPE_END	1

class DATAREC;
class FIFO;

class PSUB : public DEVICE
{
private:
	DEVICE *d_cpu, *d_pio;
	DATAREC *d_drec;
	
	cur_time_t cur_time;
	int time_register_id;
	
	uint8 databuf[32][8], *datap;
	uint8 mode, inbuf, outbuf;
	bool ibf, obf;
	int cmdlen, datalen;
	
	FIFO* key_buf;
	uint8* key_stat;
	int key_prev, key_break;
	bool key_shift, key_ctrl, key_graph;
	bool key_caps_locked, key_kana_locked;
	int key_register_id;
	
	bool play, rec, eot;
	
	bool iei, intr;
	uint32 intr_bit;
	
	void update_intr();
	void process_cmd();
	void set_ibf(bool val);
	void set_obf(bool val);
	uint8 get_key_low();
	uint16 get_key(int code, bool repeat);
	
public:
	PSUB(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~PSUB() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// interrupt common functions
	void set_context_intr(DEVICE* device, uint32 bit)
	{
		d_cpu = device;
		intr_bit = bit;
	}
	void set_intr_iei(bool val);
	uint32 intr_ack();
	void intr_reti();
	
	// unique functions
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_context_drec(DATAREC* device)
	{
		d_drec = device;
	}
	void key_down(int code, bool repeat);
	void key_up(int code);
	void play_tape(bool value);
	void rec_tape(bool value);
	void close_tape();
};

#endif

