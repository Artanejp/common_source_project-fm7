/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[Qt input ]
*/

#include <Qt>
#include <QApplication>
#include <SDL.h>

//#include "../emu.h"
#include "../fifo.h"
#include "../fileio.h"
//#include "osd.h"

#include "qt_input.h"
#include "qt_gldraw.h"
#include "qt_main.h"

#include "osd.h"
//#include "mainwidget.h"
//#include "csp_logger.h"
#include "dropdown_keyset.h"

#define KEY_KEEP_FRAMES	3

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


void OSD_BASE::initialize_input()
{
	// initialize status
	memset(key_status, 0, sizeof(key_status));
	memset(joy_status, 0, sizeof(joy_status));
	memset(mouse_status, 0, sizeof(mouse_status));
	// mouse emulation is disenabled
	mouse_enabled = false;
	mouse_ptrx = mouse_oldx = get_screen_width() / 2;
	mouse_ptry = mouse_oldy = get_screen_height() / 2;
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
	now_auto_key = false;
	
	// initialize shift+numpad conversion
	memset(key_converted, 0, sizeof(key_converted));
	key_shift_pressed = key_shift_released = false;
	lost_focus = false;
}

void OSD_BASE::release_input()
{
	// release mouse
	if(mouse_enabled) {
		disable_mouse();
	}
}

void OSD_BASE::do_assign_js_setting(int jsnum, int axis_idx, int assigned_value)
{
	if((jsnum < 0) || (jsnum >= 4)) return;
	if((axis_idx < 0) || (axis_idx >= 16)) return;
	if((assigned_value < -256) || (assigned_value >= 0x10000)) return;
	p_config->joy_buttons[jsnum][axis_idx] = assigned_value;
}

void OSD_BASE::update_input()
{
	int *keystat;
	int i_c = 0;;
	bool press_flag = false;
	bool release_flag = false;
	if(get_use_shift_numpad_key()) {
		//update numpad key status
		if(key_shift_pressed && !key_shift_released) {
			if(key_status[VK_SHIFT] == 0) {
				// shift key is newly pressed
				key_status[VK_SHIFT] = 0x80;
				if(!this->get_notify_key_down_lr_shift()) key_status[VK_LSHIFT] = 0x80;
				if(this->get_notify_key_down()) vm_key_down(VK_SHIFT, false);
			}
		} else if(!key_shift_pressed && key_shift_released) {
			if(key_status[VK_SHIFT] != 0) {
			// shift key is newly released
				key_status[VK_SHIFT] = 0;
				if(!this->get_notify_key_down_lr_shift()) key_status[VK_LSHIFT] = 0;
				if(this->get_notify_key_down()) vm_key_up(VK_SHIFT);
				// check l/r shift
				if(!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
				if(!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
			}
			if(key_status[VK_LSHIFT] != 0) {
				// shift key is newly released
				key_status[VK_LSHIFT] = 0;
			if(this->get_notify_key_down()) vm_key_up(VK_LSHIFT);
			// check l/r shift
			if(!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
			}
			if(key_status[VK_RSHIFT] != 0) {
				// shift key is newly released
				key_status[VK_RSHIFT] = 0;
				if(this->get_notify_key_down()) vm_key_up(VK_RSHIFT);
				// check l/r shift
				if(!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
			}
		}
		key_shift_pressed = key_shift_released = false;
	}
	    
	// release keys
	if(lost_focus && !now_auto_key) {
		// we lost key focus so release all pressed keys
		for(int i = 0; i < 256; i++) {
			if(key_status[i] & 0x80) {
				key_status[i] &= 0x7f;
				release_flag = true;
				if(this->get_notify_key_down()) {
					if(!key_status[i]) {
						vm_key_up(i);
					}
				}
			}
		}
	} else {
		for(int i = 0; i < 256; i++) {
			if(key_status[i] & 0x7f) {
				key_status[i] = (key_status[i] & 0x80) | ((key_status[i] & 0x7f) - 1);
				press_flag = true;
				if(this->get_notify_key_down()) {
					if(!key_status[i]) {
						vm_key_up(i);
					}
				}
			}
		}
	}
	lost_focus = false;

	update_buttons(press_flag, release_flag);
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
}

void OSD_BASE::key_down(int code, bool repeat)
{
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
	if(get_use_shift_numpad_key()) {
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
	}
	key_down_native(code, repeat);
}

void OSD_BASE::key_up(int code)
{
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
	if(get_use_shift_numpad_key()) {
		if(code == VK_LSHIFT) {
			key_shift_pressed = false;
			key_shift_released = true;
			return;
		} else if(key_converted[code] != 0) {
			key_converted[code] = 0;
			code = numpad_table[code];
		}
	}
	key_up_native(code);
}


void OSD_BASE::key_down_native(int code, bool repeat)
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
	code = keycode_conv[code];
	
	if(get_dont_keeep_key_pressed()) {
		if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU)) {
			key_status[code] = KEY_KEEP_FRAMES;
		} else {
			key_status[code] = keep_frames ? KEY_KEEP_FRAMES : 0x80;
		}
	} else {
		key_status[code] = keep_frames ? KEY_KEEP_FRAMES : 0x80;
	}

	uint8_t prev_shift = key_status[VK_SHIFT];
	uint8_t prev_control = key_status[VK_CONTROL];
	uint8_t prev_menu = key_status[VK_MENU];
	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];
	
	if(get_notify_key_down()) {
		if(keep_frames) {
			repeat = false;
		}
		if(!get_notify_key_down_lr_shift()) {
			if(code == VK_LSHIFT || code == VK_RSHIFT) {
				if(prev_shift == 0 && key_status[VK_SHIFT] != 0) {
					vm_key_down(VK_SHIFT, repeat);
				}
				return;
			}
		}
		if(!get_notify_key_down_lr_control()) {
			if(code == VK_LCONTROL|| code == VK_RCONTROL) {
				if(prev_control == 0 && key_status[VK_CONTROL] != 0) {
					vm_key_down(VK_CONTROL, repeat);
				}
				return;
			}
		}
		if(!get_notify_key_down_lr_menu()) {
			if(code == VK_LMENU|| code == VK_RMENU) {
				if(prev_menu == 0 && key_status[VK_MENU] != 0) {
					vm_key_down(VK_MENU, repeat);
				}
				return;
			}
		}
		vm_key_down(code, repeat);
	}
}

