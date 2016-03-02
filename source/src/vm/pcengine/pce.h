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
	uint8_t ram[0x8000];	// ram 32kb
#else
	uint8_t ram[0x2000];	// ram 8kb
#endif
	uint8_t cart[0x400000];	// max 4mb
#ifdef SUPPORT_BACKUP_RAM
	uint8_t backup[0x2000];
	uint32_t backup_crc32;
#endif
	uint32_t bank;
	uint8_t buffer;
	int prev_width;
	bool inserted;
	
	// vdc
	struct {
		int dvssr_write;		/* Set when the DVSSR register has been written to */
		int physical_width;		/* Width of the display */
		int physical_height;		/* Height of the display */
		uint16_t sprite_ram[64*4];	/* Sprite RAM */
		int curline;			/* the current scanline we're on */
		int current_segment;		/* current segment of display */
		int current_segment_line;	/* current line inside a segment of display */
		int vblank_triggered;		/* to indicate whether vblank has been triggered */
		int raster_count;		/* counter to compare RCR against */
		int satb_countdown;		/* scanlines to wait to trigger the SATB irq */
		uint8_t vram[0x10000];
		uint8_t inc;
		uint8_t vdc_register;
		uint8_t vdc_latch;
		pair_t vdc_data[32];
		int status;
		int y_scroll;
	} vdc[2];
	struct {
		uint8_t vce_control;		/* VCE control register */
		pair_t vce_address;		/* Current address in the palette */
		pair_t vce_data[512];		/* Palette data */
		int current_bitmap_line;	/* The current line in the display we are on */
		//bitmap_ind16 *bmp;
		scrntype_t bmp[VDC_LPF][VDC_WPF];
		scrntype_t palette[1024];
	} vce;

	struct {
		struct {
			UINT8 prio;
			UINT8 vdc0_enabled;
			UINT8 vdc1_enabled;
		} vpc_prio[4];
		UINT8	prio_map[512];		/* Pre-calculated priority map */
		pair_t	priority;			/* Priority settings registers */
		pair_t	window1;			/* Window 1 setting */
		pair_t	window2;			/* Window 2 setting */
		UINT8	vdc_select;			/* Which VDC do the ST0, ST1, and ST2 instructions write to */
	} vpc;
	
	void pce_interrupt();
	void sgx_interrupt();
	void vdc_reset();
	void vdc_advance_line(int which);
	void draw_black_line(int line);
	void draw_overscan_line(int line);
	void draw_sgx_overscan_line(int line);
	void vram_write(int which, uint32_t offset, uint8_t data);
	uint8_t vram_read(int which, uint32_t offset);
	void vdc_w(int which, uint16_t offset, uint8_t data);
	uint8_t vdc_r(int which, uint16_t offset);
	void vce_w(uint16_t offset, uint8_t data);
	uint8_t vce_r(uint16_t offset);
	void pce_refresh_line(int which, int line, int external_input, uint8_t *drawn, scrntype_t *line_buffer);
	void conv_obj(int which, int i, int l, int hf, int vf, char *buf);
	void pce_refresh_sprites(int which, int line, uint8_t *drawn, scrntype_t *line_buffer);
	void vdc_do_dma(int which);
	void vpc_update_prio_map();
	void vpc_w(uint16_t offset, uint8_t data);
	uint8_t vpc_r(uint16_t offset);
	void sgx_vdc_w(uint16_t offset, uint8_t data);
	uint8_t sgx_vdc_r(uint16_t offset);
	
	// psg
	typedef struct {
		// registers
		uint8_t regs[8];
		uint8_t wav[32];
		uint8_t wavptr;
		// sound gen
		uint32_t genptr;
		uint32_t remain;
		bool noise;
		uint32_t randval;
	} psg_t;
	psg_t psg[8];
	uint8_t psg_ch, psg_vol, psg_lfo_freq, psg_lfo_ctrl;
	int sample_rate;
	int volume_l, volume_r;
	void psg_reset();
	void psg_write(uint16_t addr, uint8_t data);
	uint8_t psg_read(uint16_t addr);
	
	// joypad
	const uint32_t *joy_stat;
	uint8_t joy_sel, joy_clr, joy_count, joy_bank;
	bool joy_6btn;
	
	void joy_reset();
	void joy_write(uint16_t addr, uint8_t data);
	uint8_t joy_read(uint16_t addr);
	
public:
	PCE(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
	}
	~PCE() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_vline(int v, int clock);
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
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
	void open_cart(const _TCHAR* file_path);
	void close_cart();
	bool is_cart_inserted()
	{
		return inserted;
	}
	void draw_screen();
};

#endif
