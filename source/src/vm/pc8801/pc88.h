/*
	NEC PC-98DO Emulator 'ePC-98DO'
	NEC PC-8801MA Emulator 'ePC-8801MA'
	NEC PC-8001mkIISR Emulator 'ePC-8001mkIISR'

	Author : Takeda.Toshiya
	Date   : 2011.12.29-

	[ PC-8801 ]
*/

#ifndef _PC88_H_
#define _PC88_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_PC88_USART_IRQ	0
#define SIG_PC88_SOUND_IRQ	1
#ifdef SUPPORT_PC88_SB2
#define SIG_PC88_SB2_IRQ	2
#endif
#define SIG_PC88_USART_OUT	3
#define SIG_PC88_DATAREC_MIX	4
#define SIG_PC88_DATAREC_VOLUME	5


#define CMT_BUFFER_SIZE		0x40000

#if defined(_PC8001SR) && !defined(PC88_EXRAM_BANKS)
#define PC88_EXRAM_BANKS	1
#endif

#if !defined(_PC8001SR)
#define NIPPY_PATCH
#endif

class YM2203;
class Z80;

typedef struct {
	struct {
		int rate, counter;
		uint8 cursor, attrib;
	} blink;
	struct {
		int type, mode;
		int x, y;
	} cursor;
	struct {
		uint8 data, mask;
		int num;
		uint8 expand[200][80];
	} attrib;
	struct {
		uint8 expand[200][80];
	} text;
	int width, height;
	int char_height;
	bool skip_line;
	int vretrace;
	bool timing_changed;
	uint8 buffer[120 * 200];
	int buffer_ptr;
	uint8 cmd;
	int cmd_ptr;
	uint8 mode, reverse, intr_mask, status;
	bool vblank;
	
	void reset(bool hireso);
	void write_cmd(uint8 data);
	void write_param(uint8 data);
	uint32 read_param();
	uint32 read_status();
	void start();
	void finish();
	void write_buffer(uint8 data);
	uint8 read_buffer(int ofs);
	void update_blink();
	void expand_buffer(bool hireso, bool line400);
	void set_attrib(uint8 code);
} pc88_crtc_t;

typedef struct {
	struct {
		pair addr, count;
		uint8 mode;
		int nbytes;
		DEVICE *io;
		bool running;
	} ch[4];
	uint8 mode, status;
	bool high_low;
	DEVICE *mem;
	
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void start(int c);
	void run(int c, int nbytes);
	void finish(int c);
} pc88_dmac_t;

class PC88 : public DEVICE
{
private:
	YM2203 *d_opn;
#ifdef SUPPORT_PC88_SB2
	YM2203 *d_sb2;
#endif
	Z80 *d_cpu;
	DEVICE *d_pcm, *d_pio, *d_prn, *d_rtc, *d_sio;
#ifdef SUPPORT_PC88_PCG8100
	DEVICE *d_pcg_pit, *d_pcg_pcm0, *d_pcg_pcm1, *d_pcg_pcm2;
#endif
	
	uint8* rbank[16];
	uint8* wbank[16];
	uint8 wdmy[0x1000];
	uint8 rdmy[0x1000];
	
	uint8 ram[0x10000];
#if defined(PC88_EXRAM_BANKS)
	uint8 exram[0x8000 * PC88_EXRAM_BANKS];
#endif
	uint8 gvram[0xc000];
	uint8 gvram_null[0x4000];
	uint8 tvram[0x1000];
#if defined(_PC8001SR)
	uint8 n80mk2rom[0x8000];
	uint8 n80mk2srrom[0xa000];
#else
	uint8 n88rom[0x8000];
	uint8 n88exrom[0x8000];
	uint8 n80rom[0x8000];
#endif
	uint8 kanji1[0x20000];
	uint8 kanji2[0x20000];
#ifdef SUPPORT_PC88_DICTIONARY
	uint8 dicrom[0x80000];
#endif
	
	// i/o port
	uint8 port[256];
	
	pc88_crtc_t crtc;
	pc88_dmac_t dmac;
	
	// memory mapper
	uint8 alu_reg[3];
	uint8 gvram_plane, gvram_sel;
	
	void update_timing();
	int get_m1_wait(bool addr_f000);
	int get_main_wait(bool read);
	int get_tvram_wait(bool read);
	int get_gvram_wait(bool read);
	void update_gvram_wait();
	void update_gvram_sel();
#if defined(_PC8001SR)
	void update_n80_write();
	void update_n80_read();
#else
	void update_low_memmap();
	void update_tvram_memmap();
#endif
	
