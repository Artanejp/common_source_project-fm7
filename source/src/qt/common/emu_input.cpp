/*
 *	Skelton for retropc emulator
 *
 *	Author : Takeda.Toshiya
 *	Date   : 2006.08.18 -
 *      Converted to QT by (C) 2015 K.Ohta
 *         History:
 *            Jan 12, 2015 (maybe) : Initial
 *	[ SDL input -> Keyboard]
*/

#include <Qt>
#include <SDL2/SDL.h>
#include "emu.h"
#include "vm/vm.h"
#include "fifo.h"
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

extern EMU* emu;

struct SDLKeyTable  SDLKeyMappings[] = {
	{ '0',			SDL_SCANCODE_0 },
	{ '1',			SDL_SCANCODE_1 },
	{ '2',			SDL_SCANCODE_2 },
	{ '3',			SDL_SCANCODE_3 },
	{ '4',			SDL_SCANCODE_4 },
	{ '5',			SDL_SCANCODE_5 },
	{ '6',			SDL_SCANCODE_6 },
	{ '7',			SDL_SCANCODE_7 },
	{ '8',			SDL_SCANCODE_8 },
	{ '9',			SDL_SCANCODE_9 },
	{ 'A',			SDL_SCANCODE_A },
	{ 'B',			SDL_SCANCODE_B },
	{ 'C',			SDL_SCANCODE_C },
	{ 'D',			SDL_SCANCODE_D },
	{ 'E',			SDL_SCANCODE_E },
	{ 'F',			SDL_SCANCODE_F },
	{ 'G',			SDL_SCANCODE_G },
	{ 'H',			SDL_SCANCODE_H },
	{ 'I',			SDL_SCANCODE_I },
	{ 'J',			SDL_SCANCODE_J },
	{ 'K',			SDL_SCANCODE_K },
	{ 'L',			SDL_SCANCODE_L },
	{ 'M',			SDL_SCANCODE_M },
	{ 'N',			SDL_SCANCODE_N },
	{ 'O',			SDL_SCANCODE_O },
	{ 'P',			SDL_SCANCODE_P },
	{ 'Q',			SDL_SCANCODE_Q },
	{ 'R',			SDL_SCANCODE_R },
	{ 'S',			SDL_SCANCODE_S },
	{ 'T',			SDL_SCANCODE_T },
	{ 'U',			SDL_SCANCODE_U },
	{ 'V',			SDL_SCANCODE_V },
	{ 'W',			SDL_SCANCODE_W },
	{ 'X',			SDL_SCANCODE_X },
	{ 'Y',			SDL_SCANCODE_Y },
	{ 'Z',			SDL_SCANCODE_Z },
        // Start Qt's workaround: Qt returns character directry when keyin/out.
	// Excepts(maybe) 'a' to 'z'?
	// So, you should change keycode, using other than 109 / 106 Keyboard.
//        {0xBA,			SDL_SCANCODE_ASTERISK}, // $2a
//        {0xBB,			SDL_SCANCODE_PLUS}, // $2b
//        {0xBC,                  SDL_SCANCODE_LESS}, // ,
//        {0xBD,                  SDL_SCANCODE_CARET}, // ^~
//        {0xBE,                  SDL_SCANCODE_GREATER}, //$2e
//        {0xBF,                  SDL_SCANCODE_QUSETION}, //$2f
        //{0xC0,                  SDL_SCANCODE_BACKQUOTE}, //`
//        {0xE2,			SDL_SCANCODE_UNDERSCORE},//_\
        {0xE2,			SDL_SCANCODE_INTERNATIONAL3},//_\
        // End.
	{ VK_F1,		SDL_SCANCODE_F1 },
	{ VK_F2,		SDL_SCANCODE_F2 },
	{ VK_F3,		SDL_SCANCODE_F3 },
	{ VK_F4,		SDL_SCANCODE_F4 },
	{ VK_F5,		SDL_SCANCODE_F5 },
	{ VK_F6,		SDL_SCANCODE_F6 },
	{ VK_F7,		SDL_SCANCODE_F7 },
	{ VK_F8,		SDL_SCANCODE_F8 },
	{ VK_F9,		SDL_SCANCODE_F9 },
	{ VK_F10,		SDL_SCANCODE_F10 },
	{ VK_F11,		SDL_SCANCODE_F11 },
	{ VK_F12,		SDL_SCANCODE_F12 },
	{ VK_F13,		SDL_SCANCODE_F13 },
	{ VK_F14,		SDL_SCANCODE_F14 },
	{ VK_F15,		SDL_SCANCODE_F15 },
	{ VK_BACK,		SDL_SCANCODE_BACKSPACE },
	{ VK_TAB,		SDL_SCANCODE_TAB },
	{ VK_CLEAR,		SDL_SCANCODE_CLEAR },
	{ VK_RETURN,		SDL_SCANCODE_RETURN },
	{ VK_PAUSE,		SDL_SCANCODE_PAUSE },
	{ VK_ESCAPE,		SDL_SCANCODE_ESCAPE },
	{ VK_SPACE,		SDL_SCANCODE_SPACE },
	{ VK_DELETE,		SDL_SCANCODE_DELETE },
	{ VK_UP,		SDL_SCANCODE_UP },
	{ VK_DOWN,		SDL_SCANCODE_DOWN},
	{ VK_RIGHT,		SDL_SCANCODE_RIGHT },
	{ VK_LEFT,		SDL_SCANCODE_LEFT },
	{ VK_INSERT,		SDL_SCANCODE_INSERT },
	{ VK_HOME,		SDL_SCANCODE_HOME },
	{ VK_END,		SDL_SCANCODE_END },
	{ VK_PRIOR,		SDL_SCANCODE_PAGEUP },
	{ VK_NEXT,		SDL_SCANCODE_PAGEDOWN },