void OSD_BASE::key_up_native(int code)
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
	
	uint8_t prev_shift = key_status[VK_SHIFT];
	uint8_t prev_control = key_status[VK_CONTROL];
	uint8_t prev_menu = key_status[VK_MENU];
	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];
	

	if(get_notify_key_down()) {
		if(!get_notify_key_down_lr_shift()) {
			if(code == VK_LSHIFT || code == VK_RSHIFT) {
				if(prev_shift != 0 && key_status[VK_SHIFT] == 0) {
					vm_key_up(VK_SHIFT);
				}
				return;
			}
		}
		if(!get_notify_key_down_lr_control()) {
			if(code == VK_LCONTROL|| code == VK_RCONTROL) {
				if(prev_control != 0 && key_status[VK_CONTROL] == 0) {
					vm_key_up(VK_CONTROL);
				}
				return;
			}
		}
		if(!get_notify_key_down_lr_menu()) {
			if(code == VK_LMENU || code == VK_RMENU) {
				if(prev_menu != 0 && key_status[VK_MENU] == 0) {
					vm_key_up(VK_MENU);
				}
				return;
			}
		}
		vm_key_up(code);
	}
}

void OSD_BASE::key_down_sub(int code, bool repeat)
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
	

	if(get_dont_keeep_key_pressed()) {
		if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU)) {
			key_status[code] = KEY_KEEP_FRAMES;
		} else {
			key_status[code] = keep_frames ? KEY_KEEP_FRAMES : 0x80;
		}
	} else {
		key_status[code] = keep_frames ? KEY_KEEP_FRAMES : 0x80;
	}

	uint8_t prev_shift = key_status[VK_SHIFT];
	uint8_t prev_control = key_status[VK_CONTROL];
	uint8_t prev_menu = key_status[VK_MENU];
	
	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];
	
	if(get_notify_key_down()) {
		if(keep_frames) {
			repeat = false;
		}
		if(!get_notify_key_down_lr_shift()) {
			if(code == VK_LSHIFT || code == VK_RSHIFT) {
				if(prev_shift == 0 && key_status[VK_SHIFT] != 0) {
					vm_key_down(VK_SHIFT, repeat);
				}
				return;
			}
		}
		if(!get_notify_key_down_lr_control()) {
			if(code == VK_LCONTROL|| code == VK_RCONTROL) {
				if(prev_control == 0 && key_status[VK_CONTROL] != 0) {
					vm_key_down(VK_CONTROL, repeat);
				}
				return;
			}
		}
		if(!get_notify_key_down_lr_menu()) {
			if(code == VK_LMENU|| code == VK_RMENU) {
				if(prev_menu == 0 && key_status[VK_MENU] != 0) {
					vm_key_down(VK_MENU, repeat);
				}
				return;
			}
		}
		vm_key_down(code, repeat);
	}
}

