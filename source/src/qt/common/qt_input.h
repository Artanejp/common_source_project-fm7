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
   VK_F14    = 0x7d,
   VK_F15    = 0x7e,
   VK_BACK   = 0x08,
   VK_TAB    = 0x09,
   VK_CLEAR  = 0x0c,
   VK_RETURN = 0x0d,
   VK_PAUSE  = 0x13,
   VK_ESCAPE = 0x1b,
   VK_SPACE  = 0x20,
   VK_DELETE = 0x2e,
   VK_UP     = 0x26,
   VK_DOWN   = 0x28,
   VK_RIGHT  = 0x27,
   VK_LEFT   = 0x25,
   VK_INSERT = 0x2d,
   VK_HOME = 0x24,
   VK_END = 0x23,
   VK_PRIOR = 0x21,
   VK_NEXT = 0x22,
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
   VK_DECIMAL = 0x6e,
   VK_DIVIDE = 0x6f,
   VK_MULTIPLY = 0x6a,
   VK_SUBTRACT = 0x6d,
   VK_ADD = 0x6b,
   VK_NUMLOCK = 0x90,
   VK_CAPITAL = 0x14,
   VK_SCROLL = 0x91,
   VK_SHIFT = 0x10,
   VK_LSHIFT = 0xa0,
   VK_RSHIFT = 0xa1,
   VK_LCONTROL = 0xa2,
   VK_RCONTROL = 0xa3,
   VK_RMENU = 0xa5,
   VK_LMENU = 0xa4,
   VK_RWIN = 0x5c,
   VK_LWIN = 0x5b,
   VK_HELP = 0x2f,	
   VK_PRINT = 0x2a,
   VK_SNAPSHOT = 0x2c,
   VK_CANCEL = 0x03,
   VK_APPS = 0x5d,
   VK_KANA = 0x15,
   VK_KANJI = 0x19,
   VK_CONTROL = 0x11,
   VK_MENU = 0x12
};

   

struct QtKeyTable {
   uint32_t vk;
   enum Qt::Key qtkey;
};
     uint32_t GetAsyncKeyState(uint32_t vk, uint32_t mod);
     uint8_t convert_AGKey2VK(uint32_t sym);
   
extern const struct QtKeyTable  QtKeyMappings[];
#ifdef __cplusplus
}
#endif
#endif

