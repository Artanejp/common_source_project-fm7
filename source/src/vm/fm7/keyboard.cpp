/*
 * Common Source code Project -> VM -> FM-7/77AV -> Keyboard
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Licence: GPLv2
 * History : 
 *  Feb 12, 2015 : Initial 
 */

#include "../../fifo.h"
#include "../device.h"
#include "fm7_keyboard.h"

//
const uint16 vk_matrix_106[0x68] = { // VK
	// +0, +1, +2, +3, +4, +5, +6, +7
	/* 0x00, ESC, 1 , 2, 3, 4, 5, 6 */
	0x00,		 VK_PAUSE,	'1',		'2',		'3',		'4',		'5',		'6',		// +0x00
	/* 7, 8, 9, 0, - , ^, \|, BS */
	'7',		'8',		'9',		'0',		0xbd,		0xde,		0xdc,		VK_BACK,	// +0x08
	/* TAB, Q, W, E, R, T, Y, U */
	VK_TAB,		'Q',		'W',		'E',		'R',		'T',		'Y',		'U',		// +0x10
	/* I, O, P, @, [, [RET], A, S */
	'I',		'O',		'P',		0xc0,	 	0xdb,		VK_RETURN,	'A',		'S',		//+0x18
	/* D, F, G, H, J, K, L, ; */
	'D',		'F',		'G',		'H',		'J',		'K',		'L',		0xbb,		// +0x20
	/* :, ], Z, X, C, V, B, N */
	0xba,		0xdd,	 	'Z',		'X',		'C',		'V',		'B',		'N',		// +0x28
	/* M, , , ., / , \, RSPACE=RWIN , * , / */
	'M',		0xbc,		0xbe,		0xbf,		0xe2,		VK_RWIN,	VK_MULTIPLY,	VK_DIVIDE,	// +0x30
	/* + , - , 7, 8, 9, = , 4, 5 */
	VK_ADD,		VK_SUBTRACT,	VK_NUMPAD7,	VK_NUMPAD8,	VK_NUMPAD9,	0x00,		VK_NUMPAD4,	VK_NUMPAD5,	// +0x38
	/* 6, NUMPADCOMMA=RMENU , 1, 2, 3, NUMPADENTER=RETURN,0, . */
	VK_NUMPAD6,	VK_RMENU,	VK_NUMPAD1,	VK_NUMPAD2,	VK_NUMPAD3,	VK_RETURN,	VK_NUMPAD0,	VK_DECIMAL,	// +0x40
	/* INS, HOME, PRIOR, DEL, END, ↑, ↓,← */
	VK_INSERT,	VK_HOME,	VK_PRIOR,	VK_DELETE,	VK_END,		VK_UP,		VK_NEXT,	VK_LEFT,	// +0x48
	/* PAgeDown, →, LCTRL, LSHIFT, RSHIFT, CAPS, Graph=Muhenkan, Lspace=LALT */
	VK_DOWN,	VK_RIGHT,	0x11,		0x10,		VK_RSHIFT,	0x14,		0x1d,	0xf3,	// +0x50
	/* Cspace=Space, *Unknown*, KANA, *Unknown* , ESC(Break), F1, F2, F3 */
	VK_SPACE,	0x00,		0x15,		0x00,		VK_ESCAPE,	VK_F1,		VK_F2,	VK_F3,	// +0x58
	/* F4, F5, F6, F7, F8, F9, F10 , *END* */
	VK_F4,		VK_F5,		VK_F6,		VK_F7,		VK_F8,		VK_F9,		VK_F10,		0xffff	// +0x60
};

struct key_tbl_t {
	uint16 phy;
	uint16 code;
};

// Key tables value from XM7.
const key_tbl_t standard_key[] = {
	{0x01, 0x1b},
	{0x02, 0x31},
	{0x03, 0x32},
	{0x04, 0x33},
	{0x05, 0x34},
	{0x06, 0x35},
	{0x07, 0x36},
	{0x08, 0x37},
	{0x09, 0x38},
	{0x0a, 0x39},
	{0x0b, 0x30},
	{0x0c, 0x2d},
	{0x0d, 0x5e},
	{0x0e, 0x5c},
	{0x0f, 0x08},
	
	{0x10, 0x09},
	{0x11, 0x71},
	{0x12, 0x77},
	{0x13, 0x65},
	{0x14, 0x72},
	{0x15, 0x74},
	{0x16, 0x79},
	{0x17, 0x75},
	{0x18, 0x69},
	{0x19, 0x6f},
	{0x1a, 0x70},
	{0x1b, 0x40},
	{0x1c, 0x5b},
	{0x1d, 0x0d},
	{0x1e, 0x61},
	{0x1f, 0x73},
	
	{0x20, 0x64},
	{0x21, 0x66},
	{0x22, 0x67},
	{0x23, 0x68},
	{0x24, 0x6a},
	{0x25, 0x6b},
	{0x26, 0x6c},
	{0x27, 0x3b},
	{0x28, 0x3a},
	{0x29, 0x5d},
	{0x2a, 0x7a},
	{0x2b, 0x78},
	{0x2c, 0x63},
	{0x2d, 0x76},
	{0x2e, 0x62},
	{0x2f, 0x6e},
	
	{0x30, 0x6d},
	{0x31, 0x2c},
	{0x32, 0x2e},
	{0x33, 0x2f},
	{0x34, 0x22},
	{0x35, 0x20},
	{0x36, 0x2a},
	{0x37, 0x2f},
	{0x38, 0x2b},
	{0x39, 0x2d},
	{0x3a, 0x37},
	{0x3b, 0x38},
	{0x3c, 0x39},
	{0x3d, 0x3d},
	{0x3e, 0x34},
	{0x3f, 0x35},
	
	{0x40, 0x36},
	{0x41, 0x2c},
	{0x42, 0x31},
	{0x43, 0x32},
	{0x44, 0x33},
	{0x45, 0x0d},
	{0x46, 0x30},
	{0x47, 0x2e},
	{0x48, 0x12},
	{0x49, 0x05},
	{0x4a, 0x0c},
	{0x4b, 0x7f},
	{0x4c, 0x11},
	{0x4d, 0x1e},
	{0x4e, 0x0b},
	{0x4f, 0x1d},
	
	{0x50, 0x1f},
	{0x51, 0x1c},
	
	{0x57, 0x20},
	{0x58, 0x20},
	
	{0x5d, 0x0101},
	{0x5e, 0x0102},
	{0x5f, 0x0103},
	{0x60, 0x0104},
	{0x61, 0x0105},
	{0x62, 0x0106},
	{0x63, 0x0107},
	{0x64, 0x0108},
	{0x65, 0x0109},
	{0x66, 0x010a},
	
	{0xffff, 0xffff}
};

