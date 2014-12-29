/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Origin : X1EMU by KM (kanji rom)
	         X-millenium by Yui (ank16 patch)
	Author : Takeda.Toshiya
	Date   : 2009.03.14-

	[ display ]
*/

#include "display.h"
#ifdef _X1TURBO_FEATURE
#include "../hd46505.h"
#endif
#include "../i8255.h"
#include "../../fileio.h"

#ifdef _X1TURBOZ
#define AEN	((zmode1 & 0x80) != 0)
#define APEN	((zmode2 & 0x80) != 0)
#define APRD	((zmode2 & 0x08) != 0)
#endif

// from X-millenium

static const uint16 ANKFONT7f_af[0x21 * 8] = {
	0x0000, 0x3000, 0x247f, 0x6c24, 0x484c, 0xce4b, 0x0000, 0x0000,

	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080, 0x8080,
	0xc0c0, 0xc0c0, 0xc0c0, 0xc0c0, 0xc0c0, 0xc0c0, 0xc0c0, 0xc0c0,
	0xe0e0, 0xe0e0, 0xe0e0, 0xe0e0, 0xe0e0, 0xe0e0, 0xe0e0, 0xe0e0,
	0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0,
	0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8,
	0xfcfc, 0xfcfc, 0xfcfc, 0xfcfc, 0xfcfc, 0xfcfc, 0xfcfc, 0xfcfc,
	0xfefe, 0xfefe, 0xfefe, 0xfefe, 0xfefe, 0xfefe, 0xfefe, 0xfefe,
	0x0101, 0x0202, 0x0404, 0x0808, 0x1010, 0x2020, 0x4040, 0x8080,

	0x0000, 0x0000, 0x0000, 0x0000, 0x00ff, 0x0000, 0x0000, 0x0000,
	0x1010, 0x1010, 0x1010, 0x1010, 0x1010, 0x1010, 0x1010, 0x1010,
	0x1010, 0x1010, 0x1010, 0x1010, 0x00ff, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x10ff, 0x1010, 0x1010, 0x1010,
	0x1010, 0x1010, 0x1010, 0x1010, 0x10f0, 0x1010, 0x1010, 0x1010,
	0x1010, 0x1010, 0x1010, 0x1010, 0x101f, 0x1010, 0x1010, 0x1010,
	0x1010, 0x1010, 0x1010, 0x1010, 0x10ff, 0x1010, 0x1010, 0x1010,
	0x0000, 0x0000, 0x0000, 0x0000, 0x10f0, 0x1010, 0x1010, 0x1010,
	0x1010, 0x1010, 0x1010, 0x1010, 0x00f0, 0x0000, 0x0000, 0x0000,
	0x1010, 0x1010, 0x1010, 0x1010, 0x001f, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x101f, 0x1010, 0x1010, 0x1010,
	0x0000, 0x0000, 0x0000, 0x0000, 0x4080, 0x2020, 0x1010, 0x1010,
	0x1010, 0x1010, 0x0810, 0x0408, 0x0003, 0x0000, 0x0000, 0x0000,
	0x1010, 0x1010, 0x2010, 0x4020, 0x0080, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0403, 0x0808, 0x1010, 0x1010,
	0x8080, 0x4040, 0x2020, 0x1010, 0x0808, 0x0404, 0x0202, 0x0101
};

static const uint16 ANKFONTe0_ff[0x20 * 8] = {
	0x0000, 0x7e3c, 0xffff, 0xdbdb, 0xffff, 0xe7db, 0x7eff, 0x003c,
	0x0000, 0x423c, 0x8181, 0xa5a5, 0x8181, 0x99a5, 0x4281, 0x003c,
	0x0000, 0x3810, 0x7c7c, 0xfefe, 0xfefe, 0x106c, 0x7c38, 0x0000,
	0x0000, 0x6c00, 0xfefe, 0xfefe, 0xfefe, 0x7c7c, 0x1038, 0x0000,
	0x0000, 0x1010, 0x3838, 0x7c7c, 0x7cfe, 0x387c, 0x1038, 0x0010,
	0x0000, 0x3810, 0x7c7c, 0x5438, 0xfefe, 0x6cfe, 0x7c10, 0x0000,
	0x0101, 0x0303, 0x0707, 0x0f0f, 0x1f1f, 0x3f3f, 0x7f7f, 0xffff,
	0x8080, 0xc0c0, 0xe0e0, 0xf0f0, 0xf8f8, 0xfcfc, 0xfefe, 0xffff,
	0x8181, 0x4242, 0x2424, 0x1818, 0x1818, 0x2424, 0x4242, 0x8181,
	0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0,
	0x0f0f, 0x0f0f, 0x0f0f, 0x0f0f, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0f0f, 0x0f0f, 0x0f0f, 0x0f0f,
	0x0f0f, 0x0f0f, 0x0f0f, 0x0f0f, 0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0,
	0xf0f0, 0xf0f0, 0xf0f0, 0xf0f0, 0x0f0f, 0x0f0f, 0x0f0f, 0x0f0f,
	0x81ff, 0x8181, 0x8181, 0x8181, 0x8181, 0x8181, 0x8181, 0xff81,

	0x55aa, 0x55aa, 0x55aa, 0x55aa, 0x55aa, 0x55aa, 0x55aa, 0x55aa,
	0x1000, 0x1010, 0xf01e, 0x1010, 0x1010, 0x1010, 0x7e10, 0x00c0,
	0x1000, 0x2418, 0x7c42, 0x1090, 0x781c, 0x5410, 0xfe54, 0x0000,
	0x1000, 0x1010, 0xfe10, 0x1010, 0x3030, 0x565c, 0x9090, 0x0010,
	0x1000, 0x1210, 0xf412, 0x3034, 0x5030, 0x9654, 0x1090, 0x0000,
	0x0800, 0x8808, 0x5292, 0x1454, 0x2020, 0x5020, 0x465c, 0x0040,
	0x0000, 0xe23c, 0x8282, 0x82fa, 0x7a82, 0x4242, 0x0044, 0x0000,
	0x0000, 0x443c, 0x8242, 0xf282, 0x8282, 0x8282, 0x3844, 0x0000,
	0x0800, 0x5e18, 0xa468, 0xe4be, 0xbea4, 0xb2a2, 0x0a5a, 0x0002,
	0x0000, 0x2628, 0x4042, 0xe23c, 0x2222, 0x4222, 0x4442, 0x0004,
	0x0800, 0x0808, 0xda7a, 0x5454, 0x46e4, 0xe442, 0x08c4, 0x0000,
	0x0000, 0x7e40, 0x8848, 0x28be, 0x2828, 0x3e28, 0x08e8, 0x0008,
	0x0000, 0x723c, 0x9252, 0x9292, 0x8292, 0x84fc, 0x8484, 0x0000,
	0x0000, 0x1010, 0x2010, 0x2020, 0x6040, 0x8c50, 0x8286, 0x0000,
	0x0000, 0x4040, 0x784e, 0x88c0, 0x388e, 0x0848, 0x7e08, 0x0000,
	0x0000, 0x7c00, 0x0000, 0x0000, 0x10fe, 0x1010, 0x1010, 0x0010
};

