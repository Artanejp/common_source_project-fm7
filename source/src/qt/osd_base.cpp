/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

//#include "emu.h"
#include <string>

#include <QObject>
#include <QWidget>

#include <mutex>
#include <chrono>

#include <QSemaphore>
#include <QPainter>
#include <QElapsedTimer>
#include <QQueue>

#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QString>
#include <QObject>
#include <QThread>

#include <QOpenGLContext>
#include <QtMultimedia>

#include "simd_types.h"

#include <ctime>
#include <limits>
#include <memory>

#include "../config.h"
#include "../fileio.h"
#include "../fifo.h"
#include "../vm/device.h"

#include "qt_input.h"
#include "osd_base.h"

#include "gui/dock_disks.h"
#include "gui/menu_flags.h"

#include "../vm/vm_template.h"

OSD_BASE::OSD_BASE(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger) : QObject(0)
{
	using_flags = p;
	locked_vm = false;
	device_node_list.clear();
	max_vm_nodes = 0;
	p_logger = logger;
	vm = NULL;

	SupportedFeatures.clear();

	is_glcontext_shared = false;
	glContext = NULL;

	m_sound_samples = -1;
	m_sound_rate = -1;
	m_fps = 0.0;
	m_sound_samples_factor = 0;
	m_sound_samples_count = 0;

	m_sound_driver.reset();
	m_sound_thread = nullptr;
	m_sound_period = 0;
	m_draw_thread.reset();
	sound_initialized = false;

	connect(this, SIGNAL(sig_debug_log(int, int, QString)), p_logger.get(), SLOT(do_debug_log(int, int, QString)), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_logger_reset()), p_logger.get(), SLOT(reset()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_logger_set_device_name(int, QString)), p_logger.get(), SLOT(do_set_device_name(int, QString)), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_logger_set_cpu_name(int, QString)), p_logger.get(), SLOT(do_set_cpu_name(int, QString)), Qt::QueuedConnection);
}

OSD_BASE::~OSD_BASE()
{
	if(log_mutex.try_lock_for(std::chrono::milliseconds(100))) {
		p_logger->set_osd(NULL);
		log_mutex.unlock();
	}
}

const _TCHAR *OSD_BASE::get_lib_osd_version()
{
	const _TCHAR *p = (const _TCHAR *)"\0";
#if defined(__LIBOSD_VERSION)
	return (const _TCHAR *)__LIBOSD_VERSION;
#endif
	return p;
}

QOpenGLContext *OSD_BASE::get_gl_context()
{
	if(glContext == NULL) return NULL;
	if(!(glContext->isValid())) return NULL;
	return glContext;
}

EmuThreadClass *OSD_BASE::get_parent_handler()
{
	return parent_thread;
}

const _TCHAR *OSD_BASE::get_vm_device_name()
{
	if(using_flags != NULL) {
		QString s = using_flags->get_device_name();
		static QByteArray __n = s.toUtf8();
		return (const _TCHAR*)(__n.constData());
	}
	return (const _TCHAR*)"";
}

void OSD_BASE::set_parent_thread(EmuThreadClass *parent)
{
	parent_thread = parent;
}

void OSD_BASE::initialize(int rate, int samples, int* presented_rate, int* presented_samples)
{
}

void OSD_BASE::release()
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
	unmute_sound();
}


void OSD_BASE::debug_log(int level, const char *fmt, ...)
{
	char strbuf[4096];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(strbuf, 4095, fmt, ap);
	debug_log(level, 0, strbuf);
	va_end(ap);
}

void OSD_BASE::debug_log(int level, int domain_num, const char *fmt, ...)
{
	char strbuf[4096];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(strbuf, 4095, fmt, ap);
	debug_log(level, domain_num, strbuf);
	va_end(ap);
}

_TCHAR *OSD_BASE::get_app_path(void)
{
	return app_path;
}

_TCHAR* OSD_BASE::bios_path(const _TCHAR* file_name)
{
	static _TCHAR file_path[_MAX_PATH];
	snprintf((char *)file_path, _MAX_PATH, "%s%s", app_path, (const char *)file_name);
	debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_OSD, "BIOS PATH:%s", file_path);
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


