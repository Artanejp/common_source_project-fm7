/*
	Nintendo Family BASIC Emulator 'eFamilyBASIC'

	Origin : nester
	Author : Takeda.Toshiya
	Date   : 2010.08.11-

	[ PPU ]
*/

#include "ppu.h"
#include "../../fileio.h"

static const uint8 palette[64][3] = {
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

#define VRAM(addr)	banks[((addr) >> 10) & 0x0f][(addr) & 0x3ff]

#define NMI_enabled()	(regs[0] & 0x80)
#define sprites_8x16()	(regs[0] & 0x20)
#define spr_enabled()	(regs[1] & 0x10)
#define bg_enabled()	(regs[1] & 0x08)
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
	// register event
	register_vline_event(this);
}

void PPU::load_rom_image(_TCHAR *file_name)
{
	FILEIO* fio = new FILEIO();
	bool file_open = false;
	
	if(fio->Fopen(emu->bios_path(file_name), FILEIO_READ_BINARY)) {
		file_open = true;
	} else if(fio->Fopen(emu->bios_path(_T("BASIC.NES")), FILEIO_READ_BINARY)) {
		// for compatibility
		file_open = true;
	}
	if(file_open) {
		// read header
		fio->Fread(header, sizeof(header), 1);
		// skip program rom
		fio->Fseek(header[4] * 0x4000, FILEIO_SEEK_CUR);
		// read chr rom (max 8kb)
		fio->Fread(chr_rom, sizeof(chr_rom), 1);
		fio->Fclose();
	} else {
		memset(header, 0, sizeof(header));
		memset(chr_rom, 0xff, sizeof(chr_rom));
	}
	delete fio;
}

void PPU::reset()
{
	// set up PPU memory space table
	for(int i = 0; i < 8; i++) {
		banks[i] = chr_rom + 0x400 * i;
	}
	
	// set mirroring
#if 0
	if(header[6] & 8) {
		// 4 screen mirroring
		banks[ 8] = banks[12] = name_tables;
		banks[ 9] = banks[13] = name_tables + 0x400;
		banks[10] = banks[14] = name_tables + 0x800;
		banks[11] = banks[15] = name_tables + 0xc00;
	} else if(header[6] & 1) {
#endif
		// vertical mirroring
		banks[ 8] = banks[10] = banks[12] = banks[14] = name_tables;
		banks[ 9] = banks[11] = banks[13] = banks[15] = name_tables + 0x400;
#if 0
	} else {
		// horizontal mirroring
		banks[ 8] = banks[ 9] = banks[12] = banks[13] = name_tables;
		banks[10] = banks[11] = banks[14] = banks[15] = name_tables + 0x400;
	}
#endif
	
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

void PPU::write_data8(uint32 addr, uint32 data)
{
	uint16 ofs;
	
	regs[addr & 7] = data;
	
	switch(addr & 0xe007) {
	case 0x2000:
		bg_pattern_table_addr = (data & 0x10) ? 0x1000 : 0;
		spr_pattern_table_addr = (data & 0x08) ? 0x1000 : 0;
		ppu_addr_inc = (data & 0x04) ? 32 : 1;
		loopy_t = (loopy_t & 0xf3ff) | (((uint16)(data & 0x03)) << 10);
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
			loopy_t = (loopy_t & 0xffe0) | (((uint16)(data & 0xf8)) >> 3);
			loopy_x = data & 0x07;
		} else {
			// second write
			loopy_t = (loopy_t & 0xfc1f) | (((uint16)(data & 0xf8)) << 2);
			loopy_t = (loopy_t & 0x8fff) | (((uint16)(data & 0x07)) << 12);
		}
		break;
	case 0x2006:
		toggle_2005_2006 = !toggle_2005_2006;
		if(toggle_2005_2006) {
			// first write
			loopy_t = (loopy_t & 0x00ff) | (((uint16)(data & 0x3f)) << 8);
		} else {
			// second write
			loopy_t = (loopy_t & 0xff00) | ((uint16)data);
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
		if(ofs >= 0x2000) {
			VRAM(ofs) = data;
		}
		break;
	}
}

uint32 PPU::read_data8(uint32 addr)
{
	uint16 ofs;
	uint8 val;
	
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
	
	for(int y = 0; y < 240; y++) {
		scrntype* dest = emu->screen_buffer(y);
		uint8* src = screen[y];
		
		for(int x = 0; x < 256; x++) {
			dest[x] = palette_pc[src[x + 8] & 0x3f];
		}
	}
}

void PPU::update_palette()
{
	for(int i = 0; i < 64; i++) {
		uint8 r = palette[i][0];
		uint8 g = palette[i][1];
		uint8 b = palette[i][2];
		
		switch(rgb_pal()) {
		case 0x20:
			g = (uint8)(g * 0.80);
			b = (uint8)(b * 0.73);
			break;
		case 0x40:
			r = (uint8)(r * 0.73);
			b = (uint8)(b * 0.70);
			break;
		case 0x60:
			r = (uint8)(r * 0.76);
			g = (uint8)(g * 0.78);
			b = (uint8)(b * 0.58);
			break;
		case 0x80:
			r = (uint8)(r * 0.86);
			g = (uint8)(g * 0.80);
			break;
		case 0xa0:
			r = (uint8)(r * 0.83);
			g = (uint8)(g * 0.68);
			b = (uint8)(b * 0.85);
			break;
		case 0xc0:
			r = (uint8)(r * 0.67);
			g = (uint8)(g * 0.77);
			b = (uint8)(b * 0.83);
			break;
		case 0xe0:
			r = (uint8)(r * 0.68);
			g = (uint8)(g * 0.68);
			b = (uint8)(b * 0.68);
			break;
		}
		palette_pc[i] = RGB_COLOR(r, g, b);
	}
}

void PPU::render_scanline(int v)
{
	uint8* buf = screen[v];
	
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
	uint32 tile_x = (loopy_v & 0x001f);
	uint32 tile_y = (loopy_v & 0x03e0) >> 5;
	uint32 name_addr = 0x2000 + (loopy_v & 0x0fff);
	uint32 attrib_addr = 0x2000 + (loopy_v & 0x0c00) + 0x03c0 + ((tile_y & 0xfffc) << 1) + (tile_x >> 2);
	uint8 attrib_bits;
	
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
	uint8 *p = screen[v] + (8 - loopy_x);
	uint8 *solid = solid_buf + (8 - loopy_x);
	
	for(int i = 33; i; i--) {
		uint32 pattern_addr = bg_pattern_table_addr + ((int32)VRAM(name_addr) << 4) + ((loopy_v & 0x7000) >> 12);
		uint8 pattern_lo = VRAM(pattern_addr);
		uint8 pattern_hi = VRAM(pattern_addr + 8);
		uint8 pattern_mask = 0x80;
		uint8 col;
		
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
	
	for(int s = 0; s < 64; s++) {
		uint8* spr = &spr_ram[s << 2];
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
		
		uint8 *p = &screen[v][8 + spr_x + start_x];
		uint8 *solid = &solid_buf[8 + spr_x + start_x];
		
		if(spr[2] & 0x40) {
			start_x = (8 - 1) - start_x;
			end_x = (8 - 1) - end_x;
			inc_x = -1;
		}
		if(spr[2] & 0x80) {
			y = (spr_height - 1) - y;
		}
		uint8 priority = spr[2] & 0x20;
		
		for(int x = start_x; x != end_x; x += inc_x) {
			uint8 col = 0;
			uint32 tile_addr;
			uint8 tile_mask;
			
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
