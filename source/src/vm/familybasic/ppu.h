/*
	Nintendo Family BASIC Emulator 'eFamilyBASIC'

	Origin : nester
	Author : Takeda.Toshiya
	Date   : 2010.08.11-

	[ PPU ]
*/

#ifndef _PPU_H_
#define _PPU_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class PPU : public DEVICE
{
private:
	DEVICE *d_cpu;
	
	scrntype palette_pc[64];
	uint8 screen[240][256 + 16];	// 2*8 = side margin
	uint8 solid_buf[512];
	
	uint8* banks[16];
	uint8 header[16];
	uint8 chr_rom[0x2000];
	uint8 name_tables[0x1000];
	uint8 spr_ram[0x100];
	uint8 bg_pal[0x10];
	uint8 spr_pal[0x10];
	uint8 spr_ram_rw_ptr;
	
	uint8 regs[8];
	uint16 bg_pattern_table_addr;
	uint16 spr_pattern_table_addr;
	uint16 ppu_addr_inc;
	uint8 rgb_bak;
	bool toggle_2005_2006;
	uint8 read_2007_buffer;
	
	uint16 loopy_v;
	uint16 loopy_t;
	uint8 loopy_x;
	
	void render_scanline(int v);
	void render_bg(int v);
	void render_spr(int v);
	void update_palette();
	
public:
	PPU(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~PPU() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void event_vline(int v, int clock);
	
	// unique function
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	uint8 *get_spr_ram()
	{
		return spr_ram;
	}
	void load_rom_image(_TCHAR *file_name);
	void draw_screen();
};

#endif
