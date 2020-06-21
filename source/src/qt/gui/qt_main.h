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

#include <SDL.h>
//#include "sdl_cpuid.h"
#include "simd_types.h"
#include "common.h"
//#include "../../emu.h"

class META_MainWindow;
class EMU;
extern class META_MainWindow *rMainWindow;
//extern EMU* emu;
// menu
extern std::string DLL_PREFIX cpp_homedir;
extern std::string DLL_PREFIX cpp_confdir;
extern std::string DLL_PREFIX my_procname;
extern std::string DLL_PREFIX sRssDir;
extern bool DLL_PREFIX now_menuloop;

extern const int DLL_PREFIX screen_mode_width[];
extern const int DLL_PREFIX screen_mode_height[];

#ifndef MAX_HISTORY
#define MAX_HISTORY 8
#endif

#ifndef UPDATE_HISTORY
#define UPDATE_HISTORY(path, recent, list) { \
	int no = MAX_HISTORY - 1; \
	bool found = false; \
	for(int i = 0; i < MAX_HISTORY; i++) { \
		if(strcmp(recent[i], path) == 0) { \
			no = i; \
			found = true; \
			break; \
		} \
	} \
	if(found) { \
		strcpy(recent[MAX_HISTORY - 1], ""); \
	} \
	for(int i = no; i > 0; i--) { \
		strcpy(recent[i], recent[i - 1]); \
	} \
	strcpy(recent[0], path); \
	list.clear(); \
	for(int i = 0; i < MAX_HISTORY; i++) { \
		QString _tmps = QString::fromLocal8Bit(recent[i]); \
		list << _tmps; \
	} \
}
#endif

#ifndef SETUP_HISTORY
#define SETUP_HISTORY(recent, list) { \
	list.clear(); \
	for(int i = 0; i < MAX_HISTORY; i++) { \
		QString _tmps = QString::fromLocal8Bit(recent[i]); \
		list << _tmps; \
	} \
}
#endif

//extern DLL_PREFIX_I _TCHAR* get_parent_dir(const _TCHAR *file);
extern DLL_PREFIX_I void get_long_full_path_name(_TCHAR* src, _TCHAR* dst);
//extern _TCHAR* DLL_PREFIX get_parent_dir(_TCHAR* file);
extern void DLL_PREFIX Convert_CP932_to_UTF8(char *dst, char *src, int n_limit, int i_limit);
//extern void DLL_PREFIX get_long_full_path_name(_TCHAR* src, _TCHAR* dst);
extern void DLL_PREFIX get_short_filename(_TCHAR *dst, _TCHAR *file, int maxlen);

// Important Flags
//extern AGAR_CPUID *pCpuID;

#endif
