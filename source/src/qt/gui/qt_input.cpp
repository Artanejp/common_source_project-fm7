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
#include "fileio.h"

#include "qt_input.h"
#include "qt_gldraw.h"
#include "qt_main.h"
#include "menuclasses.h"
#include "agar_logger.h"

#ifndef Ulong
#define Ulong unsigned long
#endif

#define KEY_KEEP_FRAMES 3

#if defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
const struct NativeScanCode convTable_QTScan106[] = {
	// Line 0
	{VK_ESCAPE,  0x01},
	{VK_F1,  0x3b},
	{VK_F2,  0x3c},
	{VK_F3,  0x3d},
	{VK_F4,  0x3e},
	{VK_F5,  0x3f},
	{VK_F6,  0x40},
	{VK_F7,  0x41},
	{VK_F8,  0x42},
	{VK_F9,  0x43},
	{VK_F10, 0x44},
	{VK_F11, 0x57},
	{VK_F12, 0x58},
	// Power, Sleep, Wake is not implemented, they are'nt safety.
	// Line 1
	{VK_KANJI, 0x29}, // Hankaku/Zenkaku
	{'1', 0x02},
	{'2', 0x03},
	{'3', 0x04},
	{'4', 0x05},
	{'5', 0x06},
	{'6', 0x07},
	{'7', 0x08},
	{'8', 0x09},
	{'9', 0x0a},
	{'0', 0x0b},
	{VK_OEM_MINUS, 0x0c}, // - =
	{VK_OEM_7, 0x0d}, // ^~
	{VK_OEM_5, 0x7d}, // \|
	{VK_BACK, 0x0e}, // Backspace
	// Line 2
	{VK_TAB, 0x0f},
	{'Q', 0x10},
	{'W', 0x11},
	{'E', 0x12},
	{'R', 0x13},
	{'T', 0x14},
	{'Y', 0x15},
	{'U', 0x16},
	{'I', 0x17},
	{'O', 0x18},
	{'P', 0x19},
	{VK_OEM_3, 0x1a}, // @
	{VK_OEM_4, 0x1b}, // [
	{VK_RETURN, 0x1c}, // Enter (Full key)
	// Line 3
	{VK_OEM_ATTN, 0x3a}, // CAPS Lock
	{'A', 0x1e},
	{'S', 0x1f},
	{'D', 0x20},
	{'F', 0x21},
	{'G', 0x22},
	{'H', 0x23},
	{'J', 0x24},
	{'K', 0x25},
	{'L', 0x26},
	{VK_OEM_PLUS, 0x27}, // ;
	{VK_OEM_1, 0x28}, // :
	{VK_OEM_6, 0x2b}, // ]
	// Line 3
	{VK_LSHIFT, 0x2a}, // LShift
	{'Z', 0x2c},
	{'X', 0x2d},
	{'C', 0x2e},
	{'V', 0x2f},
	{'B', 0x30},
	{'N', 0x31},
	{'M', 0x32},
	{VK_OEM_COMMA, 0x33}, // ,
	{VK_OEM_PERIOD, 0x34}, // .
	{VK_OEM_2, 0x35}, // /(Slash)
	{VK_OEM_102, 0x73}, //\_
	{VK_RSHIFT, 0x36},
	// Line 4
	{VK_LCONTROL, 0x1d},
	{VK_LWIN, 0x015b},
	{VK_LMENU, 0x38},
	{VK_NONCONVERT, 0x68}, // Muhenkan
	{VK_SPACE, 0x39},
	{VK_CONVERT, 0x66}, // Henkan
	{VK_OEM_COPY, 0x70}, // Katakana_Hiragana
	{VK_RMENU, 0x0138},
	{VK_RWIN,  0x015c},
	{VK_APPS, 0x015d},
	{VK_RCONTROL, 0x011d},
	// Cursors
	{VK_UP, 0x0148},
	{VK_DOWN, 0x0150},
	{VK_LEFT, 0x014b},
	{VK_RIGHT,0x014d},
	// 
	//     {VK_PRINT, },
	{VK_SCROLL, 0x46},
	{VK_PAUSE, 0x45},
	{VK_INSERT, 0x0152},
	{VK_HOME, 0x0147},
	{VK_NEXT, 0x0149},
	{VK_DELETE, 0x0153},
	{VK_END, 0x014f},
	{VK_PRIOR, 0x0151},
	// TenKey
	{VK_NUMPAD0, 0x52},
	{VK_NUMPAD1, 0x4f},
	{VK_NUMPAD2, 0x50},
	{VK_NUMPAD3, 0x51},
	{VK_NUMPAD4, 0x4b},
	{VK_NUMPAD5, 0x4c},
	{VK_NUMPAD6, 0x4d},
	{VK_NUMPAD7, 0x47},
	{VK_NUMPAD8, 0x48},
	{VK_NUMPAD9, 0x49},
	//
	{VK_DECIMAL, 0x53}, // NumLock     
	{VK_DIVIDE, 0x0135},
	{VK_MULTIPLY, 0x37},
	{VK_SUBTRACT, 0x4a},
	{VK_ADD, 0x4e},
	{VK_RETURN, 0x011c},  // Enter(ten Key)
	{0xffffffff, 0xffffffff}
};
#endif