void DISPLAY::initialize()
{
	scanline = config.scan_line;
	
	// load rom images
	FILEIO* fio = new FILEIO();
	
	// ank8 (8x8)
	if(fio->Fopen(emu->bios_path(_T("ANK8.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	} else if(fio->Fopen(emu->bios_path(_T("FNT0808.X1")), FILEIO_READ_BINARY)) {
		// xmillenium rom
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
	}
	
	// ank16 (8x16)
	if(fio->Fopen(emu->bios_path(_T("ANK16.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, 0x1000, 1);
		fio->Fclose();
	} else if(fio->Fopen(emu->bios_path(_T("FNT0816.X1")), FILEIO_READ_BINARY)) {
		// xmillenium rom
		fio->Fread(kanji, 0x1000, 1);
		fio->Fclose();
	}
	memcpy(kanji + 0x7f * 16, ANKFONT7f_af, sizeof(ANKFONT7f_af));
	memcpy(kanji + 0xe0 * 16, ANKFONTe0_ff, sizeof(ANKFONTe0_ff));
	
	// kanji (16x16)
	if(fio->Fopen(emu->bios_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji + 0x1000, 0x4ac00, 1);
		fio->Fclose();
	} else if(fio->Fopen(emu->bios_path(_T("FNT1616.X1")), FILEIO_READ_BINARY)) {
		// xmillenium rom
		fio->Fread(kanji + 0x1000, 0x4ac00, 1);
		fio->Fclose();
	}
	for(int ofs = 0x1000; ofs < 0x4bc00; ofs += 32) {
		// LRLR.. -> LL..RR..
		uint8 buf[32];
		for(int i = 0; i < 16; i++) {
			buf[i     ] = kanji[ofs + i * 2    ];
			buf[i + 16] = kanji[ofs + i * 2 + 1];
		}
		memcpy(kanji + ofs, buf, 32);
	}
	delete fio;
	
	// create pc palette
	for(int i = 0; i < 8; i++) {
		palette_pc[i    ] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);	// text
		palette_pc[i + 8] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);	// cg
	}
	
	// initialize regs
	pal[0] = 0xaa;
	pal[1] = 0xcc;
	pal[2] = 0xf0;
	priority = 0;
	update_pal();
	column40 = true;
	
	memset(vram_t, 0, sizeof(vram_t));
	memset(vram_a, 0, sizeof(vram_a));
#ifdef _X1TURBO_FEATURE
	memset(vram_k, 0, sizeof(vram_k));
#endif
	memset(pcg_b, 0, sizeof(pcg_b));
	memset(pcg_r, 0, sizeof(pcg_r));
	memset(pcg_g, 0, sizeof(pcg_g));
#ifdef _X1TURBO_FEATURE
	memset(gaiji_b, 0, sizeof(gaiji_b));
	memset(gaiji_r, 0, sizeof(gaiji_r));
	memset(gaiji_g, 0, sizeof(gaiji_g));
#endif
	
	// register event
	register_frame_event(this);
	register_vline_event(this);
}

void DISPLAY::reset()
{
#ifdef _X1TURBO_FEATURE
	mode1 = 0;//3;
	mode2 = 0;
	hireso = true;
#endif
#ifdef _X1TURBOZ
	for(int i = 0; i < 8; i++) {
		palette_pc[i    ] = RGB_COLOR((i & 2) ? 255 : 0, (i & 4) ? 255 : 0, (i & 1) ? 255 : 0);	// text
		ztpal[i] = ((i & 1) ? 0x03 : 0) | ((i & 2) ? 3 : 0x0c) | ((i & 4) ? 0x30 : 0);
	}
	for(int g = 0; g < 16; g++) {
		for(int r = 0; r < 16; r++) {
			for(int b = 0; b < 16; b++) {
				int num = b + r * 16 + g * 256;
				zpal[num].b = b;
				zpal[num].r = r;
				zpal[num].g = g;
				palette_pc[num + 16] = RGB_COLOR((r * 255) / 15, (g * 255) / 15, (b * 255) / 15);
			}
		}
	}
	zmode1 = zpriority = zscroll = zmode2 = 0;
	zpal_num = 0;
#endif
	cur_line = cur_code = 0;
	vblank_clock = 0;
	
	kaddr = kofs = kflag = 0;
	kanji_ptr = &kanji[0];
}

void DISPLAY::update_config()
{
	scanline = config.scan_line;
}

void DISPLAY::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff00) {
	case 0x0e00:
		write_kanji(addr, data);
		break;
	case 0x1000:
#ifdef _X1TURBOZ
		if(AEN && APEN && !APRD) {
			int num = ((data >> 4) & 0x0f) | ((addr << 4) & 0xff0);
			zpal[num].b = data & 0x0f;
			palette_pc[num + 16] = RGB_COLOR((zpal[num].r * 255) / 15, (zpal[num].g * 255) / 15, (zpal[num].b * 255) / 15);
		} else if(AEN && APEN && APRD) {
			zpal_num = ((data >> 4) & 0x0f) | ((addr << 4) & 0xff0);
		} else if(!AEN) {
#endif
			pal[0] = data;
			update_pal();
#ifdef _X1TURBOZ
		}
#endif
		break;
	case 0x1100:
#ifdef _X1TURBOZ
		if(AEN && APEN && !APRD) {
			int num = ((data >> 4) & 0x0f) | ((addr << 4) & 0xff0);
			zpal[num].r = data & 0x0f;
			palette_pc[num + 16] = RGB_COLOR((zpal[num].r * 255) / 15, (zpal[num].g * 255) / 15, (zpal[num].b * 255) / 15);
//		} else if(AEN && APEN && APRD) {
//			zpal_num = ((data >> 4) & 0x0f) | ((addr << 4) & 0xff0);
		} else if(!AEN) {
#endif
			pal[1] = data;
			update_pal();
#ifdef _X1TURBOZ
		}
#endif
		break;
	case 0x1200:
#ifdef _X1TURBOZ
		if(AEN && APEN && !APRD) {
			int num = ((data >> 4) & 0x0f) | ((addr << 4) & 0xff0);
			zpal[num].g = data & 0x0f;
			palette_pc[num + 16] = RGB_COLOR((zpal[num].r * 255) / 15, (zpal[num].g * 255) / 15, (zpal[num].b * 255) / 15);
//		} else if(AEN && APEN && APRD) {
//			zpal_num = ((data >> 4) & 0x0f) | ((addr << 4) & 0xff0);
		} else if(!AEN) {
#endif
			pal[2] = data;
			update_pal();
#ifdef _X1TURBOZ
		}
#endif
		break;
	case 0x1300:
		priority = data;
		update_pal();
		break;
	case 0x1500:
		get_cur_pcg(addr);
		pcg_b[cur_code][cur_line] = data;
#ifdef _X1TURBO_FEATURE
		gaiji_b[cur_code >> 1][(cur_line << 1) | (cur_code & 1)] = data;
#endif
		break;
	case 0x1600:
		get_cur_pcg(addr);
		pcg_r[cur_code][cur_line] = data;
#ifdef _X1TURBO_FEATURE
		gaiji_r[cur_code >> 1][(cur_line << 1) | (cur_code & 1)] = data;
#endif
		break;
	case 0x1700:
		get_cur_pcg(addr);
		pcg_g[cur_code][cur_line] = data;
#ifdef _X1TURBO_FEATURE
		gaiji_g[cur_code >> 1][(cur_line << 1) | (cur_code & 1)] = data;
#endif
		break;
#ifdef _X1TURBO_FEATURE
	case 0x1f00:
		switch(addr) {
#ifdef _X1TURBOZ
		case 0x1fb0:
			zmode1 = data;
			break;
		case 0x1fb1:
		case 0x1fb2:
		case 0x1fb3:
		case 0x1fb4:
		case 0x1fb5:
		case 0x1fb6:
		case 0x1fb7:
			ztpal[addr & 7] = data;
			palette_pc[addr & 7] = RGB_COLOR((((data >> 2) & 3) * 255) / 3, (((data >> 4) & 3) * 255) / 3, (((data >> 0) & 3) * 255) / 3);
			break;
		case 0x1fc0:
			zpriority = data;
			break;
		case 0x1fc4:
			zscroll = data;
			break;
		case 0x1fc5:
			zmode2 = data;
			break;
#endif
		case 0x1fd0:
//			if((mode1 & 1) != (data & 1)) {
				d_crtc->set_horiz_freq((data & 1) ? 24860 : 15980);
//			}
			mode1 = data;
//			hireso = !((mode1 & 3) == 0 || (mode1 & 3) == 2);
			break;
		case 0x1fe0:
			mode2 = data;
			update_pal();
			break;
		}
		break;
#endif
	case 0x2000:
	case 0x2100:
	case 0x2200:
	case 0x2300:
	case 0x2400:
	case 0x2500:
	case 0x2600:
	case 0x2700:
		vram_a[addr & 0x7ff] = data;
		break;
	case 0x2800:
	case 0x2900:
	case 0x2a00:
	case 0x2b00:
	case 0x2c00:
	case 0x2d00:
	case 0x2e00:
	case 0x2f00:
		vram_a[addr & 0x7ff] = data; // mirror
		break;
	case 0x3000:
	case 0x3100:
	case 0x3200:
	case 0x3300:
	case 0x3400:
	case 0x3500:
	case 0x3600:
	case 0x3700:
		vram_t[addr & 0x7ff] = data;
		break;
	case 0x3800:
	case 0x3900:
	case 0x3a00:
	case 0x3b00:
	case 0x3c00:
	case 0x3d00:
	case 0x3e00:
	case 0x3f00:
#ifdef _X1TURBO_FEATURE
		vram_k[addr & 0x7ff] = data;
#else
		vram_t[addr & 0x7ff] = data; // mirror
#endif
		break;
	}
}

uint32 DISPLAY::read_io8(uint32 addr)
{
	switch(addr & 0xff00) {
	case 0x0e00:
		return read_kanji(addr);
#ifdef _X1TURBOZ
	case 0x1000:
		if(AEN && APEN && APRD) {
			return zpal[zpal_num].b;
		}
		break;
	case 0x1100:
		if(AEN && APEN && APRD) {
			return zpal[zpal_num].r;
		}
		break;
	case 0x1200:
		if(AEN && APEN && APRD) {
			return zpal[zpal_num].g;
		}
		break;
#endif
//	case 0x1300:
//		return priority;
	case 0x1400:
		return get_cur_font(addr);
	case 0x1500:
		get_cur_pcg(addr);
		return pcg_b[cur_code][cur_line];
	case 0x1600:
		get_cur_pcg(addr);
		return pcg_r[cur_code][cur_line];
	case 0x1700:
		get_cur_pcg(addr);
		return pcg_g[cur_code][cur_line];
#ifdef _X1TURBOZ
	case 0x1f00:
		switch(addr) {
		case 0x1fb0:
			return zmode1;
		case 0x1fb1:
		case 0x1fb2:
		case 0x1fb3:
		case 0x1fb4:
		case 0x1fb5:
		case 0x1fb6:
		case 0x1fb7:
			return ztpal[addr & 7];
		case 0x1fc0:
			return zpriority;
		case 0x1fc4:
			return zscroll;
		case 0x1fc5:
			return zmode2;
		case 0x1fd0:
			return mode1;
		case 0x1fe0:
			return mode2;
		}
		break;
#endif
	case 0x2000:
	case 0x2100:
	case 0x2200:
	case 0x2300:
	case 0x2400:
	case 0x2500:
	case 0x2600:
	case 0x2700:
		return vram_a[addr & 0x7ff];
	case 0x2800:
	case 0x2900:
	case 0x2a00:
	case 0x2b00:
	case 0x2c00:
	case 0x2d00:
	case 0x2e00:
	case 0x2f00:
		return vram_a[addr & 0x7ff]; // mirror
	case 0x3000:
	case 0x3100:
	case 0x3200:
	case 0x3300:
	case 0x3400:
	case 0x3500:
	case 0x3600:
	case 0x3700:
		return vram_t[addr & 0x7ff];
	case 0x3800:
	case 0x3900:
	case 0x3a00:
	case 0x3b00:
	case 0x3c00:
	case 0x3d00:
	case 0x3e00:
	case 0x3f00:
#ifdef _X1TURBO_FEATURE
		return vram_k[addr & 0x7ff];
#else
		return vram_t[addr & 0x7ff]; // mirror
#endif
	}
	return 0xff;
}

void DISPLAY::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_DISPLAY_VBLANK) {
		if(!(data & mask)) {
			// enter vblank
			vblank_clock = current_clock();
		}
	} else if(id == SIG_DISPLAY_COLUMN40) {
		column40 = ((data & mask) != 0);
	} else if(id == SIG_DISPLAY_DETECT_VBLANK) {
		// hack: cpu detects vblank
		vblank_clock = current_clock();
	}
}

