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
	int n;
	p_emu = p;
#if defined(USE_JOYSTICK)
# if defined(USE_SDL2)
	for(i = 0; i < 16; i++) {
		controller_table[i] = NULL;
	}
# endif	
	n = SDL_NumJoysticks();
	for(i = 0; i < 16; i++) {
		joyhandle[i] = NULL;
		names[i] = QString::fromUtf8("");
		joy_num[i] = 0;
	}
	if(n > 0) {
		if(n >= 16) n = 16;
# if !defined(USE_SDL2)
		for(i = 0; i < n; i++) {
			joystick_plugged(i);
		}
# endif		
		AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : Start.");
	} else {
		AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : Any joysticks were not connected.");
	}
	bRunThread = true;
#else
	for(i = 0; i < 16; i++) {
		joyhandle[i] = NULL;
		names[i] = QString::fromUtf8("None");
	}
	AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : None launched because this VM has not supported joystick.");
	bRunThread = false;
#endif	 // defined(USE_JOYSTICK)
}

   
JoyThreadClass::~JoyThreadClass()
{
	int i;
#if defined(USE_JOYSTICK)
# if defined(USE_SDL2)
	for(i = 0; i < 16; i++) {
		SDL_GameControllerClose(controller_table[i]);
		controller_table[i] = NULL;
	}
	AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : EXIT");
# else
	for(i = 0; i < 16; i++) {
		SDL_JoystickClose(joyhandle[i]);
		joyhandle[i] = NULL;
	}
	AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : EXIT");
# endif	
#endif	
}
 
#if defined(USE_JOYSTICK)	


void JoyThreadClass::joystick_plugged(int num)
{
	int i,j;
	bool found = false;
# if defined(USE_SDL2)
	if(SDL_IsGameController(num) == SDL_TRUE) {
		if(controller_table[num] != NULL) return;
		controller_table[num] = SDL_GameControllerOpen(num);
		joyhandle[num] = SDL_GameControllerGetJoystick(controller_table[num]);
		if(controller_table[num] != NULL) {
			names[num] = QString::fromUtf8(SDL_GameControllerNameForIndex(num));
			AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : Controller %d : %s : is plugged.", num, names[num].toUtf8().constData());
			strncpy(config.assigned_joystick_name[num], names[num].toUtf8().constData(), 255);
			joy_num[num] = num;
		}
	} else 
# endif
	{
		bool matched = false;
		QString tmps;
# if 0		
		for(i = 0; i < 16; i++) {
			tmps = QString::fromUtf8(SDL_JoystickNameForIndex(num));
			if((strlen(config.assigned_joystick_name[i]) > 0) &&
			   (strncmp(config.assigned_joystick_name[i], tmps.toUtf8().constData(), 255) == 0)) {
				if(joyhandle[i] == NULL) {
					joyhandle[i] = SDL_JoystickOpen(num);
					joy_num[i] = SDL_JoystickInstanceID(joyhandle[i]);
					names[i] = tmps;
					AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : Joystick %d : %s : is plugged.",
								  i, names[i].toUtf8().constData());
					strncpy(config.assigned_joystick_name[num], names[num].toUtf8().constData(), 255);
					matched = true;
					break;
				}
			}
		}
# endif		
		if(!matched) {
			for(i = 0; i < 16; i++) {
				if(joyhandle[i] == NULL) {
					joyhandle[i] = SDL_JoystickOpen(num);
					joy_num[i] = SDL_JoystickInstanceID(joyhandle[i]);
					names[i] = QString::fromUtf8(SDL_JoystickNameForIndex(num));
					AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : Joystick %d : %s : is plugged.", num, names[i].toUtf8().data());
					strncpy(config.assigned_joystick_name[num], names[num].toUtf8().constData(), 255);
					break;
				}
			}
		}
	}
}

void JoyThreadClass::joystick_unplugged(int num)
{
	int i, j;
	if(num < 0) return;
	
# if defined(USE_SDL2)
	if(SDL_IsGameController(num)) {
		if(controller_table[num] != NULL) {
			SDL_GameControllerClose(controller_table[num]);
			controller_table[num] = NULL;
			AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : Controller %d : %s : is removed.", num, names[num].toUtf8().data());
			joy_num[num] = -1;
		}
		joyhandle[num] = NULL;
	} else 
# endif
	{
		if(joyhandle[num] != NULL) {
			SDL_JoystickClose(joyhandle[num]);
			joyhandle[num] = NULL;
			AGAR_DebugLog(AGAR_LOG_DEBUG, "JoyThread : Joystick %d : %s : is removed.", num, names[num].toUtf8().data());
			joy_num[num] = -1;
		}
	}
	names[num] = QString::fromUtf8("");
	memset(config.assigned_joystick_name[num], 0x00, 255);
}	

void JoyThreadClass::x_axis_changed(int index, int value)
{
	if(p_emu == NULL) return;
	if((index < 0) || (index >= 2)) return;
	p_emu->lock_vm();
	uint32_t *joy_status = (uint32_t *)(p_emu->get_osd()->get_joy_buffer());
   
	if(joy_status != NULL) {
		if(value < -8192) { // left
			joy_status[index] |= 0x04; joy_status[index] &= ~0x08;
		} else if(value > 8192)  { // right
			joy_status[index] |= 0x08; joy_status[index] &= ~0x04;
		}  else { // center
			joy_status[index] &= ~0x0c;
		}
	}
	p_emu->unlock_vm();
}
	   
