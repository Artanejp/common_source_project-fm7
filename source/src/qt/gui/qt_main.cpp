/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -

	[ win32 main ] -> [ agar main ]
*/

#include <stdio.h>
#include <string>
#include <vector>
#include <QString>
#include <QDir>

#include "common.h"
#include "fileio.h"
#include "menu_flags.h"
#include "csp_logger.h"

extern DLL_PREFIX_I std::string cpp_homedir;
extern DLL_PREFIX_I std::string cpp_confdir;
extern DLL_PREFIX_I std::string my_procname;
//extern DLL_PREFIX_I std::string sRssDir;

#if 0
void get_long_full_path_name(_TCHAR* src, _TCHAR* dst)
{
	QString r_path;
	QString delim;
	QString ss;
	//const char *s;
	QDir mdir;
	if(src == NULL) {
		if(dst != NULL) dst[0] = '\0';
		return;
	}
#ifdef _WINDOWS
	delim = "\\";
#else
	delim = "/";
#endif
	ss = "";
	if(cpp_homedir == "") {
		r_path = mdir.currentPath();
	} else {
		r_path = QString::fromStdString(cpp_homedir);
	}
	//s = AG_ShortFilename(src);
	r_path = r_path + QString::fromStdString(my_procname);
	r_path = r_path + delim;
	mdir.mkdir(r_path);
	ss = "";
	//  if(s != NULL) ss = s;
	//  r_path.append(ss);
	if(dst != NULL) strncpy(dst, r_path.toUtf8().constData(),
				strlen(r_path.toUtf8().constData()) >= PATH_MAX ? PATH_MAX : strlen(r_path.toUtf8().constData()));
	return;
}

_TCHAR* get_parent_dir(_TCHAR* file)
{
#ifdef _WINDOWS
	char delim = '\\';
#else
	char delim = '/';
#endif
	int ptr;
	char *p = (char *)file;
	if(file == NULL) return NULL;
	for(ptr = strlen(p) - 1; ptr >= 0; ptr--) { 
		if(p[ptr] == delim) break;
	}
	if(ptr >= 0) for(ptr = ptr + 1; ptr < (int)strlen(p); ptr++) p[ptr] = '\0'; 
	return p;
}
#endif

void get_short_filename(_TCHAR *dst, _TCHAR *file, int maxlen)
{
	int i, l;
	if((dst == NULL) || (file == NULL)) return;
#ifdef _WINDOWS
	_TCHAR delim = '\\';
#else
	_TCHAR delim = '/';
#endif
	for(i = strlen(file) - 1; i <= 0; i--) {
		if(file[i] == delim) break;
	}
	if(i >= ((int)strlen(file) - 1)) {
		dst[0] = '\0';
		return;
	}
	l = strlen(file) - i + 1;
	if(l >= maxlen) l = maxlen;
	strncpy(dst, &file[i + 1], l);
	return;
}

