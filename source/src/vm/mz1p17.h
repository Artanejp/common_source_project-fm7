/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.12.24-

	[ MZ-1P17 ]
*/

#ifndef _MZ1P17_H_
#define _MZ1P17_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

// for correct super/sub script mode
//#define PIXEL_PER_INCH	720
// for correct 1/120 inch scroll
#define PIXEL_PER_INCH	360
// for correct 1/180 inch dots
//#define PIXEL_PER_INCH	180
#define DOT_PER_INCH	180
#define DOT_SCALE	(PIXEL_PER_INCH / DOT_PER_INCH)

class FIFO;

class MZ1P17 : public DEVICE
{
private:
	int value, wait_frames;
	bool strobe;
	
	bitmap_t bitmap_paper;
	bitmap_t bitmap_line[4];
	font_t font;
	int space_left, space_top;
	
	bool ank[256][16][8];
	bool hiragana[0x38][48][24];		// 0xa6-0xdd
	bool hiragana_bold[0x38][48][24];	// 0xa6-0xdd
	bool gaiji[3][94][24][24];		// 0x78-0x7a,0x21-0x7e
	bool htab[1440 * DOT_SCALE];
	
	FIFO *fifo;
	
	int lf_pitch;
	int margin_left, margin_right;
	int pitch_mode;
	int script_mode;
	bool kanji_mode, kanji_half, hiragana_mode;
	bool bold, underline, reverse, vertical;
	
	bool ank_double_x, ank_double_y;
	bool kanji_double_x, kanji_double_y;
	int kanji_pitch, kanji_half_pitch;
	
	int dest_line_x, dest_paper_y;
	int color_mode;
	bool double_y_printed;
	bool line_printed, paper_printed;
	int paper_index;
	_TCHAR base_path[_MAX_PATH];
	
	void process();
	void draw_char(uint16 code);
	void scroll(int value);
	void finish();
	void finish_line();
	void finish_paper();
	
public:
	MZ1P17(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MZ1P17() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void event_frame();
	void write_signal(int id, uint32 data, uint32 mask);
	uint32 read_signal(int ch);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
};

#endif

