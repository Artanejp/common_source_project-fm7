/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

#include "emu.h"
#include <string>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QString>
#include <QObject>
#include "emu_thread.h"
#include "draw_thread.h"
#include "qt_gldraw.h"
#include "agar_logger.h"

OSD::OSD() : QThread(0)
{
   	VMSemaphore = new QSemaphore(1);
	locked_vm = false;
}

OSD::~OSD()
{
  	delete VMSemaphore;
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
#ifdef USE_AUTO_KEY	
	connect(parent, SIGNAL(sig_auto_key_string(QByteArray)), this, SLOT(set_auto_key_string(QByteArray)));
#endif	
}

void OSD::set_draw_thread(DrawThreadClass *handler)
{
	this->moveToThread(handler);
	connect(this, SIGNAL(sig_update_screen(bitmap_t *)), handler, SLOT(do_update_screen(bitmap_t *)));
	connect(this, SIGNAL(sig_save_screen(const char *)), glv, SLOT(do_save_frame_screen(const char *)));
	connect(this, SIGNAL(sig_close_window()), parent_thread, SLOT(doExit()));
	connect(this, SIGNAL(sig_resize_vm_screen(QImage *, int, int)), glv, SLOT(do_set_texture_size(QImage *, int, int)));
	connect(this, SIGNAL(sig_console_input_string(QString)), parent_thread, SLOT(do_call_debugger_command(QString)));
	connect(parent_thread, SIGNAL(sig_debugger_input(QString)), this, SLOT(do_set_input_string(QString)));
	connect(parent_thread, SIGNAL(sig_quit_debugger()), this, SLOT(do_close_debugger_thread()));
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
	release_printer();
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
	emit sig_close_window();
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
	tmps = tmps + QString::fromUtf8(CONFIG_NAME);
	tmps = tmps + QString::fromUtf8("_");
	tmps = tmps + nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz."));
	tmps = tmps + QString::fromUtf8(extension);
	snprintf(name, length, _T("%s"), tmps.toLocal8Bit().constData());
}

void OSD::lock_vm(void)
{
	locked_vm = true;
	if(parent_thread != NULL) { 
		if(!parent_thread->now_debugging()) VMSemaphore->acquire(1);
	} else {
		VMSemaphore->acquire(1);
	}
}

void OSD::unlock_vm(void)
{
	locked_vm = false;
	if(parent_thread != NULL) { 
		if(!parent_thread->now_debugging()) VMSemaphore->release(1);
	} else {
		VMSemaphore->release(1);
	}
}

bool OSD::is_vm_locked(void)
{
	return locked_vm;
}

void OSD::force_unlock_vm(void)
{
	if(parent_thread == NULL) {
		while(VMSemaphore->available() < 1) VMSemaphore->release(1);
		return;
	}
	if(parent_thread->now_debugging()) return;
	while(VMSemaphore->available() < 1) VMSemaphore->release(1);
}
