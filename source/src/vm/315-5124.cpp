/*
	Skelton for retropc emulator

	Origin : SMS Plus
	Author : tanam
	Date   : 2013.09.14 -

	[ _315_5124 ]
*/

#include "z80.h"
#include "315-5124.h"
#include "gamegear/keyboard.h"

/* Return values from the V counter */
static const uint8_t vcnt[0x200] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA,
                                  0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
};

/* Return values from the H counter */
static const uint8_t hcnt[0x200] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
                      0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
};

/* Attribute expansion table */
static const uint32_t atex[4] =
{
	0x00000000,
	0x10101010,
	0x20202020,
	0x30303030,
};

#ifdef _MASTERSYSTEM
static const uint8_t tms_crom[] =
{
	0x00, 0x00, 0x08, 0x0C,
	0x10, 0x30, 0x01, 0x3C,
	0x02, 0x03, 0x05, 0x0F,
	0x04, 0x33, 0x15, 0x3F
};
#endif

/* Mark a pattern as dirty */
#define MARK_BG_DIRTY(addr)								\
{														\
	int name = (addr >> 5) & 0x1FF;						\
	if(bg_name_dirty[name] == 0)						\
	{													\
		bg_name_list[bg_list_index] = name;				\
		bg_list_index++;								\
	}													\
	bg_name_dirty[name] |= (1 << ((addr >> 2) & 7));	\
}

void _315_5124::initialize()
{
	// Game Gear : console = 0x40, vdp_mode = 4
	// Mark3     : console = 0x20, vdp_mode = 4
	// SC-3000   : console = 0x10, vdp_mode = 0
	// COLECO    : console = 0x00, vdp_mode = 0
	console = 0x20;
	// register event
	register_vline_event(this);
}

void _315_5124::reset()
{
	int i, j;
	int bx, sx, b, s, bp, bf, sf, c;
	
	/* Generate 64k of data for the look up table */
	for (bx = 0; bx < 0x100; bx++) {
		for (sx = 0; sx < 0x100; sx++) {
			/* Background pixel */
			b  = (bx & 0x0F);
			/* Background priority */
			bp = (bx & 0x20) ? 1 : 0;
			/* Full background pixel + priority + sprite marker */
			bf = (bx & 0x7F);
			/* Sprite pixel */
			s  = (sx & 0x0F);
			/* Full sprite pixel, w/ palette and marker bits added */
			sf = (sx & 0x0F) | 0x10 | 0x40;
			/* Overwriting a sprite pixel ? */
			if (bx & 0x40) {
				/* Return the input */
				c = bf;
			} else {
				/* Work out priority and transparency for both pixels */
				if (bp) {
					/* Underlying pixel is high priority */
					if (b) {
						c = bf | 0x40;
					} else {
						if (s) {
							c = sf;
						} else {
							c = bf;
						}
					}
				} else {
					/* Underlying pixel is low priority */
					if (s) {
						c = sf;
					} else {
						c = bf;
					}
				}
			}
			/* Store result */
			lut[(bx << 8) | (sx)] = c;
		}
	}
	
	/* Make bitplane to pixel lookup table */
	for (i = 0; i < 0x100; i++) {
		for (j = 0; j < 0x100; j++) {
			int x;
			uint32_t out = 0;
			for (x = 0; x < 8; x++) {
				out |= (j & (0x80 >> x)) ? (uint32_t)(8 << (x << 2)) : 0;
				out |= (i & (0x80 >> x)) ? (uint32_t)(4 << (x << 2)) : 0;
			}
			bp_lut[(j << 8) | (i)] = out;
		}
	}
	
	for (i = 0; i < 4; i++) {
		uint8_t c = i << 6 | i << 4 | i << 2 | i;
		sms_cram_expand_table[i] = c;
	}
	
	for (i = 0; i < 16; i++) {
		uint8_t c = i << 4 | i;
		gg_cram_expand_table[i] = c;
	}
	
	/* Invalidate pattern cache */
	memset(bg_name_dirty, 0, sizeof(bg_name_dirty));
	memset(bg_name_list, 0, sizeof(bg_name_list));
	bg_list_index = 0;
	memset(bg_pattern_cache, 0, sizeof(bg_pattern_cache));
	memset(cram, 0, sizeof(cram)); 
	
	linebuf = 0;
	cram_latch = 0;
	addrmode = 0;
	buffer = 0;
	hlatch = 0;
	vcounter = 0;
///	counter = 0;
	vdp_mode = 4;
	
	memset(vram, 0, sizeof(vram));
	memset(screen, 0, sizeof(screen));
	memset(regs, 0, sizeof(regs));
	status_reg = read_ahead = first_byte = 0;
	vram_addr = 0;
	intstat = latch = false;
	color_table = pattern_table = name_table = 0;
	sprite_pattern = sprite_attrib = 0;
	color_mask = pattern_mask = 0;
	
	// TMS9918 palette
	palette_pc[0]=RGB_COLOR(0,   0,   0);
	palette_pc[1]=RGB_COLOR(0,   0,   0);
	palette_pc[2]=RGB_COLOR(33, 200,  66);
	palette_pc[3]=RGB_COLOR(94, 220, 120);
	palette_pc[4]=RGB_COLOR(84,  85, 237);
	palette_pc[5]=RGB_COLOR(125, 118, 252);
	palette_pc[6]=RGB_COLOR(212,  82,  77);
	palette_pc[7]=RGB_COLOR(66, 235, 245);
	palette_pc[8]=RGB_COLOR(252,  85,  84);
	palette_pc[9]=RGB_COLOR(255, 121, 120);
	palette_pc[10]=RGB_COLOR(212, 193,  84);
	palette_pc[11]=RGB_COLOR(230, 206, 128);
	palette_pc[12]=RGB_COLOR(33, 176,  59);
	palette_pc[13]=RGB_COLOR(201,  91, 186);
	palette_pc[14]=RGB_COLOR(204, 204, 204);
	palette_pc[15]=RGB_COLOR(255, 255, 255);
	palette_pc[16]=RGB_COLOR(0,   0,   0);
	palette_pc[17]=RGB_COLOR(0,   0,   0);
	palette_pc[18]=RGB_COLOR(0,   0,   0);
	palette_pc[19]=RGB_COLOR(0,   0,   0);
	palette_pc[20]=RGB_COLOR(0,   0,   0);
	palette_pc[21]=RGB_COLOR(0,   0,   0);
	palette_pc[22]=RGB_COLOR(0,   0,   0);
	palette_pc[23]=RGB_COLOR(0,   0,   0);
	palette_pc[24]=RGB_COLOR(0,   0,   0);
	palette_pc[25]=RGB_COLOR(0,   0,   0);
	palette_pc[26]=RGB_COLOR(0,   0,   0);
	palette_pc[27]=RGB_COLOR(0,   0,   0);
	palette_pc[28]=RGB_COLOR(0,   0,   0);
	palette_pc[29]=RGB_COLOR(0,   0,   0);
	palette_pc[30]=RGB_COLOR(0,   0,   0);
	palette_pc[31]=RGB_COLOR(0,   0,   0);
#ifdef _MASTERSYSTEM // 315-5124 palette
	if (console) {   // without COLECO
		for (i = 0; i < 32; i++) {
			int r = (tms_crom[i & 0x0F] >> 0) & 3;
			int g = (tms_crom[i & 0x0F] >> 2) & 3;
			int b = (tms_crom[i & 0x0F] >> 4) & 3;
			r = sms_cram_expand_table[r];
			g = sms_cram_expand_table[g];
			b = sms_cram_expand_table[b];
			palette_pc[i]=RGB_COLOR(r, g, b);
		}
	}
#endif
	vp_x = (console == 0x40) ? 48 : 0;
	vp_y = (console == 0x40) ? 24 : 0;
	vp_w = (console == 0x40) ? 160 : 256;
	vp_h = (console == 0x40) ? 144 : 192;
}

