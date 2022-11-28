/*
	Nintendo Family BASIC Emulator 'eFamilyBASIC'

	Origin : nester
	Author : Takeda.Toshiya
	Date   : 2010.08.11-

	[ PPU ]
*/

#include "ppu.h"
#include "memory.h"

static const uint8_t palette[64][3] = {
	{0x75, 0x75, 0x75}, {0x27, 0x1b, 0x8f}, {0x00, 0x00, 0xab}, {0x47, 0x00, 0x9f},
	{0x8f, 0x00, 0x77}, {0xab, 0x00, 0x13}, {0xa7, 0x00, 0x00}, {0x7f, 0x0b, 0x00},
	{0x43, 0x2f, 0x00}, {0x00, 0x47, 0x00}, {0x00, 0x51, 0x00}, {0x00, 0x3f, 0x17},
	{0x1b, 0x3f, 0x5f}, {0x00, 0x00, 0x00}, {0x05, 0x05, 0x05}, {0x05, 0x05, 0x05},
	{0xbc, 0xbc, 0xbc}, {0x00, 0x73, 0xef}, {0x23, 0x3b, 0xef}, {0x83, 0x00, 0xf3},
	{0xbf, 0x00, 0xbf}, {0xe7, 0x00, 0x5b}, {0xdb, 0x2b, 0x00}, {0xcb, 0x4f, 0x0f},
	{0x8b, 0x73, 0x00}, {0x00, 0x97, 0x00}, {0x00, 0xab, 0x00}, {0x00, 0x93, 0x3b},
	{0x00, 0x83, 0x8b}, {0x11, 0x11, 0x11}, {0x09, 0x09, 0x09}, {0x09, 0x09, 0x09},
	{0xff, 0xff, 0xff}, {0x3f, 0xbf, 0xff}, {0x5f, 0x97, 0xff}, {0xa7, 0x8b, 0xfd},
	{0xf7, 0x7b, 0xff}, {0xff, 0x77, 0xb7}, {0xff, 0x77, 0x63}, {0xff, 0x9b, 0x3b},
	{0xf3, 0xbf, 0x3f}, {0x83, 0xd3, 0x13}, {0x4f, 0xdf, 0x4b}, {0x58, 0xf8, 0x98},
	{0x00, 0xeb, 0xdb}, {0x66, 0x66, 0x66}, {0x0d, 0x0d, 0x0d}, {0x0d, 0x0d, 0x0d},
	{0xff, 0xff, 0xff}, {0xab, 0xe7, 0xff}, {0xc7, 0xd7, 0xff}, {0xd7, 0xcb, 0xff},
	{0xff, 0xc7, 0xff}, {0xff, 0xc7, 0xdb}, {0xff, 0xbf, 0xb3}, {0xff, 0xdb, 0xab},
	{0xff, 0xe7, 0xa3}, {0xe3, 0xff, 0xa3}, {0xab, 0xf3, 0xbf}, {0xb3, 0xff, 0xcf},
	{0x9f, 0xff, 0xf3}, {0xdd, 0xdd, 0xdd}, {0x11, 0x11, 0x11}, {0x11, 0x11, 0x11}
};

#define VRAM(addr)	bank_ptr[((addr) >> 10) & 0x0f][(addr) & 0x3ff]

#define NMI_enabled()	(regs[0] & 0x80)
#define sprites_8x16()	(regs[0] & 0x20)
//#define spr_enabled()	(regs[1] & 0x10)
//#define bg_enabled()	(regs[1] & 0x08)
#define spr_clip()	(!(regs[1] & 0x04))
#define bg_clip()	(!(regs[1] & 0x02))
#define monochrome()	(regs[1] & 0x01)
#define rgb_pal()	(regs[1] & 0xe0)
#define sprite0_hit()	(regs[2] & 0x40)

#define LOOPY_SCANLINE_START(v, t) { \
	v = (v & 0xfbe0) | (t & 0x041f); \
}

#define LOOPY_NEXT_LINE(v) { \
	if((v & 0x7000) == 0x7000) { \
		v &= 0x8fff; \
		if((v & 0x3e0) == 0x3a0) { \
			v ^= 0x0800; \
			v &= 0xfc1f; \
		} else { \
			if((v & 0x3e0) == 0x3e0) { \
				v &= 0xfc1f; \
			} else { \
				v += 0x20; \
			} \
		} \
	} else { \
		v += 0x1000; \
	} \
}

