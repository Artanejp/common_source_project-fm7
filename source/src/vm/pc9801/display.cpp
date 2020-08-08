/*
	NEC PC-9801 Emulator 'ePC-9801'
	NEC PC-9801E/F/M Emulator 'ePC-9801E'
	NEC PC-9801U Emulator 'ePC-9801U'
	NEC PC-9801VF Emulator 'ePC-9801VF'
	NEC PC-9801VM Emulator 'ePC-9801VM'
	NEC PC-9801VX Emulator 'ePC-9801VX'
	NEC PC-9801RA Emulator 'ePC-9801RA'
	NEC PC-98XA Emulator 'ePC-98XA'
	NEC PC-98XL Emulator 'ePC-98XL'
	NEC PC-98RL Emulator 'ePC-98RL'
	NEC PC-98DO Emulator 'ePC-98DO'

	Author : Takeda.Toshiya
	Date   : 2010.09.16-

	[ display ]
*/

#include "display.h"
#include "../i8259.h"
#include "../upd7220.h"

#if !defined(SUPPORT_HIRESO)
	#define TVRAM_ADDRESS		0xa0000
	#define VRAM_PLANE_SIZE		0x08000
	#define VRAM_PLANE_ADDR_MASK	0x07fff
	#define VRAM_PLANE_ADDR_0	0x08000
	#define VRAM_PLANE_ADDR_1	0x10000
	#define VRAM_PLANE_ADDR_2	0x18000
	#define VRAM_PLANE_ADDR_3	0x00000
#else
	#define TVRAM_ADDRESS		0xe0000
	#define VRAM_PLANE_SIZE		0x20000
	#define VRAM_PLANE_ADDR_MASK	0x1ffff
	#define VRAM_PLANE_ADDR_0	0x00000
	#define VRAM_PLANE_ADDR_1	0x20000
	#define VRAM_PLANE_ADDR_2	0x40000
	#define VRAM_PLANE_ADDR_3	0x60000
#endif

#define SCROLL_PL	0
#define SCROLL_BL	1
#define SCROLL_CL	2
#define SCROLL_SSL	3
#define SCROLL_SUR	4
#define SCROLL_SDR	5

#define MODE1_ATRSEL	0
#define MODE1_GRAPHIC	1
#define MODE1_COLUMN	2
#define MODE1_FONTSEL	3
#define MODE1_200LINE	4
#define MODE1_KAC	5
#define MODE1_MEMSW	6
#define MODE1_DISP	7

#define MODE2_16COLOR	0x00
#define MODE2_EGC	0x02
#define MDOE2_TXTSHIFT	0x20

#define GRCG_PLANE_0	0x01
#define GRCG_PLANE_1	0x02
#define GRCG_PLANE_2	0x04
#define GRCG_PLANE_3	0x08
#define GRCG_RW_MODE	0x40
#define GRCG_CG_MODE	0x80

#define ATTR_ST		0x01
#define ATTR_BL		0x02
#define ATTR_RV		0x04
#define ATTR_UL		0x08
#define ATTR_VL		0x10
#define ATTR_COL	0xe0

namespace PC9801 {
static const uint8_t memsw_default[] = {
	0xe1, 0x48, 0xe1, 0x05, 0xe1, 0x04, 0xe1, 0x00,
	0xe1, 0x01, 0xe1, 0x00, 0xe1, 0x00, 0xe1, 0x00,
};


void DISPLAY::initialize()
{
	// load font data
//	memset(font, 0xff, sizeof(font));
	memset(font, 0x00, sizeof(font));
	
	FILEIO* fio = new FILEIO();
	
#if !defined(SUPPORT_HIRESO)
	uint8_t *p = font + 0x81000;
	uint8_t *q = font + 0x83000;
	for(int i = 0; i < 256; i++) {
		for(int j = 0; j < 4; j++) {
			uint32_t bit = 0;
			if(i & (1 << j)) {
				bit |= 0xf0f0f0f0;
			}
			if(i & (0x10 << j)) {
				bit |= 0x0f0f0f0f;
			}
//			*(uint32_t *)p = bit;
//			p += 4;
			*p++ = (bit >>  0);
			*p++ = (bit >>  8);
			*p++ = (bit >> 16);
			*p++ = (bit >> 24);
//			*(uint16_t *)q = (uint16_t)bit;
//			q += 2;
			*q++ = (bit >>  0);
			*q++ = (bit >>  8);
		}
		q += 8;
	}
	for(int i = 0; i < 0x80; i++) {
		q = font + (i << 12);
		memset(q + 0x000, 0, 0x0560 - 0x000);
		memset(q + 0x580, 0, 0x0d60 - 0x580);
		memset(q + 0xd80, 0, 0x1000 - 0xd80);
	}
	if(!fio->Fopen(create_local_path(_T("FONT16.ROM")), FILEIO_READ_BINARY)) {
		fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY);
	}
	if(fio->IsOpened()) {
		uint8_t *buf = (uint8_t *)malloc(0x46800);
		fio->Fread(buf, 0x46800, 1);
		fio->Fclose();
		
		// 8x8 font
		uint8_t *dst = font + 0x82000;
		uint8_t *src = buf;
		for(int i = 0; i < 256; i++) {
			memcpy(dst, src, 8);
			dst += 16;
			src += 8;
		}
		// 8x16 font
		memcpy(font + 0x80000, buf + 0x0800, 16 * 128);
		memcpy(font + 0x80800, buf + 0x1000, 16 * 128);
		// kanji font
		kanji_copy(font, buf, 0x01, 0x30);
		kanji_copy(font, buf, 0x30, 0x56);
		kanji_copy(font, buf, 0x58, 0x5d);
		
		free(buf);
	}
#else
	if(fio->Fopen(create_local_path(_T("FONT24.ROM")), FILEIO_READ_BINARY)) {
		uint8_t pattern[72];
		
		for(int code = 0x00; code <= 0xff; code++) {
			fio->Fread(pattern, 48, 1);
			ank_copy(code, pattern);
		}
#if 1
		for(int first = 0x21; first <= 0x7c; first++) {
			for(int second = 0x21; second <= 0x7e; second++) {
				fio->Fread(pattern, 72, 1);
				kanji_copy(first, second, pattern);
			}
		}
#else
		for(int first = 0x21; first <= 0x27; first++) {
			for(int second = 0x21; second <= 0x7e; second++) {
				fio->Fread(pattern, 72, 1);
				kanji_copy(first, second, pattern);
			}
		}
		for(int first = 0x30; first <= 0x73; first++) {
			for(int second = 0x21; second <= 0x7e; second++) {
				fio->Fread(pattern, 72, 1);
				kanji_copy(first, second, pattern);
			}
		}
		for(int first = 0x79; first <= 0x7c; first++) {
			for(int second = 0x21; second <= 0x7e; second++) {
				fio->Fread(pattern, 72, 1);
				kanji_copy(first, second, pattern);
			}
		}
#endif
		for(int code = 0x20; code <= 0x7f; code++) {
			for(int line = 0; line < 24; line++) {
//				uint16_t pattern = (*(uint16_t *)(&font[ANK_FONT_OFS + FONT_SIZE * code + line * 2])) & 0x3fff;
//				*(uint16_t *)(&font[FONT_SIZE * (0x09 | (code << 8)) + line * 2                ]) = pattern << 2;
//				*(uint16_t *)(&font[FONT_SIZE * (0x09 | (code << 8)) + line * 2 + KANJI_2ND_OFS]) = 0;
				uint16_t pattern;
				pattern  = (font[ANK_FONT_OFS + FONT_SIZE * code + line * 2 + 0]       );
				pattern |= (font[ANK_FONT_OFS + FONT_SIZE * code + line * 2 + 1] & 0x3f) << 8;
				font[FONT_SIZE * (0x09 | (code << 8)) + line * 2                 + 0] = pattern << 2;
				font[FONT_SIZE * (0x09 | (code << 8)) + line * 2                 + 1] = pattern >> 6;
				font[FONT_SIZE * (0x09 | (code << 8)) + line * 2 + KANJI_2ND_OFS + 0] = 0;
				font[FONT_SIZE * (0x09 | (code << 8)) + line * 2 + KANJI_2ND_OFS + 1] = 0;
			}
		}
		for(int code = 0xa0; code <= 0xff; code++) {
			for(int line = 0; line < 24; line++) {
//				uint16_t pattern = (*(uint16_t *)(&font[ANK_FONT_OFS + FONT_SIZE * code + line * 2])) & 0x3fff;
//				*(uint16_t *)(&font[FONT_SIZE * (0x0a | (code << 8)) + line * 2                ]) = pattern << 2;
//				*(uint16_t *)(&font[FONT_SIZE * (0x0a | (code << 8)) + line * 2 + KANJI_2ND_OFS]) = 0;
				uint16_t pattern;
				pattern  = (font[ANK_FONT_OFS + FONT_SIZE * code + line * 2 + 0]       );
				pattern |= (font[ANK_FONT_OFS + FONT_SIZE * code + line * 2 + 1] & 0x3f) << 8;
				font[FONT_SIZE * (0x0a | (code << 8)) + line * 2                 + 0] = pattern << 2;
				font[FONT_SIZE * (0x0a | (code << 8)) + line * 2                 + 1] = pattern >> 6;
				font[FONT_SIZE * (0x0a | (code << 8)) + line * 2 + KANJI_2ND_OFS + 0] = 0;
				font[FONT_SIZE * (0x0a | (code << 8)) + line * 2 + KANJI_2ND_OFS + 1] = 0;
			}
		}
		memcpy(font + ANK_FONT_OFS + FONT_SIZE * 0x100, font + ANK_FONT_OFS, FONT_SIZE * 0x100);
		memcpy(font + ANK_FONT_OFS + FONT_SIZE * 0x200, font + ANK_FONT_OFS, FONT_SIZE * 0x100);
		memcpy(font + ANK_FONT_OFS + FONT_SIZE * 0x300, font + ANK_FONT_OFS, FONT_SIZE * 0x100);
	}
#endif
	
	// init palette
	for(int i = 0; i < 8; i++) {
		palette_chr[i] = RGB_COLOR((i & 2) ? 0xff : 0, (i & 4) ? 0xff : 0, (i & 1) ? 0xff : 0);
	}
	
	memset(palette_gfx8, 0, sizeof(palette_gfx8));
	memset(digipal, 0, sizeof(digipal));
	
#if defined(SUPPORT_16_COLORS)
	memset(palette_gfx16, 0, sizeof(palette_gfx16));
	memset(anapal, 0, sizeof(anapal));
	anapal_sel = 0;
#endif
	
//	memset(tvram, 0, sizeof(tvram));
	memset(vram, 0, sizeof(vram));
	