void _315_5124::viewport_check(void)
{
	vdp_mode = 4;
	int m1 = (regs[1] >> 4) & 1;
	int m3 = (regs[1] >> 3) & 1;
	int m2 = (regs[0] >> 1) & 1;
	int m4 = (regs[0] >> 2) & 1;
	
	// check if this is switching out of tms
	if (!(console & 0x40)) {
		if (m4) {
			// Game Gear or Mark3
			for(int i = 0; i < 0x20; i++) {
				palette_sync(i, 1);
			}
		} else {
			// SC-3000
			d_key->sk1100=true;
			console = 0x10;
			vdp_mode = 0; 
		}
	}
	name_table = (regs[2] << 10) & 0x3800;
	color_table = (regs[3] <<  6) & 0x3FC0;
	pattern_table = (regs[4] << 11) & 0x3800;
	sprite_attrib = (regs[5] <<  7) & 0x3F00;
	sprite_pattern = (regs[6] << 11) & 0x3800;
}

void _315_5124::vdp_reg_w(uint8_t r, uint8_t d)
{
	// Store register data
	regs[r] = d;
	
	switch(r)
	{
	case 0x00: // Mode Control No. 1
	case 0x01: // Mode Control No. 2
	case 0x02: // Name Table A Base Address
		viewport_check();
		break;
	case 0x03:
	case 0x04:
	case 0x05: // Sprite Attribute Table Base Address
	case 0x06:
	case 0x07:
		break;
	}
}

