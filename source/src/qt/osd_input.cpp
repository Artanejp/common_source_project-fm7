/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[Qt input ]
*/

#include <Qt>
#include <QApplication>
#include <SDL.h>

#include "osd.h"
#include "../fifo.h"
#include "fileio.h"

#include "qt_input.h"
#include "qt_gldraw.h"
#include "qt_main.h"

#include "mainwidget.h"
#include "agar_logger.h"

#ifdef NOTIFY_KEY_DOWN_LR_SHIFT
#define VK_SHIFT_TEMP	VK_LSHIFT
#else
#define VK_SHIFT_TEMP	VK_SHIFT
#endif
#define KEY_KEEP_FRAMES	3

#ifdef USE_SHIFT_NUMPAD_KEY
static const int numpad_table[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x65, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x00, 0x69, 0x63, 0x61, 0x67, 0x64, 0x68, 0x66, 0x62, 0x00, 0x00, 0x00, 0x00, 0x60, 0x6e, 0x00,
	0x00, 0x69, 0x63, 0x61, 0x67, 0x64, 0x68, 0x66, 0x62, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00,	// remove shift + period
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif

#ifdef USE_AUTO_KEY
static const int autokey_table_base[][2] = {
	// 0x100: shift
	// 0x200: kana
	// 0x400: alphabet
	// 0x800: ALPHABET
	{0x0d,	0x000 | 0x0d},	// Enter
	{0x20,	0x000 | 0x20},	// ' '
#ifdef AUTO_KEY_US
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
#else
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
#endif
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
#ifdef AUTO_KEY_US
	{0x3a,	0x100 | 0xbb},	// ':'
	{0x3b,	0x000 | 0xbb},	// ';'
	{0x3c,	0x100 | 0xbc},	// '<'
	{0x3d,	0x000 | 0xde},	// '='
	{0x3e,	0x100 | 0xbe},	// '>'
	{0x3f,	0x100 | 0xbf},	// '?'
	{0x40,	0x100 | 0x32},	// '@'
#else
	{0x3a,	0x000 | 0xba},	// ':'
	{0x3b,	0x000 | 0xbb},	// ';'
	{0x3c,	0x100 | 0xbc},	// '<'
	{0x3d,	0x100 | 0xbd},	// '='
	{0x3e,	0x100 | 0xbe},	// '>'
	{0x3f,	0x100 | 0xbf},	// '?'
	{0x40,	0x000 | 0xc0},	// '@'
#endif
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
#ifdef AUTO_KEY_US
	{0x5b,	0x000 | 0xc0},	// '['
	{0x5c,	0x000 | 0xe2},	// '\'
	{0x5d,	0x000 | 0xdb},	// ']'
	{0x5e,	0x100 | 0x36},	// '^'
	{0x5f,	0x100 | 0xbd},	// '_'
	{0x60,	0x000 | 0xdd},	// '`'
#else
	{0x5b,	0x000 | 0xdb},	// '['
	{0x5c,	0x000 | 0xdc},	// '\'
	{0x5d,	0x000 | 0xdd},	// ']'
	{0x5e,	0x000 | 0xde},	// '^'
	{0x5f,	0x100 | 0xe2},	// '_'
	{0x60,	0x100 | 0xc0},	// '`'
#endif
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
#ifdef AUTO_KEY_US
	{0x7b,	0x100 | 0xc0},	// '{'
	{0x7c,	0x100 | 0xe2},	// '|'
	{0x7d,	0x100 | 0xdb},	// '}'
	{0x7e,	0x100 | 0xdd},	// '~'
#else
	{0x7b,	0x100 | 0xdb},	// '{'
	{0x7c,	0x100 | 0xdc},	// '|'
	{0x7d,	0x100 | 0xdd},	// '}'
	{0x7e,	0x100 | 0xde},	// '~'
#endif
	{0xa1,	0x300 | 0xbe},	// '¡'
	{0xa2,	0x300 | 0xdb},	// '¢'
	{0xa3,	0x300 | 0xdd},	// '£'
	{0xa4,	0x300 | 0xbc},	// '¤'
	{0xa5,	0x300 | 0xbf},	// '¥'
	{0xa6,	0x300 | 0x30},	// '¦'
	{0xa7,	0x300 | 0x33},	// '§'
	{0xa8,	0x300 | 0x45},	// '¨'
	{0xa9,	0x300 | 0x34},	// '©'
	{0xaa,	0x300 | 0x35},	// 'ª'
	{0xab,	0x300 | 0x36},	// '«'
	{0xac,	0x300 | 0x37},	// '¬'
	{0xad,	0x300 | 0x38},	// '­'
	{0xae,	0x300 | 0x39},	// '®'
	{0xaf,	0x300 | 0x5a},	// '¯'
	{0xb0,	0x200 | 0xdc},	// '°'
	{0xb1,	0x200 | 0x33},	// '±'
	{0xb2,	0x200 | 0x45},	// '²'
	{0xb3,	0x200 | 0x34},	// '³'
	{0xb4,	0x200 | 0x35},	// '´'
	{0xb5,	0x200 | 0x36},	// 'µ'
	{0xb6,	0x200 | 0x54},	// '¶'
	{0xb7,	0x200 | 0x47},	// '·'
	{0xb8,	0x200 | 0x48},	// '¸'
	{0xb9,	0x200 | 0xba},	// '¹'
	{0xba,	0x200 | 0x42},	// 'º'
	{0xbb,	0x200 | 0x58},	// '»'
	{0xbc,	0x200 | 0x44},	// '¼'
	{0xbd,	0x200 | 0x52},	// '½'
	{0xbe,	0x200 | 0x50},	// '¾'
	{0xbf,	0x200 | 0x43},	// '¿'
	{0xc0,	0x200 | 0x51},	// 'À'
	{0xc1,	0x200 | 0x41},	// 'Á'
	{0xc2,	0x200 | 0x5a},	// 'Â'
	{0xc3,	0x200 | 0x57},	// 'Ã'
	{0xc4,	0x200 | 0x53},	// 'Ä'
	{0xc5,	0x200 | 0x55},	// 'Å'
	{0xc6,	0x200 | 0x49},	// 'Æ'
	{0xc7,	0x200 | 0x31},	// 'Ç'
	{0xc8,	0x200 | 0xbc},	// 'È'
	{0xc9,	0x200 | 0x4b},	// 'É'
	{0xca,	0x200 | 0x46},	// 'Ê'
	{0xcb,	0x200 | 0x56},	// 'Ë'
	{0xcc,	0x200 | 0x32},	// 'Ì'
	{0xcd,	0x200 | 0xde},	// 'Í'
	{0xce,	0x200 | 0xbd},	// 'Î'
	{0xcf,	0x200 | 0x4a},	// 'Ï'
	{0xd0,	0x200 | 0x4e},	// 'Ð'
	{0xd1,	0x200 | 0xdd},	// 'Ñ'
	{0xd2,	0x200 | 0xbf},	// 'Ò'
	{0xd3,	0x200 | 0x4d},	// 'Ó'
	{0xd4,	0x200 | 0x37},	// 'Ô'
	{0xd5,	0x200 | 0x38},	// 'Õ'
	{0xd6,	0x200 | 0x39},	// 'Ö'
	{0xd7,	0x200 | 0x4f},	// '×'
	{0xd8,	0x200 | 0x4c},	// 'Ø'
	{0xd9,	0x200 | 0xbe},	// 'Ù'
	{0xda,	0x200 | 0xbb},	// 'Ú'
	{0xdb,	0x200 | 0xe2},	// 'Û'
	{0xdc,	0x200 | 0x30},	// 'Ü'
	{0xdd,	0x200 | 0x59},	// 'Ý'
	{0xde,	0x200 | 0xc0},	// 'Þ'
	{0xdf,	0x200 | 0xdb},	// 'ß'
	{-1, -1},
};
#endif

void OSD::initialize_input()
{
	// initialize status
	memset(key_status, 0, sizeof(key_status));
	memset(joy_status, 0, sizeof(joy_status));
	memset(mouse_status, 0, sizeof(mouse_status));
	// mouse emulation is disenabled
	mouse_enabled = false;
	mouse_ptrx = mouse_oldx = SCREEN_WIDTH / 2;
	mouse_ptry = mouse_oldy = SCREEN_HEIGHT / 2;
	 joy_num = SDL_NumJoysticks();
	 for(int i = 0; i < joy_num && i < 2; i++) {
		joy_mask[i] = 0x0f; // 4buttons
	}
	// initialize keycode convert table
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(bios_path(_T("keycode.cfg")), FILEIO_READ_BINARY)) {
		fio->Fread(keycode_conv, sizeof(keycode_conv), 1);
		fio->Fclose();
	} else {
		for(int i = 0; i < 256; i++) {
			keycode_conv[i] = i;
		}
	}
	delete fio;
	
#ifdef USE_SHIFT_NUMPAD_KEY
	// initialize shift+numpad conversion
	memset(key_converted, 0, sizeof(key_converted));
	key_shift_pressed = key_shift_released = false;
#endif
#ifdef USE_AUTO_KEY
	// initialize autokey
	autokey_buffer = new FIFO(65536);
	autokey_buffer->clear();
	autokey_phase = autokey_shift = 0;
#endif
	
	lost_focus = false;
}