bool OSD_BASE::get_use_socket(void)
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
	// ToDo: Really need to lock? 20221011 K.O
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);

	if(vm != NULL) {
		vm->key_down(code, flag);
	}
}

void OSD_BASE::vm_key_up(int code)
{
	// ToDo: Really need to lock? 20221011 K.O
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);

	if(vm != NULL) {
		vm->key_up(code);
	}
}

void OSD_BASE::vm_reset(void)
{
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);
	if(vm != NULL) {
		vm->reset();
	}
}

void OSD_BASE::notify_power_off(void)
{
	emit sig_notify_power_off();
}

int OSD_BASE::get_vm_buttons_code(int num)
{
	return 0;
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
	vm_mutex.lock();
}

void OSD_BASE::unlock_vm(void)
{
	vm_mutex.unlock();
	locked_vm = false;
}


bool OSD_BASE::is_vm_locked(void)
{
	return locked_vm;
}

void OSD_BASE::force_unlock_vm(void)
{
	vm_mutex.unlock();
	locked_vm = false;
}

void OSD_BASE::set_draw_thread(std::shared_ptr<DrawThreadClass> handler)
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

double OSD_BASE::get_window_mode_power(int mode)
{
	if(mode + 1 == 2) {
		return 1.5;
	} else if(mode + 1 > 2) {
		return mode + 1 - 1;
	}
	return mode + 1;
}

void OSD_BASE::reset_vm_node(void)
{
	device_node_t sp;
	device_node_list.clear();
	emit sig_logger_reset();
	max_vm_nodes = 0;
}

void OSD_BASE::sync_some_devices()
{
	emit sig_reset_joystick();
}

void OSD_BASE::debug_log(int level, int domain_num, char *strbuf)
{
	QString tmps = QString::fromLocal8Bit(strbuf);
	emit sig_debug_log(level, domain_num, tmps);
}


void OSD_BASE::set_device_name(int id, char *name)
{
	if(name != nullptr) {
		emit sig_logger_set_device_name(id, QString::fromUtf8(name));
	} else {
		emit sig_logger_set_device_name(id, QString::fromUtf8(""));
	}
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
			set_device_name(id, (char *)name);
			device_node_list.replace(i, sp);
			if(id >= max_vm_nodes) max_vm_nodes = id + 1;
			return;
		}
	}
	// Not Found
	sp.id = id;
	sp.name = name;
	set_device_name(id, (char *)name);
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
		l.v.fvalue = value;
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
		l.v.ivalue = value;
		SupportedFeatures.append(l);
	}
}

void OSD_BASE::add_feature(const _TCHAR *key, uint64_t value)
{
	QString tmps;
	supportedlist_t l;
	tmps = QString::fromUtf8(key);
	if(!check_feature(key)) {
		l.string = tmps;
		l.v.uvalue = value;
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


void OSD_BASE::add_feature(const _TCHAR *key, int16_t value)
{
	add_feature(key, (int64_t)value);
}

void OSD_BASE::add_feature(const _TCHAR *key, int8_t value)
{
	add_feature(key, (int64_t)value);
}

void OSD_BASE::add_feature(const _TCHAR *key, uint32_t value)
{
	add_feature(key, (uint64_t)(value & 0xffffffff));
}

void OSD_BASE::add_feature(const _TCHAR *key, uint16_t value)
{
	add_feature(key, (uint64_t)(value & 0xffff));
}

void OSD_BASE::add_feature(const _TCHAR *key, uint8_t value)
{
	add_feature(key, (uint64_t)(value & 0xff));
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
			return l.v.fvalue;
		}
	}
	return std::numeric_limits<double>::quiet_NaN(); // You don't use (0.0 / 0.0).
}

int64_t OSD_BASE::get_feature_int64_value(const _TCHAR *key)
{
	QString tmps;
	supportedlist_t l;
	tmps = QString::fromUtf8(key);
	for(int i = 0; i < SupportedFeatures.size(); i++) {
		l = SupportedFeatures.at(i);
		if(l.string == tmps) {
			return l.v.ivalue;
		}
	}
	return 0;
}

int OSD_BASE::get_feature_int_value(const _TCHAR *key)
{
	return (int)get_feature_int64_value(key);
}

int32_t OSD_BASE::get_feature_int32_value(const _TCHAR *key)
{
	return (int32_t)get_feature_int64_value(key);
}

