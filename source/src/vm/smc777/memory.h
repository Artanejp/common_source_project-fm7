/*
	SONY SMC-70 Emulator 'eSMC-70'
	SONY SMC-777 Emulator 'eSMC-777'

	Author : Takeda.Toshiya
	Date   : 2015.08.13-

	[ memory and i/o bus ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_DATAREC_IN	0
#define SIG_MEMORY_CRTC_DISP	1
#define SIG_MEMORY_CRTC_VSYNC	2
#define SIG_MEMORY_FDC_DRQ	3
#define SIG_MEMORY_FDC_IRQ	4
#if defined(_SMC70)
#define SIG_MEMORY_RTC_DATA	5
#define SIG_MEMORY_RTC_BUSY	6
#endif

class MEMORY : public DEVICE
{
private:
	// contexts
	DEVICE *d_cpu, *d_crtc, *d_drec, *d_fdc, *d_pcm;
#if defined(_SMC70)
	DEVICE *d_rtc;
#elif defined(_SMC777)
	DEVICE *d_psg;
#endif
	uint8_t* crtc_regs;
	const uint8_t* key_stat;
	const uint32_t* joy_stat;
	
	// memory
	uint8_t ram[0x10000];
#if defined(_SMC70)
	uint8_t rom[0x8000];
#else
	uint8_t rom[0x4000];
#endif
	uint8_t cram[0x800];
	uint8_t aram[0x800];
	uint8_t pcg[0x800];
	uint8_t gram[0x8000];
	
	uint8_t wdmy[0x4000];
	uint8_t rdmy[0x4000];
	uint8_t* wbank[4];
	uint8_t* rbank[4];
	
	bool rom_selected;
	int rom_switch_wait;
	int ram_switch_wait;
	
	// keyboard
	uint8_t keytable[256];
	uint8_t keytable_shift[256];
	uint8_t keytable_ctrl[256];
	uint8_t keytable_kana[256];
	uint8_t keytable_kana_shift[256];
	
	uint8_t key_code;
	uint8_t key_status;
	uint8_t key_cmd;
	int key_repeat_start;
	int key_repeat_interval;
	int key_repeat_event;
	uint8_t funckey_code;
	int funckey_index;
	bool caps, kana;
	void initialize_key();
	
	// display
	uint8_t gcw;
	bool vsup;
	bool vsync, disp, blink;
	int cblink;
#if defined(_SMC777)
	bool use_palette_text;
	bool use_palette_graph;
	struct {
		int r, g, b;
	} pal[16];
#endif
	uint8_t text[200][640];
	uint8_t graph[400][640];
	scrntype_t palette_pc[16 + 16];	// color generator + palette board
#if defined(_SMC70)
	scrntype_t palette_bw_pc[2];
#else
	scrntype_t palette_line_text_pc[200][16];
	scrntype_t palette_line_graph_pc[200][16];
#endif
	
	void draw_text_80x25(int v);
	void draw_text_40x25(int v);
	void draw_graph_640x400(int v);
	void draw_graph_640x200(int v);
	void draw_graph_320x200(int v);
	void draw_graph_160x100(int v);
	
	// kanji rom
	uint8_t kanji[0x23400];
#if defined(_SMC70)
	uint8_t basic[0x8000];
#endif
	int kanji_hi, kanji_lo;
	
	// misc
	bool ief_key, ief_vsync;
	bool vsync_irq;
	bool fdc_irq, fdc_drq;
	bool drec_in;
#if defined(_SMC70)
	uint8_t rtc_data;
	bool rtc_busy;
#endif
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	uint32_t fetch_op(uint32_t addr, int *wait);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
#ifdef _IO_DEBUG_LOG
	uint32_t read_io8_debug(uint32_t addr);
#endif
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	void event_frame();
	void event_vline(int v, int clock);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_crtc(DEVICE* device, uint8_t* ptr)
	{
		d_crtc = device;
		crtc_regs = ptr;
	}
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
#if defined(_SMC70)
	void set_context_rtc(DEVICE* device)
	{
		d_rtc = device;
	}
#elif defined(_SMC777)
	void set_context_psg(DEVICE* device)
	{
		d_psg = device;
	}
#endif
	void key_down_up(int code, bool down);
	bool get_caps_locked()
	{
		return caps;
	}
	bool get_kana_locked()
	{
		return kana;
	}
	void draw_screen();
	bool warm_start;
};

#endif

