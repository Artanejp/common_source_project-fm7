/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
    Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.10
	History: 2015.11.10 Split from qt_main.cpp
	Note: This class must be compiled per VM, must not integrate shared units.
	[ win32 main ] -> [ Qt main ] -> [Emu Thread]
*/

#include <QString>
#include <QTextCodec>
#include <QWaitCondition>

#include <SDL.h>

#include "emu_thread.h"

#include "qt_gldraw.h"
#include "agar_logger.h"
#include "menu_flags.h"

// buttons
#ifdef MAX_BUTTONS
#define MAX_FONT_SIZE 32
#endif
#define MAX_SKIP_FRAMES 10

EmuThreadClass::EmuThreadClass(META_MainWindow *rootWindow, EMU *pp_emu, USING_FLAGS *p, QObject *parent) : QThread(parent) {
	MainWindow = rootWindow;
	p_emu = pp_emu;
	using_flags = p;
	p_config = p->get_config_ptr();
	
	bRunThread = true;
	prev_skip = false;
	tick_timer.start();
	update_fps_time = tick_timer.elapsed();
	next_time = update_fps_time;
	total_frames = 0;
	draw_frames = 0;
	skip_frames = 0;
	calc_message = true;
	mouse_flag = false;
	//p_emu->set_parent_handler(this);
	drawCond = new QWaitCondition();
	mouse_x = 0;
	mouse_y = 0;
	if(using_flags->is_use_tape() && !using_flags->is_tape_binary_only()) {
		tape_play_flag = false;
		tape_rec_flag = false;
		tape_pos = 0;
	}

	if(using_flags->get_use_sound_volume() > 0) {
		for(int i = 0; i < using_flags->get_use_sound_volume(); i++) {
			bUpdateVolumeReq[i] = true;
			volume_avg[i] = (using_flags->get_config_ptr()->sound_volume_l[i] +
							 using_flags->get_config_ptr()->sound_volume_r[i]) / 2;
			volume_balance[i] = (using_flags->get_config_ptr()->sound_volume_r[i] -
								 using_flags->get_config_ptr()->sound_volume_l[i]) / 2;
		}
	}
};

EmuThreadClass::~EmuThreadClass() {
	delete drawCond;
};

void EmuThreadClass::calc_volume_from_balance(int num, int balance)
{
	int level = volume_avg[num];
	int right;
	int left;
	volume_balance[num] = balance;
	right = level + balance;
	left  = level - balance;
	using_flags->get_config_ptr()->sound_volume_l[num] = left;	
	using_flags->get_config_ptr()->sound_volume_r[num] = right;
}

void EmuThreadClass::calc_volume_from_level(int num, int level)
{
	int balance = volume_balance[num];
	int right,left;
	volume_avg[num] = level;
	right = level + balance;
	left  = level - balance;
	using_flags->get_config_ptr()->sound_volume_l[num] = left;	
	using_flags->get_config_ptr()->sound_volume_r[num] = right;
}

int EmuThreadClass::get_interval(void)
{
	static int accum = 0;
	accum += p_emu->get_frame_interval();
	int interval = accum >> 10;
	accum -= interval << 10;
	return interval;
}

void EmuThreadClass::doExit(void)
{
	int status;
	bRunThread = false;
}

void EmuThreadClass::moved_mouse(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
#if defined(USE_MOUSE)   
	p_emu->set_mouse_pointer(x, y);
#endif   
}

void EmuThreadClass::button_pressed_mouse(Qt::MouseButton button)
{
	if(using_flags->is_use_mouse()) {
#if defined(USE_MOUSE)   
		int stat = p_emu->get_mouse_button();
		bool flag = p_emu->is_mouse_enabled();
		switch(button) {
		case Qt::LeftButton:
			stat |= 0x01;
			break;
		case Qt::RightButton:
			stat |= 0x02;
			break;
		case Qt::MiddleButton:
			flag = !flag;
			emit sig_mouse_enable(flag);
			break;
		}
		p_emu->set_mouse_button(stat);
#endif	   
	} else {		
		if(using_flags->get_max_button() > 0) {
			button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
			if(vm_buttons_d == NULL) return;
			switch(button) {
			case Qt::LeftButton:
			case Qt::RightButton:
				for(int i = 0; i < using_flags->get_max_button(); i++) {
					if((mouse_x >= vm_buttons_d[i].x) &&
					   (mouse_x < (vm_buttons_d[i].x + vm_buttons_d[i].width))) {
						if((mouse_y >= vm_buttons_d[i].y) &&
						   (mouse_y < (vm_buttons_d[i].y + vm_buttons_d[i].height))) {
							if(vm_buttons_d[i].code != 0x00) {
								//p_emu->key_down(vm_buttons_d[i].code, false);
								key_down_code = vm_buttons_d[i].code;
								key_repeat = false;
							} else {
								bResetReq = true;
							}
						}
					}
				}
				break;
			}
		}
	}
}

void EmuThreadClass::button_released_mouse(Qt::MouseButton button)
{
	if(using_flags->is_use_mouse()) {
#if defined(USE_MOUSE)   
		int stat = p_emu->get_mouse_button();
		switch(button) {
		case Qt::LeftButton:
			stat &= 0x7ffffffe;
			break;
		case Qt::RightButton:
			stat &= 0x7ffffffd;
			break;
		case Qt::MiddleButton:
			//emit sig_mouse_enable(false);
			break;
		}
		p_emu->set_mouse_button(stat);
#endif
	} else {
		if(using_flags->get_max_button() > 0) {
			button_desc_t *vm_buttons_d = using_flags->get_vm_buttons();
			if(vm_buttons_d == NULL) return;
			
			switch(button) {
			case Qt::LeftButton:
			case Qt::RightButton:
				for(int i = 0; i < using_flags->get_max_button(); i++) {
					if((mouse_x >= vm_buttons_d[i].x) &&
					   (mouse_x < (vm_buttons_d[i].x + vm_buttons_d[i].width))) {
						if((mouse_y >= vm_buttons_d[i].y) &&
						   (mouse_y < (vm_buttons_d[i].y + vm_buttons_d[i].height))) {
							if(vm_buttons_d[i].code != 0x00) {
								//p_emu->key_up(vm_buttons_d[i].code);
								key_up_code = vm_buttons_d[i].code;
							}
						}
					}
				}
				break;
			}
		}
	}
}

void EmuThreadClass::do_key_down(uint32_t vk, uint32_t mod, bool repeat)
{
	key_down_code = vk;
	key_mod_code = mod;
	key_repeat = repeat;
	key_changed = true;
}

void EmuThreadClass::do_key_up(uint32_t vk, uint32_t mod)
{
	key_up_code = vk;
	key_mod_code = mod;
	key_changed = true;

}


void EmuThreadClass::set_tape_play(bool flag)
{
	tape_play_flag = flag;
}

EmuThreadClass *EmuThreadClass::currentHandler()
{
	return this;
}

void EmuThreadClass::resize_screen(int screen_width, int screen_height, int stretched_width, int stretched_height)
{
	//emit sig_resize_uibar(stretched_width, stretched_height);
	emit sig_resize_screen(screen_width, screen_height);
}

