/*
 * 
 */

#ifndef _AGAR_INPUT_H_
#define _AGAR_INPUT_H_

#include <Qt>
#include <QKeyEvent>

#ifdef __cplusplus
extern "C" {
#endif
   // Include from gui/drv_wgl_keymaps.h , libagar.
   enum {
	VK_LBUTTON = 0x01,
	VK_RBUTTON = 0x02,
	
	VK_CANCEL = 0x03,
	VK_MBUTTON = 0x04,
	
	VK_BACK   = 0x08,
	VK_TAB    = 0x09,
	VK_CLEAR  = 0x0c,
	VK_RETURN = 0x0d,
	
	VK_SHIFT   = 0x10,
	VK_CONTROL = 0x11,
	VK_MENU    = 0x12,
	VK_PAUSE   = 0x13,
	VK_CAPITAL = 0x14,
	VK_KANA    = 0x15,
	VK_HANGEUL = 0x15,
	VK_HANGUL  = 0x15,
	VL_JUNJA   = 0x17,
	VK_FINAL   = 0x18,
	VK_KANJI   = 0x19,
	VK_HANJA   = 0x19,
	VK_ESCAPE  = 0x1b,
	VK_CONVERT = 0x1c,
	VK_NONCONVERT = 0x1d,
	VK_ACCEPT  = 0x1e,
	VK_MODECHANGE =	0x1f,	
	
	VK_SPACE  = 0x20,
	VK_PRIOR  = 0x21,
	VK_NEXT   = 0x22,
	VK_END    = 0x23,
	VK_HOME   = 0x24,
	VK_LEFT   = 0x25,
	VK_UP     = 0x26,
	VK_RIGHT  = 0x27,
	VK_DOWN   = 0x28,
	VK_SELECT = 0x29,
	VK_PRINT  = 0x2a,
	VK_EXECUTE  = 0x2b,
	VK_SNAPSHOT = 0x2c,
	VK_INSERT = 0x2d,
	VK_DELETE = 0x2e,
	VK_HELP   = 0x2f,

	VK_LWIN = 0x5b,
	VK_RWIN = 0x5c,
	VK_APPS = 0x5d,

	VK_NUMPAD0 = 0x60,
	VK_NUMPAD1 = 0x61,
	VK_NUMPAD2 = 0x62,
	VK_NUMPAD3 = 0x63,
	VK_NUMPAD4 = 0x64,
	VK_NUMPAD5 = 0x65,
	VK_NUMPAD6 = 0x66,
	VK_NUMPAD7 = 0x67,
	VK_NUMPAD8 = 0x68,
	VK_NUMPAD9 = 0x69,
	VK_MULTIPLY = 0x6a,
	VK_ADD = 0x6b,
	VK_SEPARATOR = 0x6c, 
	VK_SUBTRACT = 0x6d,
	VK_DECIMAL = 0x6e,
	VK_DIVIDE = 0x6f,
	
	VK_F1   = 0x70,
	VK_F2   = 0x71,
	VK_F3   = 0x72,
	VK_F4   = 0x73,
	VK_F5   = 0x74,
	VK_F6   = 0x75,
	VK_F7   = 0x76,
	VK_F8   = 0x77,
	VK_F9   = 0x78,
	VK_F10  = 0x79,
	VK_F11  = 0x7a,
	VK_F12  = 0x7b,
	VK_F13  = 0x7c,
	VK_F14  = 0x7d,
	VK_F15  = 0x7e,
	VK_F16  = 0x7f,
	VK_F17  = 0x80,
	VK_F18  = 0x81,
	VK_F19  = 0x82,
	VK_F20  = 0x83,
	VK_F21  = 0x84,
	VK_F22 	= 0x85,
	VK_F23 	= 0x86,
	VK_F24 	= 0x87,
	
	VK_NUMLOCK = 0x90,
	VK_SCROLL = 0x91,
	
	VK_LSHIFT = 0xa0,
	VK_RSHIFT = 0xa1,
	VK_LCONTROL = 0xa2,
	VK_RCONTROL = 0xa3,
	VK_LMENU = 0xa4,
	VK_RMENU = 0xa5,

	VK_$B8   = 0xb8,
	VK_$B9   = 0xb9,
	VK_OEM_1 = 0xba, // :
	VK_OEM_PLUS   = 0xbb, // ;
	VK_OEM_COMMA  = 0xbc, // ,
	VK_OEM_MINUS  = 0xbd, // -^
	VK_OEM_PERIOD = 0xbe,
	VK_OEM_2 = 0xbf, // Slash
	VK_OEM_3 = 0xc0, // @

	VK_OEM_4 = 0xdb,
	VK_OEM_5 = 0xdc,
	VK_OEM_6 = 0xdd,
	VK_OEM_7 = 0xde,
	VK_OEM_8 = 0xdf,

	VK_OEM_102 = 0xe2, // Back Slash
	VK_PROCESSKEY =	0xe5,
	
	VK_OEM_ATTN = 0xf0, //	Caps Lock 	　
	VK_OEM_FINISH = 0xf1,
	VK_OEM_COPY = 0xf2, // Katakana/Hiragana/Romaji
	VK_OEM_AUTO = 0xf3, // Hankaku / Zenkaku / Kanji (1)
	VK_OEM_ENLW = 0xf4, // Hankaku / Zenkaku / Kanji (2)
	VK_OEM_BACKTAB = 0xf5,
	VK_ATTN   =	0xf6,
	VK_CRSEL  =	0xf7,
	VK_EXSEL  =	0xf8,
	VK_EREOF  =	0xf9,
	VK_PLAY   =	0xfa,
	VK_ZOOM   =	0xfb,
	VK_NONAME =	0xfc,
	VK_PA1 	  = 0xfd,
	VK_OME_CLEAR = 0xfe, 
};

# if !defined(Q_OS_WIN32)
uint32_t GetAsyncKeyState(uint32_t vk, uint32_t mod);
# endif   
uint8_t convert_AGKey2VK(uint32_t sym);
   
#ifdef __cplusplus
}
#endif
#endif