void OSD_BASE::key_up_sub(int code)
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
	
	uint8_t prev_shift = key_status[VK_SHIFT];
	uint8_t prev_control = key_status[VK_CONTROL];
	uint8_t prev_menu = key_status[VK_MENU];
	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];
	if(get_notify_key_down()) {
		if(!get_notify_key_down_lr_shift()) {
			if(code == VK_LSHIFT || code == VK_RSHIFT) {
				if(prev_shift != 0 && key_status[VK_SHIFT] == 0) {
					vm_key_up(VK_SHIFT);
				}
				return;
			}
		}
		if(!get_notify_key_down_lr_control()) {
			if(code == VK_LCONTROL|| code == VK_RCONTROL) {
				if(prev_control != 0 && key_status[VK_CONTROL] == 0) {
					vm_key_up(VK_CONTROL);
				}
				return;
			}
		}
		if(!get_notify_key_down_lr_menu()) {
			if(code == VK_LMENU || code == VK_RMENU) {
				if(prev_menu != 0 && key_status[VK_MENU] == 0) {
					vm_key_up(VK_MENU);
				}
				return;
			}
		}
		vm_key_up(code);
	}
}


void OSD_BASE::key_modifiers(uint32_t mod)
{
	modkey_status = mod;
}


void OSD_BASE::modify_key_buffer(int code, uint8_t val)
{
	key_status[code] = val;
}

void OSD_BASE::key_lost_focus()
{
	lost_focus = true;
}

uint8_t* OSD_BASE::get_key_buffer()
{
	return key_status;
}

uint32_t* OSD_BASE::get_joy_buffer()
{
	return joy_status;
}

int* OSD_BASE::get_mouse_buffer()
{
	return mouse_status;
}

void OSD_BASE::press_button(int num)
{
	if(get_one_board_micro_computer()) {
		int code = get_vm_buttons_code(num);
		
		if(code) {
			key_down_sub(code, false);
			key_status[code] = KEY_KEEP_FRAMES;
		} else {
			// code=0: reset virtual machine
			vm_reset();
		}
	}
}


void OSD_BASE::enable_mouse()
{
	// enable mouse emulation
	if(!mouse_enabled) {
		QCursor cursor;
		QPoint pos;
		mouse_oldx = mouse_ptrx = get_screen_width() / 2;
		mouse_oldy = mouse_ptry = get_screen_height() / 2;
		cursor = glv->cursor();
		pos.setX(glv->width() / 2);
		pos.setY(glv->height() / 2);
		cursor.setPos(glv->mapToGlobal(pos));
		QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
		//mouse_shape = cursor.shape();
		//cursor.setShape(Qt::BlankCursor);
		mouse_status[0] = 0;
		mouse_status[1] = 0;
		mouse_status[2] = mouse_button;
	}
	glv->setMouseTracking(true);
	mouse_enabled = true;
}

void OSD_BASE::disable_mouse()
{
	// disenable mouse emulation
	if(mouse_enabled) {
		QCursor cursor;
		cursor = glv->cursor();
		if(QApplication::overrideCursor() != NULL) QApplication::restoreOverrideCursor();
		//QApplication::restoreOverrideCursor();
		glv->setMouseTracking(false);
	}
	mouse_enabled = false;
}

void OSD_BASE::toggle_mouse()
{
	// toggle mouse enable / disenable
	if(mouse_enabled) {
		disable_mouse();
	} else {
		enable_mouse();
	}
}

bool OSD_BASE::is_mouse_enabled()
{
	return mouse_enabled;
}

void OSD_BASE::set_mouse_pointer(int x, int y)
{
	mouse_ptrx = x;
	mouse_ptry = y;
}

void OSD_BASE::set_mouse_button(int button) 
{
	mouse_button = button;
}

int OSD_BASE::get_mouse_button() 
{
	return mouse_button;
}

#if !defined(Q_OS_WIN) && !defined(Q_OS_CYGWIN)
uint16_t OSD_BASE::GetAsyncKeyState(uint32_t vk)
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
