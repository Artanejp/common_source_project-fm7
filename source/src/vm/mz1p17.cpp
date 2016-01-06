/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.12.24-

	[ MZ-1P17 ]
*/

#include "mz1p17.h"
#include "../fifo.h"

#define PICA		0
#define ELITE		1
#define CONDENSE	2
#define PROPORTIONAL	3

#define SUPER_SCRIPT	1
#define SUB_SCRIPT	-1

#define TO_EVEN(v)	(int)(v) + (((int)(v)) & 1 ? 1 : 0)

void MZ1P17::initialize()
{
	// initialize bitmap
	int width = (int)(PIXEL_PER_INCH * 8.27);
	int height = (int)(PIXEL_PER_INCH * 11.69);
	
	if(width % 4) {
		width = (width >> 2) * 4 + 4;	// multiples of four
	}
	if(height % 4) {
		height = (height >> 2) * 4 + 4;	// multiples of four
	}
	emu->create_bitmap(&bitmap_paper, width, height);
	emu->create_bitmap(&bitmap_line[0], PIXEL_PER_INCH * 8, (48 + 2) * DOT_SCALE);
	emu->create_bitmap(&bitmap_line[1], PIXEL_PER_INCH * 8, (48 + 2) * DOT_SCALE);
	emu->create_bitmap(&bitmap_line[2], PIXEL_PER_INCH * 8, (48 + 2) * DOT_SCALE);
	emu->create_bitmap(&bitmap_line[3], PIXEL_PER_INCH * 8, (48 + 2) * DOT_SCALE);
//	emu->create_font(&font, _T("Mincho"), 12 * DOT_SCALE, 24 * DOT_SCALE, 0, false, false);
	space_left = (bitmap_paper.width - PIXEL_PER_INCH * 8) / 2;
	space_top = (bitmap_paper.height - PIXEL_PER_INCH * 11) / 2;
	
	// initialize non ank font
	memset(ank, 0, sizeof(ank));
	FILEIO *fio = new FILEIO();
#if defined(_MZ1500) || defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		for(int i = 0; i < 256; i++) {
			for(int j = 0; j < 16; j += 2) {
				uint8 p = fio->FgetUint8();
				ank[i][j][0] = ank[i][j + 1][0] = ((p & 0x80) != 0);
				ank[i][j][1] = ank[i][j + 1][1] = ((p & 0x40) != 0);
				ank[i][j][2] = ank[i][j + 1][2] = ((p & 0x20) != 0);
				ank[i][j][3] = ank[i][j + 1][3] = ((p & 0x10) != 0);
				ank[i][j][4] = ank[i][j + 1][4] = ((p & 0x08) != 0);
				ank[i][j][5] = ank[i][j + 1][5] = ((p & 0x04) != 0);
				ank[i][j][6] = ank[i][j + 1][6] = ((p & 0x02) != 0);
				ank[i][j][7] = ank[i][j + 1][7] = ((p & 0x01) != 0);
			}
		}
		fio->Fclose();
	}
#elif defined(_MZ2500) || defined(_MZ2800)
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		for(int i = 0; i < 256; i++) {
			fio->Fseek(0x6000 + 32 * i, FILEIO_SEEK_SET);
			for(int j = 0; j < 16; j++) {
				uint8 p = fio->FgetUint8();
				ank[i][j][0] = ((p & 0x80) != 0);
				ank[i][j][1] = ((p & 0x40) != 0);
				ank[i][j][2] = ((p & 0x20) != 0);
				ank[i][j][3] = ((p & 0x10) != 0);
				ank[i][j][4] = ((p & 0x08) != 0);
				ank[i][j][5] = ((p & 0x04) != 0);
				ank[i][j][6] = ((p & 0x02) != 0);
				ank[i][j][7] = ((p & 0x01) != 0);
			}
		}
		fio->Fclose();
	}
#elif defined(_MZ3500)
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		fio->Fseek(0x1000, FILEIO_SEEK_SET);
		for(int i = 0; i < 256; i++) {
			for(int j = 0; j < 16; j++) {
				uint8 p = fio->FgetUint8();
				ank[i][j][0] = ((p & 0x80) != 0);
				ank[i][j][1] = ((p & 0x40) != 0);
				ank[i][j][2] = ((p & 0x20) != 0);
				ank[i][j][3] = ((p & 0x10) != 0);
				ank[i][j][4] = ((p & 0x08) != 0);
				ank[i][j][5] = ((p & 0x04) != 0);
				ank[i][j][6] = ((p & 0x02) != 0);
				ank[i][j][7] = ((p & 0x01) != 0);
			}
		}
		fio->Fclose();
	}
