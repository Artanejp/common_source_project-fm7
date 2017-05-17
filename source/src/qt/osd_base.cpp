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
#include <QMutex>

#include "qt_gldraw.h"
#include "csp_logger.h"
#include "osd_base.h"

OSD_BASE::OSD_BASE(USING_FLAGS *p, CSP_Logger *logger) : QThread(0)
{
	using_flags = p;
	p_config = using_flags->get_config_ptr();
   	VMSemaphore = new QSemaphore(1);
   	DebugSemaphore = new QSemaphore(1);
	locked_vm = false;
	screen_mutex = new QMutex(QMutex::Recursive);
	device_node_list.clear();
	max_vm_nodes = 0;
	csp_logger = logger;
	SupportedFeatures.clear();
}

OSD_BASE::~OSD_BASE()
{
  	delete VMSemaphore;
	delete DebugSemaphore;
	delete screen_mutex;
}

extern std::string cpp_homedir;
extern std::string my_procname;

EmuThreadClass *OSD_BASE::get_parent_handler()
{
	return parent_thread;
}

void OSD_BASE::set_parent_thread(EmuThreadClass *parent)
{
	parent_thread = parent;
}

void OSD_BASE::initialize(int rate, int samples)
{
}

void OSD_BASE::release()
{
}

void OSD_BASE::power_off()
{
}

void OSD_BASE::suspend()
{
	if(get_use_movie_player()) {
		if(now_movie_play && !now_movie_pause) {
			pause_movie();
			now_movie_pause = false;
		}
	}
	mute_sound();
}

void OSD_BASE::restore()
{
	if(get_use_movie_player()) {
		if(now_movie_play && !now_movie_pause) {
			play_movie();
		}
	}
}

_TCHAR *OSD_BASE::get_app_path(void)
{
	return app_path;
}

_TCHAR* OSD_BASE::bios_path(const _TCHAR* file_name)
{
	static _TCHAR file_path[_MAX_PATH];
	snprintf((char *)file_path, _MAX_PATH, "%s%s", app_path, (const char *)file_name);
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_OSD, "BIOS PATH:%s", file_path);
	return file_path;
}

void OSD_BASE::get_host_time(cur_time_t* time)
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

void OSD_BASE::sleep(uint32_t ms)
{
	QThread::msleep(ms);
}

void OSD_BASE::create_date_file_name(_TCHAR *name, int length, const _TCHAR *extension)
{
	QDateTime nowTime = QDateTime::currentDateTime();
	QString tmps = QString::fromUtf8("emu");
	tmps = tmps + get_vm_config_name();
	tmps = tmps + QString::fromUtf8("_");
	tmps = tmps + nowTime.toString(QString::fromUtf8("yyyy-MM-dd_hh-mm-ss.zzz."));
	tmps = tmps + QString::fromUtf8((const char *)extension);
	snprintf((char *)name, length, "%s", tmps.toLocal8Bit().constData());
}

_TCHAR* OSD_BASE::application_path()
{
	return app_path;
}

void OSD_BASE::vm_draw_screen(void)
{

}

double OSD_BASE::vm_frame_rate(void)
{
	return 59.94;
}

Sint16* OSD_BASE::create_sound(int *extra_frames)
{
	return (Sint16 *)NULL;
}


bool OSD_BASE::get_use_socket(void)
{
	return false;
}
bool OSD_BASE::get_support_variable_timing(void)
{
	return false;
}

bool OSD_BASE::get_notify_key_down(void)
{
	return false;
}

bool OSD_BASE::get_notify_key_down_lr_shift(void)
{
	return false;
}

bool OSD_BASE::get_notify_key_down_lr_control(void)
{
	return false;
}

bool OSD_BASE::get_notify_key_down_lr_menu(void)
{
	return false;
}

bool OSD_BASE::get_use_shift_numpad_key(void)
{
	return false;
}

bool OSD_BASE::get_use_auto_key(void)
{
	return false;
}

bool OSD_BASE::get_dont_keeep_key_pressed(void)
{
	return false;
}

bool OSD_BASE::get_one_board_micro_computer(void)
{
	return false;
}

bool OSD_BASE::get_use_screen_rotate(void)
{
	return false;
}

bool OSD_BASE::get_use_movie_player(void)
{
	return false;
}

bool OSD_BASE::get_use_video_capture(void)
{
	return false;
}

void OSD_BASE::vm_key_down(int code, bool flag)
{
}

void OSD_BASE::vm_key_up(int code)
{
}

void OSD_BASE::vm_reset(void)
{

}

int OSD_BASE::get_vm_buttons_code(int num)
{
	return 0;
}	

void OSD_BASE::update_buttons(bool press_flag, bool release_flag)
{
}	

QString OSD_BASE::get_vm_config_name(void)
{
	return QString::fromUtf8(" ");
}

int OSD_BASE::get_screen_width(void)
{
	return 320;
}

int OSD_BASE::get_screen_height(void)
{
	return 200;
}

void OSD_BASE::lock_vm(void)
{
	locked_vm = true;
}

