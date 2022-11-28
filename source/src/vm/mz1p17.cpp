/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.12.24-

	[ MZ-1P17 ]

	Modify : Hideki Suga
	Date   : 2016.03.18-

	[ MZ-80P3 / MZ-80P4 ]
*/

#include "mz1p17.h"
#include "../fifo.h"

#define EVENT_BUSY	0
#define EVENT_ACK	1

#define PICA		0
#define ELITE		1
#define CONDENSE	2
#define PROPORTIONAL	3

#define SUPER_SCRIPT	1
#define SUB_SCRIPT	-1

// dot impact printer mode
#ifdef MZ1P17_DOT_PRINT
	#define DOT_PRINT	1
#endif
#ifndef DOT_PRINT
	#if defined(_MZ80K) || defined(_MZ1200) || defined(_MZ80A)
		#define DOT_PRINT	1
	#else
		#define DOT_PRINT	0
	#endif
#endif

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
	space_left = (width - PIXEL_PER_INCH * 8) / 2;
	space_top = (height - PIXEL_PER_INCH * 11) / 2;
	
	emu->create_bitmap(&bitmap_paper, width, height);
	emu->create_bitmap(&bitmap_line[0], PIXEL_PER_INCH * 8, (48 + 2) * DOT_SCALE);	// black
	emu->create_bitmap(&bitmap_line[1], PIXEL_PER_INCH * 8, (48 + 2) * DOT_SCALE);	// cyan
	emu->create_bitmap(&bitmap_line[2], PIXEL_PER_INCH * 8, (48 + 2) * DOT_SCALE);	// magenta
	emu->create_bitmap(&bitmap_line[3], PIXEL_PER_INCH * 8, (48 + 2) * DOT_SCALE);	// yellow
//	emu->create_font(&font, _T("Mincho"), 12 * DOT_SCALE, 24 * DOT_SCALE, 0, false, false);
	memset(&font, 0, sizeof(font_t));
	
	// initialize non ank font
	memset(ank, 0, sizeof(ank));
	FILEIO *fio = new FILEIO();
