/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[Qt input ]
*/

#include <Qt>
#include <QApplication>
#include <SDL.h>

#include "../fifo.h"
#include "../fileio.h"

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
	{
		std::lock_guard<std::recursive_timed_mutex> n(joystick_mutex);
		memset(joy_status, 0, sizeof(joy_status));
	}
	{
		std::lock_guard<std::recursive_timed_mutex> n(mouse_mutex);
		memset(mouse_status, 0, sizeof(mouse_status));
		// mouse emulation is disenabled
		mouse_enabled = false;
	}

	mouse_ptrx = mouse_oldx = (double)(get_screen_width() / 2);
	mouse_ptry = mouse_oldy = (double)(get_screen_height() / 2);
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
	numpad_5_pressed = false;
	for(int i = 0; i < 256; i++) {
		joy_to_key_status[i] = false;
	}
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

	std::lock_guard<std::recursive_timed_mutex> n(joystick_mutex);
	p_config->joy_buttons[jsnum][axis_idx] = assigned_value;
}

void OSD_BASE::update_input_mouse()
{
	std::lock_guard<std::recursive_timed_mutex>	n(mouse_mutex);
	memset(mouse_status, 0, sizeof(mouse_status));
	//bool hid = false;
	if(mouse_enabled) {
		double diffx = mouse_ptrx - mouse_oldx;
		double diffy = mouse_ptry - mouse_oldy;
		double factor = (double)(p_config->mouse_sensitivity & ((1 << 16) - 1));
		diffx = (diffx * factor) / 8192.0;
		diffy = (diffy * factor) / 8192.0;

		mouse_status[0] = (int32_t)rint(diffx);
		mouse_status[1] = (int32_t)rint(diffy);
		mouse_status[2] = mouse_button;
		//printf("Mouse delta(%d, %d)\n", delta_x, delta_y);
		mouse_oldx = mouse_ptrx;
		mouse_oldy = mouse_ptry;
	}

}

