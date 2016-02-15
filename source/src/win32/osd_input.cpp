/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 input ]
*/

#include "osd.h"
#include "../fifo.h"

#ifdef NOTIFY_KEY_DOWN_LR_SHIFT
#define VK_SHIFT_TEMP	VK_LSHIFT
#else
#define VK_SHIFT_TEMP	VK_SHIFT
#endif
#define KEY_KEEP_FRAMES	3

static const uint8 vk_dik[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xC5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x79, 0x7b, 0x00, 0x00,
	0x39, 0xc9, 0xd1, 0xcf, 0xc7, 0xcb, 0xc8, 0xcd, 0xd0, 0x00, 0x00, 0x00, 0x00, 0xd2, 0xd3, 0x00,
	0x0b, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	0x19, 0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11, 0x2d, 0x15, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00,
#ifdef USE_NUMPAD_ENTER
	0x52, 0x4f, 0x50, 0x51, 0x4b, 0x4c, 0x4d, 0x47, 0x48, 0x49, 0x37, 0x4e, 0x9c, 0x4a, 0x53, 0xb5,
#else
	0x52, 0x4f, 0x50, 0x51, 0x4b, 0x4c, 0x4d, 0x47, 0x48, 0x49, 0x37, 0x4e, 0x00, 0x4a, 0x53, 0xb5,
#endif
	0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x57, 0x58, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	DIK_LSHIFT, DIK_RSHIFT, DIK_LCONTROL, DIK_RCONTROL, DIK_LMENU, DIK_RMENU,
	                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x92, 0x27, 0x33, 0x0c, 0x34, 0x35,
	0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x7d, 0x1b, 0x90, 0x00,
	0x00, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x3a, 0x00, 0x70, 0x00, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// caps, kana, kanji
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

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
	
	// initialize direct input
	dinput_key_ok = false;
	lpdi = NULL;
	lpdikey = NULL;
	
	// XXX: no gui to change config.use_direct_inpu, so we need to modify *.ini file manually
	if(config.use_direct_input) {
		if(SUCCEEDED(DirectInputCreate(instance_handle, DIRECTINPUT_VERSION, &lpdi, NULL))) {
			if(SUCCEEDED(lpdi->CreateDevice(GUID_SysKeyboard, &lpdikey, NULL))) {
				if(SUCCEEDED(lpdikey->SetDataFormat(&c_dfDIKeyboard))) {
					if(SUCCEEDED(lpdikey->SetCooperativeLevel(main_window_handle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
						dinput_key_ok = true;
						memset(key_dik_prev, 0, sizeof(key_dik_prev));
					}
				}
			}
		}
	}
	
	// initialize joysticks
	joy_num = joyGetNumDevs();
	for(int i = 0; i < joy_num && i < 4; i++) {
		JOYCAPS joycaps;
		if(joyGetDevCaps(i, &joycaps, sizeof(joycaps)) == JOYERR_NOERROR) {
			joy_mask[i] = (1 << joycaps.wNumButtons) - 1;
		} else {
			joy_mask[i] = 0x0f; // 4buttons
		}
	}
	
	// mouse emulation is disenabled
	mouse_enabled = false;
	
	// initialize keycode convert table
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("keycode.cfg")), FILEIO_READ_BINARY)) {
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
	memset(autokey_table, 0, sizeof(autokey_table));
	for(int i = 0;; i++) {
		if(autokey_table_base[i][0] == -1) {
			break;
		}
		autokey_table[autokey_table_base[i][0]] = autokey_table_base[i][1];
	}
#ifdef USE_VM_AUTO_KEY_TABLE
	for(int i = 0;; i++) {
		if(vm_autokey_table_base[i][0] == -1) {
			break;
		}
		autokey_table[vm_autokey_table_base[i][0]] = vm_autokey_table_base[i][1];
	}
#endif
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
	
	// release direct input
	if(lpdi) {
		if(lpdikey) {
			lpdikey->Unacquire();
			lpdikey->Release();
			lpdikey = NULL;
		}
		lpdi->Release();
		lpdi = NULL;
	}
}

