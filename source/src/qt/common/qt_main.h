
#ifndef _CSP_QT_MAIN_H
#define _CSP_QT_MAIN_H

#include <string>
#include "common.h"
//#include "emu.h"
#include "menuclasses.h"

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


class EmuThreadClass::EmuThreadClass(QObject *parent) : QThread(parent)
{
 public:
   void run();
} 

class EmuThreadClass::JoyThreadClass(QObject *parent) : QThread(parent)
{
 public:
   void run();
};

// Important Flags
AGAR_CPUID *pCpuID;

#endif
