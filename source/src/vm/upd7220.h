/*
	Skelton for retropc emulator

	Origin : Neko Project 2
	Author : Takeda.Toshiya
	Date   : 2006.12.06 -

	[ uPD7220 ]
*/

#ifndef _UPD7220_H_
#define _UPD7220_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define MODE_MIX	((sync[0] & 0x22) == 0x00)
#define MODE_GFX	((sync[0] & 0x22) == 0x02)
#define MODE_CHR	((sync[0] & 0x22) == 0x20)

#define RT_MULBIT	15
#define RT_TABLEBIT	12
#define RT_TABLEMAX	(1 << RT_TABLEBIT)

class FIFO;

class UPD7220 : public DEVICE
{
private:
	// output signals
	outputs_t outputs_drq;
	outputs_t outputs_vsync;
	
	// vram
	uint8* vram;
	uint32 vram_size;
	
	// regs
	int cmdreg;
	uint8 statreg;
	
	// params
	uint8 sync[16];
	int vtotal, vs, v1, v2;
	int hc, h1, h2;
	bool sync_changed;
	bool master;
	uint8 zoom, zr, zw;
	uint8 ra[16];
	uint8 cs[3];
	uint8 pitch;
	uint32 lad;
	uint8 vect[11];
	int ead, dad;
	uint8 maskl, maskh;
	uint8 mod;
	bool hblank, vsync, start;
	int blink_cursor;
	int blink_attr;
	int blink_rate;
	bool low_high;
	bool cmd_write_done;
	
	int cpu_clocks;
#ifdef UPD7220_HORIZ_FREQ
	int horiz_freq, next_horiz_freq;
#endif
	double frames_per_sec;
	int lines_per_frame;
	
	// fifo buffers
	uint8 params[16];
	int params_count;
	FIFO *fo;
	
	// draw
	int rt[RT_TABLEMAX + 1];
	int dx, dy;	// from ead, dad
	int dir, dif, sl, dc, d, d2, d1, dm;
	uint16 pattern;
	
	// command
	void check_cmd();
	void process_cmd();
	
	void cmd_reset();
	void cmd_sync();
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
	
	void cmd_write_sub(uint32 addr, uint8 data);
	void write_vram(uint32 addr, uint8 data);
	uint8 read_vram(uint32 addr);
	void update_vect();
	void reset_vect();
	
	void draw_vectl();
	void draw_vectt();
	void draw_vectc();
	void draw_vectr();
	void draw_text();
	void draw_pset(int x, int y);
	
public:
	UPD7220(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs_drq);
		init_output_signals(&outputs_vsync);
		vram = NULL;
		vram_size = 0;
	}
	~UPD7220() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_dma_io8(uint32 addr, uint32 data);
	uint32 read_dma_io8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_pre_frame();
	void event_frame();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void update_timing(int new_clocks, double new_frames_per_sec, int new_lines_per_frame);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_drq(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_drq, device, id, mask);
	}
	void set_context_vsync(DEVICE* device, int id, uint32 mask)
	{
		register_output_signal(&outputs_vsync, device, id, mask);
	}
	void set_vram_ptr(uint8* ptr, uint32 size)
	{
		vram = ptr; vram_size = size;
	}
#ifdef UPD7220_HORIZ_FREQ
	void set_horiz_freq(int freq)
	{
		next_horiz_freq = freq;
	}
#endif
	uint8* get_sync()
	{
		return sync;
	}
	uint8* get_zoom()
	{
		return &zoom;
	}
	uint8* get_ra()
	{
		return ra;
	}
	uint8* get_cs()
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
	uint32 cursor_addr(uint32 mask);
	int cursor_top();
	int cursor_bottom();
	bool attr_blink()
	{
		return (blink_attr < (blink_rate * 3 / 4));
	}
};

#endif