#elif defined(_MZ5500) || defined(_MZ6500) || defined(_MZ6550)
	if(fio->Fopen(create_local_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fseek(0x800, FILEIO_SEEK_SET);
		for(int i = 0; i < 256; i++) {
			for(int j = 15; j >= 0; j--) {
				uint8 p = fio->FgetUint8();
				ank[i][j][0] = ((p & 0x80) != 0);
				ank[i][j][1] = ((p & 0x40) != 0);
				ank[i][j][2] = ((p & 0x20) != 0);
				ank[i][j][3] = ((p & 0x10) != 0);
				ank[i][j][4] = ((p & 0x08) != 0);
				ank[i][j][5] = ((p & 0x04) != 0);
				ank[i][j][6] = ((p & 0x02) != 0);
				ank[i][j][7] = ((p & 0x01) != 0);
			}
		}
		fio->Fclose();
	}
#elif defined(_X1) || defined(_X1TWIN) || defined(_X1TURBO) || defined(_X1TURBOZ)
	if(fio->Fopen(create_local_path(_T("ANK16.ROM")), FILEIO_READ_BINARY) ||
	   fio->Fopen(create_local_path(_T("FNT0816.X1")), FILEIO_READ_BINARY)) {
		uint8 font[0x1000];
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
		memcpy(font + 0x7f * 16, ANKFONT7f_9f, sizeof(ANKFONT7f_9f));
		memcpy(font + 0xe0 * 16, ANKFONTe0_ff, sizeof(ANKFONTe0_ff));
		
		for(int i = 0; i < 256; i++) {
			for(int j = 0; j < 16; j++) {
				uint8 p = font[i * 16 + j];
				ank[i][j][0] = ((p & 0x80) != 0);
				ank[i][j][1] = ((p & 0x40) != 0);
				ank[i][j][2] = ((p & 0x20) != 0);
				ank[i][j][3] = ((p & 0x10) != 0);
				ank[i][j][4] = ((p & 0x08) != 0);
				ank[i][j][5] = ((p & 0x04) != 0);
				ank[i][j][6] = ((p & 0x02) != 0);
				ank[i][j][7] = ((p & 0x01) != 0);
			}
		}
	}
#endif
	delete fio;
	
	// initialize hiragana font
	static const uint16 hiragana_sjis[] = {
		0x82f0,					// ‚ð
		0x829f, 0x82a1, 0x82a3, 0x82a5, 0x82a7,	// ‚Ÿ‚¡‚£‚¥‚§
		0x82e1, 0x82e3, 0x82e5, 0x82c1,		// ‚á‚ã‚å‚Á
		0,					// 
		0x82a0, 0x82a2, 0x82a4, 0x82a6, 0x82a8,	// ‚ ‚¢‚¤‚¦‚¨
		0x82a9, 0x82ab, 0x82ad, 0x82af, 0x82b1,	// ‚©‚«‚­‚¯‚±
		0x82b3, 0x82b5, 0x82b7, 0x82b9, 0x82bb,	// ‚³‚µ‚·‚¹‚»
		0x82bd, 0x82bf, 0x82c2, 0x82c4, 0x82c6,	// ‚½‚¿‚Â‚Ä‚Æ
		0x82c8, 0x82c9, 0x82ca, 0x82cb, 0x82cc,	// ‚È‚É‚Ê‚Ë‚Ì
		0x82cd, 0x82d0, 0x82d3, 0x82d6, 0x82d9,	// ‚Í‚Ð‚Ó‚Ö‚Ù
		0x82dc, 0x82dd, 0x82de, 0x82df, 0x82e0,	// ‚Ü‚Ý‚Þ‚ß‚à
		0x82e2, 0x82e4, 0x82e6,			// ‚â‚ä‚æ
		0x82e7, 0x82e8, 0x82e9, 0x82ea, 0x82eb,	// ‚ç‚è‚é‚ê‚ë
		0x82ed, 0x82f1,				// ‚í‚ñ
	};
	memset(hiragana, 0, sizeof(hiragana));
	memset(hiragana_bold, 0, sizeof(hiragana_bold));
	emu->create_font(&font, _T("Mincho"), 12, 48, 0, false, false);
	for(int i = 0; i < array_length(hiragana_sjis); i++) {
		if(hiragana_sjis[i] != 0) {
			char tmp[3];
			tmp[0] = hiragana_sjis[i] >> 8;
			tmp[1] = hiragana_sjis[i] & 0xff;
			tmp[2] = 0;
			emu->draw_rectangle_to_bitmap(&bitmap_paper, 0, 0, 24, 48, 0, 0, 0);
			emu->draw_text_to_bitmap(&bitmap_paper, &font, 0, 0, tmp, 255, 255, 255);
			for(int y = 0; y < 48; y++) {
				scrntype *p = bitmap_paper.get_buffer(y);
				for(int x = 0; x < 24; x++) {
					hiragana[i][y][x] = (p[x] != 0);
				}
			}
		}
	}
	emu->release_font(&font);
	emu->create_font(&font, _T("Mincho"), 12, 48, 0, true, false);
	for(int i = 0; i < array_length(hiragana_sjis); i++) {
		if(hiragana_sjis[i] != 0) {
			char tmp[3];
			tmp[0] = hiragana_sjis[i] >> 8;
			tmp[1] = hiragana_sjis[i] & 0xff;
			tmp[2] = 0;
			emu->draw_rectangle_to_bitmap(&bitmap_paper, 0, 0, 24, 48, 0, 0, 0);
			emu->draw_text_to_bitmap(&bitmap_paper, &font, 0, 0, tmp, 255, 255, 255);
			for(int y = 0; y < 48; y++) {
				scrntype *p = bitmap_paper.get_buffer(y);
				for(int x = 0; x < 24; x++) {
					hiragana_bold[i][y][x] = (p[x] != 0);
				}
			}
		}
	}
	emu->release_font(&font);
	
	fifo = new FIFO(65536);
	line_printed = paper_printed = false;
	paper_index = 0;
	
	value = wait_frames = -1;
	strobe = false;
	
	register_frame_event(this);
}

void MZ1P17::release()
{
	finish();
	
	emu->release_font(&font);
	emu->release_bitmap(&bitmap_line[0]);
	emu->release_bitmap(&bitmap_line[1]);
	emu->release_bitmap(&bitmap_line[2]);
	emu->release_bitmap(&bitmap_line[3]);
	emu->release_bitmap(&bitmap_paper);
	
	fifo->release();
	delete fifo;
}