void DISPLAY::event_frame()
{
	cblink = (cblink + 1) & 0x3f;
	
	// update crtc parameters
	ch_height = (regs[9] & 0x1f) + 1;
	hz_total = regs[0] + 1;
	hz_disp = regs[1];
	vt_disp = regs[6] & 0x7f;
	st_addr = (regs[12] << 8) | regs[13];
	
#ifdef _X1TURBO_FEATURE
	int vt_total = ((regs[4] & 0x7f) + 1) * ch_height + (regs[5] & 0x1f);
	hireso = (vt_total > 400);
#endif
}

void DISPLAY::event_vline(int v, int clock)
{
#ifdef _X1TURBO_FEATURE
	if(hireso) {
		if(v < 400) {
			draw_line(v);
		}
	} else {
#endif
		if(v < 200) {
			draw_line(v);
		}
#ifdef _X1TURBO_FEATURE
	}
	// restart cpu after pcg/cgrom is accessed
	d_cpu->write_signal(SIG_CPU_BUSREQ, 0, 0);
#endif
}

void DISPLAY::update_pal()
{
	uint8 pal2[8];
	for(int i = 0; i < 8; i++) {
		uint8 bit = 1 << i;
		pal2[i] = ((pal[0] & bit) ? 1 : 0) | ((pal[1] & bit) ? 2 : 0) | ((pal[2] & bit) ? 4 : 0) | 8;
	}
#ifdef _X1TURBO_FEATURE
	if(mode2 & 0x10) pal2[0] = 8;
	if(mode2 & 0x20) pal2[1] = 8;
#endif
	for(int c = 0; c < 8; c++) {
		for(int t = 0; t < 8; t++) {
			if(priority & (1 << c)) {
				pri[c][t] = pal2[c];
			} else if(t) {
#ifdef _X1TURBO_FEATURE
				pri[c][t] = ((mode2 & 8) && (mode2 & 7) == t) ? 0 : t;
#else
				pri[c][t] = t;
#endif
			} else {
				pri[c][t] = pal2[c];
			}
		}
	}
}