#define LOOPY_NEXT_TILE(v) { \
	if((v & 0x1f) == 0x1f) { \
		v ^= 0x0400; \
		v &= 0xffe0; \
	} else { \
		v++; \
	} \
}

#define LOOPY_NEXT_PIXEL(v, x) { \
	if(x == 7) { \
		LOOPY_NEXT_TILE(v); \
		x = 0; \
	} else { \
		x++; \
	} \
}

void PPU::initialize()
{
	chr_rom = NULL;
	
	// register event
	register_vline_event(this);
}

void PPU::release()
{
	if(chr_rom != NULL) {
		free(chr_rom);
	}
}

void PPU::load_rom_image(const _TCHAR *file_name)
{
	FILEIO* fio = new FILEIO();
	
	if(chr_rom != NULL) {
		free(chr_rom);
	}
	chr_rom_size = 0;
	
	memset(&header, 0x00, sizeof(header));
	
	if(!fio->Fopen(create_local_path(file_name), FILEIO_READ_BINARY)) {
		// for compatibility
		fio->Fopen(create_local_path(_T("BASIC.NES")), FILEIO_READ_BINARY);
	}
	if(fio->IsOpened()) {
		// read header
		fio->Fread(&header, sizeof(header), 1);
		// skip program rom
		fio->Fseek(0x2000 * header.num_8k_rom_banks(), FILEIO_SEEK_CUR);
		// read chr rom
		if((chr_rom_size = 8192 * header.num_8k_vrom_banks) != 0) {
			for(uint32_t bit = 0x40000; bit != 0; bit >>= 1) {
				if(chr_rom_size & bit) {
					if(chr_rom_size & (bit - 1)) {
						chr_rom_size = (chr_rom_size | (bit - 1)) + 1;
					}
					break;
				}
			}
			chr_rom = (uint8_t *)calloc(chr_rom_size, 1);
			fio->Fread(chr_rom, 8192 * header.num_8k_vrom_banks, 1);
		}
		fio->Fclose();
	}
	delete fio;
	
	if(chr_rom_size == 0) {
		chr_rom_size = 8192;
		chr_rom = (uint8_t *)calloc(chr_rom_size, 1);
	}
	chr_rom_mask = (chr_rom_size / 0x400) - 1;
}

void PPU::reset()
{
	// set up PPU memory space table
	for(int i = 0; i < 8; i++) {
		set_ppu_bank(i, i);
	}
	
	// set mirroring
	if(header.flags_1 & 8) {
		set_mirroring(MIRROR_4SCREEN);
	} else if(header.flags_1 & 1) {
		set_mirroring(MIRROR_VERT);
	} else {
		set_mirroring(MIRROR_HORIZ);
	}
	
	memset(bg_pal, 0, sizeof(bg_pal));
	memset(spr_pal, 0, sizeof(spr_pal));
	memset(solid_buf, 0, sizeof(solid_buf));
	memset(name_tables, 0, sizeof(name_tables));
	
	memset(spr_ram, 0, sizeof(spr_ram));
	spr_ram_rw_ptr = 0;
	
	memset(regs, 0, sizeof(regs));
	bg_pattern_table_addr = 0;
	spr_pattern_table_addr = 0;
	ppu_addr_inc = 0;
	rgb_bak = 0;
	toggle_2005_2006 = false;
	read_2007_buffer = 0;
	
	loopy_v = 0;
	loopy_t = 0;
	loopy_x = 0;
	
	// reset emphasised palette
	update_palette();
}

