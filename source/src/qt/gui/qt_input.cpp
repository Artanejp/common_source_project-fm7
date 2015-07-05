/*
 *	Skelton for retropc emulator
 *
 *	Author : Takeda.Toshiya
 *	Date   : 2006.08.18 -
 *      Converted to QT by (C) 2015 K.Ohta
 *         History:
 *            Jan 12, 2015 (maybe) : Initial
 *	[ Qt input -> Keyboard]
*/

#include <Qt>
#include <QKeyEvent>
#include "emu.h"
#include "vm/vm.h"
#include "config.h"

//#include "fifo.h"
//#include "fileio.h"

#include "qt_input.h"
#include "qt_gldraw.h"
#include "qt_main.h"
#include "menuclasses.h"
#include "agar_logger.h"

#ifndef Ulong
#define Ulong unsigned long
#endif

#define KEY_KEEP_FRAMES 3

static int mouse_x = 0;
static int mouse_y = 0;
static int mouse_relx = 0;
static int mouse_rely = 0;
static uint32 mouse_buttons = 0;
   
struct NativeScanCode {
	uint32 vk;
	uint32 scan;
};
struct NativeVirtualKeyCode {
	uint32 vk;
	uint32 key;
};

struct NativeScanCode convTable_QTScan106[] = {
	// Line 0
	{VK_ESCAPE,  9},
	{VK_F1,  67},
	{VK_F2,  68},
	{VK_F3,  69},
	{VK_F4,  70},
	{VK_F5,  71},
	{VK_F6,  72},
	{VK_F7,  73},
	{VK_F8,  74},
	{VK_F9,  75},
	{VK_F10, 76},
	{VK_F11, 95},
	{VK_F12, 96},
	// Power, Sleep, Wake is not implemented, they are'nt safety.
	// Line 1
	{VK_KANJI, 49}, // Hankaku/Zenkaku
	{'0', 19},
	{'1', 10},
	{'2', 11},
	{'3', 12},
	{'4', 13},
	{'5', 14},
	{'6', 15},
	{'7', 16},
	{'8', 17},
	{'9', 18},
	{0xbd, 20}, // - =
	{0xde, 21}, // ^~
	{0xdc, 132}, // \|
	{VK_BACK, 22}, // Backspace
	// Line 2
	{VK_TAB, 23},
	{'Q', 24},
	{'W', 25},
	{'E', 26},
	{'R', 27},
	{'T', 28},
	{'Y', 29},
	{'U', 30},
	{'I', 31},
	{'O', 32},
	{'P', 33},
	{0xc0, 34}, // @
	{VK_RETURN, 36}, // Enter (Full key)
	{0xdb, 35}, // [
	// Line 3
	{0xf0, 66}, // CAPS Lock
	{'A', 38},
	{'S', 39},
	{'D', 40},
	{'F', 41},
	{'G', 42},
	{'H', 43},
	{'J', 44},
	{'K', 45},
	{'L', 46},
	{0xbb, 47}, // ;
	{0xba, 48}, // :
	{0xdd, 51}, // ]
	// Line 3
	{VK_LSHIFT, 50}, // LShift
	{'Z', 52},
	{'X', 53},
	{'C', 54},
	{'V', 55},
	{'B', 56},
	{'N', 57},
	{'M', 58},
	{0xbc, 59}, // ,
	{0xbe, 60}, // .
	{0xbf, 61}, // /(Slash)
	{0xe2, 97}, //\_
	{VK_RSHIFT, 62},
	// Line 4
	{VK_LCONTROL, 37},
	{VK_LWIN, 133},
	{VK_LMENU, 64},
	{0x1d, 102}, // Muhenkan
	{VK_SPACE, 65},
	{0xf3, 100}, // Henkan
	{0xf2, 101}, // Katakana_Hiragana
	{VK_RMENU, 108},
	{VK_RWIN,  134},
	{VK_APPS, 135},
	{VK_RCONTROL, 105},
	// Cursors
	{VK_UP, 111},
	{VK_DOWN, 116},
	{VK_LEFT, 113},
	{VK_RIGHT,114},
	// 
	//     {VK_PRINT, },
	{VK_SCROLL, 78},
	{VK_PAUSE, 127},
	{VK_INSERT, 118},
	{VK_HOME, 110},
	{VK_NEXT, 112},
	{VK_DELETE, 119},
	{VK_END, 115},
	{VK_PRIOR, 117},
	// TenKey
	{VK_NUMPAD0, 90},
	{VK_NUMPAD1, 87},
	{VK_NUMPAD2, 88},
	{VK_NUMPAD3, 89},
	{VK_NUMPAD4, 83},
	{VK_NUMPAD5, 84},
	{VK_NUMPAD6, 85},
	{VK_NUMPAD7, 79},
	{VK_NUMPAD8, 80},
	{VK_NUMPAD9, 81},
	//
	{VK_DECIMAL, 77}, // NumLock     
	{VK_DIVIDE, 106},
	{VK_MULTIPLY, 63},
	{VK_SUBTRACT, 82},
	{VK_ADD, 86},
	{VK_RETURN, 104},  // Enter(ten Key)
	{0xffffffff, 0xffffffff}
};
#if defined(Q_OS_WIN32)
#else // Linux or Unix
const struct NativeVirtualKeyCode convTable_QTKey[] = {
	// Line 0
	{VK_ESCAPE,  0xff1b},
	{VK_F1,  0xffbe},
	{VK_F2,  0xffbf},
	{VK_F3,  0xffc0},
	{VK_F4,  0xffc1},
	{VK_F5,  0xffc2},
	{VK_F6,  0xffc3},
	{VK_F7,  0xffc4},
	{VK_F8,  0xffc5},
	{VK_F9,  0xffc6},
	{VK_F10, 0xffc7},
	{VK_F11, 0xffc8},
	{VK_F12, 0xffc9},
	// Power, Sleep, Wake is not implemented, they are'nt safety.
	// Line 1
	{VK_KANJI, 0xff2a}, // Hankaku/Zenkaku
	{0xbd, 0x002d}, // -=
	{0xbd, 0x003d},
	{0xde, 0x005e}, // ^~
	{0xde, 0x007e},
	{0xdc, 0x005c}, // \|
	{0xdc, 0x007c},
	{VK_BACK, 0xff08}, // Backspace
	// Line 2
	{VK_TAB, 0xff09},
	{VK_RETURN, 0xff0d}, // Enter (Full key)
	{0xdb, 0x005b}, // [
	{0xdb, 0x007b}, // [
	// Line 3
	{0xf0, 0xff30}, // CAPS Lock
	{0xbb, 0x002b}, // ;
	{0xbb, 0x003b}, // ;
	{0xba, 0x002a}, // :
	{0xba, 0x003a}, // :
	{0xdd, 0x005d}, // ]
	{0xdd, 0x007d}, // ]
	// Line 3
	{VK_LSHIFT, 0xffe1}, // LShift
	{0xbc, 0x2c}, // ,
	{0xbc, 0x3c}, // ,
	{0xbe, 0x2e}, // .
	{0xbe, 0x3e}, // .
	{0xbf, 0x2f}, // /(Slash)
	{0xbf, 0x3f}, // /(Slash)
	{0xe2, 0x5f}, //\_
	//{0xe2, 0x5c}, //\_
	{VK_RSHIFT, 0xffe2},
	// Line 4
	{VK_LCONTROL, 0xffe3},
	{VK_LWIN, 0xffeb},
	{VK_LMENU, 0xffe9},
	{0x1d, 0xff22}, // Muhenkan
	{VK_SPACE, 0x0020},
	{0xf3, 0xff23}, // Henkan
	{0xf2, 0xff27}, // Katakana_Hiragana
	{VK_RMENU, 0xffea},
	{VK_RWIN,  0xffec},
	{VK_APPS, 0xff67},
	{VK_RCONTROL, 0xffe4},
	// Cursors
	{VK_UP, 0xff52},
	{VK_DOWN, 0xff54},
	{VK_LEFT, 0xff51},
	{VK_RIGHT,0xff53},
	// 
	//     {VK_PRINT, },
	{VK_SCROLL, 0xff14},
	{VK_PAUSE, 0xff13},
	{VK_INSERT, 0xff63},
	{VK_HOME, 0xff50},
	{VK_NEXT, 0xff55},
	{VK_DELETE, 0xffff},
	{VK_END, 0xff57},
	{VK_PRIOR, 0xff56},
	// TenKey
	{VK_NUMPAD0, 0xffb0},
	{VK_NUMPAD1, 0xffb1},
	{VK_NUMPAD2, 0xffb2},
	{VK_NUMPAD3, 0xffb3},
	{VK_NUMPAD4, 0xffb4},
	{VK_NUMPAD5, 0xffb5},
	{VK_NUMPAD6, 0xffb6},
	{VK_NUMPAD7, 0xffb7},
	{VK_NUMPAD8, 0xffb8},
	{VK_NUMPAD9, 0xffb9},
	//
	{VK_DECIMAL, 0xff7f}, // NumLock     
	{VK_DIVIDE, 0xffaf},
	{VK_MULTIPLY, 0xffaa},
	{VK_SUBTRACT, 0xffad},
	{VK_ADD, 0xffab},
	{VK_RETURN, 0xff8d},  // Enter(ten Key)
	{VK_DECIMAL, 0xffae},  // Period(ten Key)
	{0xffffffff, 0xffffffff}
};
#endif