void _315_5124::write_io8(uint32_t addr, uint32_t data)
{
	int index;
	if ((addr & 0x000000ff)==0x7f) {
		d_psg->write_io8(0xff, data);
		return;
	}
	if (addr & 1) {
		// register
		if(latch) {
			latch = false;
			// Game Gear or Mark3
			if (vdp_mode == 4) {
				addrmode = (data >> 6) & 3;
				vram_addr = (data << 8 | first_byte) & 0x3FFF;
				if (addrmode == 0) {
					 buffer = vram[vram_addr & 0x3FFF];
					vram_addr = (vram_addr + 1) & 0x3FFF;
				}
				if (addrmode == 2) {
					int r = (data & 0x0F);
					int d = first_byte;
					vdp_reg_w(r, d);
				}
				return;
			}
			// SC-3000
			if (data & 0x80) {
				switch(data & 7) {
				case 0:
					regs[0] = first_byte & 3;
					if(regs[0] & 2) {
						color_table = ((regs[3] & 0x80) * 64) & ADDR_MASK;
						color_mask = ((regs[3] & 0x7f) * 8) | 7;
						pattern_table = ((regs[4] & 4) * 2048) & ADDR_MASK;
						pattern_mask = ((regs[4] & 3) * 256) | (color_mask & 0xff);
					} else {
						color_table = (regs[3] * 64) & ADDR_MASK;
						pattern_table = (regs[4] * 2048) & ADDR_MASK;
					}
					break;
				case 1:
					regs[1] = first_byte & 0xfb;
					set_intstat((regs[1] & 0x20) && (status_reg & 0x80));
					break;
				case 2:
					regs[2] = first_byte & 0x0f;
					name_table = (regs[2] * 1024) & ADDR_MASK;
					break;
				case 3:
					regs[3] = first_byte;
					if(regs[0] & 2) {
						color_table = ((regs[3] & 0x80) * 64) & ADDR_MASK;
						color_mask = ((regs[3] & 0x7f) * 8) | 7;
					} else {
						color_table = (regs[3] * 64) & ADDR_MASK;
					}
					pattern_mask = ((regs[4] & 3) * 256) | (color_mask & 0xff);
					break;
				case 4:
					regs[4] = first_byte & 7;
					if(regs[0] & 2) {
						pattern_table = ((regs[4] & 4) * 2048) & ADDR_MASK;
						pattern_mask = ((regs[4] & 3) * 256) | 255;
					} else {
						pattern_table = (regs[4] * 2048) & ADDR_MASK;
					}
					break;
				case 5:
					regs[5] = first_byte & 0x7f;
					sprite_attrib = (regs[5] * 128) & ADDR_MASK;
					break;
				case 6:
					regs[6] = first_byte & 7;
					sprite_pattern = (regs[6] * 2048) & ADDR_MASK;
					break;
				case 7:
					regs[7] = first_byte;
					break;
				}
			} else {
				vram_addr = ((data * 256) | first_byte) & ADDR_MASK;
				if(!(data & 0x40)) {
					read_io8(0);	// read ahead
				}
			}
		} else {
            vram_addr = (vram_addr & 0x3F00) | (data & 0xFF);
			first_byte = data;
			latch = true;
		}
	} else {
		latch = false;
		// Game Gear or Mark3
		if (vdp_mode == 4) {
			switch(addrmode)
			{
			case 0: /* VRAM write */
			case 1: /* VRAM write */
			case 2: /* VRAM write */
				index = (vram_addr & 0x3FFF);
				if (data != vram[index]) {
					vram[index] = data;
					MARK_BG_DIRTY(vram_addr);
				}
				break;
			case 3: /* CRAM write */
				if (console & 0x40) {
					if (vram_addr & 1) {
						cram_latch = (cram_latch & 0x00FF) | ((data & 0xFF) << 8);
						cram[(vram_addr & 0x3E) | (0)] = (cram_latch >> 0) & 0xFF;
						cram[(vram_addr & 0x3E) | (1)] = (cram_latch >> 8) & 0xFF;
						palette_sync((vram_addr >> 1) & 0x1F, 0);
					} else {
						cram_latch = (cram_latch & 0xFF00) | ((data & 0xFF) << 0);
					}
				} else {
					index = (vram_addr & 0x1F);
					if(data != cram[index]) {
						cram[index] = data;
						palette_sync(index, 0);
					}
				}
				break;
			}
			vram_addr = (vram_addr + 1) & 0x3FFF;
			return;
		}
		// SC-3000
		vram[vram_addr] = data;
		vram_addr = (vram_addr + 1) & ADDR_MASK;
		read_ahead = data;
	}
}

uint32_t _315_5124::read_io8(uint32_t addr)
{
	uint8_t temp;
	if ((addr & 0x000000ff) == 0x7e) {
		/* V Counter */
		return vcnt[vcounter & 0x01FF];
	}
	if ((addr & 0x000000ff) == 0x7f) {
		/* H Counter */
		uint16_t pixel = (((z80_icount % CYCLES_PER_LINE) / 4) * 3) * 2;
		return hcnt[(pixel >> 1) & 0x01FF];;
	}
	switch(addr & 1)
	{
	case 0: // CPU <-> VDP data buffer
		latch = false;
		temp = buffer;
		buffer = vram[vram_addr & 0x3FFF];
		vram_addr = (vram_addr + 1) & 0x3FFF;
		return temp;
	case 1: // Status flags
		temp = status_reg;
		latch = false;
		status_reg = 0;
		set_intstat(false);
		return temp;
	}
	// Just to please the compiler
	return -1;
}