void OSD_BASE::update_input()
{
	bool press_flag = false;
	bool release_flag = false;
//	bool get_keycode = false;
//	if(get_use_auto_key()) {
//		get_keycode = (!(now_auto_key) && !(config.romaji_to_kana));
//	} else {
//		get_keycode = true;
//	}
	{
		//update numpad key status
		if(key_shift_pressed && !key_shift_released) {
			if(key_status[VK_SHIFT] == 0) {
				// shift key is newly pressed
				if(!key_status[VK_RSHIFT]) {
					vm_key_down(VK_SHIFT, false);
				}
				vm_key_down(VK_LSHIFT, false);
				key_status[VK_LSHIFT] = key_status[VK_SHIFT] = 0x80;
			}
		} else if(!key_shift_pressed && key_shift_released) {
			if(key_status[VK_LSHIFT] != 0) {
				// shift key is newly released
				vm_key_up(VK_LSHIFT);
				if(!key_status[VK_RSHIFT]) {
					vm_key_up(VK_SHIFT);
				}
				key_status[VK_LSHIFT] = key_status[VK_SHIFT] = 0;
			}
		}
		key_shift_pressed = key_shift_released = false;
	}
	if(p_config->use_joy_to_key) {
		int status[256] = {0};
		if(p_config->joy_to_key_type == 0) { // Cursor
			std::lock_guard<std::recursive_timed_mutex> n(joystick_mutex);
			static const int vk[] = {VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT};
			for(int i = 0; i < 4; i++) {
				if(joy_status[0] & (1 << i)) {
					status[vk[i]] = 1;
				}
			}
		} else if(p_config->joy_to_key_type == 1) { // 2468
			std::lock_guard<std::recursive_timed_mutex>  n(joystick_mutex);
			static const int vk[] = {VK_NUMPAD8, VK_NUMPAD2, VK_NUMPAD4, VK_NUMPAD6};
			for(int i = 0; i < 4; i++) {
				if(joy_status[0] & (1 << i)) {
					status[vk[i]] = 1;
				}
			}
		} else if(p_config->joy_to_key_type == 2) { // 24681379
			// numpad key (8-directions)
			std::lock_guard<std::recursive_timed_mutex> n(joystick_mutex);
			switch(joy_status[0] & 0x0f) {
			case 0x02 + 0x04: status[VK_NUMPAD1] = 1; break; // down-left
			case 0x02       : status[VK_NUMPAD2] = 1; break; // down
			case 0x02 + 0x08: status[VK_NUMPAD3] = 1; break; // down-right
			case 0x00 + 0x04: status[VK_NUMPAD4] = 1; break; // left
//			case 0x00       : status[VK_NUMPAD5] = 1; break;
			case 0x00 + 0x08: status[VK_NUMPAD6] = 1; break; // right
			case 0x01 + 0x04: status[VK_NUMPAD7] = 1; break; // up-left
			case 0x01       : status[VK_NUMPAD8] = 1; break; // up
			case 0x01 + 0x08: status[VK_NUMPAD9] = 1; break; // up-right
			}
		} else if(p_config->joy_to_key_type == 3) { // 1235
			static const int vk[] = {VK_NUMPAD5, VK_NUMPAD2, VK_NUMPAD1, VK_NUMPAD3};
			std::lock_guard<std::recursive_timed_mutex>  n(joystick_mutex);
			for(int i = 0; i < 4; i++) {
				if(joy_status[0] & (1 << i)) {
					status[vk[i]] = 1;
				}
			}
		}
		if(p_config->joy_to_key_type == 1 || p_config->joy_to_key_type == 2) {
			// numpad key
			std::lock_guard<std::recursive_timed_mutex>  n(joystick_mutex);
			if(p_config->joy_to_key_numpad5 && !(joy_status[0] & 0x0f)) {
				if(!numpad_5_pressed) {
					status[VK_NUMPAD5] = 1;
					numpad_5_pressed = true;
				}
			}
		} else if(p_config->joy_to_key_type == 3) {
			// numpad key
			std::lock_guard<std::recursive_timed_mutex>  n(joystick_mutex);
			if(p_config->joy_to_key_numpad5 && !(joy_status[0] & 0x0f)) {
				if(!numpad_5_pressed) {
					status[VK_NUMPAD8] = 1;
					numpad_5_pressed = true;
				}
			}
		}

		for(int i = 0; i < 16; i++) {
			std::lock_guard<std::recursive_timed_mutex> n(joystick_mutex);
			if(joy_status[0] & (1 << (i + 4))) {
				if(p_config->joy_to_key_buttons[i] < 0 && -p_config->joy_to_key_buttons[i] < 256) {
					status[-p_config->joy_to_key_buttons[i]] = 1;
				}
			}
		}
		for(int i = 0; i < 256; i++) {
			if(status[i]) {
				if(!joy_to_key_status[i]) {
					if(!(key_status[i] & 0x80)) {
						key_down_native(i, false);
						// do not keep key pressed
						if(p_config->joy_to_key_numpad5 && (i >= VK_NUMPAD1 && i <= VK_NUMPAD9)) {
							key_status[i] = KEY_KEEP_FRAMES;
							if(p_config->joy_to_key_type == 3) {
								if(numpad_5_pressed && (i != VK_NUMPAD8)) {
									numpad_5_pressed = false;
								}
							} else if((p_config->joy_to_key_type == 1) || (p_config->joy_to_key_type == 2)) {
								if(numpad_5_pressed && (i != VK_NUMPAD5)) {
									numpad_5_pressed = false;
								}
							}
						}
					}
					joy_to_key_status[i] = true;
				}
			} else {
				if(joy_to_key_status[i]) {
					if(key_status[i]) {
						key_up_native(i);
					}
					joy_to_key_status[i] = false;
				}
			}
		}
	}
	// release keys
	if(lost_focus && !now_auto_key) {
		// we lost key focus so release all pressed keys
		for(int i = 0; i < 256; i++) {
			if(key_status[i] & 0x80) {
				key_status[i] &= 0x7f;
				release_flag = true;
				if(!(key_status[i])) {
					vm_key_up(i);
				}
			}
		}
	} else {
		for(int i = 0; i < 256; i++) {
			if(key_status[i] & 0x7f) {
				key_status[i] = (key_status[i] & 0x80) | ((key_status[i] & 0x7f) - 1);
				press_flag = true;
				if(!(key_status[i]) && !(joy_to_key_status[i])) {
					vm_key_up(i);
				}
			}
		}
	}
	lost_focus = false;

	// update mouse status
	//update_input_mouse();

}