void PPU::write_data8(uint32_t addr, uint32_t data)
{
	uint16_t ofs;
	
	regs[addr & 7] = data;
	
	switch(addr & 0xe007) {
	case 0x2000:
		bg_pattern_table_addr = (data & 0x10) ? 0x1000 : 0;
		spr_pattern_table_addr = (data & 0x08) ? 0x1000 : 0;
		ppu_addr_inc = (data & 0x04) ? 32 : 1;
		loopy_t = (loopy_t & 0xf3ff) | (((uint16_t)(data & 0x03)) << 10);
		break;
	case 0x2001:
		if(rgb_bak != (data & 0xe0)) {
			update_palette();
			rgb_bak = data & 0xe0;
		}
		break;
	case 0x2003:
		spr_ram_rw_ptr = data;
		break;
	case 0x2004:
		spr_ram[spr_ram_rw_ptr++] = data;
		break;
	case 0x2005:
		toggle_2005_2006 = !toggle_2005_2006;
		if(toggle_2005_2006) {
			// first write
			loopy_t = (loopy_t & 0xffe0) | (((uint16_t)(data & 0xf8)) >> 3);
			loopy_x = data & 0x07;
		} else {
			// second write
			loopy_t = (loopy_t & 0xfc1f) | (((uint16_t)(data & 0xf8)) << 2);
			loopy_t = (loopy_t & 0x8fff) | (((uint16_t)(data & 0x07)) << 12);
		}
		break;
	case 0x2006:
		toggle_2005_2006 = !toggle_2005_2006;
		if(toggle_2005_2006) {
			// first write
			loopy_t = (loopy_t & 0x00ff) | (((uint16_t)(data & 0x3f)) << 8);
		} else {
			// second write
			loopy_t = (loopy_t & 0xff00) | ((uint16_t)data);
			loopy_v = loopy_t;
		}
		break;
	case 0x2007:
		ofs = loopy_v & 0x3fff;
		loopy_v += ppu_addr_inc;
		if(ofs >= 0x3000) {
			// is it a palette entry?
			if(ofs >= 0x3f00) {
				data &= 0x3f;
				if(!(ofs & 0x000f)) {
					bg_pal[0] = spr_pal[0] = data;
				} else if(!(ofs & 0x10)) {
					bg_pal[ofs & 0x000f] = data;
				} else {
					spr_pal[ofs & 0x000f] = data;
				}
				break;
			}
			// handle mirroring
			ofs &= 0xefff;
		}
		if(ofs >= 0x2000 || header.num_8k_vrom_banks == 0) {
			VRAM(ofs) = data;
		}
		break;
	}
}

uint32_t PPU::read_data8(uint32_t addr)
{
	uint16_t ofs;
	uint8_t val;
	
	switch(addr & 0xe007) {
	case 0x2002:
		// clear toggle
		toggle_2005_2006 = false;
		val = regs[2];
		// clear v-blank flag
		regs[2] &= ~0x80;
		return val;
	case 0x2007:
		ofs = loopy_v & 0x3fff;
		loopy_v += ppu_addr_inc;
		if(ofs >= 0x3000) {
			// is it a palette entry?
			if(ofs >= 0x3f00) {
				if(!(ofs & 0x0010)) {
					return bg_pal[ofs & 0x000f];
				} else {
					return spr_pal[ofs & 0x000f];
				}
			}
			// handle mirroring
			ofs &= 0xefff;
		}
		val = read_2007_buffer;
		read_2007_buffer = VRAM(ofs);
		return val;
	}
	return regs[addr & 7];
}

void PPU::event_vline(int v, int clock)
{
	// 525 -> 262.5
	if(v & 1) {
		return;
	}
	v >>= 1;
	
	switch(v) {
	case 0:
		if(spr_enabled() || bg_enabled()) {
			loopy_v = loopy_t;
		}
		break;
	case 241:
		// set vblank register flag
		regs[2] |= 0x80;
		if(NMI_enabled()) {
			d_cpu->write_signal(SIG_CPU_NMI, 1, 1);
		}
		break;
	case 261:
		// reset vblank register flag and sprite0 hit flag1
		regs[2] &= 0x3F;
		break;
	}
	if(v < 240) {
		render_scanline(v);
	}
}

void PPU::draw_screen()
{
	if(emu->now_waiting_in_debugger) {
		// store regs
		uint16_t tmp_loopy_v = loopy_v;
		uint8_t tmp_status = regs[2];
		
		// drive vlines
		if(spr_enabled() || bg_enabled()) {
			loopy_v = loopy_t;
		}
		for(int v = 0; v < 240; v++) {
			render_scanline(v);
		}
		
		// restore regs
		loopy_v = tmp_loopy_v;
		regs[2] = tmp_status;
	}
	for(int y = 0; y < 240; y++) {
		scrntype_t* dest = emu->get_screen_buffer(y);
		uint8_t* src = screen[y];
		
		for(int x = 0; x < 256; x++) {
			dest[x] = palette_pc[src[x + 8] & 0x3f];
		}
	}
}

