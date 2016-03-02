/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.26-

	[ win32 input ]
*/

#include "osd.h"
#include "../fifo.h"

#ifdef NOTIFY_KEY_DOWN_LR_SHIFT
#define VK_SHIFT_TEMP	VK_LSHIFT
#else
#define VK_SHIFT_TEMP	VK_SHIFT
#endif

static const uint8_t vk_dik[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xC5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x79, 0x7b, 0x00, 0x00,
	0x39, 0xc9, 0xd1, 0xcf, 0xc7, 0xcb, 0xc8, 0xcd, 0xd0, 0x00, 0x00, 0x00, 0x00, 0xd2, 0xd3, 0x00,
	0x0b, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x1e, 0x30, 0x2e, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	0x19, 0x10, 0x13, 0x1f, 0x14, 0x16, 0x2f, 0x11, 0x2d, 0x15, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00,
#ifdef USE_NUMPAD_ENTER
	0x52, 0x4f, 0x50, 0x51, 0x4b, 0x4c, 0x4d, 0x47, 0x48, 0x49, 0x37, 0x4e, 0x9c, 0x4a, 0x53, 0xb5,
#else
	0x52, 0x4f, 0x50, 0x51, 0x4b, 0x4c, 0x4d, 0x47, 0x48, 0x49, 0x37, 0x4e, 0x00, 0x4a, 0x53, 0xb5,
#endif
	0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x57, 0x58, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	DIK_LSHIFT, DIK_RSHIFT, DIK_LCONTROL, DIK_RCONTROL, DIK_LMENU, DIK_RMENU,
	                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x92, 0x27, 0x33, 0x0c, 0x34, 0x35,
	0x91, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x7d, 0x1b, 0x90, 0x00,
	0x00, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	0x3a, 0x00, 0x70, 0x00, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// caps, kana, kanji
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

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

void OSD::initialize_input()
{
	// initialize status
	memset(key_status, 0, sizeof(key_status));
#ifdef USE_JOYSTICK
	memset(joy_status, 0, sizeof(joy_status));
#endif
#ifdef USE_MOUSE
	memset(mouse_status, 0, sizeof(mouse_status));
#endif
	
	// initialize direct input
	dinput_key_available = false;
	lpdi = NULL;
	lpdikey = NULL;
	
	if(config.use_direct_input) {
#if DIRECTINPUT_VERSION >= 0x0800
		if(SUCCEEDED(DirectInput8Create(instance_handle, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&lpdi, NULL))) {
#else
		if(SUCCEEDED(DirectInputCreate(instance_handle, DIRECTINPUT_VERSION, &lpdi, NULL))) {
#endif
			if(SUCCEEDED(lpdi->CreateDevice(GUID_SysKeyboard, &lpdikey, NULL))) {
				if(SUCCEEDED(lpdikey->SetDataFormat(&c_dfDIKeyboard))) {
					if(SUCCEEDED(lpdikey->SetCooperativeLevel(main_window_handle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
						dinput_key_available = true;
						memset(key_dik_prev, 0, sizeof(key_dik_prev));
					}
				}
			}
		}
	}
	
#ifdef USE_JOYSTICK
	// initialize joysticks
	joy_num = joyGetNumDevs();
	for(int i = 0; i < joy_num && i < 4; i++) {
		JOYCAPS joycaps;
		if(joyGetDevCaps(i, &joycaps, sizeof(joycaps)) == JOYERR_NOERROR) {
			joy_mask[i] = (1 << joycaps.wNumButtons) - 1;
		} else {
			joy_mask[i] = 0x0f; // 4buttons
		}
	}
#endif
	
#ifdef USE_MOUSE
	// mouse emulation is disabled
	mouse_enabled = false;
#endif
	
	// initialize keycode convert table
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("keycode.cfg")), FILEIO_READ_BINARY)) {
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
	lost_focus = false;
}

void OSD::release_input()
{
#ifdef USE_MOUSE
	// release mouse
	if(mouse_enabled) {
		disable_mouse();
	}
#endif
	
	// release direct input
	if(lpdi) {
		if(lpdikey) {
			lpdikey->Unacquire();
			lpdikey->Release();
			lpdikey = NULL;
		}
		lpdi->Release();
		lpdi = NULL;
	}
}

void OSD::update_input()
{
	if(dinput_key_available) {
		// direct input
		static uint8_t key_dik[256];
		lpdikey->Acquire();
		lpdikey->GetDeviceState(256, key_dik);
		
#if DIRECTINPUT_VERSION < 0x0800
		// DIK_RSHIFT is not detected on Vista or later
		if(vista_or_later) {
			key_dik[DIK_RSHIFT] = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) ? 0x80 : 0;
		}
#endif
#ifdef USE_SHIFT_NUMPAD_KEY
		// XXX: don't release shift key while numpad key is pressed
		uint8_t numpad_keys;
		numpad_keys  = key_dik[DIK_NUMPAD0];
		numpad_keys |= key_dik[DIK_NUMPAD1];
		numpad_keys |= key_dik[DIK_NUMPAD2];
		numpad_keys |= key_dik[DIK_NUMPAD3];
		numpad_keys |= key_dik[DIK_NUMPAD4];
		numpad_keys |= key_dik[DIK_NUMPAD5];
		numpad_keys |= key_dik[DIK_NUMPAD6];
		numpad_keys |= key_dik[DIK_NUMPAD7];
		numpad_keys |= key_dik[DIK_NUMPAD8];
		numpad_keys |= key_dik[DIK_NUMPAD9];
		if(numpad_keys & 0x80) {
			key_dik[DIK_LSHIFT] |= key_dik_prev[DIK_LSHIFT];
			key_dik[DIK_RSHIFT] |= key_dik_prev[DIK_RSHIFT];
		}
#endif
		key_dik[DIK_CIRCUMFLEX] |= key_dik[DIK_EQUALS     ];
		key_dik[DIK_COLON     ] |= key_dik[DIK_APOSTROPHE ];
		key_dik[DIK_YEN       ] |= key_dik[DIK_GRAVE      ];
#ifndef USE_NUMPAD_ENTER
		key_dik[DIK_RETURN    ] |= key_dik[DIK_NUMPADENTER];
#endif
		
		for(int vk = 0; vk < 256; vk++) {
			int dik = vk_dik[vk];
			if(dik) {
				if(key_dik[dik] & 0x80) {
					if(!(key_dik_prev[dik] & 0x80)) {
						key_down_native(vk, false);
					}
				} else {
					if(key_dik_prev[dik] & 0x80) {
						key_up_native(vk);
					}
				}
			}
		}
		memcpy(key_dik_prev, key_dik, sizeof(key_dik_prev));
#ifdef USE_SHIFT_NUMPAD_KEY
	} else {
		// update numpad key status
		if(key_shift_pressed && !key_shift_released) {
			if(key_status[VK_LSHIFT] == 0) {
				// shift key is newly pressed
				key_status[VK_LSHIFT] = key_status[VK_SHIFT] = 0x80;
#ifdef NOTIFY_KEY_DOWN
				vm->key_down(VK_SHIFT_TEMP, false);
#endif
			}
		} else if(!key_shift_pressed && key_shift_released) {
			if(key_status[VK_LSHIFT] != 0) {
				// shift key is newly released
				key_status[VK_LSHIFT] = key_status[VK_SHIFT] = 0;
#ifdef NOTIFY_KEY_DOWN
				vm->key_up(VK_SHIFT_TEMP);
#endif
			}
		}
		key_shift_pressed = key_shift_released = false;
#endif
	}
	
	// release keys
#ifdef USE_AUTO_KEY
	if(lost_focus && !now_auto_key) {
#else
	if(lost_focus) {
#endif
		// we lost key focus so release all pressed keys
#ifdef NOTIFY_KEY_DOWN
#ifdef NOTIFY_KEY_DOWN_LR_SHIFT
		key_status[VK_SHIFT] = 0;
#else
		key_status[VK_LSHIFT] = key_status[VK_RSHIFT] = 0;
#endif
#ifdef NOTIFY_KEY_DOWN_LR_CONTROL
		key_status[VK_CONTROL] = 0;
#else
		key_status[VK_LCONTROL] = key_status[VK_RCONTROL] = 0;
#endif
#ifdef NOTIFY_KEY_DOWN_LR_MENU
		key_status[VK_MENU] = 0;
#else
		key_status[VK_LMENU] = key_status[VK_RMENU] = 0;
#endif
#endif
		for(int i = 0; i < 256; i++) {
			if(key_status[i] & 0x80) {
				key_status[i] &= 0x7f;
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
#ifdef NOTIFY_KEY_DOWN
				if(!key_status[i]) {
					vm->key_up(i);
				}
#endif
			}
		}
	}
	lost_focus = false;
	
	// VK_$00 should be 0
	key_status[0] = 0;
	
#ifdef USE_JOYSTICK
	// update joystick status
	memset(joy_status, 0, sizeof(joy_status));
	
	for(int i = 0; i < joy_num && i < 4; i++) {
		JOYINFOEX joyinfo;
		joyinfo.dwSize = sizeof(JOYINFOEX);
		joyinfo.dwFlags = JOY_RETURNALL;
		joy_status[i] = 0;
		if(joyGetPosEx(i, &joyinfo) == JOYERR_NOERROR) {
			if(joyinfo.dwYpos < 0x3fff) joy_status[i] |= 0x01;	// up
			if(joyinfo.dwYpos > 0xbfff) joy_status[i] |= 0x02;	// down
			if(joyinfo.dwXpos < 0x3fff) joy_status[i] |= 0x04;	// left
			if(joyinfo.dwXpos > 0xbfff) joy_status[i] |= 0x08;	// right
			joy_status[i] |= ((joyinfo.dwButtons & joy_mask[i]) << 4);
		}
	}
#endif
	
#ifdef USE_MOUSE
	// update mouse status
	memset(mouse_status, 0, sizeof(mouse_status));
	
	if(mouse_enabled) {
		// get current status
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(main_window_handle, &pt);
		mouse_status[0]  = pt.x - host_window_width / 2;
		mouse_status[1]  = pt.y - host_window_height / 2;
		mouse_status[2]  = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 2 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 4 : 0;
		// move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
			pt.x = host_window_width / 2;
			pt.y = host_window_height / 2;
			ClientToScreen(main_window_handle, &pt);
			SetCursorPos(pt.x, pt.y);
		}
	}
#endif
}

void OSD::key_down(int code, bool repeat)
{
	if(!dinput_key_available) {
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
#ifdef USE_SHIFT_NUMPAD_KEY
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
#endif
		key_down_native(code, repeat);
	} else {
		// caps, kana, kanji
		if(repeat || code == 0xf0 || code == 0xf2 || code == 0xf3 || code == 0xf4) {
			key_down_native(code, repeat);
		}
	}
}

void OSD::key_up(int code)
{
	if(!dinput_key_available) {
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
#ifdef USE_SHIFT_NUMPAD_KEY
		if(code == VK_LSHIFT) {
			key_shift_pressed = false;
			key_shift_released = true;
			return;
		} else if(key_converted[code] != 0) {
			key_converted[code] = 0;
			code = numpad_table[code];
		}
#endif
		key_up_native(code);
	}
}

void OSD::key_down_native(int code, bool repeat)
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
	
#ifdef DONT_KEEEP_KEY_PRESSED
	if(!(code == VK_LSHIFT || code == VK_RSHIFT || code == VK_LCONTROL || code == VK_RCONTROL || code == VK_LMENU || code == VK_RMENU)) {
		key_status[code] = KEY_KEEP_FRAMES;
	} else
#endif
	key_status[code] = keep_frames ? KEY_KEEP_FRAMES : 0x80;
	
#ifdef NOTIFY_KEY_DOWN
#ifndef NOTIFY_KEY_DOWN_LR_SHIFT
	uint8_t prev_shift = key_status[VK_SHIFT];
#endif
#ifndef NOTIFY_KEY_DOWN_LR_CONTROL
	uint8_t prev_control = key_status[VK_CONTROL];
#endif
#ifndef NOTIFY_KEY_DOWN_LR_MENU
	uint8_t prev_menu = key_status[VK_MENU];
#endif
#endif
	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];
	
