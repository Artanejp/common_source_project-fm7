/*
 * Common Source Project/ Qt
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  Qt: Menu->Emulator->Define Strings
 *  History: Feb 23, 2016 : Initial
 */

#ifndef _CSP_QT_DROPDOWN_KEYTABLES_H
#define _CSP_QT_DROPDOWN_KEYTABLES_H

#include "common.h"
#include "qt_input.h"

typedef struct {
	uint32 vk;
	uint32 scan;
	const char *name;
} keydef_table_t;

enum {
	KEYBOARD_109JP = 0,
	KEYBOARD_84,
};


static const keydef_table_t default_key_table_106_QtScan[] = {
	{VK_ESCAPE,  9, "ESC"},
	{VK_F1,  67, "PF1"},
	{VK_F2,  68, "PF2"},
	{VK_F3,  69, "PF3"},
	{VK_F4,  70, "PF4"},
	{VK_F5,  71, "PF5"},
	{VK_F6,  72, "PF6"},
	{VK_F7,  73, "PF7"},
	{VK_F8,  74, "PF8"},
	{VK_F9,  75, "PF9"},
	{VK_F10, 76, "PF10"},
	{VK_F11, 95, "PF11"},
	{VK_F12, 96, "PF12"},
	// Power, Sleep, Wake is not implemented, they are'nt safety.
	// Line 1
	{VK_KANJI, 49, "KANJI"}, // Hankaku/Zenkaku
	{'0', 19, "0 (Full Key)"},
	{'1', 10, "1 (Full Key)"},
	{'2', 11, "2 (Full Key)"},
	{'3', 12, "3 (Full Key)"},
	{'4', 13, "4 (Full Key)"},
	{'5', 14, "5 (Full Key)"},
	{'6', 15, "6 (Full Key)"},
	{'7', 16, "7 (Full Key)"},
	{'8', 17, "8 (Full Key)"},
	{'9', 18, "9 (Full Key)"},
	{VK_OEM_MINUS, 20, "- (Full Key)"}, // - =
	{VK_OEM_7, 21, "^"}, // ^~
	{VK_OEM_5, 132, "\\|"}, // \|
	{VK_BACK, 22, "Back Space"}, // Backspace
	// Line 2
	{VK_TAB, 23, "TAB"},
	{'Q', 24, "Q"},
	{'W', 25, "W"},
	{'E', 26, "E"},
	{'R', 27, "R"},
	{'T', 28, "T"},
	{'Y', 29, "Y"},
	{'U', 30, "U"},
	{'I', 31, "I"},
	{'O', 32, "O"},
	{'P', 33, "P"},
	{VK_OEM_3, 34, "@"}, // @
	{VK_RETURN, 36, "ENTER(Full Key)"}, // Enter (Full key)
	{VK_OEM_4, 35, "["}, // [
	// Line 3
	{VK_OEM_ATTN, 66, "Caps Lock"}, // CAPS Lock
	{'A', 38, "A"},
	{'S', 39, "S"},
	{'D', 40, "D"},
	{'F', 41, "F"},
	{'G', 42, "G"},
	{'H', 43, "H"},
	{'J', 44, "J"},
	{'K', 45, "K"},
	{'L', 46, "L"},
	{VK_OEM_PLUS, 47, ";"}, // ;
	{VK_OEM_1, 48, ":"}, // :
	{VK_OEM_6, 51, "]"}, // ]
	// Line 3
	{VK_LSHIFT, 50, "Left Shift"}, // LShift
	{'Z', 52, "Z"},
	{'X', 53, "X"},
	{'C', 54, "C"},
	{'V', 55, "V"},
	{'B', 56, "B"},
	{'N', 57, "N"},
	{'M', 58, "M"},
	{VK_OEM_COMMA, 59, "Comma(Full Key)"}, // ,
	{VK_OEM_PERIOD, 60, "Period"}, // .
	{VK_OEM_2, 61, "/(Full Key)"}, // /(Slash)
	{VK_OEM_102, 97,"\\_"}, //\_
	{VK_RSHIFT, 62, "Right Shift"},
	// Line 4
	{VK_LCONTROL, 37, "Left Control"},
	{VK_LWIN, 133, "Left Windows"},
	{VK_LMENU, 64, "Left Menu"},
	{VK_NONCONVERT, 102, "Muhenkan"}, // Muhenkan
	{VK_SPACE, 65, "Space"},
	{VK_CONVERT, 100, "Henkan"}, // Henkan
	{VK_OEM_COPY, 101, "Katakana/Hiragana"}, // Katakana_Hiragana
	{VK_RMENU, 108, "Right Menu"},
	{VK_RWIN,  134, "Right Windows"},
	{VK_APPS, 135, "Application"},
	{VK_RCONTROL, 105, "Right Control"},
	// Cursors
	{VK_UP, 111, "↑"},
	{VK_DOWN, 116, "↓"},
	{VK_LEFT, 113, "←"},
	{VK_RIGHT,114, "→"},
	// 
	//     {VK_PRINT, },
	{VK_SCROLL, 78, "Scroll Lock"},
	{VK_PAUSE, 127, "Pause/Break"},
	{VK_INSERT, 118, "Insert"},
	{VK_HOME, 110, "Home"},
	{VK_NEXT, 112, "Page UP"},
	{VK_DELETE, 119, "Delete"},
	{VK_END, 115, "End"},
	{VK_PRIOR, 117, "Page Down"},
	// TenKey
	{VK_NUMPAD0, 90, "0 (Num Pad)"},
	{VK_NUMPAD1, 87, "1 (Num Pad)"},
	{VK_NUMPAD2, 88, "2 (Num Pad)"},
	{VK_NUMPAD3, 89, "3 (Num Pad)"},
	{VK_NUMPAD4, 83, "4 (Num Pad)"},
	{VK_NUMPAD5, 84, "5 (Num Pad)"},
	{VK_NUMPAD6, 85, "6 (Num Pad)"},
	{VK_NUMPAD7, 79, "7 (Num Pad)"},
	{VK_NUMPAD8, 80, "8 (Num Pad)"},
	{VK_NUMPAD9, 81, "9 (Num Pad)"},
	//
	{VK_DECIMAL, 0x005b, "Num Lock"}, // NumLock     
	{VK_DIVIDE, 106,  "/ (Num Pad)"},
	{VK_MULTIPLY, 63, "* (Num Pad)"},
	{VK_SUBTRACT, 82, "- (Num Pad)"},
	{VK_ADD, 86,       "+ (Num Pad)"},
	{VK_RETURN, 104, "Enter (Num Pad)"},  // Enter(ten Key)
	{0xffffffff, 0xffffffff, NULL}
};
	
#endif // #ifndef _CSP_QT_DROPDOWN_KEYTABLES_H