const key_tbl_t standard_shift_key[] = {
	{0x01, 0x1b},
	{0x02, 0x21},
	{0x03, 0x22},
	{0x04, 0x23},
	{0x05, 0x24},
	{0x06, 0x25},
	{0x07, 0x26},
	{0x08, 0x27},
	{0x09, 0x28},
	{0x0a, 0x29},
	
	{0x0c, 0x3d},
	{0x0d, 0x7e},
	{0x0e, 0x7c},
	{0x0f, 0x08},
	
	{0x10, 0x09},
	{0x11, 0x51},
	{0x12, 0x57},
	{0x13, 0x45},
	{0x14, 0x52},
	{0x15, 0x54},
	{0x16, 0x59},
	{0x17, 0x55},
	{0x18, 0x49},
	{0x19, 0x4f},
	{0x1a, 0x50},
	{0x1b, 0x60},
	{0x1c, 0x7b},
	{0x1d, 0x0d},
	{0x1e, 0x41},
	{0x1f, 0x53},
	
	{0x20, 0x44},
	{0x21, 0x46},
	{0x22, 0x47},
	{0x23, 0x48},
	{0x24, 0x4a},
	{0x25, 0x4b},
	{0x26, 0x4c},
	{0x27, 0x2b},
	{0x28, 0x2a},
	{0x29, 0x7d},
	{0x2a, 0x5a},
	{0x2b, 0x58},
	{0x2c, 0x43},
	{0x2d, 0x56},
	{0x2e, 0x42},
	{0x2f, 0x4e},
	
	{0x30, 0x4d},
	{0x31, 0x3c},
	{0x32, 0x3e},
	{0x33, 0x3f},
	{0x34, 0x5f},
	{0x35, 0x20},
	{0x36, 0x2a},
	{0x37, 0x2f},
	{0x38, 0x2b},
	{0x39, 0x2d},
	{0x3a, 0x37},
	{0x3b, 0x38},
	{0x3c, 0x39},
	{0x3d, 0x3d},
	{0x3e, 0x34},
	{0x3f, 0x35},

	{0x40, 0x36},
	{0x41, 0x2c},
	{0x42, 0x31},
	{0x43, 0x32},
	{0x44, 0x33},
	{0x45, 0x0d},
	{0x46, 0x30},
	{0x47, 0x2e},
	{0x48, 0x12},
	{0x49, 0x05},
	{0x4a, 0x0c},
	{0x4b, 0x7f},
	{0x4c, 0x11},
	{0x4d, 0x19},
	{0x4e, 0x0b},
	{0x4f, 0x02},
	
	{0x50, 0x1a},
	{0x51, 0x06},

	{0x57, 0x20},
	{0x58, 0x20},
	{0xffff, 0xffff}
};

const struct key_tbl_t ctrl_key[] = {
	{0x0c, 0x1e},
	{0x0d, 0x1c},
	
	{0x11, 0x11},
	{0x12, 0x17},
	{0x13, 0x05},
	{0x14, 0x12},
	{0x15, 0x14},
	{0x16, 0x19},
	{0x17, 0x15},
	{0x18, 0x09},
	{0x19, 0x0f}, // 09
	{0x1a, 0x10},
	{0x1b, 0x00},
	{0x1c, 0x1b},
	{0x1e, 0x01},
	{0x1f, 0x13},
	
	{0x20, 0x04},
	{0x21, 0x06},
	{0x22, 0x07},
	{0x23, 0x08},
	{0x24, 0x0a},
	{0x25, 0x0b},
	{0x26, 0x0c},
	{0x29, 0x1d},
	{0x2a, 0x1a},
	{0x2b, 0x18},
	{0x2c, 0x03},
	{0x2d, 0x16},
	{0x2e, 0x02},
	{0x2f, 0x0e},
  
	{0x30, 0x0d},
	
	{0x34, 0x1f},
	{0xffff, 0xffff}
};

const struct key_tbl_t ctrl_shift_key[] = {
	{0x0c, 0x1e},
	{0x0d, 0x1c},
	
	{0x11, 0x11},
	{0x12, 0x17},
	{0x13, 0x05},
	{0x14, 0x12},
	{0x15, 0x14},
	{0x16, 0x19},
	{0x17, 0x15},
	{0x18, 0x09},
	{0x19, 0x09},
	{0x1a, 0x10},
	{0x1b, 0x00},
	{0x1c, 0x1b},
	{0x1e, 0x01},
	{0x1f, 0x13},
  
	{0x20, 0x04},
	{0x21, 0x06},
	{0x22, 0x07},
	{0x23, 0x08},
	{0x24, 0x0a},
	{0x25, 0x0b},
	{0x26, 0x0c},
	{0x29, 0x1d},
	{0x2a, 0x1a},
	{0x2b, 0x18},
	{0x2c, 0x03},
	{0x2d, 0x16},
	{0x2e, 0x02},
	{0x2f, 0x0e},
  
	{0x30, 0x0d},
	
	{0x34, 0x1f},
	{0xffff, 0xffff}
};