void _315_5124::draw_screen()
{
	if(emu->now_waiting_in_debugger) {
		// store regs
		uint16_t tmp_z80_icount = z80_icount;
		uint16_t tmp_vcounter = vcounter;
		uint8_t tmp_status_reg = status_reg;
		uint16_t tmp_hlatch = hlatch;
		bool tmp_intstat = intstat;
		
		// drive vlines
		for(int v = /*get_cur_vline() + 1*/0; v < get_lines_per_frame(); v++) {
			event_vline(v, 0);
		}
		
		// restore regs
		z80_icount = tmp_z80_icount;
		vcounter = tmp_vcounter;
		status_reg = tmp_status_reg;
		hlatch = tmp_hlatch;
		intstat = tmp_intstat;
	}
	
	// update screen buffer
	for(int y = 0; y < 192; y++) {
		scrntype_t* dest = emu->get_screen_buffer(y);
		uint8_t* src = screen[y];
		for(int x = 0; x < 256; x++) {
			if (x>=vp_x && x<vp_x+vp_w && y>=vp_y && y<vp_y+vp_h) {
				int color=(src[x] & 0x1f);
				dest[x] = palette_pc[color];
			} else {
				dest[x] = palette_pc[0x10];
			}
		}
	}
}

void _315_5124::event_vline(int v, int clock)
{
	z80_icount++;
	vcounter = v;
	if (vdp_mode == 4) {
		if(v <= 0xC0) {
			if(v == 0xC0) {
				status_reg |= 0x80;
			}
			if(v == 0) {
				hlatch = regs[10];
			}
			if(hlatch == 0) {
				hlatch = regs[10];
				status_reg |= 0x40;
			} else {
				hlatch -= 1;
			}
			if((status_reg & 0x40) && (regs[0] & 0x10)) {
				status_reg |= 0x40;
				set_intstat((regs[0] & 0x10) != 0);
			}
		} else {
			hlatch = regs[10];
			if((v < 0xE0) && (status_reg & 0x80) && (regs[1] & 0x20)) {
				status_reg |= 0x80;
				set_intstat((regs[1] & 0x20) != 0);
			}
		}
		
		/* Ensure we're within the viewport range */
		if(v < vp_y || v >= (vp_y + vp_h)) return;
		
		/* Point to current line in output buffer */
		linebuf = (uint8_t *)screen[v];
		
		/* Update pattern cache */
		update_bg_pattern_cache();
		
		/* Blank line (full width) */
		if (!(regs[1] & 0x40)) {
			memset(linebuf, BACKDROP_COLOR, 256);
		} else {
			/* Draw background */
			render_bg(v);
			/* Draw sprites */
			render_obj(v);
			/* Blank leftmost column of display */
			if (regs[0] & 0x20) {
				memset(linebuf, BACKDROP_COLOR, 8);
			}
		}
	} else {
		if (v == 192) {
			// create virtual screen
			if(regs[1] & 0x40) {
				// draw character plane
				int mode = (regs[0] & 2) | ((regs[1] & 0x10) >> 4) | ((regs[1] & 8) >> 1);
				switch(mode)
				{
				case 0:
					draw_mode0();
					break;
				case 1:
					draw_mode1();
					break;
				case 2:
					draw_mode2();
					break;
				case 3:
					draw_mode12();
					break;
				case 4:
					draw_mode3();
					break;
				case 6:
					draw_mode23();
					break;
				case 5:
				case 7:
					draw_modebogus();
					break;
				}
				// draw sprite plane
				if((regs[1] & 0x50) == 0x40) {
					draw_sprites();
				}
			} else {
				memset(screen, 0, sizeof(screen));
			}
			// do interrupt
			status_reg |= 0x80;
			set_intstat((regs[1] & 0x20) != 0);
		}
	}
}

void _315_5124::set_intstat(bool val)
{
	if(val != intstat) {
		if(!emu->now_waiting_in_debugger) {
			write_signals(&outputs_irq, val ? 0xffffffff : 0);
		}
		intstat = val;
	}
}