void OSD::release_input()
{
	// release mouse
	if(mouse_enabled) {
		disenable_mouse();
	}
	
#ifdef USE_AUTO_KEY
	// release autokey buffer
	if(autokey_buffer) {
		autokey_buffer->release();
		delete autokey_buffer;
	}
#endif
}

void OSD::update_input()
{
	int *keystat;
	int i_c = 0;;
	bool press_flag = false;
	bool release_flag = false;
#ifdef USE_SHIFT_NUMPAD_KEY
	//update numpad key status
	if(key_shift_pressed && !key_shift_released) {
		if(key_status[VK_SHIFT] == 0) {
			// shift key is newly pressed
			key_status[VK_SHIFT] = 0x80;
# ifdef NOTIFY_KEY_DOWN
			vm->key_down(VK_SHIFT, false);
# endif
		}
	} else if(!key_shift_pressed && key_shift_released) {
		if(key_status[VK_SHIFT] != 0) {
			// shift key is newly released
			key_status[VK_SHIFT] = 0;
# ifdef NOTIFY_KEY_DOWN
			vm->key_up(VK_SHIFT);
# endif
			// check l/r shift
			if(!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
			if(!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
		}
		if(key_status[VK_LSHIFT] != 0) {
			// shift key is newly released
			key_status[VK_LSHIFT] = 0;
# ifdef NOTIFY_KEY_DOWN
			vm->key_up(VK_LSHIFT);
# endif
			// check l/r shift
			if(!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
		}
		if(key_status[VK_RSHIFT] != 0) {
			// shift key is newly released
			key_status[VK_RSHIFT] = 0;
# ifdef NOTIFY_KEY_DOWN
			vm->key_up(VK_RSHIFT);
# endif
			// check l/r shift
			if(!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
		}
	}
	key_shift_pressed = key_shift_released = false;
#endif
	    
	// release keys
#ifdef USE_AUTO_KEY
	if(lost_focus && autokey_phase == 0) {
#else
	if(lost_focus) {
#endif
		// we lost key focus so release all pressed keys
		for(int i = 0; i < 256; i++) {
			if(key_status[i] & 0x80) {
				key_status[i] &= 0x7f;
				release_flag = true;
#ifdef NOTIFY_KEY_DOWN
				if(!key_status[i]) {
					vm->key_up(i);
				}
#endif
			}
		}
	} else {
		for(int i = 0; i < 256; i++) {
			if(key_status[i] & 0x7f) {
				key_status[i] = (key_status[i] & 0x80) | ((key_status[i] & 0x7f) - 1);
				press_flag = true;
#ifdef NOTIFY_KEY_DOWN
				if(!key_status[i]) {
					vm->key_up(i);
				}
#endif
			}
		}
	}
	lost_focus = false;

	// update joystick status
#ifdef USE_KEY_TO_JOY
	// emulate joystick #1 with keyboard
	if(key_status[0x26]) joy_status[0] |= 0x01;	// up
	if(key_status[0x28]) joy_status[0] |= 0x02;	// down
	if(key_status[0x25]) joy_status[0] |= 0x04;	// left
	if(key_status[0x27]) joy_status[0] |= 0x08;	// right
#ifdef KEY_TO_JOY_BUTTON_U
	if(key_status[KEY_TO_JOY_BUTTON_U]) joy_status[0] |= 0x01;
#endif
#ifdef KEY_TO_JOY_BUTTON_D
	if(key_status[KEY_TO_JOY_BUTTON_D]) joy_status[0] |= 0x02;
#endif
#ifdef KEY_TO_JOY_BUTTON_L
	if(key_status[KEY_TO_JOY_BUTTON_L]) joy_status[0] |= 0x04;
#endif
#ifdef KEY_TO_JOY_BUTTON_R
	if(key_status[KEY_TO_JOY_BUTTON_R]) joy_status[0] |= 0x08;
#endif
#ifdef KEY_TO_JOY_BUTTON_1
	if(key_status[KEY_TO_JOY_BUTTON_1]) joy_status[0] |= 0x10;
#endif
#ifdef KEY_TO_JOY_BUTTON_2
	if(key_status[KEY_TO_JOY_BUTTON_2]) joy_status[0] |= 0x20;
#endif
#ifdef KEY_TO_JOY_BUTTON_3
	if(key_status[KEY_TO_JOY_BUTTON_3]) joy_status[0] |= 0x40;
#endif
#ifdef KEY_TO_JOY_BUTTON_4
	if(key_status[KEY_TO_JOY_BUTTON_4]) joy_status[0] |= 0x80;
#endif
#endif

	// swap joystick buttons
	if(config.swap_joy_buttons) {
		for(int i = 0; i < joy_num && i < 2; i++) {
			uint32 b0 = joy_status[i] & 0xaaaaaaa0;
			uint32 b1 = joy_status[i] & 0x55555550;
			joy_status[i] = (joy_status[i] & 0x0f) | (b0 >> 1) | (b1 << 1);
		}
	}

#if defined(MAX_BUTTONS)
	if(!press_flag && !release_flag) {
		int ii;
		ii = 0;
		for(ii = 0; buttons[ii].code != 0x00; ii++) { 
			if((mouse_ptrx >= buttons[ii].x) && (mouse_ptrx < (buttons[ii].x + buttons[ii].width))) {
				if((mouse_ptry >= buttons[ii].y) && (mouse_ptry < (buttons[ii].y + buttons[ii].height))) {
					if((key_status[buttons[ii].code] & 0x7f) == 0) this->press_button(ii);
				}
			}
		}
		if((mouse_ptrx >= buttons[ii].x) && (mouse_ptrx < (buttons[ii].x + buttons[ii].width))) {
			if((mouse_ptry >= buttons[ii].y) && (mouse_ptry < (buttons[ii].y + buttons[ii].height))) {
				this->press_button(ii);
			}
		}
		mouse_ptrx = mouse_ptry = 0;
	}
	//return;
#endif			
		
	// update mouse status
	if(mouse_enabled) {
		bool hid = false;
		memset(mouse_status, 0, sizeof(mouse_status));
		// get current status
		// move mouse cursor to the center of window
		//if(mouse_ptrx < 0) mouse_ptrx = 0;
		//if(mouse_ptrx >= SCREEN_WIDTH) mouse_ptrx = SCREEN_WIDTH - 1;
		//if(mouse_ptry < 0) mouse_ptry = 0;
		//if(mouse_ptry >= SCREEN_HEIGHT) mouse_ptry = SCREEN_HEIGHT - 1;
		
		mouse_status[0] = mouse_ptrx - mouse_oldx;
		mouse_status[1] = mouse_ptry - mouse_oldy;
		mouse_status[2] = mouse_button;
		mouse_oldx = mouse_ptrx;
		mouse_oldy = mouse_ptry;
	}
	
#ifdef USE_AUTO_KEY
	// auto key
	switch(autokey_phase) {
	case 1:
		if(autokey_buffer && !autokey_buffer->empty()) {
			// update shift key status
			int shift = autokey_buffer->read_not_remove(0) & 0x100;
			if(shift && !autokey_shift) {
				key_down(VK_SHIFT, false);
			} else if(!shift && autokey_shift) {
				key_up(VK_SHIFT);
			}
			autokey_shift = shift;
			autokey_phase++;
			break;
		}
	case 3:
		if(autokey_buffer && !autokey_buffer->empty()) {
			key_down(autokey_buffer->read_not_remove(0) & 0xff, false);
		}
		autokey_phase++;
		break;
	case USE_AUTO_KEY:
		if(autokey_buffer && !autokey_buffer->empty()) {
			key_up(autokey_buffer->read_not_remove(0) & 0xff);
		}
		autokey_phase++;
		break;
	case USE_AUTO_KEY_RELEASE:
		if(autokey_buffer && !autokey_buffer->empty()) {
			// wait enough while vm analyzes one line
			if(autokey_buffer->read() == 0xd) {
				autokey_phase++;
				break;
			}
		}
	case 30:
		if(autokey_buffer && !autokey_buffer->empty()) {
			autokey_phase = 1;
		} else {
			stop_auto_key();
		}
		break;
	default:
		if(autokey_phase) {
			autokey_phase++;
		}
	}
#endif
}

void OSD::key_down(int code, bool repeat)
{
	bool keep_frames = false;
	if(code == VK_SHIFT){
#ifndef USE_SHIFT_NUMPAD_KEY
		if(GetAsyncKeyState(VK_SHIFT) & 0x8000) {
			 key_status[VK_LSHIFT] = 0x80;
			 key_status[VK_RSHIFT] = 0x80;
			 key_status[VK_SHIFT] = 0x80;
		}
#endif
	} else if(code == VK_LSHIFT){
#ifndef USE_SHIFT_NUMPAD_KEY
		if(GetAsyncKeyState(VK_LSHIFT) & 0x8000) key_status[VK_LSHIFT] = 0x80;
#endif
	} else if(code == VK_RSHIFT){
#ifndef USE_SHIFT_NUMPAD_KEY
		if(GetAsyncKeyState(VK_RSHIFT) & 0x8000) key_status[VK_RSHIFT] = 0x80;
#endif
	} else if(code == VK_CONTROL) {
		if(GetAsyncKeyState(VK_CONTROL) & 0x8000) {
			key_status[VK_LCONTROL] = 0x80;
			key_status[VK_RCONTROL] = 0x80;
			key_status[VK_CONTROL] = 0x80;
		}
	} else if(code == VK_LCONTROL) {
		if(GetAsyncKeyState(VK_LCONTROL) & 0x8000) key_status[VK_LCONTROL] = 0x80;
	} else if(code == VK_RCONTROL) {
		if(GetAsyncKeyState(VK_RCONTROL) & 0x8000) key_status[VK_RCONTROL] = 0x80;
	} else if(code == VK_MENU) {
		if(GetAsyncKeyState(VK_MENU) & 0x8000) {
			key_status[VK_LMENU] = 0x80;
			key_status[VK_RMENU] = 0x80;
			key_status[VK_MENU] = 0x80;
		}
	} else if(code == VK_LMENU) {
		if(GetAsyncKeyState(VK_LMENU) & 0x8000) key_status[VK_LMENU] = 0x80;
	} else if(code == VK_RMENU) {
		if(GetAsyncKeyState(VK_RMENU) & 0x8000) key_status[VK_RMENU] = 0x80;
	} else if(code == 0xf0) {
		code = VK_CAPITAL;
		keep_frames = true;
	} else if(code == 0xf2) {
		code = VK_KANA;
		keep_frames = true;
	} else if(code == 0xf3 || code == 0xf4) {
		code = VK_KANJI;
		keep_frames = true;
	}

# ifdef USE_SHIFT_NUMPAD_KEY
	if(code == VK_SHIFT) {
		key_shift_pressed = true;
		key_shift_released = false;
		return;
	} else if(numpad_table[code] != 0) {
		if(key_shift_pressed || key_shift_released) {
			key_converted[code] = 1;
			key_shift_pressed = true;
			key_shift_released = false;
			code = numpad_table[code];
		}
	}
#endif

	if(!(code == VK_CONTROL || code == VK_MENU || code == VK_SHIFT || code == VK_LSHIFT || code == VK_RSHIFT)) {
		code = keycode_conv[code];
	}
	
#ifdef DONT_KEEEP_KEY_PRESSED
	if(!(code == VK_CONTROL || code == VK_MENU || code == VK_SHIFT || code == VK_LSHIFT || code == VK_RSHIFT)) {
		key_status[code] = KEY_KEEP_FRAMES;
	} else
#endif
     
	key_status[code] = keep_frames ? KEY_KEEP_FRAMES : 0x80;
#ifdef NOTIFY_KEY_DOWN
	if(keep_frames) {
		repeat = false;
	}
	vm->key_down(code, repeat);
#endif
}

void OSD::key_up(int code)
{
	if(code == VK_SHIFT) {
#ifndef USE_SHIFT_NUMPAD_KEY
		if(!(GetAsyncKeyState(VK_SHIFT) & 0x8000)) {
			key_status[VK_LSHIFT] &= 0x7f;
			key_status[VK_RSHIFT] &= 0x7f;
			key_status[VK_SHIFT] &= 0x7f;
		}
#endif
	} else if(code == VK_LSHIFT) {
#ifndef USE_SHIFT_NUMPAD_KEY
		if(!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
#endif
	} else if(code == VK_RSHIFT) {
#ifndef USE_SHIFT_NUMPAD_KEY
		if(!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
#endif
	} else if(code == VK_CONTROL) {
		if(!(GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
			key_status[VK_LCONTROL] &= 0x7f;
			key_status[VK_RCONTROL] &= 0x7f;
			key_status[VK_CONTROL] &= 0x7f;
		}
	} else if(code == VK_LCONTROL) {
		if(!(GetAsyncKeyState(VK_LCONTROL) & 0x8000)) key_status[VK_LCONTROL] &= 0x7f;
	} else if(code == VK_RCONTROL) {
		if(!(GetAsyncKeyState(VK_RCONTROL) & 0x8000)) key_status[VK_RCONTROL] &= 0x7f;
	} else if(code == VK_MENU) {
		if(!(GetAsyncKeyState(VK_MENU) & 0x8000)) {
			key_status[VK_LMENU] &= 0x7f;
			key_status[VK_RMENU] &= 0x7f;
			key_status[VK_MENU] &= 0x7f;
		}
	} else if(code == VK_LMENU) {
		if(!(GetAsyncKeyState(VK_LMENU) & 0x8000)) key_status[VK_LMENU] &= 0x7f;
	} else if(code == VK_RMENU) {
		if(!(GetAsyncKeyState(VK_RMENU) & 0x8000)) key_status[VK_RMENU] &= 0x7f;
	}

#ifdef USE_SHIFT_NUMPAD_KEY
	if((code == VK_SHIFT) || (code == VK_RSHIFT) || (code == VK_LSHIFT)) {
		key_shift_pressed = false;
		key_shift_released = true;
		return;
	} else if(key_converted[code] != 0) {
		key_converted[code] = 0;
		code = numpad_table[code];
	}
   
#endif
	if(!(code == VK_CONTROL || code == VK_MENU || code == VK_SHIFT || code == VK_LSHIFT || code == VK_RSHIFT)) {
		code = keycode_conv[code];
	}
	key_status[code] &= 0x7f;
#ifdef NOTIFY_KEY_DOWN
	vm->key_up(code);
#endif
}

void OSD::key_down_sub(int code, bool repeat)
{
	bool keep_frames = false;
	
	if(code == 0xf0) {
		code = VK_CAPITAL;
		keep_frames = true;
	} else if(code == 0xf2) {
		code = VK_KANA;
		keep_frames = true;
	} else if(code == 0xf3 || code == 0xf4) {
		code = VK_KANJI;
		keep_frames = true;
	}
	if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU)) {
		code = keycode_conv[code];
	}
	
#ifdef DONT_KEEEP_KEY_PRESSED
	if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU)) {
		key_status[code] = KEY_KEEP_FRAMES;
	} else
#endif
	key_status[code] = keep_frames ? KEY_KEEP_FRAMES : 0x80;
	
#ifdef NOTIFY_KEY_DOWN
#ifndef NOTIFY_KEY_DOWN_LR_SHIFT
	uint8 prev_shift = key_status[VK_SHIFT];
#endif
#ifndef NOTIFY_KEY_DOWN_LR_CONTROL
	uint8 prev_control = key_status[VK_CONTROL];
#endif
#ifndef NOTIFY_KEY_DOWN_LR_MENU
	uint8 prev_menu = key_status[VK_MENU];
#endif
#endif
	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];
	
#ifdef NOTIFY_KEY_DOWN
	if(keep_frames) {
		repeat = false;
	}
#ifndef NOTIFY_KEY_DOWN_LR_SHIFT
	if(code == VK_LSHIFT || code == VK_RSHIFT) {
		if(prev_shift == 0 && key_status[VK_SHIFT] != 0) {
			vm->key_down(VK_SHIFT, repeat);
		}
		return;
	}
#endif
#ifndef NOTIFY_KEY_DOWN_LR_CONTROL
	if(code == VK_LCONTROL|| code == VK_RCONTROL) {
		if(prev_control == 0 && key_status[VK_CONTROL] != 0) {
			vm->key_down(VK_CONTROL, repeat);
		}
		return;
	}
#endif
#ifndef NOTIFY_KEY_DOWN_LR_MENU
	if(code == VK_LMENU|| code == VK_RMENU) {
		if(prev_menu == 0 && key_status[VK_MENU] != 0) {
			vm->key_down(VK_MENU, repeat);
		}
		return;
	}
#endif
	vm->key_down(code, repeat);
#endif
}

void OSD::key_up_sub(int code)
{
	if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU)) {
		code = keycode_conv[code];
	}
	if(key_status[code] == 0) {
		return;
	}
	if((key_status[code] &= 0x7f) != 0) {
		return;
	}
	
#ifdef NOTIFY_KEY_DOWN
#ifndef NOTIFY_KEY_DOWN_LR_SHIFT
	uint8 prev_shift = key_status[VK_SHIFT];
#endif
#ifndef NOTIFY_KEY_DOWN_LR_CONTROL
	uint8 prev_control = key_status[VK_CONTROL];
#endif
#ifndef NOTIFY_KEY_DOWN_LR_MENU
	uint8 prev_menu = key_status[VK_MENU];
#endif
#endif
	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];
	
#ifdef NOTIFY_KEY_DOWN
#ifndef NOTIFY_KEY_DOWN_LR_SHIFT
	if(code == VK_LSHIFT || code == VK_RSHIFT) {
		if(prev_shift != 0 && key_status[VK_SHIFT] == 0) {
			vm->key_up(VK_SHIFT);
		}
		return;
	}
#endif
#ifndef NOTIFY_KEY_DOWN_LR_CONTROL
	if(code == VK_LCONTROL|| code == VK_RCONTROL) {
		if(prev_control != 0 && key_status[VK_CONTROL] == 0) {
			vm->key_up(VK_CONTROL);
		}
		return;
	}
#endif
#ifndef NOTIFY_KEY_DOWN_LR_MENU
	if(code == VK_LMENU || code == VK_RMENU) {
		if(prev_menu != 0 && key_status[VK_MENU] == 0) {
			vm->key_up(VK_MENU);
		}
		return;
	}
#endif
	vm->key_up(code);
#endif
}

