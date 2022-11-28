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

#define read_dword(address) *(uint32_t *)address
#define write_dword(address,data) *(uint32_t *)address=data
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
	
	uint16_t z80_icount;
	uint16_t vcounter;
	uint8_t vscroll;
	uint8_t status;
	uint8_t pending;
	uint16_t addr;
	uint8_t code;
	int pn, ct, pg, sa, sg;
	int ntab;
	int satb;
	int line;
	int left;
	uint16_t lpf;
	uint8_t height;
	uint8_t extended;
	uint8_t mode;
	uint8_t irq;
	uint16_t spr_col;
	uint8_t spr_ovr;
	uint8_t bd;
	uint8_t vram[0x4000];
	uint8_t screen[192][256];
	uint8_t regs[0x10], status_reg, read_ahead, first_byte;
	uint16_t vram_addr;
	bool latch, intstat;
	uint16_t color_table, pattern_table, name_table;
	uint16_t sprite_pattern, sprite_attrib;
	uint16_t color_mask, pattern_mask;
	scrntype_t palette_pc[0x20];
	void set_intstat(bool val);
	void draw_mode0();
	void draw_mode1();
	void draw_mode2();
	void draw_mode12();
	void draw_mode3();
	void draw_mode23();
	void draw_modebogus();
	void draw_sprites();
	uint8_t cram[0x40]; 
	uint16_t cram_latch;
	uint16_t hlatch;
	uint8_t addrmode;                 /* Type of VDP action */
	uint8_t buffer;
	uint8_t vdp_mode;                 /* Current mode of the VDP: 0,1,2,3,4 */
	uint8_t sms_cram_expand_table[4];
	uint8_t gg_cram_expand_table[16];
	uint8_t *linebuf;					/* Pointer to output buffer */
	uint8_t bg_name_dirty[0x200];     /* 1= This pattern is dirty */
	uint16_t bg_name_list[0x200];     /* List of modified pattern indices */
	uint16_t bg_list_index;           /* # of modified patterns in list */
	uint8_t bg_pattern_cache[0x20000];/* Cached and flipped patterns */
	uint8_t lut[0x10000];				/* Pixel look-up table */
	uint32_t bp_lut[0x10000];			/* Bitplane to packed pixel LUT */
	int vp_x;
	int vp_y;
	int vp_w;
	int vp_h;
	int console;
public:
	_315_5124(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs_irq);
		set_device_name(_T("315-5124"));
	}
	~_315_5124() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
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
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		initialize_output_signals(&outputs_irq);
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void draw_screen();
	void palette_sync(int index, int force);
	void render_bg(int line);
	void render_obj(int line);
	void update_bg_pattern_cache(void);
	void viewport_check(void);
	void vdp_reg_w(uint8_t r, uint8_t d);
	void set_console(int gg)
	{
		console = gg;
	}
};
#endif