	{ VK_NUMPAD0,		SDL_SCANCODE_KP_0 },
	{ VK_NUMPAD1,		SDL_SCANCODE_KP_1 },
	{ VK_NUMPAD2,		SDL_SCANCODE_KP_2 },
	{ VK_NUMPAD3,		SDL_SCANCODE_KP_3 },
	{ VK_NUMPAD4,		SDL_SCANCODE_KP_4 },
	{ VK_NUMPAD5,		SDL_SCANCODE_KP_5 },
	{ VK_NUMPAD6,		SDL_SCANCODE_KP_6 },
	{ VK_NUMPAD7,		SDL_SCANCODE_KP_7 },
	{ VK_NUMPAD8,		SDL_SCANCODE_KP_8 },
	{ VK_NUMPAD9,		SDL_SCANCODE_KP_9 },

	{ VK_DECIMAL,		SDL_SCANCODE_KP_PERIOD },
	{ VK_DIVIDE,		SDL_SCANCODE_KP_DIVIDE},
	{ VK_MULTIPLY,		SDL_SCANCODE_KP_MULTIPLY },
	{ VK_SUBTRACT,		SDL_SCANCODE_KP_MINUS },
	{ VK_ADD,		SDL_SCANCODE_KP_PLUS },

	{ VK_NUMLOCK,		SDL_SCANCODE_NUMLOCKCLEAR },
//	{ VK_CAPITAL,		SDL_SCANCODE_Henkan }, // Need check
	{ VK_CAPITAL,		SDL_SCANCODE_CAPSLOCK}, // Need check
	{ VK_SCROLL,		SDL_SCANCODE_SCROLLLOCK },
//	{ VK_SHIFT,		SDL_SCANCODE_LSHIFT }, // Left
	{ VK_RSHIFT,		SDL_SCANCODE_RSHIFT }, // Right
	{ VK_LSHIFT,		SDL_SCANCODE_LSHIFT }, // Right
//	{ VK_CONTROL,		SDL_SCANCODE_CTRL }, // Right
	{ VK_RCONTROL,		SDL_SCANCODE_RCTRL }, // Right
	{ VK_LCONTROL,		SDL_SCANCODE_LCTRL }, // Left
	{ VK_RMENU,		SDL_SCANCODE_RALT },  // Right
	{ VK_LMENU,		SDL_SCANCODE_LALT },  // Left
	{ VK_MENU,		SDL_SCANCODE_MENU },  // Right
	{ VK_RWIN,		SDL_SCANCODE_RGUI },
	{ VK_LWIN,		SDL_SCANCODE_LGUI },
	{ VK_HELP,		SDL_SCANCODE_HELP }, // Right?
#ifdef VK_PRINT
	{ VK_PRINT,		SDL_SCANCODE_PRINTSCREEN },
#endif
	{ VK_SNAPSHOT,		SDL_SCANCODE_PRINTSCREEN },
	{ VK_CANCEL,		SDL_SCANCODE_PAUSE },
	{ VK_APPS,		SDL_SCANCODE_APPLICATION },
        { 0xBA,			SDL_SCANCODE_KP_COLON },
        { 0xBB,			SDL_SCANCODE_SEMICOLON },
//        { 0xBB,			SDL_SCANCODE_KP_SEMICOLON },
	{ 0xBC,			SDL_SCANCODE_COMMA },
	{ 0xBD,			SDL_SCANCODE_MINUS },//
	{ 0xBE,			SDL_SCANCODE_PERIOD },//
	{ 0xBF,			SDL_SCANCODE_SLASH },//
	{ 0xBB,			SDL_SCANCODE_EQUALS },//
	{ 0xC0,			SDL_SCANCODE_KP_AT },
	{ 0xDB,			SDL_SCANCODE_LEFTBRACKET },//]
	{ 0xDC,			SDL_SCANCODE_BACKSLASH },  // Okay?
	{ 0xDD,			SDL_SCANCODE_RIGHTBRACKET }, //[
	{ 0xDE,			SDL_SCANCODE_NONUSBACKSLASH }, // ^
//	{ 0xDF,			SDL_SCANCODE_QuoteLeft },