void OSD::update_input()
{
	if(dinput_key_ok) {
		// direct input
		static uint8 key_dik[256];
		lpdikey->Acquire();
		lpdikey->GetDeviceState(256, key_dik);
		
		// DIK_RSHIFT is not detected on Vista or later
		if(vista_or_later) {
			key_dik[DIK_RSHIFT] = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) ? 0x80 : 0;
		}
#ifdef USE_SHIFT_NUMPAD_KEY
		// XXX: don't release shift key while numpad key is pressed
		uint8 numpad_keys;
		numpad_keys  = key_dik[DIK_NUMPAD0];
		numpad_keys |= key_dik[DIK_NUMPAD1];
		numpad_keys |= key_dik[DIK_NUMPAD2];
		numpad_keys |= key_dik[DIK_NUMPAD3];
		numpad_keys |= key_dik[DIK_NUMPAD4];
		numpad_keys |= key_dik[DIK_NUMPAD5];
		numpad_keys |= key_dik[DIK_NUMPAD6];
		numpad_keys |= key_dik[DIK_NUMPAD7];
		numpad_keys |= key_dik[DIK_NUMPAD8];
		numpad_keys |= key_dik[DIK_NUMPAD9];
		if(numpad_keys & 0x80) {
			key_dik[DIK_LSHIFT] |= key_dik_prev[DIK_LSHIFT];
			key_dik[DIK_RSHIFT] |= key_dik_prev[DIK_RSHIFT];
		}
#endif
		key_dik[DIK_CIRCUMFLEX] |= key_dik[DIK_EQUALS     ];
		key_dik[DIK_COLON     ] |= key_dik[DIK_APOSTROPHE ];
		key_dik[DIK_YEN       ] |= key_dik[DIK_GRAVE      ];
#ifndef USE_NUMPAD_ENTER
		key_dik[DIK_RETURN    ] |= key_dik[DIK_NUMPADENTER];
#endif
		
		for(int vk = 0; vk < 256; vk++) {
			int dik = vk_dik[vk];
			if(dik) {
				if(key_dik[dik] & 0x80) {
					if(!(key_dik_prev[dik] & 0x80)) {
						key_down_sub(vk, false);
					}
				} else {
					if(key_dik_prev[dik] & 0x80) {
						key_up_sub(vk);
					}
				}
			}
		}
		memcpy(key_dik_prev, key_dik, sizeof(key_dik_prev));
#ifdef USE_SHIFT_NUMPAD_KEY
	} else {
		// update numpad key status
		if(key_shift_pressed && !key_shift_released) {
			if(key_status[VK_LSHIFT] == 0) {
				// shift key is newly pressed
				key_status[VK_LSHIFT] = key_status[VK_SHIFT] = 0x80;
#ifdef NOTIFY_KEY_DOWN
				vm->key_down(VK_SHIFT_TEMP, false);
#endif
			}
		} else if(!key_shift_pressed && key_shift_released) {
			if(key_status[VK_LSHIFT] != 0) {
				// shift key is newly released
				key_status[VK_LSHIFT] = key_status[VK_SHIFT] = 0;
#ifdef NOTIFY_KEY_DOWN
				vm->key_up(VK_SHIFT_TEMP);
#endif
			}
		}
		key_shift_pressed = key_shift_released = false;
#endif
	}
	
	// release keys