uint8 DISPLAY::get_cur_font(uint32 addr)
{
#ifdef _X1TURBO_FEATURE
	if(mode1 & 0x20) {
		// wait next raster
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		
		// from X1EMU
		uint16 vaddr;
		if(!(vram_a[0x7ff] & 0x20)) {
			vaddr = 0x7ff;
		} else if(!(vram_a[0x3ff] & 0x20)) {
			vaddr = 0x3ff;
		} else if(!(vram_a[0x5ff] & 0x20)) {
			vaddr = 0x5ff;
		} else if(!(vram_a[0x1ff] & 0x20)) {
			vaddr = 0x1ff;
		} else {
			vaddr = 0x3ff;
		}
		uint16 ank = vram_t[vaddr];
		uint16 knj = vram_k[vaddr];
		
		if(knj & 0x80) {
			uint32 ofs = adr2knj_x1t((knj << 8) | ank);
			if(knj & 0x40) {
				ofs += 16; // right
			}
			return kanji[ofs | (addr & 15)];
		} else if(mode1 & 0x40) {
			return kanji[(ank << 4) | (addr & 15)];
		} else {
			return font[(ank << 3) | ((addr >> 1) & 7)];
		}
	}
#endif
	get_cur_code_line();
	return font[(cur_code << 3) | (cur_line & 7)];
}

