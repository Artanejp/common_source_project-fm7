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
	
	int lcd_status, lcd_addr;
	bool lcd_text;
	
	// cmt
	bool cmt_selected, cmt_ready;
	uint8 cmt_mode;
	FILEIO* cmt_fio;
	bool cmt_play, cmt_rec;
	_TCHAR cmt_rec_file_path[_MAX_PATH];
	int cmt_bufcnt, cmt_buflen;
	uint8 *cmt_buffer;
	
	bool cmt_read_buffer();
	void cmt_write_buffer(bool value, int count);
	
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
	void write_io8w(uint32 addr, uint32 data, int* wait);
	uint32 read_io8w(uint32 addr, int* wait);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
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
		return cmt_play || cmt_rec;
	}
	void draw_screen();
};

#endif