const struct key_tbl_t graph_key[] = {
	{0x01, 0x1b},
	{0x02, 0xf9},
	{0x03, 0xfa},
	{0x04, 0xfb},
	{0x05, 0xfc},
	{0x06, 0xf2},
	{0x07, 0xf3},
	{0x08, 0xf4},
	{0x09, 0xf5},
	{0x0a, 0xf6},
	{0x0b, 0xf7},
	{0x0c, 0x8c},
	{0x0d, 0x8b},
	{0x0e, 0xf1},
	{0x0f, 0x08},
	
	{0x10, 0x09},
	{0x11, 0xfd},
	{0x12, 0xf8},
	{0x13, 0xe4},
	{0x14, 0xe5},
	{0x15, 0x9c},
	{0x16, 0x9d},
	{0x17, 0xf0},
	{0x18, 0xe8},
	{0x19, 0xe9},
	{0x1a, 0x8d},
	{0x1b, 0x8a},
	{0x1c, 0xed},
	{0x1d, 0x0d},
	{0x1e, 0x95},
	{0x1f, 0x96},
  
	{0x20, 0xe6},
	{0x21, 0xe7},
	{0x22, 0x9e},
	{0x23, 0x9f},
	{0x24, 0xea},
	{0x25, 0xeb},
	{0x26, 0x8e},
	{0x27, 0x99},
	{0x28, 0x94},
	{0x29, 0xec},
	{0x2a, 0x80},
	{0x2b, 0x81},
	{0x2c, 0x82},
	{0x2d, 0x83},
	{0x2e, 0x84},
	{0x2f, 0x85},
	
	{0x30, 0x86},
	{0x31, 0x87},
	{0x32, 0x88},
	{0x33, 0x97},
	{0x34, 0xe0},
	{0x35, 0x20},
	{0x36, 0x98},
	{0x37, 0x91},
	{0x38, 0x99},
	{0x39, 0xee},
	{0x3a, 0xe1},
	{0x3b, 0xe2},
	{0x3c, 0xe3},
	{0x3d, 0xef},
	{0x3e, 0x93},
	{0x3f, 0x8f},

	{0x40, 0x92}, 
	
	{0x42, 0x9a},
	{0x43, 0x90},
	{0x44, 0x9b},
	{0x45, 0x0d},
	{0x48, 0x12},
	{0x49, 0x05},
	{0x4a, 0x0c},
	{0x4b, 0x7f},
	{0x4c, 0x11},
	{0x4d, 0x1e},
	{0x4e, 0x0b},
	{0x4f, 0x1d},
  
	{0x50, 0x1f},
	{0x51, 0x1c},

	{0x57, 0x20},
	{0x58, 0x20},
	/* Belows is none when shift */
	{0x5d, 0x101},
	{0x5e, 0x102},
	{0x5f, 0x103},
	{0x60, 0x104},
	{0x61, 0x105},
	{0x62, 0x106},
	{0x63, 0x107},
	{0x64, 0x108},
	{0x65, 0x109},
	{0x66, 0x10a},
	{0xffff, 0xffff}
};
const struct key_tbl_t graph_shift_key[] = {
	{0x01, 0x1b},
	{0x02, 0xf9},
	{0x03, 0xfa},
	{0x04, 0xfb},
	{0x05, 0xfc},
	{0x06, 0xf2},
	{0x07, 0xf3},
	{0x08, 0xf4},
	{0x09, 0xf5},
	{0x0a, 0xf6},
	{0x0b, 0xf7},
	{0x0c, 0x8c},
	{0x0d, 0x8b},
	{0x0e, 0xf1},
	{0x0f, 0x08},

	{0x10, 0x09},
	{0x11, 0xfd},
	{0x12, 0xf8},
	{0x13, 0xe4},
	{0x14, 0xe5},
	{0x15, 0x9c},
	{0x16, 0x9d},
	{0x17, 0xf0},
	{0x18, 0xe8},
	{0x19, 0xe9},
	{0x1a, 0x8d},
	{0x1b, 0x8a},
	{0x1c, 0xed},
	{0x1d, 0x0d},
	{0x1e, 0x95},
	{0x1f, 0x96},
  
	{0x20, 0xe6},
	{0x21, 0xe7},
	{0x22, 0x9e},
	{0x23, 0x9f},
	{0x24, 0xea},
	{0x25, 0xeb},
	{0x26, 0x8e},
	{0x27, 0x99},
	{0x28, 0x94},
	{0x29, 0xec},
	{0x2a, 0x80},
	{0x2b, 0x81},
	{0x2c, 0x82},
	{0x2d, 0x83},
	{0x2e, 0x84},
	{0x2f, 0x85},
  
	{0x30, 0x86},
	{0x31, 0x87},
	{0x32, 0x88},
	{0x33, 0x97},
	{0x34, 0xe0},
	{0x35, 0x20},
	{0x36, 0x98},
	{0x37, 0x91},
	{0x38, 0x99},
	{0x39, 0xee},
	{0x3a, 0xe1},
	{0x3b, 0xe2},
	{0x3c, 0xe3},
	{0x3d, 0xef},
	{0x3e, 0x93},
	{0x3f, 0x8f},

	{0x40, 0x92}, 
	
	{0x42, 0x9a},
	{0x43, 0x90},
	{0x44, 0x9b},
	{0x45, 0x0d},
	
	{0x48, 0x12},
	{0x49, 0x05},
	{0x4a, 0x0c},
	{0x4b, 0x7f},
	{0x4c, 0x11},
	{0x4d, 0x19},
	{0x4e, 0x0b},
	{0x4f, 0x02},
  
	{0x50, 0x1a},
	{0x51, 0x06},
	
	{0x57, 0x20},
	{0x58, 0x20},
	{0xffff, 0xffff}
};