void OSD_BASE::key_down(int code, bool extended, bool repeat)
{
	if((code >= 256) || (code < 0)) return; // WORKAROUND
//#ifdef USE_AUTO_KEY
	if(!__USE_AUTO_KEY || (!now_auto_key && !p_config->romaji_to_kana)) {
//#endif
		//if(!dinput_key_available) {
			if(code == VK_SHIFT) {
				if(!(key_status[VK_RSHIFT] & 0x80) && (GetAsyncKeyState(VK_RSHIFT) & 0x8000)) {
					key_down_native(VK_RSHIFT, repeat);
				}
				if(!(key_status[VK_LSHIFT] & 0x80) && (GetAsyncKeyState(VK_LSHIFT) & 0x8000)) {
					code = VK_LSHIFT;
				} else {
					return;
				}
			} else if(code == VK_CONTROL) {
				if(!(key_status[VK_RCONTROL] & 0x80) && (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) {
					key_down_native(VK_RCONTROL, repeat);
				}
				if(!(key_status[VK_LCONTROL] & 0x80) && (GetAsyncKeyState(VK_LCONTROL) & 0x8000)) {
					code = VK_LCONTROL;
				} else {
					return;
				}
			} else if(code == VK_MENU) {
				if(!(key_status[VK_RMENU] & 0x80) && (GetAsyncKeyState(VK_RMENU) & 0x8000)) {
					key_down_native(VK_RMENU, repeat);
				}
				if(!(key_status[VK_LMENU] & 0x80) && (GetAsyncKeyState(VK_LMENU) & 0x8000)) {
					code = VK_LMENU;
				} else {
					return;
				}
			}

			switch(p_config->cursor_as_ten_key) {
			case CONFIG_CURSOR_AS_2468:
				if(code == VK_RIGHT) {
					code = VK_NUMPAD6;
				} else if(code == VK_LEFT) {
					code = VK_NUMPAD4;
				} else if(code == VK_DOWN) {
					code = VK_NUMPAD2;
				} else if(code == VK_UP) {
					code = VK_NUMPAD8;
				}
				break;
			case CONFIG_CURSOR_AS_1235:
				if(code == VK_RIGHT) {
					code = VK_NUMPAD3;
				} else if(code == VK_LEFT) {
					code = VK_NUMPAD1;
				} else if(code == VK_DOWN) {
					code = VK_NUMPAD2;
				} else if(code == VK_UP) {
					code = VK_NUMPAD5;
				}
				break;
			case CONFIG_CURSOR_AS_CURSOR:
				break;
			default:
				break;
			}

//			if(code == VK_LSHIFT || code == VK_RSHIFT) {
			if(code == VK_LSHIFT) {
				key_shift_pressed = true;
				return;
			}
			if(!extended) {
				switch(code) {
				case VK_INSERT:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD0]) {
						code = VK_NUMPAD0;
						key_shift_pressed = true;
					}
					break;
				case VK_END:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD1]) {
						code = VK_NUMPAD1;
						key_shift_pressed = true;
					}
					break;
				case VK_DOWN:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD2]) {
						code = VK_NUMPAD2;
						key_shift_pressed = true;
					}
					break;
				case VK_NEXT:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD3]) {
						code = VK_NUMPAD3;
						key_shift_pressed = true;
					}
					break;
				case VK_LEFT:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD4]) {
						code = VK_NUMPAD4;
						key_shift_pressed = true;
					}
					break;
				case VK_CLEAR:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD5]) {
						code = VK_NUMPAD5;
						key_shift_pressed = true;
					}
					break;
				case VK_RIGHT:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD6]) {
						code = VK_NUMPAD6;
						key_shift_pressed = true;
					}
					break;
				case VK_HOME:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD7]) {
						code = VK_NUMPAD7;
						key_shift_pressed = true;
					}
					break;
				case VK_UP:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD8]) {
						code = VK_NUMPAD8;
						key_shift_pressed = true;
					}
					break;
				case VK_PRIOR:
					if(key_shift_pressed || key_shift_released || key_status[VK_NUMPAD9]) {
						code = VK_NUMPAD9;
						key_shift_pressed = true;
					}
					break;
				}
			}
			key_down_native(code, repeat);
		//} else {
		//	if(repeat || code == 0xf0 || code == 0xf1 || code == 0xf2 || code == 0xf3 || code == 0xf4) {
		//		key_down_native(code, repeat);
		//	}
		//}
