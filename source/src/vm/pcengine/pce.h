/*
	NEC-HE PC Engine Emulator 'ePCEngine'
	SHARP X1twin Emulator 'eX1twin'

	Origin : Ootake (joypad/cdrom)
	       : xpce (psg)
	       : MESS (vdc/vce/vpc/cdrom)
	Author : Takeda.Toshiya
	Date   : 2009.03.11-

	[ PC-Engine ]
*/

#ifndef _PCE_H_
#define _PCE_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#ifdef SUPPORT_CDROM
#define SIG_PCE_SCSI_IRQ	0
#define SIG_PCE_SCSI_DRQ	1
#define SIG_PCE_SCSI_BSY	2
#define SIG_PCE_CDDA_DONE	3
#define SIG_PCE_ADPCM_VCLK	4
#endif

#define VDC_WPF		684	/* width of a line in frame including blanking areas */
#define VDC_LPF		262	/* number of lines in a single frame */
#ifdef SUPPORT_CDROM
#define ADPCM_CLOCK	9216000
#endif

class HUC6280;
#ifdef SUPPORT_CDROM
class MSM5205;
class SCSI_HOST;
class SCSI_CDROM;
#endif

typedef struct vdc_s {
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
	pair32_t vdc_data[32];
	int status;
	int y_scroll;
} vdc_t;

typedef struct vce_s {
	uint8_t vce_control;		/* VCE control register */
	pair32_t vce_address;		/* Current address in the palette */
	pair32_t vce_data[512];		/* Palette data */
	int current_bitmap_line;	/* The current line in the display we are on */
	//bitmap_ind16 *bmp;
	scrntype_t bmp[VDC_LPF][VDC_WPF];
	scrntype_t palette[1024];
} vce_t;

typedef struct vpc_s {
	struct {
		UINT8 prio;
		UINT8 vdc0_enabled;
		UINT8 vdc1_enabled;
	} vpc_prio[4];
	UINT8	prio_map[512];		/* Pre-calculated priority map */
	pair32_t	priority;	/* Priority settings registers */
	pair32_t	window1;	/* Window 1 setting */
	pair32_t	window2;	/* Window 2 setting */
	UINT8	vdc_select;		/* Which VDC do the ST0, ST1, and ST2 instructions write to */
} vpc_t;

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

class PCE : public DEVICE
{
private:
	HUC6280* d_cpu;
#ifdef SUPPORT_CDROM
	MSM5205* d_msm;
	SCSI_HOST* d_scsi_host;
	SCSI_CDROM* d_scsi_cdrom;
#endif
	
	bool support_6btn_pad;
	bool support_multi_tap;
#ifdef SUPPORT_SUPER_GFX
	bool support_sgfx;
#endif
#ifdef SUPPORT_CDROM
	bool support_cdrom;
#endif
	
	// memory
	uint8_t cart[0x400000];	// max 4mb
#ifdef SUPPORT_SUPER_GFX
	uint8_t ram[0x8000];	// ram 32kb
#else
	uint8_t ram[0x2000];	// ram 8kb
#endif
#ifdef SUPPORT_BACKUP_RAM
	uint8_t backup[0x2000];
	uint32_t backup_crc32;
#endif
	uint32_t bank;
	uint8_t buffer;
	int prev_width;
	bool inserted;
	
	// vdc
	vdc_t vdc[2];
	vce_t vce;
	vpc_t vpc;
	
	void pce_interrupt();
#ifdef SUPPORT_SUPER_GFX
	void sgx_interrupt();
#endif
	void vdc_reset();
	void vdc_advance_line(int which);
	void draw_black_line(int line);
	void draw_overscan_line(int line);
#ifdef SUPPORT_SUPER_GFX
	void draw_sgx_overscan_line(int line);
#endif
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
#ifdef SUPPORT_SUPER_GFX
	void sgx_vdc_w(uint16_t offset, uint8_t data);
	uint8_t sgx_vdc_r(uint16_t offset);
#endif
	
	// psg
	psg_t psg[8];
	uint8_t psg_ch, psg_vol, psg_lfo_freq, psg_lfo_ctrl;
	int sample_rate;
	int volume_l, volume_r;
	void psg_reset();
	void psg_write(uint16_t addr, uint8_t data);
	uint8_t psg_read(uint16_t addr);
	
	// joypad
	const uint32_t *joy_stat;
	uint8_t joy_counter;
	bool joy_high_nibble, joy_second_byte;
	
	void joy_reset();
	void joy_write(uint16_t addr, uint8_t data);
	uint8_t joy_read(uint16_t addr);
	uint8_t joy_2btn_pad_r(uint8_t index);
	uint8_t joy_6btn_pad_r(uint8_t index);
	
#ifdef SUPPORT_CDROM
	// cd-rom
	uint8_t cdrom_ram[0x40000];
	uint8_t cdrom_regs[16];
	bool backup_locked;
	bool irq_status, drq_status;
	
	void cdrom_initialize();
	void cdrom_reset();
	void cdrom_write(uint16_t addr, uint8_t data);
	uint8_t cdrom_read(uint16_t addr);
	void write_cdrom_data(uint8_t data);
	uint8_t read_cdrom_data();
	void set_ack();
	void clear_ack();
	void set_cdrom_irq_line(int num, int state);
	
	uint8_t adpcm_ram[0x10000];
	int adpcm_read_ptr, adpcm_write_ptr, adpcm_written;
	int adpcm_length, adpcm_clock_divider;
	uint8_t adpcm_read_buf, adpcm_write_buf;
	bool adpcm_dma_enabled;
	int msm_start_addr, msm_end_addr, msm_half_addr;
	uint8_t msm_nibble, msm_idle;
	
	void reset_adpcm();
	void write_adpcm_ram(uint8_t data);
	uint8_t read_adpcm_ram();
	void adpcm_do_dma();
	void adpcm_play();
	void adpcm_stop();
	
	double cdda_volume, adpcm_volume;
	int event_cdda_fader, event_adpcm_fader;
	
	void cdda_fade_in(int time);
	void cdda_fade_out(int time);
	void adpcm_fade_in(int time);
	void adpcm_fade_out(int time);
#endif
	
public:
	PCE(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		volume_l = volume_r = 1024;
		set_device_name(_T("PC-Engine Core"));
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
#ifdef SUPPORT_CDROM
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
#endif
	void mix(int32_t* buffer, int cnt);
	void set_volume(int ch, int decibel_l, int decibel_r);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(HUC6280* device)
	{
		d_cpu = device;
	}
#ifdef SUPPORT_CDROM
	void set_context_adpcm(MSM5205* device)
	{
		d_msm = device;
	}
	void set_context_scsi_host(SCSI_HOST* device)
	{
		d_scsi_host = device;
	}
	void set_context_scsi_cdrom(SCSI_CDROM* device)
	{
		d_scsi_cdrom = device;
	}
#endif
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