int16_t OSD_BASE::get_feature_int16_value(const _TCHAR *key)
{
	return (int16_t)get_feature_int64_value(key);
}

int8_t OSD_BASE::get_feature_int8_value(const _TCHAR *key)
{
	return (int8_t)get_feature_int64_value(key);
}


uint64_t OSD_BASE::get_feature_uint64_value(const _TCHAR *key)
{
	QString tmps;
	supportedlist_t l;
	tmps = QString::fromUtf8(key);
	for(int i = 0; i < SupportedFeatures.size(); i++) {
		l = SupportedFeatures.at(i);
		if(l.string == tmps) {
			return l.v.uvalue;
		}
	}
	return 0;
}

uint32_t OSD_BASE::get_feature_uint32_value(const _TCHAR *key)
{
	return (uint32_t)(get_feature_uint64_value(key) & 0xffffffff);
}

uint16_t OSD_BASE::get_feature_uint16_value(const _TCHAR *key)
{
	return (uint16_t)(get_feature_uint64_value(key) & 0xffff);
}

uint8_t OSD_BASE::get_feature_uint8_value(const _TCHAR *key)
{
	return (uint8_t)(get_feature_uint64_value(key) & 0xff);
}

void OSD_BASE::start_waiting_in_debugger()
{
	// ToDo: Wait for rising up debugger window.
	debug_mutex.lock();
}

void OSD_BASE::finish_waiting_in_debugger()
{
	// ToDo: Wait for closing up debugger window.
	debug_mutex.unlock();
}

void OSD_BASE::process_waiting_in_debugger()
{
	// This is workaround for locking up when calling debugger.
	if(is_vm_locked()) {
		unlock_vm();
	}
	// ToDo: Check sequence
	QThread::msleep(10);
}

void OSD_BASE::set_dbg_completion_list(std::list<std::string> *p)
{
	if(p != NULL) {
		emit sig_clear_dbg_completion_list();
		for(auto n = p->begin(); n != p->end(); ++n) {
			emit sig_add_dbg_completion_list((_TCHAR *)((*n).c_str()));
		}
		emit sig_apply_dbg_completion_list();
	}
}

void OSD_BASE::clear_dbg_completion_list(void)
{
	emit sig_clear_dbg_completion_list();
	emit sig_apply_dbg_completion_list();
}

// Belows are API for GUI STATUS BAR.
void OSD_BASE::set_hdd_image_name(int drv, _TCHAR *filename)
{
	QString _n = QString::fromLocal8Bit(filename);
	emit sig_change_virtual_media(CSP_DockDisks_Domain_HD, drv, _n);
}

// Moved from OSD_WRAPPER.
const _TCHAR *OSD_BASE::get_lib_common_vm_version()
{
	return (const _TCHAR *)"\0";
}

const _TCHAR *OSD_BASE::get_lib_common_vm_git_version()
{
	// ToDo: Really need to lock? 20221011 K.O
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);
	return vm->get_vm_git_version();
}


// Screen
void OSD_BASE::vm_draw_screen(void)
{
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);
	vm->draw_screen();
}

double OSD_BASE::vm_frame_rate(void)
{
	// ToDo: Really need to lock? 20221011 K.O
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);
	return vm->get_frame_rate();
}

Sint16* OSD_BASE::create_sound(int *extra_frames)
{
	// ToDo: Really need to lock? 20221011 K.O
	std::lock_guard<std::recursive_timed_mutex> lv(vm_mutex);
	return (Sint16 *)vm->create_sound(extra_frames);
}

void OSD_BASE::osdcall_message_str(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type, QString message)
{
	uint64_t _type = media_type & EMU_MEDIA_TYPE::UI_MEDIA_MASK;
	uint64_t _slot = media_type & 255;
}

void OSD_BASE::osdcall_message_int(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type, int64_t data)
{
	uint64_t _type = media_type & EMU_MEDIA_TYPE::UI_MEDIA_MASK;
	uint64_t _slot = media_type & 255;
}

