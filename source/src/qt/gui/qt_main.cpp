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
	if(l >= maxlen) l = maxlen - 1;
	strncpy(dst, &file[i + 1], l);
	return;
}

