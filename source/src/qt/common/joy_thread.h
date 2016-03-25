/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	History : 2015.11.10 Split from qt_main.h
	[ win32 main ] -> [ Qt main ] -> [Joy Stick]
*/
#ifndef _CSP_QT_JOY_THREAD_H
#define _CSP_QT_JOY_THREAD_H

#include <QThread>
#include <SDL.h>
#include "emu.h"
#include "osd.h"

class EMU;
class QString;

QT_BEGIN_NAMESPACE

class JoyThreadClass : public QThread {
	Q_OBJECT
 private:
	int joy_num[16];
	SDL_Event event;
#if defined(USE_SDL2)   
	SDL_GameController *controller_table[16];
#endif	
	SDL_Joystick *joyhandle[16];
	QString names[16];
	EMU *p_emu;
	OSD *p_osd;
 protected:
	bool bRunThread;
	void joystick_plugged(int num);
	void joystick_unplugged(int num);
	bool EventSDL(SDL_Event *);
	void x_axis_changed(int, int);
	void y_axis_changed(int, int);
	void button_down(int, unsigned int);
	void button_up(int, unsigned int);
	int get_joy_num(int id);
# if defined(USE_SDL2)
	int get_joyid_from_instanceID(SDL_JoystickID id);
# endif
 public:
	JoyThreadClass(EMU *p, QObject *parent = 0);
	~JoyThreadClass();
	void run() { doWork("");}
	void SetEmu(EMU *p) {
		p_emu = p;
	}
public slots:
	void doWork(const QString &);
	void doExit(void);
 signals:
	int sig_finished(void);
	int call_joy_thread(EMU *);
};


QT_END_NAMESPACE

#endif