void MZ1P17::reset()
{
	finish();
	
	emu->clear_bitmap(&bitmap_paper, 255, 255, 255);
	emu->clear_bitmap(&bitmap_line[0], 255, 255, 255);
	
	memset(gaiji, 0, sizeof(gaiji));
	memset(htab, 0, sizeof(htab));
	
	fifo->clear();
	
	lf_pitch = PIXEL_PER_INCH / 6;
	margin_left = margin_right = 0;
	pitch_mode = PICA;
	script_mode = 0;
	kanji_mode = kanji_half = hiragana_mode = false;
	bold = underline = reverse = vertical = false;
	
	ank_double_x = ank_double_y = false;
	kanji_double_x = kanji_double_y = false;
	kanji_pitch = 27;
	kanji_half_pitch = 18;
	
	dest_line_x = dest_paper_y = 0;
	color_mode = 0;
	double_y_printed = false;
	line_printed = paper_printed = false;
	paper_index = 0;
	
	value = -1;
	strobe = false;
}

void MZ1P17::event_frame()
{
	if(paper_index > 0 && --wait_frames == 0) {
		finish();
	}
}

void MZ1P17::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_PRINTER_DATA) {
		value = data;
	} else if(id == SIG_PRINTER_STROBE) {
		bool new_strobe = ((data & mask) != 0);
#ifdef MZ1P17_STROBE_RISING_EDGE
		bool edge = (!strobe && new_strobe);
#else
		bool edge = (strobe && !new_strobe);
#endif
		strobe = new_strobe;
		
		if(edge && value != -1) {
			if(paper_index == 0) {
				create_date_file_path(base_path, _MAX_PATH, _T("png"));
				paper_index++;
			}
			fifo->write(value);
			process();
			// wait 1sec
#ifdef SUPPORT_VARIABLE_TIMING
			wait_frames = (int)(vm->frame_rate() * 1.0 + 0.5);
#else
			wait_frames = (int)(FRAMES_PER_SEC * 1.0 + 0.5);
#endif
		}
	}
}

uint32 MZ1P17::read_signal(int ch)
{
	if(ch == SIG_PRINTER_BUSY) {
		return 0;
	}
	return 0;
}

