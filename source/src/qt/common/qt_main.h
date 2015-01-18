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
#include <SDL2/SDL.h>
#include "sdl_cpuid.h"
#include "simd_types.h"
#include "common.h"
//#include "emu.h"
//#include "menuclasses.h"
# include "mainwidget.h"
#include "qt_dialogs.h"

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
extern bool bRunJoyThread;

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

class EmuThreadCore : public QThread {
  Q_OBJECT
 public:
  EmuThreadCore(QObject *parent = 0) : QThread(parent) {};
  ~EmuThreadCore() {};
 public slots:
     
 signals:
  int quit_emu_thread();
};

class JoyThreadCore : public QThread {
  Q_OBJECT
 public:
  JoyThreadCore(QObject *parent = 0) : QThread(parent) {};
  ~JoyThreadCore() {};
 public slots:
     
 signals:
};

class EmuThreadClass : public QObject {
  Q_OBJECT
 protected:
  EMU *p_emu;
  bool bRunThread;
 public:
  EmuThreadClass(QObject *parent = 0) : QObject(parent) {};
  ~EmuThreadClass() {};
 public slots:
  void doWork(EMU *);
  void doExit(void);
 signals:
  int message_changed(QString);
  int sig_screen_aspect(int);
  int sig_screen_size(int, int);
};

class JoyThreadClass : public QObject {
  Q_OBJECT
 protected:
   EMU *p_emu;
   bool EventSDL(SDL_Event *);
   void x_axis_changed(int, int);
   void y_axis_changed(int, int);
   void button_down(int, unsigned int);
   void button_up(int, unsigned int);
 public:
  JoyThreadClass(QObject *parent = 0) : QObject(parent) {};
  ~JoyThreadClass() {};
 public slots:
  void doWork(EMU *);

 signals:
//  int x_axis_changed(int, int);
//  int y_axis_changed(int, int);
//  int button_down(int, unsigned int);
//  int button_up(int, unsigned int);
};


QT_END_NAMESPACE

extern _TCHAR* get_parent_dir(_TCHAR* file);
extern void Convert_CP932_to_UTF8(char *dst, char *src, int n_limit);
extern void  get_long_full_path_name(_TCHAR* src, _TCHAR* dst);
extern void get_short_filename(_TCHAR *dst, _TCHAR *file, int maxlen);

// Important Flags
extern AGAR_CPUID *pCpuID;

#endif
