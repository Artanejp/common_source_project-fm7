/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 input ]
*/

#include "emu.h"
#include "vm/vm.h"
#include "fifo.h"
#include "fileio.h"

#define KEY_KEEP_FRAMES	3

// dummy dinput keycode
#define DIK_SHIFT	0xfd
#define DIK_CONTROL	0xfe
#define DIK_MENU	0xff

typedef struct {
	int lr_dik, l_dik, r_dik, l_vk, r_vk;
} lr_t;

static const lr_t lr[3] = {
	{DIK_SHIFT  , DIK_LSHIFT  , DIK_RSHIFT  , VK_LSHIFT  , VK_RSHIFT  },
	{DIK_CONTROL, DIK_LCONTROL, DIK_RCONTROL, VK_LCONTROL, VK_RCONTROL},
	{DIK_MENU   , DIK_LMENU   , DIK_RMENU   , VK_LMENU   , VK_RMENU   }
};

static const uint8 vk_dik[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x0f, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00,
	DIK_SHIFT, DIK_CONTROL, DIK_MENU,
	                  0xC5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x79, 0x7b, 0x00, 0x00,
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
//	0x2a, 0x36, 0x1d, 0x9d, 0x38, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// shift, ctrl, menu
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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

void EMU::initialize_input()
{
	// initialize status
	memset(key_status, 0, sizeof(key_status));
	memset(joy_status, 0, sizeof(joy_status));
	memset(mouse_status, 0, sizeof(mouse_status));
	
	// initialize direct input
	dinput_key_ok = false;
	lpdi = NULL;
	lpdikey = NULL;
	
	// XXX: no gui to change config.use_direct_inpu, so we need to modify *.ini file manually
	if(config.use_direct_input) {
		if (SUCCEEDED(DirectInput8Create(instance_handle, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID *)&lpdi, NULL))) {
			if(SUCCEEDED(lpdi->CreateDevice(GUID_SysKeyboard, &lpdikey, NULL))) {
				if(SUCCEEDED(lpdikey->SetDataFormat(&c_dfDIKeyboard))) {
					if(SUCCEEDED(lpdikey->SetCooperativeLevel(main_window_handle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
						dinput_key_ok = true;
						memset(key_dik_prev, 0, sizeof(key_dik_prev));
					}
				}
			}
		}
	}
	
	// initialize joysticks
	joy_num = joyGetNumDevs();
	for(int i = 0; i < joy_num && i < 2; i++) {
		JOYCAPS joycaps;
		if(joyGetDevCaps(i, &joycaps, sizeof(joycaps)) == JOYERR_NOERROR) {
			joy_mask[i] = (1 << joycaps.wNumButtons) - 1;
		} else {
			joy_mask[i] = 0x0f; // 4buttons
		}
	}
	
	// mouse emulation is disenabled
	mouse_enabled = false;
	
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

void EMU::update_input()
{
	if(dinput_key_ok) {
		// direct input
		static uint8 key_dik[256];
		lpdikey->Acquire();
		lpdikey->GetDeviceState(256, key_dik);
		
		// DIK_RSHIFT is not detected on Vista or later
		if(vista_or_later) {
			key_dik[DIK_RSHIFT] = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) ? 0x80 : 0;
		}
#ifdef USE_SHIFT_NUMPAD_KEY
		// XXX: don't release shift key while numpad key is pressed
		uint8 numpad_keys;
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
		
		for(int i = 0; i < 3; i++) {
			// left and right keys of shift, ctrl and alt
			if(key_dik[lr[i].l_dik] & 0x80) {
				key_status[lr[i].l_vk] = 0x80;
			} else {
				key_status[lr[i].l_vk] &= 0x7f;
			}
			if(key_dik[lr[i].r_dik] & 0x80) {
				key_status[lr[i].r_vk] = 0x80;
			} else {
				key_status[lr[i].r_vk] &= 0x7f;
			}
			key_dik[lr[i].lr_dik] = key_dik[lr[i].l_dik] | key_dik[lr[i].r_dik];
		}
		for(int vk = 0; vk < 256; vk++) {
			int dik = vk_dik[vk];
			if(dik) {
				if(key_dik[dik] & 0x80) {
					if(!(key_dik_prev[dik] & 0x80)) {
						key_down_sub(vk, false);
					}
				} else {
					if(key_dik_prev[dik] & 0x80) {
						key_up_sub(vk);
					}
				}
			}
		}
		memcpy(key_dik_prev, key_dik, sizeof(key_dik_prev));
#ifdef USE_SHIFT_NUMPAD_KEY
	} else {
		// update numpad key status
		if(key_shift_pressed && !key_shift_released) {
			if(key_status[VK_SHIFT] == 0) {
				// shift key is newly pressed
				key_status[VK_SHIFT] = 0x80;
#ifdef NOTIFY_KEY_DOWN
				vm->key_down(VK_SHIFT, false);
#endif
			}
		} else if(!key_shift_pressed && key_shift_released) {
			if(key_status[VK_SHIFT] != 0) {
				// shift key is newly released
				key_status[VK_SHIFT] = 0;
#ifdef NOTIFY_KEY_DOWN
				vm->key_up(VK_SHIFT);
#endif
				// check l/r shift
				if(!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
				if(!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
			}
		}
		key_shift_pressed = key_shift_released = false;
#endif
	}
	
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
	
	// update joystick status
	memset(joy_status, 0, sizeof(joy_status));
	for(int i = 0; i < joy_num && i < 2; i++) {
		JOYINFOEX joyinfo;
		joyinfo.dwSize = sizeof(JOYINFOEX);
		joyinfo.dwFlags = JOY_RETURNALL;
		if(joyGetPosEx(i, &joyinfo) == JOYERR_NOERROR) {
			if(joyinfo.dwYpos < 0x3fff) joy_status[i] |= 0x01;	// up
			if(joyinfo.dwYpos > 0xbfff) joy_status[i] |= 0x02;	// down
			if(joyinfo.dwXpos < 0x3fff) joy_status[i] |= 0x04;	// left
			if(joyinfo.dwXpos > 0xbfff) joy_status[i] |= 0x08;	// right
			joy_status[i] |= ((joyinfo.dwButtons & joy_mask[i]) << 4);
		}
	}
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
	
	// update mouse status
	memset(mouse_status, 0, sizeof(mouse_status));
	if(mouse_enabled) {
		// get current status
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(main_window_handle, &pt);
		mouse_status[0]  = pt.x - display_width / 2;
		mouse_status[1]  = pt.y - display_height / 2;
		mouse_status[2]  = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 2 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 4 : 0;
		// move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
			pt.x = display_width / 2;
			pt.y = display_height / 2;
			ClientToScreen(main_window_handle, &pt);
			SetCursorPos(pt.x, pt.y);
		}
	}
	
#ifdef USE_AUTO_KEY
#ifndef USE_AUTO_KEY_SHIFT
#define USE_AUTO_KEY_SHIFT 0
#endif
	// auto key
	switch(autokey_phase) {
	case 1:
		if(autokey_buffer && !autokey_buffer->empty()) {
			// update shift key status
			int shift = autokey_buffer->read_not_remove(0) & 0x100;
			if(shift && !autokey_shift) {
				key_down_sub(VK_SHIFT, false);
			} else if(!shift && autokey_shift) {
				key_up_sub(VK_SHIFT);
			}
			autokey_shift = shift;
			autokey_phase++;
			break;
		}
	case 3 + USE_AUTO_KEY_SHIFT:
		if(autokey_buffer && !autokey_buffer->empty()) {
			key_down_sub(autokey_buffer->read_not_remove(0) & 0xff, false);
		}
		autokey_phase++;
		break;
	case USE_AUTO_KEY + USE_AUTO_KEY_SHIFT:
		if(autokey_buffer && !autokey_buffer->empty()) {
			key_up_sub(autokey_buffer->read_not_remove(0) & 0xff);
		}
		autokey_phase++;
		break;
	case USE_AUTO_KEY_RELEASE + USE_AUTO_KEY_SHIFT:
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

void EMU::key_down(int code, bool repeat)
{
	if(!dinput_key_ok) {
		if(code == VK_SHIFT) {
			if(GetAsyncKeyState(VK_LSHIFT) & 0x8000) key_status[VK_LSHIFT] = 0x80;
			if(GetAsyncKeyState(VK_RSHIFT) & 0x8000) key_status[VK_RSHIFT] = 0x80;
			if(!(key_status[VK_LSHIFT] || key_status[VK_RSHIFT])) key_status[VK_LSHIFT] = 0x80;
		} else if(code == VK_CONTROL) {
			if(GetAsyncKeyState(VK_LCONTROL) & 0x8000) key_status[VK_LCONTROL] = 0x80;
			if(GetAsyncKeyState(VK_RCONTROL) & 0x8000) key_status[VK_RCONTROL] = 0x80;
			if(!(key_status[VK_LCONTROL] || key_status[VK_RCONTROL])) key_status[VK_LCONTROL] = 0x80;
		} else if(code == VK_MENU) {
			if(GetAsyncKeyState(VK_LMENU) & 0x8000) key_status[VK_LMENU] = 0x80;
			if(GetAsyncKeyState(VK_RMENU) & 0x8000) key_status[VK_RMENU] = 0x80;
			if(!(key_status[VK_LMENU] || key_status[VK_RMENU])) key_status[VK_LMENU] = 0x80;
		}
#ifdef USE_SHIFT_NUMPAD_KEY
		if(code == VK_SHIFT) {
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
		key_down_sub(code, repeat);
	} else {
		// caps, kana, kanji
		if(repeat || code == 0xf0 || code == 0xf2 || code == 0xf3 || code == 0xf4) {
			key_down_sub(code, repeat);
		}
	}
}

void EMU::key_up(int code)
{
	if(!dinput_key_ok) {
		if(code == VK_SHIFT) {
#ifndef USE_SHIFT_NUMPAD_KEY
			if(!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
			if(!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
#endif
		} else if(code == VK_CONTROL) {
			if(!(GetAsyncKeyState(VK_LCONTROL) & 0x8000)) key_status[VK_LCONTROL] &= 0x7f;
			if(!(GetAsyncKeyState(VK_RCONTROL) & 0x8000)) key_status[VK_RCONTROL] &= 0x7f;
		} else if(code == VK_MENU) {
			if(!(GetAsyncKeyState(VK_LMENU) & 0x8000)) key_status[VK_LMENU] &= 0x7f;
			if(!(GetAsyncKeyState(VK_RMENU) & 0x8000)) key_status[VK_RMENU] &= 0x7f;
		}
#ifdef USE_SHIFT_NUMPAD_KEY
		if(code == VK_SHIFT) {
			key_shift_pressed = false;
			key_shift_released = true;
			return;
		} else if(key_converted[code] != 0) {
			key_converted[code] = 0;
			code = numpad_table[code];
		}
#endif
		key_up_sub(code);
	}
}

void EMU::key_down_sub(int code, bool repeat)
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
	if(!(code == VK_SHIFT || code == VK_CONTROL || code == VK_MENU)) {
		code = keycode_conv[code];
	}
	
#ifdef DONT_KEEEP_KEY_PRESSED
	if(!(code == VK_SHIFT || code == VK_CONTROL || code == VK_MENU)) {
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

void EMU::key_up_sub(int code)
{
	if(!(code == VK_SHIFT || code == VK_CONTROL || code == VK_MENU)) {
		code = keycode_conv[code];
	}
	if(key_status[code]) {
		key_status[code] &= 0x7f;
#ifdef NOTIFY_KEY_DOWN
		if(!key_status[code]) {
			vm->key_up(code);
		}
#endif
	}
}

#ifdef USE_BUTTON
void EMU::press_button(int num)
{
	int code = buttons[num].code;
	
	if(code) {
		key_down_sub(code, false);
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
		// hide mouse cursor
		ShowCursor(FALSE);
		// move mouse cursor to the center of window
		POINT pt;
		pt.x = display_width / 2;
		pt.y = display_height / 2;
		ClientToScreen(main_window_handle, &pt);
		SetCursorPos(pt.x, pt.y);
	}
	mouse_enabled = true;
}

void EMU::disenable_mouse()
{
	// disenable mouse emulation
	if(mouse_enabled) {
		ShowCursor(TRUE);
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

void EMU::start_auto_key()
{
	stop_auto_key();
	
	if(OpenClipboard(NULL)) {
		HANDLE hClip = GetClipboardData(CF_TEXT);
		if(hClip) {
			autokey_buffer->clear();
			char* buf = (char*)GlobalLock(hClip);
			int size = strlen(buf), prev_kana = 0;
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
			GlobalUnlock(hClip);
			
			autokey_phase = 1;
			autokey_shift = 0;
		}
		CloseClipboard();
	}
}

void EMU::stop_auto_key()
{
	if(autokey_shift) {
		key_up_sub(VK_SHIFT);
	}
	autokey_phase = autokey_shift = 0;
}
#endif
