/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

#include "osd.h"
#include <string>
#include <QDataTime>
#include <QDate>
#include <QTime>
#include <QString>

OSD::OSD() : public QThread(0)
{

}

OSD::~OSD()
{
}

extern std::string cpp_homedir;
void OSD::initialize(int rate, int samples)
{
	// get module path
	QString tmp_path;
	tmp_path.fromStdString(cpp_homedir);
	tmp_path = tmp_path + QString::fromStdString(my_procname);
#if defined(Q_OS_WIN)
	const char *delim = "\\";
#else
	const char *delim = "/";
#endif
	tmp_path = tmp_path + tmp_path.fromUtf8(delim);
	memset(app_path, 0x00, sizeof(app_path);
	strncpy(app_path, tmp_path.toUtf8().constData(), _MAX_PATH);
	
		   //CoInitialize(NULL);
	initialize_input();
//	initialize_printer();
	initialize_screen();
	initialize_sound(rate, samples);
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	initialize_video();
#endif
#ifdef USE_SOCKET
	initialize_socket();
#endif
}

void OSD::release()
{
	release_input();
//	release_printer();
	release_screen();
	release_sound();
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	release_video();
#endif
#ifdef USE_SOCKET
	release_socket();
#endif
	//CoUninitialize();
}

void OSD::power_off()
{
	emit sig_window_close();
}

void OSD::suspend()
{
#ifdef USE_MOVIE_PLAYER
	if(now_movie_play && !now_movie_pause) {
		pause_movie();
		now_movie_pause = false;
	}
#endif
	mute_sound();
}

void OSD::restore()
{
#ifdef USE_MOVIE_PLAYER
	if(now_movie_play && !now_movie_pause) {
		play_movie();
	}
#endif
}

_TCHAR* OSD::bios_path(const _TCHAR* file_name)
{
	static _TCHAR file_path[_MAX_PATH];
	snprintf(file_path, _MAX_PATH, _T("%s%s"), app_path, file_name);
	return file_path;
}

void OSD::get_host_time(cur_time_t* time)
{
	//SYSTEMTIME sTime;
	//GetLocalTime(&sTime);
	QDateTime nowTime = QDateTime::currentDateTime();
	QDate d = nowTime.date();
	QTime t = nowTime.time();

	time->year = d.year();
	time->month = d.month();
	time->day = d.day();
	time->day_of_week = d.dayOfWeek();
	time->hour = t.hour();
	time->minute = t.mnute();
	time->second = t.second();
}

void OSD::sleep(uint32 ms)
{
	QThread::msleep(ms);
}

void OSD::create_date_file_name(_TCHAR *name, int length, _TCHAR *extension)
{
	QDateTime nowTime = QDateTime::currentDateTime();
	QString tmps = QString::fromUtf8("emu");
	tmps = tmps + QString::fromUtf8(CONFIG_NAME);
	tmps = tmps + QString::fromUtf8("_");
	tmps = tmps + nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz"));
	tmps = tmps + QString::fromUtf8(extension);
	snprintf(name, length, _T("%s%s"), app_path, tmps.toLocal8Bit().constData());
}