void PPU::update_palette()
{
	for(int i = 0; i < 64; i++) {
		uint8_t r = palette[i][0];
		uint8_t g = palette[i][1];
		uint8_t b = palette[i][2];
		
		switch(rgb_pal()) {
		case 0x20:
			g = (uint8_t)(g * 0.80);
			b = (uint8_t)(b * 0.73);
			break;
		case 0x40:
			r = (uint8_t)(r * 0.73);
			b = (uint8_t)(b * 0.70);
			break;
		case 0x60:
			r = (uint8_t)(r * 0.76);
			g = (uint8_t)(g * 0.78);
			b = (uint8_t)(b * 0.58);
			break;
		case 0x80:
			r = (uint8_t)(r * 0.86);
			g = (uint8_t)(g * 0.80);
			break;
		case 0xa0:
			r = (uint8_t)(r * 0.83);
			g = (uint8_t)(g * 0.68);
			b = (uint8_t)(b * 0.85);
			break;
		case 0xc0:
			r = (uint8_t)(r * 0.67);
			g = (uint8_t)(g * 0.77);
			b = (uint8_t)(b * 0.83);
			break;
		case 0xe0:
			r = (uint8_t)(r * 0.68);
			g = (uint8_t)(g * 0.68);
			b = (uint8_t)(b * 0.68);
			break;
		}
		palette_pc[i] = RGB_COLOR(r, g, b);
	}
}

void PPU::render_scanline(int v)
{
	uint8_t* buf = screen[v];
	
	if(!bg_enabled()) {
		// set to background color
		memset(screen[v], bg_pal[0], 256 + 16);
	}
	if(spr_enabled() || bg_enabled()) {
		LOOPY_SCANLINE_START(loopy_v, loopy_t);
		if(bg_enabled()) {
			render_bg(v);
		} else {
			memset(solid_buf, 0, sizeof(solid_buf));
		}
		if(spr_enabled()) {
			// draw sprites
			render_spr(v);
		}
		LOOPY_NEXT_LINE(loopy_v);
	}
}

#define BG_WRITTEN_FLAG 1
#define SPR_WRITTEN_FLAG 2

#define DRAW_BG_PIXEL() \
	col = attrib_bits; \
	if(pattern_lo & pattern_mask) { \
		col |= 1; \
	} \
	if(pattern_hi & pattern_mask) { \
		col |= 2; \
	} \
	*p++ = monochrome() ? (bg_pal[col] & 0xf0) : bg_pal[col]; \
	*solid++ = (col & 3) ? BG_WRITTEN_FLAG : 0; \

