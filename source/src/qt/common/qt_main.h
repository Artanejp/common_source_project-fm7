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
#include <SDL/SDL.h>
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




class EmuThreadClass : public QThread {
 private:
// public slots:
 public:
   void run();
}; 

class JoyThreadClass : public QObject {
 public slots:
   void run();
};

extern _TCHAR* get_parent_dir(_TCHAR* file);
extern void Convert_CP932_to_UTF8(char *dst, char *src, int n_limit);
extern void  get_long_full_path_name(_TCHAR* src, _TCHAR* dst);
extern void get_short_filename(_TCHAR *dst, _TCHAR *file, int maxlen);
extern void set_window(QMainWindow * hWnd, int mode);

// Important Flags
extern AGAR_CPUID *pCpuID;

#endif