	bool memsw_stat = false;
	if((config.dipswitch & (1 << DIPSWITCH_POSITION_NOINIT_MEMSW)) != 0) {
		if(fio->Fopen(create_local_path(_T("MEMSW.BIN")), FILEIO_READ_BINARY)) {
			if(fio->IsOpened()) {
				for(int i = 0; i < 16; i++) {
					tvram[0x3fe0 + (i << 1)] = fio->FgetUint8();
					if(i == 15) memsw_stat = true;
				}
				fio->Fclose();
			}
		}
	}
	if(!memsw_stat) {
		init_memsw();
	}
#ifndef HAS_UPD4990A
	cur_time_t cur_time;
	get_host_time(&cur_time);
	tvram[0x3ffe] = TO_BCD(cur_time.year);
#endif
	
	// set vram pointer to gdc
	d_gdc_chr->set_vram_ptr(tvram, 0x2000);
	d_gdc_chr->set_screen_width(80);
#if !defined(SUPPORT_HIRESO)
	#if !defined(SUPPORT_2ND_VRAM)
	d_gdc_gfx->set_vram_bus_ptr(this, 0x20000);
	#else
	d_gdc_gfx->set_vram_bus_ptr(this, 0x40000);
	#endif
#else
	d_gdc_gfx->set_vram_bus_ptr(this, 0x80000);
#endif
	d_gdc_gfx->set_screen_width(SCREEN_WIDTH >> 3);
	
	// register event
	register_frame_event(this);
}

void DISPLAY::save_memsw()
{
	FILEIO *fio = new FILEIO();
	if(fio == NULL) return;
	if(fio->Fopen(create_local_path(_T("MEMSW.BIN")), FILEIO_WRITE_BINARY)) {
		if(fio->IsOpened()) {
			for(int i = 0; i < 16; i++) {
				fio->FputUint8(tvram[0x3fe0 + (i << 1)]);
			}
			fio->Fclose();
		}
	}
	delete fio;
}
	
void DISPLAY::init_memsw()
{
	for(int i = 0; i < 16; i++) {
		tvram[0x3fe0 + (i << 1)] = memsw_default[i];
//		tvram[0x3fe0 + (i << 1)] = 0x00;
	}
}

void DISPLAY::release()
{
	if((config.dipswitch & (1 << DIPSWITCH_POSITION_NOINIT_MEMSW)) == 0) {
		init_memsw();
	}
	save_memsw();
}

#if !defined(SUPPORT_HIRESO)
void DISPLAY::kanji_copy(uint8_t *dst, uint8_t *src, int from, int to)
{
	for(int i = from; i < to; i++) {
		uint8_t *p = src + 0x1800 + (0x60 * 32 * (i - 1));
		uint8_t *q = dst + 0x20000 + (i << 4);
		for(int j = 0x20; j < 0x80; j++) {
			for(int k = 0; k < 16; k++) {
				*(q + 0x800) = *(p + 16);
				*q++ = *p++;
			}
			p += 16;
			q += 0x1000 - 16;
		}
	}
}
#else
void DISPLAY::ank_copy(int code, uint8_t *pattern)
{
	uint16_t *dest = (uint16_t *)(font + ANK_FONT_OFS + FONT_SIZE * code);
	
	for(int i = 0; i < 24; i++) {
		dest[i] = (pattern[2 * i] << 8) | pattern[2 * i + 1];
	}
}

void DISPLAY::kanji_copy(int first, int second, uint8_t *pattern)
{
	uint16_t *dest_l = (uint16_t *)(font + FONT_SIZE * ((first - 0x20) | (second << 8)));
	uint16_t *dest_r = (uint16_t *)(font + FONT_SIZE * ((first - 0x20) | (second << 8)) + KANJI_2ND_OFS);
	
	for(int i = 0; i < 24; i++) {
		uint32_t p = (pattern[3 * i] << 16) | (pattern[3 * i + 1] << 8) | pattern[3 * i + 2];
		dest_l[i] = p >> 12;
		dest_r[i] = p << 2;
	}
}
#endif

void DISPLAY::reset()
{
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
	vram_disp_sel = 0x00;
	vram_draw_sel = 0x00;
#endif
	vram_disp_b = vram + VRAM_PLANE_ADDR_0;
	vram_disp_r = vram + VRAM_PLANE_ADDR_1;
	vram_disp_g = vram + VRAM_PLANE_ADDR_2;
#if defined(SUPPORT_16_COLORS)
	vram_disp_e = vram + VRAM_PLANE_ADDR_3;
#endif
	vram_draw   = vram + 0x00000;
	vram_bank = 0x00000;
#if defined(USE_MONITOR_TYPE)
	display_high = (config.monitor_type == 0) ? true : false; // OK?
#else
	display_high = false;
#endif
	crtv = 2;
	
	scroll[SCROLL_PL ] = 0;
	scroll[SCROLL_BL ] = 0x0f;
	scroll[SCROLL_CL ] = 0x10;
	scroll[SCROLL_SSL] = 0;
	scroll[SCROLL_SUR] = 0;
	scroll[SCROLL_SDR] = 24;
	
	memset(modereg1, 0, sizeof(modereg1));
#if defined(SUPPORT_16_COLORS)
	memset(modereg2, 0, sizeof(modereg2));
	if((config.dipswitch & (1 << DIPSWITCH_POSITION_GDC_FAST)) != 0) {
		modereg2[0x82 >> 1] = 1;
		modereg2[0x84 >> 1] = 1;
		d_gdc_chr->set_clock_freq(5000 * 1000);
		d_gdc_gfx->set_clock_freq(5000 * 1000);
//		write_signals(&output_gdc_freq, 0xff);
	} else {
		d_gdc_chr->set_clock_freq(2500 * 1000);
		d_gdc_gfx->set_clock_freq(2500 * 1000);
//		write_signals(&output_gdc_freq, 0x00);
	}
#endif
#if defined(SUPPORT_GRCG)
	grcg_mode = grcg_tile_ptr = 0;
	grcg_cg_mode = false;
	grcg_rw_mode = false;
	for(int i = 0; i < 4; i++) {
		grcg_plane_enabled[i] = true;
	}
#endif
#if defined(SUPPORT_EGC)
	is_use_egc = ((config.dipswitch & (1 << DIPSWITCH_POSITION_EGC)) != 0);
	enable_egc = false;
	egc_access = 0xfff0;
	egc_fgbg = 0x00ff;
	egc_ope = 0;
	egc_fg = 0;
	egc_mask.w = 0xffff;
	egc_bg = 0;
	egc_sft = 0;
	egc_leng = 0x000f;
	egc_lastvram.q = 0;
	egc_patreg.q = 0;
	egc_fgc.q = 0;
	egc_bgc.q = 0;
	egc_func = 0;
	egc_remain = 0;
	egc_stack = 0;
	egc_inptr = egc_buf;
	egc_outptr = egc_buf;
	egc_mask2.w = 0;
	egc_srcmask.w = 0;
	egc_srcbit = 0;
	egc_dstbit = 0;
	egc_sft8bitl = 0;
	egc_sft8bitr = 0;
	memset(egc_buf, 0, sizeof(egc_buf));
	egc_vram_src.q = 0;
	egc_vram_data.q = 0;
	egc_shift();
	egc_srcmask.w = 0xffff;
#endif
	
	save_memsw();
	font_code = 0;
	font_line = 0;
//	font_lr = 0;
	for(int i = 0x00; i < 0x08; i++) {
		bank_table[i] = 0x80000000; // Disable
	}
	for(int i = 0x08; i < 0x0a; i++) {
		bank_table[i] = 0x80000000; // Disable(Enable to Modify)
	}
	for(int i = 0x0a; i < 0x0c; i++) {
		bank_table[i] = i * 0x10000; // Enable(Enable to Modify)
	}
#if defined(SUPPORT_16_COLORS)
	for(int i = 0x0c; i < 0x0f; i++) {
		bank_table[i] = i * 0x10000; // Enable (Unable to Modify)
	}
#else
	for(int i = 0x0c; i < 0x0e; i++) {
		bank_table[i] = i * 0x10000; // Disable
	}
	bank_table[0x0e] = 0x80000000; // ToDo: Hi Reso
#endif
	bank_table[0x0f] = 0x80000000; // ToDo: Hi Reso
}

void DISPLAY::event_frame()
{
	if(crtv > 1) {
		// dont raise irq at first frame
		crtv--;
	} else if(crtv == 1) {
		d_pic->write_signal(SIG_I8259_CHIP0 | SIG_I8259_IR2, 1, 1);
		crtv = 0;
	}
}