void _315_5124::draw_mode0()
{
	for(int y = 0, name = 0; y < 24; y++) {
		for(int x = 0; x < 32; x++) {
			uint16_t code = vram[name_table + (name++)];
			uint8_t* pattern_ptr = vram + pattern_table + code * 8;
			uint8_t color = vram[color_table + (code >> 3)];
			uint8_t fg = (color & 0xf0) ? (color >> 4) : (regs[7] & 0x0f);
			uint8_t bg = (color & 0x0f) ? (color & 0x0f) : (regs[7] & 0x0f);
			for(int yy=0; yy < 8; yy++) {
				uint8_t pattern = *pattern_ptr++;
				uint8_t* buffer = screen[y * 8 + yy] + x * 8;
				buffer[0] = (pattern & 0x80) ? fg : bg;
				buffer[1] = (pattern & 0x40) ? fg : bg;
				buffer[2] = (pattern & 0x20) ? fg : bg;
				buffer[3] = (pattern & 0x10) ? fg : bg;
				buffer[4] = (pattern & 0x08) ? fg : bg;
				buffer[5] = (pattern & 0x04) ? fg : bg;
				buffer[6] = (pattern & 0x02) ? fg : bg;
				buffer[7] = (pattern & 0x01) ? fg : bg;
			}
		}
	}
}

void _315_5124::draw_mode1()
{
	uint8_t fg = regs[7] >> 4;
	uint8_t bg = regs[7] & 0x0f;
	for(int y = 0, name = 0; y < 24; y++) {
		for(int x = 0; x < 40; x++) {
			uint16_t code = vram[name_table + (name++)];
			uint8_t* pattern_ptr = vram + pattern_table + code * 8;
			for(int yy = 0; yy < 8; yy++) {
				uint8_t pattern = *pattern_ptr++;
				uint8_t* buffer = screen[y * 8 + yy] + x * 6 + 8;
				buffer[0] = (pattern & 0x80) ? fg : bg;
				buffer[1] = (pattern & 0x40) ? fg : bg;
				buffer[2] = (pattern & 0x20) ? fg : bg;
				buffer[3] = (pattern & 0x10) ? fg : bg;
				buffer[4] = (pattern & 0x08) ? fg : bg;
				buffer[5] = (pattern & 0x04) ? fg : bg;
			}
		}
	}
}

void _315_5124::draw_mode2()
{
	for(int y = 0, name = 0; y < 24; y++) {
		for(int x = 0; x < 32; x++) {
			uint16_t code = vram[name_table + (name++)] + (y & 0xf8) * 32;
			uint8_t* pattern_ptr = vram + pattern_table + (code & pattern_mask) * 8;
			uint8_t* color_ptr = vram + color_table + (code & color_mask) * 8;
			for(int yy = 0; yy < 8; yy++) {
				uint8_t pattern = *pattern_ptr++;
				uint8_t color = *color_ptr++;
				uint8_t fg = (color & 0xf0) ? (color >> 4) : (regs[7] & 0x0f);
				uint8_t bg = (color & 0x0f) ? (color & 0x0f) : (regs[7] & 0x0f);
				uint8_t* buffer = screen[y * 8 + yy] + x * 8;
				buffer[0] = (pattern & 0x80) ? fg : bg;
				buffer[1] = (pattern & 0x40) ? fg : bg;
				buffer[2] = (pattern & 0x20) ? fg : bg;
				buffer[3] = (pattern & 0x10) ? fg : bg;
				buffer[4] = (pattern & 0x08) ? fg : bg;
				buffer[5] = (pattern & 0x04) ? fg : bg;
				buffer[6] = (pattern & 0x02) ? fg : bg;
				buffer[7] = (pattern & 0x01) ? fg : bg;
			}
		}
	}
}

void _315_5124::draw_mode12()
{
	uint8_t fg = regs[7] >> 4;
	uint8_t bg = regs[7] & 0x0f;
	for(int y = 0, name = 0; y < 24; y++) {
		for(int x = 0; x < 40; x++) {
			uint16_t code = vram[name_table + (name++)] + (y & 0xf8) * 32;
			uint8_t* pattern_ptr = vram + pattern_table + (code & pattern_mask) * 8;
			for(int yy = 0; yy < 8; yy++) {
				uint8_t pattern = *pattern_ptr++;
				uint8_t* buffer = screen[y * 8 + yy] + x * 6 + 8;
				buffer[0] = (pattern & 0x80) ? fg : bg;
				buffer[1] = (pattern & 0x40) ? fg : bg;
				buffer[2] = (pattern & 0x20) ? fg : bg;
				buffer[3] = (pattern & 0x10) ? fg : bg;
				buffer[4] = (pattern & 0x08) ? fg : bg;
				buffer[5] = (pattern & 0x04) ? fg : bg;
			}
		}
	}
}

void _315_5124::draw_mode3()
{
	for(int y = 0, name = 0; y < 24; y++) {
		for(int x = 0; x < 32; x++) {
			uint16_t code = vram[name_table + (name++)];
			uint8_t* pattern_ptr = vram + pattern_table + code * 8 + (y & 3) * 2;
			for(int yy = 0; yy < 2; yy++) {
				uint8_t color = *pattern_ptr++;
				uint8_t fg = (color & 0xf0) ? (color >> 4) : (regs[7] & 0x0f);
				uint8_t bg = (color & 0x0f) ? (color & 0x0f) : (regs[7] & 0x0f);
				for(int yyy = 0; yyy < 4; yyy++) {
					uint8_t* buffer = screen[y * 8 + yy * 4 + yyy] + x * 8;
					buffer[0] = buffer[1] = buffer[2] = buffer[3] = fg;
					buffer[4] = buffer[5] = buffer[6] = buffer[7] = bg;
				}
			}
		}
	}
}