void PPU::render_bg(int v)
{
	uint32_t tile_x = (loopy_v & 0x001f);
	uint32_t tile_y = (loopy_v & 0x03e0) >> 5;
	uint32_t name_addr = 0x2000 + (loopy_v & 0x0fff);
	uint32_t attrib_addr = 0x2000 + (loopy_v & 0x0c00) + 0x03c0 + ((tile_y & 0xfffc) << 1) + (tile_x >> 2);
	uint8_t attrib_bits;
	
	if(!(tile_y & 2)) {
		if(!(tile_x & 2)) {
			attrib_bits = (VRAM(attrib_addr) & 0x03) << 2;
		} else {
			attrib_bits = (VRAM(attrib_addr) & 0x0C);
		}
	} else {
		if(!(tile_x & 2)) {
			attrib_bits = (VRAM(attrib_addr) & 0x30) >> 2;
		} else {
			attrib_bits = (VRAM(attrib_addr) & 0xC0) >> 4;
		}
	}
	uint8_t *p = screen[v] + (8 - loopy_x);
	uint8_t *solid = solid_buf + (8 - loopy_x);
	
	for(int i = 33; i; i--) {
		if(header.mapper() == 5) {
			uint8_t mmc5_pal = d_memory->mmc5_ppu_latch_render(1, name_addr & 0x03ff);
			if(mmc5_pal != 0) {
				attrib_bits = mmc5_pal & 0x0c;
			}
		}
		try {
			uint32_t pattern_addr = bg_pattern_table_addr + ((int32_t)VRAM(name_addr) << 4) + ((loopy_v & 0x7000) >> 12);
			uint8_t pattern_lo = VRAM(pattern_addr);
			uint8_t pattern_hi = VRAM(pattern_addr + 8);
			uint8_t pattern_mask = 0x80;
			uint8_t col;
			
			DRAW_BG_PIXEL();
			pattern_mask >>= 1;
			DRAW_BG_PIXEL();
			pattern_mask >>= 1;
			DRAW_BG_PIXEL();
			pattern_mask >>= 1;
			DRAW_BG_PIXEL();
			pattern_mask >>= 1;
			DRAW_BG_PIXEL();
			pattern_mask >>= 1;
			DRAW_BG_PIXEL();
			pattern_mask >>= 1;
			DRAW_BG_PIXEL();
			pattern_mask >>= 1;
			DRAW_BG_PIXEL();
			
			tile_x++;
			name_addr++;
			
			if(!(tile_x & 1)) {
				if(!(tile_x & 3)) {
					if(!(tile_x & 0x1f)) {
						name_addr ^= 0x0400; // switch name tables
						attrib_addr ^= 0x0400;
						name_addr -= 0x0020;
						attrib_addr -= 0x0008;
						tile_x -= 0x0020;
					}
					attrib_addr++;
				}
				if(!(tile_y & 2)) {
					if(!(tile_x & 2)) {
						attrib_bits = (VRAM(attrib_addr) & 0x03) << 2;
					} else {
						attrib_bits = (VRAM(attrib_addr) & 0x0c);
					}
				} else {
					if(!(tile_x & 2)) {
						attrib_bits = (VRAM(attrib_addr) & 0x30) >> 2;
					} else {
						attrib_bits = (VRAM(attrib_addr) & 0xc0) >> 4;
					}
				}
			}
		} catch(...) {
			// do nothing
		}
	}
	if(bg_clip()) {
		memset(&screen[v][8], bg_pal[0], 8);
		memset(solid + 8, 0, 8);
	}
}

void PPU::render_spr(int v)
{
	int num_sprites = 0;
	int spr_height = sprites_8x16() ? 16 : 8;
	
	if(header.mapper() == 5) {
		d_memory->mmc5_ppu_latch_render(0, 0);
	}
	for(int s = 0; s < 64; s++) {
		uint8_t* spr = &spr_ram[s << 2];
		int spr_y = spr[0] + 1;
		
		if(spr_y > v || (spr_y + spr_height) <= v) {
			continue;
		}
		num_sprites++;
		if(num_sprites > 8) {
			break;
		}
		int spr_x = spr[3];
		int start_x = 0;
		int end_x = 8;
		int inc_x = 1;
		
		if((spr_x + 7) > 255) {
			end_x -= ((spr_x + 7) - 255);
		}
		if((spr_x < 8) && (spr_clip())) {
			if(!spr_x) {
				continue;
			}
			start_x += (8 - spr_x);
		}
		int y = v - spr_y;
		
		uint8_t *p = &screen[v][8 + spr_x + start_x];
		uint8_t *solid = &solid_buf[8 + spr_x + start_x];
		
		if(spr[2] & 0x40) {
			start_x = (8 - 1) - start_x;
			end_x = (8 - 1) - end_x;
			inc_x = -1;
		}
		if(spr[2] & 0x80) {
			y = (spr_height - 1) - y;
		}
		uint8_t priority = spr[2] & 0x20;
		
		for(int x = start_x; x != end_x; x += inc_x) {
			uint8_t col = 0;
			uint32_t tile_addr;
			uint8_t tile_mask;
			
			if(!((*solid) & SPR_WRITTEN_FLAG)) {
				if(sprites_8x16()) {
					tile_addr = spr[1] << 4;
					if(spr[1] & 0x01) {
						tile_addr += 0x1000;
						if(y < 8) {
							tile_addr -= 16;
						}
					} else {
						if(y >= 8) {
							tile_addr += 16;
						}
					}
					tile_addr += y & 0x07;
					tile_mask = (0x80 >> (x & 0x07));
				} else {
					tile_addr = spr[1] << 4;
					tile_addr += y & 0x07;
					tile_addr += spr_pattern_table_addr;
					tile_mask = (0x80 >> (x & 0x07));
				}
				if(VRAM(tile_addr) & tile_mask) {
					col |= 1;
				}
				tile_addr += 8;
				if(VRAM(tile_addr) & tile_mask) {
					col |= 2;
				}
				if(spr[2] & 2) {
					col |= 8;
				}
				if(spr[2] & 1) {
					col |= 4;
				}
				if(col & 3) {
					if(s && (*solid & BG_WRITTEN_FLAG)) {
						regs[2] |= 0x40;
					}
					if(priority) {
						*solid |= SPR_WRITTEN_FLAG;
						if(!(*solid & BG_WRITTEN_FLAG)) {
							*p = monochrome() ? (spr_pal[col] & 0xf0) : spr_pal[col];
						}
					} else {
						if(!(*solid & SPR_WRITTEN_FLAG)) {
							*p = monochrome() ? (spr_pal[col] & 0xf0) : spr_pal[col];
							*solid |= SPR_WRITTEN_FLAG;
						}
					}
				}
			}
			p++;
			solid++;
		}
	}
	if(num_sprites >= 8) {
		regs[2] |= 0x20;
	} else {
		regs[2] &= ~0x20;
	}
}