const int auto_key_table_base[][2] = {
	// 0x100: shift
	// 0x200: kana
	// 0x400: alphabet
	// 0x800: ALPHABET
	{0x0a,	0x000 | 0x0d},	// Enter(Unix)
	{0x0d,	0x000 | 0x0d},	// Enter
	{0x20,	0x000 | 0x20},	// ' '

	{0x21,	0x100 | 0x31},	// '!'
	{0x22,	0x100 | 0x32},	// '"'
	{0x23,	0x100 | 0x33},	// '#'
	{0x24,	0x100 | 0x34},	// '$'
	{0x25,	0x100 | 0x35},	// '%'
	{0x26,	0x100 | 0x36},	// '&'
	{0x27,	0x100 | 0x37},	// '''
	{0x28,	0x100 | 0x38},	// '('
	{0x29,	0x100 | 0x39},	// ')'
	{0x2a,	0x100 | 0xba},	// '*'
	{0x2b,	0x100 | 0xbb},	// '+'
	{0x2c,	0x000 | 0xbc},	// ','
	{0x2d,	0x000 | 0xbd},	// '-'
	{0x2e,	0x000 | 0xbe},	// '.'
	{0x2f,	0x000 | 0xbf},	// '/'

	{0x30,	0x000 | 0x30},	// '0'
	{0x31,	0x000 | 0x31},	// '1'
	{0x32,	0x000 | 0x32},	// '2'
	{0x33,	0x000 | 0x33},	// '3'
	{0x34,	0x000 | 0x34},	// '4'
	{0x35,	0x000 | 0x35},	// '5'
	{0x36,	0x000 | 0x36},	// '6'
	{0x37,	0x000 | 0x37},	// '7'
	{0x38,	0x000 | 0x38},	// '8'
	{0x39,	0x000 | 0x39},	// '9'

	{0x3a,	0x000 | 0xba},	// ':'
	{0x3b,	0x000 | 0xbb},	// ';'
	{0x3c,	0x100 | 0xbc},	// '<'
	{0x3d,	0x100 | 0xbd},	// '='
	{0x3e,	0x100 | 0xbe},	// '>'
	{0x3f,	0x100 | 0xbf},	// '?'
	{0x40,	0x000 | 0xc0},	// '@'

	{0x41,	0x400 | 0x41},	// 'A'
	{0x42,	0x400 | 0x42},	// 'B'
	{0x43,	0x400 | 0x43},	// 'C'
	{0x44,	0x400 | 0x44},	// 'D'
	{0x45,	0x400 | 0x45},	// 'E'
	{0x46,	0x400 | 0x46},	// 'F'
	{0x47,	0x400 | 0x47},	// 'G'
	{0x48,	0x400 | 0x48},	// 'H'
	{0x49,	0x400 | 0x49},	// 'I'
	{0x4a,	0x400 | 0x4a},	// 'J'
	{0x4b,	0x400 | 0x4b},	// 'K'
	{0x4c,	0x400 | 0x4c},	// 'L'
	{0x4d,	0x400 | 0x4d},	// 'M'
	{0x4e,	0x400 | 0x4e},	// 'N'
	{0x4f,	0x400 | 0x4f},	// 'O'
	{0x50,	0x400 | 0x50},	// 'P'
	{0x51,	0x400 | 0x51},	// 'Q'
	{0x52,	0x400 | 0x52},	// 'R'
	{0x53,	0x400 | 0x53},	// 'S'
	{0x54,	0x400 | 0x54},	// 'T'
	{0x55,	0x400 | 0x55},	// 'U'
	{0x56,	0x400 | 0x56},	// 'V'
	{0x57,	0x400 | 0x57},	// 'W'
	{0x58,	0x400 | 0x58},	// 'X'
	{0x59,	0x400 | 0x59},	// 'Y'
	{0x5a,	0x400 | 0x5a},	// 'Z'

	{0x5b,	0x000 | 0xdb},	// '['
	{0x5c,	0x000 | 0xdc},	// '\'
	{0x5d,	0x000 | 0xdd},	// ']'
	{0x5e,	0x000 | 0xde},	// '^'
	{0x5f,	0x100 | 0xe2},	// '_'
	{0x60,	0x100 | 0xc0},	// '`'

	{0x61,	0x800 | 0x41},	// 'a'
	{0x62,	0x800 | 0x42},	// 'b'
	{0x63,	0x800 | 0x43},	// 'c'
	{0x64,	0x800 | 0x44},	// 'd'
	{0x65,	0x800 | 0x45},	// 'e'
	{0x66,	0x800 | 0x46},	// 'f'
	{0x67,	0x800 | 0x47},	// 'g'
	{0x68,	0x800 | 0x48},	// 'h'
	{0x69,	0x800 | 0x49},	// 'i'
	{0x6a,	0x800 | 0x4a},	// 'j'
	{0x6b,	0x800 | 0x4b},	// 'k'
	{0x6c,	0x800 | 0x4c},	// 'l'
	{0x6d,	0x800 | 0x4d},	// 'm'
	{0x6e,	0x800 | 0x4e},	// 'n'
	{0x6f,	0x800 | 0x4f},	// 'o'
	{0x70,	0x800 | 0x50},	// 'p'
	{0x71,	0x800 | 0x51},	// 'q'
	{0x72,	0x800 | 0x52},	// 'r'
	{0x73,	0x800 | 0x53},	// 's'
	{0x74,	0x800 | 0x54},	// 't'
	{0x75,	0x800 | 0x55},	// 'u'
	{0x76,	0x800 | 0x56},	// 'v'
	{0x77,	0x800 | 0x57},	// 'w'
	{0x78,	0x800 | 0x58},	// 'x'
	{0x79,	0x800 | 0x59},	// 'y'
	{0x7a,	0x800 | 0x5a},	// 'z'

	{0x7b,	0x100 | 0xdb},	// '{'
	{0x7c,	0x100 | 0xdc},	// '|'
	{0x7d,	0x100 | 0xdd},	// '}'
	{0x7e,	0x100 | 0xde},	// '~'

	{0xa1,	0x300 | 0xbe},	// '
	{0xa2,	0x300 | 0xdb},	// '
	{0xa3,	0x300 | 0xdd},	// '
	{0xa4,	0x300 | 0xbc},	// '
	{0xa5,	0x300 | 0xbf},	// '･'
	{0xa6,	0x300 | 0x30},	// 'ｦ'
	{0xa7,	0x300 | 0x33},	// 'ｧ'
	{0xa8,	0x300 | 0x45},	// 'ｨ'
	{0xa9,	0x300 | 0x34},	// 'ｩ'
	{0xaa,	0x300 | 0x35},	// 'ｪ'
	{0xab,	0x300 | 0x36},	// 'ｫ'
	{0xac,	0x300 | 0x37},	// 'ｬ'
	{0xad,	0x300 | 0x38},	// 'ｭ'
	{0xae,	0x300 | 0x39},	// 'ｮ'
	{0xaf,	0x300 | 0x5a},	// 'ｯ'
	{0xb0,	0x200 | 0xdc},	// 'ｰ'
	{0xb1,	0x200 | 0x33},	// 'ｱ'
	{0xb2,	0x200 | 0x45},	// 'ｲ'
	{0xb3,	0x200 | 0x34},	// 'ｳ'
	{0xb4,	0x200 | 0x35},	// 'ｴ'
	{0xb5,	0x200 | 0x36},	// 'ｵ'
	{0xb6,	0x200 | 0x54},	// 'ｶ'
	{0xb7,	0x200 | 0x47},	// 'ｷ'
	{0xb8,	0x200 | 0x48},	// 'ｸ'
	{0xb9,	0x200 | 0xba},	// 'ｹ'
	{0xba,	0x200 | 0x42},	// 'ｺ'
	{0xbb,	0x200 | 0x58},	// 'ｻ'
	{0xbc,	0x200 | 0x44},	// 'ｼ'
	{0xbd,	0x200 | 0x52},	// 'ｽ'
	{0xbe,	0x200 | 0x50},	// 'ｾ'
	{0xbf,	0x200 | 0x43},	// 'ｿ'
	{0xc0,	0x200 | 0x51},	// 'ﾀ'
	{0xc1,	0x200 | 0x41},	// 'ﾁ'
	{0xc2,	0x200 | 0x5a},	// 'ﾂ'
	{0xc3,	0x200 | 0x57},	// 'ﾃ'
	{0xc4,	0x200 | 0x53},	// 'ﾄ'
	{0xc5,	0x200 | 0x55},	// 'ﾅ'
	{0xc6,	0x200 | 0x49},	// 'ﾆ'
	{0xc7,	0x200 | 0x31},	// 'ﾇ'
	{0xc8,	0x200 | 0xbc},	// 'ﾈ'
	{0xc9,	0x200 | 0x4b},	// 'ﾉ'
	{0xca,	0x200 | 0x46},	// 'ﾊ'
	{0xcb,	0x200 | 0x56},	// 'ﾋ'
	{0xcc,	0x200 | 0x32},	// 'ﾌ'
	{0xcd,	0x200 | 0xde},	// 'ﾍ'
	{0xce,	0x200 | 0xbd},	// 'ﾎ'
	{0xcf,	0x200 | 0x4a},	// 'ﾏ'
	{0xd0,	0x200 | 0x4e},	// 'ﾐ'
	{0xd1,	0x200 | 0xdd},	// 'ﾑ'
	{0xd2,	0x200 | 0xbf},	// 'ﾒ'
	{0xd3,	0x200 | 0x4d},	// 'ﾓ'
	{0xd4,	0x200 | 0x37},	// 'ﾔ'
	{0xd5,	0x200 | 0x38},	// 'ﾕ'
	{0xd6,	0x200 | 0x39},	// 'ﾖ'
	{0xd7,	0x200 | 0x4f},	// 'ﾗ'
	{0xd8,	0x200 | 0x4c},	// 'ﾘ'
	{0xd9,	0x200 | 0xbe},	// 'ﾙ'
	{0xda,	0x200 | 0xbb},	// 'ﾚ'
	{0xdb,	0x200 | 0xe2},	// 'ﾛ'
	{0xdc,	0x200 | 0x30},	// 'ﾜ'
	{0xdd,	0x200 | 0x59},	// 'ﾝ'
	{0xde,	0x200 | 0xc0},	// 'ﾞ'
	{0xdf,	0x200 | 0xdb},	// 'ﾟ'
	{-1, -1},
};