const struct key_tbl_t kana_key[] = {
	{0x01, 0x1b},
	{0x02, 0xc7},
	{0x03, 0xcc},
	{0x04, 0xb1},
	{0x05, 0xb3},
	{0x06, 0xb4},
	{0x07, 0xb5},
	{0x08, 0xd4},
	{0x09, 0xd5},
	{0x0a, 0xd6},
	{0x0b, 0xdc},
	{0x0c, 0xce},
	{0x0d, 0xcd},
	{0x0e, 0xb0},
	{0x0f, 0x08},
	
	{0x10, 0x09},
	{0x11, 0xc0},
	{0x12, 0xc3},
	{0x13, 0xb2},
	{0x14, 0xbd},
	{0x15, 0xb6},
	{0x16, 0xdd},
	{0x17, 0xc5},
	{0x18, 0xc6},
	{0x19, 0xd7},
	{0x1a, 0xbe},
	{0x1b, 0xde},
	{0x1c, 0xdf},
	{0x1d, 0x0d},
	{0x1e, 0xc1},
	{0x1f, 0xc4},
	
	{0x20, 0xbc},
	{0x21, 0xca},
	{0x22, 0xb7},
	{0x23, 0xb8},
	{0x24, 0xcf},
	{0x25, 0xc9},
	{0x26, 0xd8},
	{0x27, 0xda},
	{0x28, 0xb9},
	{0x29, 0xd1},
	{0x2a, 0xc2},
	{0x2b, 0xbb},
	{0x2c, 0xbf},
	{0x2d, 0xcb},
	{0x2e, 0xba},
	{0x2f, 0xd0},
		  
	{0x30, 0xd3},
	{0x31, 0xc8},
	{0x32, 0xd9},
	{0x33, 0xd2},
	{0x34, 0xdb},
	{0x35, 0x20},
	{0x36, 0x2a},
	{0x37, 0x2f},
	{0x38, 0x2b},
	{0x39, 0x2d},
	{0x3a, 0x37},
	{0x3b, 0x38},
	{0x3c, 0x39},
	{0x3d, 0x3d},
	{0x3e, 0x34},
	{0x3f, 0x35},
		  
	{0x40, 0x36},
	{0x41, 0x2c},
	{0x42, 0x31},
	{0x43, 0x32},
	{0x44, 0x33},
	{0x45, 0x0d},
	{0x46, 0x30},
	{0x47, 0x2e},
  
	{0x48, 0x12},
	{0x49, 0x05},
	{0x4a, 0x0c},
	{0x4b, 0x7f},
	{0x4c, 0x11},
	{0x4d, 0x1e},
	
	{0x4e, 0x0b},
	{0x4f, 0x1d},
	{0x50, 0x1f},
	{0x51, 0x1c},
	
	
	{0x57, 0x20},
	{0x58, 0x20},
	
	{0x5d, 0x0101},
	{0x5e, 0x0102},
	{0x5f, 0x0103},
	{0x60, 0x0104},
	{0x61, 0x0105},
	{0x62, 0x0106},
	{0x63, 0x0107},
	{0x64, 0x0108},
	{0x65, 0x0109},
	{0x66, 0x010a},

	{0xffff, 0xffff}
};

const struct key_tbl_t kana_shift_key[] = {
	{0x01, 0x1b},
	{0x04, 0xa7},
	{0x05, 0xa9},
	{0x06, 0xaa},
	{0x07, 0xab},
	{0x08, 0xac},
	{0x09, 0xad},
	{0x0a, 0xae},
	{0x0b, 0xa6},
	{0x0f, 0x08},
	
	{0x10, 0x09},
	{0x13, 0xa8},
	{0x1c, 0xa2},
	{0x1d, 0x0d},
	
	{0x29, 0xa3},
	{0x2a, 0xaf},
	
	{0x31, 0xa4},
	{0x32, 0xa1},
	{0x33, 0xa5},
	
	{0x35, 0x20},
	{0x36, 0x2a},
	{0x37, 0x2f},
	{0x38, 0x2b},
	{0x39, 0x2d},
	{0x3a, 0x37},
	{0x3b, 0x38},
	{0x3c, 0x39},
	{0x3d, 0x3d},
	{0x3e, 0x34},
	{0x3f, 0x35},
	
	{0x40, 0x36},
	{0x41, 0x2c},
	{0x42, 0x31},
	{0x43, 0x32},
	{0x44, 0x33},
	{0x45, 0x0d},
	{0x46, 0x30},
	{0x47, 0x2e},
  
	{0x48, 0x12},
	{0x49, 0x05},
	{0x4a, 0x0c},
	{0x4b, 0x7f},
	{0x4c, 0x11},
	{0x4d, 0x19},

	{0x4e, 0x0b},
	{0x4f, 0x02},
	{0x50, 0x1a},
	{0x51, 0x06},
	
	{0x57, 0x20},
	{0x58, 0x20},

	{0xffff, 0xffff}
};

/*
 * I/O API (subio)
 */
// 0xd400(SUB) or 0xfd00(MAIN)
uint8 KEYBOARD::get_keycode_high(void)
{
	uint8 data = 0x00;
	if((keycode_7 & 0x0100) != 0) data |= 0x01;
	return data;
}

// 0xd401(SUB) or 0xfd01(MAIN)
uint8 KEYBOARD::get_keycode_low(void)
{
	uint8 data = keycode_7 & 0xff;
	mainio->write_signal(FM7_MAINIO_KEYBOARDIRQ, 0, 1);
	display->write_signal(SIG_FM7_SUB_KEY_FIRQ, 0, 1);
	return data;
}

// 0xd40d : R
void KEYBOARD::turn_on_ins_led(void)
{
	this->write_signals(&ins_led, 0xff);
}

// 0xd40d : W
void KEYBOARD::turn_off_ins_led(void)
{
	this->write_signals(&ins_led, 0x00);
}

