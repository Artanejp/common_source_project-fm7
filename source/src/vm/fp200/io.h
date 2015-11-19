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
#define SIG_IO_CMT	1


class FILEIO;

class IO : public DEVICE
{
private:
	DEVICE *d_cpu, *d_drec, *d_rtc;
	
	bool mode_basic;
	bool sod;
	
	// display
	uint8 screen[64][160];
	uint8 font[8*256];
	
	struct {
		uint8 ram[0x400];
		int offset, cursor;
	} lcd[2];
	
	int lcd_status, lcd_addr;
	bool lcd_text;
	
	// cmt
	bool cmt_selected;
	uint8 cmt_mode;
	bool cmt_play_ready, cmt_play_signal, cmt_rec_ready;
	FILEIO* cmt_fio;
	bool cmt_rec, cmt_is_wav;
	_TCHAR cmt_rec_file_path[_MAX_PATH];
	int cmt_bufcnt;
	uint8 cmt_buffer[0x10000];
	void cmt_write_buffer(uint8 value, int samples);
	
	// from FP-1100
	uint8 cmt_clock;
	struct {
		bool in_d, in_ck, in_s, in_r;
		bool out_q, out_nq;
		bool tmp_ck;
		void update()
		{
			if(!in_s && in_r) {
				out_q = true;
				out_nq = false;
			} else if(in_s && !in_r) {
				out_q = false;
				out_nq = true;
			} else if(!in_s && !in_r) {
				out_q = out_nq = true; // undetermined
			} else if(!tmp_ck && in_ck) {
				out_q = in_d;
				out_nq = !in_d;
			}
			tmp_ck = in_ck;
		}
	} b16_1, b16_2, g21_1, g21_2;
	struct {
		bool in_d0, in_d1, in_d2, in_d3, in_d4, in_d5, in_d6, in_d7;
		bool in_a, in_b, in_c, in_s;
		bool out_y, out_ny;
		void update()
		{
			if(in_s) {
				out_y = false;
			} else if(!in_a && !in_b && !in_c) {
				out_y = in_d0;
			} else if( in_a && !in_b && !in_c) {
				out_y = in_d1;
			} else if(!in_a &&  in_b && !in_c) {
				out_y = in_d2;
			} else if( in_a &&  in_b && !in_c) {
				out_y = in_d3;
			} else if(!in_a && !in_b &&  in_c) {
				out_y = in_d4;
			} else if( in_a && !in_b &&  in_c) {
				out_y = in_d5;
			} else if(!in_a &&  in_b &&  in_c) {
				out_y = in_d6;
			} else if( in_a &&  in_b &&  in_c) {
				out_y = in_d7;
			}
			out_ny = !out_y;
		}
	} c15;
	struct {
		bool in_a, in_b, in_rc1, in_rc2;
		bool out_qa, out_qb, out_qc;
		bool tmp_a, tmp_b;
		uint8 counter_a, counter_b;
		void update()
		{
			if(in_rc1 && in_rc2) {
				counter_a = counter_b = 0;
			} else {
				if(tmp_a && !in_a) {
					counter_a++;
				}
				if(tmp_b && !in_b) {
					counter_b++;
				}
			}
			tmp_a = in_a;
			tmp_b = in_b;
			out_qa = ((counter_a & 1) != 0);
			out_qb = ((counter_b & 1) != 0);
			out_qc = ((counter_b & 2) != 0);
		}
	} c16;
	struct {
		bool in_ck, in_clr;
		bool out_q5, out_q6;
		bool tmp_ck;
		uint8 counter;
		void update()
		{
			if(in_clr) {
				counter = 0;
			} else if(tmp_ck && !in_ck) {
				counter++;
			}
			tmp_ck = in_ck;
			out_q5 = ((counter & 0x10) != 0);
			out_q6 = ((counter & 0x20) != 0);
		}
	} f21;
	void update_cmt();
	
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
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
	void key_down(int code);
	void key_up();
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool tape_inserted()
	{
		return cmt_rec;
	}
	void draw_screen();
};

#endif