void MZ1P17::process()
{
	if(fifo->read_not_remove(0) == 0x08) {
		// BS
		if(kanji_mode) {
			if(kanji_half) {
				dest_line_x -= kanji_half_pitch * DOT_SCALE;
			} else {
				dest_line_x -= kanji_pitch * DOT_SCALE;
			}
		} else {
			if(pitch_mode == ELITE) {
				dest_line_x -= PIXEL_PER_INCH / 12;
			} else if(pitch_mode == CONDENSE) {
				dest_line_x -= PIXEL_PER_INCH / 17;
			} else {
				dest_line_x -= PIXEL_PER_INCH / 10;
			}
		}
		if(dest_line_x < margin_left) {
			dest_line_x = margin_left;
		}
		fifo->clear();
	} else if(fifo->read_not_remove(0) == 0x09) {
		// HT
		for(int i = dest_line_x + 1; i < array_length(htab); i++) {
			if(htab[i]) {
				dest_line_x = min(i, PIXEL_PER_INCH * 8 - margin_right);
				break;
			}
		}
		fifo->clear();
	} else if(fifo->read_not_remove(0) == 0x11) {
		// DC 1 (not supported: printer select)
		fifo->clear();
	} else if(fifo->read_not_remove(0) == 0x13) {
		// DC 3 (not supported: printer disselect)
		fifo->clear();
	} else if(fifo->read_not_remove(0) == 0x0a) {
		// LF
		int next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
		finish_line();
		scroll(next_lf_pitch);
		fifo->clear();
	} else if(fifo->read_not_remove(0) == 0x0c) {
		// FF
		finish_line();
		finish_paper();
		fifo->clear();
	} else if(fifo->read_not_remove(0) == 0x0d) {
		// CR
#ifdef MZ1P17_AUTO_LF
		int next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
		finish_line();
		scroll(next_lf_pitch);
#else
		if(color_mode) {
			finish_line();
		} else {
			dest_line_x = margin_left;
		}
#endif
		fifo->clear();
	} else if(fifo->read_not_remove(0) == 0x0e) {
		// SO
		ank_double_x = kanji_double_x = true;
		fifo->clear();
	} else if(fifo->read_not_remove(0) == 0x0f) {
		// SI
		ank_double_x = kanji_double_x = false;
		fifo->clear();
	} else if(fifo->read_not_remove(0) == 0x18) {
		// CAN (not supported: cancel line buffer)
		fifo->clear();
	} else if(fifo->read_not_remove(0) == 0x1b) {
		// ESC
		if(fifo->count() >= 2) {
			if(fifo->read_not_remove(1) == 0x0b) {
				// ESC VT p
				if(fifo->count() == 4) {
					int p = (fifo->read_not_remove(2) - '0') * 10 + (fifo->read_not_remove(3) - '0');
					if(p >= 1 && p <= 99) {
						int next_lf_pitch = lf_pitch * p + (double_y_printed ? 24 * DOT_SCALE : 0);
						finish_line();
						scroll(next_lf_pitch);
					}
					fifo->clear();
				}
			} else if(fifo->read_not_remove(1) == 0x19) {
				// ESC EM
				emu->clear_bitmap(&bitmap_line[1], 0, 0, 0);
				emu->clear_bitmap(&bitmap_line[2], 0, 0, 0);
				emu->clear_bitmap(&bitmap_line[3], 0, 0, 0);
				color_mode = 3;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x21) {
				// ESC !
				bold = true;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x22) {
				// ESC "
				bold = false;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x24) {
				if(fifo->count() >= 3) {
					if(fifo->read_not_remove(2) == 0x40) {
						// ESC $ @
						kanji_mode = true;
						fifo->clear();
					} else {
						fifo->clear(); // unknown
					}
				}
			} else if(fifo->read_not_remove(1) == 0x25) {
				if(fifo->count() >= 3) {
					if(fifo->read_not_remove(2) == 0x31) {
						// ESC % 1 n1 n2
						if(fifo->count() >= 5) {
							int n = fifo->read_not_remove(3) * 256 + fifo->read_not_remove(4);
							if(fifo->count() == n * 3 + 5) {
								uint8 c = (reverse != (color_mode != 0)) ? 255 : 0;
								for(int i = 0; i < n; i++) {
									if(dest_line_x < 1440 * DOT_SCALE) {
										if(reverse) {
											emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, DOT_SCALE, 48 * DOT_SCALE, ~c, ~c, ~c);
										}
										int d1 = fifo->read_not_remove(5 + i * 3 + 0);
										int d2 = fifo->read_not_remove(5 + i * 3 + 1);
										int d3 = fifo->read_not_remove(5 + i * 3 + 2);
										if(d1 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  0) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  1) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  2) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  3) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  4) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  5) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  6) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  7) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  8) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  9) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 10) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 11) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 12) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 13) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 14) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 15) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 16) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 17) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 18) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 19) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 20) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 21) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 22) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 23) * DOT_SCALE, DOT_SCALE, DOT_SCALE, c, c, c);
										dest_line_x += DOT_SCALE;
										line_printed = true;
									}
								}
								fifo->clear();
							}
						}
					} else if(fifo->read_not_remove(2) == 0x33) {
						// ESC % 3 n1 n2
						if(fifo->count() == 5) {
							int n = fifo->read_not_remove(3) * 256 + fifo->read_not_remove(4);
							uint8 c = (reverse != (color_mode != 0)) ? 255 : 0;
							for(int i = 0; i < n; i++) {
								if(dest_line_x < PIXEL_PER_INCH * 8 - margin_right) {
									if(reverse) {
										emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, DOT_SCALE, bitmap_line[color_mode].height, ~c, ~c, ~c);
									}
									if(underline) {
										if(color_mode) {
											emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (48 + 1) * DOT_SCALE, DOT_SCALE, DOT_SCALE, 255, 255, 255);
										} else {
											emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (48 + 1) * DOT_SCALE, DOT_SCALE, DOT_SCALE, 0, 0, 0);
										}
									}
									dest_line_x += DOT_SCALE;
									line_printed = true;
								}
							}
							fifo->clear();
						}
					} else if(fifo->read_not_remove(2) == 0x35) {
						// ESC % 5 n
						if(fifo->count() == 4) {
							if(fifo->read_not_remove(3) != 0) {
								finish_line();
								scroll(PIXEL_PER_INCH * fifo->read_not_remove(3) / 120);
							}
							fifo->clear();
						}
					} else if(fifo->read_not_remove(2) == 0x36) {
						// ESC % 6 n
						if(fifo->count() == 4) {
							if(fifo->read_not_remove(3) != 0) {
								lf_pitch = PIXEL_PER_INCH * fifo->read_not_remove(3) / 120;
							}
							fifo->clear();
						}
					} else if(fifo->read_not_remove(2) == 0x42) {
						// ESC % B
						fifo->clear();
					} else if(fifo->read_not_remove(2) == 0x55) {
						// ESC % U
						fifo->clear();
					} else {
						fifo->clear(); // unknown
					}
				}
			} else if(fifo->read_not_remove(1) == 0x26) {
				// ESC &
				hiragana_mode = true;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x27) {
				// ESC '
				hiragana_mode = false;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x28) {
				if(fifo->count() >= 3) {
					if(fifo->read_not_remove(2) == 0x48) {
						// ESC ( H
						kanji_mode = false;
						fifo->clear();
					} else {
						fifo->clear(); // unknown
					}
				}
			} else if(fifo->read_not_remove(1) == 0x29) {
				// ESC + p1,p2,...,pk.
				if(fifo->read_not_remove(fifo->count() - 1) == 0x2e) {
					for(int i = 2; i < fifo->count(); i += 4) {
						int p = (fifo->read_not_remove(i) - '0') * 100 + (fifo->read_not_remove(i + 1) - '0') * 10 + (fifo->read_not_remove(i + 2) - '0');
						if(p >= 1 && p <= 999) {
							p -= 1;
							if(kanji_mode) {
								if(kanji_half) {
									p *= kanji_half_pitch * DOT_SCALE;
								} else {
									p *= kanji_pitch * DOT_SCALE;
								}
							} else {
								p *= PIXEL_PER_INCH / 10;
							}
							if(p < array_length(htab)) {
								htab[p] = false;
							}
						}
					}
					fifo->clear();
				}
			} else if(fifo->read_not_remove(1) == 0x2b) {
				// ESC + p1,p2,...,pk.
				if(fifo->read_not_remove(fifo->count() - 1) == 0x2e) {
					for(int i = 2; i < fifo->count(); i += 4) {
						int p = (fifo->read_not_remove(i) - '0') * 100 + (fifo->read_not_remove(i + 1) - '0') * 10 + (fifo->read_not_remove(i + 2) - '0');
						if(p >= 1 && p <= 999) {
							p -= 1;
							if(kanji_mode) {
								if(kanji_half) {
									p *= kanji_half_pitch * DOT_SCALE;
								} else {
									p *= kanji_pitch * DOT_SCALE;
								}
							} else {
								p *= PIXEL_PER_INCH / 10;
							}
							if(p < array_length(htab)) {
								htab[p] = true;
							}
						}
					}
					fifo->clear();
				}
			} else if(fifo->read_not_remove(1) == 0x32) {
				// ESC 2
				memset(htab, 0, sizeof(htab));
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x35) {
				// ESC 5 (not supported: set TOF)
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x36) {
				// ESC 6
				lf_pitch = PIXEL_PER_INCH / 6;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x38) {
				// ESC 8
				lf_pitch = PIXEL_PER_INCH / 8;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x45) {
				// ESC E
				pitch_mode = ELITE;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x46) {
				// ESC F p (not supported: set page length)
				if(fifo->count() == 4) {
					int p = (fifo->read_not_remove(2) - '0') * 10 + (fifo->read_not_remove(3) - '0');
					if(p >= 1 && p <= 99) {
					}
					fifo->clear();
				}
			} else if(fifo->read_not_remove(1) == 0x48) {
				// ESC H
				pitch_mode = PICA;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x49) {
				// ESC I n
				if(fifo->count() == 3) {
					int n = fifo->read_not_remove(2);
					if(kanji_mode) {
						if(kanji_half) {
							margin_left = n * kanji_half_pitch * DOT_SCALE;
						} else {
							margin_left = n * kanji_pitch * DOT_SCALE;
						}
					} else {
						margin_left = n * PIXEL_PER_INCH / 10;
					}
					fifo->clear();
				}
			} else if(fifo->read_not_remove(1) == 0x4e) {
				// ESC N
				pitch_mode = PICA;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x50) {
				// ESC P
				pitch_mode = PROPORTIONAL;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x51) {
				// ESC Q
				pitch_mode = CONDENSE;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x52) {
				// ESC R
				ank_double_x = false;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x53) {
				// ESC S n1 n2
				if(fifo->count() >= 4) {
					int n = fifo->read_not_remove(2) * 256 + fifo->read_not_remove(3);
					if(fifo->count() == n + 4) {
						uint8 c = (reverse != (color_mode != 0)) ? 255 : 0;
						for(int i = 0; i < n; i++) {
							if(dest_line_x < 1280 * DOT_SCALE) {
								if(reverse) {
									emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, 2 * DOT_SCALE, 48 * DOT_SCALE, ~c, ~c, ~c);
								}
								int d = fifo->read_not_remove(4 + i);
								if(d & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 0) * DOT_SCALE, 2 * DOT_SCALE, 3 * DOT_SCALE, c, c, c);
								if(d & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 1) * DOT_SCALE, 2 * DOT_SCALE, 3 * DOT_SCALE, c, c, c);
								if(d & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 2) * DOT_SCALE, 2 * DOT_SCALE, 3 * DOT_SCALE, c, c, c);
								if(d & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 3) * DOT_SCALE, 2 * DOT_SCALE, 3 * DOT_SCALE, c, c, c);
								if(d & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 4) * DOT_SCALE, 2 * DOT_SCALE, 3 * DOT_SCALE, c, c, c);
								if(d & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 5) * DOT_SCALE, 2 * DOT_SCALE, 3 * DOT_SCALE, c, c, c);
								if(d & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 6) * DOT_SCALE, 2 * DOT_SCALE, 3 * DOT_SCALE, c, c, c);
								if(d & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 7) * DOT_SCALE, 2 * DOT_SCALE, 3 * DOT_SCALE, c, c, c);
								dest_line_x += 2 * DOT_SCALE;
								line_printed = true;
							}
						}
						fifo->clear();
					}
				}
			} else if(fifo->read_not_remove(1) == 0x54) {
				// ESC T n
				if(fifo->count() == 3) {
					int n = fifo->read_not_remove(2);
					if(kanji_mode) {
						if(kanji_half) {
							margin_right = max(0, PIXEL_PER_INCH * 8 - n * kanji_half_pitch * DOT_SCALE);
						} else {
							margin_right = max(0, PIXEL_PER_INCH * 8 - n * kanji_pitch * DOT_SCALE);
						}
					} else {
						margin_right = max(0, PIXEL_PER_INCH * 8 - n * PIXEL_PER_INCH / 10);
					}
					fifo->clear();
				}
			} else if(fifo->read_not_remove(1) == 0x55) {
				// ESC U
				ank_double_x = true;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x58) {
				// ESC X
				underline = true;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x59) {
				// ESC Y
				underline = false;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x67) {
				// ESC g
				reverse = true;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x68) {
				// ESC h
				reverse = false;
				fifo->clear();
			} else {
				fifo->clear(); // unknown
			}
		}
	} else if(fifo->read_not_remove(0) == 0x1c) {
		// CEX
		if(fifo->count() >= 2) {
			if(fifo->read_not_remove(1) == 0x24) {
				// CEX $ n
				if(fifo->count() == 3) {
					if(kanji_half) {
						kanji_half_pitch = fifo->read_not_remove(2);
					} else {
						kanji_pitch = fifo->read_not_remove(2);
					}
					fifo->clear();
				}
			} else if(fifo->read_not_remove(1) == 0x30) {
				// CEX 0 n1 n2 (not supported: set gaiji size)
				if(fifo->count() == 4) {
					// n1, n2 shoud be 24
					fifo->clear();
				}
			} else if(fifo->read_not_remove(1) == 0x32) {
				// CEX 2 n1 n2 d1 d2 ... d72
				if(fifo->count() == 76) {
					int n1 = fifo->read_not_remove(2);
					int n2 = fifo->read_not_remove(3);
					if(n1 >= 0x78 && n1 <= 0x7a && n2 >= 0x21 && n2 <= 0x7e) {
						n1 -= 0x78;
						n2 -= 0x21;
						int p = 4;
						for(int x = 0; x < 24; x++) {
							for(int y = 0; y < 24; y += 8) {
								int d = fifo->read_not_remove(p++);
								gaiji[n1][n2][y + 0][x] = ((d & 0x80) != 0);
								gaiji[n1][n2][y + 1][x] = ((d & 0x40) != 0);
								gaiji[n1][n2][y + 2][x] = ((d & 0x20) != 0);
								gaiji[n1][n2][y + 3][x] = ((d & 0x10) != 0);
								gaiji[n1][n2][y + 4][x] = ((d & 0x08) != 0);
								gaiji[n1][n2][y + 5][x] = ((d & 0x04) != 0);
								gaiji[n1][n2][y + 6][x] = ((d & 0x02) != 0);
								gaiji[n1][n2][y + 7][x] = ((d & 0x01) != 0);
							}
						}
					}
					fifo->clear();
				}
			} else if(fifo->read_not_remove(1) == 0x4a) {
				// CEX J
				vertical = true;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x4b) {
				// CEX K
				vertical = false;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x4e) {
				// CEX N
				script_mode = SUPER_SCRIPT;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x4f) {
				// CEX O
				script_mode = 0;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x50) {
				// CEX P
				script_mode = SUB_SCRIPT;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x51) {
				// CEX Q
				script_mode = 0;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x5d) {
				// CEX ]
				reset();
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x5f) {
				// CEX _ (not supported: typesetting)
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x70) {
				// CEX p
				kanji_double_x = true;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x71) {
				// CEX q
				kanji_double_x = false;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x72) {
				// CEX r
				kanji_half = true;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x73) {
				// CEX s
				kanji_half = false;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x74) {
				// CEX t
				ank_double_y = kanji_double_y = true;
				fifo->clear();
			} else if(fifo->read_not_remove(1) == 0x75) {
				// CEX u
				ank_double_y = kanji_double_y = false;
				fifo->clear();
			} else {
				fifo->clear(); // unknown
			}
		}
	} else {
		if(kanji_mode) {
			if(fifo->count() == 2) {
				int code = fifo->read() << 8;
				code += fifo->read();
				draw_char(code);
			}
		} else {
			draw_char(fifo->read());
		}
	}
}

