/*
	EPSON HC-40 Emulator 'eHC-40'

	Author : Takeda.Toshiya
	Date   : 2008.02.23 -

	[ i/o ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IO_DREC	0
#define SIG_IO_ART	1

class FIFO;

class IO : public DEVICE
{
private:
	DEVICE *d_cpu, *d_mem, *d_tf20, *d_beep, *d_drec;
	uint8* ram;
	
	// registers
	void update_intr();
	uint32 cur_clock;
	uint8 bcr, slbcr;
	uint8 isr, ier;
	uint8 bankr;
	uint8 ioctlr;
	uint32 icrc, icrb;
	bool ear;
	uint8 vadr, yoff;
	
	// sub cpu
	void send_to_7508(uint8 val);
	uint8 rec_from_7508();
	FIFO *cmd_buf, *rsp_buf;
	// rtc
	cur_time_t cur_time;
	int register_id_1sec;
	bool onesec_intr, onesec_intr_enb;
	bool alarm_intr, alarm_intr_enb;
	uint8 alarm[6];
	// keyboard
	void update_key(int code);
	FIFO *key_buf;
	bool kb_intr_enb;
	bool kb_rep_enb, kb_caps;
	uint8 kb_rep_spd1, kb_rep_spd2;
	// art
	FIFO *art_buf;
	uint8 artsr, artdir;
	bool txen, rxen, dsr;
	int register_id_art;
	// beep
	bool beep;
	// reset
	bool res_z80, res_7508;
	
	// externam ram disk
	uint8 ext[0x40000];
	uint32 extar;
	uint8 extcr;
	
	// display
	scrntype pd, pb;
	
public:
	IO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~IO() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void sysreset();
	void write_signal(int id, uint32 data, uint32 mask);
	void event_frame();
	void event_callback(int event_id, int err);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	uint32 intr_ack();
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_mem(DEVICE* device, uint8* ptr)
	{
		d_mem = device;
		ram = ptr;
	}
	void set_context_tf20(DEVICE* device)
	{
		d_tf20 = device;
	}
	void set_context_beep(DEVICE* device)
	{
		d_beep = device;
	}
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void draw_screen();
	void key_down(int code);
	void key_up(int code);
};

#endif
