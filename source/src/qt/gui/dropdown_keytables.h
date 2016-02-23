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

static const _TCHAR *vk_names[] = {
	_T("VK_$00"),			_T("VK_LBUTTON"),		_T("VK_RBUTTON"),		_T("VK_CANCEL"),		
	_T("VK_MBUTTON"),		_T("VK_XBUTTON1"),		_T("VK_XBUTTON2"),		_T("VK_$07"),			
	_T("VK_BACK"),			_T("VK_TAB"),			_T("VK_$0A"),			_T("VK_$0B"),			
	_T("VK_CLEAR"),			_T("VK_RETURN"),		_T("VK_$0E"),			_T("VK_$0F"),			
	_T("VK_SHIFT"),			_T("VK_CONTROL"),		_T("VK_MENU"),			_T("VK_PAUSE"),			
	_T("VK_CAPITAL"),		_T("VK_KANA"),			_T("VK_$16"),			_T("VK_JUNJA"),			
	_T("VK_FINAL"),			_T("VK_KANJI"),			_T("VK_$1A"),			_T("VK_ESCAPE"),		
	_T("VK_CONVERT"),		_T("VK_NONCONVERT"),		_T("VK_ACCEPT"),		_T("VK_MODECHANGE"),		
	_T("VK_SPACE"),			_T("VK_PRIOR"),			_T("VK_NEXT"),			_T("VK_END"),			
	_T("VK_HOME"),			_T("VK_LEFT"),			_T("VK_UP"),			_T("VK_RIGHT"),			
	_T("VK_DOWN"),			_T("VK_SELECT"),		_T("VK_PRINT"),			_T("VK_EXECUTE"),		
	_T("VK_SNAPSHOT"),		_T("VK_INSERT"),		_T("VK_DELETE"),		_T("VK_HELP"),			
	_T("VK_0"),			_T("VK_1"),			_T("VK_2"),			_T("VK_3"),			
	_T("VK_4"),			_T("VK_5"),			_T("VK_6"),			_T("VK_7"),			
	_T("VK_8"),			_T("VK_9"),			_T("VK_$3A"),			_T("VK_$3B"),			
	_T("VK_$3C"),			_T("VK_$3D"),			_T("VK_$3E"),			_T("VK_$3F"),			
	_T("VK_$40"),			_T("VK_A"),			_T("VK_B"),			_T("VK_C"),			
	_T("VK_D"),			_T("VK_E"),			_T("VK_F"),			_T("VK_G"),			
	_T("VK_H"),			_T("VK_I"),			_T("VK_J"),			_T("VK_K"),			
	_T("VK_L"),			_T("VK_M"),			_T("VK_N"),			_T("VK_O"),			
	_T("VK_P"),			_T("VK_Q"),			_T("VK_R"),			_T("VK_S"),			
	_T("VK_T"),			_T("VK_U"),			_T("VK_V"),			_T("VK_W"),			
	_T("VK_X"),			_T("VK_Y"),			_T("VK_Z"),			_T("VK_LWIN"),			
	_T("VK_RWIN"),			_T("VK_APPS"),			_T("VK_$5E"),			_T("VK_SLEEP"),			
	_T("VK_NUMPAD0"),		_T("VK_NUMPAD1"),		_T("VK_NUMPAD2"),		_T("VK_NUMPAD3"),		
	_T("VK_NUMPAD4"),		_T("VK_NUMPAD5"),		_T("VK_NUMPAD6"),		_T("VK_NUMPAD7"),		
	_T("VK_NUMPAD8"),		_T("VK_NUMPAD9"),		_T("VK_MULTIPLY"),		_T("VK_ADD"),			
	_T("VK_SEPARATOR"),		_T("VK_SUBTRACT"),		_T("VK_DECIMAL"),		_T("VK_DIVIDE"),		
	_T("VK_F1"),			_T("VK_F2"),			_T("VK_F3"),			_T("VK_F4"),			
	_T("VK_F5"),			_T("VK_F6"),			_T("VK_F7"),			_T("VK_F8"),			
	_T("VK_F9"),			_T("VK_F10"),			_T("VK_F11"),			_T("VK_F12"),			
	_T("VK_F13"),			_T("VK_F14"),			_T("VK_F15"),			_T("VK_F16"),			
	_T("VK_F17"),			_T("VK_F18"),			_T("VK_F19"),			_T("VK_F20"),			
	_T("VK_F21"),			_T("VK_F22"),			_T("VK_F23"),			_T("VK_F24"),			
	_T("VK_$88"),			_T("VK_$89"),			_T("VK_$8A"),			_T("VK_$8B"),			
	_T("VK_$8C"),			_T("VK_$8D"),			_T("VK_$8E"),			_T("VK_$8F"),			
	_T("VK_NUMLOCK"),		_T("VK_SCROLL"),		_T("VK_$92"),			_T("VK_$93"),			
	_T("VK_$94"),			_T("VK_$95"),			_T("VK_$96"),			_T("VK_$97"),			
	_T("VK_$98"),			_T("VK_$99"),			_T("VK_$9A"),			_T("VK_$9B"),			
	_T("VK_$9C"),			_T("VK_$9D"),			_T("VK_$9E"),			_T("VK_$9F"),			
	_T("VK_LSHIFT"),		_T("VK_RSHIFT"),		_T("VK_LCONTROL"),		_T("VK_RCONTROL"),		
	_T("VK_LMENU"),			_T("VK_RMENU"),			_T("VK_BROWSER_BACK"),		_T("VK_BROWSER_FORWARD"),	
	_T("VK_BROWSER_REFRESH"),	_T("VK_BROWSER_STOP"),		_T("VK_BROWSER_SEARCH"),	_T("VK_BROWSER_FAVORITES"),	
	_T("VK_BROWSER_HOME"),		_T("VK_VOLUME_MUTE"),		_T("VK_VOLUME_DOWN"),		_T("VK_VOLUME_UP"),		
	_T("VK_MEDIA_NEXT_TRACK"),	_T("VK_MEDIA_PREV_TRACK"),	_T("VK_MEDIA_STOP"),		_T("VK_MEDIA_PLAY_PAUSE"),	
	_T("VK_LAUNCH_MAIL"),		_T("VK_LAUNCH_MEDIA_SELECT"),	_T("VK_LAUNCH_APP1"),		_T("VK_LAUNCH_APP2"),		
	_T("VK_$B8"),			_T("VK_$B9"),			_T("VK_OEM_1"),			_T("VK_OEM_PLUS"),		
	_T("VK_OEM_COMMA"),		_T("VK_OEM_MINUS"),		_T("VK_OEM_PERIOD"),		_T("VK_OEM_2"),			
	_T("VK_OEM_3"),			_T("VK_$C1"),			_T("VK_$C2"),			_T("VK_$C3"),			
	_T("VK_$C4"),			_T("VK_$C5"),			_T("VK_$C6"),			_T("VK_$C7"),			
	_T("VK_$C8"),			_T("VK_$C9"),			_T("VK_$CA"),			_T("VK_$CB"),			
	_T("VK_$CC"),			_T("VK_$CD"),			_T("VK_$CE"),			_T("VK_$CF"),			
	_T("VK_$D0"),			_T("VK_$D1"),			_T("VK_$D2"),			_T("VK_$D3"),			
	_T("VK_$D4"),			_T("VK_$D5"),			_T("VK_$D6"),			_T("VK_$D7"),			
	_T("VK_$D8"),			_T("VK_$D9"),			_T("VK_$DA"),			_T("VK_OEM_4"),			
	_T("VK_OEM_5"),			_T("VK_OEM_6"),			_T("VK_OEM_7"),			_T("VK_OEM_8"),			
	_T("VK_$E0"),			_T("VK_OEM_AX"),		_T("VK_OEM_102"),		_T("VK_ICO_HELP"),		
	_T("VK_ICO_00"),		_T("VK_PROCESSKEY"),		_T("VK_ICO_CLEAR"),		_T("VK_PACKET"),		
	_T("VK_$E8"),			_T("VK_OEM_RESET"),		_T("VK_OEM_JUMP"),		_T("VK_OEM_PA1"),		
	_T("VK_OEM_PA2"),		_T("VK_OEM_PA3"),		_T("VK_OEM_WSCTRL"),		_T("VK_OEM_CUSEL"),		
	_T("VK_OEM_ATTN"),		_T("VK_OEM_FINISH"),		_T("VK_OEM_COPY"),		_T("VK_OEM_AUTO"),		
	_T("VK_OEM_ENLW"),		_T("VK_OEM_BACKTAB"),		_T("VK_ATTN"),			_T("VK_CRSEL"),			
	_T("VK_EXSEL"),			_T("VK_EREOF"),			_T("VK_PLAY"),			_T("VK_ZOOM"),			
	_T("VK_NONAME"),		_T("VK_PA1"),			_T("VK_OEM_CLEAR"),		_T("VK_$FF"),			
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