const int auto_key_table_base_us[][2] = {
	// 0x100: shift
	// 0x200: kana
	// 0x400: alphabet
	// 0x800: ALPHABET
	{0x0a,	0x000 | 0x0d},	// Enter(Unix)
	{0x0d,	0x000 | 0x0d},	// Enter
	{0x20,	0x000 | 0x20},	// ' '
	{0x21,	0x100 | 0x31},	// '!'
	{0x22,	0x100 | 0xba},	// '"'
	{0x23,	0x100 | 0x33},	// '#'
	{0x24,	0x100 | 0x34},	// '$'
	{0x25,	0x100 | 0x35},	// '%'
	{0x26,	0x100 | 0x37},	// '&'
	{0x27,	0x000 | 0xba},	// '''
	{0x28,	0x100 | 0x39},	// '('
	{0x29,	0x100 | 0x30},	// ')'
	{0x2a,	0x100 | 0x38},	// '*'
	{0x2b,	0x100 | 0xde},	// '+'
	{0x2c,	0x000 | 0xbc},	// ','
	{0x2d,	0x000 | 0xbd},	// '-'
	{0x2e,	0x000 | 0xbe},	// '.'
	{0x2f,	0x000 | 0xbf},	// '/'
	
	{0x30,	0x000 | 0x30},	// '0'
	{0x31,	0x000 | 0x31},	// '1'
	{0x32,	0x000 | 0x32},	// '2'
	{0x33,	0x000 | 0x33},	// '3'
	{0x34,	0x000 | 0x34},	// '4'
	{0x35,	0x000 | 0x35},	// '5'
	{0x36,	0x000 | 0x36},	// '6'
	{0x37,	0x000 | 0x37},	// '7'
	{0x38,	0x000 | 0x38},	// '8'
	{0x39,	0x000 | 0x39},	// '9'
	
	{0x3a,	0x100 | 0xbb},	// ':'
	{0x3b,	0x000 | 0xbb},	// ';'
	{0x3c,	0x100 | 0xbc},	// '<'
	{0x3d,	0x000 | 0xde},	// '='
	{0x3e,	0x100 | 0xbe},	// '>'
	{0x3f,	0x100 | 0xbf},	// '?'
	{0x40,	0x100 | 0x32},	// '@'
	
	{0x41,	0x400 | 0x41},	// 'A'
	{0x42,	0x400 | 0x42},	// 'B'
	{0x43,	0x400 | 0x43},	// 'C'
	{0x44,	0x400 | 0x44},	// 'D'
	{0x45,	0x400 | 0x45},	// 'E'
	{0x46,	0x400 | 0x46},	// 'F'
	{0x47,	0x400 | 0x47},	// 'G'
	{0x48,	0x400 | 0x48},	// 'H'
	{0x49,	0x400 | 0x49},	// 'I'
	{0x4a,	0x400 | 0x4a},	// 'J'
	{0x4b,	0x400 | 0x4b},	// 'K'
	{0x4c,	0x400 | 0x4c},	// 'L'
	{0x4d,	0x400 | 0x4d},	// 'M'
	{0x4e,	0x400 | 0x4e},	// 'N'
	{0x4f,	0x400 | 0x4f},	// 'O'
	{0x50,	0x400 | 0x50},	// 'P'
	{0x51,	0x400 | 0x51},	// 'Q'
	{0x52,	0x400 | 0x52},	// 'R'
	{0x53,	0x400 | 0x53},	// 'S'
	{0x54,	0x400 | 0x54},	// 'T'
	{0x55,	0x400 | 0x55},	// 'U'
	{0x56,	0x400 | 0x56},	// 'V'
	{0x57,	0x400 | 0x57},	// 'W'
	{0x58,	0x400 | 0x58},	// 'X'
	{0x59,	0x400 | 0x59},	// 'Y'
	{0x5a,	0x400 | 0x5a},	// 'Z'

	{0x5b,	0x000 | 0xc0},	// '['
	{0x5c,	0x000 | 0xe2},	// '\'
	{0x5d,	0x000 | 0xdb},	// ']'
	{0x5e,	0x100 | 0x36},	// '^'
	{0x5f,	0x100 | 0xbd},	// '_'
	{0x60,	0x000 | 0xdd},	// '`'
	
	{0x61,	0x800 | 0x41},	// 'a'
	{0x62,	0x800 | 0x42},	// 'b'
	{0x63,	0x800 | 0x43},	// 'c'
	{0x64,	0x800 | 0x44},	// 'd'
	{0x65,	0x800 | 0x45},	// 'e'
	{0x66,	0x800 | 0x46},	// 'f'
	{0x67,	0x800 | 0x47},	// 'g'
	{0x68,	0x800 | 0x48},	// 'h'
	{0x69,	0x800 | 0x49},	// 'i'
	{0x6a,	0x800 | 0x4a},	// 'j'
	{0x6b,	0x800 | 0x4b},	// 'k'
	{0x6c,	0x800 | 0x4c},	// 'l'
	{0x6d,	0x800 | 0x4d},	// 'm'
	{0x6e,	0x800 | 0x4e},	// 'n'
	{0x6f,	0x800 | 0x4f},	// 'o'
	{0x70,	0x800 | 0x50},	// 'p'
	{0x71,	0x800 | 0x51},	// 'q'
	{0x72,	0x800 | 0x52},	// 'r'
	{0x73,	0x800 | 0x53},	// 's'
	{0x74,	0x800 | 0x54},	// 't'
	{0x75,	0x800 | 0x55},	// 'u'
	{0x76,	0x800 | 0x56},	// 'v'
	{0x77,	0x800 | 0x57},	// 'w'
	{0x78,	0x800 | 0x58},	// 'x'
	{0x79,	0x800 | 0x59},	// 'y'
	{0x7a,	0x800 | 0x5a},	// 'z'

	{0x7b,	0x100 | 0xc0},	// '{'
	{0x7c,	0x100 | 0xe2},	// '|'
	{0x7d,	0x100 | 0xdb},	// '}'
	{0x7e,	0x100 | 0xdd},	// '~'

	{0xa1,	0x300 | 0xbe},	// '
	{0xa2,	0x300 | 0xdb},	// '
	{0xa3,	0x300 | 0xdd},	// '
	{0xa4,	0x300 | 0xbc},	// '
	{0xa5,	0x300 | 0xbf},	// '･'
	{0xa6,	0x300 | 0x30},	// 'ｦ'
	{0xa7,	0x300 | 0x33},	// 'ｧ'
	{0xa8,	0x300 | 0x45},	// 'ｨ'
	{0xa9,	0x300 | 0x34},	// 'ｩ'
	{0xaa,	0x300 | 0x35},	// 'ｪ'
	{0xab,	0x300 | 0x36},	// 'ｫ'
	{0xac,	0x300 | 0x37},	// 'ｬ'
	{0xad,	0x300 | 0x38},	// 'ｭ'
	{0xae,	0x300 | 0x39},	// 'ｮ'
	{0xaf,	0x300 | 0x5a},	// 'ｯ'
	{0xb0,	0x200 | 0xdc},	// 'ｰ'
	{0xb1,	0x200 | 0x33},	// 'ｱ'
	{0xb2,	0x200 | 0x45},	// 'ｲ'
	{0xb3,	0x200 | 0x34},	// 'ｳ'
	{0xb4,	0x200 | 0x35},	// 'ｴ'
	{0xb5,	0x200 | 0x36},	// 'ｵ'
	{0xb6,	0x200 | 0x54},	// 'ｶ'
	{0xb7,	0x200 | 0x47},	// 'ｷ'
	{0xb8,	0x200 | 0x48},	// 'ｸ'
	{0xb9,	0x200 | 0xba},	// 'ｹ'
	{0xba,	0x200 | 0x42},	// 'ｺ'
	{0xbb,	0x200 | 0x58},	// 'ｻ'
	{0xbc,	0x200 | 0x44},	// 'ｼ'
	{0xbd,	0x200 | 0x52},	// 'ｽ'
	{0xbe,	0x200 | 0x50},	// 'ｾ'
	{0xbf,	0x200 | 0x43},	// 'ｿ'
	{0xc0,	0x200 | 0x51},	// 'ﾀ'
	{0xc1,	0x200 | 0x41},	// 'ﾁ'
	{0xc2,	0x200 | 0x5a},	// 'ﾂ'
	{0xc3,	0x200 | 0x57},	// 'ﾃ'
	{0xc4,	0x200 | 0x53},	// 'ﾄ'
	{0xc5,	0x200 | 0x55},	// 'ﾅ'
	{0xc6,	0x200 | 0x49},	// 'ﾆ'
	{0xc7,	0x200 | 0x31},	// 'ﾇ'
	{0xc8,	0x200 | 0xbc},	// 'ﾈ'
	{0xc9,	0x200 | 0x4b},	// 'ﾉ'
	{0xca,	0x200 | 0x46},	// 'ﾊ'
	{0xcb,	0x200 | 0x56},	// 'ﾋ'
	{0xcc,	0x200 | 0x32},	// 'ﾌ'
	{0xcd,	0x200 | 0xde},	// 'ﾍ'
	{0xce,	0x200 | 0xbd},	// 'ﾎ'
	{0xcf,	0x200 | 0x4a},	// 'ﾏ'
	{0xd0,	0x200 | 0x4e},	// 'ﾐ'
	{0xd1,	0x200 | 0xdd},	// 'ﾑ'
	{0xd2,	0x200 | 0xbf},	// 'ﾒ'
	{0xd3,	0x200 | 0x4d},	// 'ﾓ'
	{0xd4,	0x200 | 0x37},	// 'ﾔ'
	{0xd5,	0x200 | 0x38},	// 'ﾕ'
	{0xd6,	0x200 | 0x39},	// 'ﾖ'
	{0xd7,	0x200 | 0x4f},	// 'ﾗ'
	{0xd8,	0x200 | 0x4c},	// 'ﾘ'
	{0xd9,	0x200 | 0xbe},	// 'ﾙ'
	{0xda,	0x200 | 0xbb},	// 'ﾚ'
	{0xdb,	0x200 | 0xe2},	// 'ﾛ'
	{0xdc,	0x200 | 0x30},	// 'ﾜ'
	{0xdd,	0x200 | 0x59},	// 'ﾝ'
	{0xde,	0x200 | 0xc0},	// 'ﾞ'
	{0xdf,	0x200 | 0xdb},	// 'ﾟ'
	{-1, -1},
};