void DISPLAY::get_cur_pcg(uint32 addr)
{
#ifdef _X1TURBO_FEATURE
	if(mode1 & 0x20) {
		// wait next raster
		d_cpu->write_signal(SIG_CPU_BUSREQ, 1, 1);
		
		// from X1EMU
		uint16 vaddr;
		if(vram_a[0x7ff] & 0x20) {
			vaddr = 0x7ff;
		} else if(vram_a[0x3ff] & 0x20) {
			vaddr = 0x3ff;
		} else if(vram_a[0x5ff] & 0x20) {
			vaddr = 0x5ff;
		} else if(vram_a[0x1ff] & 0x20) {
			vaddr = 0x1ff;
		} else {
			vaddr = 0x3ff;
		}
		cur_code = vram_t[vaddr];
		cur_line = (addr >> 1) & 7;
		
		if(vram_k[vaddr] & 0x90) {
			cur_code &= 0xfe;
			cur_code += addr & 1;
		}
	} else
#endif
	get_cur_code_line();
}

void DISPLAY::get_cur_code_line()
{
//#ifdef _X1TURBO_FEATURE
//	int ht_clock = hireso ? 161 : 250;
//#else
	#define ht_clock 250
//#endif
	int clock = passed_clock(vblank_clock);
	int vt_line = vt_disp * ch_height + (int)(clock / ht_clock);
	
	int addr = (hz_total * (clock % ht_clock)) / ht_clock;
	addr += hz_disp * (int)(vt_line / ch_height);
	if(addr > 0x7ff) {
		addr = 0x7ff;
	}
	addr += st_addr;
	
	cur_code = vram_t[addr & 0x7ff];
	cur_line = (vt_line % ch_height) & 7;
}

void DISPLAY::draw_line(int v)
{
	if(v == 0) {
		memset(text, 0, sizeof(text));
		memset(cg, 0, sizeof(cg));
		prev_vert_double = false;
		raster = 0;
	}
	if((regs[8] & 0x30) != 0x30) {
		if((v % ch_height) == 0) {
			draw_text(v / ch_height);
		}
		draw_cg(v);
		memcpy(&pri_line[v][0][0], &pri[0][0], sizeof(pri));
	} else {
		memset(&pri_line[v][0][0], 0, sizeof(pri));
	}
}

void DISPLAY::draw_screen()
{
	// copy to real screen
#ifdef _X1TURBO_FEATURE
	if(hireso) {
		// 400 lines
		if(column40) {
			// 40 columns
			for(int y = 0; y < 400; y++) {
				scrntype* dest = emu->screen_buffer(y);
				uint8* src_text = text[y];
				uint8* src_cg = cg[y];
				
				for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
					dest[x2] = dest[x2 + 1] = palette_pc[pri_line[y][src_cg[x]][src_text[x]]];
				}
			}
		} else {
			// 80 columns
			for(int y = 0; y < 400; y++) {
				scrntype* dest = emu->screen_buffer(y);
				uint8* src_text = text[y];
				uint8* src_cg = cg[y];
				
				for(int x = 0; x < 640; x++) {
					dest[x] = palette_pc[pri_line[y][src_cg[x]][src_text[x]]];
				}
			}
		}
		emu->screen_skip_line = false;
	} else {
#endif
		// 200 lines
		if(column40) {
			// 40 columns
			for(int y = 0; y < 200; y++) {
				scrntype* dest0 = emu->screen_buffer(y * 2 + 0);
				scrntype* dest1 = emu->screen_buffer(y * 2 + 1);
				uint8* src_text = text[y];
				uint8* src_cg = cg[y];
				
				for(int x = 0, x2 = 0; x < 320; x++, x2 += 2) {
					dest0[x2] = dest0[x2 + 1] = palette_pc[pri_line[y][src_cg[x]][src_text[x]]];
				}
				if(!scanline) {
					memcpy(dest1, dest0, 640 * sizeof(scrntype));
				} else {
					memset(dest1, 0, 640 * sizeof(scrntype));
				}
			}
		} else {
			// 80 columns
			for(int y = 0; y < 200; y++) {
				scrntype* dest0 = emu->screen_buffer(y * 2 + 0);
				scrntype* dest1 = emu->screen_buffer(y * 2 + 1);
				uint8* src_text = text[y];
				uint8* src_cg = cg[y];
				
				for(int x = 0; x < 640; x++) {
					dest0[x] = palette_pc[pri_line[y][src_cg[x]][src_text[x]]];
				}
				if(!scanline) {
					memcpy(dest1, dest0, 640 * sizeof(scrntype));
				} else {
					memset(dest1, 0, 640 * sizeof(scrntype));
				}
			}
		}
		emu->screen_skip_line = true;
#ifdef _X1TURBO_FEATURE
	}
#endif
}

