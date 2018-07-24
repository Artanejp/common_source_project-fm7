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
#include <QApplication>
#include <QString>
#include <QTextCodec>
#include <QImage>
#include <QImageReader>
#include <QDateTime>
#include <QDir>
#include <QTranslator>
#include <QProcessEnvironment>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "common.h"
#include "fileio.h"
#include "menu_flags.h"
#include "csp_logger.h"
#include "config.h"

extern config_t config;
extern std::string cpp_homedir;
extern std::string cpp_confdir;
extern std::string my_procname;
extern std::string sRssDir;

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

QCommandLineOption *_opt_homedir;
QCommandLineOption *_opt_cfgfile;
QCommandLineOption *_opt_cfgdir;
QCommandLineOption *_opt_resdir;
QCommandLineOption *_opt_opengl;
QCommandLineOption *_opt_envver;
QCommandLineOption *_opt_dump_envver;
QCommandLineOption *_opt_dipsw_on;
QCommandLineOption *_opt_dipsw_off;
QProcessEnvironment _envvers;
extern QApplication *GuiMain;
extern bool _b_dump_envver;
extern std::string config_fullpath;

DLL_PREFIX void SetOptions_Sub(QCommandLineParser *cmdparser)
{
	QStringList _cl;
    _cl.append("d");
    _cl.append("homedir");
    _opt_homedir = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Custom home directory."), "homedir");
    _cl.clear();
   
    _cl.append("c");
    _cl.append("cfgfile");
    _opt_cfgfile = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Custom config file (without path)."), "cfgfile");
    _cl.clear();
   
    _opt_cfgdir = new QCommandLineOption("cfgdir", QCoreApplication::translate("main", "Custom config directory."), "cfgdir");

    _cl.append("r");
    _cl.append("res");
    _opt_resdir = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Custom resource directory (ROMs, WAVs, etc)."), "resdir");
    _cl.clear();

    _cl.append("on");
    _cl.append("dipsw-on");
    _opt_dipsw_on = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Turn on <onbit> of dip switch."), "onbit");
    _cl.clear();
   
    _cl.append("off");
    _cl.append("dipsw-off");
    _opt_dipsw_off = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Turn off <offbit> of dip switch."), "offbit");
    _cl.clear();
	
    _cl.append("g");
    _cl.append("gl");
    _cl.append("opengl");
    _cl.append("render");
    _opt_opengl = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Force set using renderer type."), "{ GL | GL2 | GL3 | GL4 | GLES}");
    _cl.clear();
	
    _cl.append("v");
    _cl.append("env");
    _cl.append("envver");
    _opt_envver = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Set / Delete environment variable."), "{NAME[=VAL] | -NAME}");
    _cl.clear();

    _cl.append("dump-env");
    _cl.append("dump-envver");
    _opt_dump_envver = new QCommandLineOption(_cl, QCoreApplication::translate("main", "Dump environment variables."), "");
    _cl.clear();

    cmdparser->addOption(*_opt_opengl);
    cmdparser->addOption(*_opt_homedir);
    cmdparser->addOption(*_opt_cfgfile);
    cmdparser->addOption(*_opt_cfgdir);
    cmdparser->addOption(*_opt_resdir);
    cmdparser->addOption(*_opt_dipsw_on);
    cmdparser->addOption(*_opt_dipsw_off);
}

DLL_PREFIX void ProcessCmdLine_Sub(QCommandLineParser *cmdparser, QStringList *_l)
{
	char homedir[PATH_MAX];
	std::string delim;
	memset(homedir, 0x00, PATH_MAX);
	if(cmdparser->isSet(*_opt_homedir)) {
		strncpy(homedir, cmdparser->value(*_opt_homedir).toLocal8Bit().constData(), PATH_MAX - 1);
		cpp_homedir = homedir;
		size_t _len = cpp_homedir.length() - 1;
		size_t _pos = cpp_homedir.rfind(delim);
		if((_pos < _len) ||
		   (_pos == std::string::npos)) {
			cpp_homedir.append(delim);
		}
	} else {
		cpp_homedir.copy(homedir, PATH_MAX - 1, 0);
	}

	if(cmdparser->isSet(*_opt_cfgdir)) {
		char tmps[PATH_MAX];
		std::string tmpstr;
		memset(tmps, 0x00, PATH_MAX);
		strncpy(tmps, cmdparser->value(*_opt_cfgdir).toLocal8Bit().constData(), PATH_MAX - 1);
		cpp_confdir = tmps;
		size_t _len = cpp_confdir.length() - 1;
		size_t _pos = cpp_confdir.rfind(delim);
		if((_pos < _len) ||
		   (_pos == std::string::npos)) {
			cpp_confdir.append(delim);
		}
	}
	if(cmdparser->isSet(*_opt_resdir)) {
		char tmps[PATH_MAX];
		std::string tmpstr;
		memset(tmps, 0x00, PATH_MAX);
		strncpy(tmps, cmdparser->value(*_opt_resdir).toLocal8Bit().constData(), PATH_MAX - 1);
		sRssDir = tmps;
		size_t _len = sRssDir.length() - 1;
		size_t _pos = sRssDir.rfind(delim);
		if((_pos < _len) ||
		   (_pos == std::string::npos)) {
			sRssDir.append(delim);
		}
	}
	
	if(cmdparser->isSet(*_opt_envver)) {
		QStringList nList = cmdparser->values(*_opt_envver);
		QString tv;
		//QProcessEnvironment ev = QProcessEnvironment::systemEnvironment();
		QProcessEnvironment ev = _envvers;
		if(nList.size() > 0) {
			for(int i = 0; i < nList.size(); i++) {
				tv = nList.at(i);
				if(tv.indexOf(QString::fromUtf8("-")) == 0) {
					// Delete var
					int n1 = tv.indexOf(QString::fromUtf8("="));
					if(n1 > 0) {
						tv = tv.left(n1).right(n1 - 1);
					} else {
						tv = tv.right(tv.length() - 1);
					}
					printf("DEBUG: DEL ENV:%s\n", tv.toLocal8Bit().constData());
					ev.remove(tv);
				} else if(tv.indexOf(QString::fromUtf8("=")) > 0) {
					// Delete var
					int n1 = tv.indexOf(QString::fromUtf8("="));
					QString skey;
					QString sval;
					skey = tv.left(n1);
					if((tv.length() - n1) < 1) {
						sval = QString::fromUtf8("");
					} else {
						sval = tv.right(tv.length() - n1 - 1);
					}
					printf("DEBUG: SET ENV:%s to %s\n", skey.toLocal8Bit().constData(), sval.toLocal8Bit().constData());
					if(skey.length() > 0) ev.insert(skey, sval);
				} else if(tv.indexOf(QString::fromUtf8("=")) < 0) {
					printf("DEBUG: SET ENV:%s to (NULL)\n", tv.toLocal8Bit().constData());
					if(tv.length() > 0) ev.insert(tv, QString::fromUtf8(""));
				}
			}
			_envvers.swap(ev);
		}
	}
	_b_dump_envver = false;
	if(cmdparser->isSet(*_opt_dump_envver)) {
		_b_dump_envver = true;
	}
}