// UI Handler. 
uint16 KEYBOARD::vk2scancode(uint32 vk)
{
	uint16 i;
	i = 0;
	do {
		if(vk_matrix_106[i] == vk) return i;
		i++;
	} while(vk_matrix_106[i] != 0xffff);
	return 0x0000;
}

bool KEYBOARD::isModifier(uint16 scancode)
{
	if(((scancode >= 0x52) && (scancode <= 0x56)) || // CTRL LSHIFT RSHIFT CAPS GRPH
     		(scancode == 0x5a) || (scancode == 0x5c)) { // KANA BREAK
		return true;
	}
	return false;
}

void KEYBOARD::set_modifiers(uint16 scancode, bool flag)
{
	if(scancode == 0x52) { // CTRL
		ctrl_pressed = flag; 
	} else if(scancode == 0x53) { // LSHIFT
		lshift_pressed = flag;
		if(rshift_pressed) {
			shift_pressed = true;
		} else {
			shift_pressed = flag;
		}
	} else if(scancode == 0x54) { // RSHIFT
		rshift_pressed = flag;
		if(lshift_pressed) {
		  shift_pressed = true;
		} else {
		  shift_pressed = flag;
		}
	} else if(scancode == 0x56) { // GRPH
		graph_pressed = flag;
	} else if(scancode == 0x55) { // CAPS
		// Toggle on press.
		if(flag) {
			if(caps_pressed) {
				caps_pressed = false;
			} else {
				caps_pressed = true;
			}
			if(keymode == KEYMODE_STANDARD) this->write_signals(&caps_led, caps_pressed ? 0xff : 0x00);
		}
	} else if(scancode == 0x5a) { // KANA
		// Toggle on press.
		if(flag) {
			if(kana_pressed) {
				kana_pressed = false;
			} else {
				kana_pressed = true;
			}
			if(keymode == KEYMODE_STANDARD) this->write_signals(&kana_led, kana_pressed ? 0xff : 0x00);
		}
	} else if(scancode == 0x5c) { // Break
		break_pressed = flag;
	}
}

uint16 KEYBOARD::scan2fmkeycode(uint16 scancode)
{
	const struct key_tbl_t *keyptr;
	uint16 code;
	bool stdkey = false;
	int i;
	uint16 retval;
	
	if((scancode == 0) || (scancode >= 0x67)) return 0xffff;
	// Set repeat flag(s)
	if(shift_pressed && ctrl_pressed) {
		switch(scancode) {
			case 0x02: // 1
			case 0x42: // 1
				repeat_mode = true;
				//return 0xffff;
				break;
			case 0x0b: // 0
			case 0x46: // 0
				repeat_mode = false;
				//return 0xffff;
				break;
		}
	}
	if(keymode == KEYMODE_STANDARD) {
		if(ctrl_pressed) {
			if(shift_pressed) {
				keyptr = ctrl_shift_key;
			} else {
				keyptr = ctrl_key;
			}
		} else if(graph_pressed) {
			if(shift_pressed) {
				keyptr = graph_shift_key;
			} else {
				keyptr = graph_key;
			}
		} else if(kana_pressed) {
			if(shift_pressed) {
				keyptr = kana_shift_key;
			} else {
				keyptr = kana_key;
			}
		} else { // Standard
			stdkey = true;
			if(shift_pressed) {
				keyptr = standard_shift_key;
			} else {
				keyptr = standard_key;
			}
		}
		if(keyptr == NULL) return 0xffff;
	}
#if defined(_FM77AV_VARIANTS)
	else 	if(shift_pressed) {
	  // DO super-impose mode:
	  // F7 : PC
	  // F8 : IMPOSE (High brightness)
	  // F9 : IMPOSE (Low brightness)
	  // F10: TV
	}
	if(keymode == KEYMODE_SCAN) {
		retval = scancode;
		return retval;
	} else if(keymode == KEYMODE_16BETA) { // Will Implement
		return 0xffff;
	}
#endif //_FM77AV_VARIANTS	
	i = 0;
	retval = 0xffff;
	do {
		if(keyptr[i].phy == scancode) {
			retval = keyptr[i].code;
			break;
		}
		i++;
	} while(keyptr[i].phy != 0xffff);

	if(stdkey) {
		if((retval >= 'A') && (retval <= 'Z')) {
			if(caps_pressed) {
				retval += 0x20;
			}
		} else if((retval >= 'a') && (retval <= 'z')) {
			if(caps_pressed) {
				retval -= 0x20;
			}
		}
	}
	return retval;
}

void KEYBOARD::key_up(uint32 vk)
{
	uint16 scancode = vk2scancode(vk);
	bool stat_break = break_pressed;
	uint32 code_7;

	if(scancode == 0) return;
	if(event_ids[scancode] >= 0){
		cancel_event(this, event_ids[scancode]);
		event_ids[scancode] = -1;
	}
	key_pressed_flag[scancode] = false; 
	if(this->isModifier(scancode)) {
		set_modifiers(scancode, false);
		if(break_pressed != stat_break) { // Break key UP.
			//mainio->write_signal(FM7_MAINIO_PUSH_BREAK, 0x00, 0xff);
			this->write_signals(&break_line, 0x00);
			return;	   
		}
	}
	if(keymode == KEYMODE_SCAN) {
		code_7 = scan2fmkeycode(scancode);
		if(code_7 < 0x200) {
			keycode_7 = code_7;
			//mainio->write_signal(FM7_MAINIO_PUSH_KEYBOARD, code_7, 0x1ff);
			//mainio->write_signal(FM7_MAINIO_KEYBOARDIRQ, 0, 1);
			//display->write_signal(SIG_FM7_SUB_KEY_FIRQ, 0, 1);
		}
	}	  
}