#ifdef USE_AUTO_KEY
	if(lost_focus && autokey_phase == 0) {
#else
	if(lost_focus) {
#endif
		// we lost key focus so release all pressed keys
#ifdef NOTIFY_KEY_DOWN
#ifdef NOTIFY_KEY_DOWN_LR_SHIFT
		key_status[VK_SHIFT] = 0;
#else
		key_status[VK_LSHIFT] = key_status[VK_RSHIFT] = 0;
#endif
#ifdef NOTIFY_KEY_DOWN_LR_CONTROL
		key_status[VK_CONTROL] = 0;
#else
		key_status[VK_LCONTROL] = key_status[VK_RCONTROL] = 0;
#endif
#ifdef NOTIFY_KEY_DOWN_LR_MENU
		key_status[VK_MENU] = 0;
#else
		key_status[VK_LMENU] = key_status[VK_RMENU] = 0;
#endif
#endif
		for(int i = 0; i < 256; i++) {
			if(key_status[i] & 0x80) {
				key_status[i] &= 0x7f;
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
#ifdef NOTIFY_KEY_DOWN
				if(!key_status[i]) {
					vm->key_up(i);
				}
#endif
			}
		}
	}
	lost_focus = false;
	
	// VK_$00 should be 0
	key_status[0] = 0;
	
	// update joystick status
	memset(joy_status, 0, sizeof(joy_status));
	uint32 tmp_status[4];
	memset(tmp_status, 0, sizeof(tmp_status));
	
	for(int i = 0; i < joy_num && i < 4; i++) {
		JOYINFOEX joyinfo;
		joyinfo.dwSize = sizeof(JOYINFOEX);
		joyinfo.dwFlags = JOY_RETURNALL;
		tmp_status[i] = 0;
		if(joyGetPosEx(i, &joyinfo) == JOYERR_NOERROR) {
			if(joyinfo.dwYpos < 0x3fff) tmp_status[i] |= 0x01;	// up
			if(joyinfo.dwYpos > 0xbfff) tmp_status[i] |= 0x02;	// down
			if(joyinfo.dwXpos < 0x3fff) tmp_status[i] |= 0x04;	// left
			if(joyinfo.dwXpos > 0xbfff) tmp_status[i] |= 0x08;	// right
			tmp_status[i] |= ((joyinfo.dwButtons & joy_mask[i]) << 4);
		}
	}
	for(int i = 0; i < joy_num && i < 4; i++) {
		for(int j = 0; j < 16; j++) {
			if(config.joy_buttons[i][j] < 0) {
				int code = -config.joy_buttons[i][j];
				if(code < 256 && key_status[code]) {
					joy_status[i] |= (1 << j);
				}
			} else {
				int stick = config.joy_buttons[i][j] >> 4;
				int button = config.joy_buttons[i][j] & 15;
				if(stick < 2 && (tmp_status[stick] & (1 << button))) {
					joy_status[i] |= (1 << j);
				}
			}
		}
	}
	
	// update mouse status
	memset(mouse_status, 0, sizeof(mouse_status));
	if(mouse_enabled) {
		// get current status
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(main_window_handle, &pt);
		mouse_status[0]  = pt.x - host_window_width / 2;
		mouse_status[1]  = pt.y - host_window_height / 2;
		mouse_status[2]  = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 2 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 4 : 0;
		// move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
			pt.x = host_window_width / 2;
			pt.y = host_window_height / 2;
			ClientToScreen(main_window_handle, &pt);
			SetCursorPos(pt.x, pt.y);
		}
	}
	