void DISPLAY::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_DISPLAY98_SET_PAGE_A0:
		{
			data = data & 0x000e0000; // ToDo: Hi RESO
			if((data < 0x000a0000) || (data >= 0x000f0000)) data = 0x80000000;
			bank_table[0x0a] = data;
			bank_table[0x0b] = data + 0x00010000;
		}
		break;
	case SIG_DISPLAY98_SET_PAGE_80:
		{
			data = data & 0x000e0000; // ToDo: Hi RESO
			if((data < 0x000a0000) || (data >= 0x000f0000)) data = 0x80000000;
			bank_table[0x08] = data;
			bank_table[0x09] = data + 0x00010000;
		}
		break;
	case SIG_DISPLAY98_SET_BANK:
		// WIP: Still dummy.
		vram_bank = ((data & mask) != 0) ? 0x10000 : 0x00000;
		break;
	/*case SIG_DISPLAY98_HIGH_RESOLUTION:
		  {
		  display_high = ((data & mask) != 0);
		  printf("DISP MODE=%d\n", display_high);
		  }
		  break;
	*/
	default:
		break;
	}
}
void DISPLAY::write_io8(uint32_t addr, uint32_t data)
{
	uint8_t m_bak;
	switch(addr) {
	case 0x0064:
		crtv = 1;
		break;
	case 0x0068:
		switch((data >> 1) & 7) { // From MAME 0.208
			// Related information:
			/*
			  TODO: this is my best bet so far. Register 4 is annoying, the pattern seems to be:
			  Write to video FF register Graphic -> 00
			  Write to video FF register 200 lines -> 0x
			  Write to video FF register 200 lines -> 00
			  
			  where x is the current mode.
			*/
		case 1:
			b_gfx_ff = true;
			break;
		case 4:
			if(b_gfx_ff) {
				modereg1[(data >> 1) & 7] = data & 1;
				b_gfx_ff = false;
			}
			break;
		default:
			modereg1[(data >> 1) & 7] = data & 1;
			break;
		}
		modereg1[(data >> 1) & 7] = data & 1;
		break;
#if defined(SUPPORT_16_COLORS)
	case 0x006a:
//		modereg2[(data >> 1) & 127] = data & 1;
#if !defined(PC9821_VARIANTS)
		if((data & 0xf0) != 0) { // From MAME 0.208. Disable pages .
			m_bak = 0;
			modereg2[(data >> 1) & 127] = 0;
		} else { 
			m_bak = modereg2[(data >> 1) & 127];
			modereg2[(data >> 1) & 127] = data & 1;
		}
#else
		m_bak = modereg2[(data >> 1) & 127];
		modereg2[(data >> 1) & 127] = data & 1;
#endif
		if(modereg2[MODE2_EGC_WP] != 0) {
			if(is_use_egc) {
				enable_egc = (modereg2[MODE2_EGC]) ? true : false;
			} else {
				enable_egc = false;
				modereg2[MODE2_EGC] = 0;
			}
		}
		if(m_bak != modereg2[(data >> 1) & 127]) {
			if((data & 0xfe) == 0x82) {
				// ToDo: 006Eh: CMD=0000001nb
				if((modereg2[0x84 >> 1] != 0) && (modereg2[0x82 >> 1] != 0)) {
					d_gdc_chr->set_clock_freq(5000 * 1000);
					d_gdc_gfx->set_clock_freq(5000 * 1000);
					//write_signals(&output_gdc_freq, 0xff);
				} else if(modereg2[0x82 >> 1] == 0) {
					// ToDo: 006Eh: CMD=0000001nb
					d_gdc_chr->set_clock_freq(2500 * 1000);
					d_gdc_gfx->set_clock_freq(2500 * 1000);
					//write_signals(&output_gdc_freq, 0x00);
				}
			} else if((data & 0xfe) == 0x84) {
				if((data & 0x01) == 0) {
					d_gdc_chr->set_clock_freq(2500 * 1000);
					d_gdc_gfx->set_clock_freq(2500 * 1000);
					//write_signals(&output_gdc_freq, 0x00);
				}
			}
		}
		break;
#endif
#if !defined(SUPPORT_HIRESO)
	case 0x006c:
		border = (data >> 3) & 7;
		border_color = RGB_COLOR((border & 2) ? 0xff : 0, (border & 4) ? 0xff : 0, (border & 1) ? 0xff : 0);
//		border = (data >> 3) & 7;
		break;
	case 0x006e:
#if defined(_PC9801)
		border = (data >> 3) & 7;
		border_color = RGB_COLOR((border & 2) ? 0xff : 0, (border & 4) ? 0xff : 0, (border & 1) ? 0xff : 0);
#else
//		border = (data >> 3) & 7;
#endif
#if !defined(_PC9801)
		if(data & 1) {
			d_gdc_chr->set_horiz_freq(24830);
			d_gdc_gfx->set_horiz_freq(24830);
		} else {
			d_gdc_chr->set_horiz_freq(15750);
			d_gdc_gfx->set_horiz_freq(15750);
		}
#endif
		break;
#endif
	case 0x0070:
	case 0x0072:
	case 0x0074:
	case 0x0076:
	case 0x0078:
	case 0x007a:
		scroll[(addr >> 1) & 7] = data;
		break;
#if defined(SUPPORT_GRCG)
#if !defined(SUPPORT_HIRESO)
	case 0x007c:
#else
	case 0x00a4:
#endif
		grcg_mode = data;
		grcg_cg_mode = ((grcg_mode & GRCG_CG_MODE) != 0) ? true : false;
		grcg_rw_mode = ((grcg_mode & GRCG_RW_MODE) != 0) ? true : false;
		for(int i = 0; i < 4; i++) {
			grcg_plane_enabled[i] = ((grcg_mode & (1 << i)) == 0) ? true : false;
		}
		grcg_tile_ptr = 0;
		break;
#if !defined(SUPPORT_HIRESO)
	case 0x007e:
#else
	case 0x00a6:
#endif
		grcg_tile[grcg_tile_ptr] = data;
		grcg_tile_word[grcg_tile_ptr] = ((uint16_t)grcg_tile[grcg_tile_ptr]) | ((uint16_t)grcg_tile[grcg_tile_ptr] << 8);
		grcg_tile_ptr = (grcg_tile_ptr + 1) & 3;
		break;
#endif
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
	// vram select
	case 0x00a4:
		if(data & 1) {
			vram_disp_b = vram + 0x28000;
			vram_disp_r = vram + 0x30000;
			vram_disp_g = vram + 0x38000;
#if defined(SUPPORT_16_COLORS)
			vram_disp_e = vram + 0x20000;
#endif
		} else {
			vram_disp_b = vram + 0x08000;
			vram_disp_r = vram + 0x10000;
			vram_disp_g = vram + 0x18000;
#if defined(SUPPORT_16_COLORS)
			vram_disp_e = vram + 0x00000;
#endif
		}
		vram_disp_sel = data;
		break;
	case 0x00a6:
		if(data & 1) {
			vram_draw = vram + 0x20000;
		} else {
			vram_draw = vram + 0x00000;
		}
		vram_draw_sel = data;
		break;
#endif
	// palette
	case 0x00a8:
#if defined(SUPPORT_16_COLORS)
		if(modereg2[MODE2_16COLOR]) {
			anapal_sel = data & 0x0f;
			break;
		}
#endif
		digipal[0] = data;
		palette_gfx8[7] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		data >>= 4;
		palette_gfx8[3] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		break;
	case 0x00aa:
#if defined(SUPPORT_16_COLORS)
		if(modereg2[MODE2_16COLOR]) {
			anapal[anapal_sel][0] = (data & 0x0f) << 4;
			palette_gfx16[anapal_sel] = RGB_COLOR(anapal[anapal_sel][1], anapal[anapal_sel][0], anapal[anapal_sel][2]);
			break;
		}
#endif
		digipal[1] = data;
		palette_gfx8[5] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		data >>= 4;
		palette_gfx8[1] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		break;
	case 0x00ac:
#if defined(SUPPORT_16_COLORS)
		if(modereg2[MODE2_16COLOR]) {
			anapal[anapal_sel][1] = (data & 0x0f) << 4;
			palette_gfx16[anapal_sel] = RGB_COLOR(anapal[anapal_sel][1], anapal[anapal_sel][0], anapal[anapal_sel][2]);
			break;
		}
#endif
		digipal[2] = data;
		palette_gfx8[6] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		data >>= 4;
		palette_gfx8[2] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		break;
	case 0x00ae:
#if defined(SUPPORT_16_COLORS)
		if(modereg2[MODE2_16COLOR]) {
			anapal[anapal_sel][2] = (data & 0x0f) << 4;
			palette_gfx16[anapal_sel] = RGB_COLOR(anapal[anapal_sel][1], anapal[anapal_sel][0], anapal[anapal_sel][2]);
			break;
		}
#endif
		digipal[3] = data;
		palette_gfx8[4] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		data >>= 4;
		palette_gfx8[0] = RGB_COLOR((data & 2) ? 0xff : 0, (data & 4) ? 0xff : 0, (data & 1) ? 0xff : 0);
		break;
	// cg window
	case 0x00a1:
		font_code = (data << 8) | (font_code & 0xff);
		break;
	case 0x00a3:
		font_code = (font_code & 0xff00) | data;
		break;
	case 0x00a5:
//		font_line = data & 0x1f;
//		font_lr = ((~data) & 0x20) << 6;
		font_line = data;
		break;
	case 0x00a9:
		if((font_code & 0x7e) == 0x56) {
			uint16_t font_lr = ((~font_line) & 0x20) << 6;
			font[((font_code & 0x7f7f) << 4) + font_lr + (font_line & 0x0f)] = data;
		}
		break;
#if defined(SUPPORT_EGC)
	// egc
	case 0x04a0:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_access &= 0xff00;
			egc_access |= data;
		}
		break;
	case 0x04a1:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_access &= 0x00ff;
			egc_access |= data << 8;
		}
		break;
	case 0x04a2:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_fgbg &= 0xff00;
			egc_fgbg |= data;
		}
		break;
	case 0x04a3:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_fgbg &= 0x00ff;
			egc_fgbg |= data << 8;
		}
		break;
	case 0x04a4:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_ope &= 0xff00;
			egc_ope |= data;
		}
		break;
	case 0x04a5:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_ope &= 0x00ff;
			egc_ope |= data << 8;
		}
		break;
	case 0x04a6:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_fg &= 0xff00;
			egc_fg |= data;
//			egc_fgc.d[0] = *(uint32_t *)(&egc_maskword[data & 0x0f][0]);
//			egc_fgc.d[1] = *(uint32_t *)(&egc_maskword[data & 0x0f][2]);
			egc_fgc.d[0] = egc_maskdword[data & 0x0f][0];
			egc_fgc.d[1] = egc_maskdword[data & 0x0f][1];
		}
		break;
	case 0x04a7:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_fg &= 0x00ff;
			egc_fg |= data << 8;
		}
		break;
	case 0x04a8:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			if(!(egc_fgbg & 0x6000)) {
				egc_mask.b[0] = data;
			}
		}
		break;
	case 0x04a9:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			if(!(egc_fgbg & 0x6000)) {
				egc_mask.b[1] = data;
			}
		}
		break;
	case 0x04aa:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_bg &= 0xff00;
			egc_bg |= data;
//			egc_bgc.d[0] = *(uint32_t *)(&egc_maskword[data & 0x0f][0]);
//			egc_bgc.d[1] = *(uint32_t *)(&egc_maskword[data & 0x0f][2]);
			egc_bgc.d[0] = egc_maskdword[data & 0x0f][0];
			egc_bgc.d[1] = egc_maskdword[data & 0x0f][1];
		}
		break;
	case 0x04ab:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_bg &= 0x00ff;
			egc_bg |= data << 8;
		}
		break;
	case 0x04ac:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_sft &= 0xff00;
			egc_sft |= data;
			egc_shift();
			egc_srcmask.w = 0xffff;
		}
		break;
	case 0x04ad:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_sft &= 0x00ff;
			egc_sft |= data << 8;
			egc_shift();
			egc_srcmask.w = 0xffff;
		}
		break;
	case 0x04ae:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_leng &= 0xff00;
			egc_leng |= data;
			egc_shift();
			egc_srcmask.w = 0xffff;
		}
		break;
	case 0x04af:
		if((grcg_cg_mode) && enable_egc) {
//		if((grcg_mode & GRCG_CG_MODE) && modereg2[MODE2_EGC]) {
			egc_leng &= 0x00ff;
			egc_leng |= data << 8;
			egc_shift();
			egc_srcmask.w = 0xffff;
		}
		break;
#endif
	}
}

