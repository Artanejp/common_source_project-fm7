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

#define CMT_BUFFER_SIZE		0x40000

class BEEP;
class FIFO;

class MEMORY : public DEVICE
{
private:
	BEEP *d_beep;
	DEVICE *d_cpu, *d_rtc, *d_sio_tf20;
	
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t* wbank[8];
	uint8_t* rbank[8];
	
	// memory with expansion unit
	uint8_t ram[0x8000];	// 0000h-7fffh
	uint8_t rom[0x8000];	// 8000h-ffffh (internal)
	uint8_t ext[0x4000];	// 8000h-bfffh
	
	FIFO *cmd_buf;
	bool sio_select;
	bool special_cmd_masked;
	uint8_t slave_mem[0x10000];
	
	struct {
		double freq;
		int period;
		int remain;
	} sound[256];
	int sound_ptr;
	int sound_count;
	uint8_t sound_reply;
	double sound_freq;
	double tone_table[57];
	
	uint8_t key_stat[256], key_flag[256];
	int key_data, key_strobe, key_intmask;
	
	FILEIO* cmt_fio;
	bool cmt_play, cmt_rec;
	_TCHAR cmt_file_path[_MAX_PATH];
	int cmt_count;
	uint8_t cmt_buffer[CMT_BUFFER_SIZE];
	
	typedef struct {
		uint8_t buffer[80];
		int bank;
		int addr;
	} lcd_t;
	lcd_t lcd[6];
	
	scrntype_t lcd_render[32][120];
	scrntype_t pd, pb;
	uint8_t lcd_select, lcd_data;
	int lcd_clock;
	
	int int_status;
	int int_mask;
	
	void update_sound();
	void update_keyboard();
	void update_intr();
	void send_to_slave(uint8_t val);
	void send_to_main(uint8_t val);
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
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
	void set_context_sio_tf20(DEVICE* device)
	{
		d_sio_tf20 = device;
	}
	void notify_power_off();
	void key_down(int code);
	void key_up(int code);
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted()
	{
		return (cmt_play || cmt_rec);
	}
	void draw_screen();
};

#endif

