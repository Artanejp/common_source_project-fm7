/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

//#include "emu.h"
#include <string>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QString>
#include <QObject>
#include <QThread>

#include "qt_gldraw.h"
#include "agar_logger.h"
#include "osd.h"

OSD::OSD() : QThread(0)
{
   	VMSemaphore = new QSemaphore(1);
   	DebugSemaphore = new QSemaphore(1);
	locked_vm = false;
}

OSD::~OSD()
{
  	delete VMSemaphore;
	delete DebugSemaphore;
}

extern std::string cpp_homedir;
extern std::string my_procname;

EmuThreadClass *OSD::get_parent_handler()
{
	return parent_thread;
}

void OSD::set_parent_thread(EmuThreadClass *parent)
{
	parent_thread = parent;
}

void OSD::initialize(int rate, int samples)
{
	// get module path
	QString tmp_path;
	tmp_path = QString::fromStdString(cpp_homedir);
	tmp_path = tmp_path + QString::fromStdString(my_procname);
#if defined(Q_OS_WIN)
	const char *delim = "\\";
#else
	const char *delim = "/";
#endif
	tmp_path = tmp_path + QString::fromUtf8(delim);
	memset(app_path, 0x00, sizeof(app_path));
	strncpy(app_path, tmp_path.toUtf8().constData(), _MAX_PATH);
	
	//memset(console_string, 0x00, sizeof(console_string));
	console_cmd_str.clear();
	osd_console_opened = false;
	osd_timer.start();
	//CoInitialize(NULL);
	initialize_input();
	initialize_printer();
	initialize_screen();
	initialize_sound(rate, samples);
	if(get_use_movie_player() || get_use_video_capture()) initialize_video();
	if(get_use_socket()) initialize_socket();
}

void OSD::release()
{
	release_input();
	release_printer();
	release_screen();
	release_sound();
	if(get_use_movie_player() || get_use_video_capture()) release_video();
	if(get_use_socket()) release_socket();
	//CoUninitialize();
}

void OSD::power_off()
{
	emit sig_close_window();
}

void OSD::suspend()
{
	if(get_use_movie_player()) {
		if(now_movie_play && !now_movie_pause) {
			pause_movie();
			now_movie_pause = false;
		}
	}
	mute_sound();
}

void OSD::restore()
{
	if(get_use_movie_player()) {
		if(now_movie_play && !now_movie_pause) {
			play_movie();
		}
	}
}

_TCHAR* OSD::bios_path(const _TCHAR* file_name)
{
	static _TCHAR file_path[_MAX_PATH];
	snprintf(file_path, _MAX_PATH, _T("%s%s"), app_path, file_name);
	AGAR_DebugLog(AGAR_LOG_INFO, "BIOS PATH:%s", file_path);
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
	time->minute = t.minute();
	time->second = t.second();
}

void OSD::sleep(uint32_t ms)
{
	QThread::msleep(ms);
}

void OSD::create_date_file_name(_TCHAR *name, int length, const _TCHAR *extension)
{
	QDateTime nowTime = QDateTime::currentDateTime();
	QString tmps = QString::fromUtf8("emu");
	tmps = tmps + get_vm_config_name();
	tmps = tmps + QString::fromUtf8("_");
	tmps = tmps + nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz."));
	tmps = tmps + QString::fromUtf8(extension);
	snprintf(name, length, _T("%s"), tmps.toLocal8Bit().constData());
}