void KEYBOARD::key_down(uint32 vk)
{
	double usec = (double)repeat_time_long * 1000.0;
	uint32 code_7;
	uint16 scancode = vk2scancode(vk);
	bool stat_break = break_pressed;
	//printf("VK=%04x SCAN=%04x break=%d\n", vk, scancode, stat_break);

	if(scancode == 0) return;
	key_pressed_flag[scancode] = true;
	
	if(this->isModifier(scancode)) {  // modifiers
		set_modifiers(scancode, true);
		if(break_pressed != stat_break) { // Break key Down.
			//mainio->write_signal(FM7_MAINIO_PUSH_BREAK, 0xff, 0xff);
			this->write_signals(&break_line, 0xff);
		}
	}
	code_7 = scan2fmkeycode(scancode);
	if(code_7 < 0x200) {
		keycode_7 = code_7;
		mainio->write_signal(FM7_MAINIO_PUSH_KEYBOARD, code_7, 0x1ff);
		mainio->write_signal(FM7_MAINIO_KEYBOARDIRQ, 1, 1);
		display->write_signal(SIG_FM7_SUB_KEY_FIRQ, 1, 1);
	}
	// If repeat && !(PF) && !(BREAK) 
	if((repeat_mode) && (scancode < 0x5c) && (scancode != 0)) {
		register_event(this,
			       ID_KEYBOARD_AUTOREPEAT_FIRST + scancode,
			       usec, false, &event_ids[scancode]);
	} else {
		event_ids[scancode] = -1;
	}
	key_pressed_flag[scancode] = true;

}

void KEYBOARD::do_repeatkey(uint16 scancode)
{
	uint16 code_7;
	if((scancode == 0) || (scancode >= 0x67)) return; // scancode overrun.
	if(!repeat_mode) {
		if(event_ids[scancode] >= 0) {
			cancel_event(this, event_ids[scancode]);
			event_ids[scancode] = -1;
		}
		return;
	}
	key_pressed_flag[scancode] = true;
	code_7 = scan2fmkeycode(scancode);
	if(code_7 < 0x200) {
		keycode_7 = code_7;
		mainio->write_signal(FM7_MAINIO_PUSH_KEYBOARD, code_7, 0x1ff);
		mainio->write_signal(FM7_MAINIO_KEYBOARDIRQ, 1, 1);
		display->write_signal(SIG_FM7_SUB_KEY_FIRQ, 1, 1);
	}
	//if(this->isModifiers(scancode)) {  // modifiers
	  //if(break_pressed != stat_break) { // Break key Down.
	  //		break_line->write_signal(0x00, 1, 1);
	  //		maincpu->write_signal(SIG_CPU_FIRQ, 1, 1);
	  //	}
	//}
}

void KEYBOARD::event_callback(int event_id, int err)
{
  	if(event_id == ID_KEYBOARD_RXRDY_OK) {
		write_signals(&rxrdy, 0xff);
	} else if(event_id == ID_KEYBOARD_RXRDY_BUSY) {
		write_signals(&rxrdy, 0x00);
	} else if(event_id == ID_KEYBOARD_ACK) {
		write_signals(&key_ack, 0xff);
	} else if((event_id >= ID_KEYBOARD_AUTOREPEAT_FIRST) && (event_id <= (ID_KEYBOARD_AUTOREPEAT_FIRST + 0x1ff))) {
		uint32 scancode = event_id - ID_KEYBOARD_AUTOREPEAT_FIRST;
		double usec = (double)repeat_time_short * 1000.0;

		if((scancode >= 0x67) || (scancode == 0)) return;
		do_repeatkey((uint16)scancode);
		register_event(this,
			       ID_KEYBOARD_AUTOREPEAT + scancode,
			       usec, true, &event_ids[scancode]);
		// Key repeat.
	} else if((event_id >= ID_KEYBOARD_AUTOREPEAT) && (event_id <= (ID_KEYBOARD_AUTOREPEAT + 0x1ff))){
		uint32 scancode = event_id - ID_KEYBOARD_AUTOREPEAT;
		do_repeatkey((uint16)scancode);
	}
}

// Commands
void KEYBOARD::reset(void)
{
	repeat_time_short = 70; // mS
	repeat_time_long = 700; // mS
	repeat_mode = true;
	keycode_7 = 0x00;
	
	lshift_pressed = false;
	rshift_pressed = false;
	ctrl_pressed   = false;
	graph_pressed = false;
	//	ins_pressed = false;
	cmd_fifo->clear();
	data_fifo->clear();
	datareg = 0x00;
	// Bus
	this->write_signals(&break_line, 0x00);		  
	this->write_signals(&rxrdy, 0x00);		  
	this->write_signals(&key_ack, 0x00);		  
	this->write_signals(&kana_led, 0x00);		  
	this->write_signals(&caps_led, 0x00);		  
	this->write_signals(&ins_led, 0x00);		  

}
#if defined(_FM77AV_VARIANTS)  
// 0xd431 : Read
uint8 KEYBOARD::read_data_reg(void)
{
	if(rxrdy->read_signal(0) != 0) {
		if(!data_fifo->empty()) {
			datareg = data_fifo->read() & 0xff;
		}
	}
	if(data_fifo->empty()) {
		rxrdy->write_signal(0x00, 0x00, 0x01);
	} else {
		rxrdy->write_signal(0x00, 0x01, 0x01);
	}
	return datareg;
}

// 0xd432
uint8 KEYBOARD::read_stat_reg(void)
{
	uint8 data = 0xff;
	
	if(!data_fifo->empty()) {
		rxrdy->write_signal(0x00, 0x01, 0x01);
		data &= 0x7f;
	}
	if(key_ack->read_signal(0) == 0x00) {
	  data &= 0xfe;
	}
	// Digityze : bit0 = '0' when waiting,
	return data;
}