void EmuThreadClass::do_start_auto_key(QString ctext)
{
#ifdef USE_AUTO_KEY
	if(using_flags->is_use_auto_key()) {
		QTextCodec *codec = QTextCodec::codecForName("Shift-Jis");
		QByteArray array;
		clipBoardText = ctext;
		array = codec->fromUnicode(clipBoardText);
		if(clipBoardText.size() > 0) {
			static int auto_key_table[256];
			static bool initialized = false;
			if(!initialized) {
				memset(auto_key_table, 0, sizeof(auto_key_table));
				if(using_flags->is_use_auto_key_us()) {
					for(int i = 0;; i++) {
						if(auto_key_table_base_us[i][0] == -1) {
							break;
						}
						auto_key_table[auto_key_table_base_us[i][0]] = auto_key_table_base_us[i][1];
					}
				} else {
					for(int i = 0;; i++) {
						if(auto_key_table_base[i][0] == -1) {
							break;
						}
						auto_key_table[auto_key_table_base[i][0]] = auto_key_table_base[i][1];
					}
				}
				if(using_flags->is_use_vm_auto_key_table()) {
					if(using_flags->is_use_auto_key_us()) {
						for(int i = 0;; i++) {
							if(auto_key_table_base_us[i][0] == -1) {
								break;
							}
							auto_key_table[auto_key_table_base_us[i][0]] = auto_key_table_base_us[i][1];
						}
					} else {
						for(int i = 0;; i++) {
							if(auto_key_table_base[i][0] == -1) {
								break;
							}
							auto_key_table[auto_key_table_base[i][0]] = auto_key_table_base[i][1];
						}
					}
				}
				initialized = true;
			}
			
			FIFO* auto_key_buffer = emu->get_auto_key_buffer();
			auto_key_buffer->clear();
			
			int size = strlen(array.constData()), prev_kana = 0;
			const char *buf = (char *)(array.constData());
			
			for(int i = 0; i < size; i++) {
				int code = buf[i] & 0xff;
				if((0x81 <= code && code <= 0x9f) || 0xe0 <= code) {
					i++;	// kanji ?
					continue;
				}
				// Effect [Enter] even Unix etc.(0x0a should not be ignored). 
				//else if(code == 0xa) { 
				//continue;	// cr-lf
				//}
				if((code = auto_key_table[code]) != 0) {
					int kana = code & 0x200;
					if(prev_kana != kana) {
						auto_key_buffer->write(0xf2);
					}
					prev_kana = kana;
					if(using_flags->is_use_auto_key_no_caps()) {
						if((code & 0x100) && !(code & (0x400 | 0x800))) {
							auto_key_buffer->write((code & 0xff) | 0x100);
						} else {
							auto_key_buffer->write(code & 0xff);
						}
					} else if(using_flags->is_use_auto_key_caps()) {
						if(code & (0x100 | 0x800)) {
							auto_key_buffer->write((code & 0xff) | 0x100);
						} else {
							auto_key_buffer->write(code & 0xff);
						}
					} else if(code & (0x100 | 0x400)) {
						auto_key_buffer->write((code & 0xff) | 0x100);
					} else {
						auto_key_buffer->write(code & 0xff);
					}
				}
			}
	   		
			if(prev_kana) {
				auto_key_buffer->write(0xf2);
			}
			p_emu->stop_auto_key();
			p_emu->start_auto_key();
		}
	}
#endif	
}