uint32_t DISPLAY::read_io8(uint32_t addr)
{
	switch(addr) {
#if defined(SUPPORT_GRCG)
#if !defined(SUPPORT_HIRESO)
	case 0x007c:
#else
	case 0x00a4:
#endif
		return grcg_mode; // From NP2 v0.83
		break;
#endif
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
	// vram select
	case 0x00a4:
		return vram_disp_sel;
	case 0x00a6:
		return vram_draw_sel;
#endif
	// palette
	case 0x00a8:
		return digipal[0];
	case 0x00aa:
		return digipal[1];
	case 0x00ac:
		return digipal[2];
	case 0x00ae:
		return digipal[3];
	// cg window
	case 0x00a1:
		return (font_code >> 8) & 0xff;
	case 0x00a3:
		return (font_code >> 0) & 0xff;
	case 0x00a5:
		return font_line;
	case 0x00a9:
		if((font_code & 0xff) >= 0x09 && (font_code & 0xff) < 0x0c) {
			uint16_t font_lr = ((~font_line) & 0x20) << 6;
			if(!font_lr) {
				return font[((font_code & 0x7f7f) << 4) + (font_line & 0x0f)];
			}
		} else if(font_code & 0xff00) {
			uint16_t font_lr = ((~font_line) & 0x20) << 6;
			return font[((font_code & 0x7f7f) << 4) + font_lr + (font_line & 0x0f)];
		} else if(!(font_line & 0x10)) {
			return font[0x80000 + (font_code << 4) + (font_line & 0x1f)];
		}
		return 0;
	}
	return 0xff;
}

//		CPU	GDC
//	B	A8000h	08000h
//	R	B0000h	10000h
//	G	B8000h	18000h
//	I	E0000h	00000h

// CPU memory bus

void DISPLAY::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	uint32_t idx = (addr & 0x000f0000) >> 16;
	if(bank_table[idx] >= 0x80000000) return;
	addr = bank_table[idx] | (addr & 0x0000ffff);
	
	if(TVRAM_ADDRESS <= addr && addr < (TVRAM_ADDRESS + 0x3fe2)) {
		tvram[addr - TVRAM_ADDRESS] = data;
	} else if((TVRAM_ADDRESS + 0x3fe2) <= addr && addr < (TVRAM_ADDRESS + 0x4000)) {
		// memory switch
		if(modereg1[MODE1_MEMSW]) {
			tvram[addr - TVRAM_ADDRESS] = data;
		}
	} else if((TVRAM_ADDRESS + 0x4000) <= addr && addr < (TVRAM_ADDRESS + 0x5000)) {
		if((font_code & 0x7e) == 0x56) {
#if !defined(SUPPORT_HIRESO)
			uint32_t low = 0x7fff0, high;
			uint8_t code = font_code & 0x7f;
			uint16_t lr = ((~font_line) & 0x20) << 6;
			if(!(font_code & 0xff00)) {
				high = 0x80000 + (font_code << 4);
				if(!modereg1[MODE1_FONTSEL]) {
					high += 0x2000;
				}
			} else {
				high = (font_code & 0x7f7f) << 4;
				if(code >= 0x56 && code < 0x58) {
					high += lr;
				} else if(code >= 0x09 && code < 0x0c) {
					if(lr) {
						high = low;
					}
				} else if((code >= 0x0c && code < 0x10) || (code >= 0x58 && code < 0x60)) {
					high += lr;
				} else {
					low = high;
					high += 0x800;
				}
			}
			if(addr & 1) {
				font[high + ((addr >> 1) & 0x0f)] = data;
			} else {
				font[low  + ((addr >> 1) & 0x0f)] = data;
			}
#else
			int line = (addr >> 1) & 31, shift = 0;
			bool is_kanji = false;
			uint32_t offset, pattern;
			
			if(!(font_code & 0xff00)) {
				if((addr & 0x30) == 0x30 || (addr & 0x40)) {
					return;
				}
				offset = ANK_FONT_OFS + FONT_SIZE * (font_code & 0xff) + line * 2;
//				pattern = (*(uint16_t *)(&font[offset])) & 0x3fff;
				pattern  = (font[offset + 0]       );
				pattern |= (font[offset + 1] & 0x3f) << 8;
			} else {
//				if((addr & 0x30) == 0x30 || ((addr & 0x40) && !(addr & 1))) {
				if((addr & 0x30) == 0x30) {
					return;
				}
				uint16_t lo = font_code & 0x7f;
				uint16_t hi = (font_code >> 8) & 0x7f;
				
				offset = FONT_SIZE * (lo | (hi << 8)) + line * 2;
//				pattern = (*(uint16_t *)(&font[offset])) & 0x3fff;
				pattern  = (font[offset + 0]       );
				pattern |= (font[offset + 1] & 0x3f) << 8;
				
				if(lo == 0x56 || lo == 0x57) {
					is_kanji = true;
				} else {
					uint16_t lo = font_code & 0xff;
					if(lo < 0x09 || lo >= 0x0c) {
						is_kanji = true;
					}
				}
				if(is_kanji) {
					pattern <<= 14;
//					pattern |= (*(uint16_t *)(&font[offset + KANJI_2ND_OFS])) & 0x3fff;
					pattern |= (font[offset + KANJI_2ND_OFS + 0]       );
					pattern |= (font[offset + KANJI_2ND_OFS + 1] & 0x3f) << 8;
				}
				shift += 2;
				if(addr & 0x40) shift += 16;
			}
			if(!(addr & 1)) shift += 8;
			pattern &= ~(0xff << shift);
			pattern |= data << shift;
			
			if(is_kanji) {
//				*(uint16_t *)(&font[offset                ]) = (pattern >> 14) & 0x3fff;
//				*(uint16_t *)(&font[offset + KANJI_2ND_OFS]) = (pattern >>  0) & 0x3fff;
				font[offset                 + 0] = (pattern >> 14);
				font[offset                 + 1] = (pattern >> 22) & 0x3f;
				font[offset + KANJI_2ND_OFS + 0] = (pattern >>  0);
				font[offset + KANJI_2ND_OFS + 1] = (pattern >>  8) & 0x3f;
			} else {
//				*(uint16_t *)(&font[offset]) = pattern & 0x3fff;
				font[offset + 0] = (pattern >> 0);
				font[offset + 1] = (pattern >> 8) & 0x3f;
			}
#endif
		}
#if !defined(SUPPORT_HIRESO)
	} else if(0xa8000 <= addr && addr < 0xc0000) {
		write_dma_io8(addr - 0xa0000, data);
#if defined(SUPPORT_16_COLORS)
	} else if(0xe0000 <= addr && addr < 0xe8000) {
		write_dma_io8(addr - 0xe0000, data);
#endif
#else
	} else if(0xc0000 <= addr && addr < 0xe0000) {
		write_dma_io8(addr - 0xc0000, data);
#endif
	}
}

void DISPLAY::write_memory_mapped_io16(uint32_t addr, uint32_t data)
{
	uint32_t idx = (addr & 0x000f0000) >> 16;
	if(bank_table[idx] >= 0x80000000) return;
	addr = bank_table[idx] | (addr & 0x0000ffff);
	
	addr = addr & 0x000fffff; // For 32bit
//	uint32_t naddr = (addr & 0xf8000) >> 12;
//	bool is_tvram = false;
	if(TVRAM_ADDRESS <= addr && addr < (TVRAM_ADDRESS + 0x5000)) {
		write_memory_mapped_io8(addr + 0, (data >> 0) & 0xff);
		write_memory_mapped_io8(addr + 1, (data >> 8) & 0xff);
#if !defined(SUPPORT_HIRESO)
	} else if(0xa8000 <= addr && addr < 0xc0000) {
		write_dma_io16(addr - 0xa0000, data);
#if defined(SUPPORT_16_COLORS)
	} else if(0xe0000 <= addr && addr < 0xe8000) {
		write_dma_io16(addr - 0xe0000, data);
#endif
#else
	} else if(0xc0000 <= addr && addr < 0xe0000) {
		write_dma_io16(addr - 0xc0000, data);
#endif
	}
}

uint32_t DISPLAY::read_memory_mapped_io8(uint32_t addr)
{
	uint32_t idx = (addr & 0x000f0000) >> 16;
	if(bank_table[idx] >= 0x80000000) return 0xff;
	addr = bank_table[idx] | (addr & 0x0000ffff);
	
	if(TVRAM_ADDRESS <= addr && addr < (TVRAM_ADDRESS + 0x2000)) {
		return tvram[addr - TVRAM_ADDRESS];
	} else if((TVRAM_ADDRESS + 0x2000) <= addr && addr < (TVRAM_ADDRESS + 0x4000)) {
		if(addr & 1) {
			return 0xff;
		}
		return tvram[addr - TVRAM_ADDRESS];
	} else if((TVRAM_ADDRESS + 0x4000) <= addr && addr < (TVRAM_ADDRESS + 0x5000)) {
#if !defined(SUPPORT_HIRESO)
		uint32_t low = 0x7fff0, high;
		uint8_t code = font_code & 0x7f;
		uint16_t lr = ((~font_line) & 0x20) << 6;
		if(!(font_code & 0xff00)) {
			high = 0x80000 + (font_code << 4);
			if(!modereg1[MODE1_FONTSEL]) {
				high += 0x2000;
			}
		} else {
			high = (font_code & 0x7f7f) << 4;
			if(code >= 0x56 && code < 0x58) {
				high += lr;
			} else if(code >= 0x09 && code < 0x0c) {
				if(lr) {
					high = low;
				}
			} else if((code >= 0x0c && code < 0x10) || (code >= 0x58 && code < 0x60)) {
				high += lr;
			} else {
				low = high;
				high += 0x800;
			}
		}
		if(addr & 1) {
			return font[high + ((addr >> 1) & 0x0f)];
		} else {
			return font[low  + ((addr >> 1) & 0x0f)];
		}
#else
		int line = (addr >> 1) & 31, shift = 0;
		bool is_kanji = false;
		uint32_t offset, pattern;
		
		if(!(font_code & 0xff00)) {
			if((addr & 0x30) == 0x30 || (addr & 0x40)) {
				return 0;
			}
			offset = ANK_FONT_OFS + FONT_SIZE * (font_code & 0xff) + line * 2;
//			pattern = (*(uint16_t *)(&font[offset])) & 0x3fff;
			pattern  = (font[offset + 0]       );
			pattern |= (font[offset + 1] & 0x3f) << 8;
		} else {
//			if((addr & 0x30) == 0x30 || ((addr & 0x40) && !(addr & 1))) {
			if((addr & 0x30) == 0x30) {
				return 0;
			}
			uint16_t lo = font_code & 0x7f;
			uint16_t hi = (font_code >> 8) & 0x7f;
			
			offset = FONT_SIZE * (lo | (hi << 8)) + line * 2;
//			pattern = (*(uint16_t *)(&font[offset])) & 0x3fff;
			pattern  = (font[offset + 0]       );
			pattern |= (font[offset + 1] & 0x3f) << 8;
			
			if(lo == 0x56 || lo == 0x57) {
				is_kanji = true;
			} else {
				uint16_t lo = font_code & 0xff;
				if(lo < 0x09 || lo >= 0x0c) {
					is_kanji = true;
				}
			}
			if(is_kanji) {
				pattern <<= 14;
//				pattern |= (*(uint16_t *)(&font[offset + KANJI_2ND_OFS])) & 0x3fff;
				pattern |= (font[offset + KANJI_2ND_OFS + 0]       );
				pattern |= (font[offset + KANJI_2ND_OFS + 1] & 0x3f) << 8;
			}
			shift += 2;
			if(addr & 0x40) shift += 16;
		}
		if(!(addr & 1)) shift += 8;
		return (pattern >> shift) & 0xff;
#endif
#if !defined(SUPPORT_HIRESO)
	} else if(0xa8000 <= addr && addr < 0xc0000) {
		return read_dma_io8(addr - 0xa0000);
#if defined(SUPPORT_16_COLORS)
	} else if(0xe0000 <= addr && addr < 0xe8000) {
		return read_dma_io8(addr - 0xe0000);
#endif
#else
	} else if(0xc0000 <= addr && addr < 0xe0000) {
		return read_dma_io8(addr - 0xc0000);
#endif
	}
	return 0xff;
}

