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
	uint8_t* crtc_regs;
	const uint8_t* key_stat;
	const uint32_t* joy_stat;
	
	// memory
	uint8_t ram[0x10000];
	uint8_t rom[0x4000];
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
	bool vsync, disp;
	int cblink;
	bool use_palette_text;
	bool use_palette_graph;
	struct {
		int r, g, b;
	} pal[16];
	uint8_t text[200][640];
	uint8_t graph[200][640];
	scrntype_t palette_pc[16 + 16];	// color generator + palette board
	
	void draw_text_80x25();
	void draw_text_40x25();
	void draw_graph_640x200();
	void draw_graph_320x200();
	
	// kanji rom
	uint8_t kanji[0x23400];
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
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
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
	void set_context_psg(DEVICE* device)
	{
		d_psg = device;
	}
	void key_down_up(int code, bool down);
	void draw_screen();
	bool warm_start;
};

#endif