void EmuThreadClass::do_stop_auto_key(void)
{
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "AutoKey: stop\n");
#if defined(USE_AUTO_KEY)   
	p_emu->stop_auto_key();
#endif   
}

void EmuThreadClass::do_write_protect_disk(int drv, bool flag)
{
#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
	p_emu->is_floppy_disk_protected(drv, flag);
#endif	
}

void EmuThreadClass::do_close_disk(int drv)
{
#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
	p_emu->close_floppy_disk(drv);
	p_emu->d88_file[drv].bank_num = 0;
	p_emu->d88_file[drv].cur_bank = -1;
#endif	
}

void EmuThreadClass::do_open_disk(int drv, QString path, int bank)
{
#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
	QByteArray localPath = path.toLocal8Bit();
   
	p_emu->d88_file[drv].bank_num = 0;
	p_emu->d88_file[drv].cur_bank = -1;
	
	if(check_file_extension(localPath.constData(), ".d88") || check_file_extension(localPath.constData(), ".d77")) {
		
		FILEIO *fio = new FILEIO();
		if(fio->Fopen(localPath.constData(), FILEIO_READ_BINARY)) {
			try {
				fio->Fseek(0, FILEIO_SEEK_END);
				int file_size = fio->Ftell(), file_offset = 0;
				while(file_offset + 0x2b0 <= file_size && p_emu->d88_file[drv].bank_num < MAX_D88_BANKS) {
					fio->Fseek(file_offset, FILEIO_SEEK_SET);
					char tmp[18];
					memset(tmp, 0x00, sizeof(tmp));
					fio->Fread(tmp, 17, 1);
					memset(p_emu->d88_file[drv].disk_name[p_emu->d88_file[drv].bank_num], 0x00, 128);
					if(strlen(tmp) > 0) Convert_CP932_to_UTF8(p_emu->d88_file[drv].disk_name[p_emu->d88_file[drv].bank_num], tmp, 127, 17);
					
					fio->Fseek(file_offset + 0x1c, FILEIO_SEEK_SET);
					file_offset += fio->FgetUint32_LE();
					p_emu->d88_file[drv].bank_num++;
				}
				strcpy(emu->d88_file[drv].path, path.toUtf8().constData());
				if(bank >= p_emu->d88_file[drv].bank_num) bank = p_emu->d88_file[drv].bank_num - 1;
				if(bank < 0) bank = 0;
				p_emu->d88_file[drv].cur_bank = bank;
			}
			catch(...) {
				bank = 0;
				p_emu->d88_file[drv].bank_num = 0;
			}
		   	fio->Fclose();
		}
	   	delete fio;
	} else {
	   bank = 0;
	}
	p_emu->open_floppy_disk(drv, localPath.constData(), bank);
	emit sig_update_recent_disk(drv);
#endif	
}
void EmuThreadClass::do_play_tape(QString name)
{
#if defined(USE_TAPE)
	p_emu->play_tape(name.toLocal8Bit().constData());
#endif
}

void EmuThreadClass::do_rec_tape(QString name)
{
#if defined(USE_TAPE)
	p_emu->rec_tape(name.toLocal8Bit().constData());
#endif
}

void EmuThreadClass::do_close_tape(void)
{
#if defined(USE_TAPE)
	p_emu->close_tape();
#endif
}

void EmuThreadClass::do_cmt_push_play(void)
{
#if defined(USE_TAPE_BUTTON)
	p_emu->push_play();
#endif
}

void EmuThreadClass::do_cmt_push_stop(void)
{
#if defined(USE_TAPE_BUTTON)
	p_emu->push_stop();
#endif
}

void EmuThreadClass::do_cmt_push_fast_forward(void)
{
#if defined(USE_TAPE_BUTTON)
	p_emu->push_fast_forward();
#endif
}

void EmuThreadClass::do_cmt_push_fast_rewind(void)
{
#if defined(USE_TAPE_BUTTON)
	p_emu->push_fast_rewind();
#endif
}

void EmuThreadClass::do_cmt_push_apss_forward(void)
{
#if defined(USE_TAPE_BUTTON)
	p_emu->push_apss_forward();
#endif
}

void EmuThreadClass::do_cmt_push_apss_rewind(void)
{
#if defined(USE_TAPE_BUTTON)
	p_emu->push_apss_rewind();
#endif	
}


void EmuThreadClass::do_write_protect_quickdisk(int drv, bool flag)
{
#if defined(USE_QD1)
	//p_emu->write_protect_Qd(drv, flag);
#endif	
}

void EmuThreadClass::do_close_quickdisk(int drv)
{
#if defined(USE_QD1)
	p_emu->close_quick_disk(drv);
#endif	
}

void EmuThreadClass::do_open_quickdisk(int drv, QString path)
{
#if defined(USE_QD1)
	p_emu->open_quick_disk(drv, path.toLocal8Bit().constData());
#endif	
}