uint32_t GLDrawClass::get106Scancode2VK(uint32_t data)
{
	uint32_t val = 0;
	uint32_t vk;
	int i = 0;
	vk = key_table->get_vk_from_scan(data);
	if(config.swap_kanji_pause) {
		if(vk == VK_KANJI) {
			vk = VK_PAUSE;
		} else if(vk == VK_PAUSE) {
			vk = VK_KANJI;
		}
	}
	if(!using_flags.is_notify_key_down_lr_shift()) {
		if((vk == VK_LSHIFT) || (vk == VK_RSHIFT)) vk = VK_SHIFT;
		if((vk == VK_LMENU) || (vk == VK_RMENU)) vk = VK_MENU;
	}
	if((vk == VK_LCONTROL) || (vk == VK_RCONTROL)) vk = VK_CONTROL;
	return vk;
}

void GLDrawClass::initKeyCode(void)
{
   	int i;
	key_table = new CSP_KeyTables(this, default_key_table_106_QtScan);

	{
		// Replace only ScanCode
		FILEIO *fio = new FILEIO();
		std::string app_path2;
		// Read scan table.
		app_path2 = cpp_confdir + "scancode.cfg";
		if(fio->Fopen(app_path2.c_str(), FILEIO_READ_ASCII)) {
			char buf[512];
			memset(buf, 0x00, sizeof(buf));
			while(fio->Fgets(buf, 512) != NULL) {
				QString nstr;
				QStringList nlist;
				bool ok1, ok2;
				nstr = QString::fromUtf8(buf);
				nlist = nstr.split(",", QString::SkipEmptyParts);
				if(nlist.count() < 2) continue;
				uint32_t vk   = nlist.at(0).toULong(&ok1, 16);
				uint32_t scan = nlist.at(1).toULong(&ok2, 16);
				if((vk == 0) || (vk > 255)) continue;
				if(ok1 && ok2) {
					key_table->do_set_scan_code(vk, scan);
				}
			}
			fio->Fclose();
		}
		delete fio;
	}
}

void GLDrawClass::releaseKeyCode(void)
{
	// Replace only ScanCode
	int i;
	FILEIO *fio = new FILEIO();
	std::string app_path2;
	uint32_t scan;
	// Read scan table.

	app_path2 = cpp_confdir + "scancode.cfg";
	if(fio->Fopen(app_path2.c_str(), FILEIO_WRITE_ASCII)) {
		for(i = 0; i < 256; i++) {
			scan = key_table->get_scan_from_vk((uint32_t)i);
			if(scan >= 0xffffffff)  continue;
			fio->Fprintf("%02x,%08x\n", i, scan);
		}
		fio->Fclose();
	}
	if(key_table != NULL) {
		delete key_table;
		key_table = NULL;
	}
}

