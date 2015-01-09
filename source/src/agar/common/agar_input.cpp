/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 input ]
*/

#include <agar/core.h>
#include <agar/gui.h>
#include "emu.h"
#include "vm/vm.h"
#include "fifo.h"
#include "fileio.h"
#include "agar_input.h"
#include "agar_main.h"

#ifndef Ulong
#define Ulong unsigned long
#endif

#define KEY_KEEP_FRAMES 3

extern "C" {
const struct WIndowsKeyTable  WindowsKeyMappings[] = {
	{ '0',			AG_KEY_0 },
	{ '1',			AG_KEY_1 },
	{ '2',			AG_KEY_2 },
	{ '3',			AG_KEY_3 },
	{ '4',			AG_KEY_4 },
	{ '5',			AG_KEY_5 },
	{ '6',			AG_KEY_6 },
	{ '7',			AG_KEY_7 },
	{ '8',			AG_KEY_8 },
	{ '9',			AG_KEY_9 },
	{ 'A',			AG_KEY_A },
	{ 'B',			AG_KEY_B },
	{ 'C',			AG_KEY_C },
	{ 'D',			AG_KEY_D },
	{ 'E',			AG_KEY_E },
	{ 'F',			AG_KEY_F },
	{ 'G',			AG_KEY_G },
	{ 'H',			AG_KEY_H },
	{ 'I',			AG_KEY_I },
	{ 'J',			AG_KEY_J },
	{ 'K',			AG_KEY_K },
	{ 'L',			AG_KEY_L },
	{ 'M',			AG_KEY_M },
	{ 'N',			AG_KEY_N },
	{ 'O',			AG_KEY_O },
	{ 'P',			AG_KEY_P },
	{ 'Q',			AG_KEY_Q },
	{ 'R',			AG_KEY_R },
	{ 'S',			AG_KEY_S },
	{ 'T',			AG_KEY_T },
	{ 'U',			AG_KEY_U },
	{ 'V',			AG_KEY_V },
	{ 'W',			AG_KEY_W },
	{ 'X',			AG_KEY_X },
	{ 'Y',			AG_KEY_Y },
	{ 'Z',			AG_KEY_Z },
	{ VK_F1,		AG_KEY_F1 },
	{ VK_F2,		AG_KEY_F2 },
	{ VK_F3,		AG_KEY_F3 },
	{ VK_F4,		AG_KEY_F4 },
	{ VK_F5,		AG_KEY_F5 },
	{ VK_F6,		AG_KEY_F6 },
	{ VK_F7,		AG_KEY_F7 },
	{ VK_F8,		AG_KEY_F8 },
	{ VK_F9,		AG_KEY_F9 },
	{ VK_F10,		AG_KEY_F10 },
	{ VK_F11,		AG_KEY_F11 },
	{ VK_F12,		AG_KEY_F12 },
	{ VK_F13,		AG_KEY_F13 },
	{ VK_F14,		AG_KEY_F14 },
	{ VK_F15,		AG_KEY_F15 },

	{ VK_BACK,		AG_KEY_BACKSPACE },
	{ VK_TAB,		AG_KEY_TAB },
	{ VK_CLEAR,		AG_KEY_CLEAR },
	{ VK_RETURN,		AG_KEY_RETURN },
	{ VK_PAUSE,		AG_KEY_PAUSE },
	{ VK_ESCAPE,		AG_KEY_ESCAPE },
	{ VK_SPACE,		AG_KEY_SPACE },
	{ VK_DELETE,		AG_KEY_DELETE },
	{ VK_UP,		AG_KEY_UP },
	{ VK_DOWN,		AG_KEY_DOWN },
	{ VK_RIGHT,		AG_KEY_RIGHT },
	{ VK_LEFT,		AG_KEY_LEFT },
	{ VK_INSERT,		AG_KEY_INSERT },
	{ VK_HOME,		AG_KEY_HOME },
	{ VK_END,		AG_KEY_END },
	{ VK_PRIOR,		AG_KEY_PAGEUP },
	{ VK_NEXT,		AG_KEY_PAGEDOWN },

	{ VK_NUMPAD0,		AG_KEY_KP0 },
	{ VK_NUMPAD1,		AG_KEY_KP1 },
	{ VK_NUMPAD2,		AG_KEY_KP2 },
	{ VK_NUMPAD3,		AG_KEY_KP3 },
	{ VK_NUMPAD4,		AG_KEY_KP4 },
	{ VK_NUMPAD5,		AG_KEY_KP5 },
	{ VK_NUMPAD6,		AG_KEY_KP6 },
	{ VK_NUMPAD7,		AG_KEY_KP7 },
	{ VK_NUMPAD8,		AG_KEY_KP8 },
	{ VK_NUMPAD9,		AG_KEY_KP9 },
	{ VK_DECIMAL,		AG_KEY_KP_PERIOD },
	{ VK_DIVIDE,		AG_KEY_KP_DIVIDE },
	{ VK_MULTIPLY,		AG_KEY_KP_MULTIPLY },
	{ VK_SUBTRACT,		AG_KEY_KP_MINUS },
	{ VK_ADD,		AG_KEY_KP_PLUS },

	{ VK_NUMLOCK,		AG_KEY_NUMLOCK },
	{ VK_CAPITAL,		AG_KEY_CAPSLOCK },
	{ VK_SCROLL,		AG_KEY_SCROLLOCK },
	{ VK_SHIFT,		AG_KEY_RSHIFT },
	{ VK_RCONTROL,		AG_KEY_RCTRL },
	{ VK_LCONTROL,		AG_KEY_LCTRL },
	{ VK_RMENU,		AG_KEY_RALT },
	{ VK_LMENU,		AG_KEY_LALT },
	{ VK_RWIN,		AG_KEY_RSUPER },
	{ VK_LWIN,		AG_KEY_LSUPER },
	{ VK_HELP,		AG_KEY_HELP },
#ifdef VK_PRINT
	{ VK_PRINT,		AG_KEY_PRINT },
#endif
	{ VK_SNAPSHOT,		AG_KEY_PRINT },
	{ VK_CANCEL,		AG_KEY_BREAK },
	{ VK_APPS,		AG_KEY_MENU },
	{ 0xBA,			AG_KEY_SEMICOLON },
	{ 0xBC,			AG_KEY_COMMA },
	{ 0xBD,			AG_KEY_MINUS },
	{ 0xBE,			AG_KEY_PERIOD },
	{ 0xBF,			AG_KEY_SLASH },
	{ 0xBB,			AG_KEY_EQUALS },
	{ 0xC0,			AG_KEY_BACKQUOTE },
	{ 0xDB,			AG_KEY_LEFTBRACKET },
	{ 0xDC,			AG_KEY_BACKSLASH },
	{ 0xDD,			AG_KEY_RIGHTBRACKET },
	{ 0xDE,			AG_KEY_QUOTE },
	{ 0xDF,			AG_KEY_BACKQUOTE },
	{ 0xE2,			AG_KEY_LESS },
        { 0xffff, 0xffff}
};


static int mouse_x = 0;
static int mouse_y = 0;
static int mouse_relx = 0;
static int mouse_rely = 0;
static uint32 mouse_buttons = 0;
   
   
void ProcessKeyUp(AG_Event *event)
{
  int key = AG_INT(1);
  int mod = AG_INT(2);
  uint32_t unicode = AG_ULONG(3);
  
//#ifdef USE_BUTTON
  emu->key_up(key);

//#endif
}

void ProcessKeyDown(AG_Event *event)
{
  int key = AG_INT(1);
  int mod = AG_INT(2);
  uint32_t unicode = AG_ULONG(3);
//#ifdef USE_BUTTON
  emu->key_down(key, false);
//#endif
}

void OnMouseMotion(AG_Event *event)
{
  // Need lock?
  int x = AG_INT(1);
  int y = AG_INT(2);
  mouse_relx = AG_INT(3);
  mouse_rely = AG_INT(4);
  int buttons = AG_INT(5);

  if((hScreenWidget != NULL) && (emu != NULL)){
    //mouse_x = (x * emu->screen_width)  /  hScreenWidget->w;
    //mouse_y = (y * emu->screen_height) /  hScreenWidget->h;
    mouse_x = x;
    mouse_y = y;
  }
  // Need Unlock?
}

void OnMouseButtonDown(AG_Event *event)
{
  // Need Lock?
  int buttons = AG_INT(1);
  switch (buttons){
  case AG_MOUSE_NONE:
    break;
  case AG_MOUSE_LEFT:
    mouse_buttons |= UI_MOUSE_LEFT;
    break;
  case AG_MOUSE_MIDDLE:
    mouse_buttons |= UI_MOUSE_MIDDLE;
    break;
  case AG_MOUSE_RIGHT:
    mouse_buttons |= UI_MOUSE_RIGHT;
    break;
  case AG_MOUSE_X1:
    mouse_buttons |= UI_MOUSE_X1;
    break;
  case AG_MOUSE_X2:
    mouse_buttons |= UI_MOUSE_X2;
    break;
  case AG_MOUSE_WHEELUP:
    mouse_buttons |= UI_MOUSE_WHEELUP;
    break;
  case AG_MOUSE_WHEELDOWN:
    mouse_buttons |= UI_MOUSE_WHEELDOWN;
    break;
  default:
    break;
  }
  // Need Unlock?
}

void OnMouseButtonUp(AG_Event *event)
{
  // Need Lock?
  int buttons = AG_INT(1);
  switch (buttons){
  case AG_MOUSE_NONE:
    break;
  case AG_MOUSE_LEFT:
    mouse_buttons &= ~UI_MOUSE_LEFT;
    break;
  case AG_MOUSE_MIDDLE:
    mouse_buttons &= ~UI_MOUSE_MIDDLE;
    break;
  case AG_MOUSE_RIGHT:
    mouse_buttons &= ~UI_MOUSE_RIGHT;
    break;
  case AG_MOUSE_X1:
    mouse_buttons &= ~UI_MOUSE_X1;
    break;
  case AG_MOUSE_X2:
    mouse_buttons &= ~UI_MOUSE_X2;
    break;
  case AG_MOUSE_WHEELUP:
    mouse_buttons &= ~UI_MOUSE_WHEELUP;
    break;
  case AG_MOUSE_WHEELDOWN:
    mouse_buttons &= ~UI_MOUSE_WHEELDOWN;
    break;
  default:
    break;
  }
}


   
uint32 GetAsyncKeyState(uint32 vk)
{
   vk = vk & 0xff; // OK?
   uint32_t modstate;
   
   if(hScreenWidget == NULL) return 0;
   modstate =  AG_GetModState(hScreenWidget);

   switch(vk) {
    case VK_LSHIFT:
      if((modstate & AG_KEYMOD_LSHIFT) != 0) return 0xffffffff;
      break;
    case VK_RSHIFT:
      if((modstate & AG_KEYMOD_RSHIFT) != 0) return 0xffffffff;
      break;
    case VK_LCONTROL:
      if((modstate & AG_KEYMOD_LCTRL) != 0) return 0xffffffff;
      break;
    case VK_RCONTROL:
      if((modstate & AG_KEYMOD_RCTRL) != 0) return 0xffffffff;
      break;
    case VK_LMENU:
      if((modstate & AG_KEYMOD_LALT) != 0) return 0xffffffff;
      break;
    case VK_RMENU:
      if((modstate & AG_KEYMOD_RALT) != 0) return 0xffffffff;
      break;
    default:
      break;
   }
   return 0;
}

uint8_t convert_AGKey2VK(int sym)
{
   uint32 n;
   int i = 0;
   do {
      if(WindowsKeyMappings[i].agkey == sym) {
	   n = WindowsKeyMappings[i].vk;
	   return (uint8_t)n;
      }
      
      i++;
   } while(WindowsKeyMappings[i].vk != 0xffff);
   return 0;
}
   
}