#ifdef NOTIFY_KEY_DOWN
	if(keep_frames) {
		repeat = false;
	}
#ifndef NOTIFY_KEY_DOWN_LR_SHIFT
	if(code == VK_LSHIFT || code == VK_RSHIFT) {
		if(prev_shift == 0 && key_status[VK_SHIFT] != 0) {
			vm->key_down(VK_SHIFT, repeat);
		}
		return;
	}
#endif
#ifndef NOTIFY_KEY_DOWN_LR_CONTROL
	if(code == VK_LCONTROL|| code == VK_RCONTROL) {
		if(prev_control == 0 && key_status[VK_CONTROL] != 0) {
			vm->key_down(VK_CONTROL, repeat);
		}
		return;
	}
#endif
#ifndef NOTIFY_KEY_DOWN_LR_MENU
	if(code == VK_LMENU|| code == VK_RMENU) {
		if(prev_menu == 0 && key_status[VK_MENU] != 0) {
			vm->key_down(VK_MENU, repeat);
		}
		return;
	}
#endif
	vm->key_down(code, repeat);
#endif
}

void OSD::key_up_native(int code)
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
	
#ifdef NOTIFY_KEY_DOWN
#ifndef NOTIFY_KEY_DOWN_LR_SHIFT
	uint8_t prev_shift = key_status[VK_SHIFT];