void OSD_BASE::unlock_vm(void)
{
	locked_vm = false;
}


bool OSD_BASE::is_vm_locked(void)
{
	return locked_vm;
}

void OSD_BASE::force_unlock_vm(void)
{
}

void OSD_BASE::set_draw_thread(DrawThreadClass *handler)
{
}

void OSD_BASE::initialize_screen()
{
	first_draw_screen = false;
	first_invalidate = true;
	self_invalidate = false;
}

void OSD_BASE::release_screen()
{
}

int OSD_BASE::get_window_mode_width(int mode)
{
	return 640;
}

int OSD_BASE::get_window_mode_height(int mode)
{
	return 200;
}

void OSD_BASE::reset_vm_node(void)
{
	device_node_list.clear();
	csp_logger->reset();
	max_vm_nodes = 0;
}

void OSD_BASE::set_vm_node(int id, const _TCHAR *name)
{
	device_node_t sp;
	int i;
	for(i = 0; i < device_node_list.size(); i++) {
		sp = device_node_list.at(i);
		if(id == sp.id) {
			sp.id = id;
			sp.name = name;
			csp_logger->set_device_name(id, (char *)name);
			device_node_list.replace(i, sp);
			if(id >= max_vm_nodes) max_vm_nodes = id + 1;
			return;
		}
	}
	// Not Found
	sp.id = id;
	sp.name = name;
	csp_logger->set_device_name(id, (char *)name);
	device_node_list.append(sp);
	if(id >= max_vm_nodes) max_vm_nodes = id + 1;
}

const _TCHAR *OSD_BASE::get_vm_node_name(int id)
{
	int i;
	device_node_t sp;
	for(i = 0; i < device_node_list.size(); i++) {
		sp = device_node_list.at(i);
		if(id == sp.id) {
			return sp.name;
		}
	}
	return NULL;
}

int OSD_BASE::get_vm_node_size(void)
{
	return max_vm_nodes;
}

void OSD_BASE::add_feature(const _TCHAR *key, double value)
{
	QString tmps;
	supportedlist_t l;
	tmps = QString::fromUtf8(key);
	if(!check_feature(key)) {
		l.string = tmps;
		l.fvalue = value;
		l.ivalue = 0;
		SupportedFeatures.append(l);
	}
}
void OSD_BASE::add_feature(const _TCHAR *key, int64_t value)
{
	QString tmps;
	supportedlist_t l;
	tmps = QString::fromUtf8(key);
	if(!check_feature(key)) {
		l.string = tmps;
		l.fvalue = 0.0;
		l.ivalue = value;
		SupportedFeatures.append(l);
	}
}

void OSD_BASE::add_feature(const _TCHAR *key, float value)
{
	add_feature(key, (double)value);
}

void OSD_BASE::add_feature(const _TCHAR *key, int value)
{
	add_feature(key, (int64_t)value);
}

void OSD_BASE::add_feature(const _TCHAR *key, uint32_t value)
{
	add_feature(key, (int64_t)(value & 0xffffffff));
}

void OSD_BASE::add_feature(const _TCHAR *key, uint16_t value)
{
	add_feature(key, (int64_t)(value & 0xffff));
}

void OSD_BASE::add_feature(const _TCHAR *key, uint8_t value)
{
	add_feature(key, (int64_t)(value & 0xff));
}


bool OSD_BASE::check_feature(const _TCHAR *key)
{
	QString tmps;
	supportedlist_t l;
	tmps = QString::fromUtf8(key);
	for(int i = 0; i < SupportedFeatures.size(); i++) {
		l = SupportedFeatures.at(i);
		if(l.string == tmps) {
			return true;
		}
	}
	return false;
}

double OSD_BASE::get_feature_double_value(const _TCHAR *key)
{
	QString tmps;
	supportedlist_t l;
	tmps = QString::fromUtf8(key);
	for(int i = 0; i < SupportedFeatures.size(); i++) {
		l = SupportedFeatures.at(i);
		if(l.string == tmps) {
			return l.fvalue;
		}
	}
	return std::numeric_limits<double>::quiet_NaN(); // You don't use (0.0 / 0.0). 
}

int64_t OSD_BASE::get_feature_int_value(const _TCHAR *key)
{
	QString tmps;
	supportedlist_t l;
	tmps = QString::fromUtf8(key);
	for(int i = 0; i < SupportedFeatures.size(); i++) {
		l = SupportedFeatures.at(i);
		if(l.string == tmps) {
			return l.ivalue;
		}
	}
	return 0;
}

uint32_t OSD_BASE::get_feature_uint32_value(const _TCHAR *key)
{
	return (uint32_t)(get_feature_int_value(key) & 0xffffffff);
}

uint16_t OSD_BASE::get_feature_uint16_value(const _TCHAR *key)
{
	return (uint16_t)(get_feature_uint32_value(key) & 0xffff);
}

uint8_t OSD_BASE::get_feature_uint8_value(const _TCHAR *key)
{
	return (uint8_t)(get_feature_uint32_value(key) & 0xff);
}

		
		