//#define IS_HANKAKU(c) \
//	((c >= 0x20 && c <= 0xff) || (c >= 0x2820 && c <= 0x28ff))
#define IS_HANKAKU(c) \
	((c >= 0x00 && c <= 0xff) || (c >= 0x2800 && c <= 0x28ff))
#define IS_NOT_ANK(c) \
	((c >= 0x00 && c <= 0x1f) || (c >= 0x7f && c <= 0xa0) || (c >= 0xe0 && c <= 0xff) || (c >= 0x2800 && c <= 0x281f) || (c >= 0x287f && c <= 0x28a0) || (c >= 0x28e0 && c <= 0x28ff))
#define IS_KANA(c) \
	((c >= 0xa6 && c <= 0xdd && c != 0xb0) || (c >= 0x28a6 && c <= 0x28dd && c != 0x28b0))
#define IS_GAIJI(c) \
	((c >= 0x7821 && c <= 0x787e) || (c >= 0x7921 && c <= 0x797e) || (c >= 0x7a21 && c <= 0x7a2b))
#define IS_NOT_VERTICAL(c) \
	(c == 0x2121 || (c >= 0x2126 && c <= 0x2128) || (c >= 0x213c && c <= 0x213e) || (c >= 0x2142 && c <= 0x2145) || (c >= 0x214a && c <= 0x215b) || (c >= 0x2161 && c <= 0x2166) || (c >= 0x217b && c <= 0x217d) || \
	 c == 0x2228 || (c >= 0x222a && c <= 0x222d))

