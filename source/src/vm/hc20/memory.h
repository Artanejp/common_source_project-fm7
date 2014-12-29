/*
	EPSON HC-20 Emulator 'eHC-20'

	Author : Takeda.Toshiya
	Date   : 2011.05.23-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_PORT_2	0
#define SIG_MEMORY_PORT_3	1
#define SIG_MEMORY_PORT_4	2
#define SIG_MEMORY_SIO_MAIN	3
#define SIG_MEMORY_SIO_TF20	4
#define SIG_MEMORY_RTC_IRQ	5

#define CMT_BUFFER_SIZE		0x10000

class BEEP;
class FIFO;

class MEMORY : public DEVICE
{
private:
	BEEP *d_beep;
	DEVICE *d_cpu, *d_rtc, *d_tf20;
	
	uint8 wdmy[0x2000];
	uint8 rdmy[0x2000];
	uint8* wbank[8];
	uint8* rbank[8];
	
	// memory with expansion unit
	uint8 ram[0x8000];	// 0000h-7fffh
	uint8 rom[0x8000];	// 8000h-ffffh (internal)
	uint8 ext[0x4000];	// 8000h-bfffh
	
	FIFO *cmd_buf;
	bool sio_select;
	bool special_cmd_masked;
	uint8 slave_mem[0x10000];
	
	typedef struct {
		double freq;
		int period;
		int remain;
	} sound_t;
	sound_t sound[256];
	int sound_ptr;
	int sound_count;
	uint8 sound_reply;
	double sound_freq;
	double tone_table[57];
	
	uint8 key_stat[256], key_flag[256];
	int key_data, key_strobe, key_intmask;
	
	uint8 cmt_buffer[CMT_BUFFER_SIZE];
	int cmt_count;
	bool cmt_play, cmt_rec;
	FILEIO* cmt_fio;
	
	typedef struct {
		uint8 buffer[80];
		int bank;
		int addr;
	} lcd_t;
	lcd_t lcd[6];
	
	scrntype lcd_render[32][120];
	scrntype pd, pb;
	uint8 lcd_select, lcd_data;
	int lcd_clock;
	
	int int_status;
	int int_mask;
	
	void update_sound();
	void update_keyboard();
	void update_intr();
	void send_to_slave(uint8 val);
	void send_to_main(uint8 val);
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	
	// unitque function
	void set_context_beep(BEEP* device)
	{
		d_beep = device;
	}
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
	void set_context_tf20(DEVICE* device)
	{
		d_tf20 = device;
	}
	void notify_power_off();
	void key_down(int code);
	void key_up(int code);
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

