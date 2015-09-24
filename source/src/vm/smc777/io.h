/*
	SONY SMC-777 Emulator 'eSMC-777'

	Author : Takeda.Toshiya
	Date   : 2015.08.13-

	[ i/o and memory bus ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_IO_DATAREC_IN	0
#define SIG_IO_CRTC_DISP	1
#define SIG_IO_CRTC_VSYNC	2
#define SIG_IO_FDC_DRQ		3
#define SIG_IO_FDC_IRQ		4

class IO : public DEVICE
{
private:
	// contexts
	DEVICE *d_cpu, *d_crtc, *d_drec, *d_fdc, *d_pcm, *d_psg;
	uint8* crtc_regs;
	uint8* key_stat;
	uint32* joy_stat;
	
	// memory
	uint8 ram[0x10000];
	uint8 rom[0x4000];
	uint8 cram[0x800];
	uint8 aram[0x800];
	uint8 pcg[0x800];
	uint8 gram[0x8000];
	
	uint8 wdmy[0x4000];
	uint8 rdmy[0x4000];
	uint8* wbank[4];
	uint8* rbank[4];
	
	bool rom_selected;
	int rom_switch_wait;
	int ram_switch_wait;
	
	// keyboard
	uint8 keytable[256];
	uint8 keytable_shift[256];
	uint8 keytable_ctrl[256];
	uint8 keytable_kana[256];
	uint8 keytable_kana_shift[256];
	
	uint8 key_code;
	uint8 key_status;
	uint8 key_cmd;
	int key_repeat_start;
	int key_repeat_interval;
	int key_repeat_event;
	uint8 funckey_code;
	int funckey_index;
	bool caps, kana;
	void initialize_key();
	
	// display
	uint8 gcw;
	bool vsup;
	bool vsync, disp;
	int cblink;
	bool use_palette_text;
	bool use_palette_graph;
	struct {
		int r, g, b;
	} pal[16];
	uint8 text[200][640];
	uint8 graph[200][640];
	scrntype palette_pc[16 + 16];	// color generator + palette board
	
	void draw_text_80x25();
	void draw_text_40x25();
	void draw_graph_640x200();
	void draw_graph_320x200();
	
	// kanji rom
	uint8 kanji[0x23400];
	int kanji_hi, kanji_lo;
	
	// misc
	bool ief_key, ief_vsync;
	bool fdc_irq, fdc_drq;
	bool drec_in;
	
public:
	IO(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~IO() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	uint32 fetch_op(uint32 addr, int *wait);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
#ifdef _IO_DEBUG_LOG
	uint32 read_io8_debug(uint32 addr);
#endif
	void write_signal(int id, uint32 data, uint32 mask);
	void event_callback(int event_id, int err);
	void event_frame();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_crtc(DEVICE* device, uint8* ptr)
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
	void set_context_psg(DEVICE* device)
	{
		d_psg = device;
	}
	void key_down_up(int code, bool down);
	void draw_screen();
	bool warm_start;
};

#endif