#if defined(Q_OS_WIN32)
uint32_t GLDrawClass::getNativeKey2VK(uint32_t data)
{
	uint32_t vk = data;
#if defined(ENABLE_SWAP_KANJI_PAUSE)
	if(config.swap_kanji_pause) {
		if(vk == VK_KANJI) {
			vk = VK_PAUSE;
		} else if(vk == VK_PAUSE) {
			vk = VK_KANJI;
		}
	}
#endif
	return vk;
}
#else
uint32_t GLDrawClass::getNativeKey2VK(uint32_t data)
{
	uint32 val = 0;
	uint32 vk;
	int i = 0;

	if(data < 0x100) {
		if((data >= 0x60) && (data <= 'z')) {
	  		return data & 0x5f;
		}
		if((data >= 0x40) && (data <= 'Z')) {
	  		return data;
		}
		if((data >= 0x30) && (data <= 0x39)) {
	  		return data;
		}
		if((data > 0x20) && (data <= 0x29)) {
	  		return data | 0x10;
		}
	}
	while(convTable_QTKey[i].vk != 0xffffffff) {
		val = convTable_QTKey[i].key;
		if(val == data) break;
		i++;
	}
	vk = convTable_QTKey[i].vk;
	//printf("SCAN=%02x VK=%02x\n", val, vk);
	if(vk == 0xffffffff) return 0;
#if defined(ENABLE_SWAP_KANJI_PAUSE)
	if(config.swap_kanji_pause) {
		if(vk == VK_KANJI) {
			vk = VK_PAUSE;
		} else if(vk == VK_PAUSE) {
			vk = VK_KANJI;
		}
	}
#endif	   
//	if(vk == VK_KANJI) vk = 0xf3;
	if((vk == VK_LSHIFT) || (vk == VK_RSHIFT)) vk = VK_SHIFT;
	if((vk == VK_LCONTROL) || (vk == VK_RCONTROL)) vk = VK_CONTROL;
	if((vk == VK_LMENU) || (vk == VK_RMENU)) vk = VK_MENU;
	return vk;
}
#endif