#if defined(_MZ80B) || defined(_MZ2000) || defined(_MZ2200)
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		for(int i = 0; i < 256; i++) {
			for(int j = 0; j < 16; j += 2) {
				uint8_t p = fio->FgetUint8();
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
#elif defined(_MZ80K) || defined(_MZ1200) || defined(_MZ80A) || defined(_MZ700) || defined(_MZ800) || defined(_MZ1500)
	if(fio->Fopen(create_local_path(_T("FONT.ROM")), FILEIO_READ_BINARY)) {
		static const int table[]= {
#if defined(_MZ80A)
			0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6b, 0x6a, 0x2f, 0x2a, 0x2e, 0x2d,
			0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x4f, 0x2c, 0x51, 0x2b, 0x57, 0x49,
			0x55, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x52, 0x59, 0x54, 0x50, 0x45,
			0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xdf, 0xe7, 0xe8, 0xe9, 0xea, 0xec, 0xed,
			0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0x00,
			0x40, 0xbd, 0x9d, 0xb1, 0xb5, 0xb9, 0xb4, 0x9e, 0xb2, 0xb6, 0xba, 0xbe, 0x9f, 0xb3, 0xb7, 0xbb,
			0xbf, 0xa3, 0x85, 0xa4, 0xa5, 0xa6, 0x94, 0x87, 0x88, 0x9c, 0x82, 0x98, 0x84, 0x92, 0x90, 0x83,
			0x91, 0x81, 0x9a, 0x97, 0x93, 0x95, 0x89, 0xa1, 0xaf, 0x8b, 0x86, 0x96, 0xa2, 0xab, 0xaa, 0x8a,
			0x8e, 0xb0, 0xad, 0x8d, 0xa7, 0xa8, 0xa9, 0x8f, 0x8c, 0xae, 0xac, 0x9b, 0xa0, 0x99, 0xbc, 0xb8,
			0x80, 0x3b, 0x3a, 0x70, 0x3c, 0x71, 0x5a, 0x3d, 0x43, 0x56, 0x3f, 0x1e, 0x4a, 0x1c, 0x5d, 0x3e,
			0x5c, 0x1f, 0x5f, 0x5e, 0x37, 0x7b, 0x7f, 0x36, 0x7a, 0x7e, 0x33, 0x4b, 0x4c, 0x1d, 0x6c, 0x5b,
			0x78, 0x41, 0x35, 0x34, 0x74, 0x30, 0x38, 0x75, 0x39, 0x4d, 0x6f, 0x6e, 0x32, 0x77, 0x76, 0x72,
			0x73, 0x47, 0x7c, 0x53, 0x31, 0x4e, 0x6d, 0x48, 0x46, 0x7d, 0x44, 0x1b, 0x58, 0x79, 0x42, 0x60,
#else
			0xf0, 0xf0, 0xf0, 0xf3, 0xf0, 0xf5, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
			0xf0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
			0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6b, 0x6a, 0x2f, 0x2a, 0x2e, 0x2d,
			0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x4f, 0x2c, 0x51, 0x2b, 0x57, 0x49,
			0x55, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
			0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x52, 0x59, 0x54, 0x50, 0x45,
			0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xdf, 0xe7, 0xe8, 0xe9, 0xea, 0xec, 0xed,
			0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xc0,
			0x80, 0xbd, 0x9d, 0xb1, 0xb5, 0xb9, 0xb4, 0x9e, 0xb2, 0xb6, 0xba, 0xbe, 0x9f, 0xb3, 0xb7, 0xbb,
			0xbf, 0xa3, 0x85, 0xa4, 0xa5, 0xa6, 0x94, 0x87, 0x88, 0x9c, 0x82, 0x98, 0x84, 0x92, 0x90, 0x83,
			0x91, 0x81, 0x9a, 0x97, 0x93, 0x95, 0x89, 0xa1, 0xaf, 0x8b, 0x86, 0x96, 0xa2, 0xab, 0xaa, 0x8a,
			0x8e, 0xb0, 0xad, 0x8d, 0xa7, 0xa8, 0xa9, 0x8f, 0x8c, 0xae, 0xac, 0x9b, 0xa0, 0x99, 0xbc, 0xb8,
			0x40, 0x3b, 0x3a, 0x70, 0x3c, 0x71, 0x5a, 0x3d, 0x43, 0x56, 0x3f, 0x1e, 0x4a, 0x1c, 0x5d, 0x3e,
			0x5c, 0x1f, 0x5f, 0x5e, 0x37, 0x7b, 0x7f, 0x36, 0x7a, 0x7e, 0x33, 0x4b, 0x4c, 0x1d, 0x6c, 0x5b,
			0x78, 0x41, 0x35, 0x34, 0x74, 0x30, 0x38, 0x75, 0x39, 0x4d, 0x6f, 0x6e, 0x32, 0x77, 0x76, 0x72,
			0x73, 0x47, 0x7c, 0x53, 0x31, 0x4e, 0x6d, 0x48, 0x46, 0x7d, 0x44, 0x1b, 0x58, 0x79, 0x42, 0x60,
#endif
		};
		for(int i = 0; i < 256; i++) {
			fio->Fseek(table[i] * 8, FILEIO_SEEK_SET);
			for(int j = 0; j < 16; j += 2) {
				uint8_t p = fio->FgetUint8();
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
				uint8_t p = fio->FgetUint8();
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
				uint8_t p = fio->FgetUint8();
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
				uint8_t p = fio->FgetUint8();
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
		uint8_t font[0x1000];
		fio->Fread(font, sizeof(font), 1);
		fio->Fclose();
		memcpy(font + 0x7f * 16, ANKFONT7f_9f, sizeof(ANKFONT7f_9f));
		memcpy(font + 0xe0 * 16, ANKFONTe0_ff, sizeof(ANKFONTe0_ff));
		
		for(int i = 0; i < 256; i++) {
			for(int j = 0; j < 16; j++) {
				uint8_t p = font[i * 16 + j];
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
	
	fifo = new FIFO(65536);
	line_printed = paper_printed = false;
	paper_index = written_length = 0;
	
	value = busy_id = ack_id = wait_frames = -1;
#ifdef PRINTER_STROBE_RISING_EDGE
	strobe = false;
#else
	strobe = true;
#endif
	res = busy = false;
	set_busy(false);
	set_ack(true);
	
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
	emu->clear_bitmap(&bitmap_line[0], 0, 0, 0);
	
	memset(gaiji, 0, sizeof(gaiji));
	memset(htab, 0, sizeof(htab));
	memset(vtab, 0, sizeof(vtab));
	
	fifo->clear();
	
	lf_pitch = PIXEL_PER_INCH / 6;
	prev_esc_6 = true;
	margin_left = 0;
	margin_right = 1440 * DOT_SCALE;
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
	paper_index = written_length = 0;
	
	busy_id = ack_id = wait_frames = -1;
	set_busy(false);
	set_ack(true);
}

void MZ1P17::event_frame()
{
	if(wait_frames > 0 && --wait_frames == 0) {
		finish();
	}
}

void MZ1P17::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_PRINTER_DATA) {
		if(value == -1) {
			value = 0;
		}
		value &= ~mask;
		value |= (data & mask);
	} else if(id == SIG_PRINTER_STROBE) {
		bool new_strobe = ((data & mask) != 0);
#ifdef PRINTER_STROBE_RISING_EDGE
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
			written_length++;
			
			// process
			if(mode == MZ1P17_MODE_MZ1) {
				process_mz1();
			} else if(mode == MZ1P17_MODE_MZ2) {
				process_mz2();
			} else if(mode == MZ1P17_MODE_MZ3) {
				process_mz3();
			} else if(mode == MZ1P17_MODE_X1) {
				process_x1();
			} else if(mode == MZ1P17_MODE_MZ80P4) {
				process_mz80p4();
			}
			
			// busy 1msec
			if(busy_id != -1) {
				cancel_event(this, busy_id);
			}
			register_event(this, EVENT_BUSY, 1000.0, false, &busy_id);
			set_busy(true);
			
			// wait 1sec and finish printing
			wait_frames = (int)(vm->get_frame_rate() * 1.0 + 0.5);
		}
	} else if(id == SIG_PRINTER_RESET) {
		bool new_res = ((data & mask) != 0);
		if(res && !new_res) {
			reset();
		}
		res = new_res;
	}
}

uint32_t MZ1P17::read_signal(int ch)
{
	if(ch == SIG_PRINTER_BUSY) {
		if(busy) {
			if(busy_id != -1) {
				cancel_event(this, busy_id);
				busy_id = -1;
			}
			set_busy(false);
			return 0xffffffff;
		}
	} else if(ch == SIG_PRINTER_ACK) {
		if(ack) {
			return 0xffffffff;
		}
	}
	return 0;
}

void MZ1P17::event_callback(int event_id, int err)
{
	if(event_id == EVENT_BUSY) {
		busy_id = -1;
		set_busy(false);
	} else if(event_id == EVENT_ACK) {
		ack_id = -1;
		set_ack(true);
	}
}

void MZ1P17::set_busy(bool value)
{
	if(busy && !value) {
		// ack 10usec
		if(ack_id != -1) {
			cancel_event(this, ack_id);
		}
		register_event(this, EVENT_ACK, 10.0, false, &ack_id);
		set_ack(false);
	}
	busy = value;
	write_signals(&outputs_busy, busy ? 0xffffffff : 0);
}

void MZ1P17::set_ack(bool value)
{
	ack = value;
	write_signals(&outputs_ack, ack ? 0xffffffff : 0);
}

void MZ1P17::process_mz1()
{
	int n, n1, n2, p, d, d1, d2, d3;
	int next_lf_pitch, next_margin_right;
	uint8_t c = 255;
	
	switch(fifo->read_not_remove(0)) {
	case 0x08:
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
		break;
	case 0x09:
		// HT
		for(int i = dest_line_x + 1; i < array_length(htab); i++) {
			if(htab[i]) {
				dest_line_x = min(i, margin_right);
				break;
			}
		}
		fifo->clear();
		break;
	case 0x11:
		// DC 1 (not supported: select printer)
		fifo->clear();
		break;
	case 0x13:
		// DC 3 (not supported: disselect printer)
		fifo->clear();
		break;
	case 0x0a:
		// LF
		next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
		finish_line();
		scroll(next_lf_pitch);
		fifo->clear();
		break;
	case 0x0c:
		// FF
		finish_line();
		finish_paper();
		fifo->clear();
		break;
	case 0x0d:
		// CR
		if(color_mode) {
			finish_line();
		} else {
			dest_line_x = margin_left;
		}
		fifo->clear();
		break;
	case 0x0e:
		// SO
		ank_double_x = kanji_double_x = true;
		fifo->clear();
		break;
	case 0x0f:
		// SI
		ank_double_x = kanji_double_x = false;
		fifo->clear();
		break;
	case 0x18:
		// CAN (not supported: cancel line buffer)
		fifo->clear();
		break;
	case 0x1b:
		// ESC
		if(fifo->count() >= 2) {
			switch(fifo->read_not_remove(1)) {
			case 0x0b:
				// ESC VT p
				if(fifo->count() == 4) {
					p = (fifo->read_not_remove(2) - '0') * 10 + (fifo->read_not_remove(3) - '0');
					if(p >= 1 && p <= 99) {
						next_lf_pitch = lf_pitch * p + (double_y_printed ? 24 * DOT_SCALE : 0);
						finish_line();
						scroll(next_lf_pitch);
					}
					fifo->clear();
				}
				break;
			case 0x19:
				// ESC EM
				emu->clear_bitmap(&bitmap_line[1], 0, 0, 0);
				emu->clear_bitmap(&bitmap_line[2], 0, 0, 0);
				emu->clear_bitmap(&bitmap_line[3], 0, 0, 0);
				color_mode = 3;
				fifo->clear();
				break;
			case 0x21:
				// ESC !
				bold = true;
				fifo->clear();
				break;
			case 0x22:
				// ESC "
				bold = false;
				fifo->clear();
				break;
			case 0x24:
				// ESC $
				if(fifo->count() >= 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x40:
						// ESC $ @
						kanji_mode = true;
						fifo->clear();
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			case 0x25:
				// ESC %
				if(fifo->count() >= 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x31:
						// ESC % 1 n1 n2 d1 d2 ... dk
						if(fifo->count() >= 5) {
							n = fifo->read_not_remove(3) * 256 + fifo->read_not_remove(4);
							if(fifo->count() == n * 3 + 5) {
								for(int i = 0; i < n; i++) {
									if(dest_line_x < 1440 * DOT_SCALE) {
										if(reverse) {
											emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, DOT_SCALE, 48 * DOT_SCALE, 255, 255, 255);
											c = 0;
										}
										d1 = fifo->read_not_remove(5 + i * 3 + 0);
										d2 = fifo->read_not_remove(5 + i * 3 + 1);
										d3 = fifo->read_not_remove(5 + i * 3 + 2);
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
						break;
					case 0x32:
						// ESC % 2 n1 n2 d1 d2 ... dk
						if(fifo->count() >= 5) {
							n = fifo->read_not_remove(3) * 256 + fifo->read_not_remove(4);
							if(fifo->count() == n * 3 + 5) {
								for(int i = 0; i < n; i++) {
									if(dest_line_x < 1440 * DOT_SCALE) {
										if(reverse) {
											emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, 2 * DOT_SCALE, 48 * DOT_SCALE, 255, 255, 255);
											c = 0;
										}
										d1 = fifo->read_not_remove(5 + i * 3 + 0);
										d2 = fifo->read_not_remove(5 + i * 3 + 1);
										d3 = fifo->read_not_remove(5 + i * 3 + 2);
										if(d1 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  0) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  1) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  2) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  3) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  4) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  5) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  6) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  7) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  8) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  9) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 10) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 11) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 12) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 13) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 14) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 15) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 16) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 17) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 18) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 19) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 20) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 21) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 22) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 23) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										dest_line_x += 2 * DOT_SCALE;
										line_printed = true;
									}
								}
								fifo->clear();
							}
						}
						break;
					case 0x33:
						// ESC % 3 n1 n2
						if(fifo->count() == 5) {
							n = fifo->read_not_remove(3) * 256 + fifo->read_not_remove(4);
							for(int i = 0; i < n; i++) {
								if(dest_line_x < margin_right) {
									if(reverse) {
										emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, DOT_SCALE, bitmap_line[color_mode].height, 255, 255, 255);
									}
									if(underline) {
										emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (48 + 1) * DOT_SCALE, DOT_SCALE, DOT_SCALE, 255, 255, 255);
									}
									dest_line_x += DOT_SCALE;
									line_printed = true;
								}
							}
							fifo->clear();
						}
						break;
					case 0x35:
						// ESC % 5 n
						if(fifo->count() == 4) {
							n = fifo->read_not_remove(3);
							if(n >= 1 && n <= 255) {
								finish_line();
								scroll(PIXEL_PER_INCH * n / 120);
							}
							fifo->clear();
						}
						break;
					case 0x36:
						// ESC % 6 n
						if(fifo->count() == 4) {
							n = fifo->read_not_remove(3);
							if(n >= 1 && n <= 255) {
								lf_pitch = PIXEL_PER_INCH * n / 120;
							}
							fifo->clear();
						}
						break;
					case 0x42:
						// ESC % B
						fifo->clear();
						break;
					case 0x55:
						// ESC % U
						fifo->clear();
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			case 0x26:
				// ESC &
				hiragana_mode = true;
				fifo->clear();
				break;
			case 0x27:
				// ESC '
				hiragana_mode = false;
				fifo->clear();
				break;
			case 0x28:
				// ESC (
				if(fifo->count() >= 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x48:
						// ESC ( H
						kanji_mode = false;
						fifo->clear();
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			case 0x29:
				// ESC ) p1,p2,...,pk.
				if(fifo->read_not_remove(fifo->count() - 1) == 0x2e) {
					for(int i = 2; i < fifo->count(); i += 4) {
						p = (fifo->read_not_remove(i) - '0') * 100 + (fifo->read_not_remove(i + 1) - '0') * 10 + (fifo->read_not_remove(i + 2) - '0');
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
				break;
			case 0x2b:
				// ESC + p1,p2,...,pk.
				if(fifo->read_not_remove(fifo->count() - 1) == 0x2e) {
					for(int i = 2; i < fifo->count(); i += 4) {
						p = (fifo->read_not_remove(i) - '0') * 100 + (fifo->read_not_remove(i + 1) - '0') * 10 + (fifo->read_not_remove(i + 2) - '0');
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
				break;
			case 0x32:
				// ESC 2
				memset(htab, 0, sizeof(htab));
				fifo->clear();
				break;
			case 0x35:
				// ESC 5 (not supported: set tof)
				fifo->clear();
				break;
			case 0x36:
				// ESC 6
				lf_pitch = PIXEL_PER_INCH / 6;
				fifo->clear();
				break;
			case 0x38:
				// ESC 8
				lf_pitch = PIXEL_PER_INCH / 8;
				fifo->clear();
				break;
			case 0x45:
				// ESC E
				pitch_mode = ELITE;
				fifo->clear();
				break;
			case 0x46:
				// ESC F p (not supported: set page length)
				if(fifo->count() == 4) {
					p = (fifo->read_not_remove(2) - '0') * 10 + (fifo->read_not_remove(3) - '0');
					if(p >= 1 && p <= 99) {
					}
					fifo->clear();
				}
				break;
			case 0x48:
				// ESC H
				pitch_mode = PICA;
				fifo->clear();
				break;
			case 0x49:
				// ESC I n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
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
				break;
			case 0x4e:
				// ESC N
//				pitch_mode = PICA;
				fifo->clear();
				break;
			case 0x50:
				// ESC P
				pitch_mode = PROPORTIONAL;
				fifo->clear();
				break;
			case 0x51:
				// ESC Q
				pitch_mode = CONDENSE;
				fifo->clear();
				break;
			case 0x52:
				// ESC R
				ank_double_x = false;
				fifo->clear();
				break;
			case 0x53:
				// ESC S n1 n2
				if(fifo->count() >= 4) {
					n = fifo->read_not_remove(2) * 256 + fifo->read_not_remove(3);
					if(fifo->count() == n + 4) {
						for(int i = 0; i < n; i++) {
							if(dest_line_x < 1280 * DOT_SCALE) {
								if(reverse) {
									emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, 2 * DOT_SCALE, 48 * DOT_SCALE, 255, 255, 255);
									c = 0;
								}
								d = fifo->read_not_remove(4 + i);
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
				break;
			case 0x54:
				// ESC T n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(kanji_mode) {
						if(kanji_half) {
							next_margin_right = n * kanji_half_pitch * DOT_SCALE;
						} else {
							next_margin_right = n * kanji_pitch * DOT_SCALE;
						}
					} else {
						next_margin_right = n * PIXEL_PER_INCH / 10;
					}
					if(next_margin_right < 1440 * DOT_SCALE) {
						margin_right = next_margin_right;
					}
					fifo->clear();
				}
				break;
			case 0x55:
				// ESC U
				ank_double_x = true;
				fifo->clear();
				break;
			case 0x58:
				// ESC X
				underline = true;
				fifo->clear();
				break;
			case 0x59:
				// ESC Y
				underline = false;
				fifo->clear();
				break;
			case 0x67:
				// ESC g
				reverse = true;
				fifo->clear();
				break;
			case 0x68:
				// ESC h
				reverse = false;
				fifo->clear();
				break;
			default:
				// unknown
				fifo->clear();
				break;
			}
		}
		break;
	case 0x1c:
		// CEX
		if(fifo->count() >= 2) {
			switch(fifo->read_not_remove(1)) {
			case 0x24:
				// CEX $ n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(kanji_half) {
						kanji_half_pitch = n;
					} else {
						kanji_pitch = n;
					}
					fifo->clear();
				}
				break;
			case 0x30:
				// CEX 0 n1 n2 (not supported: set gaiji size)
				if(fifo->count() == 4) {
					// n1, n2 shoud be 24
					fifo->clear();
				}
				break;
			case 0x32:
				// CEX 2 n1 n2 d1 d2 ... d72
				if(fifo->count() == 76) {
					n1 = fifo->read_not_remove(2);
					n2 = fifo->read_not_remove(3);
					if(n1 >= 0x78 && n1 <= 0x7a && n2 >= 0x21 && n2 <= 0x7e) {
						bool tmp[24][24];
						n1 -= 0x78;
						n2 -= 0x21;
						for(int x = 0, p = 4; x < 24; x++) {
							for(int y = 0; y < 24; y += 8) {
								d = fifo->read_not_remove(p++);
								tmp[y + 0][x] = ((d & 0x80) != 0);
								tmp[y + 1][x] = ((d & 0x40) != 0);
								tmp[y + 2][x] = ((d & 0x20) != 0);
								tmp[y + 3][x] = ((d & 0x10) != 0);
								tmp[y + 4][x] = ((d & 0x08) != 0);
								tmp[y + 5][x] = ((d & 0x04) != 0);
								tmp[y + 6][x] = ((d & 0x02) != 0);
								tmp[y + 7][x] = ((d & 0x01) != 0);
							}
						}
						for(int y = 0; y < 48; y++) {
							for(int x = 0; x < 48; x++) {
								gaiji[n1][n2][y / 2][x / 2] = tmp[y][x];
							}
						}
					}
					fifo->clear();
				}
				break;
			case 0x4a:
				// CEX J
				vertical = true;
				fifo->clear();
				break;
			case 0x4b:
				// CEX K
				vertical = false;
				fifo->clear();
				break;
			case 0x4e:
				// CEX N
				script_mode = SUPER_SCRIPT;
				fifo->clear();
				break;
			case 0x4f:
				// CEX O
				script_mode = 0;
				fifo->clear();
				break;
			case 0x50:
				// CEX P
				script_mode = SUB_SCRIPT;
				fifo->clear();
				break;
			case 0x51:
				// CEX Q
				script_mode = 0;
				fifo->clear();
				break;
			case 0x5d:
				// CEX ]
				reset();
				fifo->clear();
				break;
			case 0x5f:
				// CEX _ (not supported: typesetting)
				fifo->clear();
				break;
			case 0x70:
				// CEX p
				kanji_double_x = true;
				fifo->clear();
				break;
			case 0x71:
				// CEX q
				kanji_double_x = false;
				fifo->clear();
				break;
			case 0x72:
				// CEX r
				kanji_half = true;
				fifo->clear();
				break;
			case 0x73:
				// CEX s
				kanji_half = false;
				fifo->clear();
				break;
			case 0x74:
				// CEX t
				ank_double_y = kanji_double_y = true;
				fifo->clear();
				break;
			case 0x75:
				// CEX u
				ank_double_y = kanji_double_y = false;
				fifo->clear();
				break;
			default:
				// unknown
				fifo->clear();
				break;
			}
		}
		break;
	default:
		if(kanji_mode) {
			if(fifo->count() == 2) {
				int code = fifo->read() << 8;
				code += fifo->read();
				draw_char(code);
			}
		} else {
			draw_char(fifo->read());
		}
		break;
	}
}