void _315_5124::draw_mode23()
{
	for(int y = 0, name = 0; y < 24;y++) {
		for(int x = 0; x < 32; x++) {
			uint16_t code = vram[name_table + (name++)];
			uint8_t* pattern_ptr = vram + pattern_table + ((code + (y & 3) * 2 + (y & 0xf8) * 32) & pattern_mask) * 8;
			for(int yy = 0; yy < 2; yy++) {
				uint8_t color = *pattern_ptr++;
				uint8_t fg = (color & 0xf0) ? (color >> 4) : (regs[7] & 0x0f);
				uint8_t bg = (color & 0x0f) ? (color & 0x0f) : (regs[7] & 0x0f);
				for(int yyy = 0; yyy < 4; yyy++) {
					uint8_t* buffer = screen[y * 8 + yy * 4 + yyy] + x * 8;
					buffer[0] = buffer[1] = buffer[2] = buffer[3] = fg;
					buffer[4] = buffer[5] = buffer[6] = buffer[7] = bg;
				}
			}
		}
	}
}

void _315_5124::draw_modebogus()
{
	uint8_t fg = regs[7] >> 4;
	uint8_t bg = regs[7] & 0x0f;
	for(int y = 0; y < 192; y++) {
		uint8_t* buffer = screen[y];
		int x = 0;
		for(int i = 0; i < 8; i++) {
			buffer[x++] = bg;
		}
		for(int i = 0; i < 40; i++) {
			for(int j = 0; j < 4; j++) {
				buffer[x++] = fg;
			}
			for(int j = 0; j < 2; j++) {
				buffer[x++] = bg;
			}
		}
		for(int i = 0; i < 8; i++) {
			buffer[x++] = bg;
		}
	}
}

void _315_5124::draw_sprites()
{
	uint8_t* attrib_ptr = vram + sprite_attrib;
	int size = (regs[1] & 2) ? 16 : 8;
	bool large = ((regs[1] & 1) != 0);
	uint8_t limit[192], collision[192][256];
	int illegal_sprite = 0, illegal_sprite_line = 255, p;
	memset(limit, 4, sizeof(limit));
	memset(collision, 0, sizeof(collision));
	status_reg = 0x80;
	
	for(p = 0; p < 32; p++) {
		int y = *attrib_ptr++;
		if(y == 208) {
			break;
		}
		if(y > 208) {
			y = -(~y & 0xff);
		} else {
			y++;
		}
		int x = *attrib_ptr++;
		uint8_t* pattern_ptr = vram + sprite_pattern + ((size == 16) ? (*attrib_ptr & 0xfc) : *attrib_ptr) * 8;
		attrib_ptr++;
		uint8_t c = *attrib_ptr & 0x0f;
		if(*attrib_ptr & 0x80) {
			x -= 32;
		}
		attrib_ptr++;
		
		if(!large) {
			// draw sprite (not enlarged)
			for(int yy = y; yy < y + size; yy++) {
				if(yy < 0 || 191 < yy) {
					continue;
				}
				if(limit[yy] == 0) {
					// illegal sprite line
					if(yy < illegal_sprite_line) {
						illegal_sprite_line = yy;
						illegal_sprite = p;
					} else if(illegal_sprite_line == yy) {
						if(illegal_sprite > p) {
							illegal_sprite = p;
						}
					}
#ifdef _315_5124_LIMIT_SPRITES
					continue;
#endif
				} else {
					limit[yy]--;
				}
				uint16_t line = pattern_ptr[yy - y] * 256 + pattern_ptr[yy - y + 16];
				for(int xx = x; xx < x + size; xx++) {
					if(line & 0x8000) {
						if(0 <= xx && xx < 256) {
							if(collision[yy][xx]) {
								status_reg |= 0x20;
							} else {
								collision[yy][xx] = 1;
							}
							if(c && !(collision[yy][xx] & 2)) {
								collision[yy][xx] |= 2;
								screen[yy][xx] = c;
							}
						}
					}
					line *= 2;
				}
			}
		} else {
			// draw enlarged sprite
			for(int i = 0; i < size; i++) {
				int yy = y + i * 2;
				uint16_t line2 = pattern_ptr[i] * 256 + pattern_ptr[i + 16];
				for(int j = 0; j < 2; j++) {
					if(0 <= yy && yy <= 191) {
						if(limit[yy] == 0) {
							// illegal sprite line
							if(yy < illegal_sprite_line) {
								illegal_sprite_line = yy;
								 illegal_sprite = p;
							} else if(illegal_sprite_line == yy) {
								if(illegal_sprite > p) {
									illegal_sprite = p;
								}
							}
#ifdef _315_5124_LIMIT_SPRITES
							continue;
#endif
						} else {
							limit[yy]--;
						}
						uint16_t line = line2;
						for(int xx = x; xx < x + size * 2; xx += 2) {
							if(line & 0x8000) {
								if(0 <= xx && xx < 256) {
									if(collision[yy][xx]) {
										status_reg |= 0x20;
									} else {
										collision[yy][xx] = 1;
									}
									if(c && !(collision[yy][xx] & 2)) {
										collision[yy][xx] |= 2;
										screen[yy][xx] = c;
									}
								}
								if(0 <= xx + 1 && xx + 1 < 256) {
									if(collision[yy][xx + 1]) {
										status_reg |= 0x20;
									} else {
										collision[yy][xx + 1] = 1;
									}
									if(c && !(collision[yy][xx + 1] & 2)) {
										collision[yy][xx + 1] |= 2;
										screen[yy][xx + 1] = c;
									}
								}
							}
							line *= 2;
						}
					}
					yy++;
				}
			}
		}
	}
	if(illegal_sprite_line == 255) {
		status_reg |= (p > 31) ? 31 : p;
	} else {
		status_reg |= 0x40 + illegal_sprite;
	}
}