//#ifdef USE_AUTO_KEY
	}
//#endif
}

void OSD_BASE::key_up(int code, bool extended)
{
	if((code >= 256) || (code < 0)) return; // WORKAROUND
//#ifdef USE_AUTO_KEY
	if(!__USE_AUTO_KEY || (!now_auto_key && !p_config->romaji_to_kana)) {
//#endif
		// _TCHAR __tmps1[128] = {0};
		// my_stprintf_s(__tmps1, sizeof(__tmps1) - 1, "KEY UP %d", code);
		//emit sig_debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_OSD, QString::fromUtf8(__tmps));
		//if(!dinput_key_available) {
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
			switch(p_config->cursor_as_ten_key) {
			case CONFIG_CURSOR_AS_2468:
				if(code == VK_RIGHT) {
					code = VK_NUMPAD6;
				} else if(code == VK_LEFT) {
					code = VK_NUMPAD4;
				} else if(code == VK_DOWN) {
					code = VK_NUMPAD2;
				} else if(code == VK_UP) {
					code = VK_NUMPAD8;
				}
				break;
			case CONFIG_CURSOR_AS_1235:
				if(code == VK_RIGHT) {
					code = VK_NUMPAD3;
				} else if(code == VK_LEFT) {
					code = VK_NUMPAD1;
				} else if(code == VK_DOWN) {
					code = VK_NUMPAD2;
				} else if(code == VK_UP) {
					code = VK_NUMPAD5;
				}
				break;
			case CONFIG_CURSOR_AS_CURSOR:
				break;
			default:
				break;
			}

//			if(code == VK_LSHIFT || code == VK_RSHIFT) {
				if(code == VK_LSHIFT) {
					key_shift_pressed = false;
					key_shift_released = true;
					return;
				}

				if(!extended) {
					switch(code) {
					case VK_NUMPAD0: case VK_INSERT:
						key_up_native(VK_NUMPAD0);
						key_up_native(VK_INSERT);
						return;
					case VK_NUMPAD1: case VK_END:
						key_up_native(VK_NUMPAD1);
						key_up_native(VK_END);
						return;
					case VK_NUMPAD2: case VK_DOWN:
						key_up_native(VK_NUMPAD2);
						key_up_native(VK_DOWN);
						return;
					case VK_NUMPAD3: case VK_NEXT:
						key_up_native(VK_NUMPAD3);
						key_up_native(VK_NEXT);
						return;
					case VK_NUMPAD4: case VK_LEFT:
						key_up_native(VK_NUMPAD4);
						key_up_native(VK_LEFT);
						return;
					case VK_NUMPAD5: case VK_CLEAR:
						key_up_native(VK_NUMPAD5);
						key_up_native(VK_CLEAR);
						return;
					case VK_NUMPAD6: case VK_RIGHT:
						key_up_native(VK_NUMPAD6);
						key_up_native(VK_RIGHT);
						return;
					case VK_NUMPAD7: case VK_HOME:
						key_up_native(VK_NUMPAD7);
						key_up_native(VK_HOME);
						return;
					case VK_NUMPAD8: case VK_UP:
						key_up_native(VK_NUMPAD8);
						key_up_native(VK_UP);
						return;
					case VK_NUMPAD9: case VK_PRIOR:
						key_up_native(VK_NUMPAD9);
						key_up_native(VK_PRIOR);
						return;
					}
				}
			key_up_native(code);
	}
}