void MZ1P17::process_mz2()
{
	int n, n1, n2, p, d, d1, d2, d3;
	int width, height;
	int next_lf_pitch, next_dest_line_x, next_margin_right;
	uint8_t c = 255;
	
	switch(fifo->read_not_remove(0)) {
	case 0x01:
		// 01 (not supported: select printer)
		fifo->clear();
		break;
	case 0x02:
		// CAN (not supported: cancel line buffer)
		fifo->clear();
		break;
	case 0x03:
		// 03 (not supported: disselect printer)
		fifo->clear();
		break;
	case 0x07:
		// HT
		for(int i = dest_line_x + 1; i < array_length(htab); i++) {
			if(htab[i]) {
				dest_line_x = min(i, margin_right);
				break;
			}
		}
		fifo->clear();
		break;
	case 0x09:
		// 09
		if(fifo->count() >= 2) {
			switch(fifo->read_not_remove(1)) {
			case 0x09:
				// 09 09
				if(fifo->count() >= 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x09:
						// 09 09 09
						pitch_mode = CONDENSE;
						fifo->clear();
						break;
					case 0x0b:
						// 09 09 0B
						pitch_mode = PICA;
						fifo->clear();
						break;
					case 0x30:
					case 0x31:
					case 0x32:
					case 0x33:
					case 0x34:
					case 0x35:
					case 0x36:
					case 0x37:
					case 0x38:
					case 0x39:
						// 09 09 p (not supported: set page length)
						if(fifo->count() == 4) {
							p = (fifo->read_not_remove(2) - '0') * 10 + (fifo->read_not_remove(3) - '0');
							fifo->clear();
						}
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			default:
				// 09
				lf_pitch = PIXEL_PER_INCH * 24 / 180;
				fifo->read();
				process_mz2();
				break;
			}
		}
		break;
	case 0x0a:
		// 0A
		lf_pitch = PIXEL_PER_INCH / 6;
		fifo->clear();
		break;
	case 0x0b:
		// 0B
		if(fifo->count() >= 2) {
			switch(fifo->read_not_remove(1)) {
			case 0x0b:
				// 0B 0B p1 p2 d1 d2 ... dk
				if(fifo->count() >= 6) {
					int tmp[4];
					for(int i = 0; i < 4; i++) {
						p = fifo->read_not_remove(2 + i);
						if(p >= '0' && p <= '9') {
							tmp[i] = p - '0';
						} else if(p >= 'a' && p <= 'f') {
							tmp[i] = p - 'a' + 10;
						} else if(p >= 'A' && p <= 'F') {
							tmp[i] = p - 'A' + 10;
						} else {
							tmp[i] = 0;
						}
					}
					p = (tmp[0] * 16 + tmp[1]) + (tmp[2] * 16 + tmp[3]) * 256;
					if(fifo->count() == 6 + p) {
						if(line_printed) {
							next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
							finish_line();
							scroll(next_lf_pitch);
						}
						if(pitch_mode == ELITE) {
							width = 2 * DOT_SCALE;
						} else if(pitch_mode == CONDENSE) {
							width = (int)(1.5 * DOT_SCALE);
						} else {
							width = 3 * DOT_SCALE;
						}
						if(lf_pitch == PIXEL_PER_INCH / 120) {
							height = 2 * DOT_SCALE;
						} else {
							height = 3 * DOT_SCALE;
						}
						for(int i = 0; i < p; i++) {
							if(dest_line_x < 1440 * DOT_SCALE) {
								if(reverse) {
									emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, width, 48 * DOT_SCALE, 255, 255, 255);
									c = 0;
								}
								d = fifo->read_not_remove(6 + i);
								if(d & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 0) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 1) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 2) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 3) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 4) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 5) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 6) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 7) * DOT_SCALE, width, height, c, c, c);
								dest_line_x += width;
								line_printed = true;
							}
						}
						fifo->clear();
					}
				}
				break;
			default:
				// 0B
				ank_double_x = kanji_double_x = true;
				fifo->read();
				process_mz2();
				break;
			}
		}
		break;
	case 0x0c:
		// 0C
		ank_double_x = kanji_double_x = false;
		fifo->clear();
		break;
	case 0x0d:
		// CR
		next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
		finish_line();
		scroll(next_lf_pitch);
		fifo->clear();
		break;
	case 0x0f:
		// FF
		finish_line();
		finish_paper();
		fifo->clear();
		break;
	case 0x18:
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
		break;
	case 0x19:
		// 19
		hiragana_mode = true;
		fifo->clear();
		break;
	case 0x1a:
		// 1A
		hiragana_mode = false;
		fifo->clear();
		break;
	case 0x1b:
		// ESC
		if(fifo->count() >= 2) {
			switch(fifo->read_not_remove(1)) {
			case 0x04:
				// ESC 04
				emu->clear_bitmap(&bitmap_line[1], 0, 0, 0);
				emu->clear_bitmap(&bitmap_line[2], 0, 0, 0);
				emu->clear_bitmap(&bitmap_line[3], 0, 0, 0);
				color_mode = 3;
				fifo->clear();
				break;
			case 0x0b:
				// ESC VT p
				if(fifo->count() == 4) {
					p = (fifo->read_not_remove(2) - '0') * 10 + (fifo->read_not_remove(3) - '0');
					if(p >= 1 && p <= 99) {
						next_lf_pitch = lf_pitch * p + (double_y_printed ? 24 * DOT_SCALE : 0);
						finish_line();
						scroll(next_lf_pitch);
					}
					fifo->clear();
				}
				break;
			case 0x10:
				// ESC 10 n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
#ifdef MZ1P17_SW1_4_ON
					n >>= 1;
#endif
					if(n >= 1 && n <= 255) {
						lf_pitch = PIXEL_PER_INCH * n / 120;
					}
					fifo->clear();
				}
				break;
			case 0x18:
				// ESC 18 n1 n2
				if(fifo->count() >= 4) {
					n = fifo->read_not_remove(2) + fifo->read_not_remove(3) * 256;
					if(fifo->count() == n + 4) {
						if(line_printed) {
							next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
							finish_line();
							scroll(next_lf_pitch);
						}
						if(pitch_mode == ELITE) {
							width = 2 * DOT_SCALE;
						} else if(pitch_mode == CONDENSE) {
							width = (int)(1.5 * DOT_SCALE);
						} else {
							width = 3 * DOT_SCALE;
						}
						if(lf_pitch == PIXEL_PER_INCH / 120) {
							height = 2 * DOT_SCALE;
						} else {
							height = 3 * DOT_SCALE;
						}
						for(int i = 0; i < n; i++) {
							if(dest_line_x < 1440 * DOT_SCALE) {
								if(reverse) {
									emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, width, 48 * DOT_SCALE, 255, 255, 255);
									c = 0;
								}
								d = fifo->read_not_remove(4 + i);
								if(d & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 0) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 1) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 2) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 3) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 4) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 5) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 6) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 7) * DOT_SCALE, width, height, c, c, c);
								dest_line_x += width;
								line_printed = true;
							}
						}
						fifo->clear();
					}
				}
				break;
			case 0x19:
				// ESC 19 n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(kanji_mode) {
						if(kanji_half) {
							next_margin_right = margin_left + n * kanji_half_pitch * DOT_SCALE;
						} else {
							next_margin_right = margin_left + n * kanji_pitch * DOT_SCALE;
						}
					} else {
						next_margin_right = margin_left + n * PIXEL_PER_INCH / 10;
					}
					if(next_margin_right < 1440 * DOT_SCALE) {
						margin_right = next_margin_right;
					}
					fifo->clear();
				}
				break;
			case 0x24:
				// ESC $
				if(fifo->count() >= 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x40:
						// ESC $ @
						kanji_mode = true;
						fifo->clear();
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			case 0x25:
				// ESC %
				if(fifo->count() >= 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x31:
						// ESC % 1 n1 n2 d1 d2 ... dk
						if(fifo->count() >= 5) {
							n = fifo->read_not_remove(3) * 256 + fifo->read_not_remove(4);
							if(fifo->count() == n * 3 + 5) {
								for(int i = 0; i < n; i++) {
									if(dest_line_x < 1440 * DOT_SCALE) {
										if(reverse) {
											emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, DOT_SCALE, 48 * DOT_SCALE, 255, 255, 255);
											c = 0;
										}
										d1 = fifo->read_not_remove(5 + i * 3 + 0);
										d2 = fifo->read_not_remove(5 + i * 3 + 1);
										d3 = fifo->read_not_remove(5 + i * 3 + 2);
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
						break;
					case 0x32:
						// ESC % 2 n1 n2 d1 d2 ... dk
						if(fifo->count() >= 5) {
							n = fifo->read_not_remove(3) * 256 + fifo->read_not_remove(4);
							if(fifo->count() == n * 3 + 5) {
								for(int i = 0; i < n; i++) {
									if(dest_line_x < 1440 * DOT_SCALE) {
										if(reverse) {
											emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, 2 * DOT_SCALE, 48 * DOT_SCALE, 255, 255, 255);
											c = 0;
										}
										d1 = fifo->read_not_remove(5 + i * 3 + 0);
										d2 = fifo->read_not_remove(5 + i * 3 + 1);
										d3 = fifo->read_not_remove(5 + i * 3 + 2);
										if(d1 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  0) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  1) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  2) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  3) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  4) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  5) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  6) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d1 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  7) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  8) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  9) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 10) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 11) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 12) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 13) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 14) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d2 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 15) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 16) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 17) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 18) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 19) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 20) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 21) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 22) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										if(d3 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 23) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
										dest_line_x += 2 * DOT_SCALE;
										line_printed = true;
									}
								}
								fifo->clear();
							}
						}
						break;
					case 0x33:
						// ESC % 3 n1 n2
						if(fifo->count() == 5) {
							n = fifo->read_not_remove(3) * 256 + fifo->read_not_remove(4);
							for(int i = 0; i < n; i++) {
								if(dest_line_x < margin_right) {
									if(reverse) {
										emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, DOT_SCALE, bitmap_line[color_mode].height, 255, 255, 255);
									}
									if(underline) {
										emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (48 + 1) * DOT_SCALE, DOT_SCALE, DOT_SCALE, 255, 255, 255);
									}
									dest_line_x += DOT_SCALE;
									line_printed = true;
								}
							}
							fifo->clear();
						}
						break;
					case 0x35:
						// ESC % 5 n
						if(fifo->count() == 4) {
							n = fifo->read_not_remove(3);
							if(n >= 1 && n <= 255) {
								finish_line();
								scroll(PIXEL_PER_INCH * n / 120);
							}
							fifo->clear();
						}
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			case 0x28:
				// ESC (
				if(fifo->count() >= 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x48:
						// ESC ( H
						kanji_mode = false;
						fifo->clear();
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			case 0x29:
				// ESC ) p1,p2,...,pk.
				if(fifo->read_not_remove(fifo->count() - 1) == 0x2e) {
					for(int i = 2; i < fifo->count(); i += 4) {
						p = (fifo->read_not_remove(i) - '0') * 100 + (fifo->read_not_remove(i + 1) - '0') * 10 + (fifo->read_not_remove(i + 2) - '0');
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
				break;
			case 0x2b:
				// ESC + p1,p2,...,pk.
				if(fifo->read_not_remove(fifo->count() - 1) == 0x2e) {
					for(int i = 2; i < fifo->count(); i += 4) {
						p = (fifo->read_not_remove(i) - '0') * 100 + (fifo->read_not_remove(i + 1) - '0') * 10 + (fifo->read_not_remove(i + 2) - '0');
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
				break;
			case 0x2d:
				// ESC - n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(n == 0x00) {
						underline = false;
					} else if(n == 0x01) {
						underline = true;
					}
					fifo->clear();
				}
				break;
			case 0x32:
				// ESC 2
				memset(htab, 0, sizeof(htab));
				fifo->clear();
				break;
			case 0x35:
				// ESC 5 (not supported: set tof)
				fifo->clear();
				break;
			case 0x38:
				// ESC 8
				lf_pitch = PIXEL_PER_INCH / 8;
				fifo->clear();
				break;
			case 0x41:
				// ESC A n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(n >= 1 && n <= 63) {
						lf_pitch = PIXEL_PER_INCH * n / 60;
					}
					fifo->clear();
				}
				break;
			case 0x43:
				// ESC C
				if(fifo->count() >= 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x00:
						// ESC C 00 n (not supported: set page length)
						if(fifo->count() == 4) {
							p = fifo->read_not_remove(3);
							fifo->clear();
						}
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			case 0x49:
				// ESC I n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
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
				break;
			case 0x4a:
				// ESC J n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(n >= 1 && n <= 255) {
						next_dest_line_x = dest_line_x;
						finish_line();
						scroll(PIXEL_PER_INCH * n / 120);
						dest_line_x = next_dest_line_x;
					}
					fifo->clear();
				}
				break;
			case 0x44:
				// ESC D
				fifo->clear();
				break;
			case 0x45:
				// ESC E
				bold = true;
				fifo->clear();
				break;
			case 0x46:
				// ESC F
				bold = false;
				fifo->clear();
				break;
			case 0x47:
				// ESC G
				bold = true;
				fifo->clear();
				break;
			case 0x48:
				// ESC H
				bold = false;
				fifo->clear();
				break;
			case 0x4c:
				// ESC L
				pitch_mode = PROPORTIONAL;
				fifo->clear();
				break;
			case 0x4d:
				// ESC M
				pitch_mode = ELITE;
				fifo->clear();
				break;
			case 0x4e:
				// ESC N n (not supported: enable skip perforation)
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					fifo->clear();
				}
				break;
			case 0x4f:
				// ESC O (not supported: disable skip perforation)
				fifo->clear();
				break;
			case 0x50:
				// ESC P
				pitch_mode = PICA;
				fifo->clear();
				break;
			case 0x51:
				// ESC Q
				ank_double_x = true;
				fifo->clear();
				break;
			case 0x52:
				// ESC R
				ank_double_x = false;
				fifo->clear();
				break;
			case 0x53:
				// ESC S
				fifo->clear();
				break;
			case 0x54:
				// ESC T n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(kanji_mode) {
						if(kanji_half) {
							next_margin_right = n * kanji_half_pitch * DOT_SCALE;
						} else {
							next_margin_right = n * kanji_pitch * DOT_SCALE;
						}
					} else {
						next_margin_right = n * PIXEL_PER_INCH / 10;
					}
					if(next_margin_right < 1440 * DOT_SCALE) {
						margin_right = next_margin_right;
					}
					fifo->clear();
				}
				break;
			case 0x56:
				// ESC V
				pitch_mode = PICA;
				fifo->clear();
				break;
			case 0x57:
				// ESC W n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(n == 0x00) {
						ank_double_x = kanji_double_x = false;
					} else if(n == 0x01) {
						ank_double_x = kanji_double_x = true;
					}
					fifo->clear();
				}
				break;
			case 0x58:
				// ESC X n1 n2
				if(fifo->count() >= 4) {
					n = fifo->read_not_remove(2) + fifo->read_not_remove(3) * 256;
					if(fifo->count() == n + 4) {
						if(line_printed) {
							next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
							finish_line();
							scroll(next_lf_pitch);
						}
						for(int i = 0; i < n; i++) {
							if(dest_line_x < 1440 * DOT_SCALE) {
								if(reverse) {
									emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, 2 * DOT_SCALE, 48 * DOT_SCALE, 255, 255, 255);
									c = 0;
								}
								d = fifo->read_not_remove(4 + i);
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
				break;
			case 0x67:
				// ESC 67
				reverse = true;
				fifo->clear();
				break;
			case 0x68:
				// ESC 68
				reverse = false;
				fifo->clear();
				break;
			default:
				// unknown
				fifo->clear();
				break;
			}
		}
		break;
	case 0x1c:
		// CEX
		if(fifo->count() >= 2) {
			switch(fifo->read_not_remove(1)) {
			case 0x24:
				// CEX $ n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(kanji_half) {
						kanji_half_pitch = n;
					} else {
						kanji_pitch = n;
					}
					fifo->clear();
				}
				break;
			case 0x30:
				// CEX 0 n1 n2 (not supported: set gaiji size)
				if(fifo->count() == 4) {
					// n1, n2 shoud be 24
					fifo->clear();
				}
				break;
			case 0x32:
				// CEX 2 n1 n2 d1 d2 ... d72
				if(fifo->count() == 76) {
					n1 = fifo->read_not_remove(2);
					n2 =  fifo->read_not_remove(3);
					if(n1 >= 0x78 && n1 <= 0x7a && n2 >= 0x21 && n2 <= 0x7e) {
						bool tmp[24][24];
						n1 -= 0x78;
						n2 -= 0x21;
						for(int x = 0, p = 4; x < 24; x++) {
							for(int y = 0; y < 24; y += 8) {
								d = fifo->read_not_remove(p++);
								tmp[y + 0][x] = ((d & 0x80) != 0);
								tmp[y + 1][x] = ((d & 0x40) != 0);
								tmp[y + 2][x] = ((d & 0x20) != 0);
								tmp[y + 3][x] = ((d & 0x10) != 0);
								tmp[y + 4][x] = ((d & 0x08) != 0);
								tmp[y + 5][x] = ((d & 0x04) != 0);
								tmp[y + 6][x] = ((d & 0x02) != 0);
								tmp[y + 7][x] = ((d & 0x01) != 0);
							}
						}
						for(int y = 0; y < 48; y++) {
							for(int x = 0; x < 48; x++) {
								gaiji[n1][n2][y / 2][x / 2] = tmp[y][x];
							}
						}
					}
					fifo->clear();
				}
				break;
			case 0x4a:
				// CEX J
				vertical = true;
				fifo->clear();
				break;
			case 0x4b:
				// CEX K
				vertical = false;
				fifo->clear();
				break;
			case 0x4e:
				// CEX N
				script_mode = SUPER_SCRIPT;
				fifo->clear();
				break;
			case 0x4f:
				// CEX O
				script_mode = 0;
				fifo->clear();
				break;
			case 0x50:
				// CEX P
				script_mode = SUB_SCRIPT;
				fifo->clear();
				break;
			case 0x51:
				// CEX Q
				script_mode = 0;
				fifo->clear();
				break;
			case 0x55:
				// ESC U n (not supported)
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					fifo->clear();
				}
				break;
			case 0x5d:
				// CEX ]
				reset();
				fifo->clear();
				break;
			case 0x5f:
				// CEX _ (not supported: typesetting)
				fifo->clear();
				break;
			case 0x70:
				// CEX 70
				kanji_double_x = true;
				fifo->clear();
				break;
			case 0x71:
				// CEX 71
				kanji_double_x = false;
				fifo->clear();
				break;
			case 0x72:
				// CEX 72
				kanji_half = true;
				fifo->clear();
				break;
			case 0x73:
				// CEX 73
				kanji_half = false;
				fifo->clear();
				break;
			case 0x74:
				// CEX 74
				ank_double_y = kanji_double_y = true;
				fifo->clear();
				break;
			case 0x75:
				// CEX 75
				ank_double_y = kanji_double_y = false;
				fifo->clear();
				break;
			default:
				// unknown
				fifo->clear();
				break;
			}
		}
		break;
	default:
		if(kanji_mode) {
			if(fifo->count() == 2) {
				int code = fifo->read() << 8;
				code += fifo->read();
				draw_char(code);
			}
		} else {
			int code = fifo->read();
			if(hiragana_mode && code >= 'A' && code <= 'Z') {
				code = code - 'A' + 'a';
			}
			draw_char(code);
		}
		break;
	}
}