uint32_t GLDrawClass::get106Scancode2VK(uint32_t data)
{
	uint32 val = 0;
	uint32 vk;
	int i = 0;
	while(convTable_QTScan106[i].vk != 0xffffffff) {
		val = convTable_QTScan106[i].scan;
		if(val == data) break;
		i++;
	}
	vk = convTable_QTScan106[i].vk;
	//printf("SCAN=%02x VK=%02x\n", val, vk);
	if(vk == 0xffffffff) return 0;
#if defined(ENABLE_SWAP_KANJI_PAUSE)
	if(config.swap_kanji_pause) {
		if(vk == VK_KANJI) {
			vk = VK_PAUSE;
		} else if(vk == VK_PAUSE) {
			vk = VK_KANJI;
		}
	}
#endif	   
//	if(vk == VK_KANJI) vk = 0xf3;
	if((vk == VK_LSHIFT) || (vk == VK_RSHIFT)) vk = VK_SHIFT;
	if((vk == VK_LCONTROL) || (vk == VK_RCONTROL)) vk = VK_CONTROL;
	if((vk == VK_LMENU) || (vk == VK_RMENU)) vk = VK_MENU;
	return vk;
}

void GLDrawClass::keyReleaseEvent(QKeyEvent *event)
{
	int key = event->key();
	uint32 mod = event->modifiers();
	uint32 scan;
	uint32 vk;
	if(event->isAutoRepeat()) return;
#if 0
	scan = event->nativeScanCode();
	vk = get106Scancode2VK(scan);
#else
	scan = event->nativeVirtualKey();
	vk = getNativeKey2VK(scan);
#endif	
	//printf("Key: VK=%d SCAN=%04x MOD=%08x\n", vk, scan, mod);
	emu->LockVM();
	emu->key_mod(mod);
	// Note: Qt4 with 106KEY, event->modifier() don't get Shift key as KEYMOD.
	// At least, linux.
//#ifdef NOTIFY_KEY_DOWN
	if(vk != 0) {
		emu->key_up(vk);
	}
//#endif
	emu->UnlockVM();
}

