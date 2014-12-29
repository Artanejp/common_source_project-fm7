/*
	CASIO FP-200 Emulator 'eFP-200'

	Author : Takeda.Toshiya
	Date   : 2013.03.21-

	[ io ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IO_SOD	0

class FILEIO;

class IO : public DEVICE
{
private:
	DEVICE *d_cpu, *d_rtc;
	
	bool mode_basic;
	bool sod;
	
	// display
	uint8 screen[64][160];
	uint8 font[8*256];
	
	struct {
		struct {
			uint8 data;
			bool is_text;
		} ram[0x400];
		int offset, cursor;
	} lcd[2];
	
//	lcd_t lcd[2];
	int lcd_status, lcd_addr;
	bool lcd_text;
	
	// cmt
	bool cmt_selected;
	uint8 cmt_mode;
	bool cmt_play, cmt_rec;
	int cmt_count, cmt_data;
	FILEIO *cmt_fio;
	
	// keyboard
	uint8* key_stat;
	uint8 key_column;
	void update_sid();
	
public:
	IO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~IO() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	
	// unique function
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
	void key_down(int code);
	void key_up();
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted()
	{
		return (cmt_play || cmt_rec);
	}
	void draw_screen();
};

#endif