#ifdef ONE_BOARD_MICRO_COMPUTER
void OSD::press_button(int num)
{
	int code = buttons[num].code;
	
	if(code) {
		key_down_sub(code, false);
		key_status[code] = KEY_KEEP_FRAMES;
	} else {
		// code=0: reset virtual machine
		vm->reset();
	}
}
#endif

void OSD::enable_mouse()
{
	// enable mouse emulation
	if(!mouse_enabled) {
		QCursor cursor;
		QPoint pos;
		mouse_oldx = mouse_ptrx = SCREEN_WIDTH / 2;
		mouse_oldy = mouse_ptry = SCREEN_HEIGHT / 2;
		cursor = glv->cursor();
		pos.setX(glv->width() / 2);
		pos.setY(glv->height() / 2);
		cursor.setPos(glv->mapToGlobal(pos));
		QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
		//mouse_shape = cursor.shape();
		//cursor.setShape(Qt::BlankCursor);
		mouse_status[0] = 0;
		mouse_status[1] = 0;
		mouse_status[2] = mouse_button;
	}
	glv->setMouseTracking(true);
	mouse_enabled = true;
}

void OSD::disenable_mouse()
{
	// disenable mouse emulation
	if(mouse_enabled) {
		QCursor cursor;
		cursor = glv->cursor();
		if(QApplication::overrideCursor() != NULL) QApplication::restoreOverrideCursor();
		//QApplication::restoreOverrideCursor();
		glv->setMouseTracking(false);
	}
	mouse_enabled = false;
}

