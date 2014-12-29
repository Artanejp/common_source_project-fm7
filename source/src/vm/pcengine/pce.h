/*
	NEC-HE PC Engine Emulator 'ePCEngine'
	SHARP X1twin Emulator 'eX1twin'

	Origin : Ootake (joypad)
	       : xpce (psg)
	       : MESS (vdc/vce/vpc)
	Author : Takeda.Toshiya
	Date   : 2009.03.11-

	[ PC-Eninge ]
*/

#ifndef _PCE_H_
#define _PCE_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define VDC_WPF		684	/* width of a line in frame including blanking areas */
#define VDC_LPF		262	/* number of lines in a single frame */

class HUC6280;

class PCE : public DEVICE
{
private:
	HUC6280* d_cpu;
	
	bool support_6btn, support_sgfx;
	
	// memory
#ifdef SUPPORT_SUPER_GFX
	uint8 ram[0x8000];	// ram 32kb
#else
	uint8 ram[0x2000];	// ram 8kb
#endif
	uint8 cart[0x400000];	// max 4mb
#ifdef SUPPORT_BACKUP_RAM
	uint8 backup[0x2000];
	uint32 backup_crc32;
#endif
	uint32 bank;
	uint8 buffer;
	int prev_width;
	bool inserted;
	
	// vdc
	struct {
		int dvssr_write;		/* Set when the DVSSR register has been written to */
		int physical_width;		/* Width of the display */
		int physical_height;		/* Height of the display */
		uint16 sprite_ram[64*4];	/* Sprite RAM */
		int curline;			/* the current scanline we're on */
		int current_segment;		/* current segment of display */
		int current_segment_line;	/* current line inside a segment of display */
		int vblank_triggered;		/* to indicate whether vblank has been triggered */
		int raster_count;		/* counter to compare RCR against */
		int satb_countdown;		/* scanlines to wait to trigger the SATB irq */
		uint8 vram[0x10000];
		uint8 inc;
		uint8 vdc_register;
		uint8 vdc_latch;
		pair vdc_data[32];
		int status;
		int y_scroll;
	} vdc[2];
	struct {
		uint8 vce_control;		/* VCE control register */
		pair vce_address;		/* Current address in the palette */
		pair vce_data[512];		/* Palette data */
		int current_bitmap_line;	/* The current line in the display we are on */
		//bitmap_ind16 *bmp;
		scrntype bmp[VDC_LPF][VDC_WPF];
		scrntype palette[1024];
	} vce;

	struct {
		struct {
			UINT8 prio;
			UINT8 vdc0_enabled;
			UINT8 vdc1_enabled;
		} vpc_prio[4];
		UINT8	prio_map[512];		/* Pre-calculated priority map */
		pair	priority;			/* Priority settings registers */
		pair	window1;			/* Window 1 setting */
		pair	window2;			/* Window 2 setting */
		UINT8	vdc_select;			/* Which VDC do the ST0, ST1, and ST2 instructions write to */
	} vpc;
	
	void pce_interrupt();
	void sgx_interrupt();
	void vdc_reset();
	void vdc_advance_line(int which);
	void draw_black_line(int line);
	void draw_overscan_line(int line);
	void draw_sgx_overscan_line(int line);
	void vram_write(int which, uint32 offset, uint8 data);
	uint8 vram_read(int which, uint32 offset);
	void vdc_w(int which, uint16 offset, uint8 data);
	uint8 vdc_r(int which, uint16 offset);
	void vce_w(uint16 offset, uint8 data);
	uint8 vce_r(uint16 offset);
	void pce_refresh_line(int which, int line, int external_input, uint8 *drawn, scrntype *line_buffer);
	void conv_obj(int which, int i, int l, int hf, int vf, char *buf);
	void pce_refresh_sprites(int which, int line, uint8 *drawn, scrntype *line_buffer);
	void vdc_do_dma(int which);
	void vpc_update_prio_map();
	void vpc_w(uint16 offset, uint8 data);
	uint8 vpc_r(uint16 offset);
	void sgx_vdc_w(uint16 offset, uint8 data);
	uint8 sgx_vdc_r(uint16 offset);
	
	// psg
	typedef struct {
		// registers
		uint8 regs[8];
		uint8 wav[32];
		uint8 wavptr;
		// sound gen
		uint32 genptr;
		uint32 remain;
		bool noise;
		uint32 randval;
	} psg_t;
	psg_t psg[8];
	uint8 psg_ch, psg_vol, psg_lfo_freq, psg_lfo_ctrl;
	int sample_rate;
	void psg_reset();
	void psg_write(uint16 addr, uint8 data);
	uint8 psg_read(uint16 addr);
	
	// joypad
	uint32 *joy_stat;
	uint8 *key_stat;
	uint8 joy_sel, joy_clr, joy_count, joy_bank;
	bool joy_6btn;
	
	void joy_reset();
	void joy_write(uint16 addr, uint8 data);
	uint8 joy_read(uint16 addr);
	
public:
	PCE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~PCE() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_vline(int v, int clock);
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void mix(int32* buffer, int cnt);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_cpu(HUC6280* device)
	{
		d_cpu = device;
	}
	void initialize_sound(int rate)
	{
		sample_rate = rate;
	}
	void open_cart(_TCHAR* file_path);
	void close_cart();
	bool cart_inserted()
	{
		return inserted;
	}
	void draw_screen();
};

#endif