void DISPLAY::draw_text(int y)
{
	int width = column40 ? 40 : 80;
	uint16 src = st_addr + hz_disp * y;
	
	bool cur_vert_double = true;
	uint8 prev_attr = 0, prev_pattern_b[32], prev_pattern_r[32], prev_pattern_g[32];
	
	for(int x = 0; x < hz_disp && x < width; x++) {
		src &= 0x7ff;
		uint8 code = vram_t[src];
#ifdef _X1TURBO_FEATURE
		uint8 knj = vram_k[src];
#endif
		uint8 attr = vram_a[src];
		src++;
		
		uint8 col = attr & 7;
		bool reverse = ((attr & 8) != 0);
		bool blink = ((attr & 0x10) && (cblink & 0x20));
		reverse = (reverse != blink);
		
		// select pcg or ank
		const uint8 *pattern_b, *pattern_r, *pattern_g;
#ifdef _X1TURBO_FEATURE
		int shift = 0;
#endif
		if(attr & 0x20) {
			// pcg
#ifdef _X1TURBO_FEATURE
			if(knj & 0x90) {
				pattern_b = gaiji_b[code >> 1];
				pattern_r = gaiji_r[code >> 1];
				pattern_g = gaiji_g[code >> 1];
			} else {
#endif
				pattern_b = pcg_b[code];
				pattern_r = pcg_r[code];
				pattern_g = pcg_g[code];
#ifdef _X1TURBO_FEATURE
				shift = hireso ? 1 : 0;
			}
#endif
#ifdef _X1TURBO_FEATURE
		} else if(knj & 0x80) {
			uint32 ofs = adr2knj_x1t((knj << 8) | code);
			if(knj & 0x40) {
				ofs += 16; // right
			}
			pattern_b = pattern_r = pattern_g = &kanji[ofs];
			shift = hireso ? ((ch_height >= 32) ? 1 : 0) : ((ch_height >= 16) ? 0 : -1);
		} else if(hireso || (mode1 & 4)) {
			// ank 8x16 or kanji
			pattern_b = pattern_r = pattern_g = &kanji[code << 4];
			shift = hireso ? ((ch_height >= 32) ? 1 : 0) : ((ch_height >= 16) ? 0 : -1);
#endif
		} else {
			// ank 8x8
			pattern_b = pattern_r = pattern_g = &font[code << 3];
		}
		
		// check vertical doubled char
		if(!(attr & 0x40)) {
			cur_vert_double = false;
		}
		
		// render character
		for(int l = 0; l < ch_height; l++) {
			int line = cur_vert_double ? raster + (l >> 1) : l;
#ifdef _X1TURBO_FEATURE
			if(shift == 1) {
				line >>= 1;
			} else if(shift == -1) {
				line <<= 1;
				if(cur_vert_double) {
					line |= l & 1;
				}
			}
#endif
			uint8 b, r, g;
			if((x & 1) && (prev_attr & 0x80)) {
				b = prev_pattern_b[line] << 4;
				r = prev_pattern_r[line] << 4;
				g = prev_pattern_g[line] << 4;
			} else {
				b = prev_pattern_b[line] = pattern_b[line];
				r = prev_pattern_r[line] = pattern_r[line];
				g = prev_pattern_g[line] = pattern_g[line];
			}
			if(reverse) {
				b = (!(col & 1)) ? 0xff : ~b;
				r = (!(col & 2)) ? 0xff : ~r;
				g = (!(col & 4)) ? 0xff : ~g;
			} else {
				b = (!(col & 1)) ? 0 : b;
				r = (!(col & 2)) ? 0 : r;
				g = (!(col & 4)) ? 0 : g;
			}
			
			int yy = y * ch_height + l;
#ifdef _X1TURBO_FEATURE
			if(yy >= 400) {
#else
			if(yy >= 200) {
#endif
				break;
			}
			uint8* d = &text[yy][x << 3];
			
			if(attr & 0x80) {
				// horizontal doubled char
				d[ 0] = d[ 1] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
				d[ 2] = d[ 3] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
				d[ 4] = d[ 5] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
				d[ 6] = d[ 7] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
			} else {
				d[0] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
				d[1] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
				d[2] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
				d[3] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
				d[4] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
				d[5] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) >> 0);
				d[6] = ((b & 0x02) >> 1) | ((r & 0x02) >> 0) | ((g & 0x02) << 1);
				d[7] = ((b & 0x01) >> 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
			}
		}
		prev_attr = attr;
	}
	if(cur_vert_double && !prev_vert_double) {
		prev_vert_double = true;
		raster = ch_height >> 1;
	} else {
		prev_vert_double = false;
		raster = 0;
	}
}