#endif
#ifndef NOTIFY_KEY_DOWN_LR_CONTROL
	uint8_t prev_control = key_status[VK_CONTROL];
#endif
#ifndef NOTIFY_KEY_DOWN_LR_MENU
	uint8_t prev_menu = key_status[VK_MENU];
#endif
#endif
	key_status[VK_SHIFT] = key_status[VK_LSHIFT] | key_status[VK_RSHIFT];
	key_status[VK_CONTROL] = key_status[VK_LCONTROL] | key_status[VK_RCONTROL];
	key_status[VK_MENU] = key_status[VK_LMENU] | key_status[VK_RMENU];
	
#ifdef NOTIFY_KEY_DOWN
#ifndef NOTIFY_KEY_DOWN_LR_SHIFT
	if(code == VK_LSHIFT || code == VK_RSHIFT) {
		if(prev_shift != 0 && key_status[VK_SHIFT] == 0) {
			vm->key_up(VK_SHIFT);
		}
		return;
	}
#endif
#ifndef NOTIFY_KEY_DOWN_LR_CONTROL
	if(code == VK_LCONTROL|| code == VK_RCONTROL) {
		if(prev_control != 0 && key_status[VK_CONTROL] == 0) {
			vm->key_up(VK_CONTROL);
		}
		return;
	}
#endif
#ifndef NOTIFY_KEY_DOWN_LR_MENU
	if(code == VK_LMENU || code == VK_RMENU) {
		if(prev_menu != 0 && key_status[VK_MENU] == 0) {
			vm->key_up(VK_MENU);
		}
		return;
	}
#endif
	vm->key_up(code);
#endif
}

#ifdef USE_MOUSE
void OSD::enable_mouse()
{
	// enable mouse emulation
	if(!mouse_enabled) {
		// hide mouse cursor
		ShowCursor(FALSE);
		// move mouse cursor to the center of window
		POINT pt;
		pt.x = host_window_width / 2;
		pt.y = host_window_height / 2;
		ClientToScreen(main_window_handle, &pt);
		SetCursorPos(pt.x, pt.y);
	}
	mouse_enabled = true;
}

void OSD::disable_mouse()
{
	// disable mouse emulation
	if(mouse_enabled) {
		ShowCursor(TRUE);
	}
	mouse_enabled = false;
}

void OSD::toggle_mouse()
{
	// toggle mouse enable / disable
	if(mouse_enabled) {
		disable_mouse();
	} else {
		enable_mouse();
	}
}
#endif

