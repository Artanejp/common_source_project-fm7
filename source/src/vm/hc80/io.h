/*
	EPSON HC-80 Emulator 'eHC-80'

	Author : Takeda.Toshiya
	Date   : 2008.03.14 -

	[ i/o ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IO_RXRDY	0
#define SIG_IO_BARCODE	1
#define SIG_IO_TF20	2

class FIFO;

class IO : public DEVICE
{
private:
	DEVICE *d_cpu, *d_mem, *d_sio, *d_beep, *d_tf20;
	
	// registers
	void update_intr();
	uint32 cur_clock;
	uint8 bcr, slbcr;
	uint8 isr, ier;
	uint8 ioctlr;
	uint32 icrc, icrb;
	bool ear;
	uint8 vadr, yoff;
	
	// 7508
	void send_to_7508(uint8 val);
	uint8 rec_from_7508();
	FIFO *cmd7508_buf, *rsp7508_buf;
	// rtc
	cur_time_t cur_time;
	int register_id;
	bool onesec_intr, onesec_intr_enb;
	bool alarm_intr, alarm_intr_enb;
	uint8 alarm[6];
	// keyboard
	void update_key(int code);
	FIFO *key_buf;
	bool kb_intr_enb;
	bool kb_rep_enb, kb_caps;
	uint8 kb_rep_spd1, kb_rep_spd2;
	// beep
	bool beep;
	// reset
	bool res_z80, res_7508;
	
	// 6303
	void process_6303();
	uint8 get_point(int x, int y);
	void draw_point(int x, int y, uint16 dot);
	void draw_line(int sx, int sy, int ex, int ey, uint16 ope);
	uint8 cmd6303, psr;
	FIFO *cmd6303_buf, *rsp6303_buf;
	uint8 ram[0x10000];
	uint8 basic[0x8000];
	uint8 util[0x8000];
	uint16 cs_addr;		// character screen starting address
	uint16 gs_addr;		// graphics screen starting address
	uint8 lcd_on;		// lcd on/off (0=off, others=on)
	uint8 scr_mode;		// screen mode (0=graph, others=char)
	uint16 scr_ptr;		// screen pointer (8000h-97ffh)
	uint8 num_lines;	// number of lines
	uint8 curs_mode;	// cursor mode
	uint8 curs_x;		// cursor position
	uint8 curs_y;
	uint8 wnd_ptr_x;	// window pointer
	uint8 wnd_ptr_y;
	uint8 flash_block;	// block flashing
	uint8 cs_blocks;	// for character screen
	uint8 cs_block[40][3];
	uint8 gs_blocks;	// for graphics screen
	uint8 gs_block[144][3];
	uint8 font[256*8];
	uint8 udgc[256][255+2];
	uint8 mov[64][80];
	uint8 lcd[SCREEN_HEIGHT][SCREEN_WIDTH];
	scrntype pd, pb;
	int blink;
	// tf20
	FIFO *tf20_buf;
	
	// ramdisk
	int device_type;
	
	// externam ram disk
	uint8 ext[0x40000];
	uint32 extar;
	uint8 extcr;
	
	// intelligent ram disk
	void iramdisk_write_data(uint8 val);
	void iramdisk_write_cmd(uint8 val);
	uint8 iramdisk_read_data();
	uint8 iramdisk_read_stat();
	uint8 iramdisk_sectors[15][64][128];
	uint8 iramdisk_cmd;
	int iramdisk_count,iramdisk_dest;
	uint8 iramdisk_buf[130];
	uint8 *iramdisk_ptr;
	
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
	void set_context_mem(DEVICE* device)
	{
		d_mem = device;
	}
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
	void set_context_beep(DEVICE* device)
	{
		d_beep = device;
	}
	void set_context_tf20(DEVICE* device)
	{
		d_tf20 = device;
	}
	void draw_screen();
	void key_down(int code);
	void key_up(int code);
};

#endif