void EmuThreadClass::do_open_cdrom(QString path)
{
#ifdef USE_COMPACT_DISC
	p_emu->open_compact_disc(path.toLocal8Bit().constData());
#endif	
}
void EmuThreadClass::do_eject_cdrom(void)
{
#ifdef USE_COMPACT_DISC
	p_emu->close_compact_disc();
#endif	
}

void EmuThreadClass::do_close_cart(int drv)
{
#ifdef USE_CART1
	p_emu->close_cart(drv);
#endif	
}

void EmuThreadClass::do_open_cart(int drv, QString path)
{
#ifdef USE_CART1
	p_emu->open_cart(drv, path.toLocal8Bit().constData());
#endif	
}


void EmuThreadClass::do_close_laser_disk(void)
{
#ifdef USE_LASER_DISK
	p_emu->close_laser_disk();
#endif
}

void EmuThreadClass::do_open_laser_disk(QString path)
{
#ifdef USE_LASER_DISK
	p_emu->open_laser_disk(path.toLocal8Bit().constData());
#endif	
}

void EmuThreadClass::do_load_binary(int drv, QString path)
{
#ifdef USE_BINARY_FILE1
	p_emu->load_binary(drv, path.toLocal8Bit().constData());
#endif	
}

void EmuThreadClass::do_save_binary(int drv, QString path)
{
#ifdef USE_BINARY_FILE1
	p_emu->save_binary(drv, path.toLocal8Bit().constData());
#endif	
}


void EmuThreadClass::do_write_protect_bubble_casette(int drv, bool flag)
{
#ifdef USE_BUBBLE1
	p_emu->is_bubble_casette_protected(drv, flag);
#endif	
}

void EmuThreadClass::do_close_bubble_casette(int drv)
{
#ifdef USE_BUBBLE1
	p_emu->close_bubble_casette(drv);
	p_emu->b77_file[drv].bank_num = 0;
	p_emu->b77_file[drv].cur_bank = -1;
#endif	
}

void EmuThreadClass::do_open_bubble_casette(int drv, QString path, int bank)
{
#ifdef USE_BUBBLE1
	QByteArray localPath = path.toLocal8Bit();
   
	p_emu->b77_file[drv].bank_num = 0;
	p_emu->b77_file[drv].cur_bank = -1;
	
	if(check_file_extension(localPath.constData(), ".b77")) {
		
		FILEIO *fio = new FILEIO();
		if(fio->Fopen(localPath.constData(), FILEIO_READ_BINARY)) {
			try {
				fio->Fseek(0, FILEIO_SEEK_END);
				int file_size = fio->Ftell(), file_offset = 0;
				while(file_offset + 0x2b0 <= file_size && p_emu->d88_file[drv].bank_num < MAX_B77_BANKS) {
					fio->Fseek(file_offset, FILEIO_SEEK_SET);
					char tmp[18];
					memset(tmp, 0x00, sizeof(tmp));
					fio->Fread(tmp, 16, 1);
					memset(p_emu->b77_file[drv].bubble_name[p_emu->b77_file[drv].bank_num], 0x00, 128);
					if(strlen(tmp) > 0) Convert_CP932_to_UTF8(p_emu->b77_file[drv].bubble_name[p_emu->b77_file[drv].bank_num], tmp, 127, 17);
					
					fio->Fseek(file_offset + 0x1c, FILEIO_SEEK_SET);
					file_offset += fio->FgetUint32_LE();
					p_emu->b77_file[drv].bank_num++;
				}
				strcpy(p_emu->b77_file[drv].path, path.toUtf8().constData());
				if(bank >= p_emu->b77_file[drv].bank_num) bank = p_emu->b77_file[drv].bank_num - 1;
				if(bank < 0) bank = 0;
				p_emu->b77_file[drv].cur_bank = bank;
			}
			catch(...) {
				bank = 0;
				p_emu->b77_file[drv].bank_num = 0;
			}
		   	fio->Fclose();
		}
	   	delete fio;
	} else {
	   bank = 0;
	}
	p_emu->open_bubble_casette(drv, localPath.constData(), bank);
	emit sig_update_recent_bubble(drv);
#endif	
}

void EmuThreadClass::print_framerate(int frames)
{
	if(frames >= 0) draw_frames += frames;
	if(calc_message) {
		qint64 current_time = tick_timer.elapsed();
		if(update_fps_time <= current_time && update_fps_time != 0) {
			_TCHAR buf[256];
			QString message;
			int ratio = (int)(100.0 * (double)draw_frames / (double)total_frames + 0.5);

#ifdef USE_POWER_OFF
				if(MainWindow->GetPowerState() == false){ 	 
					snprintf(buf, 255, _T("*Power OFF*"));
				} else {
#endif // USE_POWER_OFF		
					if(p_emu->message_count > 0) {
						snprintf(buf, 255, _T("%s - %s"), DEVICE_NAME, p_emu->message);
						p_emu->message_count--;
					} else {
						snprintf(buf, 255, _T("%s - %d fps (%d %%)"), DEVICE_NAME, draw_frames, ratio);
					}
#ifdef USE_POWER_OFF
				} 
#endif // USE_POWER_OFF	 
				message = buf;
				emit message_changed(message);
				emit window_title_changed(message);
				update_fps_time += 1000;
				total_frames = draw_frames = 0;
				
			}
			if(update_fps_time <= current_time) {
				update_fps_time = current_time + 1000;
			}
			calc_message = false;  
		} else {
			calc_message = true;
		}
}

void EmuThreadClass::do_draw_timing(bool f)
{
//	draw_timing = f;
}

