/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	History : 2015.11.10 Split from emu_input.cpp
	[ win32 main ] -> [ Qt main ] -> [Joy Stick]
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
#include "agar_logger.h"

#include "joy_thread.h"

JoyThreadClass::JoyThreadClass(EMU *p, QObject *parent) : QThread(parent)
{
	int i, j;
	
	p_emu = p;
	joy_num = SDL_NumJoysticks();
	for(i = 0; i < 16; i++) {
		joyhandle[i] = NULL;
#if defined(USE_SDL2)  
		for(j = 0; j < 16; j++) guid_list[i].data[j] = 0;
		for(j = 0; j < 16; j++) guid_assign[i].data[j] = 0;
#endif	   
		names[i] = QString::fromUtf8("");
	}
	if(joy_num > 0) {
		if(joy_num >= 16) joy_num = 16;
		for(i = 0; i < joy_num; i++) {
		   
			joyhandle[i] = SDL_JoystickOpen(i);
			if(joyhandle[i] != NULL) {
#if defined(USE_SDL2)			   
				guid_list[i] = SDL_JoystickGetGUID(joyhandle[i]);
				guid_assign[i] = SDL_JoystickGetGUID(joyhandle[i]);
				names[i] = QString::fromUtf8(SDL_JoystickNameForIndex(i));
#else
				names[i] = QString::fromUtf8(SDL_JoystickName(i));
#endif			   
				AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : Joystick %d : %s.", i, names[i].toUtf8().data());
			}
		}
		AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : Start.");
		bRunThread = true;
	} else {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : Any joysticks were not connected.");
		bRunThread = false;
	}
}

   
JoyThreadClass::~JoyThreadClass()
{
	int i;
	for(i = 0; i < 16; i++) {
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

#if defined(USE_SDL2)
// SDL Event Handler
bool JoyThreadClass::MatchJoyGUID(SDL_JoystickGUID *a, SDL_JoystickGUID *b)
{ 
	int i;
	for(i = 0; i < 16; i++) {
		if(a->data[i] != b->data[i]) return false;
	}
	return true;
}

bool JoyThreadClass::CheckJoyGUID(SDL_JoystickGUID *a)
{ 
	int i;
	bool b = false;
	for(i = 0; i < 16; i++) {
		if(a->data[i] != 0) b = true;
	}
	return b;
}
#endif

bool  JoyThreadClass::EventSDL(SDL_Event *eventQueue)
{
	//	SDL_Surface *p;
	Sint16 value;
	unsigned int button;
	int vk;
	uint32_t sym;
	uint32_t mod;
#if defined(USE_SDL2)
	SDL_JoystickGUID guid;
#endif   
	int i;
	if(eventQueue == NULL) return false;
	/*
	 * JoyStickなどはSDLが管理する
	 */
	switch (eventQueue->type){
		case SDL_JOYAXISMOTION:
			value = eventQueue->jaxis.value;
			i = eventQueue->jaxis.which;
	   
#if defined(USE_SDL2)
			guid = SDL_JoystickGetDeviceGUID(i);
			if(!CheckJoyGUID(&guid)) break;
			for(i = 0; i < 2; i++) {
				if(MatchJoyGUID(&guid, &(guid_assign[i]))) {
					if(eventQueue->jaxis.axis == 0) { // X
						x_axis_changed(i, value);
					} else if(eventQueue->jaxis.axis == 1) { // Y
						y_axis_changed(i, value);
					}
				}
			}
#else
			if(eventQueue->jaxis.axis == 0) { // X
				x_axis_changed(i, value);
			} else if(eventQueue->jaxis.axis == 1) { // Y
				y_axis_changed(i, value);
			}
#endif
			break;
		case SDL_JOYBUTTONDOWN:
			button = eventQueue->jbutton.button;
			i = eventQueue->jbutton.which;
#if defined(USE_SDL2)
			guid = SDL_JoystickGetDeviceGUID(i);
			if(!CheckJoyGUID(&guid)) break;
			for(i = 0; i < 2; i++) {
				if(MatchJoyGUID(&guid, &(guid_assign[i]))) {
					button_down(i, button);
				}
			}
#else	   
			button_down(i, button);
#endif	   
			break;
		case SDL_JOYBUTTONUP:	   
			button = eventQueue->jbutton.button;
			i = eventQueue->jbutton.which;
#if defined(USE_SDL2)
			guid = SDL_JoystickGetDeviceGUID(i);
			if(!CheckJoyGUID(&guid)) break;
			for(i = 0; i < 2; i++) {
				if(MatchJoyGUID(&guid, &(guid_assign[i]))) {
					button_up(i, button);
				}
			}
#else	   
			button_up(i, button);
#endif	   
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
			break;
		}
		while(SDL_PollEvent(&event)) {
			EventSDL(&event);
		}
		msleep(10);
	} while(1);
	this->quit();
}
	   

void JoyThreadClass::doExit(void)
{
	bRunThread = false;
	//this->quit();
}