void MZ1P17::process_mz3()
{
	int n, d;
	int width, height;
	int next_lf_pitch, next_margin_right;
	uint8_t c = 255;
	
	switch(fifo->read_not_remove(0)) {
	case 0x08:
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
		break;
	case 0x09:
		// HT
		for(int i = dest_line_x + 1; i < array_length(htab); i++) {
			if(htab[i]) {
				dest_line_x = min(i, margin_right);
				break;
			}
		}
		fifo->clear();
		break;
	case 0x0a:
		// LF
		next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
		finish_line();
		scroll(next_lf_pitch);
		fifo->clear();
		break;
	case 0x0b:
		// VT (not supported: vertical tab)
		next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
		finish_line();
		scroll(next_lf_pitch);
		fifo->clear();
		break;
	case 0x0c:
		// FF (not supported: set tof)
		fifo->clear();
		break;
	case 0x0d:
		// CR
		if(color_mode) {
			finish_line();
		} else {
			dest_line_x = margin_left;
		}
		fifo->clear();
		break;
	case 0x0e:
		// SO
		ank_double_x = kanji_double_x = true;
		fifo->clear();
		break;
	case 0x0f:
		// SI
		pitch_mode = CONDENSE;
		fifo->clear();
		break;
	case 0x12:
		// 12
		pitch_mode = PICA;
		fifo->clear();
		break;
	case 0x14:
		// 14
		ank_double_x = kanji_double_x = false;
		fifo->clear();
		break;
	case 0x18:
		// CAN (not supported: cancel line buffer)
		fifo->clear();
		break;
	case 0x1b:
		// ESC
		if(fifo->count() >= 2) {
			switch(fifo->read_not_remove(1)) {
			case 0x00:
				// ESC 0
				lf_pitch = PIXEL_PER_INCH * 2 / 15;
				fifo->clear();
				break;
			case 0x01:
				// ESC 1
				lf_pitch = PIXEL_PER_INCH / 8;
				fifo->clear();
				break;
			case 0x02:
				// ESC 2
				lf_pitch = PIXEL_PER_INCH / 6;
				fifo->clear();
				break;
			case 0x05:
				// ESC 5
				lf_pitch = PIXEL_PER_INCH * 7 / 60;
				fifo->clear();
				break;
			case 0x08:
				// ESC 8 (not supported: disable page empty)
				fifo->clear();
				break;
			case 0x09:
				// ESC 9 (not supported: enable page empty)
				fifo->clear();
				break;
			case 0x10:
				// ESC 10 n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
//#ifdef MZ1P17_SW1_4_ON
//					n >>= 1;
//#endif
					if(n >= 1 && n <= 255) {
						lf_pitch = PIXEL_PER_INCH * n / 120;
					}
					fifo->clear();
				}
				break;
			case 0x12:
				// ESC 12 n (not supported: set page length)
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					fifo->clear();
				}
				break;
			case 0x13:
				// ESC 13 n1,n2,...,00
				if(fifo->read_not_remove(fifo->count() - 1) == 0x00) {
					for(int i = 2; i < fifo->count() - 1; i++) {
						n = fifo->read_not_remove(i);
						if(n >= 1) {
							n -= 1;
							if(kanji_mode) {
								if(kanji_half) {
									n *= kanji_half_pitch * DOT_SCALE;
								} else {
									n *= kanji_pitch * DOT_SCALE;
								}
							} else {
								n *= PIXEL_PER_INCH / 10;
							}
							if(n < array_length(htab)) {
								htab[n] = true;
							}
						}
					}
					fifo->clear();
				}
				break;
			case 0x18:
				// ESC 18 n1 n2
				if(fifo->count() >= 4) {
					n = fifo->read_not_remove(2) + fifo->read_not_remove(3) * 256;
					if(fifo->count() == n + 4) {
						if(line_printed) {
							next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
							finish_line();
							scroll(next_lf_pitch);
						}
						if(pitch_mode == ELITE) {
							width = 2 * DOT_SCALE;
						} else if(pitch_mode == CONDENSE) {
							width = (int)(1.5 * DOT_SCALE);
						} else {
							width = 3 * DOT_SCALE;
						}
						if(lf_pitch == PIXEL_PER_INCH / 120) {
							height = 2 * DOT_SCALE;
						} else {
							height = 3 * DOT_SCALE;
						}
						for(int i = 0; i < n; i++) {
							if(dest_line_x < 1440 * DOT_SCALE) {
								if(reverse) {
									emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, width, 48 * DOT_SCALE, 255, 255, 255);
									c = 0;
								}
								d = fifo->read_not_remove(4 + i);
								if(d & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 0) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 1) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 2) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 3) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 4) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 5) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 6) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 7) * DOT_SCALE, width, height, c, c, c);
								dest_line_x += width;
								line_printed = true;
							}
						}
						fifo->clear();
					}
				}
				break;
			case 0x19:
				// ESC 19 n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(kanji_mode) {
						if(kanji_half) {
							next_margin_right = margin_left + n * kanji_half_pitch * DOT_SCALE;
						} else {
							next_margin_right = margin_left + n * kanji_pitch * DOT_SCALE;
						}
					} else {
						next_margin_right = margin_left + n * PIXEL_PER_INCH / 10;
					}
					if(next_margin_right < 1440 * DOT_SCALE) {
						margin_right = next_margin_right;
					}
					fifo->clear();
				}
				break;
			case 0x2d:
				// ESC - n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(n == 0x00) {
						underline = false;
					} else if(n == 0x01) {
						underline = true;
					}
					fifo->clear();
				}
				break;
			case 0x41:
				// ESC A n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(n >= 1 && n <= 255) {
						lf_pitch = PIXEL_PER_INCH * n / 60;
					}
					fifo->clear();
				}
				break;
			case 0x43:
				// ESC C
				if(fifo->count() >= 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x00:
						// ESC C 00 n (not supported: set page length)
						if(fifo->count() == 4) {
							fifo->clear();
						}
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			case 0x45:
				// ESC E
				bold = true;
				fifo->clear();
				break;
			case 0x46:
				// ESC F
				bold = false;
				fifo->clear();
				break;
			case 0x47:
				// ESC G
				bold = true;
				fifo->clear();
				break;
			case 0x48:
				// ESC H
				bold = false;
				fifo->clear();
				break;
			case 0x4a:
				// ESC J n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(n >=1 && n <= 255) {
						next_lf_pitch = lf_pitch * n + (double_y_printed ? 24 * DOT_SCALE : 0);
						finish_line();
						scroll(next_lf_pitch);
					}
					fifo->clear();
				}
				break;
			case 0x4e:
				// ESC N n (not supported: enable skip perforation)
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					fifo->clear();
				}
				break;
			case 0x4f:
				// ESC O (not supported: disable skip perforation)
				fifo->clear();
				break;
			case 0x55:
				// ESC U n (not supported)
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					fifo->clear();
				}
				break;
			case 0x57:
				// ESC W n
				if(fifo->count() == 3) {
					n = fifo->read_not_remove(2);
					if(n == 0x00) {
						ank_double_x = kanji_double_x = false;
					} else if(n == 0x01) {
						ank_double_x = kanji_double_x = true;
					}
					fifo->clear();
				}
				break;
			case 0x58:
				// ESC X n1 n2
				if(fifo->count() >= 4) {
					n = fifo->read_not_remove(2) + fifo->read_not_remove(3) * 256;
					if(fifo->count() == n + 4) {
						if(line_printed) {
							next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
							finish_line();
							scroll(next_lf_pitch);
						}
						for(int i = 0; i < n; i++) {
							if(dest_line_x < 1440 * DOT_SCALE) {
								if(reverse) {
									emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, 3 * DOT_SCALE, 48 * DOT_SCALE, 255, 255, 255);
									c = 0;
								}
								d = fifo->read_not_remove(4 + i);
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
				break;
			default:
				// unknown
				fifo->clear();
				break;
			}
		}
		break;
	default:
		if(kanji_mode) {
			if(fifo->count() == 2) {
				int code = fifo->read() << 8;
				code += fifo->read();
				draw_char(code);
			}
		} else {
			draw_char(fifo->read());
		}
		break;
	}
}