void KEYBOARD::set_mode(void)
{
	int count = cmd_fifo->count();
	int cmd;
	if(count < 2) return;
	cmd = cmd_fifo->read();
	keymode = cmd_fifo->read();
	if(keymode <= KEYMODE_SCAN) reset();
	cmd_fifo->clear();
	data_fifo->clear(); // right?
	rxrdy->write_signal(0x00, 0x00, 0x01);
}

void KEYBOARD::get_mode(void)
{
	int cmd;
	int dummy;
	cmd = cmd_fifo->read();
	if(data_fifo->full()) {
		dummy = data_fifo->read();
	}
	data_fifo->write(keymode);
	rxrdy->write_signal(0x01, 0x01, 0x01);
}

void KEYBOARD::set_leds(void)
{
	int count = cmd_fifo->count();
	int cmd;
	int ledvar;
	if(count < 2) return;
	cmd = cmd_fifo->read();
	ledvar = cmd_fifo->read();
	if(ledvar < 4) {
		if((ledvar & 0x02) != 0) {
			// Kana
			kana_pressed = ((ledvar & 0x01) == 0);
			kana_led.write_signal(0x00, ~ledvar, 0x01);
		} else {
			// Caps
			caps_pressed = ((ledvar & 0x01) == 0);
			caps_led.write_signal(0x00, ~ledvar, 0x01);
		}
	}
	cmd_fifo->clear();
	data_fifo->clear(); // right?
	rxrdy->write_signal(0x01, 0x00, 0x01);
}

void KEYBOARD::get_leds(void)
{
	uint8 ledvar = 0x00;
	data_fifo->clear();
	ledvar |= caps_pressed ? 0x01 : 0x00;
	ledvar |= kana_pressed ? 0x02 : 0x00;
	data_fifo->write(ledvar);
	cmd_fifo->clear();
	rxrdy->write_signal(0x01, 0x01, 0x01);
}

void KEYBOARD::set_repeat_type(void)
{
	int cmd;
	int modeval;

	cmd = cmd_fifo->read();
	if(cmd_fifo->empty()) return;
	modeval = cmd_fifo->read();
	if((modeval >= 2) || (modeval < 0)) return;
	repeat_mode = (modeval == 0);
	data_fifo->clear();
	cmd_fifo->clear();
	rxrdy->write_signal(0x01, 0x00, 0x01);
}

void KEYBOARD::set_repeat_time(void)
{
	int cmd;
	int time_high;
	int time_low;
	cmd = cmd_fifo->read();
	if(cmd_fifo->empty()) return;
	time_high = cmd_fifo->read();
	if(cmd_fifo->empty()) return;
	time_low = cmd_fifo->read();
	if(cmd_fifo->empty()) return;
	
	if((time_high == 0) || (time_low == 0)) {
	  repeat_time_long = 700;
	  repeat_time_short = 70;
	} else {
	  repeat_time_long = time_high * 10;
	  repeat_time_short = time_low * 10;
	}
	data_fifo->clear();
	cmd_fifo->clear();
	rxrdy->write_signal(0x01, 0x00, 0x01);
}

void KEYBOARD::set_rtc(void)
{
	int cmd;
	int tmp;
	int localcmd;
	if(cmd_fifo->count() < 9) return;
	cmd = cmd_fifo->read();
	localcmd = cmd_fifo->read();
	// YY
	tmp = cmd_fifo->read();
	rtc_yy = ((tmp >> 4) * 10) | (tmp & 0x0f);
	// MM
	tmp = cmd_fifo->read();
	rtc_mm = ((tmp >> 4) * 10) | (tmp & 0x0f);
	// DD
	tmp = cmd_fifo->read();
	rtc_dd = (((tmp & 0x30) >> 4) * 10) | (tmp & 0x0f);
	// DayOfWeek + Hour
	tmp = cmd_fifo->read();
	rtc_count24h = ((tmp & 0x08) != 0);
	if(!rtc_count24h) {
		rtc_ispm = ((tmp & 0x04) != 0);
	}
	rtc_dayofweek = (tmp >> 4) % 0x07;
	rtc_hh = ((tmp & 0x03) * 10);
	// Low
	tmp = cmd_fifo->read();
	rtc_hh = rtc_hh | (tmp >> 4);
	if(rtc_count24h) {
	  rtc_ispm = (rtc_hh >= 12);
	}
	rtc_minute = (tmp & 0x0f) * 10;
	
	tmp = cmd_fifo->read();
	rtc_minute = rtc_minute | (tmp >> 4);
	rtc_sec = (tmp & 0x0f) * 10;
	
	tmp = cmd_fifo->read();
	rtc_sec = rtc_sec | (tmp >> 4);
	
	data_fifo->clear();
	cmd_fifo->clear();
	rxrdy->write_signal(0x01, 0x00, 0x01);
}

void KEYBOARD::get_rtc(void)
{
	int cmd;
	int tmp;
	int localcmd;
	data_fifo->clear();
	// YY
	tmp = ((rtc_yy / 10) << 4) | (rtc_yy % 10);
	data_fifo->write(tmp);
	// MM
	tmp = ((rtc_mm / 10) << 4) | (rtc_mm % 10);
	data_fifo->write(tmp);
	// DD
	tmp = ((rtc_dd / 10) << 4) | (rtc_dd % 10);
	tmp = tmp | (0 << 6); // leap
	data_fifo->write(tmp);
	// DayOfWeek + Hour
	tmp = rtc_dayofweek << 4;
	tmp = tmp | (rtc_hh / 10);
	if(rtc_count24h) {
	  tmp = tmp | 0x08;
	} else {
	  if(rtc_ispm) {
	    tmp = tmp | 0x04;
	  }
	}
	data_fifo->write(tmp);
	// Low
	tmp = (rtc_hh % 10) << 4;
	tmp = tmp | (rtc_mm / 10);
	data_fifo->write(tmp);
	
	tmp = (rtc_minute % 10) << 4;
	tmp = tmp | (rtc_sec / 10);
	data_fifo->write(tmp);
	
	tmp = (rtc_sec % 10) << 4;
	data_fifo->write(tmp);
	
	cmd_fifo->clear();
	rxrdy->write_signal(0x01, 0x01, 0x01);
}