void EMU::initialize_input()
{
	// initialize status
	memset(key_status, 0, sizeof(key_status));
	memset(joy_status, 0, sizeof(joy_status));
	memset(mouse_status, 0, sizeof(mouse_status));
	
	// initialize joysticks
#if 0
        joy_num = joyGetNumDevs();
	for(int i = 0; i < joy_num && i < 2; i++) {
		JOYCAPS joycaps;
		if(joyGetDevCaps(i, &joycaps, sizeof(joycaps)) == JOYERR_NOERROR) {
			joy_mask[i] = (1 << joycaps.wNumButtons) - 1;
		} else {
			joy_mask[i] = 0x0f; // 4buttons
		}
	}
#else
        joy_num = 0;
#endif	
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
}

void EMU::update_input()
{

    int *keystat;
    int i_c = 0;;
   

# ifdef USE_SHIFT_NUMPAD_KEY
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
#if 0	
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

#endif
	// update mouse status
	memset(mouse_status, 0, sizeof(mouse_status));
#if 0
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
		 move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
			pt.x = display_width / 2;
			pt.y = display_height / 2;
		//	ClientToScreen(main_window_handle, &pt);
		//	SetCursorPos(pt.x, pt.y);
		}
	}
#endif

#if 0
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
	uint8 code;
        code = convert_AGKey2VK(sym);
   //printf("Key down %03x %03x\n", sym, code);