void MZ1P17::process_x1()
{
	int n, n1, n2, p, d, d1, d2, d3;
	int width, height, top = 24, py = 3;
	int next_lf_pitch, next_margin_right, next_margin_left;
	uint8_t c = 255;
	
	switch(fifo->read_not_remove(0)) {
	case 0x00:
		// 0 n
		if(fifo->count() == 2) {
			n = fifo->read_not_remove(1);
			bool tmp = kanji_half;
			kanji_half = true;
			draw_char(n);
			kanji_half = tmp;
			fifo->clear();
		}
		break;
	case 0x08:
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
		break;
	case 0x09:
		// HT
		for(int i = dest_line_x + 1; i < array_length(htab); i++) {
			if(htab[i]) {
				dest_line_x = min(i, margin_right);
				break;
			}
		}
		fifo->clear();
		break;
	case 0x0a:
		// LF
		next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
		finish_line();
		scroll(next_lf_pitch);
		fifo->clear();
		break;
	case 0x0b:
		// VT n
		if(fifo->count() == 2) {
			n = fifo->read_not_remove(1);
			if(n >= '1' && n <= '>') {
				n -= '1';
				if(vtab[n].active) {
					finish_line();
					dest_paper_y = vtab[n].y;
				}
			}
			fifo->clear();
		}
		break;
	case 0x0c:
		// FF
		finish_line();
		finish_paper();
		fifo->clear();
		break;
	case 0x0d:
		// CR
		if(color_mode) {
			finish_line();
		} else {
			dest_line_x = margin_left;
		}
		fifo->clear();
		break;
	case 0x0e:
		// SO
		ank_double_x = true;
		fifo->clear();
		break;
	case 0x0f:
		// SI
		ank_double_x = false;
		fifo->clear();
		break;
	case 0x10:
		// POS p
		if(fifo->count() == 4) {
			p = (fifo->read_not_remove(1) - '0') * 100 + (fifo->read_not_remove(2) - '0') * 10 + (fifo->read_not_remove(3) - '0');
			if(kanji_mode) {
				dest_line_x = p * kanji_half_pitch * DOT_SCALE;
			} else {
				if(pitch_mode == ELITE) {
					dest_line_x = p * PIXEL_PER_INCH / 12;
				} else if(pitch_mode == CONDENSE) {
					dest_line_x = p * PIXEL_PER_INCH / 17;
				} else {
					dest_line_x = p * PIXEL_PER_INCH / 10;
				}
			}
			fifo->clear();
		}
		break;
	case 0x11:
		// DC1 (not supported: select printer)
		fifo->clear();
		break;
	case 0x13:
		// DC3 (not supported: disselect printer)
		fifo->clear();
		break;
	case 0x14:
		// DC4 sp ... sp n1 sp ...sp n2 ... nk ?
		if(fifo->read_not_remove(fifo->count() - 1) == 0x3f) {
			for(int i = 1; i < fifo->count() - 1; i++) {
				n = fifo->read_not_remove(i);
				if(n >= '1' && n<= '>') {
					n -= '1';
					vtab[n].y = dest_paper_y + lf_pitch * (i - 1);
					vtab[n].active = true;
				}
			}
			fifo->clear();
		}
		break;
	case 0x18:
		// CAN (not supported: cancel line buffer)
		fifo->clear();
		break;
	case 0x1a:
		// SUB
		if(fifo->count() >= 2) {
			switch(fifo->read_not_remove(1)) {
			case 0x56:
				// SUB V
				ank_double_y = kanji_double_y = true;
				fifo->clear();
				break;
			case 0x57:
				// SUB W
				ank_double_y = kanji_double_y = false;
				fifo->clear();
				break;
			default:
				// unknown
				fifo->clear();
				break;
			}
		}
		break;
	case 0x1b:
		// ESC
		if(fifo->count() >= 2) {
			switch(fifo->read_not_remove(1)) {
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
			case 0x06:
				// ESC n
				n = fifo->read_not_remove(1);
				if(kanji_mode ? kanji_double_x : ank_double_x) {
					n *= 2;
				}
				for(int i = 0; i < n; i++) {
					if(dest_line_x < margin_right) {
						if(reverse) {
							emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, DOT_SCALE, bitmap_line[color_mode].height, 255, 255, 255);
						}
						if(underline) {
							emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (48 + 1) * DOT_SCALE, DOT_SCALE, DOT_SCALE, 255, 255, 255);
						}
						dest_line_x += DOT_SCALE;
						line_printed = true;
					}
				}
				fifo->clear();
				break;
			case 0x0b:
				// ESC VT p
				if(fifo->count() == 4) {
					p = (fifo->read_not_remove(2) - '0') * 10 + (fifo->read_not_remove(3) - '0');
					if(p >= 1 && p <= 99) {
						next_lf_pitch = lf_pitch * p + (double_y_printed ? 24 * DOT_SCALE : 0);
						finish_line();
						scroll(next_lf_pitch);
					}
					fifo->clear();
				}
				break;
			case 0x10:
				// ESC POS p
				if(fifo->count() == 6) {
					p = (fifo->read_not_remove(2) - '0') * 1000 + (fifo->read_not_remove(3) - '0') * 100 + (fifo->read_not_remove(4) - '0') * 10 + (fifo->read_not_remove(5) - '0');
					if(p >= 0 && p <= 9999) {
						dest_line_x = margin_left + p * DOT_SCALE;
					}
					fifo->clear();
				}
				break;
			case 0x21:
				// ESC !
				bold = true;
				fifo->clear();
				break;
			case 0x22:
				// ESC "
				bold = false;
				fifo->clear();
				break;
			case 0x24:
				// ESC $
				hiragana_mode = false;
				fifo->clear();
				break;
			case 0x25:
				// ESC %
				if(fifo->count() >= 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x32:
						// ESC % 2 n1 n2
						if(fifo->count() >= 5) {
							n = fifo->read_not_remove(3) * 256 + fifo->read_not_remove(4);
							if(fifo->count() == n + 5) {
								if(pitch_mode == ELITE) {
									width = (int)(1.5 * DOT_SCALE);
								} else {
									if(lf_pitch == PIXEL_PER_INCH * 15 / 120) {
										py = 2;
									}
									width = 2 * DOT_SCALE;
								}
								height = 2 * DOT_SCALE;
								
								if(ank_double_x) {
									width *= 2;
								}
								if(ank_double_y) {
									height *= 2;
									top = 0;
									py *= 2;
									double_y_printed = true;
								}
								for(int i = 0; i < n; i++) {
									if(dest_line_x < 1440 * DOT_SCALE) {
										if(reverse) {
											emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, width, 48 * DOT_SCALE, 255, 255, 255);
											c = 0;
										}
										d = fifo->read_not_remove(5 + i);
										if(d & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 0 * py) * DOT_SCALE, width, height, c, c, c);
										if(d & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 1 * py) * DOT_SCALE, width, height, c, c, c);
										if(d & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 2 * py) * DOT_SCALE, width, height, c, c, c);
										if(d & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 3 * py) * DOT_SCALE, width, height, c, c, c);
										if(d & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 4 * py) * DOT_SCALE, width, height, c, c, c);
										if(d & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 5 * py) * DOT_SCALE, width, height, c, c, c);
										if(d & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 6 * py) * DOT_SCALE, width, height, c, c, c);
										if(d & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 7 * py) * DOT_SCALE, width, height, c, c, c);
										dest_line_x += width;
										line_printed = true;
									}
								}
								next_lf_pitch = (double_y_printed ? 16 : 8) * py * DOT_SCALE;
								finish_line();
								scroll(next_lf_pitch);
								fifo->clear();
							}
						}
						break;
					case 0x39:
						// ESC % 9 n
						if(fifo->count() == 4) {
							n = fifo->read_not_remove(3);
							if(n == 0) {
								if(prev_esc_6) {
									lf_pitch = PIXEL_PER_INCH / 6;
								} else {
									lf_pitch = PIXEL_PER_INCH / 8;
								}
							} else if(n >= 1 && n <= 255) {
								lf_pitch = PIXEL_PER_INCH * n / 120;
							}
							fifo->clear();
						}
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			case 0x26:
				// ESC &
				hiragana_mode = true;
				fifo->clear();
				break;
			case 0x28:
				// ESC ( p1,p2,...,pk.
				if(fifo->read_not_remove(fifo->count() - 1) == 0x2e) {
					for(int i = 2; i < fifo->count(); i += 4) {
						p = (fifo->read_not_remove(i) - '0') * 100 + (fifo->read_not_remove(i + 1) - '0') * 10 + (fifo->read_not_remove(i + 2) - '0');
						if(p >= 1 && p <= 137) {
							p -= 1;
							if(kanji_mode) {
								p *= kanji_half_pitch * DOT_SCALE;
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
				break;
			case 0x29:
				// ESC ) p1,p2,...,pk.
				if(fifo->read_not_remove(fifo->count() - 1) == 0x2e) {
					for(int i = 2; i < fifo->count(); i += 4) {
						p = (fifo->read_not_remove(i) - '0') * 100 + (fifo->read_not_remove(i + 1) - '0') * 10 + (fifo->read_not_remove(i + 2) - '0');
						if(p >= 1 && p <= 137) {
							p -= 1;
							if(kanji_mode) {
								p *= kanji_half_pitch * DOT_SCALE;
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
				break;
			case 0x2a:
				// ESC * n1 n2 d1 d2 ... d32
				if(fifo->count() == 36) {
					n1 = fifo->read_not_remove(2);
					n2 = fifo->read_not_remove(3);
					if(n1 >= 0x78 && n1 <= 0x7a && n2 >= 0x21 && n2 <= 0x7e) {
						bool tmp[16][16];
						n1 -= 0x78;
						n2 -= 0x21;
						for(int x = 0, p = 4; x < 16; x++) {
							for(int y = 0; y < 16; y += 8) {
								d = fifo->read_not_remove(p++);
								tmp[y + 0][x] = ((d & 0x80) != 0);
								tmp[y + 1][x] = ((d & 0x40) != 0);
								tmp[y + 2][x] = ((d & 0x20) != 0);
								tmp[y + 3][x] = ((d & 0x10) != 0);
								tmp[y + 4][x] = ((d & 0x08) != 0);
								tmp[y + 5][x] = ((d & 0x04) != 0);
								tmp[y + 6][x] = ((d & 0x02) != 0);
								tmp[y + 7][x] = ((d & 0x01) != 0);
							}
						}
						for(int y = 0; y < 48; y++) {
							for(int x = 0; x < 48; x++) {
								gaiji[n1][n2][y / 3][x / 3] = tmp[y][x];
							}
						}
					}
					fifo->clear();
				}
				break;
			case 0x2b:
				// ESC + n1 n2 d1 d2 ... d72
				if(fifo->count() == 76) {
					n1 = fifo->read_not_remove(2);
					n2 = fifo->read_not_remove(3);
					if(n1 >= 0x78 && n1 <= 0x7a && n2 >= 0x21 && n2 <= 0x7e) {
						bool tmp[24][24];
						n1 -= 0x78;
						n2 -= 0x21;
						for(int x = 0, p = 4; x < 24; x++) {
							for(int y = 0; y < 24; y += 8) {
								d = fifo->read_not_remove(p++);
								tmp[y + 0][x] = ((d & 0x80) != 0);
								tmp[y + 1][x] = ((d & 0x40) != 0);
								tmp[y + 2][x] = ((d & 0x20) != 0);
								tmp[y + 3][x] = ((d & 0x10) != 0);
								tmp[y + 4][x] = ((d & 0x08) != 0);
								tmp[y + 5][x] = ((d & 0x04) != 0);
								tmp[y + 6][x] = ((d & 0x02) != 0);
								tmp[y + 7][x] = ((d & 0x01) != 0);
							}
						}
						for(int y = 0; y < 48; y++) {
							for(int x = 0; x < 48; x++) {
								gaiji[n1][n2][y / 2][x / 2] = tmp[y][x];
							}
						}
					}
					fifo->clear();
				}
				break;
			case 0x2f:
				// ESC / p
				if(fifo->count() == 5) {
					p = (fifo->read_not_remove(2) - '0') * 100 + (fifo->read_not_remove(3) - '0') * 10 + (fifo->read_not_remove(4) - '0');
					if(p >= 0 && p <= 999) {
						if(kanji_mode) {
							next_margin_right = p * kanji_half_pitch * DOT_SCALE;
						} else {
							next_margin_right = p * PIXEL_PER_INCH / 10;
						}
						if(next_margin_right < 1440 * DOT_SCALE) {
							margin_right = next_margin_right;
						}
					}
					fifo->clear();
				}
				break;
			case 0x35:
				// ESC 5 (not supported: set tof)
				fifo->clear();
				break;
			case 0x36:
				// ESC 6
				lf_pitch = PIXEL_PER_INCH / 6;
				prev_esc_6 = true;
				fifo->clear();
				break;
			case 0x32:
				// ESC 2
				memset(htab, 0, sizeof(htab));
				fifo->clear();
				break;
			case 0x38:
				// ESC 8
				lf_pitch = PIXEL_PER_INCH / 8;
				prev_esc_6 = false;
				fifo->clear();
				break;
			case 0x3e:
				// ESC > (not supported)
				fifo->clear();
				break;
			case 0x43:
				// ESC C p (not supported: set bottom margin)
				if(fifo->count() == 4) {
					p = (fifo->read_not_remove(2) - '0') * 10 + (fifo->read_not_remove(3) - '0');
					if(p >= 0 && p <= 99) {
						memset(vtab, 0, sizeof(vtab));
					}
					fifo->clear();
				}
				break;
			case 0x45:
				// ESC E
				pitch_mode = ELITE;
				ank_double_x = false;
				fifo->clear();
				break;
			case 0x46:
				// ESC F p (not supported: set page length)
				if(fifo->count() == 4) {
					p = (fifo->read_not_remove(2) - '0') * 10 + (fifo->read_not_remove(3) - '0');
					if(p >= 1 && p <= 99) {
						memset(vtab, 0, sizeof(vtab));
					}
					fifo->clear();
				}
				break;
			case 0x48:
				// ESC H
				kanji_mode = false;
				fifo->clear();
				break;
			case 0x49:
				// ESC I p d1 d2 ... dk
				if(fifo->count() >= 6) {
					p = (fifo->read_not_remove(2) - '0') * 1000 + (fifo->read_not_remove(3) - '0') * 100 + (fifo->read_not_remove(4) - '0') * 10 + (fifo->read_not_remove(5) - '0');
					if(fifo->count() == p * 2 + 6) {
						if(pitch_mode == ELITE) {
							width = (int)(1.5 * DOT_SCALE);
						} else {
							width = 2 * DOT_SCALE;
						}
						height = 2 * DOT_SCALE;
						py = 1;
						
						if(ank_double_x) {
							width *= 2;
						}
						if(ank_double_y) {
							height *= 2;
							top = 0;
							py *= 2;
							double_y_printed = true;
						}
						for(int i = 0; i < p * 2; i += 2) {
							if(dest_line_x < 1440 * DOT_SCALE) {
								if(reverse) {
									emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, width, 48 * DOT_SCALE, 255, 255, 255);
									c = 0;
								}
								d1 = fifo->read_not_remove(6 + i + 0);
								d2 = fifo->read_not_remove(6 + i + 1);
								if(d1 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  0 * py) * DOT_SCALE, width, height, c, c, c);
								if(d1 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  1 * py) * DOT_SCALE, width, height, c, c, c);
								if(d1 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  3 * py) * DOT_SCALE, width, height, c, c, c);
								if(d1 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  4 * py) * DOT_SCALE, width, height, c, c, c);
								if(d1 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  6 * py) * DOT_SCALE, width, height, c, c, c);
								if(d1 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  7 * py) * DOT_SCALE, width, height, c, c, c);
								if(d1 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  9 * py) * DOT_SCALE, width, height, c, c, c);
								if(d1 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 10 * py) * DOT_SCALE, width, height, c, c, c);
								if(d2 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 12 * py) * DOT_SCALE, width, height, c, c, c);
								if(d2 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 13 * py) * DOT_SCALE, width, height, c, c, c);
								if(d2 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 15 * py) * DOT_SCALE, width, height, c, c, c);
								if(d2 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 16 * py) * DOT_SCALE, width, height, c, c, c);
								if(d2 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 18 * py) * DOT_SCALE, width, height, c, c, c);
								if(d2 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 19 * py) * DOT_SCALE, width, height, c, c, c);
								if(d2 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 21 * py) * DOT_SCALE, width, height, c, c, c);
								if(d2 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 22 * py) * DOT_SCALE, width, height, c, c, c);
								dest_line_x += width;
								line_printed = true;
							}
						}
						next_lf_pitch = (double_y_printed ? 48 : 24) * DOT_SCALE;
						finish_line();
						scroll(next_lf_pitch);
						fifo->clear();
					}
				}
				break;
			case 0x4a:
				// ESC J n1 n2 d1 d2 ... dk
				if(fifo->count() >= 4) {
					n = fifo->read_not_remove(2) * 256 + fifo->read_not_remove(3);
					if(fifo->count() == n * 3 + 4) {
						for(int i = 0; i < n; i++) {
							if(dest_line_x < 1440 * DOT_SCALE) {
								if(reverse) {
									emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, 2 * DOT_SCALE, 48 * DOT_SCALE, 255, 255, 255);
									c = 0;
								}
								d1 = fifo->read_not_remove(4 + i * 3 + 0);
								d2 = fifo->read_not_remove(4 + i * 3 + 1);
								d3 = fifo->read_not_remove(4 + i * 3 + 2);
								if(d1 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  0) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d1 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  1) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d1 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  2) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d1 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  3) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d1 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  4) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d1 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  5) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d1 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  6) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d1 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  7) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d2 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  8) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d2 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 +  9) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d2 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 10) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d2 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 11) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d2 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 12) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d2 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 13) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d2 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 14) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d2 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 15) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d3 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 16) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d3 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 17) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d3 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 18) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d3 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 19) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d3 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 20) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d3 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 21) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d3 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 22) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								if(d3 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (24 + 23) * DOT_SCALE, 2 * DOT_SCALE, DOT_SCALE, c, c, c);
								dest_line_x += 2 * DOT_SCALE;
								line_printed = true;
							}
						}
						next_lf_pitch = (double_y_printed ? 48 : 24) * DOT_SCALE;
						finish_line();
						scroll(next_lf_pitch);
						fifo->clear();
					}
				}
				break;
			case 0x4b:
				// ESC K
				kanji_mode = true;
				fifo->clear();
				break;
			case 0x4c:
				// ESC L p
				if(fifo->count() == 5) {
					p = (fifo->read_not_remove(2) - '0') * 100 + (fifo->read_not_remove(3) - '0') * 10 + (fifo->read_not_remove(4) - '0');
					if(p >= 0 && p <= 999) {
						if(kanji_mode) {
							next_margin_left = p * kanji_half_pitch * DOT_SCALE;
						} else {
							next_margin_left = p * PIXEL_PER_INCH / 10;
						}
						if(next_margin_left < 1440 * DOT_SCALE) {
							margin_left = next_margin_left;
						}
					}
					fifo->clear();
				}
				break;
			case 0x4e:
				// ESC N p d
				if(fifo->count() == 6) {
					p = (fifo->read_not_remove(2) - '0') * 100 + (fifo->read_not_remove(3) - '0') * 10 + (fifo->read_not_remove(4) - '0');
					draw_char(fifo->read_not_remove(5));
					fifo->clear();
				}
				break;
			case 0x50:
				// ESC P
				kanji_mode = false;
				fifo->clear();
				break;
			case 0x51:
				// ESC Q
				pitch_mode = CONDENSE;
				ank_double_x = false;
				fifo->clear();
				break;
			case 0x52:
				// ESC R
				pitch_mode = PICA;
				ank_double_x = false;
				fifo->clear();
				break;
			case 0x55:
				// ESC U
				ank_double_x = true;
				fifo->clear();
				break;
			case 0x56:
				// ESC V p d
				if(fifo->count() == 7) {
					p = (fifo->read_not_remove(2) - '0') * 1000 + (fifo->read_not_remove(3) - '0') * 100 + (fifo->read_not_remove(4) - '0') * 10 + (fifo->read_not_remove(5) - '0');
					d = fifo->read_not_remove(6);
					
					if(pitch_mode == ELITE) {
						width = (int)(1.5 * DOT_SCALE);
					} else {
						if(lf_pitch == PIXEL_PER_INCH * 15 / 120) {
							py = 2;
						}
						width = 2 * DOT_SCALE;
					}
					height = 2 * DOT_SCALE;
					
					if(ank_double_x) {
						width *= 2;
					}
					if(ank_double_y) {
						height *= 2;
						top = 0;
						py *= 2;
						double_y_printed = true;
					}
					for(int i = 0; i < p; i++) {
						if(dest_line_x < 1440 * DOT_SCALE) {
							if(reverse) {
								emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, width, 48 * DOT_SCALE, 255, 255, 255);
								c = 0;
							}
							if(d & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 0 * py) * DOT_SCALE, width, height, c, c, c);
							if(d & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 1 * py) * DOT_SCALE, width, height, c, c, c);
							if(d & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 2 * py) * DOT_SCALE, width, height, c, c, c);
							if(d & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 3 * py) * DOT_SCALE, width, height, c, c, c);
							if(d & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 4 * py) * DOT_SCALE, width, height, c, c, c);
							if(d & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 5 * py) * DOT_SCALE, width, height, c, c, c);
							if(d & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 6 * py) * DOT_SCALE, width, height, c, c, c);
							if(d & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 7 * py) * DOT_SCALE, width, height, c, c, c);
							dest_line_x += width;
							line_printed = true;
						}
					}
					next_lf_pitch = (double_y_printed ? 16 : 8) * py * DOT_SCALE;
					finish_line();
					scroll(next_lf_pitch);
					fifo->clear();
				}
				break;
			case 0x57:
				// ESC W p d1 d2
				if(fifo->count() == 8) {
					p = (fifo->read_not_remove(2) - '0') * 1000 + (fifo->read_not_remove(3) - '0') * 100 + (fifo->read_not_remove(4) - '0') * 10 + (fifo->read_not_remove(5) - '0');
					d1 = fifo->read_not_remove(6);
					d2 = fifo->read_not_remove(7);
					
					if(pitch_mode == ELITE) {
						width = (int)(1.5 * DOT_SCALE);
					} else {
						width = 2 * DOT_SCALE;
					}
					height = 2 * DOT_SCALE;
					py = 1;
					
					if(ank_double_x) {
						width *= 2;
					}
					if(ank_double_y) {
						height *= 2;
						top = 0;
						py *= 2;
						double_y_printed = true;
					}
					for(int i = 0; i < p * 2; i += 2) {
						if(dest_line_x < 1440 * DOT_SCALE) {
							if(reverse) {
								emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, width, 48 * DOT_SCALE, 255, 255, 255);
								c = 0;
							}
							if(d1 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  0 * py) * DOT_SCALE, width, height, c, c, c);
							if(d1 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  1 * py) * DOT_SCALE, width, height, c, c, c);
							if(d1 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  3 * py) * DOT_SCALE, width, height, c, c, c);
							if(d1 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  4 * py) * DOT_SCALE, width, height, c, c, c);
							if(d1 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  6 * py) * DOT_SCALE, width, height, c, c, c);
							if(d1 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  7 * py) * DOT_SCALE, width, height, c, c, c);
							if(d1 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top +  9 * py) * DOT_SCALE, width, height, c, c, c);
							if(d1 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 10 * py) * DOT_SCALE, width, height, c, c, c);
							if(d2 & 0x80) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 12 * py) * DOT_SCALE, width, height, c, c, c);
							if(d2 & 0x40) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 13 * py) * DOT_SCALE, width, height, c, c, c);
							if(d2 & 0x20) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 15 * py) * DOT_SCALE, width, height, c, c, c);
							if(d2 & 0x10) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 16 * py) * DOT_SCALE, width, height, c, c, c);
							if(d2 & 0x08) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 18 * py) * DOT_SCALE, width, height, c, c, c);
							if(d2 & 0x04) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 19 * py) * DOT_SCALE, width, height, c, c, c);
							if(d2 & 0x02) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 21 * py) * DOT_SCALE, width, height, c, c, c);
							if(d2 & 0x01) emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, (top + 22 * py) * DOT_SCALE, width, height, c, c, c);
							dest_line_x += width;
							line_printed = true;
						}
					}
					next_lf_pitch = (double_y_printed ? 48 : 24) * DOT_SCALE;
					finish_line();
					scroll(next_lf_pitch);
					fifo->clear();
				}
				break;
			case 0x58:
				// ESC X
				underline = true;
				fifo->clear();
				break;
			case 0x59:
				// ESC Y
				underline = false;
				fifo->clear();
				break;
			case 0x5c:
				// ESC \ n1 n2
				if(fifo->count() == 4) {
					n = (int16_t)((uint16_t)(fifo->read_not_remove(2) + fifo->read_not_remove(2) * 256));
					if(n >= -1440 && n <= 1440) {
						dest_line_x += n * DOT_SCALE;
					}
					fifo->clear();
				}
				break;
			case 0x5d:
				// ESC ] (not supported)
				fifo->clear();
				break;
			case 0x63:
				// ESC 63
				if(fifo->count() == 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x31:
						// ESC c1
						reset();
						fifo->clear();
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			case 0x67:
				// ESC g
				bold = true;
				fifo->clear();
				break;
			case 0x68:
				// ESC h
				bold = false;
				fifo->clear();
				break;
			case 0x70:
				// ESC 70
				if(fifo->count() == 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x30:
						// ESC p0 (not supported: disable pe)
						fifo->clear();
						break;
					case 0x31:
						// ESC p1 (not supported: enable pe)
						fifo->clear();
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			case 0x73:
				// ESC 73
				if(fifo->count() == 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x30:
						// ESC s0
						script_mode = 0;
						fifo->clear();
						break;
					case 0x31:
						// ESC s1
						script_mode = SUPER_SCRIPT;
						fifo->clear();
						break;
					case 0x32:
						// ESC s2
						script_mode = SUB_SCRIPT;
						fifo->clear();
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			default:
				// unknown
				fifo->clear();
				break;
			}
		}
		break;
	case 0x1c:
		// FS
		if(fifo->count() >= 2) {
			switch(fifo->read_not_remove(1)) {
			case 0x4a:
				// FS J
				vertical = true;
				fifo->clear();
				break;
			case 0x4b:
				// FS K
				vertical = false;
				fifo->clear();
				break;
			case 0x53:
				// FS S n1 n2
				if(fifo->count() == 4) {
					// FIXME
					n1 = fifo->read_not_remove(2);
					n2 = fifo->read_not_remove(3);
					kanji_pitch = n1 + 24 + n2;
					fifo->clear();
				}
				break;
			case 0x54:
				// FS T n1 n2
				if(fifo->count() == 4) {
					// FIXME
					n1 = fifo->read_not_remove(2);
					n2 = fifo->read_not_remove(3);
					kanji_half_pitch = n1 + 12 + n2;
					fifo->clear();
				}
				break;
			case 0x70:
				// FS p
				kanji_double_x = true;
				fifo->clear();
				break;
			case 0x71:
				// FS q
				kanji_double_x = false;
				fifo->clear();
				break;
			default:
				// unknown
				fifo->clear();
				break;
			}
		}
		break;
	default:
		if(kanji_mode) {
			if(fifo->count() == 2) {
				int code = fifo->read() << 8;
				code += fifo->read();
				draw_char(code);
			}
		} else {
			draw_char(fifo->read());
		}
		break;
	}
}