void OSD::toggle_mouse()
{
	// toggle mouse enable / disenable
	if(mouse_enabled) {
		disenable_mouse();
	} else {
		enable_mouse();
	}
}

#ifdef USE_AUTO_KEY

static const int autokey_table[256] = {
	// 0x100: shift
	// 0x200: kana
	// 0x400: alphabet
	// 0x800: ALPHABET
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x00d,0x000,0x000,0x00d,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x020,0x131,0x132,0x133,0x134,0x135,0x136,0x137,0x138,0x139,0x1ba,0x1bb,0x0bc,0x0bd,0x0be,0x0bf,
	0x030,0x031,0x032,0x033,0x034,0x035,0x036,0x037,0x038,0x039,0x0ba,0x0bb,0x1bc,0x1bd,0x1be,0x1bf,
	0x0c0,0x441,0x442,0x443,0x444,0x445,0x446,0x447,0x448,0x449,0x44a,0x44b,0x44c,0x44d,0x44e,0x44f,
	0x450,0x451,0x452,0x453,0x454,0x455,0x456,0x457,0x458,0x459,0x45a,0x0db,0x0dc,0x0dd,0x0de,0x1e2,
	0x1c0,0x841,0x842,0x843,0x844,0x845,0x846,0x847,0x848,0x849,0x84a,0x84b,0x84c,0x84d,0x84e,0x84f,
	0x850,0x851,0x852,0x853,0x854,0x855,0x856,0x857,0x858,0x859,0x85a,0x1db,0x1dc,0x1dd,0x1de,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	// kana -->
	0x000,0x3be,0x3db,0x3dd,0x3bc,0x3bf,0x330,0x333,0x345,0x334,0x335,0x336,0x337,0x338,0x339,0x35a,
	0x2dc,0x233,0x245,0x234,0x235,0x236,0x254,0x247,0x248,0x2ba,0x242,0x258,0x244,0x252,0x250,0x243,
	0x251,0x241,0x25a,0x257,0x253,0x255,0x249,0x231,0x2bc,0x24b,0x246,0x256,0x232,0x2de,0x2bd,0x24a,
	0x24e,0x2dd,0x2bf,0x24d,0x237,0x238,0x239,0x24f,0x24c,0x2be,0x2bb,0x2e2,0x230,0x259,0x2c0,0x2db,
	// <--- kana
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
	0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000
};