void OSD_BASE::key_down_native(int code, bool repeat)
{
	bool keep_frames = false;

	if(code == 0xf0) {
		code = VK_CAPITAL;
		keep_frames = true;
	} else if(code == 0xf1 || code == 0xf2) {
		code = VK_KANA;
		keep_frames = true;
	} else if(code == 0xf3 || code == 0xf4) {
		code = VK_KANJI;
		keep_frames = true;
	}
	if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU || code == VK_MENU)) {
		code = keycode_conv[code];
	}
	if(key_status[code] == 0 || keep_frames) {
		repeat = false;
	}
	if(get_dont_keeep_key_pressed()) {
		if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU || code == VK_MENU)) {
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

	if(code == VK_LSHIFT || code == VK_RSHIFT) {
		if(prev_shift == 0 && key_status[VK_SHIFT] != 0) {
			vm_key_down(VK_SHIFT, repeat);
		}
	} else if(code == VK_LCONTROL|| code == VK_RCONTROL) {
		if(prev_control == 0 && key_status[VK_CONTROL] != 0) {
			vm_key_down(VK_CONTROL, repeat);
		}
	} else if(code == VK_LMENU|| code == VK_RMENU) {
		if(prev_menu == 0 && key_status[VK_MENU] != 0) {
			vm_key_down(VK_MENU, repeat);
		}
	}
	vm_key_down(code, repeat);
}