void MZ1P17::process_mz80p4()
{
#if !defined(_MZ80K) && !defined(_MZ1200)	// MZ-80P4A
	int p, d;
	int width, height;
#endif
	int next_lf_pitch;
	uint8_t c = 255;
	
	switch(fifo->read_not_remove(0)) {
	case 0x05:
		// 05 (not supported: status check?)
	case 0x06:
		// 06 (not supported: unknown)
	case 0x07:
		// 07 (not supported: out of paper check)
	case 0x08:
		// 08 (not supported: mechanical trouble check)
		fifo->clear();
		break;
#if defined(_MZ80K) || defined(_MZ1200)		// MZ-80P3
	case 0x09:
		// 09 (CPR)
		lf_pitch = PIXEL_PER_INCH * 24 / 180;
		fifo->clear();
		break;
#else						// MZ-80P4A
	case 0x09:
		// 09
		if(fifo->count() >= 2) {
			switch(fifo->read_not_remove(1)) {
			case 0x09:
				// 09 09
				if(fifo->count() >= 3) {
					switch(fifo->read_not_remove(2)) {
					case 0x09:
						// 09 09 09
						pitch_mode = CONDENSE;
						fifo->clear();
						break;
					case 0x0b:
						// 09 09 0B
						pitch_mode = PICA;
						fifo->clear();
						break;
					case 0x30:
					case 0x31:
					case 0x32:
					case 0x33:
					case 0x34:
					case 0x35:
					case 0x36:
					case 0x37:
					case 0x38:
					case 0x39:
						// 09 09 p (not supported: set page length)
						if(fifo->count() == 4) {
							p = (fifo->read_not_remove(2) - '0') * 10 + (fifo->read_not_remove(3) - '0');
							fifo->clear();
						}
						break;
					default:
						// unknown
						fifo->clear();
						break;
					}
				}
				break;
			default:
				// 09
				lf_pitch = PIXEL_PER_INCH * 24 / 180;
				fifo->read();
				process_mz80p4();
				break;
			}
		}
		break;
#endif
	case 0x0a:
		// 0A (NMR)
		lf_pitch = PIXEL_PER_INCH / 6;
		fifo->clear();
		break;
#if defined(_MZ80K) || defined(_MZ1200)		// MZ-80P3
	// MZ-80P3
	case 0x0b:
		// 0B (40C)
		ank_double_x = true;
		fifo->clear();
		break;
#else						// MZ-80P4A
	case 0x0b:
		// 0B
		if(fifo->count() >= 2) {
			switch(fifo->read_not_remove(1)) {
			case 0x0b:
				// 0B 0B p1 p2 d1 d2 ... dk
				if(fifo->count() >= 6) {
					int tmp[4];
					for(int i = 0; i < 4; i++) {
						p = fifo->read_not_remove(2 + i);
						if(p >= '0' && p <= '9') {
							tmp[i] = p - '0';
						} else if(p >= 'a' && p <= 'f') {
							tmp[i] = p - 'a' + 10;
						} else if(p >= 'A' && p <= 'F') {
							tmp[i] = p - 'A' + 10;
						} else {
							tmp[i] = 0;
						}
					}
					p = (tmp[0] * 16 + tmp[1]) + (tmp[2] * 16 + tmp[3]) * 256;
					if(fifo->count() == 6 + p) {
						if(line_printed) {
							next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
							finish_line();
							scroll(next_lf_pitch);
						}
						if(pitch_mode == ELITE) {
							width = 2 * DOT_SCALE;
						} else if(pitch_mode == CONDENSE) {
							width = (int)(1.5 * DOT_SCALE);
						} else {
							width = 3 * DOT_SCALE;
						}
						if(lf_pitch == PIXEL_PER_INCH / 120) {
							height = 2 * DOT_SCALE;
						} else {
							height = 3 * DOT_SCALE;
						}
						for(int i = 0; i < p; i++) {
							if(dest_line_x < 1440 * DOT_SCALE) {
								if(reverse) {
									emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, width, 48 * DOT_SCALE, 255, 255, 255);
									c = 0;
								}
								d = fifo->read_not_remove(6 + i);
								if(d & 0x01) draw_dot(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 0) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x02) draw_dot(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 1) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x04) draw_dot(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 2) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x08) draw_dot(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 3) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x10) draw_dot(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 4) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x20) draw_dot(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 5) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x40) draw_dot(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 6) * DOT_SCALE, width, height, c, c, c);
								if(d & 0x80) draw_dot(&bitmap_line[color_mode], dest_line_x, (24 + 3 * 7) * DOT_SCALE, width, height, c, c, c);
								dest_line_x += width;
								line_printed = true;
							}
						}
						fifo->clear();
					}
				}
				break;
			default:
				// 0B
				ank_double_x = true;
				fifo->read();
				process_mz80p4();
				break;
			}
		}
		break;
