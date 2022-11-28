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
#define CMT_BUF_SIZE	0x40000

class BEEP;
class FIFO;
class FILEIO;

class IO : public DEVICE
{
private:
	BEEP* d_beep;
	DEVICE *d_cpu, *d_mem;
	uint8_t* ram;
	
	// registers
	uint8_t rregs[8], wregs[8];
	
	// t6834
	void update_intr();
	void send_to_sub();
	void recv_from_sub();
	void ack_from_sub();
	void process_sub();
	dll_cur_time_t cur_time;
	int register_id_1sec;
	FIFO* cmd_buf;
	FIFO* rsp_buf;
	uint8_t sub_int;
	uint8_t wram[0x800];
	uint8_t alarm[8];
	
	// keyboard
	FIFO* key_buf;
	bool ctrl, shift, kana, graph, brk;
	uint8_t stick, strig, strig1;
	
	// data recorder
	void send_to_cmt();
	void recv_from_cmt();
	FILEIO* cmt_fio;
	bool cmt_play, cmt_rec, cmt_mode;
	_TCHAR rec_file_path[_MAX_PATH];
	int cmt_len, cmt_ptr;
	uint8_t cmt_buf[CMT_BUF_SIZE];
	
	// x-720
	bool vblank;
	uint8_t font_code;
	
	// video
	void draw_font(int x, int y, uint8_t code);
	void draw_udk();
	void draw_line(int sx, int sy, int ex, int ey);
	void draw_circle(int x, int y, int r);
	void line_clear(int y);
	void scroll();
	uint8_t font[256 * 8], udc[256 * 8];
	uint8_t lcd[32][120];
	bool locate_on, cursor_on, udk_on;
	int locate_x, locate_y;
	int cursor_x, cursor_y, cursor_blink;
	int scroll_min, scroll_max;
	
	// beep
	int register_id_beep;
	
public:
	IO(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("I/O Bus"));
	}
	~IO() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_frame();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted()
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
	void set_context_mem(DEVICE* device, uint8_t* ptr)
	{
		d_mem = device;
		ram = ptr;
	}
	void draw_screen();
	void key_down(int code);
	void key_up(int code);
	bool get_caps_locked()
	{
//		return caps;
		return true;
	}
	bool get_kana_locked()
	{
		return kana;
	}
};

#endif