uint32_t DISPLAY::read_memory_mapped_io16(uint32_t addr)
{
	uint32_t idx = (addr & 0x000f0000) >> 16;
	if(bank_table[idx] >= 0x80000000) return 0xffff;
	addr = bank_table[idx] | (addr & 0x0000ffff);
	
	if(TVRAM_ADDRESS <= addr && addr < (TVRAM_ADDRESS + 0x5000)) {
		return read_memory_mapped_io8(addr) | (read_memory_mapped_io8(addr + 1) << 8);
#if !defined(SUPPORT_HIRESO)
	} else if(0xa8000 <= addr && addr < 0xc0000) {
		return read_dma_io16(addr - 0xa0000);
#if defined(SUPPORT_16_COLORS)
	} else if(0xe0000 <= addr && addr < 0xe8000) {
		return read_dma_io16(addr - 0xe0000);
#endif
#else
	} else if(0xc0000 <= addr && addr < 0xe0000) {
		return read_dma_io16(addr - 0xc0000);
#endif
	}
	return 0xffff;
}

// Graphic GDC bus

void DISPLAY::write_dma_io8(uint32_t addr, uint32_t data)
{
#if defined(SUPPORT_GRCG)
	if(grcg_cg_mode) {
//	if(grcg_mode & GRCG_CG_MODE) {
#if defined(SUPPORT_EGC)
		if(enable_egc) {
//		if(modereg2[MODE2_EGC]) {
//			out_debug_log(_T("ADDR = %08X"), addr);
			egc_writeb(addr, data);
		} else
#endif
//		out_debug_log(_T("ADDR = %08X"), addr);
		grcg_writeb(addr, data);
		return;
	}
#endif
#if !defined(SUPPORT_HIRESO)
	vram_draw[addr & 0x1ffff] = data;
#else
	if(!(grcg_mode & GRCG_PLANE_0)) {
		vram_draw[(addr & 0x1ffff) | VRAM_PLANE_ADDR_0] = data;
	}
	if(!(grcg_mode & GRCG_PLANE_1)) {
		vram_draw[(addr & 0x1ffff) | VRAM_PLANE_ADDR_1] = data;
	}
	if(!(grcg_mode & GRCG_PLANE_2)) {
		vram_draw[(addr & 0x1ffff) | VRAM_PLANE_ADDR_2] = data;
	}
	if(!(grcg_mode & GRCG_PLANE_3)) {
		vram_draw[(addr & 0x1ffff) | VRAM_PLANE_ADDR_3] = data;
	}
#endif
}

void DISPLAY::write_dma_io16(uint32_t addr, uint32_t data)
{
#if defined(SUPPORT_GRCG)
	if(grcg_cg_mode) {
//	if(grcg_mode & GRCG_CG_MODE) {
#if defined(SUPPORT_EGC)
		if(enable_egc) {
//		if(modereg2[MODE2_EGC]) {
			egc_writew(addr, data);
		} else
#endif
		grcg_writew(addr, data);
		return;
	}
#endif
	if((addr &= 0x1ffff) == 0x1ffff) {
		pair16_t d;
		d.w = (uint16_t)data;
#if !defined(SUPPORT_HIRESO)
		vram_draw[0x1ffff] = d.b.l;
		vram_draw[0x00000] = d.b.h;
#else
		if(!(grcg_mode & GRCG_PLANE_0)) {
			vram_draw[0x1ffff | VRAM_PLANE_ADDR_0] = d.b.l;
			vram_draw[0x00000 | VRAM_PLANE_ADDR_0] = d.b.h;
		}
		if(!(grcg_mode & GRCG_PLANE_1)) {
			vram_draw[0x1ffff | VRAM_PLANE_ADDR_1] = d.b.l;
			vram_draw[0x00000 | VRAM_PLANE_ADDR_1] = d.b.h;
		}
		if(!(grcg_mode & GRCG_PLANE_2)) {
			vram_draw[0x1ffff | VRAM_PLANE_ADDR_2] = d.b.l;
			vram_draw[0x00000 | VRAM_PLANE_ADDR_2] = d.b.h;
		}
		if(!(grcg_mode & GRCG_PLANE_3)) {
			vram_draw[0x1ffff | VRAM_PLANE_ADDR_3] = d.b.l;
			vram_draw[0x00000 | VRAM_PLANE_ADDR_3] = d.b.h;
		}
#endif
	} else {
#if !defined(SUPPORT_HIRESO)
		#ifdef __BIG_ENDIAN__
			vram_draw_writew(addr, data);
		#else
			*(uint16_t *)(&vram_draw[addr]) = data;
		#endif
#else
		if(!(grcg_mode & GRCG_PLANE_0)) {
			#ifdef __BIG_ENDIAN__
				vram_draw_writew(addr | VRAM_PLANE_ADDR_0, data);
			#else
				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]) = data;
			#endif
		}
		if(!(grcg_mode & GRCG_PLANE_1)) {
			#ifdef __BIG_ENDIAN__
				vram_draw_writew(addr | VRAM_PLANE_ADDR_1, data);
			#else
				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]) = data;
			#endif
		}
		if(!(grcg_mode & GRCG_PLANE_2)) {
			#ifdef __BIG_ENDIAN__
				vram_draw_writew(addr | VRAM_PLANE_ADDR_2, data);
			#else
				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]) = data;
			#endif
		}
		if(!(grcg_mode & GRCG_PLANE_3)) {
			#ifdef __BIG_ENDIAN__
				vram_draw_writew(addr | VRAM_PLANE_ADDR_3, data);
			#else
				*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]) = data;
			#endif
		}
#endif
	}
}

uint32_t DISPLAY::read_dma_io8(uint32_t addr)
{
#if defined(SUPPORT_GRCG)
	if(grcg_cg_mode) {
//	if(grcg_mode & GRCG_CG_MODE) {
#if defined(SUPPORT_EGC)
		if(enable_egc) {
//		if(modereg2[MODE2_EGC]) {
			return egc_readb(addr);
		}
#endif
		return grcg_readb(addr);
	}
#endif
#if !defined(SUPPORT_HIRESO)
	return vram_draw[addr & 0x1ffff];
#else
	int plane = (grcg_mode >> 4) & 3;
	return vram_draw[(addr & 0x1ffff) | (0x20000 * plane)];
#endif
}

uint32_t DISPLAY::read_dma_io16(uint32_t addr)
{
#if defined(SUPPORT_GRCG)
	if(grcg_cg_mode) {
//	if(grcg_mode & GRCG_CG_MODE) {
#if defined(SUPPORT_EGC)
		if(enable_egc) {
//		if(modereg2[MODE2_EGC]) {
			return egc_readw(addr);
		}
#endif
		return grcg_readw(addr);
	}
#endif
	if((addr &= 0x1ffff) == 0x1ffff) {
		pair16_t d;
		d.w = 0;
#if !defined(SUPPORT_HIRESO)
		d.b.l = vram_draw[0x1ffff];
		d.b.h = vram_draw[0x00000];
#else
		int plane = (grcg_mode >> 4) & 3;
		d.b.l = vram_draw[0x1ffff | (0x20000 * plane)];
		d.b.h = vram_draw[0x00000 | (0x20000 * plane)];
#endif
		return (uint32_t)(d.w);
	} else {
#if !defined(SUPPORT_HIRESO)
		#ifdef __BIG_ENDIAN__
			return vram_draw_readw(addr);
		#else
			return *(uint16_t *)(&vram_draw[addr]);
		#endif
#else
		int plane = (grcg_mode >> 4) & 3;
		#ifdef __BIG_ENDIAN__
			return vram_draw_readw(addr | (0x20000 * plane));
		#else
			return *(uint16_t *)(&vram_draw[addr | (0x20000 * plane)]);
		#endif
#endif
	}
}

#ifdef __BIG_ENDIAN__
inline void DISPLAY::vram_draw_writew(uint32_t addr, uint32_t data)
{
	vram_draw[addr    ] = (data     ) & 0xff;
	vram_draw[addr + 1] = (data >> 8) & 0xff;
}

inline uint32_t DISPLAY::vram_draw_readw(uint32_t addr)
{
	uint32_t val;
	val  = vram_draw[addr    ];
	val |= vram_draw[addr + 1] << 8;
	return val;
}
#endif

// GRCG

#if defined(SUPPORT_GRCG)
void DISPLAY::grcg_writeb(uint32_t addr1, uint32_t data)
{
	uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
	
	if(grcg_mode & GRCG_RW_MODE) {
		// RMW
		if(!(grcg_mode & GRCG_PLANE_0)) {
			vram_draw[addr | VRAM_PLANE_ADDR_0] &= ~data;
			vram_draw[addr | VRAM_PLANE_ADDR_0] |= grcg_tile[0] & data;
		}
		if(!(grcg_mode & GRCG_PLANE_1)) {
			vram_draw[addr | VRAM_PLANE_ADDR_1] &= ~data;
			vram_draw[addr | VRAM_PLANE_ADDR_1] |= grcg_tile[1] & data;
		}
		if(!(grcg_mode & GRCG_PLANE_2)) {
			vram_draw[addr | VRAM_PLANE_ADDR_2] &= ~data;
			vram_draw[addr | VRAM_PLANE_ADDR_2] |= grcg_tile[2] & data;
		}
		if(!(grcg_mode & GRCG_PLANE_3)) {
			vram_draw[addr | VRAM_PLANE_ADDR_3] &= ~data;
			vram_draw[addr | VRAM_PLANE_ADDR_3] |= grcg_tile[3] & data;
		}
	} else {
		// TDW
		if(!(grcg_mode & GRCG_PLANE_0)) {
			vram_draw[addr | VRAM_PLANE_ADDR_0] = grcg_tile[0];
		}
		if(!(grcg_mode & GRCG_PLANE_1)) {
			vram_draw[addr | VRAM_PLANE_ADDR_1] = grcg_tile[1];
		}
		if(!(grcg_mode & GRCG_PLANE_2)) {
			vram_draw[addr | VRAM_PLANE_ADDR_2] = grcg_tile[2];
		}
		if(!(grcg_mode & GRCG_PLANE_3)) {
			vram_draw[addr | VRAM_PLANE_ADDR_3] = grcg_tile[3];
		}
	}
}

