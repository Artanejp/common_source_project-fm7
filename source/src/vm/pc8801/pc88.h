/*
	NEC PC-8001 Emulator 'ePC-8001'
	NEC PC-8001mkII Emulator 'ePC-8001mkII'
	NEC PC-8001mkIISR Emulator 'ePC-8001mkIISR'
	NEC PC-8801 Emulator 'ePC-8801'
	NEC PC-8801mkII Emulator 'ePC-8801mkII'
	NEC PC-8801MA Emulator 'ePC-8801MA'
	NEC PC-98DO Emulator 'ePC-98DO'

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
#ifdef SUPPORT_PC88_OPN1
#define SIG_PC88_OPN1_IRQ	1
#endif
#ifdef SUPPORT_PC88_OPN2
#define SIG_PC88_OPN2_IRQ	2
#endif
#ifdef SUPPORT_PC88_FDD_8INCH
#define SIG_PC88_8INCH_IRQ	3
#define SIG_PC88_8INCH_DRQ	4
#endif
#ifdef SUPPORT_PC88_CDROM
#define SIG_PC88_SCSI_DRQ	5
#endif
#ifdef SUPPORT_PC88_GSX8800
#define SIG_PC88_GSX_IRQ	6
#endif
#define SIG_PC88_USART_OUT	7

#define CMT_BUFFER_SIZE		0x40000

#if !defined(_PC8001)
#define SUPPORT_PC88_GVRAM
#endif

#if defined(PC8801_VARIANT)
#define NIPPY_PATCH
#endif

#if defined(SUPPORT_PC88_OPN1) || defined(SUPPORT_PC88_OPN2)
class YM2203;
#endif
class Z80;
#ifdef SUPPORT_PC88_FDD_8INCH
class UPD765A;
#endif
#ifdef SUPPORT_PC88_CDROM
class SCSI_HOST;
class SCSI_CDROM;
#endif

typedef struct {
	struct {
		int rate, counter;
		uint8_t cursor, attrib;
	} blink;
	struct {
		int type, mode;
		int x, y;
	} cursor;
	struct {
		uint8_t data;
		int num;
		uint8_t expand[200][80];
	} attrib;
	struct {
		uint8_t expand[200][80];
	} text;
	int width, height;
	int char_height;
	bool skip_line;
	int vretrace;
	bool timing_changed;
	uint8_t buffer[120 * 200];
	int buffer_ptr;
	uint8_t cmd;
	int cmd_ptr;
	uint8_t mode, reverse, intr_mask, status;
	bool vblank;
	
	void reset(bool hireso);
	void write_cmd(uint8_t data);
	void write_param(uint8_t data);
	uint32_t read_param();
	uint32_t read_status();
	void start();
	void finish();
	void write_buffer(uint8_t data);
	uint8_t read_buffer(int ofs);
	void update_blink();
	void expand_buffer(bool hireso, bool line400);
	void set_attrib(uint8_t code);
} pc88_crtc_t;

typedef struct {
	struct {
		pair32_t addr, count;
		uint8_t mode;
		int nbytes;
		DEVICE *io;
		bool running;
	} ch[4];
	uint8_t mode, status;
	bool high_low;
	DEVICE *mem;
	
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void start(int c);
	void run(int c, int nbytes);
	void finish(int c);
} pc88_dmac_t;

class PC88 : public DEVICE
{
private:
	Z80 *d_cpu;
	DEVICE *d_pcm, *d_pio, *d_prn, *d_rtc, *d_sio;
#ifdef SUPPORT_PC88_FDD_8INCH
	UPD765A *d_fdc_8inch;
#endif
#ifdef SUPPORT_PC88_CDROM
	SCSI_HOST* d_scsi_host;
	SCSI_CDROM* d_scsi_cdrom;
#endif
#ifdef SUPPORT_PC88_OPN1
	YM2203 *d_opn1;
#endif
#ifdef SUPPORT_PC88_OPN2
	YM2203 *d_opn2;
#endif
#ifdef SUPPORT_PC88_HMB20
	DEVICE *d_opm;
#endif
#ifdef SUPPORT_PC88_GSX8800
//	DEVICE *d_gsx_pit;
	DEVICE *d_gsx_psg1, *d_gsx_psg2, *d_gsx_psg3, *d_gsx_psg4;
#endif
#ifdef SUPPORT_PC88_PCG8100
	DEVICE *d_pcg_pit, *d_pcg_pcm1, *d_pcg_pcm2, *d_pcg_pcm3;
#endif
#ifdef SUPPORT_M88_DISKDRV
	DEVICE *d_diskio;
#endif
	
	uint8_t* rbank[16];
	uint8_t* wbank[16];
	uint8_t wdmy[0x1000];
	uint8_t rdmy[0x1000];
	
	uint8_t ram[0x10000];
#if defined(PC88_EXRAM_BANKS)
	uint8_t exram[0x8000 * PC88_EXRAM_BANKS];
#endif
#if defined(SUPPORT_PC88_GVRAM)
	uint8_t gvram[0xc000];
	uint8_t gvram_null[0x4000];
#endif
#if defined(PC8801SR_VARIANT)
	uint8_t tvram[0x1000];
#endif
#if defined(PC8001_VARIANT)
	uint8_t n80rom[0x8000];
#if defined(_PC8001MK2) || defined(_PC8001SR)
	uint8_t n80erom[0x2000];
#endif
#if defined(_PC8001SR)
	uint8_t n80srrom[0xa000];
	uint8_t hiragana[0x200];
	uint8_t katakana[0x200];
#endif
#else
	uint8_t n88rom[0x8000];
	uint8_t n88exrom[0x8000];
	uint8_t n80rom[0x8000];
	uint8_t n88erom[9][0x2000];
	int n88erom_loaded;
#endif
//#ifdef SUPPORT_PC88_KANJI1
	uint8_t kanji1[0x20000];
//#endif
#ifdef SUPPORT_PC88_KANJI2
	uint8_t kanji2[0x20000];
#endif
#ifdef SUPPORT_PC88_DICTIONARY
	uint8_t dicrom[0x80000];
#endif
#ifdef SUPPORT_PC88_CDROM
	uint8_t cdbios[0x10000];
	bool cdbios_loaded;
#endif
	
	// i/o port
	uint8_t port[256];
	uint8_t prev_port[256];
	
	pc88_crtc_t crtc;
	pc88_dmac_t dmac;
	
	// memory mapper
#if defined(_PC8001SR) || defined(PC8801SR_VARIANT)
	uint8_t alu_reg[3];
#endif
#if defined(SUPPORT_PC88_GVRAM)
	uint8_t gvram_plane, gvram_sel;
#endif
	
	void update_timing();
	int get_m1_wait(bool addr_f000);
	int get_main_wait(bool read);
#if defined(PC8801SR_VARIANT)
	int get_tvram_wait(bool read);
#endif
#if defined(SUPPORT_PC88_GVRAM)
	int get_gvram_wait(bool read);
	void update_gvram_wait();
	void update_gvram_sel();
#endif
#if defined(PC8001_VARIANT)
	void update_n80_write();
	void update_n80_read();
#else
	void update_low_write();
	void update_low_read();
#if defined(PC8801SR_VARIANT)
	void update_tvram_memmap();
#endif
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
#if defined(PC8801SR_VARIANT)
	int tvram_wait_clocks_r, tvram_wait_clocks_w;
#endif
#if defined(SUPPORT_PC88_GVRAM)
	int gvram_wait_clocks_r, gvram_wait_clocks_w;
#endif
	int busreq_clocks;
	
	// screen
	typedef struct palette_s {
		uint8_t b, r, g;
	} palette_t;
	palette_t palette[10]; // 8 = digital back color, 9 = analog back color
	bool update_palette;
	bool hireso;
	
	uint8_t sg_pattern[0x800];
	uint8_t text[200][640];
	uint8_t text_color[200][80];
	bool text_reverse[200][80];
	uint8_t graph[400][640];
	
	palette_t palette_digital[9];
	palette_t palette_analog [9];
	palette_t palette_line_digital[400][9];
	palette_t palette_line_analog [400][9];
	bool palette_line_changed[400];
#if defined(SUPPORT_PC88_VAB)
	scrntype_t palette_vab_pc[0x10000];
#endif
	
	void draw_text();
#if defined(SUPPORT_PC88_GVRAM)
#if defined(PC8001_VARIANT)
#if defined(_PC8001SR)
	bool draw_320x200_color_graph();
#endif
	bool draw_320x200_4color_graph();
	void draw_320x200_attrib_graph();
#endif
	bool draw_640x200_color_graph();
	void draw_640x200_mono_graph();
	void draw_640x200_attrib_graph();
#if defined(PC8801_VARIANT)
	void draw_640x400_mono_graph();
	void draw_640x400_attrib_graph();
#endif
#endif
	
	// misc
	bool usart_dcd;
	bool opn_busy;
	
	// keyboard
	uint8_t key_status[256];
	uint8_t key_caps, key_kana;
	
#ifdef SUPPORT_PC88_JOYSTICK
	// joystick & mouse
	const uint32_t *joystick_status;
	const int32_t* mouse_status;
	uint32_t mouse_strobe_clock;
	uint32_t mouse_strobe_clock_lim;
	int mouse_phase;
	int mouse_dx, mouse_dy;
	int mouse_lx, mouse_ly;
#endif
	
	// intterrupt
	uint8_t intr_req;
#ifdef SUPPORT_PC88_OPN1
	bool intr_req_opn1;
#endif
#ifdef SUPPORT_PC88_OPN2
	bool intr_req_opn2;
#endif
	uint8_t intr_mask1, intr_mask2;
	void request_intr(int level, bool status);
	void update_intr();
	
	// data recorder
	FILEIO *cmt_fio;
	bool cmt_play, cmt_rec;
	_TCHAR rec_file_path[_MAX_PATH];
	int cmt_bufptr, cmt_bufcnt;
	uint8_t cmt_buffer[CMT_BUFFER_SIZE];
	int cmt_data_carrier[1024], cmt_data_carrier_cnt;
	int cmt_register_id;
	
	void release_tape();
	bool check_data_carrier();
	
	// beep/sing
	bool beep_on, beep_signal, sing_signal;
	
#ifdef SUPPORT_PC88_PCG8100
	// pcg
	uint16_t pcg_addr;
	uint8_t pcg_data, pcg_ctrl;
	uint8_t pcg_pattern[0x800];
#endif
	
#ifdef SUPPORT_PC88_CDROM
	int cdda_register_id;
	double cdda_volume;
#endif
	
#ifdef NIPPY_PATCH
	// dirty patch for NIPPY
	bool nippy_patch;
#endif
	
public:
	PC88(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
#ifdef SUPPORT_PC88_FDD_8INCH
		d_fdc_8inch = NULL;
#endif
#ifdef SUPPORT_PC88_OPN1
		d_opn1 = NULL;
#endif
#ifdef SUPPORT_PC88_OPN2
		d_opn2 = NULL;
#endif
#ifdef SUPPORT_PC88_HMB20
		d_opm = NULL;
#endif
#ifdef SUPPORT_PC88_GSX8800
//		d_gsx_pit = NULL;
		d_gsx_psg1 = NULL;
		d_gsx_psg2 = NULL;
		d_gsx_psg3 = NULL;
		d_gsx_psg4 = NULL;
#endif
#ifdef SUPPORT_PC88_PCG8100
		d_pcg_pit = NULL;
		d_pcg_pcm1 = NULL;
		d_pcg_pcm2 = NULL;
		d_pcg_pcm3 = NULL;
#endif
#ifdef SUPPORT_M88_DISKDRV
		d_diskio = NULL;
#endif
#if defined(PC8001_VARIANT)
		set_device_name(_T("PC-8001 Core"));
#else
		set_device_name(_T("PC-8801 Core"));
#endif
	}
	~PC88() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	
	void write_data8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data8w(uint32_t addr, int* wait);
	uint32_t fetch_op(uint32_t addr, int *wait);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
#ifdef _IO_DEBUG_LOG
	uint32_t read_io8_debug(uint32_t addr);
#endif
	
	uint32_t read_dma_data8(uint32_t addr);
	void write_dma_data8(uint32_t addr, uint32_t data);
	void write_dma_io8(uint32_t addr, uint32_t data);
	
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	void event_frame();
	void event_vline(int v, int clock);
	uint32_t get_intr_ack();
	void notify_intr_ei();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
#ifdef PC8801SR_VARIANT
	bool is_sr_mr()
	{
		return (n88rom[0x79d7] < 0x38);
	}
#endif
	void set_context_cpu(Z80* device)
	{
		d_cpu = device;
	}
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
#ifdef SUPPORT_PC88_FDD_8INCH
	void set_context_fdc_8inch(UPD765A* device)
	{
		d_fdc_8inch = device;
	}
#endif
#ifdef SUPPORT_PC88_CDROM
	void set_context_scsi_host(SCSI_HOST* device)
	{
		d_scsi_host = device;
	}
	void set_context_scsi_cdrom(SCSI_CDROM* device)
	{
		d_scsi_cdrom = device;
	}
#endif
#ifdef SUPPORT_PC88_OPN1
	void set_context_opn1(YM2203* device)
	{
		d_opn1 = device;
	}
#endif
#ifdef SUPPORT_PC88_OPN2
	void set_context_opn2(YM2203* device)
	{
		d_opn2 = device;
	}
#endif
#ifdef SUPPORT_PC88_HMB20
	void set_context_opm(DEVICE* device)
	{
		d_opm = device;
	}
#endif
#ifdef SUPPORT_PC88_GSX8800
//	void set_context_gsx_pit(DEVICE* device)
//	{
//		d_gsx_pit = device;
//	}
	void set_context_gsx_psg1(DEVICE* device)
	{
		d_gsx_psg1 = device;
	}
	void set_context_gsx_psg2(DEVICE* device)
	{
		d_gsx_psg2 = device;
	}
	void set_context_gsx_psg3(DEVICE* device)
	{
		d_gsx_psg3 = device;
	}
	void set_context_gsx_psg4(DEVICE* device)
	{
		d_gsx_psg4 = device;
	}
#endif
#ifdef SUPPORT_PC88_PCG8100
	void set_context_pcg_pit(DEVICE* device)
	{
		d_pcg_pit = device;
	}
	void set_context_pcg_pcm1(DEVICE* device)
	{
		d_pcg_pcm1 = device;
	}
	void set_context_pcg_pcm2(DEVICE* device)
	{
		d_pcg_pcm2 = device;
	}
	void set_context_pcg_pcm3(DEVICE* device)
	{
		d_pcg_pcm3 = device;
	}
#endif
#ifdef SUPPORT_M88_DISKDRV
	void set_context_diskio(DEVICE* device)
	{
		d_diskio = device;
	}
#endif
	void key_down(int code, bool repeat);
	bool get_caps_locked()
	{
		return (key_caps != 0);
	}
	bool get_kana_locked()
	{
		return (key_kana != 0);
	}
	
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted()
	{
		return (cmt_play || cmt_rec);
	}
	bool is_frame_skippable();
	
	void draw_screen();
};

#endif