/* Draw the background */
void _315_5124::render_bg(int line)
{
	int locked = 0;
	int yscroll_mask = 224;
	int v_line = (line + regs[9]) % yscroll_mask;
	int v_row  = (v_line & 7) << 3;
	int hscroll = ((regs[0] & 0x40) && (line < 0x10)) ? 0 : (0x100 - regs[8]);
	int column = 0;
	uint16_t attr;
	uint16_t *nt = (uint16_t *)&vram[name_table + ((v_line >> 3) << 6)];
	int nt_scroll = (hscroll >> 3);
	int shift = (hscroll & 7);
	uint32_t atex_mask;
	uint32_t *cache_ptr;
	uint32_t *linebuf_ptr = (uint32_t *)&linebuf[0 - shift];
	
	/* Draw first column (clipped) */
	if(shift) {
		int x;
		for(x = shift; x < 8; x++)
			linebuf[(0 - shift) + (x)] = 0;
		column++;
	}
	/* Draw a line of the background */
	for (; column < 32; column++) {
		/* Stop vertical scrolling for leftmost eight columns */
		if ((regs[0] & 0x80) && (!locked) && (column >= 24)) {
			locked = 1;
			v_row = (line & 7) << 3;
			nt = (uint16_t *)&vram[((regs[2] << 10) & 0x3800) + ((line >> 3) << 6)];
		}
		/* Get name table attribute word */
		attr = nt[(column + nt_scroll) & 0x1F];
		/* Expand priority and palette bits */
		atex_mask = atex[(attr >> 11) & 3];
		/* Point to a line of pattern data in cache */
		cache_ptr = (uint32_t *)&bg_pattern_cache[((attr & 0x7FF) << 6) | (v_row)];
		/* Copy the left half, adding the attribute bits in */
		write_dword( &linebuf_ptr[(column << 1)] , read_dword( &cache_ptr[0] ) | (atex_mask));
		/* Copy the right half, adding the attribute bits in */
		write_dword( &linebuf_ptr[(column << 1) | (1)], read_dword( &cache_ptr[1] ) | (atex_mask));
	}
	/* Draw last column (clipped) */
	if (shift) {
		int x, c, a;
		uint8_t *p = &linebuf[(0 - shift)+(column << 3)];
		attr = nt[(column + nt_scroll) & 0x1F];
		a = (attr >> 7) & 0x30;
		for (x = 0; x < shift; x++) {
			c = bg_pattern_cache[((attr & 0x7FF) << 6) | (v_row) | (x)];
			p[x] = ((c) | (a));
		}
	}
}