#ifdef USE_AUTO_KEY
#ifndef USE_AUTO_KEY_SHIFT
#define USE_AUTO_KEY_SHIFT 0
#endif
	// auto key
	switch(autokey_phase) {
	case 1:
		if(autokey_buffer && !autokey_buffer->empty()) {
			// update shift key status
			int shift = autokey_buffer->read_not_remove(0) & 0x100;
			if(shift && !autokey_shift) {
				key_down_sub(VK_LSHIFT, false);
			} else if(!shift && autokey_shift) {
				key_up_sub(VK_LSHIFT);
			}
			autokey_shift = shift;
			autokey_phase++;
			break;
		}
	case 3 + USE_AUTO_KEY_SHIFT:
		if(autokey_buffer && !autokey_buffer->empty()) {
			key_down_sub(autokey_buffer->read_not_remove(0) & 0xff, false);
		}
		autokey_phase++;
		break;
	case USE_AUTO_KEY + USE_AUTO_KEY_SHIFT:
		if(autokey_buffer && !autokey_buffer->empty()) {
			key_up_sub(autokey_buffer->read_not_remove(0) & 0xff);
		}
		autokey_phase++;
		break;
	case USE_AUTO_KEY_RELEASE + USE_AUTO_KEY_SHIFT:
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
	if(!dinput_key_ok) {
		if(code == VK_SHIFT) {
			if(!(key_status[VK_LSHIFT] & 0x80) && (GetAsyncKeyState(VK_LSHIFT) & 0x8000)) {
				code = VK_LSHIFT;
			} else if(!(key_status[VK_RSHIFT] & 0x80) && (GetAsyncKeyState(VK_RSHIFT) & 0x8000)) {
				code = VK_RSHIFT;
			} else {
				return;
			}
		} else if(code == VK_CONTROL) {
			if(!(key_status[VK_LCONTROL] & 0x80) && (GetAsyncKeyState(VK_LCONTROL) & 0x8000)) {
				code = VK_LCONTROL;
			} else if(!(key_status[VK_RCONTROL] & 0x80) && (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) {
				code = VK_RCONTROL;
			} else {
				return;
			}
		} else if(code == VK_MENU) {
			if(!(key_status[VK_LMENU] & 0x80) && (GetAsyncKeyState(VK_LMENU) & 0x8000)) {
				code = VK_LMENU;
			} else if(!(key_status[VK_RMENU] & 0x80) && (GetAsyncKeyState(VK_RMENU) & 0x8000)) {
				code = VK_RMENU;
			} else {
				return;
			}
		}
#ifdef USE_SHIFT_NUMPAD_KEY
		if(code == VK_LSHIFT) {
			key_shift_pressed = true;
			return;
		} else if(numpad_table[code] != 0) {
			if(key_shift_pressed || key_shift_released) {
				key_converted[code] = 1;
				key_shift_pressed = true;
				code = numpad_table[code];
			}
		}
#endif
		key_down_sub(code, repeat);
	} else {
		// caps, kana, kanji
		if(repeat || code == 0xf0 || code == 0xf2 || code == 0xf3 || code == 0xf4) {
			key_down_sub(code, repeat);
		}
	}
}

void OSD::key_up(int code)
{
	if(!dinput_key_ok) {
		if(code == VK_SHIFT) {
			if((key_status[VK_LSHIFT] & 0x80) && !(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) {
				code = VK_LSHIFT;
			} else if((key_status[VK_RSHIFT] & 0x80) && !(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) {
				code = VK_RSHIFT;
			} else {
				return;
			}
		} else if(code == VK_CONTROL) {
			if((key_status[VK_LCONTROL] & 0x80) && !(GetAsyncKeyState(VK_LCONTROL) & 0x8000)) {
				code = VK_LCONTROL;
			} else if((key_status[VK_RCONTROL] & 0x80) && !(GetAsyncKeyState(VK_RCONTROL) & 0x8000)) {
				code = VK_RCONTROL;
			} else {
				return;
			}
		} else if(code == VK_MENU) {
			if((key_status[VK_LMENU] & 0x80) && !(GetAsyncKeyState(VK_LMENU) & 0x8000)) {
				code = VK_LMENU;
			} else if((key_status[VK_RMENU] & 0x80) && !(GetAsyncKeyState(VK_RMENU) & 0x8000)) {
				code = VK_RMENU;
			} else {
				return;
			}
		}
#ifdef USE_SHIFT_NUMPAD_KEY
		if(code == VK_LSHIFT) {
			key_shift_pressed = false;
			key_shift_released = true;
			return;
		} else if(key_converted[code] != 0) {
			key_converted[code] = 0;
			code = numpad_table[code];
		}
#endif
		key_up_sub(code);
	}
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
		// hide mouse cursor
		ShowCursor(FALSE);
		// move mouse cursor to the center of window
		POINT pt;
		pt.x = host_window_width / 2;
		pt.y = host_window_height / 2;
		ClientToScreen(main_window_handle, &pt);
		SetCursorPos(pt.x, pt.y);
	}
	mouse_enabled = true;
}

void OSD::disenable_mouse()
{
	// disenable mouse emulation
	if(mouse_enabled) {
		ShowCursor(TRUE);
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
void OSD::start_auto_key()
{
	stop_auto_key();
	
	if(OpenClipboard(NULL)) {
		HANDLE hClip = GetClipboardData(CF_TEXT);
		if(hClip) {
			autokey_buffer->clear();
			char* buf = (char*)GlobalLock(hClip);
			int size = strlen(buf), prev_kana = 0;
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
			GlobalUnlock(hClip);
			
			autokey_phase = 1;
			autokey_shift = 0;
		}
		CloseClipboard();
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
