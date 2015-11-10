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

class EMU;
class QString;

QT_BEGIN_NAMESPACE

class JoyThreadClass : public QThread {
  Q_OBJECT
 private:
	int joy_num;
	SDL_Event event;
	SDL_Joystick *joyhandle[16];
#if defined(USE_SDL2)   
	SDL_JoystickGUID guid_list[16];
	SDL_JoystickGUID guid_assign[16];
#endif   
	QString names[16];
	EMU *p_emu;
 protected:
	bool bRunThread;
	bool EventSDL(SDL_Event *);
	void x_axis_changed(int, int);
	void y_axis_changed(int, int);
	void button_down(int, unsigned int);
	void button_up(int, unsigned int);
#if defined(USE_SDL2)
	bool CheckJoyGUID(SDL_JoystickGUID *a);
	bool MatchJoyGUID(SDL_JoystickGUID *a, SDL_JoystickGUID *b);
#endif   
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