        // VK_CAPITAL 
	{ 0xF0,			SDL_SCANCODE_CAPSLOCK },
        // VK_KANA 
	{ 0xF2,			SDL_SCANCODE_LANG3 },
	{ 0xF2,			SDL_SCANCODE_LANG4 },
        // VK_KANJI 
	{ 0xF3,			SDL_SCANCODE_LANG5 },
//	{ 0xF4,			SDL_SCANCODE_Hankaku },
   
        { 0xffffffff, 0xffffffff}
};




uint32_t convert_SDLKey2VK(uint32_t sym)
{
   uint32 n = 0;
   int i = 0;
   do {
      if(SDLKeyMappings[i].sdlkey == sym) {
	   n = SDLKeyMappings[i].vk;
	   break;
      }
      
      i++;
   } while(QtKeyMappings[i].vk != 0xffffffff);
   //if((n == VK_LSHIFT) || (n == VK_RSHIFT)) n = VK_SHIFT;
   //if((n == VK_LCTRL) || (n == VK_RCTRL)) n = VK_CONTROL;
   //if((n == VL_RMENU) || (n == VK_RMENU)) n = VK_MENU;
   return n;
}

void EMU::initialize_input()
{
	// initialize status
	memset(key_status, 0, sizeof(key_status));
	memset(joy_status, 0, sizeof(joy_status));
	memset(mouse_status, 0, sizeof(mouse_status));
	
	// initialize joysticks
	// mouse emulation is disenabled
	mouse_enabled = false;
        joy_num = SDL_NumJoysticks();
        for(int i = 0; i < joy_num && i < 2; i++) {
	   //SDL_Joystick *joycaps = SDL_JoystickOpen(i);
	   //if(joycaps != NULL) {
	   //   joy_mask[i] = (1 << SDL_JoystickNumButtons(joycaps)) - 1;
	   //   SDL_JoystickClose(joycaps);
	   //} else {
	      joy_mask[i] = 0x0f; // 4buttons
	   //}
  
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
	 if(!(GetAsyncKeyState(VK_LSHIFT, modkey_status) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
	 if(!(GetAsyncKeyState(VK_RSHIFT, modkey_status) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
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
#if 1	
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
		mouse_status[2]  = (GetAsyncKeyState(VK_LBUTTON, modkey_status) & 0x8000) ? 1 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_RBUTTON, modkey_status) & 0x8000) ? 2 : 0;
		mouse_status[2] |= (GetAsyncKeyState(VK_MBUTTON, modkey_status) & 0x8000) ? 4 : 0;
		 move mouse cursor to the center of window
		if(!(mouse_status[0] == 0 && mouse_status[1] == 0)) {
			pt.x = display_width / 2;
			pt.y = display_height / 2;
		//	ClientToScreen(main_window_handle, &pt);
		//	SetCursorPos(pt.x, pt.y);
		}
	}
#endif

#if 1
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
	uint8 code = sym;
//        code = convert_AGKey2VK(sym);

         //printf("Key down %08x\n", sym);

#if 1  // No needed with SDL?
       if(code == VK_SHIFT){
#ifndef USE_SHIFT_NUMPAD_KEY
		if(GetAsyncKeyState(VK_LSHIFT, modkey_status) & 0x8000) key_status[VK_LSHIFT] = 0x80;
		if(GetAsyncKeyState(VK_RSHIFT, modkey_status) & 0x8000) key_status[VK_RSHIFT] = 0x80;
		if(GetAsyncKeyState(VK_SHIFT, modkey_status) & 0x8000) key_status[VK_LSHIFT] = 0x80;
		if(!(key_status[VK_LSHIFT] || key_status[VK_RSHIFT])) key_status[VK_LSHIFT] = 0x80;
#endif
       } else if(code == VK_LSHIFT){
#ifndef USE_SHIFT_NUMPAD_KEY
		if(GetAsyncKeyState(VK_LSHIFT, modkey_status) & 0x8000) key_status[VK_LSHIFT] = 0x80;
//		if(GetAsyncKeyState(VK_RSHIFT, modkey_status) & 0x8000) key_status[VK_RSHIFT] = 0x80;
		if(!(key_status[VK_LSHIFT] || key_status[VK_RSHIFT])) key_status[VK_LSHIFT] = 0x80;
#endif
	} else if(code == VK_RSHIFT){
#ifndef USE_SHIFT_NUMPAD_KEY
//		if(GetAsyncKeyState(VK_LSHIFT, modkey_status) & 0x8000) key_status[VK_LSHIFT] = 0x80;
		if(GetAsyncKeyState(VK_RSHIFT, modkey_status) & 0x8000) key_status[VK_RSHIFT] = 0x80;
		if(!(key_status[VK_LSHIFT] || key_status[VK_RSHIFT])) key_status[VK_LSHIFT] = 0x80;
#endif
	} else if(code == VK_CONTROL) {
		if(GetAsyncKeyState(VK_LCONTROL, modkey_status) & 0x8000) key_status[VK_LCONTROL] = 0x80;
		if(GetAsyncKeyState(VK_RCONTROL, modkey_status) & 0x8000) key_status[VK_RCONTROL] = 0x80;
		if(!(key_status[VK_LCONTROL] || key_status[VK_RCONTROL])) key_status[VK_LCONTROL] = 0x80;
	} else if(code == VK_MENU) {
		if(GetAsyncKeyState(VK_LMENU, modkey_status) & 0x8000) key_status[VK_LMENU] = 0x80;
		if(GetAsyncKeyState(VK_RMENU, modkey_status) & 0x8000) key_status[VK_RMENU] = 0x80;
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
#else  //SDL
       if((code == VK_LSHIFT) || (code == VK_RSHIFT)) {
       		key_status[code] = 0x80;
		if(!(key_status[VK_LSHIFT] || key_status[VK_RSHIFT])) key_status[VK_LSHIFT] = 0x80;
       } else if(code == VK_SHIFT) {
       		key_status[code] = 0x80;
		if(!(key_status[VK_LSHIFT] || key_status[VK_RSHIFT])) key_status[VK_LSHIFT] = 0x80;
       } else if((code == VK_LCONTROL) || (code == VK_RCONTROL)) {
       		key_status[code] = 0x80;
       } else if(code == VK_CONTROL) {
       		key_status[code] = 0x80;
       		key_status[VK_LCONTROL] = 0x80;
       } else if((code == VK_LMENU) || (code == VK_RMENU)) {
       		key_status[code] = 0x80;
       } else if(code == VK_MENU) {
       		key_status[code] = 0x80;
       		key_status[VK_LMENU] = 0x80;
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
#endif
   
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

       if(!(code == VK_CONTROL || code == VK_MENU || code == VK_SHIFT)) {
		code = keycode_conv[code];
	}
	
#ifdef DONT_KEEEP_KEY_PRESSED
	if(!(code == VK_CONTROL || code == VK_MENU || code == VK_SHIFT)) {
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
//        code = convert_AGKey2VK(sym);
   //printf("Key up %03x %03x\n", sym, code);
#if 1
   if(code == VK_SHIFT) {
#ifndef USE_SHIFT_NUMPAD_KEY
		if(!(GetAsyncKeyState(VK_SHIFT, modkey_status) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
//		if(!(GetAsyncKeyState(VK_RSHIFT, modkey_status) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
#endif
	} else if(code == VK_LSHIFT) {
#ifndef USE_SHIFT_NUMPAD_KEY
		if(!(GetAsyncKeyState(VK_LSHIFT, modkey_status) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
//		if(!(GetAsyncKeyState(VK_RSHIFT, modkey_status) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
#endif
	} else if(code == VK_RSHIFT) {
#ifndef USE_SHIFT_NUMPAD_KEY
//		if(!(GetAsyncKeyState(VK_LSHIFT, modkey_status) & 0x8000)) key_status[VK_LSHIFT] &= 0x7f;
		if(!(GetAsyncKeyState(VK_RSHIFT, modkey_status) & 0x8000)) key_status[VK_RSHIFT] &= 0x7f;
#endif
	} else if(code == VK_CONTROL) {
		if(!(GetAsyncKeyState(VK_LCONTROL, modkey_status) & 0x8000)) key_status[VK_LCONTROL] &= 0x7f;
		if(!(GetAsyncKeyState(VK_RCONTROL, modkey_status) & 0x8000)) key_status[VK_RCONTROL] &= 0x7f;
	} else if(code == VK_MENU) {
		if(!(GetAsyncKeyState(VK_LMENU, modkey_status) & 0x8000)) key_status[VK_LMENU] &= 0x7f;
		if(!(GetAsyncKeyState(VK_RMENU, modkey_status) & 0x8000)) key_status[VK_RMENU] &= 0x7f;
	} else
#else 
       if((code == VK_LSHIFT) || (code == VK_RSHIFT)) {
       		key_status[code] &= 0x7f;
       } else if(code == VK_SHIFT) {
       		key_status[code] &= 0x7f;
       		key_status[VK_LSHIFT] &= 0x7f;
       } else if((code == VK_LCONTROL) || (code == VK_RCONTROL)) {
       		key_status[code] &= 0x7f;
       } else if(code == VK_CONTROL) {
       		key_status[code] &= 0x7f;
       		key_status[VK_LCONTROL] &= 0x7f;
       } else if((code == VK_LMENU) || (code == VK_RMENU)) {
       		key_status[code] &= 0x7f;
       } else if(code == VK_MENU) {
       		key_status[code] &= 0x7f;
       		key_status[VK_LMENU] &= 0x7f;
       } else
#endif
     {
	   key_status[code] &= 0x7f;
#ifdef NOTIFY_KEY_DOWN
	   vm->key_up(code);
#endif
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

 
 JoyThreadClass::JoyThreadClass(QObject *parent) : QThread(parent)
 {
   int i;
   for(i = 0; i < 2; i++) joyhandle[i] = SDL_JoystickOpen(i);
     joy_num = SDL_NumJoysticks();
     AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : Start.");
     bRunThread = true;
 }
 
 JoyThreadClass::~JoyThreadClass()
 {
  int i;
    for(i = 0; i < 2; i++) {
      if(joyhandle[i] != NULL) SDL_JoystickClose(joyhandle[i]);
    }
    AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : EXIT");
 }
 
void JoyThreadClass::x_axis_changed(int index, int value)
{
   if(p_emu == NULL) return;
   p_emu->LockVM();
   uint32_t *joy_status = p_emu->getJoyStatPtr();
   
   if(joy_status != NULL) {
      if(value < -8192) { // left
	 joy_status[index] |= 0x04; joy_status[index] &= ~0x08;
      } else if(value > 8192)  { // right
	 joy_status[index] |= 0x08; joy_status[index] &= ~0x04;
      }  else { // center
	 joy_status[index] &= ~0x0c;
      }
   }
   
   p_emu->UnlockVM();
}
	   
void JoyThreadClass::y_axis_changed(int index, int value)
{
   if(p_emu == NULL) return;
   p_emu->LockVM();
   uint32_t *joy_status = p_emu->getJoyStatPtr();
   
   if(joy_status != NULL) {
      if(value < -8192) {// up
	 joy_status[index] |= 0x01; joy_status[index] &= ~0x02;
      } else if(value > 8192)  {// down 
	 joy_status[index] |= 0x02; joy_status[index] &= ~0x01;
      } else {
	 joy_status[index] &= ~0x03;
      }
   }
   
   p_emu->UnlockVM();
}

void JoyThreadClass::button_down(int index, unsigned int button)
{
      if(p_emu == NULL) return;
      p_emu->LockVM();
      uint32_t *joy_status = p_emu->getJoyStatPtr();
      if(joy_status != NULL) {
	 joy_status[index] |= (1 << (button + 4));
      }
      p_emu->UnlockVM();
}

void JoyThreadClass::button_up(int index, unsigned int button)
{
   if(p_emu == NULL) return;
   
   p_emu->LockVM();
   uint32_t *joy_status = p_emu->getJoyStatPtr();
      if(joy_status != NULL) {
	 joy_status[index] &= ~(1 << (button + 4));
      }
   p_emu->UnlockVM();
}
	   
// SDL Event Handler
bool  JoyThreadClass::EventSDL(SDL_Event *eventQueue)
{
   //	SDL_Surface *p;
   Sint16 value;
   unsigned int button;
   int vk;
   uint32_t sym;
   int i;
   if(eventQueue == NULL) return false;
	/*
	 * JoyStickなどはSDLが管理する
	 */
   switch (eventQueue->type){
    case SDL_JOYAXISMOTION:
      value = eventQueue->jaxis.value;
      i = eventQueue->jaxis.which;
      if((i < 0) || (i > 1)) break;

      if(eventQueue->jaxis.axis == 0) { // X
	 x_axis_changed(i, value);
      } else if(eventQueue->jaxis.axis == 1) { // Y
	 y_axis_changed(i, value);
      }
      break;
    case SDL_JOYBUTTONDOWN:
      button = eventQueue->jbutton.button;
      i = eventQueue->jbutton.which;
      if((i < 0) || (i > 1)) break;
      button_down(i, button);
      break;
    case SDL_JOYBUTTONUP:
      button = eventQueue->jbutton.button;
      i = eventQueue->jbutton.which;
      if((i < 0) || (i > 1)) break;
      button_up(i, button);
      break;
    default:
      break;
   }
   return true;
}


void JoyThreadClass::doWork(const QString &params)
{
  do {
  if(bRunThread == false) {
    return;
  }
  while(SDL_PollEvent(&event) == 1) {
    EventSDL(&event);
  }
     SDL_Delay(10);
  } while(1);
   
  //timer.setInterval(5);
  return;
}

void JoyThreadClass::doExit(void)
{
    bRunThread = false;
}


void Ui_MainWindow::LaunchJoyThread(void)
{
    hRunJoy = new JoyThreadClass(this);
    hRunJoy->SetEmu(emu);
    connect(this, SIGNAL(quit_joy_thread()), hRunJoy, SLOT(doExit()));
    hRunJoy->setObjectName("JoyThread");
    hRunJoy->start();
}
void Ui_MainWindow::StopJoyThread(void) {
    emit quit_joy_thread();
}

void Ui_MainWindow::delete_joy_thread(void)
{
  //    delete hRunJoyThread;
  //  delete hRunJoy;
}
