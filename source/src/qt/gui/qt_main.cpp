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

QProcessEnvironment _envvars;

bool _b_dump_envvar;
std::string config_fullpath;

DLL_PREFIX QList<QCommandLineOption> SetOptions_Sub(QCommandLineParser *parser)
{
	QList<QCommandLineOption> _ret;
	QStringList _cl;
    _cl.append("d");
    _cl.append("homedir");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Custom home directory."), QCoreApplication::translate("main", "DIRECTORY PATH")));
    _cl.clear();

    _cl.append("c");
    _cl.append("cfgfile");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Custom config file (without path)."), QCoreApplication::translate("main", "PATH")));
    _cl.clear();


    _cl.append("r");
    _cl.append("res");
    _cl.append("resdir");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Custom resource directory (ROMs, WAVs, etc)."), QCoreApplication::translate("main", "DIRECTORY PATH")));
    _cl.clear();

    _cl.append("on");
    _cl.append("dipsw-on");
    _cl.append("onbit");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Turn on <onbit> of dip switch."), QCoreApplication::translate("main", "Bit to SET")));
    _cl.clear();

    _cl.append("off");
    _cl.append("dipsw-off");
    _cl.append("offbit");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Turn off <offbit> of dip switch."), QCoreApplication::translate("main", "Bit to RESET")));
    _cl.clear();

    _cl.append("g");
    _cl.append("gl");
    _cl.append("opengl");
    _cl.append("render");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Force set using renderer type."), "render", QCoreApplication::translate("main", "Renderer TYPE")));
    _cl.clear();

    _ret.append(QCommandLineOption("gl2", QCoreApplication::translate("main", "Force set using renderer type to OpenGL v2.")));
    _ret.append(QCommandLineOption("gl3", QCoreApplication::translate("main", "Force set using renderer type to OpenGL v3(main profile).")));

    _cl.append("gl4");
    _cl.append("gl4core");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Force set using renderer type to OpenGL v4(core profile).")));
	_cl.clear();

    _cl.append("gl43");
    _cl.append("gl4_3");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Force set using renderer type to OpenGL v4.3(core profile).")));
	_cl.clear();
    _cl.append("gl46");
    _cl.append("gl4_6");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Force set using renderer type to OpenGL v4.6(core profile).")));
	_cl.clear();

    _cl.append("gles");
    _cl.append("gles2");
    _cl.append("es2");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Force set using renderer type to OpenGL ESv2.")));
	_cl.clear();

    _cl.append("gles3");
    _cl.append("es3");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Force set using renderer type to OpenGL ESv3.")));
	_cl.clear();

    _cl.append("v");
    _cl.append("env");
    _cl.append("envvar");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Set / Delete environment variable."), QCoreApplication::translate("main", "NAME=VALUE")));
    _cl.clear();

    _cl.append("dump-env");
    _cl.append("dump-envvar");
    _ret.append(QCommandLineOption(_cl, QCoreApplication::translate("main", "Dump environment variables.")));
    _cl.clear();

	if(parser != nullptr) {
		parser->addOptions(_ret);
	}
	return _ret;
}

DLL_PREFIX void ProcessCmdLine_Sub(QCommandLineParser *cmdparser)
{
	char homedir[_MAX_PATH] = {0};
	std::string delim;
	if(cmdparser->isSet("homedir")) {
		strncpy(homedir, cmdparser->value("homedir").toLocal8Bit().constData(), _MAX_PATH - 1);
		cpp_homedir = homedir;
		size_t _len = cpp_homedir.length() - 1;
		size_t _pos = cpp_homedir.rfind(delim);
		if((_pos < _len) ||
		   (_pos == std::string::npos)) {
			cpp_homedir.append(delim);
		}
	} else {
		cpp_homedir.copy(homedir, _MAX_PATH - 1, 0);
	}

	if(cmdparser->isSet("cfgfile")) {
		char tmps[_MAX_PATH] = {0};
		std::string tmpstr;
		strncpy(tmps, cmdparser->value("cfgfile").toLocal8Bit().constData(), _MAX_PATH - 1);
		cpp_confdir = tmps;
		size_t _len = cpp_confdir.length() - 1;
		size_t _pos = cpp_confdir.rfind(delim);
		if((_pos < _len) ||
		   (_pos == std::string::npos)) {
			cpp_confdir.append(delim);
		}
	}
	if(cmdparser->isSet("resdir")) {
		char tmps[_MAX_PATH] = {0};
		std::string tmpstr;
		strncpy(tmps, cmdparser->value("resdir").toLocal8Bit().constData(), _MAX_PATH - 1);
		sRssDir = tmps;
		size_t _len = sRssDir.length() - 1;
		size_t _pos = sRssDir.rfind(delim);
		if((_pos < _len) ||
		   (_pos == std::string::npos)) {
			sRssDir.append(delim);
		}
	}

	if(cmdparser->isSet("envvar")) {
		QStringList nList = cmdparser->values("envvar");
		QString tv;
		//QProcessEnvironment ev = QProcessEnvironment::systemEnvironment();
		QProcessEnvironment ev = _envvars;
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
			_envvars.swap(ev);
		}
	}
	_b_dump_envvar = false;
	if(cmdparser->isSet("dump-envvar")) {
		_b_dump_envvar = true;
	}
}
