/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	[ win32 main ] -> [ Qt main ]
*/

#ifndef _CSP_QT_MAIN_H
#define _CSP_QT_MAIN_H

#include <string>
#include <qthread.h>
#include <QTimer>
#include <QIcon>
#include <QImage>
#include <QString>

#include <SDL2/SDL.h>
#include "sdl_cpuid.h"
#include "simd_types.h"
#include "common.h"
#include "../../emu.h"
//#include "menuclasses.h"
//# include "mainwidget.h"
//#include "qt_dialogs.h"
class META_MainWindow;


extern class META_MainWindow *rMainWindow;
extern EMU* emu;
// menu
extern std::string cpp_homedir;
extern std::string cpp_confdir;
extern std::string my_procname;
extern std::string cpp_simdtype;
//std::string sAG_Driver;
extern std::string sRssDir;
extern bool now_menuloop;

extern const int screen_mode_width[];
extern const int screen_mode_height[];

#ifndef MAX_HISTORY
#define MAX_HISTORY 8
#endif

#ifndef UPDATE_HISTORY
#define UPDATE_HISTORY(path, recent) { \
	int no = MAX_HISTORY - 1; \
	for(int i = 0; i < MAX_HISTORY; i++) { \
		if(strcmp(recent[i], path) == 0) { \
			no = i; \
			break; \
		} \
	} \
	for(int i = no; i > 0; i--) { \
		strcpy(recent[i], recent[i - 1]); \
	} \
	strcpy(recent[0], path); \
}
#endif

QT_BEGIN_NAMESPACE

class EmuThreadClass : public QThread {
  Q_OBJECT
private:
	bool calc_message;
	bool tape_play_flag;
	bool mouse_flag;
 protected:
	EMU *p_emu;
	bool bRunThread;
	bool bResetReq;
	bool bSpecialResetReq;
	bool bLoadStateReq;
	bool bSaveStateReq;
	uint32 next_time;
	uint32 update_fps_time;
	bool prev_skip;
	int total_frames;
	int draw_frames;
	int skip_frames;
 public:

	EmuThreadClass(QObject *parent = 0) : QThread(parent) {
		bRunThread = true;
		prev_skip = false;
		update_fps_time = SDL_GetTicks();
		next_time = update_fps_time;
		total_frames = 0;
		draw_frames = 0;
		skip_frames = 0;
		calc_message = true;
		mouse_flag = false;
	};
	~EmuThreadClass() {};
	void SetEmu(EMU *p) {
		p_emu = p;
	}
	void set_tape_play(bool);
	void run() { doWork("");}
	
public slots:
	void doWork(const QString &param);
	void doExit(void);
	void print_framerate(int frames);
	void doReset();
	void doSpecialReset();
	void doLoadState();
	void doSaveState();
	void moved_mouse(int, int);
	void button_pressed_mouse(Qt::MouseButton);
	void button_released_mouse(Qt::MouseButton);
#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
	void do_close_disk(int);
	void do_open_disk(int, QString, int);
#endif
signals:
	int message_changed(QString);
	int sig_draw_thread(void);
	int quit_draw_thread(void);
	int sig_screen_aspect(int);
	int sig_screen_size(int, int);
	int sig_finished(void);
	int call_emu_thread(EMU *);
	int sig_check_grab_mouse(bool);
	int sig_mouse_enable(bool);
#ifdef USE_TAPE_BUTTON
	int sig_tape_play_stat(bool);
#endif
#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
	int sig_update_recent_disk(int);
#endif
};

class JoyThreadClass : public QThread {
  Q_OBJECT
 private:
	int joy_num;
	SDL_Event event;
	SDL_Joystick *joyhandle[16];
	SDL_JoystickGUID guid_list[16];
	SDL_JoystickGUID guid_assign[16];
	QString names[16];
	EMU *p_emu;
 protected:
	bool bRunThread;
	bool EventSDL(SDL_Event *);
	void x_axis_changed(int, int);
	void y_axis_changed(int, int);
	void button_down(int, unsigned int);
	void button_up(int, unsigned int);
	bool CheckJoyGUID(SDL_JoystickGUID *a);
	bool MatchJoyGUID(SDL_JoystickGUID *a, SDL_JoystickGUID *b);
 public:
	JoyThreadClass(QObject *parent = 0);
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


class DrawThreadClass : public QThread {
  Q_OBJECT
 private:
	EMU *p_emu;
	Ui_MainWindow *MainWindow;
 protected:
	int draw_frames;
	bool bRunThread;
 public:
	DrawThreadClass(QObject *parent = 0);
	~DrawThreadClass() {};
	void run() { doWork("");}
	void SetEmu(EMU *p) {
		p_emu = p;
	}
  
public slots:
	void doWork(const QString &);
	void doExit(void);
	void doDraw(void);
signals:
	int sig_draw_frames(int);
	int message_changed(QString);
	int sig_update_screen(QImage *);
};


QT_END_NAMESPACE

extern _TCHAR* get_parent_dir(_TCHAR* file);
extern void Convert_CP932_to_UTF8(char *dst, char *src, int n_limit, int i_limit);
extern void  get_long_full_path_name(_TCHAR* src, _TCHAR* dst);
extern void get_short_filename(_TCHAR *dst, _TCHAR *file, int maxlen);

// Important Flags
extern AGAR_CPUID *pCpuID;

#endif