void PPU::set_ppu_bank(uint8_t bank, uint32_t bank_num)
{
	if(bank < 8) {
		bank_ptr[bank] = chr_rom + 0x400 * (bank_num & chr_rom_mask);
	} else if(bank < 12) {
		bank_ptr[bank] = bank_ptr[bank + 4] = name_tables + 0x400 * (bank_num & 0x03);
	}
	banks[bank] = bank_num;
}

void PPU::set_mirroring(int mirror)
{
	switch(mirror) {
	case MIRROR_HORIZ:
		// horizontal mirroring
		set_mirroring(0, 0, 1, 1);
		break;
	case MIRROR_VERT:
		// vertical mirroring
		set_mirroring(0, 1, 0, 1);
		break;
	case MIRROR_4SCREEN:
		// 4 screen mirroring
		set_mirroring(0, 1, 2, 3);
		break;
	}
}

void PPU::set_mirroring(uint32_t nt0, uint32_t nt1, uint32_t nt2, uint32_t nt3)
{
	set_ppu_bank( 8, nt0);
	set_ppu_bank( 9, nt1);
	set_ppu_bank(10, nt2);
	set_ppu_bank(11, nt3);
}

#define STATE_VERSION	3

bool PPU::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(palette_pc, sizeof(palette_pc), 1);
	state_fio->StateArray(solid_buf, sizeof(solid_buf), 1);
	state_fio->StateArray(header.id, sizeof(header.id), 1);
	state_fio->StateValue(header.ctrl_z);
	state_fio->StateValue(header.dummy);
	state_fio->StateValue(header.num_8k_vrom_banks);
	state_fio->StateValue(header.flags_1);
	state_fio->StateValue(header.flags_2);
	state_fio->StateArray(header.reserved, sizeof(header.reserved), 1);
	state_fio->StateArray(banks, sizeof(banks), 1);
	state_fio->StateValue(chr_rom_size);
//	state_fio->StateValue(chr_rom_mask);
	if(loading) {
		chr_rom_mask = (chr_rom_size / 0x400) - 1;
		if(chr_rom != NULL) {
			free(chr_rom);
		}
		chr_rom = (uint8_t *)malloc(chr_rom_size);
	}
	state_fio->StateArray(chr_rom, chr_rom_size, 1);
	state_fio->StateArray(name_tables, sizeof(name_tables), 1);
	state_fio->StateArray(spr_ram, sizeof(spr_ram), 1);
	state_fio->StateArray(bg_pal, sizeof(bg_pal), 1);
	state_fio->StateArray(spr_pal, sizeof(spr_pal), 1);
	state_fio->StateValue(spr_ram_rw_ptr);
	state_fio->StateArray(regs, sizeof(regs), 1);
	state_fio->StateValue(bg_pattern_table_addr);
	state_fio->StateValue(spr_pattern_table_addr);
	state_fio->StateValue(ppu_addr_inc);
	state_fio->StateValue(rgb_bak);
	state_fio->StateValue(toggle_2005_2006);
	state_fio->StateValue(read_2007_buffer);
	state_fio->StateValue(loopy_v);
	state_fio->StateValue(loopy_t);
	state_fio->StateValue(loopy_x);
	
	// post process
	if(loading) {
		for(int i = 0; i < 12; i++) {
			set_ppu_bank(i, banks[i]);
		}
	}
	return true;
}