void OSD_BASE::osdcall_mount(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type, QString path)
{
	uint64_t _type = media_type & EMU_MEDIA_TYPE::UI_MEDIA_MASK;
	uint64_t _slot = media_type & 255;
	switch(_type) {
	case EMU_MEDIA_TYPE::BINARY :
		if((message_type & EMU_MESSAGE_TYPE::MEDIA_PLAYREC_MASK) == 0)
		{
			emit sig_ui_binary_loading_insert_history(drive, path);
		} else {
			emit sig_ui_binary_saving_insert_history(drive, path);
		}
		break;
	case EMU_MEDIA_TYPE::BUBBLE_CASETTE :
		emit sig_ui_bubble_insert_history(drive,
										  path,
										  (quint64)_slot);
		emit sig_ui_bubble_write_protect(drive,
										 (quint64)(message_type & EMU_MESSAGE_TYPE::WRITE_PROTECT));
		break;
	case EMU_MEDIA_TYPE::CARTRIDGE :
		emit sig_ui_cartridge_insert_history(drive,
											 path);
		break;
	case EMU_MEDIA_TYPE::COMPACT_DISC :
		emit sig_ui_compact_disc_insert_history(drive,
												path);
		break;
	case EMU_MEDIA_TYPE::FLOPPY_DISK :
		emit sig_ui_floppy_insert_history(drive,
										  path,
										  (quint64)_slot);
		emit sig_ui_floppy_write_protect(drive,
										 (quint64)(message_type & EMU_MESSAGE_TYPE::WRITE_PROTECT));
		break;
	case EMU_MEDIA_TYPE::HARD_DISK :
		emit sig_ui_hard_disk_insert_history(drive,
											 path);
		break;
	case EMU_MEDIA_TYPE::LASER_DISC :
		emit sig_ui_laser_disc_insert_history(drive,
											  path);
		break;
	case EMU_MEDIA_TYPE::QUICK_DISK :
		// ToDo: Write protect and bank
		emit sig_ui_quick_disk_insert_history(drive,
											  path);
		break;
	case EMU_MEDIA_TYPE::TAPE :
		if((message_type & EMU_MESSAGE_TYPE::RECORD) != 0) {
			emit sig_ui_tape_record_insert_history(drive, path);
		} else {
			emit sig_ui_tape_play_insert_history(drive, path);
		}
		break;
	default:
		break;
	}
}

void OSD_BASE::osdcall_unmount(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type)
{
	uint64_t _type = media_type & EMU_MEDIA_TYPE::UI_MEDIA_MASK;
	uint64_t _slot = media_type & 255;
	switch(_type) {
	case EMU_MEDIA_TYPE::BINARY :
		emit sig_ui_binary_closed(drive);
		break;
	case EMU_MEDIA_TYPE::BUBBLE_CASETTE :
		emit sig_ui_bubble_closed(drive);
		break;
	case EMU_MEDIA_TYPE::CARTRIDGE :
		emit sig_ui_cartridge_eject(drive);
		break;
	case EMU_MEDIA_TYPE::COMPACT_DISC :
		emit sig_ui_compact_disc_eject(drive);
		break;
	case EMU_MEDIA_TYPE::FLOPPY_DISK :
		emit sig_ui_floppy_close(drive);
		break;
	case EMU_MEDIA_TYPE::HARD_DISK :
		emit sig_ui_hard_disk_close(drive);
		break;
	case EMU_MEDIA_TYPE::LASER_DISC :
		emit sig_ui_laser_disc_eject(drive);
		break;
	case EMU_MEDIA_TYPE::QUICK_DISK :
		emit sig_ui_quick_disk_close(drive);
		break;
	case EMU_MEDIA_TYPE::TAPE :
		// ToDo: close by rec/play.
		emit sig_ui_tape_eject(drive);
		break;
	default:
		break;
	}
}