	// cpu
	bool cpu_clock_low;
#if defined(SUPPORT_PC88_HIGH_CLOCK)
	bool cpu_clock_high_fe2;
#endif
	bool mem_wait_on;
	int m1_wait_clocks;
	int f000_m1_wait_clocks;
	int mem_wait_clocks_r, mem_wait_clocks_w;
	int tvram_wait_clocks_r, tvram_wait_clocks_w;
	int gvram_wait_clocks_r, gvram_wait_clocks_w;
	int busreq_clocks;
	
	// screen
	struct {
		uint8 b, r, g;
	} palette[9];
	bool update_palette;
	bool hireso;
	
	uint8 sg_pattern[0x800];
	uint8 text[200][640];
	uint8 graph[400][640];
	scrntype palette_text_pc[9];	// 0 = back color for attrib mode, 8 = black
	scrntype palette_graph_pc[8];
	
	void draw_text();
#if defined(_PC8001SR)
	bool draw_320x200_color_graph();
	bool draw_320x200_4color_graph();
	void draw_320x200_attrib_graph();
#endif
	bool draw_640x200_color_graph();
	void draw_640x200_mono_graph();
#if defined(_PC8001SR)
	void draw_640x200_attrib_graph();
#else
	void draw_640x400_attrib_graph();
#endif
	
	// misc
	bool usart_dcd;
	bool opn_busy;
	
	// keyboard
	uint8 key_status[256];
	uint8 key_caps, key_kana;
	
#ifdef SUPPORT_PC88_JOYSTICK
	// joystick & mouse
	uint32 *joystick_status;
	int* mouse_status;
	uint32 mouse_strobe_clock;
	uint32 mouse_strobe_clock_lim;
	int mouse_phase;
	int mouse_dx, mouse_dy;
	int mouse_lx, mouse_ly;
#endif
	
	// intterrupt
	uint8 intr_req;
	bool intr_req_sound;
#ifdef SUPPORT_PC88_SB2
	bool intr_req_sb2;
#endif
	uint8 intr_mask1, intr_mask2;
	void request_intr(int level, bool status);
	void update_intr();
	
	// data recorder
	FILEIO *cmt_fio;
	bool cmt_play, cmt_rec;
	_TCHAR rec_file_path[_MAX_PATH];
	int cmt_bufptr, cmt_bufcnt;
	uint8 cmt_buffer[CMT_BUFFER_SIZE];
	int cmt_data_carrier[1024], cmt_data_carrier_cnt;
	int cmt_register_id;
	
	void release_tape();
	bool check_data_carrier();
	
	// beep/sing
	bool beep_on, beep_signal, sing_signal;
#ifdef SUPPORT_PC88_PCG8100
	// pcg
	uint16 pcg_addr;
	uint8 pcg_data, pcg_ctrl;
	uint8 pcg_pattern[0x800];
#endif
	
#ifdef NIPPY_PATCH
	// dirty patch for NIPPY
	bool nippy_patch;
#endif
	
public:
	PC88(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~PC88() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	
	void write_data8w(uint32 addr, uint32 data, int* wait);
	uint32 read_data8w(uint32 addr, int* wait);
	uint32 fetch_op(uint32 addr, int *wait);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
#ifdef _IO_DEBUG_LOG
	uint32 read_io8_debug(uint32 addr);
#endif
	
	uint32 read_dma_data8(uint32 addr);
	void write_dma_io8(uint32 addr, uint32 data);
	
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	void event_frame();
	void event_vline(int v, int clock);
	uint32 intr_ack();
	void intr_ei();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	bool is_sr_mr()
	{
#if !defined(_PC8001SR)
		return (n88rom[0x79d7] < 0x38);
#else
		return true;
#endif
	}
	void set_context_cpu(Z80* device)
	{
		d_cpu = device;
	}
	void set_context_opn(YM2203* device)
	{
		d_opn = device;
	}
#ifdef SUPPORT_PC88_SB2
	void set_context_sb2(YM2203* device)
	{
		d_sb2 = device;
	}
#endif
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_context_prn(DEVICE* device)
	{
		d_prn = device;
	}
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
#ifdef SUPPORT_PC88_PCG8100
	void set_context_pcg_pit(DEVICE* device)
	{
		d_pcg_pit = device;
	}
	void set_context_pcg_pcm0(DEVICE* device)
	{
		d_pcg_pcm0 = device;
	}
	void set_context_pcg_pcm1(DEVICE* device)
	{
		d_pcg_pcm1 = device;
	}
	void set_context_pcg_pcm2(DEVICE* device)
	{
		d_pcg_pcm2 = device;
	}
#endif
	void key_down(int code, bool repeat);
	
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool tape_inserted()
	{
		return (cmt_play || cmt_rec);
	}
	bool now_skip();
	
	void draw_screen();
};

#endif