#endif
	case 0x0c:
		// 0C (NMC)
		ank_double_x = false;
		fifo->clear();
		break;
	case 0x0d:
		// CR
		next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
		finish_line();
		scroll(next_lf_pitch);
		fifo->clear();
		break;
	case 0x0f:
		// FF (HOM)
#if defined(_MZ80K) || defined(_MZ1200)		// MZ-80P3
		lf_pitch = PIXEL_PER_INCH / 6;	// CPR cancel
#endif
		finish_line();
		finish_paper();
		fifo->clear();
		break;
	default:
		draw_char(fifo->read());
		break;
	}
}

//#define IS_HANKAKU(c) \
//	((c >= 0x20 && c <= 0xff) || (c >= 0x2820 && c <= 0x28ff))
#define IS_HANKAKU(c) \
	((c >= 0x00 && c <= 0xff) || (c >= 0x2800 && c <= 0x28ff))
#if defined(_MZ80K) || defined(_MZ1200) || defined(_MZ80A)
#define IS_NOT_ANK(c) \
	((c >= 0x00 && c <= 0x1f) || (c == 0x5c) || (c >= 0x5e && c <= 0xff))
#elif defined(_MZ700) || defined(_MZ800) || defined(_MZ1500)
#define IS_NOT_ANK(c) \
	((c >= 0x00 && c <= 0x1f) || (c == 0x5c) || (c >= 0x5e && c <= 0xff) || (c >= 0x2800 && c <= 0x281f) || (c == 0x285c) || (c >= 0x285e && c <= 0x28ff))