void MZ1P17::draw_char(uint16 code)
{
	bool is_kanji = kanji_mode;
	int font_width = 12 * DOT_SCALE;
	int font_height = 24 * DOT_SCALE;
	int font_rotate = 0;
	int gap_p1, gap_p2;
	int dest_line_y = 24 * DOT_SCALE;
	bool double_x, double_y;
	uint8 c = (reverse != (color_mode != 0)) ? 255 : 0;
	char tmp[3];
	
	if(kanji_mode) {
		gap_p1 = ((kanji_pitch <= 26) ? kanji_pitch - 24 : 2) * DOT_SCALE;
		gap_p2 = ((kanji_pitch <= 26) ? 0 : kanji_pitch - 26) * DOT_SCALE;
		
		if(IS_HANKAKU(code)) {
			if(kanji_half) {
				gap_p1 = ((kanji_half_pitch <= 15) ? kanji_half_pitch - 12 : 3) * DOT_SCALE;
				gap_p2 = ((kanji_half_pitch <= 15) ? 0 : kanji_half_pitch - 15) * DOT_SCALE;
				code &= 0x00ff;
			} else {
				code |= 0x2800;
			}
		}
		double_x = kanji_double_x;
		double_y = kanji_double_y;
	} else {
		if(pitch_mode == ELITE) {
			gap_p1 = 3 * DOT_SCALE;
			gap_p2 = 0 * DOT_SCALE;
		} else if(pitch_mode == CONDENSE) {
			// 17chars/inch, pitch = 180 / 17 = 10.58
			gap_p1 = 2 * DOT_SCALE;
			gap_p2 = 0 * DOT_SCALE;
			font_width = 9 * DOT_SCALE;
		} else {
			gap_p1 = 3 * DOT_SCALE;
			gap_p2 = 3 * DOT_SCALE;
		}
		double_x = ank_double_x;
		double_y = ank_double_y;
	}
	if(code & 0xff00) {
		if(vertical && !IS_NOT_VERTICAL(code)) {
			font_rotate = 900;
		}
		if(double_x) {
			font_width *= 2;
			gap_p1 *= 2;
			gap_p2 *= 2;
		}
		if(double_y) {
			font_height *= 2;
			dest_line_y = 0;
		}
		uint16 sjis = jis_to_sjis(code);
		tmp[0] = sjis >> 8;
		tmp[1] = sjis & 0xff;
		tmp[2] = '\0';
	} else {
		if(script_mode == SUPER_SCRIPT) {
			font_width /= 2;
			font_height /= 2;
			gap_p1 /= 2;
			gap_p2 /= 2;
		} else if(script_mode == SUB_SCRIPT) {
			font_width /= 2;
			font_height /= 2;
			gap_p1 /= 2;
			gap_p2 /= 2;
			dest_line_y = (24 + 12) * DOT_SCALE;
		} else {
			// is this not applied in script mode ?
			if(double_y) {
				font_height *= 2;
				dest_line_y = 0;
			}
		}
		if(double_x) {
			font_width *= 2;
			gap_p1 *= 2;
			gap_p2 *= 2;
		}
		tmp[0] = code & 0xff;
		tmp[1] = '\0';
	}
	bool proportional = (!(code & 0xff00) && pitch_mode == PROPORTIONAL);
	
	if(!font.initialized() || font.width != font_width || font.height != font_height || font.rotate != font_rotate || font.bold != bold || 
	   (proportional && font.family[0] != _T('P')) || (!proportional && font.family[0] == _T('P'))) {
		emu->release_font(&font);
		emu->create_font(&font, proportional ? _T("PMincho") : _T("Mincho"), font_width, font_height, font_rotate, bold, false);
	}
	if(code & 0xff00) {
		font_width *= 2;
	} else if(proportional) {
		if(IS_NOT_ANK(code) || (IS_KANA(code) && hiragana_mode)) {
			// use internal font
		} else {
			font_width = emu->get_text_width(&bitmap_line[color_mode], &font, tmp);
		}
	}
	if(dest_line_x + gap_p1 + font_width + gap_p2 > PIXEL_PER_INCH * 8 - margin_right) {
		int next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
		finish_line();
		scroll(next_lf_pitch);
	}
	if(dest_line_y == 0) {
		double_y_printed = true;
	}
	if(reverse) {
		emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, gap_p1 + font_width + gap_p2, 48 * DOT_SCALE, ~c, ~c, ~c);
	}
	if(IS_NOT_ANK(code)) {
		// use internal font
		for(int y = 0; y < font_height; y++) {
			for(int x = 0; x < font_width; x++) {
				if(ank[code & 0xff][16 * y / font_height][8 * x / font_width]) {
					emu->draw_point_to_bitmap(&bitmap_line[color_mode], dest_line_x + gap_p1 + x, dest_line_y +  y, c, c, c);
				}
			}
		}
	} else if(IS_KANA(code) && hiragana_mode) {
		// use internal font
		for(int y = 0; y < font_height; y++) {
			for(int x = 0; x < font_width; x++) {
				if(bold) {
					if(hiragana_bold[(code & 0xff) - 0xa6][48 * y / font_height][24 * x / font_width]) {
						emu->draw_point_to_bitmap(&bitmap_line[color_mode], dest_line_x + gap_p1 + x, dest_line_y +  y, c, c, c);
					}
				} else {
					if(hiragana[(code & 0xff) - 0xa6][48 * y / font_height][24 * x / font_width]) {
						emu->draw_point_to_bitmap(&bitmap_line[color_mode], dest_line_x + gap_p1 + x, dest_line_y +  y, c, c, c);
					}
				}
			}
		}
	} else if(IS_GAIJI(code)) {
		// use gaiji
		int n1 = (code >> 8) - 0x78;
		int n2 = (code & 0xff) - 0x21;
		for(int y = 0; y < font_height; y++) {
			for(int x = 0; x < font_width; x++) {
				if(gaiji[n1][n2][24 * y / font_height][24 * x / font_width]) {
					emu->draw_point_to_bitmap(&bitmap_line[color_mode], dest_line_x + gap_p1 + x, dest_line_y +  y, c, c, c);
				}
			}
		}
	} else {
		if(font_rotate == 900) {
			dest_line_y += font_width;
		}
		emu->draw_text_to_bitmap(&bitmap_line[color_mode], &font, dest_line_x + gap_p1, dest_line_y, tmp, c, c, c);
	}
	if(underline) {
		for(int x = 0; x < gap_p1 + font_width + gap_p2; x++) {
			if(dest_line_x + x < PIXEL_PER_INCH * 8 - margin_right) {
				if(color_mode) {
					emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x + x, (48 + 1) * DOT_SCALE, 1, DOT_SCALE, 255, 255, 255);
				} else {
					emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x + x, (48 + 1) * DOT_SCALE, 1, DOT_SCALE, 0, 0, 0);
				}
			}
		}
	}
	dest_line_x += gap_p1 + font_width + gap_p2;
	line_printed = true;
}

