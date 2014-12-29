/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2013.05.01-

	[ sub cpu ]
*/

#ifndef _SUB_H_
#define _SUB_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_SUB_PIO_PORT_C	0
#define SIG_SUB_TAPE_END	1
#define SIG_SUB_TAPE_APSS	2

class DATAREC;

class SUB : public DEVICE
{
private:
	DEVICE *d_pio, *d_rtc;
	DATAREC *d_drec;
	
	uint8 p1_out, p1_in, p2_out, p2_in;
	uint8 portc;
	bool tape_play, tape_rec, tape_eot, tape_apss;
	void update_tape();
	
	// interrupt
	bool intr, obf;
	
	// z80 daisy chain
	DEVICE *d_cpu;
	bool iei;
	uint32 intr_bit;
	void update_intr();
	
public:
	SUB(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~SUB() {}
	
	// common functions
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
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
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
	void set_context_drec(DATAREC* device)
	{
		d_drec = device;
	}
	void play_tape(bool value);
	void rec_tape(bool value);
	void close_tape();
	uint32 rom_crc32;
};

#endif