void DISPLAY::draw_cg(int line)
{
	int width = column40 ? 40 : 80;
	
	int y = line / ch_height;
	int l = line % ch_height;
	if(y >= vt_disp) {
		return;
	}
	int ofs, src = st_addr + hz_disp * y;
#ifdef _X1TURBO_FEATURE
	int page = (hireso && !(mode1 & 2)) ? (l & 1) : (mode1 & 8);
	int ll = hireso ? (l >> 1) : l;
	
	if(mode1 & 4) {
		ofs = (0x400 * (ll & 15)) + (page ? 0xc000 : 0);
	} else {
		ofs = (0x800 * (ll & 7)) + (page ? 0xc000 : 0);
	}
#else
	ofs = 0x800 * (l & 7);
#endif
	int ofs_b = ofs + 0x0000;
	int ofs_r = ofs + 0x4000;
	int ofs_g = ofs + 0x8000;
	
	for(int x = 0; x < hz_disp && x < width; x++) {
		src &= 0x7ff;
		uint8 b = vram_ptr[ofs_b | src];
		uint8 r = vram_ptr[ofs_r | src];
		uint8 g = vram_ptr[ofs_g | src++];
		uint8* d = &cg[line][x << 3];
		
		d[0] = ((b & 0x80) >> 7) | ((r & 0x80) >> 6) | ((g & 0x80) >> 5);
		d[1] = ((b & 0x40) >> 6) | ((r & 0x40) >> 5) | ((g & 0x40) >> 4);
		d[2] = ((b & 0x20) >> 5) | ((r & 0x20) >> 4) | ((g & 0x20) >> 3);
		d[3] = ((b & 0x10) >> 4) | ((r & 0x10) >> 3) | ((g & 0x10) >> 2);
		d[4] = ((b & 0x08) >> 3) | ((r & 0x08) >> 2) | ((g & 0x08) >> 1);
		d[5] = ((b & 0x04) >> 2) | ((r & 0x04) >> 1) | ((g & 0x04) >> 0);
		d[6] = ((b & 0x02) >> 1) | ((r & 0x02) >> 0) | ((g & 0x02) << 1);
		d[7] = ((b & 0x01) >> 0) | ((r & 0x01) << 1) | ((g & 0x01) << 2);
	}
}

// kanji rom (from X1EMU by KM)

void DISPLAY::write_kanji(uint32 addr, uint32 data)
{
	switch(addr) {
	case 0xe80:
		kaddr = (kaddr & 0xff00) | data;
		break;
	case 0xe81:
		kaddr = (kaddr & 0xff) | (data << 8);
		break;
	case 0xe82:
		// TODO: bit0 L->H: Latch
		kanji_ptr = &kanji[adr2knj_x1(kaddr & 0xfff0)];
		break;
	}
}

uint32 DISPLAY::read_kanji(uint32 addr)
{
	switch(addr) {
	case 0xe80:
		if(kaddr & 0xff00) {
			uint32 val = kanji_ptr[kofs];
			kflag |= 1;
			if(kflag == 3) {
				kofs = (kofs + 1) & 15;
				kflag = 0;
			}
			return val;
		}
		return jis2adr_x1(kaddr << 8) >> 8;
	case 0xe81:
		if(kaddr & 0xff00) {
			uint32 val = kanji_ptr[kofs + 16];
			kflag |= 2;
			if(kflag == 3) {
				kofs = (kofs + 1) & 15;
				kflag = 0;
			}
			return val;
		}
		return 0;
	}
	return 0xff;
}

uint16 DISPLAY::jis2adr_x1(uint16 jis)
{
	uint16 jh, jl, adr;
	
	jh = jis >> 8;
	jl = jis & 0xff;
	if(jh > 0x28) {
		adr = 0x4000 + (jh - 0x30) * 0x600;
	} else {
		adr = 0x0100 + (jh - 0x21) * 0x600;
	}
	if(jl >= 0x20) {
		adr += (jl - 0x20) * 0x10;
	}
	return adr;
}

uint32 DISPLAY::adr2knj_x1(uint16 adr)
{
	uint16 jh, jl, jis;
	
	if(adr < 0x4000) {
		jh = adr - 0x0100;
		jh = 0x21 + jh / 0x600;
	} else {
		jh = adr - 0x4000;
		jh = 0x30 + jh / 0x600;
	}
	if(jh > 0x28) {
		adr -= 0x4000 + (jh - 0x30) * 0x600;
	} else {
		adr -= 0x0100 + (jh - 0x21) * 0x600;
	}
	jl = 0x20;
	if(adr) {
		jl += adr / 0x10;
	}
	
	jis = (jh << 8) | jl;
	return jis2knj(jis);
}

#ifdef _X1TURBO_FEATURE
uint32 DISPLAY::adr2knj_x1t(uint16 adr)
{
	uint16 j1, j2;
	uint16 rl, rh;
	uint16 jis;
	
	rh = adr >> 8;
	rl = adr & 0xff;
	
	rh &= 0x1f;
	if(!rl && !rh) {
		return jis2knj(0);
	}
	j2 = rl & 0x1f;		// rl4,3,2,1,0
	j1 = (rl / 0x20) & 7;	// rl7,6,5
	
	if(rh < 0x04) {
		// 2121-277e
		j1 |= 0x20;
		switch(rh & 3){
		case 0: j2 |= 0x20; break;
		case 1: j2 |= 0x60; break;
		case 2: j2 |= 0x40; break;
		default: j1 = j2 = 0; break;
		}
	} else if(rh > 0x1c) {
		// 7021-777e
		j1 |= 0x70;
		switch(rh & 3) {
		case 0: j2 |= 0x20; break;
		case 1: j2 |= 0x60; break;
		case 2: j2 |= 0x40; break;
		default: j1 = j2 = 0; break;
		}
	} else {
		j1 |= (((rh >> 1) + 7) / 3) * 0x10;
		j1 |= (rh & 1) * 8;
		j2 |= ((((rh >> 1) + 1) % 3) + 1) * 0x20;
	}
	
	jis = (j1 << 8) | j2;
	return jis2knj(jis);
}
#endif

uint32 DISPLAY::jis2knj(uint16 jis)
{
	uint32 sjis = jis2sjis(jis);
	
	if(sjis < 0x100){
		return sjis * 16;
	} else if(sjis >= 0x8140 && sjis < 0x84c0) {
		return 0x01000 + (sjis - 0x8140) * 32;
	} else if(sjis >= 0x8890 && sjis < 0xa000) {
		return 0x08000 + (sjis - 0x8890) * 32;
	} else if(sjis >= 0xe040 && sjis < 0xeab0) {
		return 0x36e00 + (sjis - 0xe040) * 32;
	} else {
		return 0;
	}
}