void MZ1P17::scroll(int value)
{
	dest_paper_y += value;
//	emu->clear_bitmap(&bitmap_line[0], 255, 255, 255);
}

void MZ1P17::finish()
{
	finish_line();
	finish_paper();
	paper_index = 0;
}

void MZ1P17::finish_line()
{
	if(line_printed) {
		int source_y = (double_y_printed ? 0 : 24) * DOT_SCALE;
		int height = bitmap_line[0].height - source_y;
		
		if(dest_paper_y + height > PIXEL_PER_INCH * 11) {
			finish_paper();
		}
		if(color_mode) {
			for(int y = 0; y < bitmap_line[0].height; y++) {
				scrntype *d  = bitmap_line[0].get_buffer(y);
				scrntype *p1 = bitmap_line[1].get_buffer(y);	// cyan
				scrntype *p2 = bitmap_line[2].get_buffer(y);	// magenta
				scrntype *p3 = bitmap_line[3].get_buffer(y);	// yellow
				
				for(int x = 0; x < bitmap_line[0].width; x++) {
					uint8 r = ~R_OF_COLOR(p1[x]);
					uint8 g = ~R_OF_COLOR(p2[x]);
					uint8 b = ~R_OF_COLOR(p3[x]);
					d[x] = RGB_COLOR(r, g, b);
				}
			}
			color_mode--;
		}
		emu->stretch_bitmap(&bitmap_paper, space_left, space_top + dest_paper_y, bitmap_line[0].width, height, &bitmap_line[0], 0, source_y, bitmap_line[0].width, height);
		emu->clear_bitmap(&bitmap_line[0], 255, 255, 255);
		paper_printed = true;
	}
//	int next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
	line_printed = double_y_printed = false;
	dest_line_x = margin_left;
//	scroll(next_lf_pitch); // we should not do line feed here
}