void OSD::set_auto_key_string(QByteArray cstr)
{
	stop_auto_key();
	memset(auto_key_str, 0x00, sizeof(auto_key_str));
	strncpy(auto_key_str, (const char *)(cstr.constData()), sizeof(auto_key_str) - 2);
}

void OSD::start_auto_key()
{
	{
		{
			autokey_buffer->clear();
			char* buf = (char*)auto_key_str;
			int size = strlen(buf), prev_kana = 0;
			AGAR_DebugLog(AGAR_LOG_DEBUG, "AutoKey: SET :%s\n", buf);
			for(int i = 0; i < size; i++) {
				int code = buf[i] & 0xff;
				if((0x81 <= code && code <= 0x9f) || 0xe0 <= code) {
					i++;	// kanji ?
					continue;
				} else if(code == 0xa) {
					continue;	// cr-lf
				}

				if((code = autokey_table[code]) != 0) {
					int kana = code & 0x200;
					if(prev_kana != kana) {
						autokey_buffer->write(0xf2);
					}
					prev_kana = kana;
#if defined(USE_AUTO_KEY_NO_CAPS)
					if((code & 0x100) && !(code & (0x400 | 0x800))) {
#elif defined(USE_AUTO_KEY_CAPS)
					if(code & (0x100 | 0x800)) {
#else
					if(code & (0x100 | 0x400)) {
#endif
						autokey_buffer->write((code & 0xff) | 0x100);
					} else {
						autokey_buffer->write(code & 0xff);
					}
				}
			}
			if(prev_kana) {
				autokey_buffer->write(0xf2);
			}
			autokey_phase = 1;
			autokey_shift = 0;
		}
	}
}

void OSD::stop_auto_key()
{
	if(autokey_shift) {
		key_up_sub(VK_LSHIFT);
	}
	autokey_phase = autokey_shift = 0;
}
#endif

#if !defined(Q_OS_WIN) && !defined(Q_OS_CYGWIN)
uint16_t OSD::GetAsyncKeyState(uint32_t vk)
{
	vk = vk & 0xff; // OK?
	quint32 modstate = modkey_status;
   //printf("Mod %d %08x\n", vk, mod);
	switch(vk) {
	case VK_SHIFT:
		if((modstate & Qt::ShiftModifier) != 0) return 0xffff;
		break;
	case VK_LSHIFT:
		if((modstate & Qt::ShiftModifier) != 0) return 0xffff;
		break;
	case VK_RSHIFT:
		if((modstate & Qt::ShiftModifier) != 0) return 0xffff;
		break;
	case VK_CONTROL:
		if((modstate & Qt::ControlModifier) != 0) return 0xffff;
		break;
	case VK_LCONTROL:
		if((modstate & Qt::ControlModifier) != 0) return 0xffff;
		break;
	case VK_RCONTROL:
		if((modstate & Qt::ControlModifier) != 0) return 0xffff;
		break;
	case VK_LMENU:
		if((modstate & Qt::AltModifier) != 0) return 0xffff;
		break;
	case VK_RMENU:
		if((modstate & Qt::AltModifier) != 0) return 0xffff;
		break;
	default:
		break;
	}
	return 0;
}
#endif