void GLDrawClass::keyPressEvent(QKeyEvent *event)
{
	int key = event->key();
	uint32 mod = event->modifiers();;
	uint32 scan;
	uint32 vk;
   
	if(event->isAutoRepeat()) return;
#if 0
	scan = event->nativeScanCode();
	vk = get106Scancode2VK(scan);
#else
	scan = event->nativeVirtualKey();
	vk = getNativeKey2VK(scan);
#endif
	if(vk == VK_APPS) { // Special key : capture/uncapture mouse.
		emit sig_toggle_mouse();
		return;
	}
   
	//printf("Key: VK=%d SCAN=%04x MOD=%08x\n", vk, scan, mod);
	emu->LockVM();
	emu->key_mod(mod);
//#ifdef NOTIFY_KEY_DOWN
	if(vk != 0) {
		emu->key_down(vk, false);
	}
//#endif
	emu->UnlockVM();
}


extern "C"{   
uint32_t GetAsyncKeyState(uint32_t vk, uint32_t mod)
{
	vk = vk & 0xff; // OK?
	quint32 modstate = mod;
   //printf("Mod %d %08x\n", vk, mod);
#if 1
	switch(vk) {
	case VK_SHIFT:
		if((modstate & Qt::ShiftModifier) != 0) return 0xffffffff;
		break;
	case VK_LSHIFT:
		if((modstate & Qt::ShiftModifier) != 0) return 0xffffffff;
		break;
	case VK_RSHIFT:
		if((modstate & Qt::ShiftModifier) != 0) return 0xffffffff;
		break;
	case VK_CONTROL:
		if((modstate & Qt::ControlModifier) != 0) return 0xffffffff;
		break;
	case VK_LCONTROL:
		if((modstate & Qt::ControlModifier) != 0) return 0xffffffff;
		break;
	case VK_RCONTROL:
		if((modstate & Qt::ControlModifier) != 0) return 0xffffffff;
		break;
	case VK_LMENU:
		if((modstate & Qt::AltModifier) != 0) return 0xffffffff;
		break;
	case VK_RMENU:
		if((modstate & Qt::AltModifier) != 0) return 0xffffffff;
		break;
	default:
		break;
	}
#endif
	return 0;
}


}