void MZ1P17::finish_paper()
{
	if(paper_printed) {
		_TCHAR file_path[_MAX_PATH];
		
		my_stprintf_s(file_path, _MAX_PATH, _T("%s_#%02d.png"), get_file_path_without_extensiton(base_path), paper_index++);
		emu->write_bitmap_to_file(&bitmap_paper, file_path);
		emu->clear_bitmap(&bitmap_paper, 255, 255, 255);
	}
	paper_printed = false;
	dest_paper_y = 0;
}

#define STATE_VERSION	2

void MZ1P17::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputInt32(value);
	state_fio->FputBool(strobe);
	state_fio->Fwrite(gaiji, sizeof(gaiji), 1);
	state_fio->Fwrite(htab, sizeof(htab), 1);
	fifo->save_state((void *)state_fio);
	state_fio->FputInt32(lf_pitch);
	state_fio->FputInt32(margin_left);
	state_fio->FputInt32(margin_right);
	state_fio->FputInt32(pitch_mode);
	state_fio->FputInt32(script_mode);
	state_fio->FputBool(kanji_mode);
	state_fio->FputBool(kanji_half);
	state_fio->FputBool(bold);
	state_fio->FputBool(underline);
	state_fio->FputBool(hiragana_mode);
	state_fio->FputBool(reverse);
	state_fio->FputBool(vertical);
	state_fio->FputBool(ank_double_x);
	state_fio->FputBool(ank_double_y);
	state_fio->FputBool(kanji_double_x);
	state_fio->FputBool(kanji_double_y);
	state_fio->FputInt32(kanji_pitch);
	state_fio->FputInt32(kanji_half_pitch);
	state_fio->FputInt32(dest_line_x);
	state_fio->FputInt32(dest_paper_y);
	state_fio->FputInt32(color_mode);
	state_fio->FputBool(double_y_printed);
}

bool MZ1P17::load_state(FILEIO* state_fio)
{
	finish();
	
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	value = state_fio->FgetInt32();
	strobe = state_fio->FgetBool();
	state_fio->Fread(gaiji, sizeof(gaiji), 1);
	state_fio->Fread(htab, sizeof(htab), 1);
	if(!fifo->load_state((void *)state_fio)) {
		return false;
	}
	lf_pitch = state_fio->FgetInt32();
	margin_left = state_fio->FgetInt32();
	margin_right = state_fio->FgetInt32();
	pitch_mode = state_fio->FgetInt32();
	script_mode = state_fio->FgetInt32();
	kanji_mode = state_fio->FgetBool();
	kanji_half = state_fio->FgetBool();
	bold = state_fio->FgetBool();
	underline = state_fio->FgetBool();
	hiragana_mode = state_fio->FgetBool();
	reverse = state_fio->FgetBool();
	vertical = state_fio->FgetBool();
	ank_double_x = state_fio->FgetBool();
	ank_double_y = state_fio->FgetBool();
	kanji_double_x = state_fio->FgetBool();
	kanji_double_y = state_fio->FgetBool();
	kanji_pitch = state_fio->FgetInt32();
	kanji_half_pitch = state_fio->FgetInt32();
	dest_line_x = state_fio->FgetInt32();
	dest_paper_y = state_fio->FgetInt32();
	color_mode = state_fio->FgetInt32();
	double_y_printed = state_fio->FgetBool();
	
	// post process
	emu->clear_bitmap(&bitmap_paper, 255, 255, 255);
	emu->clear_bitmap(&bitmap_line[0], 255, 255, 255);
	emu->clear_bitmap(&bitmap_line[1], 0, 0, 0);
	emu->clear_bitmap(&bitmap_line[2], 0, 0, 0);
	emu->clear_bitmap(&bitmap_line[3], 0, 0, 0);
	wait_frames = -1;
	line_printed = paper_printed = false;
	paper_index = 0;
	return true;
}