void DISPLAY::grcg_writew(uint32_t addr1, uint32_t data)
{
	if(addr1 & 1) {
		grcg_writeb(addr1 + 0, (data >> 0) & 0xff);
		grcg_writeb(addr1 + 1, (data >> 8) & 0xff);
	} else {
		uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
		
		if(grcg_mode & GRCG_RW_MODE) {
			// RMW
			if(!(grcg_mode & GRCG_PLANE_0)) {
				#ifdef __BIG_ENDIAN__
					uint16_t p = vram_draw_readw(addr | VRAM_PLANE_ADDR_0);
					p &= ~data;
					p |= grcg_tile_word[0] & data;
					vram_draw_writew(addr | VRAM_PLANE_ADDR_0, p);
				#else
					uint16_t *p = (uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]);
					*p &= ~data;
					*p |= grcg_tile_word[0] & data;
				#endif
			}
			if(!(grcg_mode & GRCG_PLANE_1)) {
				#ifdef __BIG_ENDIAN__
					uint16_t p = vram_draw_readw(addr | VRAM_PLANE_ADDR_1);
					p &= ~data;
					p |= grcg_tile_word[1] & data;
					vram_draw_writew(addr | VRAM_PLANE_ADDR_1, p);
				#else
					uint16_t *p = (uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]);
					*p &= ~data;
					*p |= grcg_tile_word[1] & data;
				#endif
			}
			if(!(grcg_mode & GRCG_PLANE_2)) {
				#ifdef __BIG_ENDIAN__
					uint16_t p = vram_draw_readw(addr | VRAM_PLANE_ADDR_2);
					p &= ~data;
					p |= grcg_tile_word[2] & data;
					vram_draw_writew(addr | VRAM_PLANE_ADDR_2, p);
				#else
					uint16_t *p = (uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]);
					*p &= ~data;
					*p |= grcg_tile_word[2] & data;
				#endif
			}
			if(!(grcg_mode & GRCG_PLANE_3)) {
				#ifdef __BIG_ENDIAN__
					uint16_t p = vram_draw_readw(addr | VRAM_PLANE_ADDR_3);
					p &= ~data;
					p |= grcg_tile_word[3] & data;
					vram_draw_writew(addr | VRAM_PLANE_ADDR_3, p);
				#else
					uint16_t *p = (uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]);
					*p &= ~data;
					*p |= grcg_tile_word[3] & data;
				#endif
			}
		} else {
			// TDW
			if(!(grcg_mode & GRCG_PLANE_0)) {
				#ifdef __BIG_ENDIAN__
					vram_draw_writew(addr | VRAM_PLANE_ADDR_0, grcg_tile_word[0]);
				#else
					*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]) = grcg_tile_word[0];
				#endif
			}
			if(!(grcg_mode & GRCG_PLANE_1)) {
				#ifdef __BIG_ENDIAN__
					vram_draw_writew(addr | VRAM_PLANE_ADDR_1, grcg_tile_word[1]);
				#else
					*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]) = grcg_tile_word[1];
				#endif
			}
			if(!(grcg_mode & GRCG_PLANE_2)) {
				#ifdef __BIG_ENDIAN__
					vram_draw_writew(addr | VRAM_PLANE_ADDR_2, grcg_tile_word[2]);
				#else
					*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]) = grcg_tile_word[2];
				#endif
			}
			if(!(grcg_mode & GRCG_PLANE_3)) {
				#ifdef __BIG_ENDIAN__
					vram_draw_writew(addr | VRAM_PLANE_ADDR_3, grcg_tile_word[3]);
				#else
					*(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]) = grcg_tile_word[3];
				#endif
			}
		}
	}
}

uint32_t DISPLAY::grcg_readb(uint32_t addr1)
{
	if(grcg_mode & GRCG_RW_MODE) {
		// VRAM
#if !defined(SUPPORT_HIRESO)
		return vram_draw[addr1 & 0x1ffff];
#else
		int plane = (grcg_mode >> 4) & 3;
		return vram_draw[(addr1 & 0x1ffff) | (0x20000 * plane)];
#endif
	} else {
		// TCR
		uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
		uint8_t data = 0;
		
		if(!(grcg_mode & GRCG_PLANE_0)) {
			data |= vram_draw[addr | VRAM_PLANE_ADDR_0] ^ grcg_tile[0];
		}
		if(!(grcg_mode & GRCG_PLANE_1)) {
			data |= vram_draw[addr | VRAM_PLANE_ADDR_1] ^ grcg_tile[1];
		}
		if(!(grcg_mode & GRCG_PLANE_2)) {
			data |= vram_draw[addr | VRAM_PLANE_ADDR_2] ^ grcg_tile[2];
		}
		if(!(grcg_mode & GRCG_PLANE_3)) {
			data |= vram_draw[addr | VRAM_PLANE_ADDR_3] ^ grcg_tile[3];
		}
		return data ^ 0xff;
	}
}

uint32_t DISPLAY::grcg_readw(uint32_t addr1)
{
	if(addr1 & 1) {
		return grcg_readb(addr1) | (grcg_readb(addr1 + 1) << 8);
	} else {
		if(grcg_mode & GRCG_RW_MODE) {
			// VRAM
#if !defined(SUPPORT_HIRESO)
			#ifdef __BIG_ENDIAN__
				return vram_draw_readw(addr1 & 0x1ffff);
			#else
				return *(uint16_t *)(&vram_draw[addr1 & 0x1ffff]);
			#endif
#else
			int plane = (grcg_mode >> 4) & 3;
			#ifdef __BIG_ENDIAN__
				return vram_draw_readw((addr1 & 0x1ffff) | (0x20000 * plane));
			#else
				return *(uint16_t *)(&vram_draw[(addr1 & 0x1ffff) | (0x20000 * plane)]);
			#endif
#endif
		} else {
			// TCR
			uint32_t addr = addr1 & VRAM_PLANE_ADDR_MASK;
			uint16_t data = 0;
			
			if(!(grcg_mode & GRCG_PLANE_0)) {
				#ifdef __BIG_ENDIAN__
					data |= vram_draw_readw(addr | VRAM_PLANE_ADDR_0) ^ grcg_tile_word[0];
				#else
					data |= *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_0]) ^ grcg_tile_word[0];
				#endif
			}
			if(!(grcg_mode & GRCG_PLANE_1)) {
				#ifdef __BIG_ENDIAN__
					data |= vram_draw_readw(addr | VRAM_PLANE_ADDR_1) ^ grcg_tile_word[1];
				#else
					data |= *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_1]) ^ grcg_tile_word[1];
				#endif
			}
			if(!(grcg_mode & GRCG_PLANE_2)) {
				#ifdef __BIG_ENDIAN__
					data |= vram_draw_readw(addr | VRAM_PLANE_ADDR_2) ^ grcg_tile_word[2];
				#else
					data |= *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_2]) ^ grcg_tile_word[2];
				#endif
			}
			if(!(grcg_mode & GRCG_PLANE_3)) {
				#ifdef __BIG_ENDIAN__
					data |= vram_draw_readw(addr | VRAM_PLANE_ADDR_3) ^ grcg_tile_word[3];
				#else
					data |= *(uint16_t *)(&vram_draw[addr | VRAM_PLANE_ADDR_3]) ^ grcg_tile_word[3];
				#endif
			}
			return data ^ 0xffff;
		}
	}
}
#endif

// EGC based on Neko Project 2 and QEMU/9821
// -> moved to egc.cpp and egc_inline.h .