#else
#define IS_NOT_ANK(c) \
	((c >= 0x00 && c <= 0x1f) || (c >= 0x7f && c <= 0xa0) || (c >= 0xe0 && c <= 0xff) || (c >= 0x2800 && c <= 0x281f) || (c >= 0x287f && c <= 0x28a0) || (c >= 0x28e0 && c <= 0x28ff))
#endif
#define IS_KANA(c) \
	((c >= 0xa6 && c <= 0xdd && c != 0xb0) || (c >= 0x28a6 && c <= 0x28dd && c != 0x28b0))
#define IS_GAIJI(c) \
	((c >= 0x7821 && c <= 0x787e) || (c >= 0x7921 && c <= 0x797e) || (c >= 0x7a21 && c <= 0x7a2b))
#define IS_NOT_VERTICAL(c) \
	(c == 0x2121 || (c >= 0x2126 && c <= 0x2128) || (c >= 0x213c && c <= 0x213e) || (c >= 0x2142 && c <= 0x2145) || (c >= 0x214a && c <= 0x215b) || (c >= 0x2161 && c <= 0x2166) || (c >= 0x217b && c <= 0x217d) || \
	 c == 0x2228 || (c >= 0x222a && c <= 0x222d))

void MZ1P17::draw_char(uint16_t code)
{
	bool is_kanji = kanji_mode;
	int font_width = 12 * DOT_SCALE;
	int font_height = 24 * DOT_SCALE;
	int font_rotate = 0;
	int gap_p1, gap_p2;
	int dest_line_y = 24 * DOT_SCALE;
	bool double_x, double_y;
	uint8_t c = 255;
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
		uint16_t sjis = jis_to_sjis(code);
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
	if(IS_KANA(code) && hiragana_mode) {
		static const uint16_t hiragana_sjis[] = {
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
		uint16_t sjis = hiragana_sjis[(code & 0xff) - 0xa6];
		tmp[0] = sjis >> 8;
		tmp[1] = sjis & 0xff;
		tmp[2] = '\0';
		font_width /= 2;
	}
	bool proportional = (!(code & 0xff00) && pitch_mode == PROPORTIONAL);
	
	if(!font.initialized() || font.width != font_width || font.height != font_height || font.rotate != font_rotate || font.bold != bold || 
	   (proportional && font.family[0] != _T('P')) || (!proportional && font.family[0] == _T('P'))) {
		emu->release_font(&font);
		emu->create_font(&font, proportional ? _T("PMincho") : _T("Mincho"), font_width, font_height, font_rotate, bold, false);
	}
	if((code & 0xff00) || (IS_KANA(code) && hiragana_mode)) {
		font_width *= 2;
	} else if(proportional) {
		if(IS_NOT_ANK(code) || DOT_PRINT) {
			// use internal font
		} else {
			font_width = emu->get_text_width(&bitmap_line[color_mode], &font, tmp);
		}
	}
	if(dest_line_x + gap_p1 + font_width + gap_p2 > margin_right) {
		int next_lf_pitch = lf_pitch + (double_y_printed ? 24 * DOT_SCALE : 0);
		finish_line();
		scroll(next_lf_pitch);
	}
	if(dest_line_y == 0) {
		double_y_printed = true;
	}
	if(reverse) {
		emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x, 0, gap_p1 + font_width + gap_p2, 48 * DOT_SCALE, 255, 255, 255);
		c = 0;
	}
	if(IS_NOT_ANK(code) || DOT_PRINT) {
		// use internal font
#if DOT_PRINT
		int dot_y_split = 16;
		for(int y = 0; y < dot_y_split; y++) {
			int ys = font_height * y / dot_y_split + dest_line_y;
			int ye = font_height * (y + 1) / dot_y_split + dest_line_y;
			int yw = ye - ys;
			int yc = y * (16 / dot_y_split);
			for(int x = 0; x < 8; x++) {
				if(ank[code & 0xff][yc][x]) {
					int xs = font_width * x / 8 + dest_line_x + gap_p1;
					int xe = font_width * (x + 1) / 8 + dest_line_x + gap_p1;
					int xw = xe - xs;
					draw_dot(&bitmap_line[color_mode], xs, ys, xw, yw, c, c, c);
				}
			}
		}
#else
		for(int y = 0; y < font_height; y++) {
			for(int x = 0; x < font_width; x++) {
				if(ank[code & 0xff][16 * y / font_height][8 * x / font_width]) {
					emu->draw_point_to_bitmap(&bitmap_line[color_mode], dest_line_x + gap_p1 + x, dest_line_y +  y, c, c, c);
				}
			}
		}
#endif
	} else if(IS_GAIJI(code)) {
		// use gaiji
		int n1 = (code >> 8) - 0x78;
		int n2 = (code & 0xff) - 0x21;
		for(int y = 0; y < font_height; y++) {
			for(int x = 0; x < font_width; x++) {
				if(gaiji[n1][n2][48 * y / font_height][48 * x / font_width]) {
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
			if(dest_line_x + x < margin_right) {
				emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], dest_line_x + x, (48 + 1) * DOT_SCALE, 1, DOT_SCALE, 255, 255, 255);
			}
		}
	}
	dest_line_x += gap_p1 + font_width + gap_p2;
	line_printed = true;
}

void MZ1P17::draw_dot(bitmap_t *bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b)
{
	if(!DOT_PRINT || width < 3 || height < 3) {
		// dot pattern : square (¡Œ^)
		emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], x, y, width, height, r, g, b);
	} else {
		// dot pattern : convex (“ÊŒ^)
		int loop = (int)(width / 3) + (width % 3 > 0 ? 1 : 0);
		for(int i = 0; i < loop; i++) {
			int sx = x + 3 * i;
			int sw = 3;
			if(3 * i > width) {
				sw = 3 * i - width;
				emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], sx, y, sw, height, r, g, b);
			} else {
				emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], sx + 1 , y,     sw - 2, height    , r, g, b);
				emu->draw_rectangle_to_bitmap(&bitmap_line[color_mode], sx     , y + 1, sw    , height - 1, r, g, b);
			}
		}
	}
}

void MZ1P17::scroll(int value)
{
	dest_paper_y += value;
//	emu->clear_bitmap(&bitmap_line[0], 0, 0, 0);
}

void MZ1P17::finish()
{
	finish_line();
	finish_paper();
	paper_index = written_length = 0;
}

void MZ1P17::finish_line()
{
	if(line_printed) {
		int source_y = (double_y_printed ? 0 : 24) * DOT_SCALE;
		int height = bitmap_line[0].height - source_y;
		
		if(dest_paper_y + height > PIXEL_PER_INCH * 11) {
			finish_paper();
		}
		for(int y = 0; y < bitmap_line[0].height; y++) {
			scrntype_t *d  = bitmap_paper.get_buffer(space_top + dest_paper_y + y) + space_left;
			scrntype_t *p1 = bitmap_line[color_mode ? 1 : 0].get_buffer(y);	// cyan
			scrntype_t *p2 = bitmap_line[color_mode ? 2 : 0].get_buffer(y);	// magenta
			scrntype_t *p3 = bitmap_line[color_mode ? 3 : 0].get_buffer(y);	// yellow
			
			for(int x = 0; x < bitmap_line[0].width; x++) {
				uint8_t r = ~R_OF_COLOR(p1[x]);
				uint8_t g = ~R_OF_COLOR(p2[x]);
				uint8_t b = ~R_OF_COLOR(p3[x]);
				if(!(r == 255 && g == 255 && b == 255)) {
					d[x] = RGB_COLOR(r, g, b);
				}
			}
		}
		if(color_mode) {
			color_mode--;
		}
		emu->clear_bitmap(&bitmap_line[0], 0, 0, 0);
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
		if(written_length > 1) {
			emu->write_bitmap_to_file(&bitmap_paper, create_string(_T("%s_#%02d.png"), get_file_path_without_extensiton(base_path), paper_index++));
		}
		emu->clear_bitmap(&bitmap_paper, 255, 255, 255);
	}
	paper_printed = false;
	dest_paper_y = 0;
}

#define STATE_VERSION	4

bool MZ1P17::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(value);
	state_fio->StateValue(busy_id);
	state_fio->StateValue(ack_id);
	state_fio->StateValue(strobe);
	state_fio->StateValue(res);
	state_fio->StateValue(busy);
	state_fio->StateValue(ack);
	state_fio->StateArray(&gaiji[0][0][0][0], sizeof(gaiji), 1);
	state_fio->StateArray(htab, sizeof(htab), 1);
	for(int i = 0; i < array_length(vtab); i++) {
		state_fio->StateValue(vtab[i].y);
		state_fio->StateValue(vtab[i].active);
	}
	if(!fifo->process_state((void *)state_fio, loading)) {
		return false;
	}
	state_fio->StateValue(lf_pitch);
	state_fio->StateValue(prev_esc_6);
	state_fio->StateValue(margin_left);
	state_fio->StateValue(margin_right);
	state_fio->StateValue(pitch_mode);
	state_fio->StateValue(script_mode);
	state_fio->StateValue(kanji_mode);
	state_fio->StateValue(kanji_half);
	state_fio->StateValue(bold);
	state_fio->StateValue(underline);
	state_fio->StateValue(hiragana_mode);
	state_fio->StateValue(reverse);
	state_fio->StateValue(vertical);
	state_fio->StateValue(ank_double_x);
	state_fio->StateValue(ank_double_y);
	state_fio->StateValue(kanji_double_x);
	state_fio->StateValue(kanji_double_y);
	state_fio->StateValue(kanji_pitch);
	state_fio->StateValue(kanji_half_pitch);
	state_fio->StateValue(dest_line_x);
	state_fio->StateValue(dest_paper_y);
	state_fio->StateValue(color_mode);
	state_fio->StateValue(double_y_printed);
	
	// post process
	if(loading) {
		emu->clear_bitmap(&bitmap_paper, 255, 255, 255);
		emu->clear_bitmap(&bitmap_line[0], 0, 0, 0);
		emu->clear_bitmap(&bitmap_line[1], 0, 0, 0);
		emu->clear_bitmap(&bitmap_line[2], 0, 0, 0);
		emu->clear_bitmap(&bitmap_line[3], 0, 0, 0);
		wait_frames = -1;
		line_printed = paper_printed = false;
		paper_index = written_length = 0;
	}
	return true;
}

