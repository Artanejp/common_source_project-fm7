/*
 *	Skelton for retropc emulator
 *
 *	Author : Takeda.Toshiya
 *	Date   : 2006.08.18 -
 *      Converted to QT by (C) 2015 K.Ohta
 *	  History:
 *	     Jan 12, 2015 (maybe) : Initial
 *	[ SDL input -> Keyboard]
*/

#include <Qt>
#include <QApplication>
#include <SDL.h>
#include "emu.h"
#include "vm/vm.h"
#include "fifo.h"
#include "fileio.h"
#include "qt_input.h"
#include "qt_gldraw.h"
#include "qt_main.h"
//#include "menuclasses.h"
//#include "commonclasses.h"
#include "mainwidget.h"
#include "agar_logger.h"


#ifndef Ulong
#define Ulong unsigned long
#endif

#define KEY_KEEP_FRAMES 3

extern EMU* emu;

void EMU::initialize_input()
{
	// initialize status
	memset(key_status, 0, sizeof(key_status));
	memset(joy_status, 0, sizeof(joy_status));
	memset(mouse_status, 0, sizeof(mouse_status));
	
	// initialize joysticks
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

void EMU::release_input()
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


void EMU::update_input()
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

#if defined(USE_BUTTON)
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

void EMU::key_down(int sym, bool repeat)
{
	bool keep_frames = false;
	uint8 code = sym;
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

	
void EMU::key_up(int sym)
{
	uint8 code = sym;
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

#ifdef USE_BUTTON
void EMU::press_button(int num)
{
	int code = buttons[num].code;
	
	if(code) {
		key_down(code, false);
		key_status[code] = KEY_KEEP_FRAMES;
	} else {
		// code=0: reset virtual machine
		vm->reset();
	}
}
#endif


void EMU::enable_mouse()
{
	// enable mouse emulation
	if(!mouse_enabled) {
		QCursor cursor;
		QPoint pos;
		mouse_oldx = mouse_ptrx = SCREEN_WIDTH / 2;
		mouse_oldy = mouse_ptry = SCREEN_HEIGHT / 2;
		cursor = instance_handle->cursor();
		pos.setX(instance_handle->width() / 2);
		pos.setY(instance_handle->height() / 2);
		cursor.setPos(instance_handle->mapToGlobal(pos));
		QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
		//mouse_shape = cursor.shape();
		//cursor.setShape(Qt::BlankCursor);
		mouse_status[0] = 0;
		mouse_status[1] = 0;
		mouse_status[2] = mouse_button;
	}
	instance_handle->setMouseTracking(true);
	mouse_enabled = true;

}



void EMU::disenable_mouse()
{
	// disenable mouse emulation
	if(mouse_enabled) {
		QCursor cursor;
		cursor = instance_handle->cursor();
		if(QApplication::overrideCursor() != NULL) QApplication::restoreOverrideCursor();
		//QApplication::restoreOverrideCursor();
		instance_handle->setMouseTracking(false);
	}
	mouse_enabled = false;
}

void EMU::toggle_mouse()
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


void EMU::set_auto_key_string(const char *cstr)
{
	stop_auto_key();
	memset(auto_key_str, 0x00, sizeof(auto_key_str));
	strncpy(auto_key_str, cstr, sizeof(auto_key_str) - 2);
}
	
void EMU::start_auto_key()
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
  
void EMU::stop_auto_key()
{
	 if(autokey_shift) {
		key_up(VK_SHIFT);
	}
	autokey_phase = autokey_shift = 0;
	//memset(auto_key_str, 0x00, sizeof(auto_key_str);
}
#endif


#if !defined(Q_OS_WIN) && !defined(Q_OS_CYGWIN)
uint16_t EMU::GetAsyncKeyState(uint32_t vk)
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