/* Draw sprites */
void _315_5124::render_obj(int line)
{
	int i;
	uint8_t collision_buffer = 0;
	/* Sprite count for current line (8 max.) */
	int count = 0;
	/* Sprite dimensions */
	int width = 8;
	int height = (regs[1] & 0x02) ? 16 : 8;
	/* Pointer to sprite attribute table */
	uint8_t *st = (uint8_t *)&vram[sprite_attrib];
	/* Adjust dimensions for double size sprites */
	if (regs[1] & 0x01) {
		width *= 2;
		height *= 2;
	}
	/* Draw sprites in front-to-back order */
	for (i = 0; i < 64; i++) {
		/* Sprite Y position */
		int yp = st[i];
		/* Found end of sprite list marker for non-extended modes? */
		if (yp == 208) goto end;
		/* Actual Y position is +1 */
		yp++;
		/* Wrap Y coordinate for sprites > 240 */
		if (yp > 240) yp -= 256;
		/* Check if sprite falls on current line */
		if ((line >= yp) && (line < (yp + height))) {
			uint8_t *linebuf_ptr;
			/* Width of sprite */
			int start = 0;
			int end = width;
			/* Sprite X position */
			int xp = st[0x80 + (i << 1)];
			/* Pattern name */
			int n = st[0x81 + (i << 1)];
			/* Bump sprite count */
			count++;
			/* Too many sprites on this line ? */
			if (count == 9) {
				status_reg |= 0x40;
				goto end;
			}
			/* X position shift */
			if (regs[0] & 0x08) xp -= 8;
			/* Add MSB of pattern name */
			if (regs[6] & 0x04) n |= 0x0100;
			/* Mask LSB for 8x16 sprites */
			if (regs[1] & 0x02) n &= 0x01FE;
			/* Point to offset in line buffer */
			linebuf_ptr = (uint8_t *)&linebuf[xp];
			/* Clip sprites on left edge */
			if (xp < 0) {
				start = (0 - xp);
			}
			/* Clip sprites on right edge */
			if ((xp + width) > 256) {
				end = (256 - xp);
			}
			/* Draw double size sprite */
			if (regs[1] & 0x01) {
				int x;
				uint8_t *cache_ptr = (uint8_t *)&bg_pattern_cache[(n << 6) | (((line - yp) >> 1) << 3)];
				/* Draw sprite line */
				for (x = start; x < end; x++) {
					/* Source pixel from cache */
					uint8_t sp = cache_ptr[(x >> 1)];
					/* Only draw opaque sprite pixels */
					if (sp) {
						/* Background pixel from line buffer */
						uint8_t bg = linebuf_ptr[x];
						/* Look up result */
						linebuf_ptr[x] = lut[(bg << 8) | (sp)];
						/* Update collision buffer */
						collision_buffer |= bg;
					}
				}
			} else { /* Regular size sprite (8x8 / 8x16) */
				int x;
				uint8_t *cache_ptr = (uint8_t *)&bg_pattern_cache[(n << 6) | ((line - yp) << 3)];
				/* Draw sprite line */
				for (x = start; x < end; x++) {
					/* Source pixel from cache */
					uint8_t sp = cache_ptr[x];
					/* Only draw opaque sprite pixels */
					if (sp) {
						/* Background pixel from line buffer */
						uint8_t bg = linebuf_ptr[x];
						/* Look up result */
						linebuf_ptr[x] = lut[(bg << 8) | (sp)];
						/* Update collision buffer */
						collision_buffer |= bg;
					}
				}
			}
		}
	}
end:
	/* Set sprite collision flag */
	if (collision_buffer & 0x40) status_reg |= 0x20;
}

void _315_5124::update_bg_pattern_cache(void)
{
	int i;
	uint8_t x, y;
	uint16_t name;
	if (!bg_list_index) return;
	for (i = 0; i < bg_list_index; i++) {
		name = bg_name_list[i];
		bg_name_list[i] = 0;
		for (y = 0; y < 8; y++) {
			if (bg_name_dirty[name] & (1 << y)) {
				uint8_t *dst = &bg_pattern_cache[name << 6];
				uint16_t bp01 = *(uint16_t *)&vram[(name << 5) | (y << 2) | (0)];
				uint16_t bp23 = *(uint16_t *)&vram[(name << 5) | (y << 2) | (2)];
				uint32_t temp = (bp_lut[bp01] >> 2) | (bp_lut[bp23]);
				for (x = 0; x < 8; x++) {
					uint8_t c = (temp >> (x << 2)) & 0x0F;
					dst[0x00000 | (y << 3) | (x)] = (c);
					dst[0x08000 | (y << 3) | (x ^ 7)] = (c);
					dst[0x10000 | ((y ^ 7) << 3) | (x)] = (c);
					dst[0x18000 | ((y ^ 7) << 3) | (x ^ 7)] = (c);
				}
			}
		}
		bg_name_dirty[name] = 0;
	}
	bg_list_index = 0;
}

void _315_5124::palette_sync(int index, int force)
{
	int r, g, b;
	if (console & 0x40) {
		/* ----BBBBGGGGRRRR */
		r = (cram[(index << 1) | (0)] >> 0) & 0x0F;
		g = (cram[(index << 1) | (0)] >> 4) & 0x0F;
		b = (cram[(index << 1) | (1)] >> 0) & 0x0F;
		r = gg_cram_expand_table[r];
		g = gg_cram_expand_table[g];
		b = gg_cram_expand_table[b];
	} else {
		// unless we are forcing an update,
		// if not in mode 4, exit
		if (!force && (regs[0] & 4) == 0) return;
		/* --BBGGRR */
		r = (cram[index] >> 0) & 3;
		g = (cram[index] >> 2) & 3;
		b = (cram[index] >> 4) & 3;
		r = sms_cram_expand_table[r];
		g = sms_cram_expand_table[g];
		b = sms_cram_expand_table[b];
	}
	palette_pc[index] = RGB_COLOR(r, g, b);
}
