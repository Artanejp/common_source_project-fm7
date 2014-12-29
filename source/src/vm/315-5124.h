/*
	Skelton for retropc emulator

	Origin : SMS Plus
	Author : tanam
	Date   : 2013.09.14 -

	[ 315-5124 ]
*/

#ifndef _315_5124_H_
#define _315_5124_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define read_dword(address) *(uint32 *)address
#define write_dword(address,data) *(uint32 *)address=data
#define BACKDROP_COLOR	((vdp_mode == 4 ? 0x10 : 0x00) + (regs[0x07] & 0x0f))
#define ADDR_MASK	(0x4000 - 1)

#define CYCLES_PER_LINE 228

class _315_5124 : public DEVICE
{
private:
	KEYBOARD *d_key;
	DEVICE *d_psg;
	// output signals
	outputs_t outputs_irq;
	
	uint16 z80_icount;
	uint16 vcounter;
	uint8 vscroll;
	uint8 status;
	uint8 pending;
	uint16 addr;
	uint8 code;
	int pn, ct, pg, sa, sg;
	int ntab;
	int satb;
	int line;
	int left;
	uint16 lpf;
	uint8 height;
	uint8 extended;
	uint8 mode;
	uint8 irq;
	uint16 spr_col;
	uint8 spr_ovr;
	uint8 bd;
	uint8 vram[0x4000];
	uint8 screen[192][256];
	uint8 regs[0x10], status_reg, read_ahead, first_byte;
	uint16 vram_addr;
	bool latch, intstat;
	uint16 color_table, pattern_table, name_table;
	uint16 sprite_pattern, sprite_attrib;
	uint16 color_mask, pattern_mask;
	scrntype palette_pc[0x20];
	void set_intstat(bool val);
	void draw_mode0();
	void draw_mode1();
	void draw_mode2();
	void draw_mode12();
	void draw_mode3();
	void draw_mode23();
	void draw_modebogus();
	void draw_sprites();
	uint8 cram[0x40]; 
	uint16 cram_latch;
	uint16 hlatch;
	uint8 addrmode;                 /* Type of VDP action */
	uint8 buffer;
	uint8 vdp_mode;                 /* Current mode of the VDP: 0,1,2,3,4 */
	uint8 sms_cram_expand_table[4];
	uint8 gg_cram_expand_table[16];
	uint8 *linebuf;					/* Pointer to output buffer */
	uint8 bg_name_dirty[0x200];     /* 1= This pattern is dirty */
	uint16 bg_name_list[0x200];     /* List of modified pattern indices */
	uint16 bg_list_index;           /* # of modified patterns in list */
	uint8 bg_pattern_cache[0x20000];/* Cached and flipped patterns */
	uint8 lut[0x10000];				/* Pixel look-up table */
	uint32 bp_lut[0x10000];			/* Bitplane to packed pixel LUT */
	int vp_x;
	int vp_y;
	int vp_w;
	int vp_h;
	int console;
public:
	_315_5124(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		init_output_signals(&outputs_irq);
	}
	~_315_5124() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void event_vline(int v, int clock);
	
	// unique function
	void set_context_key(KEYBOARD* device)
	{
		d_key = device;
	}
	void set_context_psg(DEVICE* device)
	{
		d_psg = device;
	}
	void set_context_irq(DEVICE* device, int id, uint32 mask)
	{
		init_output_signals(&outputs_irq);
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void draw_screen();
	void palette_sync(int index, int force);
	void render_bg(int line);
	void render_obj(int line);
	void update_bg_pattern_cache(void);
	void viewport_check(void);
	void vdp_reg_w(uint8 r, uint8 d);
	void set_console(int gg)
	{
		console = gg;
	}
};
#endif
