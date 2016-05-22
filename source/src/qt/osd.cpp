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

OSD::OSD() : OSD_BASE()
{
}

OSD::~OSD()
{
}

extern std::string cpp_homedir;
extern std::string my_procname;

//EmuThreadClass *OSD::get_parent_handler()
//{
//	return parent_thread;
//}

//void OSD::set_parent_thread(EmuThreadClass *parent)
//{
//	parent_thread = parent;
//}

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

