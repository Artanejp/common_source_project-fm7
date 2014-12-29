/*
	CANON X-07 Emulator 'eX-07'

	Origin : J.Brigaud
	Author : Takeda.Toshiya
	Date   : 2007.12.26 -

	[ i/o ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define WRAM_OFS_UDC0	0x200
#define WRAM_OFS_UDC1	0x300
#define WRAM_OFS_KBUF	0x400
#define WRAM_OFS_SPGM	0x500
#define WRAM_OFS_RSVD	0x700
#define FONT_OFS_UDC0	0x400
#define FONT_OFS_UDC1	0x700
#define CMT_BUF_SIZE	0x8000

class BEEO;
class FIFO;
class FILEIO;

class IO : public DEVICE
{
private:
	BEEP* d_beep;
	DEVICE *d_cpu, *d_mem;
	uint8* ram;
	
	// registers
	uint8 rregs[8], wregs[8];
	
	// t6834
	void update_intr();
	void send_to_sub();
	void recv_from_sub();
	void ack_from_sub();
	void process_sub();
	cur_time_t cur_time;
	int register_id_1sec;
	FIFO* cmd_buf;
	FIFO* rsp_buf;
	uint8 sub_int;
	uint8 wram[0x800];
	uint8 alarm[8];
	
	// keyboard
	FIFO* key_buf;
	bool ctrl, shift, kana, graph, brk;
	uint8 stick, strig, strig1;
	
	// data recorder
	void send_to_cmt();
	void recv_from_cmt();
	FILEIO* cmt_fio;
	uint8 cmt_buf[CMT_BUF_SIZE];
	int cmt_ptr;
	bool cmt_play, cmt_rec, cmt_mode;
	
	// x-720
	bool vblank;
	uint8 font_code;
	
	// video
	void draw_font(int x, int y, uint8 code);
	void draw_udk();
	void draw_line(int sx, int sy, int ex, int ey);
	void draw_circle(int x, int y, int r);
	void line_clear(int y);
	void scroll();
	uint8 font[256 * 8], udc[256 * 8];
	uint8 lcd[32][120];
	bool locate_on, cursor_on, udk_on;
	int locate_x, locate_y;
	int cursor_x, cursor_y, cursor_blink;
	int scroll_min, scroll_max;
	
	// beep
	int register_id_beep;
	
public:
	IO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~IO() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_frame();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	
	// unique functions
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted()
	{
		return (cmt_play || cmt_rec);
	}
	void set_context_beep(BEEP* device)
	{
		d_beep = device;
	}
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_mem(DEVICE* device, uint8* ptr)
	{
		d_mem = device;
		ram = ptr;
	}
	void draw_screen();
	void key_down(int code);
	void key_up(int code);
};

#endif