void GLDrawClass::keyReleaseEvent(QKeyEvent *event)
{
	int key = event->key();
	uint32_t mod = event->modifiers();
	uint32_t scan;
	uint32_t vk;
	if(event->isAutoRepeat()) return;
	scan = event->nativeScanCode();
	vk = get106Scancode2VK(scan);
#if defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)	
	if(using_flags.is_notify_key_down_lr_shift()) {
		if(vk == VK_SHIFT) {
			if((GetAsyncKeyState(VK_LSHIFT) & 0x8000) == 0) vk = VK_LSHIFT;
			if((GetAsyncKeyState(VK_RSHIFT) & 0x8000) == 0) vk = VK_RSHIFT;
		}
		if(vk == VK_MENU) {
			if(GetAsyncKeyState(VK_LMENU) & 0x8000) vk = VK_LMENU;
			if(GetAsyncKeyState(VK_RMENU) & 0x8000) vk = VK_RMENU;
		}
	}
#endif
	//printf("Key: UP: VK=%d SCAN=%04x MOD=%08x\n", vk, scan, mod);
	emu->lock_vm();
	emu->key_modifiers(mod);
	// Note: Qt4 with 106KEY, event->modifier() don't get Shift key as KEYMOD.
	// At least, linux.
	if(vk != 0) {
		emu->key_up(vk);
	}
	emu->unlock_vm();
}

void GLDrawClass::keyPressEvent(QKeyEvent *event)
{
	int key = event->key();
	uint32_t mod = event->modifiers();;
	uint32_t scan;
	uint32_t vk;
   
	if(event->isAutoRepeat()) return;
	scan = event->nativeScanCode();
	vk = get106Scancode2VK(scan);

#if defined(USE_MOUSE)
	if(vk == VK_APPS) { // Special key : capture/uncapture mouse.
		emit sig_toggle_mouse();
		return;
	}
#endif	
#if defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)	
	if(using_flags.is_notify_key_down_lr_shift()) {
		if(vk == VK_SHIFT) {
			if(GetAsyncKeyState(VK_LSHIFT) & 0x8000) vk = VK_LSHIFT;
			if(GetAsyncKeyState(VK_RSHIFT) & 0x8000) vk = VK_RSHIFT;
		}
		if(vk == VK_MENU) {
			if(GetAsyncKeyState(VK_LMENU) & 0x8000) vk = VK_LMENU;
			if(GetAsyncKeyState(VK_RMENU) & 0x8000) vk = VK_RMENU;
		}
	}
#endif
   
	//printf("Key: DOWN: VK=%d SCAN=%04x MOD=%08x\n", vk, scan, mod);
	emu->lock_vm();
	emu->key_modifiers(mod);
	if(vk != 0) {
		emu->key_down(vk, false);
	}
	emu->unlock_vm();
}


QStringList *GLDrawClass::getKeyNames(void)
{
	if(key_table) {
		return key_table->get_scan_name_list();
	}
	return NULL;
}

QStringList *GLDrawClass::getVKNames(void)
{
	if(key_table) {
		return key_table->get_vk_name_list();
	}
	return NULL;
}

keydef_table_t *GLDrawClass::get_key_table(int index)
{
	if(key_table) {
		return key_table->get_using_key_table(index);
	}
	return NULL;
}

keydef_table_t *GLDrawClass::get_key_tables(void)
{
	if(key_table) {
		return key_table->get_using_key_table(0);
	}
	return NULL;
}

int GLDrawClass::get_key_table_size(void)
{
	if(key_table) {
		return key_table->get_key_table_size();
	}
	return 0;
}


void GLDrawClass::do_update_keyboard_scan_code(uint32_t vk, uint32_t scan)
{
	if(key_table) {
		key_table->do_set_scan_code(vk, scan);
	}
}

uint32_t GLDrawClass::get_vk_from_index(int index)
{
	if(key_table) {
		return key_table->get_vk_from_index(index);
	}
	return 0;
}

uint32_t GLDrawClass::get_scan_from_index(int index)
{
	if(key_table) {
		return key_table->get_scan_from_index(index);
	}
	return 0;
}

const char *GLDrawClass::get_key_vk_name(int index)
{
	if(key_table) {
		return key_table->get_vk_name(index);
	}
	return NULL;
}

const keydef_table_t *GLDrawClass::get_default_key_table()
{
	if(key_table) {
		return (const keydef_table_t *)key_table->get_default_key_table();
	}
	return NULL;
}