uint16 DISPLAY::jis2sjis(uint16 jis)
{
	uint16 c1, c2;
	
	if(!jis) {
		return 0;
	}
	c1 = jis >> 8;
	c2 = jis & 0xff;
	
	if(c1 & 1){
		c2 += 0x1f;
		if(c2 >= 0x7f) {
			c2++;
		}
	} else {
		c2 += 0x7e;
	}
	c1 = (c1 - 0x20 - 1) / 2 + 0x81;
	if(c1 >= 0xa0) {
		c1 += 0x40;
	}
	return (c1 << 8) | c2;
}

#define STATE_VERSION	1

void DISPLAY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(vram_t, sizeof(vram_t), 1);
	state_fio->Fwrite(vram_a, sizeof(vram_a), 1);
#ifdef _X1TURBO_FEATURE
	state_fio->Fwrite(vram_k, sizeof(vram_k), 1);
#endif
	state_fio->Fwrite(pcg_b, sizeof(pcg_b), 1);
	state_fio->Fwrite(pcg_r, sizeof(pcg_r), 1);
	state_fio->Fwrite(pcg_g, sizeof(pcg_g), 1);
#ifdef _X1TURBO_FEATURE
	state_fio->Fwrite(gaiji_b, sizeof(gaiji_b), 1);
	state_fio->Fwrite(gaiji_r, sizeof(gaiji_r), 1);
	state_fio->Fwrite(gaiji_g, sizeof(gaiji_g), 1);
#endif
	state_fio->FputUint8(cur_code);
	state_fio->FputUint8(cur_line);
	state_fio->FputInt32(kaddr);
	state_fio->FputInt32(kofs);
	state_fio->FputInt32(kflag);
	state_fio->FputInt32((int)(kanji_ptr - &kanji[0]));
	state_fio->Fwrite(pal, sizeof(pal), 1);
	state_fio->FputUint8(priority);
	state_fio->Fwrite(pri, sizeof(pri), 1);
	state_fio->FputBool(column40);
#ifdef _X1TURBO_FEATURE
	state_fio->FputUint8(mode1);
	state_fio->FputUint8(mode2);
	state_fio->FputBool(hireso);
#endif
#ifdef _X1TURBOZ
	state_fio->FputUint8(zmode1);
	state_fio->FputUint8(zpriority);
	state_fio->FputUint8(zscroll);
	state_fio->FputUint8(zmode2);
	state_fio->Fwrite(ztpal, sizeof(ztpal), 1);
	state_fio->Fwrite(zpal, sizeof(zpal), 1);
	state_fio->FputInt32(zpal_num);
	state_fio->Fwrite(palette_pc, sizeof(palette_pc), 1);
#endif
	state_fio->FputBool(prev_vert_double);
	state_fio->FputInt32(raster);
	state_fio->FputInt32(cblink);
	state_fio->FputBool(scanline);
	state_fio->FputInt32(ch_height);
	state_fio->FputInt32(hz_total);
	state_fio->FputInt32(hz_disp);
	state_fio->FputInt32(vt_disp);
	state_fio->FputInt32(st_addr);
	state_fio->FputUint32(vblank_clock);
}

bool DISPLAY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(vram_t, sizeof(vram_t), 1);
	state_fio->Fread(vram_a, sizeof(vram_a), 1);
#ifdef _X1TURBO_FEATURE
	state_fio->Fread(vram_k, sizeof(vram_k), 1);
#endif
	state_fio->Fread(pcg_b, sizeof(pcg_b), 1);
	state_fio->Fread(pcg_r, sizeof(pcg_r), 1);
	state_fio->Fread(pcg_g, sizeof(pcg_g), 1);
#ifdef _X1TURBO_FEATURE
	state_fio->Fread(gaiji_b, sizeof(gaiji_b), 1);
	state_fio->Fread(gaiji_r, sizeof(gaiji_r), 1);
	state_fio->Fread(gaiji_g, sizeof(gaiji_g), 1);
#endif
	cur_code = state_fio->FgetUint8();
	cur_line = state_fio->FgetUint8();
	kaddr = state_fio->FgetInt32();
	kofs = state_fio->FgetInt32();
	kflag = state_fio->FgetInt32();
	kanji_ptr = &kanji[0] + state_fio->FgetInt32();
	state_fio->Fread(pal, sizeof(pal), 1);
	priority = state_fio->FgetUint8();
	state_fio->Fread(pri, sizeof(pri), 1);
	column40 = state_fio->FgetBool();
#ifdef _X1TURBO_FEATURE
	mode1 = state_fio->FgetUint8();
	mode2 = state_fio->FgetUint8();
	hireso = state_fio->FgetBool();
#endif
#ifdef _X1TURBOZ
	zmode1 = state_fio->FgetUint8();
	zpriority = state_fio->FgetUint8();
	zscroll = state_fio->FgetUint8();
	zmode2 = state_fio->FgetUint8();
	state_fio->Fread(ztpal, sizeof(ztpal), 1);
	state_fio->Fread(zpal, sizeof(zpal), 1);
	zpal_num = state_fio->FgetInt32();
	state_fio->Fread(palette_pc, sizeof(palette_pc), 1);
#endif
	prev_vert_double = state_fio->FgetBool();
	raster = state_fio->FgetInt32();
	cblink = state_fio->FgetInt32();
	scanline = state_fio->FgetBool();
	ch_height = state_fio->FgetInt32();
	hz_total = state_fio->FgetInt32();
	hz_disp = state_fio->FgetInt32();
	vt_disp = state_fio->FgetInt32();
	st_addr = state_fio->FgetInt32();
	vblank_clock = state_fio->FgetUint32();
	return true;
}