void JoyThreadClass::y_axis_changed(int index, int value)
{
	if(p_emu == NULL) return;
	if((index < 0) || (index >= 2)) return;
	p_emu->lock_vm();
	uint32_t *joy_status = p_emu->get_osd()->get_joy_buffer();
   
	if(joy_status != NULL) {
		if(value < -8192) {// up
			joy_status[index] |= 0x01; joy_status[index] &= ~0x02;
		} else if(value > 8192)  {// down 
			joy_status[index] |= 0x02; joy_status[index] &= ~0x01;
		} else {
			joy_status[index] &= ~0x03;
		}
	}
	p_emu->unlock_vm();
}

void JoyThreadClass::button_down(int index, unsigned int button)
{
	if(p_emu == NULL) return;
	if((index < 0) || (index >= 4)) return;
	if(button >= 12) return;
	p_emu->lock_vm();
	uint32_t *joy_status = p_emu->get_osd()->get_joy_buffer();
	if(joy_status != NULL) {
		joy_status[index] |= (1 << (button + 4));
	}
	p_emu->unlock_vm();
}

void JoyThreadClass::button_up(int index, unsigned int button)
{
	if(p_emu == NULL) return;
	if((index < 0) || (index >= 4)) return;
	if(button >= 12) return;
	p_emu->lock_vm();
	uint32_t *joy_status = p_emu->get_osd()->get_joy_buffer();
	if(joy_status != NULL) {
		joy_status[index] &= ~(1 << (button + 4));
	}
	p_emu->unlock_vm();
}

#if defined(USE_SDL2)			   
int JoyThreadClass::get_joyid_from_instanceID(SDL_JoystickID id)
{
	int i;
	SDL_Joystick *js;
	for(i = 0; i < 16; i++) {
		if(controller_table[i] == NULL) continue;
		js = SDL_GameControllerGetJoystick(controller_table[i]);
		if(js == NULL) continue;
		if(id == SDL_JoystickInstanceID(js)) return i;
	}
	return -1;
}
#endif
int JoyThreadClass::get_joy_num(int id)
{
	int i;
	for(i = 0; i < 16; i++) {
		if(joy_num[i] == id) return i;
	}
	return -1;
}

bool  JoyThreadClass::EventSDL(SDL_Event *eventQueue)
{
	//	SDL_Surface *p;
	Sint16 value;
	unsigned int button;
	int vk;
	uint32_t sym;
	uint32_t mod;
# if defined(USE_SDL2)
	SDL_JoystickID id;
	SDL_GameControllerButton cont_button;
# endif   
	int i, j;
	if(eventQueue == NULL) return false;
	/*
	 * JoyStickなどはSDLが管理する
	 */
	switch (eventQueue->type){
# if defined(USE_SDL2)
		case SDL_CONTROLLERAXISMOTION:
			value = eventQueue->caxis.value;
			if(eventQueue->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
				id = (int)eventQueue->caxis.which;
				i = get_joyid_from_instanceID(id);
				x_axis_changed(i, value);
			} else if(eventQueue->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
				id = (int)eventQueue->caxis.which;
				i = get_joyid_from_instanceID(id);
				y_axis_changed(i, value);
			}
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			button = eventQueue->cbutton.button;
			id = eventQueue->cbutton.which;
			i = get_joyid_from_instanceID(id);
			//button = SDL_GameControllerGetButton(controller_table[i], cont_button);
			button_down(i, button);
			break;
		case SDL_CONTROLLERBUTTONUP:
			button = eventQueue->cbutton.button;
			id = eventQueue->cbutton.which;
			i = get_joyid_from_instanceID(id);
			//button = SDL_GameControllerGetButton(controller_table[i], cont_button);
			button_up(i, button);
			break;
		case SDL_JOYDEVICEADDED:
			i = eventQueue->jdevice.which;
			joystick_plugged(i);
			break;
		case SDL_JOYDEVICEREMOVED:
			i = eventQueue->jdevice.which;
			i = get_joy_num(i);
			joystick_unplugged(i);
			break;
# endif		
		case SDL_JOYAXISMOTION:
			value = eventQueue->jaxis.value;
			i = eventQueue->jaxis.which;
			i = get_joy_num(i);
			if(eventQueue->jaxis.axis == 0) { // X
				x_axis_changed(i, value);
			} else if(eventQueue->jaxis.axis == 1) { // Y
				y_axis_changed(i, value);
			}
			break;
		case SDL_JOYBUTTONDOWN:
			button = eventQueue->jbutton.button;
			i = eventQueue->jbutton.which;
			i = get_joy_num(i);
			button_down(i, button);
			break;
		case SDL_JOYBUTTONUP:	   
			button = eventQueue->jbutton.button;
			i = eventQueue->jbutton.which;
			i = get_joy_num(i);
			button_up(i, button);
			break;
		default:
			break;
	}
	return true;
}
#endif // defined(USE_JOYSTICK)	

void JoyThreadClass::doWork(const QString &params)
{
#if defined(USE_JOYSTICK)	
	do {
		if(bRunThread == false) {
			break;
		}
		while(SDL_PollEvent(&event)) {
			EventSDL(&event);
		}
		msleep(10);
	} while(1);
#endif	
	this->quit();
}
	   

void JoyThreadClass::doExit(void)
{
	bRunThread = false;
	//this->quit();
}

