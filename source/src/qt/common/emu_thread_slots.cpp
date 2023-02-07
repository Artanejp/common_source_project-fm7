/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
    Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.10
	History: 2016.8.26 Split from emu_thread.cpp
	Note: This class must be compiled per VM, must not integrate shared units.
	[ win32 main ] -> [ Qt main ] -> [Emu Thread] -> [QObject SLOTs depended by VM]
*/

#include <QString>
#include <QTextCodec>
#include <QWaitCondition>

#include <SDL.h>

#include "../gui/emu_thread_tmpl.h"

#include "qt_gldraw.h"
#include "csp_logger.h"
#include "../gui/menu_flags.h"
#include "dock_disks.h"
// buttons
#ifdef MAX_BUTTONS
#define MAX_FONT_SIZE 32
#endif
#define MAX_SKIP_FRAMES 10


const int auto_key_table_base[][2] = {
	// 0x100: shift
	// 0x200: kana
	// 0x400: alphabet
	// 0x800: ALPHABET
	// 0x1000 : LOCK
	// 0x2000 : UNLOCK
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

const int auto_key_table_50on_base[][2] = {
	{0xa1,	0x300 | 0xbf},	// '｡'
	{0xa2,	0x300 | 0xdb},	// '｢'
	{0xa3,	0x300 | 0xdd},	// '｣'
	{0xa4,	0x300 | 0xbe},	// '､'
	{0xa5,	0x300 | 0xe2},	// '･'
	{0xa6,	0x200 | 0xbf},	// 'ｦ'
	{0xa7,	0x300 | 0x31},	// 'ｧ'
	{0xa8,	0x300 | 0x32},	// 'ｨ'
	{0xa9,	0x300 | 0x33},	// 'ｩ'
	{0xaa,	0x300 | 0x34},	// 'ｪ'
	{0xab,	0x300 | 0x35},	// 'ｫ'
	{0xac,	0x300 | 0x4e},	// 'ｬ'
	{0xad,	0x300 | 0x4d},	// 'ｭ'
	{0xae,	0x300 | 0xbc},	// 'ｮ'
	{0xaf,	0x300 | 0x43},	// 'ｯ'
	{0xb0,	0x300 | 0xba},	// 'ｰ'
	{0xb1,	0x200 | 0x31},	// 'ｱ'
	{0xb2,	0x200 | 0x32},	// 'ｲ'
	{0xb3,	0x200 | 0x33},	// 'ｳ'
	{0xb4,	0x200 | 0x34},	// 'ｴ'
	{0xb5,	0x200 | 0x35},	// 'ｵ'
	{0xb6,	0x200 | 0x51},	// 'ｶ'
	{0xb7,	0x200 | 0x57},	// 'ｷ'
	{0xb8,	0x200 | 0x45},	// 'ｸ'
	{0xb9,	0x200 | 0x52},	// 'ｹ'
	{0xba,	0x200 | 0x54},	// 'ｺ'
	{0xbb,	0x200 | 0x41},	// 'ｻ'
	{0xbc,	0x200 | 0x53},	// 'ｼ'
	{0xbd,	0x200 | 0x44},	// 'ｽ'
	{0xbe,	0x200 | 0x46},	// 'ｾ'
	{0xbf,	0x200 | 0x47},	// 'ｿ'
	{0xc0,	0x200 | 0x5a},	// 'ﾀ'
	{0xc1,	0x200 | 0x58},	// 'ﾁ'
	{0xc2,	0x200 | 0x43},	// 'ﾂ'
	{0xc3,	0x200 | 0x56},	// 'ﾃ'
	{0xc4,	0x200 | 0x42},	// 'ﾄ'
	{0xc5,	0x200 | 0x36},	// 'ﾅ'
	{0xc6,	0x200 | 0x37},	// 'ﾆ'
	{0xc7,	0x200 | 0x38},	// 'ﾇ'
	{0xc8,	0x200 | 0x39},	// 'ﾈ'
	{0xc9,	0x200 | 0x30},	// 'ﾉ'
	{0xca,	0x200 | 0x59},	// 'ﾊ'
	{0xcb,	0x200 | 0x55},	// 'ﾋ'
	{0xcc,	0x200 | 0x49},	// 'ﾌ'
	{0xcd,	0x200 | 0x4f},	// 'ﾍ'
	{0xce,	0x200 | 0x50},	// 'ﾎ'
	{0xcf,	0x200 | 0x48},	// 'ﾏ'
	{0xd0,	0x200 | 0x4a},	// 'ﾐ'
	{0xd1,	0x200 | 0x4b},	// 'ﾑ'
	{0xd2,	0x200 | 0x4c},	// 'ﾒ'
	{0xd3,	0x200 | 0xbb},	// 'ﾓ'
	{0xd4,	0x200 | 0x4e},	// 'ﾔ'
	{0xd5,	0x200 | 0x4d},	// 'ﾕ'
	{0xd6,	0x200 | 0xbc},	// 'ﾖ'
	{0xd7,	0x200 | 0xbd},	// 'ﾗ'
	{0xd8,	0x200 | 0xde},	// 'ﾘ'
	{0xd9,	0x200 | 0xdc},	// 'ﾙ'
	{0xda,	0x200 | 0xc0},	// 'ﾚ'
	{0xdb,	0x200 | 0xdb},	// 'ﾛ'
	{0xdc,	0x200 | 0xbe},	// 'ﾜ'
	{0xdd,	0x200 | 0xe2},	// 'ﾝ'
	{0xde,	0x200 | 0xba},	// 'ﾞ'
	{0xdf,	0x200 | 0xdd},	// 'ﾟ'
	{-1, -1},
};

void EmuThreadClassBase::do_start_auto_key(QString ctext)
{
	//QMutexLocker _locker(&uiMutex);

	if(using_flags->is_use_auto_key()) {
		QTextCodec *codec = QTextCodec::codecForName("Shift-Jis");
		QByteArray array;
		QVector<uint> ucs4_src = ctext.toUcs4();
		QString dst;
		dst.clear();
		uint32_t pool[8] = {0};
		for(auto itr = ucs4_src.constBegin(); itr != ucs4_src.constEnd(); ++itr) {
			uint val = (*itr);
			int chrs = ucs4_kana_zenkaku_to_hankaku((const uint32_t)val, pool, sizeof(pool) / sizeof(uint32_t));
			if(chrs > 0) {
		#if QT_VERSION >= 0x060000
				dst.append(QString::fromUcs4((char32_t*)pool, chrs));
		#else
				dst.append(QString::fromUcs4((uint*)pool, chrs));
		#endif
			}
		}
		clipBoardText = dst;
		//printf("%s\n", clipBoardText.toLocal8Bit().constData());
		array = codec->fromUnicode(clipBoardText);
		//printf("Array is:");
		//for(int l = 0; l < array.size(); l++) {
		//	printf("%02X ", array.at(l));
		//}
		//printf("\n");
		if(clipBoardText.size() > 0) {
			int size = array.size();
			const char *buf = (char *)(array.constData());
			p_emu->stop_auto_key();
			p_emu->set_auto_key_list((char *)buf, size);
			p_emu->start_auto_key();
		}
	}

}

void EmuThreadClassBase::do_stop_auto_key(void)
{
	//QMutexLocker _locker(&uiMutex);
	//csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL,
	//					  "AutoKey: stop\n");
	if(using_flags->is_use_auto_key()) {
		p_emu->stop_auto_key();
	}
}

void EmuThreadClassBase::do_write_protect_floppy_disk(int drv, bool flag)
{
	//QMutexLocker _locker(&uiMutex);
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if((p->get_max_drive() > drv) && (p->is_use_fd())) {
		p_emu->is_floppy_disk_protected(drv, flag);
	}
}

void EmuThreadClassBase::do_close_floppy_disk(int drv)
{
	//QMutexLocker _locker(&uiMutex);
	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if((p->get_max_drive() > drv) && (p->is_use_fd())) {
		p_emu->close_floppy_disk(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_FD, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_floppy_disk(int drv, QString path, int bank)
{
	if(path.isEmpty()) return;
	if(path.isNull()) return;

	std::shared_ptr<USING_FLAGS> p = using_flags;
	if(p.get() == nullptr) return;
	if(!(using_flags->is_use_fd())) return;
	if(!((p->get_max_drive() > drv) && (p->is_use_fd()))) return;

	const _TCHAR *file_path = (const _TCHAR *)(path.toLocal8Bit().constData());
	if(!(FILEIO::IsFileExisting(file_path))) return; // File not found.

	p_emu->open_floppy_disk(drv, file_path, bank);
}


void EmuThreadClassBase::do_select_floppy_disk_d88(int drive, int slot)
{
	if(p_emu == nullptr) return;

	int bank_num = p_emu->d88_file[drive].bank_num;
	if(bank_num <= 0) return;

	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;
	if(p->get_max_d88_banks() <= slot) slot = p->get_max_d88_banks() - 1;
	if(slot < 0) return;
	if(bank_num <= slot) return;

	if((p_emu->is_floppy_disk_inserted(drive)) &&
	   (slot != p_emu->d88_file[drive].cur_bank)) {
		QString path = get_d88_file_path(drive);
		do_open_floppy_disk(drive, path, slot);
	}
}


void EmuThreadClassBase::do_play_tape(int drv, QString name)
{
	if(p_emu == nullptr) return;
	std::shared_ptr<USING_FLAGS>p = using_flags;
	if(p.get() == nullptr) return;

	if(p->is_use_tape()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->play_tape(drv, name.toLocal8Bit().constData());
	}
}

void EmuThreadClassBase::do_rec_tape(int drv, QString name)
{
	if(using_flags->is_use_tape()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->rec_tape(drv, name.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_CMT, drv, name);
	}
}

void EmuThreadClassBase::do_close_tape(int drv)
{
	if(using_flags->is_use_tape()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_tape(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_CMT, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_cmt_push_play(int drv)
{
	if(using_flags->is_use_tape()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->push_play(drv);
	}
}

void EmuThreadClassBase::do_cmt_push_stop(int drv)
{
	if(using_flags->is_use_tape()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->push_stop(drv);
	}
}

void EmuThreadClassBase::do_cmt_push_fast_forward(int drv)
{
	if(using_flags->is_use_tape()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->push_fast_forward(drv);
	}
}

void EmuThreadClassBase::do_cmt_push_fast_rewind(int drv)
{
	if(using_flags->is_use_tape()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->push_fast_rewind(drv);
	}
}

void EmuThreadClassBase::do_cmt_push_apss_forward(int drv)
{
	if(using_flags->is_use_tape()) {
		////QMutexLocker _locker(&uiMutex);
		p_emu->push_apss_forward(drv);
	}
}

void EmuThreadClassBase::do_cmt_push_apss_rewind(int drv)
{
	if(using_flags->is_use_tape()) {
		////QMutexLocker _locker(&uiMutex);
		p_emu->push_apss_rewind(drv);
	}
}

// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_tape(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_tape())) return;
	if((drive < 0) || (drive >= using_flags->get_max_tape())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_tape_path[drive], list);

	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_tape_dir, __dir, _MAX_PATH - 1);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	emit sig_ui_update_tape_list(drive, list);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_CMT, drive, relpath);
}
void EmuThreadClassBase::done_close_tape(int drive)
{
	emit sig_ui_close_tape(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_CMT, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::do_write_protect_quickdisk(int drv, bool flag)
{
	if(using_flags->is_use_qd()) {
		////QMutexLocker _locker(&uiMutex);
		//p_emu->write_protect_Qd(drv, flag);
	}
}

void EmuThreadClassBase::do_close_quickdisk(int drv)
{
	if(using_flags->is_use_qd()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_quick_disk(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_QD, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_quickdisk(int drv, QString path)
{
	if(using_flags->is_use_qd()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->open_quick_disk(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_QD, drv, path);
	}
}
// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_quick_disk(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_qd())) return;
	if((drive < 0) || (drive >= using_flags->get_max_qd())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_quick_disk_path[drive], list);
	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_quick_disk_dir, __dir, _MAX_PATH - 1);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	emit sig_ui_update_quick_disk_list(drive, list);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_QD, drive, relpath);
}
void EmuThreadClassBase::done_close_quick_disk(int drive)
{
	emit sig_ui_close_quick_disk(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_QD, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::do_open_cdrom(int drv, QString path)
{
	if(using_flags->is_use_compact_disc()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->open_compact_disc(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_CD, drv, path);
	}
}
void EmuThreadClassBase::do_eject_cdrom(int drv)
{
	if(using_flags->is_use_compact_disc()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_compact_disc(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_CD, drv, QString::fromUtf8(""));
	}
}
// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_compact_disc(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_compact_disc())) return;
	if((drive < 0) || (drive >= using_flags->get_max_cd())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_compact_disc_path[drive], list);

	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_compact_disc_dir, __dir, _MAX_PATH - 1);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	emit sig_ui_update_compact_disc_list(drive, list);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_CD, drive, relpath);
}
void EmuThreadClassBase::done_close_compact_disc(int drive)
{
	emit sig_ui_close_compact_disc(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_CD, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::do_close_hard_disk(int drv)
{
	if(using_flags->is_use_hdd()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_hard_disk(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_HD, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_hard_disk(int drv, QString path)
{
	if(using_flags->is_use_hdd()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->open_hard_disk(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_HD, drv, path);
	}
}
// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_hard_disk(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_hdd())) return;
	if((drive < 0) || (drive >= using_flags->get_max_hdd())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_hard_disk_path[drive], list);

	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_hard_disk_dir, __dir, _MAX_PATH - 1);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	emit sig_ui_update_hard_disk_list(drive, list);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_HD, drive, relpath);
}
void EmuThreadClassBase::done_close_hard_disk(int drive)
{
	emit sig_ui_close_hard_disk(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_HD, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::do_close_cart(int drv)
{
	if(using_flags->is_use_cart()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_cart(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_Cart, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_cart(int drv, QString path)
{
	if(using_flags->is_use_cart()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->open_cart(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_Cart, drv, path);
	}
}

// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_cart(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_cart())) return;
	if((drive < 0) || (drive >= using_flags->get_max_cart())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_cart_path[drive], list);

	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_cart_dir, __dir, _MAX_PATH - 1);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	emit sig_ui_update_cart_list(drive, list);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Cart, drive, relpath);
}
void EmuThreadClassBase::done_close_cart(int drive)
{
	emit sig_ui_close_cart(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Cart, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::do_close_laser_disc(int drv)
{
	if(using_flags->is_use_laser_disc()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_laser_disc(drv);
		emit sig_change_virtual_media(CSP_DockDisks_Domain_LD, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_laser_disc(int drv, QString path)
{
	if(using_flags->is_use_laser_disc()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->open_laser_disc(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_LD, drv, path);
	}
}
// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_laser_disc(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_laser_disc())) return;
	if((drive < 0) || (drive >= using_flags->get_max_ld())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_laser_disc_path[drive], list);

	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_laser_disc_dir, __dir, _MAX_PATH - 1);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	emit sig_ui_update_laser_disc_list(drive, list);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_LD, drive, relpath);
}
void EmuThreadClassBase::done_close_laser_disc(int drive)
{
	emit sig_ui_close_laser_disc(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_LD, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::do_load_binary(int drv, QString path)
{
	if(using_flags->is_use_binary_file()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->load_binary(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_Binary, drv, path);
	}
}

void EmuThreadClassBase::do_save_binary(int drv, QString path)
{
	if(using_flags->is_use_binary_file()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->save_binary(drv, path.toLocal8Bit().constData());
		emit sig_change_virtual_media(CSP_DockDisks_Domain_Binary, drv, QString::fromUtf8(""));
	}
}

// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_binary(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_binary_file())) return;
	if((drive < 0) || (drive >= using_flags->get_max_binary())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_binary_path[drive], list);
	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_binary_dir, __dir, _MAX_PATH - 1);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	emit sig_ui_update_binary_list(drive, list);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Binary, drive, relpath);
}
void EmuThreadClassBase::done_close_binary(int drive)
{
	emit sig_ui_close_binary(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Binary, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::do_write_protect_bubble_casette(int drv, bool flag)
{
	if(using_flags->is_use_bubble()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->is_bubble_casette_protected(drv, flag);
	}
}

void EmuThreadClassBase::do_close_bubble_casette(int drv)
{
	if(using_flags->is_use_bubble()) {
		//QMutexLocker _locker(&uiMutex);
		p_emu->close_bubble_casette(drv);
		p_emu->b77_file[drv].bank_num = 0;
		p_emu->b77_file[drv].cur_bank = -1;
//		emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, drv, QString::fromUtf8(""));
	}
}

void EmuThreadClassBase::do_open_bubble_casette(int drv, QString path, int bank)
{
	if(!(using_flags->is_use_bubble())) return;

	//QMutexLocker _locker(&uiMutex);
	QByteArray localPath = path.toLocal8Bit();

	p_emu->b77_file[drv].bank_num = 0;
	p_emu->b77_file[drv].cur_bank = -1;

	if(check_file_extension(localPath.constData(), ".b77")) {

		FILEIO *fio = new FILEIO();
		if(fio->Fopen(localPath.constData(), FILEIO_READ_BINARY)) {
			try {
				fio->Fseek(0, FILEIO_SEEK_END);
				int file_size = fio->Ftell(), file_offset = 0;
				while(file_offset + 0x2b0 <= file_size && p_emu->b77_file[drv].bank_num < using_flags->get_max_b77_banks()) {
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
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, drv, path);
	emit sig_update_recent_bubble(drv);

}
// Signal from EMU:: -> OSD:: -> EMU_THREAD (-> GUI)
void EmuThreadClassBase::done_open_bubble(int drive, QString path)
{
	if((using_flags.get() == nullptr) || (p_config == nullptr)) return;

	if(!(using_flags->is_use_bubble())) return;
	if((drive < 0) || (drive >= using_flags->get_max_bubble())) return;

	QStringList list;
	_TCHAR path_shadow[_MAX_PATH] = {0};
	strncpy(path_shadow, path.toLocal8Bit().constData(), _MAX_PATH - 1);
	UPDATE_HISTORY(path_shadow, p_config->recent_bubble_casette_path[drive], list);

	const _TCHAR* __dir = get_parent_dir((const _TCHAR *)path_shadow);
	strncpy(p_config->initial_bubble_casette_dir, __dir, _MAX_PATH - 1);

	emit sig_ui_update_bubble_casette_list(drive, list);
	emit sig_ui_clear_b77(drive);

	QString relpath = QString::fromUtf8("");
	if(strlen(&(__dir[0])) > 1) {
		relpath = QString::fromLocal8Bit(&(__dir[1]));
	}
	bool _f = check_file_extension(path_shadow, ".b77");
	if(_f) {
		if(p_emu != nullptr) {
			int slot = p_emu->b77_file[drive].cur_bank;
			for(int i = 0; i < p_emu->b77_file[drive].bank_num; i++) {
				if(i >= 16) break;
				_TCHAR tmpname[128] = {0};
				my_strcpy_s(tmpname, 127, p_emu->b77_file[drive].bubble_name[i]);
				QString tmps = QString::fromLocal8Bit(tmpname);
				emit sig_ui_update_b77(drive, i, tmps);
				if(i == slot) {
					emit sig_ui_select_b77(drive, i);
					relpath = tmps;
				}
			}
		}
	}
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, drive, relpath);
}

void EmuThreadClassBase::done_close_bubble(int drive)
{
	emit sig_ui_close_bubble_casette(drive);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, drive, QString::fromUtf8(""));
}

void EmuThreadClassBase::done_select_b77(int drive, int slot)
{
	if(p_emu == nullptr) return;

	if(slot < 0) return;
	if(slot >= 16) return;
	if(p_emu->b77_file[drive].bank_num < 0) return;
	if(p_emu->b77_file[drive].bank_num >= 64) return;
	if(p_emu->b77_file[drive].bank_num <= slot) return;
	p_emu->b77_file[drive].cur_bank = slot;
	_TCHAR tmpname[128] = {0};
	my_strcpy_s(tmpname, 127, p_emu->b77_file[drive].bubble_name[slot]);
	QString tmps = QString::fromLocal8Bit(tmpname);
	emit sig_ui_select_b77(drive, slot);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_Bubble, drive, tmps);
}

// Debugger

void EmuThreadClassBase::do_close_debugger(void)
{
	if(using_flags->is_use_debugger()) {
		emit sig_quit_debugger();
	}
}

void EmuThreadClassBase::set_romakana(bool flag)
{
	if(using_flags->is_use_auto_key()) {
		p_emu->set_auto_key_char(flag ? 1 : 0);
	}
}

void EmuThreadClassBase::moved_mouse(double x, double y, double globalx, double globaly)
{
	if(using_flags->is_use_one_board_computer() || (using_flags->get_max_button() > 0)) {
		mouse_x = x;
		mouse_y = y;
//		bool flag = p_osd->is_mouse_enabled();
//		if(!flag) return;
//		printf("Mouse Moved: %g, %g\n", x, y);
//		p_osd->set_mouse_pointer(floor(x), floor(y));
	} else if(using_flags->is_use_mouse()) {
//		double factor = (double)(p_config->mouse_sensitivity & ((1 << 16) - 1));
//		mouse_x = (int)(floor((globalx * factor) / 8192.0));
//		mouse_y = (int)(floor((globaly * factor) / 8192.0));
		mouse_x = globalx;
		mouse_y = globaly;
		//printf("Moved Mouse %d, %d\n", x, y);
		bool flag = p_osd->is_mouse_enabled();
		if(!flag) return;
		//printf("Mouse Moved: %d, %d\n", x, y);
		p_osd->set_mouse_pointer(mouse_x, mouse_y);
	}
}

void EmuThreadClassBase::button_pressed_mouse_sub(Qt::MouseButton button)
{

	if(using_flags->is_use_one_board_computer() || using_flags->is_use_mouse() || (using_flags->get_max_button() > 0)) {
		int stat = p_osd->get_mouse_button();
		bool flag = (p_osd->is_mouse_enabled() || using_flags->is_use_one_board_computer() || (using_flags->get_max_button() > 0));
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
			return;
			break;
		default:
			break;
		}
		if(!flag) return;
		p_osd->set_mouse_button(stat);
	}
}

void EmuThreadClassBase::button_released_mouse_sub(Qt::MouseButton button)
{

	if(using_flags->is_use_one_board_computer() || using_flags->is_use_mouse() || (using_flags->get_max_button() > 0)) {
		int stat = p_osd->get_mouse_button();
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
		default:
			break;
		}
		p_osd->set_mouse_button(stat);
	}
}

void EmuThreadClassBase::do_notify_power_off()
{
	poweroff_notified = true;
}

void EmuThreadClassBase::do_set_display_size(int w, int h, int ww, int wh)
{
	p_emu->suspend();
	p_emu->set_host_window_size(w, h, true);
}

void EmuThreadClassBase::dec_message_count(void)
{
	p_emu->message_count--;
}