const int rtc_month_days[12] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

void KEYBOARD::rtc_count(void)
{
	// count per 1sec
	rtc_sec++;
	if(rtc_sec >= 60) {
		rtc_sec = 0;
		rtc_minute++;
		if(rtc_minute >= 60) {
			rtc_minute = 0;
			rtc_hour++;
			if(rtc_count24h) {
				rtc_ispm = (rtc_hour >= 12);
				if(rtc_hour < 24) return;
			} else {
				if(rtc_ispm) {
					if(rtc_hour < 12) return;
				} else {
					if(rtc_hour < 12) return;
					rtc_ispm = true;
					rtc_hour = 0;
					return;
				}
			}
			// Day count up
			rtc_hour = 0;
			rtc_dd++;
			rtc_dayofweek++;
			if(rtc_dayofweek >= 7) rtc_dayofweek = 0;
			if(rtc_dd > rtc_month_days[rtc_mm]){
				if((rtc_mm ==1) && (rtc_dd == 29)) {
					if((rtc_yy % 4) == 0) return;
				}
				rtc_dd = 1;
				rtc_mm++;
				if(rtc_mm >= 12) {
					rtc_yy++;
					rtc_mm = 0;
					if(rtc_yy >= 100) rtc_yy = 0;
				}
			}
		}
	}
}
void KEYBOARD::rtc_adjust(void)
{
}
#endif // FM77AV_VARIANTS

void KEYBOARD::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_FM7KEY_SET_INSLED) {
		write_signals(&ins_led, data & mask);
	}
#if defined(_FM77AV_VARIANTS)  
	 else if(id == SIG_FM7KEY_PUSH_TO_ENCODER) {
		/*
		 * I refered XM7's sourcecode : VM/keyboard.c act of key-encoder.
		 * Thanks to Ryu.Takegami and PI.
		 */
		int count;
		if(key_ack->read_signal(0x00) == 0) return; // If (not(ACK)) noop.

		if(cmd_fifo->full()) {
			cmd_fifo->clear();
		}
		if(cmd_fifo->empty()) {
			cmd_phase = data & 0xff;
		}
		
		cmd_fifo->write(data & 0xff);
		count = cmd_fifo->count();
		
		key_ack->write_signal(0x00, 0x00, 0x01);
		switch(cmd_phase) {
			case 0: // Set mode
				if(count >= 2) set_mode();
				break;
			case 1: // Get mode
				get_mode();
				break;
			case 2: // Set LED Phase
				if(count >= 2) set_leds();
				break;
			case 3: // Get LED Phase
				get_leds();
				break;
			case 4:
				if(count >= 2) set_repeat_type();
				break;
			case 5:
				if(count >= 3) set_repeat_time();
				break;
			case 0x80: // Communicate to/from RTC.
				if(count == 1) {
					rtc_set = false;
				}
				if(count == 2) {
					if((data & 0xff) == 0) { // Get
						get_rtc();
					} else if((data & 0xff) == 1) { // Set
						rtc_set_flag = true;
					} else { // Illegal
						cmd_fifo->clear(); 
					}
				}
				if(rtc_set_flag) {
					if(count >= 9) {
						set_rtc();
					}
				}
				break;
			case 0x81: // Digitize.
				if(count >= 2) do_digitize(); // WILL Implement?
				break;
			case 0x82:
				if(count >= 2) set_screen_mode();
				break;
			case 0x83:
				get_screen_mode();
				break;
			case 0x84:
				if(count >= 2) set_brightness();
				break;
			default:
				//cmd_fifo->clear();
				break;
		}
		register_event(this, ID_KEYBOARD_ACK, 5, false, NULL); // Delay 5us until ACK is up.
	}
#endif
}

uint32 KEYBOARD::read_data8(uint32 addr)
{
	uint32 retval = 0xff;
	switch(addr) {
		case 0x00:
			retval = (keycode_7 >> 8) & 0x80;
			break;
		case 0x01:
			retval = keycode_7 & 0xff;
			break;
#if defined(_FM77AV_VARIANTS)			
		case 0x31:
			retval = read_data_reg();
			break;
		case 0x32:
			retval = read_stat_reg();
			break;
#endif
		default:
			break;
	}
	return retval;
}

void KEYBOARD::write_data8(uint32 addr, uint32 data)
{
	switch(addr) {
#if defined(_FM77AV_VARIANTS)			
		case 0x31:
			this->write_signal(SIG_FM7KEY_PUSH_TO_ENCODER, data, 0x000000ff);
			break;
#endif
	}
}

KEYBOARD::KEYBOARD(VM *parent_vm, EMU *parent_emu) : DEVICE(parent_vm, parent_emu)
{
	int i;
	p_vm = parent_vm;
	p_emu = parent_emu;
  
	keycode_7 = 0;
   
	ctrl_pressed = false; 
	lshift_pressed = false; 
	rshift_pressed = false; 
	shift_pressed = false; 
	graph_pressed = false;
	caps_pressed = false;
	kana_pressed = false;
	break_pressed = false;
	
	for(i = 0; i < 0x70; i++) {
		event_ids[i] = 0;
		key_pressed_flag[i] = false;
	}
   
	cmd_fifo = new FIFO(16);
	data_fifo = new FIFO(16);
	keymode = KEYMODE_STANDARD;
	
	init_output_signals(&rxrdy);
	init_output_signals(&key_ack);
	
	init_output_signals(&break_line);
	
	init_output_signals(&kana_led);
	init_output_signals(&caps_led);
	init_output_signals(&ins_led);

	
}

KEYBOARD::~KEYBOARD()
{
}

   
   
