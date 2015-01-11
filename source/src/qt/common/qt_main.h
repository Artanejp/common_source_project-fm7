
#ifndef _CSP_QT_MAIN_H
#define _CSP_QT_MAIN_H

#include <string>
#include <qthread.h>
#include "sdl_cpuid.h"
#include "simd_types.h"
#include "common.h"
//#include "emu.h"
#include "menuclasses.h"
#include "qt_dialogs.h"

extern class Ui_MainWindow *rMainWindow;

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
extern bool bRunEmuThread;
extern bool bRunJoyThread;

#ifndef MAX_HISTORY
#define MAX_HISTORY 8
#endif


class EmuThreadClass : public QThread {
 public:
   void run();
signals:
   void valueChanged(QString);
}; 

class JoyThreadClass : public QThread {
 public:
   void run();
};

extern _TCHAR* get_parent_dir(_TCHAR* file);
extern void Convert_CP932_to_UTF8(char *dst, char *src, int n_limit);
extern void  get_long_full_path_name(_TCHAR* src, _TCHAR* dst);
extern void get_short_filename(_TCHAR *dst, _TCHAR *file, int maxlen);
extern void set_window(QMainWindow * hWnd, int mode);

// Important Flags
AGAR_CPUID *pCpuID;

#endif