void DISPLAY::draw_screen()
{
	bool gdc_chr_start = d_gdc_chr->get_start();
	bool gdc_gfx_start = d_gdc_gfx->get_start();
	int _height = d_gdc_chr->read_signal(SIG_UPD7220_DISP_HEIGHT) << 4;
	int _height2 = d_gdc_gfx->read_signal(SIG_UPD7220_DISP_HEIGHT);
	if(_height < _height2) _height = _height2;
	if(modereg1[MODE1_DISP] && (gdc_chr_start || gdc_gfx_start)) {
		if(gdc_chr_start) {
			draw_chr_screen();
		} else {
			memset(screen_chr, 0, sizeof(screen_chr));
		}
		if(gdc_gfx_start) {
			draw_gfx_screen();
		} else {
			memset(screen_gfx, 0, sizeof(screen_gfx));
		}
		int _width = d_gdc_gfx->read_signal(SIG_UPD7220_DISP_WIDTH);
		_width <<= 4;
//		_width -= 16;
		if(_width > SCREEN_WIDTH) _width = SCREEN_WIDTH;
		for(int y = 0; y < SCREEN_HEIGHT; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			uint8_t *src_chr;
#if defined(USE_MONITOR_TYPE) && !defined(SUPPORT_HIRESO)
			if(display_high) {
				src_chr = screen_chr[y];
			} else {
				src_chr = screen_chr[y >> 1];
			}
#else
			src_chr = screen_chr[y];
#endif
#if defined(SUPPORT_16_COLORS)
			if(!modereg2[MDOE2_TXTSHIFT]) {
				src_chr++;
			}
#endif
			if(y >= _height) {
				for(int x = _width; x < SCREEN_WIDTH; x++) {
					dest[x] = border_color;
				}
				continue;
			}
			uint8_t *src_gfx = screen_gfx[y];
			
#if defined(SUPPORT_16_COLORS)
			if(!modereg2[MODE2_16COLOR]) {
#endif
				for(int x = 0; x < _width; x++) {
					uint8_t chr = src_chr[x];
					dest[x] = chr ? palette_chr[chr & 7] : palette_gfx8[src_gfx[x] & 7];
				}
				// ToDo: Variable width
				for(int x = _width; x < SCREEN_WIDTH; x++) {
					uint8_t chr = src_chr[x];
					dest[x] = (chr) ? palette_chr[chr & 7] : border_color;
				}
#if defined(SUPPORT_16_COLORS)
			} else {
				for(int x = 0; x < _width; x++) {
					uint8_t chr = src_chr[x];
					dest[x] = chr ? palette_chr[chr & 7] : palette_gfx16[src_gfx[x]];
				}
				// ToDo: Variable width
				for(int x = _width; x < SCREEN_WIDTH; x++) {
					uint8_t chr = src_chr[x];
					dest[x] = (chr) ? palette_chr[chr & 7] : border_color;
				}
			}
#endif
		}
	} else {
		for(int y = 0; y < SCREEN_HEIGHT; y++) {
			scrntype_t *dest = emu->get_screen_buffer(y);
			memset(dest, 0, SCREEN_WIDTH * sizeof(scrntype_t));
		}
	}
	emu->set_vm_screen_lines(SCREEN_HEIGHT);
	emu->screen_skip_line(false);
}
void DISPLAY::draw_chr_screen()
{
	// scroll registers
	int _height = d_gdc_chr->read_signal(SIG_UPD7220_HEIGHT);
	int _width  = d_gdc_chr->read_signal(SIG_UPD7220_PITCH);
	int _width2  = d_gdc_gfx->read_signal(SIG_UPD7220_PITCH);
	int _height2 = d_gdc_gfx->read_signal(SIG_UPD7220_HEIGHT) >> 4;
//	if(_width > _width2) _width = _width2;
//	if(_height > _height2) _height = _height2;
	if(_height > (SCREEN_HEIGHT >> 4)) _height = SCREEN_HEIGHT >> 4;
	int pl = scroll[SCROLL_PL] & 31;
	if(pl) {
		pl = 32 - pl;
	}
	int bl = scroll[SCROLL_BL] + pl + 1;
	int cl = scroll[SCROLL_CL];
	int ssl = scroll[SCROLL_SSL];
	int sur = scroll[SCROLL_SUR] & 31;
	if(sur) {
		sur = 32 - sur;
	}
	int sdr = scroll[SCROLL_SDR] + 1;
	
	// address from gdc
	uint32_t gdc_addr[30][80] = {0};
	// ToDo: Will Support 30lines project.
//	printf("PITCH=%d HEIGHT=%d\n", _width, _height);
	for(int i = 0, ytop = 0; i < 4; i++) {
		uint32_t ra = ra_chr[i * 4];
		ra |= ra_chr[i * 4 + 1] << 8;
		ra |= ra_chr[i * 4 + 2] << 16;
		ra |= ra_chr[i * 4 + 3] << 24;
		uint32_t sad = (ra << 1) & 0x1fff;
		int len = (ra >> 20) & 0x3ff;
//		if(!len) len = 1024;
//		printf("#%d: %04X %d\n", i, sad, len);
		int lcount = ytop / bl;
		for(int y = ytop; y < (ytop + len) && (lcount < (_height  > 30) ? 30 : _height); y += bl, lcount++) {
//		for(int y = ytop; (y < (ytop + len)) && (y < 30); y++) {
			if(lcount >= (_height )) break;
			for(int x = 0; x < ((_width > 80) ? 80 : _width); x++) {
				gdc_addr[lcount][x] = sad;
				sad = (sad + 2) & 0x1fff;
			}
		}
		ytop += len;
//		if(ytop >= (_height << 4)) break;
	}
	
	uint32_t *addr = &gdc_addr[0][0];
//	uint32_t *addr2 = addr + 160 * (sur + sdr);
	uint32_t *addr2 = addr + 80 * (sur + sdr);
	
	uint32_t cursor_addr = d_gdc_chr->cursor_addr(0x1fff);
	int cursor_top = d_gdc_chr->cursor_top();
	int cursor_bottom = d_gdc_chr->cursor_bottom();
#if defined(SUPPORT_HIRESO)
	cursor_top <<= 1;
	cursor_bottom <<= 1;
#endif
	bool attr_blink = d_gdc_chr->attr_blink();
	
	// render
	int ysur = bl * sur;
	int ysdr = bl * (sur + sdr);
	int xofs = modereg1[MODE1_COLUMN] ? (FONT_WIDTH * 2) : FONT_WIDTH;
	int addrofs = modereg1[MODE1_COLUMN] ? 2 : 1;
	
	memset(screen_chr, 0, sizeof(screen_chr));
	
	_width <<= 4;
	_height <<= 4;
	if(_width > SCREEN_WIDTH) _width = SCREEN_WIDTH;
	//out_debug_log("WxH: %dx%d", _width, _height);
	if(_width  < 0) _width = 0;
	static const uint32_t __vramsize = SCREEN_HEIGHT * (SCREEN_WIDTH >> 3);
	uint32_t of = 0;
	
	for(int y = 0, cy = 0, ytop = 0; y < _height && cy < 25; y += bl, cy++) {
		uint32_t gaiji1st = 0, last = 0, offset;
		int kanji2nd = 0;
		if(y == ysur) {
			ytop = y;
			y -= ssl;
			ysur = _height;
		}
		if(y >= ysdr) {
			y = ytop = ysdr;
			addr = addr2;
			ysdr = _height;
		}
		for(int x = 0, cx = 0; x < _width && cx < 80; x += xofs, cx++) {
			uint16_t code = tvram[(*addr)] | (tvram[(*addr) + 1] << 8);
			uint8_t attr = tvram[(*addr) | 0x2000];
			uint8_t color = (attr & ATTR_COL) ? (attr >> 5) : 8;
			bool cursor = ((*addr) == cursor_addr);
			addr += addrofs;
			if(kanji2nd) {
				kanji2nd = 0;
				offset = last + KANJI_2ND_OFS;
			} else if(code & 0xff00) {
				uint16_t lo = code & 0x7f;
				uint16_t hi = (code >> 8) & 0x7f;
				offset = FONT_SIZE * (lo | (hi << 8));
				if(lo == 0x56 || lo == 0x57) {
					offset += gaiji1st;
					gaiji1st = gaiji1st ? 0 : KANJI_2ND_OFS;
				} else {
					uint16_t lo = code & 0xff;
					if(lo < 0x09 || lo >= 0x0c) {
						kanji2nd = 1;
					}
					gaiji1st = 0;
				}
			} else {
				offset = ANK_FONT_OFS + FONT_SIZE * (code & 0xff);
#if !defined(SUPPORT_HIRESO)
				if((attr & ATTR_VL) && modereg1[MODE1_ATRSEL]) {
					offset += FONT_SIZE * 0x100;
				}
				if(!modereg1[MODE1_FONTSEL]) {
					offset += FONT_SIZE * 0x200;
				}
#endif
				gaiji1st = 0;
			}
			last = offset;
			for(int l = 0; l < bl; l++) {
				int yy;
				yy =  y + l + pl;
				if(yy >= ytop && yy < _height) {
					uint8_t *dest = &screen_chr[yy][x];
#if !defined(SUPPORT_HIRESO)
					uint8_t pattern;
#if defined(USE_MONITOR_TYPE) && !defined(SUPPORT_HIRESO)
					if(!(display_high) && ((code >= 0x100) || (kanji2nd))) {
						pattern = (l < cl && l < (FONT_HEIGHT >> 1)) ? (font[offset + (l << 1)] | font[offset + (l << 1) + 1]) : 0;
					} else 
#endif
					pattern  = (l < cl && l < FONT_HEIGHT) ? font[offset + l] : 0;
#else
					uint16_t pattern = (l < cl && l < FONT_HEIGHT) ? (font[offset + l * 2] | (font[offset + l * 2 + 1] << 8)) : 0;
#endif
					if(!(attr & ATTR_ST)) {
						pattern = 0;
					} else if(((attr & ATTR_BL) && attr_blink) || (attr & ATTR_RV)) {
						pattern = ~pattern;
					}
					if((attr & ATTR_UL) && l == (FONT_HEIGHT - 1)) {
#if !defined(SUPPORT_HIRESO)
						pattern = 0xff;
#else
						pattern = 0x3ff;
#endif
					}
					if((attr & ATTR_VL) && !modereg1[MODE1_ATRSEL]) {
#if !defined(SUPPORT_HIRESO)
						pattern |= 0x08;
#else
						pattern |= 0x40;
#endif
					}
					if(cursor && l >= cursor_top && l < cursor_bottom) {
						pattern = ~pattern;
					}
					if(modereg1[MODE1_COLUMN]) {
#if !defined(SUPPORT_HIRESO)
						if(pattern & 0x80) dest[ 0] = dest[ 1] = color;
						if(pattern & 0x40) dest[ 2] = dest[ 3] = color;
						if(pattern & 0x20) dest[ 4] = dest[ 5] = color;
						if(pattern & 0x10) dest[ 6] = dest[ 7] = color;
						if(pattern & 0x08) dest[ 8] = dest[ 9] = color;
						if(pattern & 0x04) dest[10] = dest[11] = color;
						if(pattern & 0x02) dest[12] = dest[13] = color;
						if(pattern & 0x01) dest[14] = dest[15] = color;
#else
						if(pattern & 0x2000) dest[ 0] = dest[ 1] = color;
						if(pattern & 0x1000) dest[ 2] = dest[ 3] = color;
						if(pattern & 0x0800) dest[ 4] = dest[ 5] = color;
						if(pattern & 0x0400) dest[ 6] = dest[ 7] = color;
						if(pattern & 0x0200) dest[ 8] = dest[ 9] = color;
						if(pattern & 0x0100) dest[10] = dest[11] = color;
						if(pattern & 0x0080) dest[12] = dest[13] = color;
						if(pattern & 0x0040) dest[14] = dest[15] = color;
						if(pattern & 0x0020) dest[16] = dest[17] = color;
						if(pattern & 0x0010) dest[18] = dest[19] = color;
						if(pattern & 0x0008) dest[20] = dest[21] = color;
						if(pattern & 0x0004) dest[22] = dest[23] = color;
						if(pattern & 0x0002) dest[24] = dest[25] = color;
						if(pattern & 0x0001) dest[26] = dest[27] = color;
#endif
					} else {
#if !defined(SUPPORT_HIRESO)
						if(pattern & 0x80) dest[0] = color;
						if(pattern & 0x40) dest[1] = color;
						if(pattern & 0x20) dest[2] = color;
						if(pattern & 0x10) dest[3] = color;
						if(pattern & 0x08) dest[4] = color;
						if(pattern & 0x04) dest[5] = color;
						if(pattern & 0x02) dest[6] = color;
						if(pattern & 0x01) dest[7] = color;
#else
						if(pattern & 0x2000) dest[ 0] = color;
						if(pattern & 0x1000) dest[ 1] = color;
						if(pattern & 0x0800) dest[ 2] = color;
						if(pattern & 0x0400) dest[ 3] = color;
						if(pattern & 0x0200) dest[ 4] = color;
						if(pattern & 0x0100) dest[ 5] = color;
						if(pattern & 0x0080) dest[ 6] = color;
						if(pattern & 0x0040) dest[ 7] = color;
						if(pattern & 0x0020) dest[ 8] = color;
						if(pattern & 0x0010) dest[ 9] = color;
						if(pattern & 0x0008) dest[10] = color;
						if(pattern & 0x0004) dest[11] = color;
						if(pattern & 0x0002) dest[12] = color;
						if(pattern & 0x0001) dest[13] = color;
#endif
					}
				}
			}
			of++;
			if(of >= __vramsize) break;
		}
		if(of >= __vramsize) break;
	}
}

