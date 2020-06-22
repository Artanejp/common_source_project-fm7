/*
	Skelton for retropc emulator

	Origin : Neko Project 2
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ uPD7220 ]
*/

#ifndef _UPD7220_H_
#define _UPD7220_H_

//#include "vm.h"
//#include "../emu.h"
#include "device.h"

#define MODE_MIX	((sync[0] & 0x22) == 0x00)
#define MODE_GFX	((sync[0] & 0x22) == 0x02)
#define MODE_CHR	((sync[0] & 0x22) == 0x20)

#define RT_MULBIT	15
#define RT_TABLEBIT	12
#define RT_TABLEMAX	(1 << RT_TABLEBIT)

#define SIG_UPD7220_CLOCK_FREQ  1
#define SIG_UPD7220_EXT_VSYNC   2
#define SIG_UPD7220_WIDTH_BYTES 3
#define SIG_UPD7220_HEIGHT      4
#define SIG_UPD7220_PITCH       5
#define SIG_UPD7220_DISP_WIDTH  6
#define SIG_UPD7220_DISP_HEIGHT 7

class FIFO;

class UPD7220 : public DEVICE
{
protected:
	// output signals
	outputs_t outputs_drq;
	outputs_t outputs_vsync;
	outputs_t outputs_vblank;
	
	// vram
	DEVICE* d_vram_bus;
	uint8_t* vram;
	uint32_t vram_size;
	uint16_t vram_data_mask;
	
	// feature flags
	bool __QC10;
	bool _UPD7220_MSB_FIRST;
	bool _UPD7220_UGLY_PC98_HACK;
	int  _UPD7220_FIXED_PITCH;
	int  _UPD7220_HORIZ_FREQ;
	int  _UPD7220_A_VERSION;
	int  _LINES_PER_FRAME;
	int  _CHARS_PER_LINE;

	// regs
	int cmdreg;
	uint8_t statreg;
	
	// params
	uint8_t sync[16];
	int vtotal, vfp, vs, vbp, v1, v2, v3, v4;
	int hfp, hs, hbp, h1, h2, h3, h4;
	bool sync_changed;
	bool master;
	uint8_t zoom, zr, zw;
	uint8_t ra[16];
	uint8_t cs[3];
	uint8_t pitch;
	uint32_t lad;
	uint8_t vect[11];
	int ead, dad;
	uint8_t maskl, maskh;
	uint8_t mod;
	bool hsync, hblank;
	bool vsync, vblank;
	bool start;
	int blink_cursor;
	int blink_attr;
	int blink_rate;
	bool low_high;
	bool cmd_write_done;
	int width;
	int height;
	
	int cpu_clocks;

//#ifdef UPD7220_HORIZ_FREQ
	int horiz_freq, next_horiz_freq;
//#endif
	double frames_per_sec;
	int lines_per_frame;
	
	// waiting
	int event_cmdready;
	uint32_t wrote_bytes;
	bool cmd_drawing;
	uint32_t clock_freq;
	
	// fifo buffers
	uint8_t params[16];
	int params_count;
	FIFO *fo;
	
	// draw
	int rt[RT_TABLEMAX + 1];
	int dx, dy;	// from ead, dad
	int dir, dif, sl, dc, d, d2, d1, dm;
	uint16_t pattern;
	
	// command
	void check_cmd();
	void process_cmd();
	
	void cmd_reset();
	void __FASTCALL cmd_sync(bool flag);
	void cmd_master();
	void cmd_slave();
	void cmd_start();
	void cmd_stop();
	void cmd_zoom();
	void cmd_scroll();
	void cmd_csrform();
	void cmd_pitch();
	void cmd_lpen();
	void cmd_vectw();
	void cmd_vecte();
	void cmd_texte();
	void cmd_csrw();
	void cmd_csrr();
	void cmd_mask();
	void cmd_write();
	void cmd_read();
	void cmd_dmaw();
	void cmd_dmar();
	void cmd_unk_5a();
	
	void __FASTCALL cmd_write_sub(uint32_t addr, uint8_t data);
	void __FASTCALL write_vram(uint32_t addr, uint8_t data);
	uint8_t __FASTCALL read_vram(uint32_t addr);
	void update_vect();
	void reset_vect();
	
	void draw_vectl();
	void draw_vectt();
	void draw_vectc();
	void draw_vectr();
	void draw_text();
	void __FASTCALL draw_pset(int x, int y);
	void register_event_wait_cmd(uint32_t bytes);
	
public:
	UPD7220(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_drq);
		initialize_output_signals(&outputs_vsync);
		initialize_output_signals(&outputs_vblank);
		d_vram_bus = NULL;
		vram = NULL;
		vram_data_mask = 0xffff;
		width = 80;
		height = 25; // ToDo
		clock_freq = 2500 * 1000; // Hz
		set_device_name(_T("uPD7220 GDC"));
	}
	~UPD7220() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_dma_io8(uint32_t addr);
	void __FASTCALL write_io8(uint32_t addr, uint32_t data);
	uint32_t __FASTCALL read_io8(uint32_t addr);

	void __FASTCALL write_signal(int ch, uint32_t data, uint32_t mask);
	uint32_t __FASTCALL read_signal(int ch);
	
	void event_pre_frame();
	void event_frame();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_clock_freq(uint32_t val)
	{
		clock_freq = val;
	}
	void set_context_drq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_drq, device, id, mask);
	}
	void set_context_vsync(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_vsync, device, id, mask);
	}
	void set_vram_ptr(uint8_t* ptr, uint32_t size)
	{
		vram = ptr;
		vram_size = size;
	}
	void set_vram_ptr(uint8_t* ptr, uint32_t size, uint16_t mask)
	{
		set_vram_ptr(ptr, size);
		vram_data_mask = mask;
	}
	void set_vram_bus_ptr(DEVICE* device, uint32_t size)
	{
		d_vram_bus = device;
		vram_size = size;
	}
	void set_context_vblank(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_vblank, device, id, mask);
	}
	void set_vram_bus_ptr(DEVICE* device, uint32_t size, uint16_t mask)
	{
		set_vram_bus_ptr(device, size);
		vram_data_mask = mask;
	}
	void set_screen_width(int value)
	{
		width = value;
	}
	void set_screen_height(int val)
	{
		height = val;
	}
//#ifdef UPD7220_HORIZ_FREQ
	void set_horiz_freq(int freq)
	{
		next_horiz_freq = freq;
	}
//#endif
	uint8_t* get_sync()
	{
		return sync;
	}
	uint8_t* get_zoom()
	{
		return &zoom;
	}
	uint8_t* get_ra()
	{
		return ra;
	}
	uint8_t* get_cs()
	{
		return cs;
	}
	int* get_ead()
	{
		return &ead;
	}
	bool get_start()
	{
		return start;
	}
	uint32_t __FASTCALL cursor_addr(uint32_t mask);
	int cursor_top();
	int cursor_bottom();
	bool attr_blink()
	{
		return (blink_attr < (blink_rate * 3 / 4));
	}
};

#endif

