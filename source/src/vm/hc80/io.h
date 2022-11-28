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
	uint32_t cur_clock;
	uint8_t bcr, slbcr;
	uint8_t isr, ier;
	uint8_t ioctlr;
	uint32_t icrc, icrb;
	bool ear;
	uint8_t vadr, yoff;
	
	// 7508
	void send_to_7508(uint8_t val);
	uint8_t rec_from_7508();
	FIFO *cmd7508_buf, *rsp7508_buf;
	// rtc
	dll_cur_time_t cur_time;
	int register_id;
	bool onesec_intr, onesec_intr_enb;
	bool alarm_intr, alarm_intr_enb;
	uint8_t alarm[6];
	// keyboard
	void update_key(int code);
	FIFO *key_buf;
	bool kb_intr_enb;
	bool kb_rep_enb, kb_caps;
	uint8_t kb_rep_spd1, kb_rep_spd2;
	// beep
	bool beep;
	// reset
	bool res_z80, res_7508;
	
	// 6303
	void process_6303();
	uint8_t get_point(int x, int y);
	void draw_point(int x, int y, uint16_t dot);
	void draw_line(int sx, int sy, int ex, int ey, uint16_t ope, uint8_t mode);
	uint8_t cmd6303, psr;
	FIFO *cmd6303_buf, *rsp6303_buf;
	uint8_t ram[0x10000];
	uint8_t basic[0x8000];
	uint8_t util[0x8000];
	uint16_t cs_addr;		// character screen starting address
	uint16_t gs_addr;		// graphics screen starting address
	uint8_t lcd_on;		// lcd on/off (0=off, others=on)
	uint8_t scr_mode;		// screen mode (0=graph, others=char)
	uint16_t scr_ptr;		// screen pointer (8000h-97ffh)
	uint8_t num_lines;	// number of lines
	uint8_t curs_mode;	// cursor mode
	uint8_t curs_x;		// cursor position
	uint8_t curs_y;
	uint8_t wnd_ptr_x;	// window pointer
	uint8_t wnd_ptr_y;
	uint8_t flash_block;	// block flashing
	uint8_t cs_blocks;	// for character screen
	uint8_t cs_block[40][3];
	uint8_t gs_blocks;	// for graphics screen
	uint8_t gs_block[144][3];
	uint8_t font[256*8];
	uint8_t udgc[256][255+2];
	uint8_t mov[64][80];
	uint8_t lcd[SCREEN_HEIGHT][SCREEN_WIDTH];
	scrntype_t pd, pb;
	int blink;
	// tf20
	FIFO *tf20_buf;
	
	// ramdisk
	int device_type;
	
	// externam ram disk
	uint8_t ext[0x40000];
	uint32_t extar;
	uint8_t extcr;
	
	// intelligent ram disk
	void iramdisk_write_data(uint8_t val);
	void iramdisk_write_cmd(uint8_t val);
	uint8_t iramdisk_read_data();
	uint8_t iramdisk_read_stat();
	uint8_t iramdisk_sectors[15][64][128];
	uint8_t iramdisk_cmd;
	int iramdisk_count,iramdisk_dest;
	uint8_t iramdisk_buf[130];
	uint8_t *iramdisk_ptr;
	
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
	void sysreset();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_frame();
	void event_callback(int event_id, int err);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	uint32_t get_intr_ack();
	bool process_state(FILEIO* state_fio, bool loading);
	
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