void DISPLAY::draw_gfx_screen()
{
	// address from gdc
	int _height = d_gdc_gfx->read_signal(SIG_UPD7220_HEIGHT) << 4;
	int _width  = d_gdc_gfx->read_signal(SIG_UPD7220_PITCH);
	
	uint32_t gdc_addr[480][SCREEN_WIDTH >> 3] = {0}; // Dragon Buster.
	
	if(_height < 0) _height = 0;
	if(_height > 480) _height = 480;
	_width <<= 4;
	int _width2 = SCREEN_WIDTH - _width;;
	if(_width  < 0) _width = 0;
	if(_width > SCREEN_WIDTH) _width = SCREEN_WIDTH;
	if(_width2 < 0) _width2 = 0;
	//out_debug_log("WxH: %dx%d", _width, _height);
	int ytop = 0;
	for(int i = 0; i < 4; i++) {
		uint32_t ra = ra_gfx[i * 4];
		ra |= ra_gfx[i * 4 + 1] << 8;
		ra |= ra_gfx[i * 4 + 2] << 16;
		ra |= ra_gfx[i * 4 + 3] << 24;
		uint32_t sad = (ra << 1) & VRAM_PLANE_ADDR_MASK;
		int len = (ra >> 20) & 0x3ff;
		
		for(int y = ytop; y < (ytop + len) && y < _height; y++) {
			for(int x = 0; x < (_width >> 3); x++) {
				gdc_addr[y][x] = sad;
				sad = (sad + 1) & VRAM_PLANE_ADDR_MASK;
			}
//			for(int x = (_width >> 3); x < (SCREEN_WIDTH >> 3); x++) {	
//				gdc_addr[y][x] = 0;
//			}
		}
		if((ytop += len) >= _height) break;
	}
	if(ytop < SCREEN_HEIGHT) {
		// Madou Monogatari 1-2-3
		uint32_t ra = ra_gfx[0];
		ra |= ra_gfx[1] << 8;
		ra |= ra_gfx[2] << 16;
		ra |= ra_gfx[3] << 24;
		uint32_t sad = (ra << 1) & VRAM_PLANE_ADDR_MASK;
		
		for(int y = 0; y < SCREEN_HEIGHT; y++) {
			for(int x = 0; x < (SCREEN_WIDTH >> 3); x++) {
				gdc_addr[y][x] = sad;
				sad = (sad + 1) & VRAM_PLANE_ADDR_MASK;
			}
		}
	}
	uint32_t *addr = &gdc_addr[0][0];
	uint8_t *dest = &screen_gfx[0][0];

	static const uint32_t __vramsize = SCREEN_HEIGHT * (SCREEN_WIDTH >> 3);
	uint32_t of = 0;
	for(int y = 0; y < _height; y++) {
		for(int x = 0; x < _width; x += 8) {
			uint8_t b = vram_disp_b[(*addr)];
			uint8_t r = vram_disp_r[(*addr)];
			uint8_t g = vram_disp_g[(*addr)];
#if defined(SUPPORT_16_COLORS)
			uint8_t e = vram_disp_e[(*addr)];
#else
			uint8_t e = 0;
#endif
			addr++;
			
			*dest++ = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5) | ((e & 0x80) >> 4);
			*dest++ = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4) | ((e & 0x40) >> 3);
			*dest++ = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3) | ((e & 0x20) >> 2);
			*dest++ = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2) | ((e & 0x10) >> 1);
			*dest++ = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1) | ((e & 0x08)     );
			*dest++ = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04)     ) | ((e & 0x04) << 1);
			*dest++ = ((b & 0x02) >> 1) | ((r & 0x02)     ) | ((g & 0x02) << 1) | ((e & 0x02) << 2);
			*dest++ = ((b & 0x01)     ) | ((r & 0x01) << 1) | ((g & 0x01) << 2) | ((e & 0x01) << 3);
			of++;
			if(of >= __vramsize) break;
		}
		for(int x = 0; x < _width2; x += 8) {
			*dest++ = 0;
			*dest++ = 0;
			*dest++ = 0;
			*dest++ = 0;
			*dest++ = 0;
			*dest++ = 0;
			*dest++ = 0;
			*dest++ = 0;
			addr++;
			of++;
			if(of >= __vramsize) break;
		}
		if(((cs_gfx[0] & 0x1f) == 1) || !(display_high)){
			// 200 line
			if(modereg1[MODE1_200LINE]) {
				//memset(dest, 0, 640);
				if(config.scan_line) {
					memset(dest, 0, SCREEN_WIDTH);
				} else {
					my_memcpy(dest, dest - SCREEN_WIDTH, SCREEN_WIDTH);
				}					
			} else {
				//my_memcpy(dest, dest - 640, 640);
				my_memcpy(dest, dest - SCREEN_WIDTH, SCREEN_WIDTH);
			}
			dest += 640;
			of += 80;
			y++;
			if(y >= SCREEN_HEIGHT) break; // Temp: Dragon Buster.
			if(of >= __vramsize) break;
		}
		if(of >= __vramsize) break;
		if(y >= SCREEN_HEIGHT) break; // Temp: Dragon Buster.
	}
}

#define STATE_VERSION	4

bool DISPLAY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(tvram, sizeof(tvram), 1);
	state_fio->StateArray(vram, sizeof(vram), 1);
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
	state_fio->StateValue(vram_disp_sel);
	state_fio->StateValue(vram_draw_sel);
#endif
	state_fio->StateArray(palette_gfx8, sizeof(palette_gfx8), 1);
	state_fio->StateArray(digipal, sizeof(digipal), 1);
#if defined(SUPPORT_16_COLORS)
	state_fio->StateArray(palette_gfx16, sizeof(palette_gfx16), 1);
	state_fio->StateArray(&anapal[0][0], sizeof(anapal), 1);
	state_fio->StateValue(anapal_sel);
#endif
	state_fio->StateValue(crtv);
	state_fio->StateArray(scroll, sizeof(scroll), 1);
	state_fio->StateArray(modereg1, sizeof(modereg1), 1);
#if defined(SUPPORT_16_COLORS)
	state_fio->StateArray(modereg2, sizeof(modereg2), 1);
#endif
#if defined(SUPPORT_GRCG)
	state_fio->StateValue(grcg_mode);
	state_fio->StateValue(grcg_tile_ptr);
	state_fio->StateArray(grcg_tile, sizeof(grcg_tile), 1);
#endif
#if defined(SUPPORT_EGC)
	state_fio->StateValue(egc_access);
	state_fio->StateValue(egc_fgbg);
	state_fio->StateValue(egc_ope);
	state_fio->StateValue(egc_fg);
	state_fio->StateValue(egc_mask.w);
	state_fio->StateValue(egc_bg);
	state_fio->StateValue(egc_sft);
	state_fio->StateValue(egc_leng);
	state_fio->StateValue(egc_lastvram.q);
	state_fio->StateValue(egc_patreg.q);
	state_fio->StateValue(egc_fgc.q);
	state_fio->StateValue(egc_bgc.q);
	state_fio->StateValue(egc_func);
	state_fio->StateValue(egc_remain);
	state_fio->StateValue(egc_stack);
	if(loading) {
		int inptr_ofs = state_fio->FgetInt32_LE();
		int outptr_ofs = state_fio->FgetInt32_LE();
		egc_inptr = egc_buf + inptr_ofs;
		egc_outptr = egc_buf + outptr_ofs;
	} else {
		int inptr_ofs = egc_inptr - egc_buf;
		int outptr_ofs = egc_outptr - egc_buf;
		state_fio->FputInt32_LE(inptr_ofs);
		state_fio->FputInt32_LE(outptr_ofs);
	}
	state_fio->StateValue(egc_mask2.w);
	state_fio->StateValue(egc_srcmask.w);
	state_fio->StateValue(egc_srcbit);
	state_fio->StateValue(egc_dstbit);
	state_fio->StateValue(egc_sft8bitl);
	state_fio->StateValue(egc_sft8bitr);
	state_fio->StateArray(egc_buf, sizeof(egc_buf), 1);
	state_fio->StateValue(egc_vram_src.q);
	state_fio->StateValue(egc_vram_data.q);
#endif
	state_fio->StateValue(font_code);
	state_fio->StateValue(font_line);
//	state_fio->StateValue(font_lr);
	state_fio->StateArray(bank_table, sizeof(bank_table), 1);
	
	// post process
	if(loading) {
 
#if defined(SUPPORT_16_COLORS) && defined(SUPPORT_EGC)
		is_use_egc = ((config.dipswitch & (1 << DIPSWITCH_POSITION_EGC)) != 0);
		enable_egc = ((is_use_egc) && (modereg2[MODE2_EGC] != 0)) ? true : false;
#endif		
#if defined(SUPPORT_2ND_VRAM) && !defined(SUPPORT_HIRESO)
		if(vram_disp_sel & 1) {
			vram_disp_b = vram + 0x28000;
			vram_disp_r = vram + 0x30000;
			vram_disp_g = vram + 0x38000;
#if defined(SUPPORT_16_COLORS)
			vram_disp_e = vram + 0x20000;
#endif
		} else {
			vram_disp_b = vram + 0x08000;
			vram_disp_r = vram + 0x10000;
			vram_disp_g = vram + 0x18000;
#if defined(SUPPORT_16_COLORS)
			vram_disp_e = vram + 0x00000;
#endif
		}
		if(vram_draw_sel & 1) {
			vram_draw = vram + 0x20000;
		} else {
			vram_draw = vram + 0x00000;
		}
#endif
#if defined(SUPPORT_GRCG)
		grcg_cg_mode = ((grcg_mode & GRCG_CG_MODE) != 0) ? true : false;
		grcg_rw_mode = ((grcg_mode & GRCG_RW_MODE) != 0) ? true : false;
		for(int i = 0; i < 4; i++) {
			grcg_tile_word[i] = ((uint16_t)grcg_tile[i]) | ((uint16_t)grcg_tile[i] << 8);
		}
#endif
	}
	return true;
}
}