void OSD_BASE::key_up_native(int code)
{
	if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU || code == VK_MENU)) {
		code = keycode_conv[code];
	}
	if(key_status[code] == 0) {
		return;
	}
	if((key_status[code] &= 0x7f) != 0) {
		return;
	}
	vm_key_up(code);

	uint8_t prev_shift = key_status[VK_SHIFT];
	uint8_t prev_control = key_status[VK_CONTROL];
	uint8_t prev_menu = key_status[VK_MENU];

	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];

	if(code == VK_LSHIFT || code == VK_RSHIFT) {
		if(prev_shift != 0 && key_status[VK_SHIFT] == 0) {
			vm_key_up(VK_SHIFT);
		}
	} else if(code == VK_LCONTROL|| code == VK_RCONTROL) {
		if(prev_control != 0 && key_status[VK_CONTROL] == 0) {
			vm_key_up(VK_CONTROL);
		}
	} else if(code == VK_LMENU || code == VK_RMENU) {
		if(prev_menu != 0 && key_status[VK_MENU] == 0) {
			vm_key_up(VK_MENU);
		}
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
	if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU || code == VK_MENU)) {
		code = keycode_conv[code];
	}


	if(get_dont_keeep_key_pressed()) {
		if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU || code == VK_MENU)) {
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

	{
		if(keep_frames) {
			repeat = false;
		}
		{
			if(code == VK_LSHIFT || code == VK_RSHIFT) {
				if(prev_shift == 0 && key_status[VK_SHIFT] != 0) {
					vm_key_down(VK_SHIFT, repeat);
				}
				return;
			}
		}
		{
			if(code == VK_LCONTROL|| code == VK_RCONTROL) {
				if(prev_control == 0 && key_status[VK_CONTROL] != 0) {
					vm_key_down(VK_CONTROL, repeat);
				}
				return;
			}
		}
		{
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
	if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU || code == VK_MENU)) {
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
	{
		{
			if(code == VK_LSHIFT || code == VK_RSHIFT) {
				if(prev_shift != 0 && key_status[VK_SHIFT] == 0) {
					vm_key_up(VK_SHIFT);
				}
				return;
			}
		}
		{
			if(code == VK_LCONTROL|| code == VK_RCONTROL) {
				if(prev_control != 0 && key_status[VK_CONTROL] == 0) {
					vm_key_up(VK_CONTROL);
				}
				return;
			}
		}
		{
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
	joystick_mutex.lock();
	return joy_status;
}

void OSD_BASE::release_joy_buffer(uint32_t* ptr)
{
	joystick_mutex.unlock();
}

int32_t* OSD_BASE::get_mouse_buffer()
{
	std::lock_guard<std::recursive_timed_mutex> n(mouse_mutex);
	update_input_mouse();
	return mouse_status;
}
void OSD_BASE::release_mouse_buffer(int32_t* ptr)
{
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
		std::lock_guard<std::recursive_timed_mutex>  n(mouse_mutex);
		double xx = (double)(get_screen_width() / 2);
		double yy = (double)(get_screen_height() / 2);

		mouse_oldx = xx;
		mouse_oldy = yy;
		mouse_ptrx = xx;
		mouse_ptry = yy;
		mouse_status[0] = 0;
		mouse_status[1] = 0;
		mouse_status[2] = mouse_button;
		emit sig_enable_mouse();
	}
	mouse_enabled = true;
}

void OSD_BASE::disable_mouse()
{
	// disenable mouse emulation
	if(mouse_enabled) {
		emit sig_disable_mouse();
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

void OSD_BASE::set_mouse_pointer(double x, double y)
{
	if((mouse_enabled)) {
		std::lock_guard<std::recursive_timed_mutex>  n(mouse_mutex);

		mouse_ptrx = x;
		mouse_ptry = y;
	}
}

void OSD_BASE::set_mouse_button(int button)
{
	std::lock_guard<std::recursive_timed_mutex> n(mouse_mutex);
	mouse_button = button;
}

int32_t OSD_BASE::get_mouse_button()
{
	std::lock_guard<std::recursive_timed_mutex> n(mouse_mutex);
	return mouse_button;
}

int OSD_BASE::get_key_name_table_size(void)
{
	if(vm != NULL) {
		return vm->get_key_name_table_size();
	}
	return 0;
}

void OSD_BASE::update_keyname_table(void)
{
	emit sig_clear_keyname_table();
	const _TCHAR *p = NULL;
	for(uint32_t i = 0x00; i < get_key_name_table_size(); i++) {
		p = get_key_name_by_scancode(i);
		uint32_t vk = get_vk_by_scancode(i);
		if((p != NULL) && (vk != 0xffffffff)) {
			QString tmps = QString::fromUtf8(p);
			emit sig_add_keyname_table(vk, tmps);
		}
	}
}

const _TCHAR *OSD_BASE::get_key_name_by_scancode(uint32_t scancode)
{
	if(vm != NULL) {
		return vm->get_phy_key_name_by_scancode(scancode);
	}
	return (const _TCHAR *)NULL;
}

const _TCHAR *OSD_BASE::get_key_name_by_vk(uint32_t vk)
{
	if(vm != NULL) {
		return vm->get_phy_key_name_by_vk(vk);
	}
	return (const _TCHAR *)NULL;
}

uint32_t OSD_BASE::get_scancode_by_vk(uint32_t vk)
{
	if(vm != NULL) {
		return vm->get_scancode_by_vk(vk);
	}
	return 0xffffffff;
}

uint32_t OSD_BASE::get_vk_by_scancode(uint32_t scancode)
{
	if(vm != NULL) {
		return vm->get_vk_by_scancode(scancode);
	}
	return 0xffffffff;
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