void EmuThreadClass::sample_access_drv(void)
{
	uint32_t access_drv;
	QString alamp;
	QString tmpstr;
	QString iname;
	int i;
#if defined(USE_QD1)
# if defined(USE_ACCESS_LAMP)      
	access_drv = p_emu->get_access_lamp_status();
# endif
	for(i = 0; i < MAX_QD ; i++) {
		if(p_emu->is_quick_disk_inserted(i)) {
		//	     printf("%d\n", access_drv);
# if defined(USE_ACCESS_LAMP)      
			if(i == (access_drv - 1)) {
				alamp = QString::fromUtf8("● ");
			} else {
				alamp = QString::fromUtf8("○ ");
			}
			tmpstr = QString::fromUtf8("QD");
			tmpstr = alamp + tmpstr + QString::number(i) + QString::fromUtf8(":");
# else
			tmpstr = QString::fromUtf8("QD");
			tmpstr = tmpstr + QString::number(i) + QString::fromUtf8(":");
# endif
			iname = QString::fromUtf8("*Inserted*");
			tmpstr = tmpstr + iname;
		} else {
			tmpstr = QString::fromUtf8("× QD") + QString::number(i) + QString::fromUtf8(":");
			tmpstr = tmpstr + QString::fromUtf8(" ");
		}
		if(tmpstr != qd_text[i]) {
			emit sig_change_osd_qd(i, tmpstr);
			qd_text[i] = tmpstr;
		}
	}
#endif

#if defined(USE_FD1)
# if defined(USE_ACCESS_LAMP)      
	access_drv = p_emu->get_access_lamp_status();
# endif
	for(i = 0; i < MAX_FD; i++) {
		if(p_emu->is_floppy_disk_inserted(i)) {
# if defined(USE_ACCESS_LAMP)      
			if(i == (access_drv - 1)) {
				alamp = QString::fromUtf8("<FONT COLOR=RED>●</FONT> ");
			} else {
				alamp = QString::fromUtf8("○ ");
			}
			tmpstr = QString::fromUtf8("FD");
			tmpstr = alamp + tmpstr + QString::number(i) + QString::fromUtf8(":");
# else
			tmpstr = QString::fromUtf8("FD");
			tmpstr = tmpstr + QString::number(i) + QString::fromUtf8(":");
# endif
			if(emu->d88_file[i].bank_num > 0) {
				iname = QString::fromUtf8(emu->d88_file[i].disk_name[emu->d88_file[i].cur_bank]);
			} else {
				iname = QString::fromUtf8("*Inserted*");
			}
			tmpstr = tmpstr + iname;
		} else {
			tmpstr = QString::fromUtf8("× FD") + QString::number(i) + QString::fromUtf8(":");
			tmpstr = tmpstr + QString::fromUtf8(" ");
		}

		if(tmpstr != fd_text[i]) {
			emit sig_change_osd_fd(i, tmpstr);
			fd_text[i] = tmpstr;
		}
	}
#endif
#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
	if(p_emu->is_tape_inserted()) {
		int tape_counter = p_emu->get_tape_position();
		tmpstr = QString::fromUtf8("");
		if(p_emu->is_tape_playing()) {
			tmpstr = QString::fromUtf8("<FONT COLOR=BLUE>▶ </FONT>");
		} else if(p_emu->is_tape_recording()) {
			tmpstr = QString::fromUtf8("<FONT COLOR=RED>● </FONT>");
		} else {
			tmpstr = QString::fromUtf8("<FONT COLOR=BLACK>■ </FONT>");
		}
		if(tape_counter >= 100) {
			tmpstr = tmpstr + QString::fromUtf8("BOTTOM");
		} else if(tape_counter >= 0) {
			tmpstr = tmpstr + QString::number(tape_counter) + QString::fromUtf8("%");
		} else {
			tmpstr = tmpstr + QString::fromUtf8("TOP");
		}
	} else {
		tmpstr = QString::fromUtf8("EMPTY");
	}
	if(tmpstr != cmt_text) {
		emit sig_change_osd_cmt(tmpstr);
		cmt_text = tmpstr;
	}
#endif
#if defined(USE_COMPACT_DISC)
	if(p_emu->is_compact_disc_inserted()) {
		tmpstr = QString::fromUtf8("○");
	} else {
		tmpstr = QString::fromUtf8("Not Inserted");
	}
	if(tmpstr != cdrom_text) {
		emit sig_change_osd_cdrom(tmpstr);
		cdrom_text = tmpstr;
	}
#endif
#if defined(USE_BUBBLE1)
	for(i = 0; i < MAX_BUBBLE ; i++) {
		if(p_emu->is_bubble_casette_inserted(i)) {
			//if(i == (access_drv - 1)) {
			alamp = QString::fromUtf8("● ");
			//} else {
			//alamp = QString::fromUtf8("○ ");
			//}
			tmpstr = QString::fromUtf8("BUB");
			tmpstr = alamp + tmpstr + QString::number(i) + QString::fromUtf8(":");
		} else {
			tmpstr = QString::fromUtf8("× BUB") + QString::number(i) + QString::fromUtf8(":");
			tmpstr = tmpstr + QString::fromUtf8(" ");
		}
		if(tmpstr != bubble_text[i]) {
			emit sig_change_osd_bubble(i, tmpstr);
			bubble_text[i] = tmpstr;
		}
	}
#endif
}