#if 1
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
#endif
}

void EMU::key_up(int sym)
{
	uint8 code;
        code = convert_AGKey2VK(sym);
   //printf("Key up %03x %03x\n", sym, code);
#if 1
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
	} else {
	   key_status[code] &= 0x7f;
	   vm->key_up(code);
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
#endif
}

#ifdef USE_BUTTON
void EMU::press_button(int num)
{
#if 1
	int code = buttons[num].code;
	
	if(code) {
		key_down(code, false);
		key_status[code] = KEY_KEEP_FRAMES;
	} else {
		// code=0: reset virtual machine
		vm->reset();
	}
#endif
}
#endif

void EMU::enable_mouse()
{
	// enable mouse emulation
	if(!mouse_enabled) {
#if 0
	        // hide mouse cursor
		ShowCursor(FALSE);
		// move mouse cursor to the center of window
		POINT pt;
		pt.x = display_width / 2;
		pt.y = display_height / 2;
		ClientToScreen(main_window_handle, &pt);
		SetCursorPos(pt.x, pt.y);
#endif
	}
	mouse_enabled = true;

}



void EMU::disenable_mouse()
{
#if 0
	// disenable mouse emulation
	if(mouse_enabled) {
		ShowCursor(TRUE);
	}
#endif
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
#if 0
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
#endif
}

void EMU::stop_auto_key()
{
#if 1
        if(autokey_shift) {
		key_up(VK_SHIFT);
	}
	autokey_phase = autokey_shift = 0;
#endif
}
	   
#endif
