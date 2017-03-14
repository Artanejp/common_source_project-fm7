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
	
	scrntype_t palette_pc[64];
	uint8_t screen[240][256 + 16];	// 2*8 = side margin
	uint8_t solid_buf[512];
	
	uint8_t* banks[16];
	uint8_t header[16];
	uint8_t chr_rom[0x2000];
	uint8_t name_tables[0x1000];
	uint8_t spr_ram[0x100];
	uint8_t bg_pal[0x10];
	uint8_t spr_pal[0x10];
	uint8_t spr_ram_rw_ptr;
	
	uint8_t regs[8];
	uint16_t bg_pattern_table_addr;
	uint16_t spr_pattern_table_addr;
	uint16_t ppu_addr_inc;
	uint8_t rgb_bak;
	bool toggle_2005_2006;
	uint8_t read_2007_buffer;
	
	uint16_t loopy_v;
	uint16_t loopy_t;
	uint8_t loopy_x;
	
	void render_scanline(int v);
	void render_bg(int v);
	void render_spr(int v);
	void update_palette();
	
public:
	PPU(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("PPU"));
	}
	~PPU() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void event_vline(int v, int clock);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	uint8_t *get_spr_ram()
	{
		return spr_ram;
	}
	void load_rom_image(const _TCHAR *file_name);
	void draw_screen();
};

#endif