void OSD_BASE::osdcall_misc(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type, QString message_str, int64_t data)
{
	uint64_t _type = media_type & EMU_MEDIA_TYPE::UI_MEDIA_MASK;
	uint64_t _slot = media_type & 255;

	switch(_type) {
	case EMU_MEDIA_TYPE::BINARY :
		// ToDO : write protect.
		break;
	case EMU_MEDIA_TYPE::BUBBLE_CASETTE :
		emit sig_ui_bubble_write_protect(drive,
										 (quint64)(message_type & EMU_MESSAGE_TYPE::WRITE_PROTECT));
		break;
	case EMU_MEDIA_TYPE::CARTRIDGE :
		// ToDo
		break;
	case EMU_MEDIA_TYPE::COMPACT_DISC :
		// ToDo
		break;
	case EMU_MEDIA_TYPE::FLOPPY_DISK :
		// ToDo
		emit sig_ui_floppy_write_protect(drive,
										 (quint64)(message_type & EMU_MESSAGE_TYPE::WRITE_PROTECT));
		break;
	case EMU_MEDIA_TYPE::HARD_DISK :
		// ToDo
		break;
	case EMU_MEDIA_TYPE::LASER_DISC :
		// ToDo
		break;
	case EMU_MEDIA_TYPE::QUICK_DISK :
		// ToDo: Write protect and bank
		break;
	case EMU_MEDIA_TYPE::TAPE :
	{

		bool is_pause_unpause = ((message_type & EMU_MESSAGE_TYPE::TOGGLE_PAUSE) != 0) ? true : false;

		if(!(is_pause_unpause)) {
			switch(message_type & EMU_MESSAGE_TYPE::MEDIA_FFREW_MASK) {
			case EMU_MESSAGE_TYPE::TAPE_FF:
				emit sig_ui_tape_push_fast_forward(drive);
				break;
			case EMU_MESSAGE_TYPE::TAPE_REW:
				emit sig_ui_tape_push_fast_rewind(drive);
				break;
			case EMU_MESSAGE_TYPE::TAPE_APSS_FF:
				emit sig_ui_tape_push_apss_forward(drive);
				break;
			case EMU_MESSAGE_TYPE::TAPE_APSS_REW:
				emit sig_ui_tape_push_apss_rewind(drive);
				break;
			default:
				// ToDo: write protect
				break;
			}
		}
	}
	break;
	}
}

void OSD_BASE::string_message_from_emu(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type, _TCHAR* message)
{
//	switch(media_type) {
//	case EMU_MEDIA_TYPE::BINARY:
//	}
	QString tmps;
	tmps.clear();
	// Below are update status to UI.
	if(message != nullptr) {
		tmps = QString::fromLocal8Bit(message);
	}
	EMU_MESSAGE_TYPE::type_t message_mode
		= message_type & EMU_MESSAGE_TYPE::MEDIA_MODE_MASK;
	switch(message_type & (EMU_MESSAGE_TYPE::TYPE_MASK))
	{
	case EMU_MESSAGE_TYPE::TYPE_MESSAGE:
		osdcall_message_str(media_type, drive, message_type, tmps);
		break;
	case EMU_MESSAGE_TYPE::TYPE_MEDIA:
		switch(message_mode) {
		case EMU_MESSAGE_TYPE::MEDIA_MOUNTED :
			osdcall_mount(media_type, drive, message_type, tmps);
			break;
		case EMU_MESSAGE_TYPE::MEDIA_REMOVED :
			osdcall_unmount(media_type, drive, message_type);
			break;
		case EMU_MESSAGE_TYPE::MEDIA_OTHERS  :
			osdcall_misc(media_type, drive, message_type, tmps, INT64_MIN);
			break;
		default:
			break;
		}
	default:
		break;
	}
}


void OSD_BASE::int_message_from_emu(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type, int64_t data)
{
	EMU_MESSAGE_TYPE::type_t message_mode
		= message_type & EMU_MESSAGE_TYPE::MEDIA_MODE_MASK;
	switch(message_type & (EMU_MESSAGE_TYPE::TYPE_MASK))
	{
	case EMU_MESSAGE_TYPE::TYPE_MESSAGE:
		osdcall_message_int(media_type, drive, message_type, data);
		break;
	case EMU_MESSAGE_TYPE::TYPE_MEDIA:
		switch(message_mode) {
		case EMU_MESSAGE_TYPE::MEDIA_MOUNTED :
			// ToDo.
			break;
		case EMU_MESSAGE_TYPE::MEDIA_REMOVED :
			osdcall_unmount(media_type, drive, message_type);
			break;
		case EMU_MESSAGE_TYPE::MEDIA_OTHERS  :
			osdcall_misc(media_type, drive, message_type, QString::fromUtf8(""), data);
			break;
		default:
			break;
		}
		break;
	}
}