void EmuThreadClass::doWork(const QString &params)
{
	int interval = 0, sleep_period = 0;
	int run_frames;
	bool now_skip;
	qint64 current_time;
	bool first = true;
#ifdef USE_LED_DEVICE
	uint32_t led_data = 0x00000000;
	uint32_t led_data_old = 0x00000000;
#endif
#if defined(USE_TAPE) && !defined(TAPE_BINARY_ONLY)
	bool tape_flag;
	int tpos;
#endif
#ifdef USE_DIG_RESOLUTION
	int width, height;
#endif
	QString ctext;
	bool req_draw = true;
	bool vert_line_bak = using_flags->get_config_ptr()->opengl_scanline_vert;
	bool horiz_line_bak = using_flags->get_config_ptr()->opengl_scanline_horiz;
	bool gl_crt_filter_bak = using_flags->get_config_ptr()->use_opengl_filters;
	int opengl_filter_num_bak = using_flags->get_config_ptr()->opengl_filter_num;
	//uint32_t key_mod_old = 0xffffffff;
	int no_draw_count = 0;	
	bool prevRecordReq;

	doing_debug_command = false;
	
	ctext.clear();
//	draw_timing = false;
	bResetReq = false;
	bSpecialResetReq = false;
	bLoadStateReq = false;
	bSaveStateReq = false;
	bUpdateConfigReq = false;
	bStartRecordSoundReq = false;
	bStopRecordSoundReq = false;
	bStartRecordMovieReq = false;
	record_fps = -1;

	next_time = 0;
	mouse_flag = false;

	key_repeat = false;
	key_down_code = 0;
	key_up_code = 0;
	key_mod_code = 0;
	key_changed = false;
#if defined(USE_QD1)
	for(int i = 0; i < 2; i++) qd_text[i].clear();
#endif
#if defined(USE_FD1)
	for(int i = 0; i < MAX_FD; i++) fd_text[i].clear();
#endif
#if defined(USE_TAPE)
	cmt_text.clear();
#endif
#if defined(USE_COMPACT_DISC)
	cdrom_text.clear();
#endif
#if defined(USE_BUBBLE1)
	for(int i = 0; i < MAX_BUBBLE; i++) bubble_text[i].clear();
#endif
	
	do {
		//p_emu->SetHostCpus(this->idealThreadCount());
   		if(MainWindow == NULL) {
			if(bRunThread == false){
				goto _exit;
			}
			msleep(10);
			continue;
		}
		if(first) {
#ifdef USE_LED_DEVICE
			emit sig_send_data_led((quint32)led_data);
#endif
			first = false;
		}
		interval = 0;
		sleep_period = 0;
		if(p_emu) {
			// drive machine
#ifdef USE_STATE
			if(bLoadStateReq != false) {
				p_emu->load_state();
				bLoadStateReq = false;
				req_draw = true;
			}
#endif			
			if(bResetReq != false) {
				p_emu->reset();
				bResetReq = false;
				req_draw = true;
			}
#ifdef USE_SPECIAL_RESET
			if(bSpecialResetReq != false) {
				p_emu->special_reset();
				bSpecialResetReq = false;
			}
#endif
#ifdef USE_STATE
			if(bSaveStateReq != false) {
				p_emu->save_state();
				bSaveStateReq = false;
			}
#endif
#if defined(USE_MINIMUM_RENDERING)
			if((vert_line_bak != p_config->opengl_scanline_vert) ||
			   (horiz_line_bak != p_config->opengl_scanline_horiz) ||
			   (gl_crt_filter_bak != p_config->use_opengl_filters) ||
			   (opengl_filter_num_bak != p_config->opengl_filter_num)) req_draw = true;
			vert_line_bak = p_config->opengl_scanline_vert;
			horiz_line_bak = p_config->opengl_scanline_horiz;
			gl_crt_filter_bak = p_config->use_opengl_filters;
			opengl_filter_num_bak = p_config->opengl_filter_num;
#endif
			if(bStartRecordSoundReq != false) {
				p_emu->start_record_sound();
				bStartRecordSoundReq = false;
				req_draw = true;
			}
			if(bStopRecordSoundReq != false) {
				p_emu->stop_record_sound();
				bStopRecordSoundReq = false;
				req_draw = true;
			}
			if(bUpdateConfigReq != false) {
				p_emu->update_config();
				bUpdateConfigReq = false;
				req_draw = true;
			}
			if(bStartRecordMovieReq != false) {
				if(!prevRecordReq && (record_fps > 0) && (record_fps < 75)) { 		
					p_emu->start_record_video(record_fps);
					prevRecordReq = true;
				}
			} else {
				if(prevRecordReq) {
					p_emu->stop_record_video();
					record_fps = -1;
					prevRecordReq = false;
				}
			}
		   
		   

#if defined(USE_MOUSE)	// Will fix
			emit sig_is_enable_mouse(p_emu->is_mouse_enabled());
#endif			
#if defined(USE_SOUND_VOLUME)
			for(int ii = 0; ii < USE_SOUND_VOLUME; ii++) {
				if(bUpdateVolumeReq[ii]) {
					p_emu->set_sound_device_volume(ii, p_config->sound_volume_l[ii], p_config->sound_volume_r[ii]);
					bUpdateVolumeReq[ii] = false;
				}
			}
#endif
			if(key_changed) {
				p_emu->key_modifiers(key_mod_code);
				key_changed = false;
			}
			if(key_down_code != 0) {
				p_emu->key_down(key_down_code, key_repeat);
				key_down_code = 0;
			}
			if(key_up_code != 0) {
				p_emu->key_modifiers(key_mod_code);
				p_emu->key_up(key_up_code);
				key_up_code = 0;
			}
			
			run_frames = p_emu->run();
			total_frames += run_frames;
#if defined(USE_MINIMUM_RENDERING)
			req_draw |= p_emu->is_screen_changed();
#else
			req_draw = true;
#endif			
#ifdef USE_LED_DEVICE
	   		led_data = p_emu->get_led_status();
			if(led_data != led_data_old) {
				emit sig_send_data_led((quint32)led_data);
				led_data_old = led_data;
			}
#endif
			sample_access_drv();

			interval += get_interval();
			now_skip = p_emu->is_frame_skippable() && !p_emu->is_video_recording();

			if((prev_skip && !now_skip) || next_time == 0) {
				next_time = tick_timer.elapsed();
			}
			if(!now_skip) {
				next_time += interval;
			}
			prev_skip = now_skip;
			//printf("p_emu::RUN Frames = %d SKIP=%d Interval = %d NextTime = %d\n", run_frames, now_skip, interval, next_time);
			if(next_time > tick_timer.elapsed()) {
				//  update window if enough time
//				draw_timing = false;
				if(!req_draw) {
					no_draw_count++;
					if(no_draw_count > (int)(FRAMES_PER_SEC / 4)) {
						req_draw = true;
						no_draw_count = 0;
					}
				} else {
					no_draw_count = 0;
				}
				emit sig_draw_thread(req_draw);
				skip_frames = 0;
			
				// sleep 1 frame priod if need
				current_time = tick_timer.elapsed();
				if((int)(next_time - current_time) >= 10) {
					sleep_period = next_time - current_time;
				}
			} else if(++skip_frames > MAX_SKIP_FRAMES) {
				// update window at least once per 10 frames
//				draw_timing = false;
				emit sig_draw_thread(true);
				no_draw_count = 0;
				skip_frames = 0;
				qint64 tt = tick_timer.elapsed();
				next_time = tt + get_interval();
				sleep_period = next_time - tt;
			}
		}
		req_draw = false;
		if(bRunThread == false){
			goto _exit;
		}
		if(sleep_period <= 0) sleep_period = 1;
		msleep(sleep_period);
	} while(1);
_exit:
	//emit quit_draw_thread();
	AGAR_DebugLog(AGAR_LOG_DEBUG, "EmuThread : EXIT");
	emit sig_finished();
	this->quit();
}

void EmuThreadClass::doSetDisplaySize(int w, int h, int ww, int wh)
{
	p_emu->suspend();
	//p_emu->set_vm_screen_size(w, h, -1, -1, ww, wh);
	p_emu->set_host_window_size(w, h, true);
}

void EmuThreadClass::doUpdateVolumeLevel(int num, int level)
{
#if defined(USE_SOUND_VOLUME)
	if((num < USE_SOUND_VOLUME) && (num >= 0)) {
		calc_volume_from_level(num, level);
		bUpdateVolumeReq[num] = true;
	}
#endif	
}

void EmuThreadClass::doUpdateVolumeBalance(int num, int level)
{
#if defined(USE_SOUND_VOLUME)
	if((num < USE_SOUND_VOLUME) && (num >= 0)) {
		calc_volume_from_balance(num, level);
		bUpdateVolumeReq[num] = true;
	}
#endif	
}


void EmuThreadClass::doUpdateConfig()
{
	bUpdateConfigReq = true;
}

void EmuThreadClass::doStartRecordSound()
{
	bStartRecordSoundReq = true;
}

void EmuThreadClass::doStopRecordSound()
{
	bStopRecordSoundReq = true;
}

void EmuThreadClass::doReset()
{
	bResetReq = true;
}

void EmuThreadClass::doSpecialReset()
{
	bSpecialResetReq = true;
}

void EmuThreadClass::doLoadState()
{
	bLoadStateReq = true;
}

void EmuThreadClass::doSaveState()
{
	bSaveStateReq = true;
}

void EmuThreadClass::doStartRecordVideo(int fps)
{
	record_fps = fps;
	bStartRecordMovieReq = true;
}

void EmuThreadClass::doStopRecordVideo()
{
	bStartRecordMovieReq = false;
}


// Debugger
#if defined(USE_DEBUGGER)
extern int debugger_command(debugger_thread_t *p, _TCHAR *command, _TCHAR *prev_command, bool cp932);
#endif
void EmuThreadClass::do_call_debugger_command(QString s)
{
#if defined(USE_DEBUGGER)
	_TCHAR command[MAX_COMMAND_LEN + 1];

	if(doing_debug_command) {
		emit sig_debugger_input(s);
		return;
	}
	memset(command, 0x00, MAX_COMMAND_LEN + 1);
	if(s.isEmpty()) {
		strncpy(command, dbg_prev_command, MAX_COMMAND_LEN);
	} else {
		strncpy(command, s.toUtf8().constData(), MAX_COMMAND_LEN);
	}
	doing_debug_command = true;
	if(debugger_command(&(p_emu->debugger_thread_param), command, dbg_prev_command, false) < 0) {
		do_close_debugger();
	}
	doing_debug_command = false;
#endif
}

void EmuThreadClass::do_close_debugger(void)
{
#if defined(USE_DEBUGGER)
	emit sig_quit_debugger();
#endif
}

bool EmuThreadClass::now_debugging() {
#if defined(USE_DEBUGGER)
	return p_emu->now_debugging;
#else
	return false;
#endif
}
