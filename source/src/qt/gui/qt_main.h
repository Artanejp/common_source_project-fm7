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
extern class META_MainWindow *rMainWindow;
// menu
extern std::string DLL_PREFIX cpp_homedir;
extern std::string DLL_PREFIX cpp_confdir;
extern std::string DLL_PREFIX my_procname;
extern std::string DLL_PREFIX sRssDir;

extern const int DLL_PREFIX screen_mode_width[];
extern const int DLL_PREFIX screen_mode_height[];

#ifndef MAX_HISTORY
#define MAX_HISTORY 8
#endif

static inline void SETUP_HISTORY(void* recent, QStringList& list, const ssize_t width = MAX_HISTORY)
{
	if(recent == nullptr) return;

	list.clear();
	const size_t length = _MAX_PATH;
	_TCHAR* p = (_TCHAR *)recent;
	for(int i = 0; i < width; i++) {
		QString _tmps = QString::fromLocal8Bit(p);
		list << _tmps;
		p = &(p[length]);
	}
}

static inline void WRITEBACK_TO_HISTORY(void* recent, QStringList list, const ssize_t width = MAX_HISTORY)
{
	const size_t length = _MAX_PATH;
	if(recent == nullptr) return;

	_TCHAR* p = (_TCHAR *)recent;
	// Clear LIST
	for(int i = 0; i < width; i++) {
		memset(p, 0x00, length);
		p = &(p[length]);
	}
	// Update List
	p = (_TCHAR *)recent;
	for(int i = 0; i < width; i++) {
		if(i >= list.size()) break;
		my_tcscpy_s(p, length - 1, list.at(i).toLocal8Bit().constData());
		p = &(p[length]);
	}
}

static inline QStringList SHRINK_HISTORY(QStringList list, const ssize_t width = MAX_HISTORY)
{
	QStringList tmpl2;
	for(auto _l = list.begin(); _l != list.end(); ++_l) {
		if(tmpl2.size() >= width) break;
		tmpl2.push_back((*_l));
	}
	return tmpl2;
}
static inline void UPDATE_HISTORY(QString path, void *recent, QStringList& list, const ssize_t width = MAX_HISTORY)
{
	if(recent == nullptr) return;
	// Set temporally list
	QStringList tmpl;
	SETUP_HISTORY(recent, tmpl, width);

	if(!(path.isEmpty())) {
		tmpl.push_front(path);
	}
	QString tmps = tmpl.at(0);
	// Remove Duplicates
	ssize_t ix = 0;
	do {
		ix = tmpl.indexOf(tmps, 1);
		if(ix > 0) {
			tmpl.removeAt(ix);
		}
	} while((ix > 0) && (ix < width));
	// copy list, shrink to MAX_HISTORY.
	QStringList tmpl2 = SHRINK_HISTORY(tmpl, width);
	WRITEBACK_TO_HISTORY(recent, tmpl2, width);
	list = tmpl2;
}

static inline void UPDATE_HISTORY(_TCHAR *path, void *recent, QStringList& list, const ssize_t width = MAX_HISTORY)
{

	if(recent == nullptr) return;
	if(path == nullptr) return;
	QString _path = QString::fromLocal8Bit(path);
	//QString _path = QString::fromUtf8(path);
	UPDATE_HISTORY(_path, recent, list, width);
}

//extern DLL_PREFIX_I _TCHAR* get_parent_dir(const _TCHAR *file);
extern DLL_PREFIX_I void get_long_full_path_name(_TCHAR* src, _TCHAR* dst);
//extern _TCHAR* DLL_PREFIX get_parent_dir(_TCHAR* file);
extern void DLL_PREFIX Convert_CP932_to_UTF8(char *dst, char *src, int n_limit, int i_limit);
//extern void DLL_PREFIX get_long_full_path_name(_TCHAR* src, _TCHAR* dst);
extern void DLL_PREFIX get_short_filename(_TCHAR *dst, _TCHAR *file, int maxlen);

// Important Flags
//extern AGAR_CPUID *pCpuID;

#endif
